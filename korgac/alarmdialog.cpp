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

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qfile.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qcstring.h>
#include <qdatastream.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <dcopclient.h>
#include <klocale.h>
#include <kprocess.h>
#include <kaudioplayer.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kcombobox.h>
#include <klistview.h>
#include <kwin.h>
#include <klockfile.h>

#include <libkcal/event.h>

#include "koeventviewer.h"

#include "alarmdialog.h"
#include "alarmdialog.moc"

class AlarmListItem : public KListViewItem
{
  public:
    AlarmListItem( Incidence *incidence, QListView *parent ) :
      KListViewItem( parent ),
      mIncidence( incidence->clone() ),
      mNotified( false )
    {}

    ~AlarmListItem()
    {
      delete mIncidence;
    }

    Incidence *mIncidence;
    QDateTime mRemindAt;
    bool mNotified;
};

typedef QValueList<AlarmListItem*> ItemList;

AlarmDialog::AlarmDialog( QWidget *parent, const char *name )
  : KDialogBase( Plain, WType_TopLevel | WStyle_Customize | WStyle_StaysOnTop |
                 WStyle_DialogBorder,
                 parent, name, false, i18n("Reminder"), Ok | User1 | User2 | User3, User1/*3*/,
                 false, i18n("Dismiss all"), i18n("Edit..."), i18n("Suspend") ),
                 mSuspendTimer(this)
{
  KGlobal::iconLoader()->addAppDir( "kdepim" );
  setButtonOK( i18n( "Dismiss" ) );

  QWidget *topBox = plainPage();
  QBoxLayout *topLayout = new QVBoxLayout( topBox );
  topLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18n("The following events triggered reminders:"),
                              topBox );
  topLayout->addWidget( label );

  mIncidenceListView = new KListView( topBox );
  mIncidenceListView->addColumn( i18n( "Summary" ) );
  mIncidenceListView->addColumn( i18n( "Due" ) );
  mIncidenceListView->setAllColumnsShowFocus( true );
  mIncidenceListView->setSelectionMode( QListView::Extended );
  topLayout->addWidget( mIncidenceListView );
  connect( mIncidenceListView, SIGNAL(selectionChanged()), SLOT(updateButtons()) );
  connect( mIncidenceListView, SIGNAL(doubleClicked(QListViewItem*)), SLOT(slotUser2()) );
  connect( mIncidenceListView, SIGNAL(currentChanged(QListViewItem*)), SLOT(showDetails()) );
  connect( mIncidenceListView, SIGNAL(selectionChanged()), SLOT(showDetails()) );

  mDetailView = new KOEventViewer( topBox );
  topLayout->addWidget( mDetailView );

  QHBox *suspendBox = new QHBox( topBox );
  suspendBox->setSpacing( spacingHint() );
  topLayout->addWidget( suspendBox );

  QLabel *l = new QLabel( i18n("Suspend &duration:"), suspendBox );
  mSuspendSpin = new QSpinBox( 1, 9999, 1, suspendBox );
  mSuspendSpin->setValue( 5 );  // default suspend duration
  l->setBuddy( mSuspendSpin );

  mSuspendUnit = new KComboBox( suspendBox );
  mSuspendUnit->insertItem( i18n("minute(s)") );
  mSuspendUnit->insertItem( i18n("hour(s)") );
  mSuspendUnit->insertItem( i18n("day(s)") );
  mSuspendUnit->insertItem( i18n("week(s)") );
  connect( &mSuspendTimer, SIGNAL(timeout()), SLOT(wakeUp()) );

  // showButton( User2/*3*/, false );

  setMinimumSize( 300, 200 );
}

AlarmDialog::~AlarmDialog()
{
  mIncidenceListView->clear();
}

void AlarmDialog::addIncidence( Incidence *incidence, const QDateTime &reminderAt )
{
  AlarmListItem *item = new AlarmListItem( incidence, mIncidenceListView );
  item->setText( 0, incidence->summary() );
  item->mRemindAt = reminderAt;
  Todo *todo;
  if ( dynamic_cast<Event*>( incidence ) ) {
    item->setPixmap( 0, SmallIcon( "appointment" ) );
    if ( incidence->doesRecur() ) {
      QDateTime nextStart = incidence->recurrence()->getNextDateTime( reminderAt );
      if ( nextStart.isValid() )
        item->setText( 1, KGlobal::locale()->formatDateTime( nextStart ) );
    }
    if ( item->text( 1 ).isEmpty() )
      item->setText( 1, incidence->dtStartStr() );
  } else if ( (todo = dynamic_cast<Todo*>( incidence )) ) {
    item->setPixmap( 0, SmallIcon( "todo" ) );
    item->setText( 1, todo->dtDueStr() );
  }
  if ( activeCount() == 1 ) {// previously empty
    mIncidenceListView->clearSelection();
    item->setSelected( true );
  }
  showDetails();
}

