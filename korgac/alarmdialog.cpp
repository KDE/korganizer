/*
    This file is part of the KOrganizer alarm daemon.

    Copyright (c) 2000,2003 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/



#include <QLabel>
#include <QFile>
#include <QSpinBox>
#include <QLayout>
#include <QPushButton>
#include <QDataStream>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QBoxLayout>
#include <QtDBus>
#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kprocess.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <knotification.h>
#include <kcombobox.h>
#include <kwin.h>
#include <klockfile.h>
#include <kpushbutton.h>
#include <phonon/audioplayer.h>

#include <kcal/event.h>
#include <ktoolinvocation.h>
#include <kglobal.h>
#include <kvbox.h>

#include "koeventviewer.h"

#include "alarmdialog.h"
#include "alarmdialog.moc"
#include <korganizer_interface.h>

AlarmDialog::AlarmDialog( QWidget *parent )
  : KDialog( parent/*, Qt::WType_TopLevel | Qt::WStyle_Customize | Qt::WStyle_StaysOnTop |
                     Qt::WStyle_DialogBorder,*/ ),
                 mSuspendTimer(this)
{
  kapp->iconLoader()->addAppDir( "kdepim" );

  QWidget *topBox = new QWidget( this);
  setMainWidget( topBox );
  setCaption( i18n("Reminder") );
  setButtons( Ok | User1 | User2 );
  setDefaultButton( User1 );
  setButtonText( User1, i18n("Suspend") );
  setButtonText( User2, i18n("Edit...") );
  QBoxLayout *topLayout = new QVBoxLayout( topBox );
  topLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18n("The following events triggered reminders:"),
                              topBox );
  topLayout->addWidget( label );

  mEventViewer = new KOEventViewer( topBox );
  topLayout->addWidget( mEventViewer );

  KHBox *suspendBox = new KHBox( topBox );
  suspendBox->setSpacing( spacingHint() );
  topLayout->addWidget( suspendBox );

  QLabel *l = new QLabel( i18n("Suspend &duration:"), suspendBox );
  mSuspendSpin = new QSpinBox( suspendBox );
  mSuspendSpin->setRange( 1, 9999 );
  mSuspendSpin->setValue( 5 );  // default suspend duration
  l->setBuddy( mSuspendSpin );

  mSuspendUnit = new KComboBox( suspendBox );
  mSuspendUnit->addItem( i18n("minute(s)") );
  mSuspendUnit->addItem( i18n("hour(s)") );
  mSuspendUnit->addItem( i18n("day(s)") );
  mSuspendUnit->addItem( i18n("week(s)") );

  // showButton( User2/*3*/, false );

  setMinimumSize( 300, 200 );
  connect(this,SIGNAL(okClicked()),this, SLOT(slotOk()));
  connect(this,SIGNAL(user1Clicked()),this,SLOT(slotUser1()));
  connect(this,SIGNAL(user2Clicked()),this,SLOT(slotUser2()));
}

AlarmDialog::~AlarmDialog()
{
  delete mIncidence;
}

void AlarmDialog::setIncidence( Incidence *incidence )
{
  mIncidence = incidence->clone();
  mEventViewer->appendIncidence( mIncidence );
}

void AlarmDialog::setRemindAt( const QDateTime &dt )
{
  mRemindAt = dt;
}

void AlarmDialog::slotOk()
{
  accept();
  emit finishedSignal( this );
}

void AlarmDialog::slotUser1()
{
  if ( !isVisible() )
    return;

  int unit=1;
  switch ( mSuspendUnit->currentIndex() ) {
    case 3: // weeks
      unit *=  7;
    case 2: // days
      unit *= 24;
    case 1: // hours
      unit *= 60;
    case 0: // minutes
      unit *= 60;
    default:
      break;
  }

  setTimer( unit * mSuspendSpin->value() );
  accept();
}

