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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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
#include <dcopclient.h>
#include <klocale.h>
#include <kprocess.h>
#include <kaudioplayer.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <knotifyclient.h>
#include <kcombobox.h>
#include <kwin.h>

#include <libkcal/event.h>

#include "koeventviewer.h"

#include "alarmdialog.h"
#include "alarmdialog.moc"

AlarmDialog::AlarmDialog( QWidget *parent, const char *name )
  : KDialogBase( Plain, WType_TopLevel | WStyle_Customize | WStyle_StaysOnTop |
                 WStyle_DialogBorder,
                 parent, name, false, i18n("Alarm"), Ok | User1 | User2/* | User3*/, Ok/*3*/,
                 false, i18n("Suspend"), i18n("Edit...") )
{
  QWidget *topBox = plainPage();

  QBoxLayout *topLayout = new QVBoxLayout( topBox );
  topLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18n("The following events triggered alarms:"),
                              topBox );
  topLayout->addWidget( label );

  mIncidences.setAutoDelete( true );

  mEventViewer = new KOEventViewer( topBox );
  topLayout->addWidget( mEventViewer );

  QHBox *suspendBox = new QHBox( topBox );
  suspendBox->setSpacing( spacingHint() );
  topLayout->addWidget( suspendBox );

  new QLabel( i18n("Suspend duration:"), suspendBox );
  mSuspendSpin = new QSpinBox( 1, 9999, 1, suspendBox );
  mSuspendSpin->setValue( 5 );  // default suspend duration

  mSuspendUnit = new KComboBox( suspendBox );
  mSuspendUnit->insertItem( i18n("minute(s)") );
  mSuspendUnit->insertItem( i18n("hour(s)") );
  mSuspendUnit->insertItem( i18n("day(s)") );
  mSuspendUnit->insertItem( i18n("week(s)") );

  connect( mSuspendSpin, SIGNAL( valueChanged(int) ), actionButton(User1), SLOT( setFocus() ) );
  connect( mSuspendUnit, SIGNAL( activated(int) ), actionButton(User1), SLOT( setFocus() ) );
  connect( mSuspendUnit, SIGNAL( activated(int) ), actionButton(User2), SLOT( setFocus() ) );

  // showButton( User2/*3*/, false );

  setMinimumSize( 300, 200 );
}

AlarmDialog::~AlarmDialog()
{
}

void AlarmDialog::appendIncidence( Incidence *incidence )
{
  mEventViewer->appendIncidence( incidence );
  mIncidences.append( incidence->clone() );
}

void AlarmDialog::clearEvents()
{
  mEventViewer->clearEvents();

  mIncidences.clear();
}

void AlarmDialog::slotOk()
{
  clearEvents();
  accept();
}

void AlarmDialog::slotUser1()
{
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

  emit suspendSignal( unit * mSuspendSpin->value() );
  accept();
}

void AlarmDialog::slotUser2()
{
  if ( !kapp->dcopClient()->isApplicationRegistered( "korganizer" ) ) {
    if ( kapp->startServiceByDesktopName( "korganizer", QString::null ) )
      KMessageBox::error( 0, i18n("Could not start KOrganizer.") );
  }

  kapp->dcopClient()->send( "korganizer", "KOrganizerIface",
                            "editIncidence(QString)",
                             mIncidences.first()->uid() );

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

void AlarmDialog::eventNotification()
{
  bool beeped = false;

  Incidence *in;
  for (in = mIncidences.first(); in; in = mIncidences.next()) {
    Alarm::List alarms = in->alarms();
    Alarm::List::ConstIterator it;
    for ( it = alarms.begin(); it != alarms.end(); ++it ) {
      Alarm *alarm = *it;
// FIXME: Check whether this should be done for all multiple alarms
      if (alarm->type() == Alarm::Procedure) {
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

  if ( !beeped ) {
    KNotifyClient::beep();
  }
}