void AlarmDialog::slotOk()
{
  ItemList selection = selectedItems();
  for ( ItemList::Iterator it = selection.begin(); it != selection.end(); ++it ) {
    if ( (*it)->itemBelow() )
      (*it)->itemBelow()->setSelected( true );
    else if ( (*it)->itemAbove() )
      (*it)->itemAbove()->setSelected( true );
    delete *it;
  }
  if ( activeCount() == 0 )
    accept();
  else {
    updateButtons();
    showDetails();
  }
  emit reminderCount( activeCount() );
}

void AlarmDialog::slotUser1()
{
  dismissAll();
}

void AlarmDialog::suspend()
{
  if ( !isVisible() )
    return;

  int unit=1;
  switch (mSuspendUnit->currentItem()) {
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

  for ( QListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    AlarmListItem * item = static_cast<AlarmListItem*>( it.current() );
    if ( item->isSelected() && item->isVisible() ) {
      item->setVisible( false );
      item->setSelected( false );
      item->mRemindAt = QDateTime::currentDateTime().addSecs( unit * mSuspendSpin->value() );
      item->mNotified = false;
    }
  }

  setTimer();
  if ( activeCount() == 0 )
    accept();
  else {
    updateButtons();
    showDetails();
  }
  emit reminderCount( activeCount() );
}

void AlarmDialog::setTimer()
{
  int nextReminderAt = -1;
  for ( QListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    AlarmListItem * item = static_cast<AlarmListItem*>( it.current() );
    if ( item->mRemindAt > QDateTime::currentDateTime() ) {
      int secs = QDateTime::currentDateTime().secsTo( item->mRemindAt );
      nextReminderAt = nextReminderAt <= 0 ? secs : QMIN( nextReminderAt, secs );
    }
  }

  if ( nextReminderAt >= 0 ) {
    mSuspendTimer.stop();
    mSuspendTimer.start( 1000 * (nextReminderAt + 1), true );
  }
}

void AlarmDialog::slotUser2()
{
  ItemList selection = selectedItems();
  if ( selection.count() != 1 )
    return;
  Incidence *incidence = selection.first()->mIncidence;

  if ( !kapp->dcopClient()->isApplicationRegistered( "korganizer" ) ) {
    if ( kapp->startServiceByDesktopName( "korganizer", QString::null ) )
      KMessageBox::error( 0, i18n("Could not start KOrganizer.") );
  }

  kapp->dcopClient()->send( "korganizer", "KOrganizerIface",
                            "editIncidence(QString)",
                             incidence->uid() );

  // get desktop # where korganizer (or kontact) runs
  QByteArray replyData;
  QCString object, replyType;
  object = kapp->dcopClient()->isApplicationRegistered( "kontact" ) ?
           "kontact-mainwindow#1" : "KOrganizer MainWindow";
  if (!kapp->dcopClient()->call( "korganizer", object,
                            "getWinID()", 0, replyType, replyData, true, -1 ) ) {
  }

  if ( replyType == "int" ) {
    int desktop, window;
    QDataStream ds( replyData, IO_ReadOnly );
    ds >> window;
    desktop = KWin::windowInfo( window ).desktop();

    if ( KWin::currentDesktop() == desktop ) {
      KWin::iconifyWindow( winId(), false );
    }
    else
      KWin::setCurrentDesktop( desktop );

    KWin::activateWindow( KWin::transientFor( window ) );
  }
}

void AlarmDialog::slotUser3()
{
  suspend();
}

void AlarmDialog::dismissAll()
{
  for ( QListViewItemIterator it( mIncidenceListView ) ; it.current() ; ) {
    AlarmListItem *item = static_cast<AlarmListItem*>( it.current() );
    if ( !item->isVisible() ) {
      ++it;
      continue;
    }
    delete item;
  }
  setTimer();
  accept();
  emit reminderCount( activeCount() );
}

void AlarmDialog::show()
{
  mIncidenceListView->clearSelection();
  if ( mIncidenceListView->firstChild() )
    mIncidenceListView->firstChild()->setSelected( true );
  updateButtons();
  KDialogBase::show();
  KWin::setState( winId(), NET::KeepAbove );
  KWin::setOnAllDesktops( winId(), true );
  eventNotification();
}

void AlarmDialog::eventNotification()
{
  bool beeped = false, found = false;

  QValueList<AlarmListItem*> list;
  for ( QListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    AlarmListItem *item = static_cast<AlarmListItem*>( it.current() );
    if ( !item->isVisible() || item->mNotified )
      continue;
    found = true;
    item->mNotified = true;
    Alarm::List alarms = item->mIncidence->alarms();
    Alarm::List::ConstIterator it;
    for ( it = alarms.begin(); it != alarms.end(); ++it ) {
      Alarm *alarm = *it;
      // FIXME: Check whether this should be done for all multiple alarms
      if (alarm->type() == Alarm::Procedure) {
        // FIXME: Add a message box asking whether the procedure should really be executed
        kdDebug(5890) << "Starting program: '" << alarm->programFile() << "'" << endl;
        KProcess proc;
        proc << QFile::encodeName(alarm->programFile());
        proc.start(KProcess::DontCare);
      }
      else if (alarm->type() == Alarm::Audio) {
        beeped = true;
        KAudioPlayer::play(QFile::encodeName(alarm->audioFile()));
      }
    }
  }

  if ( !beeped && found ) {
    KNotifyClient::beep();
  }
}

void AlarmDialog::wakeUp()
{
  bool activeReminders = false;
  for ( QListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    AlarmListItem * item = static_cast<AlarmListItem*>( it.current() );
    if ( item->mRemindAt <= QDateTime::currentDateTime() ) {
      if ( !item->isVisible() ) {
        item->setVisible( true );
        item->setSelected( false );
      }
      activeReminders = true;
    } else {
      item->setVisible( false );
    }
  }

  if ( activeReminders )
    show();
  setTimer();
  showDetails();
  emit reminderCount( activeCount() );
}

void AlarmDialog::slotSave()
{
  KConfig *config = kapp->config();
  KLockFile::Ptr lock = config->lockFile();
  if ( lock.data()->lock() != KLockFile::LockOK )
    return;

  config->setGroup( "General" );
  int numReminders = config->readNumEntry("Reminders", 0);

  for ( QListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    AlarmListItem * item = static_cast<AlarmListItem*>( it.current() );
    config->setGroup( QString("Incidence-%1").arg(numReminders + 1) );
    config->writeEntry( "UID", item->mIncidence->uid() );
    config->writeEntry( "RemindAt", item->mRemindAt );
    ++numReminders;
  }

  config->setGroup( "General" );
  config->writeEntry( "Reminders", numReminders );
  config->sync();
  lock.data()->unlock();
}

void AlarmDialog::updateButtons()
{
  ItemList selection = selectedItems();
  enableButton( User2, selection.count() == 1 );
  enableButton( Ok, selection.count() > 0 );
  enableButton( User3, selection.count() > 0 );
}

QValueList< AlarmListItem * > AlarmDialog::selectedItems() const
{
  QValueList<AlarmListItem*> list;
  for ( QListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    if ( it.current()->isSelected() )
      list.append( static_cast<AlarmListItem*>( it.current() ) );
  }
  return list;
}

int AlarmDialog::activeCount()
{
  int count = 0;
  for ( QListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    AlarmListItem * item = static_cast<AlarmListItem*>( it.current() );
    if ( item->isVisible() )
      ++count;
  }
  return count;
}

void AlarmDialog::suspendAll()
{
  mIncidenceListView->clearSelection();
  for ( QListViewItemIterator it( mIncidenceListView ) ; it.current() ; ++it ) {
    if ( it.current()->isVisible() )
      it.current()->setSelected( true );
  }
  suspend();
}

void AlarmDialog::showDetails()
{
  mDetailView->clearEvents( true );
  mDetailView->clear();
  AlarmListItem *item = static_cast<AlarmListItem*>( mIncidenceListView->currentItem() );
  if ( !item || !item->isVisible() )
    return;
  mDetailView->appendIncidence( item->mIncidence );
}