void AlarmDialog::setTimer( int seconds )
{
  connect( &mSuspendTimer, SIGNAL( timeout() ), SLOT( show() ) );
  mSuspendTimer.setSingleShot( true );
  mSuspendTimer.start( 1000 * seconds );
  mRemindAt = QDateTime::currentDateTime();
  mRemindAt = mRemindAt.addSecs( seconds );
}

void AlarmDialog::slotUser2()
{
  if ( !QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.korganizer" ) ) {
    if ( KToolInvocation::startServiceByDesktopName( "korganizer", QString() ) )
      KMessageBox::error( 0, i18n("Could not start KOrganizer.") );
  }
  org::kde::korganizer::Korganizer korganizer("org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus());
  korganizer.editIncidence(mIncidence->uid());

#ifdef Q_OS_UNIX
#warning "kde4: verify it when kontact will not crash"
  // get desktop # where korganizer (or kontact) runs
  QString object = QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.kontact" ) ?
           "kontact-mainwindow_1" : "korganizer/MainWindow_1";
  QDBusInterface korganizerObj("org.kde.korganizer", '/'+object);
  QDBusReply<int> reply = korganizerObj.call( "getWinID" );
  if ( reply.isValid() ) {
    int window = reply;
    int desktop = KWin::windowInfo( window, NET::WMDesktop ).desktop();

    if ( KWin::currentDesktop() == desktop ) {
      KWin::iconifyWindow( winId(), false );
    }
    else
      KWin::setCurrentDesktop( desktop );

    KWin::activateWindow( KWin::transientFor( window ) );
  }
#endif
}

void AlarmDialog::show()
{
  KDialog::show();
#ifdef Q_OS_UNIX
  KWin::setState( winId(), NET::KeepAbove );
  KWin::setOnAllDesktops( winId(), true );
#endif
  eventNotification();
}

void AlarmDialog::suspendValueChanged()
{
  setButtonFocus( User1 );
}

void AlarmDialog::suspendUnitChanged()
{
  setButtonFocus( User1 );
  setButtonFocus( User2 );
}

void AlarmDialog::eventNotification()
{
  bool beeped = false;
  Alarm::List alarms = mIncidence->alarms();
  Alarm::List::ConstIterator it;
  for ( it = alarms.begin(); it != alarms.end(); ++it ) {
    Alarm *alarm = *it;
// FIXME: Check whether this should be done for all multiple alarms
    if (alarm->type() == Alarm::Procedure) {
// FIXME: Add a message box asking whether the procedure should really be executed
      kDebug(5890) << "Starting program: '" << alarm->programFile() << "'" << endl;
      KProcess proc;
      proc << QFile::encodeName(alarm->programFile());
      proc.start(KProcess::DontCare);
    }
    else if (alarm->type() == Alarm::Audio) {
      beeped = true;
      Phonon::AudioPlayer* player = new Phonon::AudioPlayer( Phonon::NotificationCategory, this );
      player->play( KUrl( QFile::encodeName(alarm->audioFile() ) ) );
    }
  }

  if ( !beeped ) {
    KNotification::beep();
  }
}

void AlarmDialog::wakeUp()
{
  if ( mRemindAt <= QDateTime::currentDateTime() )
    show();
  else
    setTimer( QDateTime::currentDateTime().secsTo( mRemindAt ) );
}

void AlarmDialog::slotSave()
{
  KConfig *config = KGlobal::config();
  KLockFile::Ptr lock = config->lockFile();
  if ( lock.data()->lock() != KLockFile::LockOK )
    return;

  config->setGroup( "General" );
  int numReminders = config->readEntry("Reminders", 0);
  config->writeEntry( "Reminders", ++numReminders );

  config->setGroup( QString("Incidence-%1").arg(numReminders) );
  config->writeEntry( "UID", mIncidence->uid() );
  config->writeEntry( "RemindAt", mRemindAt );
  config->sync();
  lock.data()->unlock();
}

