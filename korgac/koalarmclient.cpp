/*
    KOrganizer Alarm Daemon Client.

    This file is part of KOrganizer.

    Copyright (c) 2002,2003 Cornelius Schumacher

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

#include "koalarmclient.h"

#include "alarmdockwindow.h"
#include "alarmdialog.h"

#include <libkcal/calendarresources.h>

#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kapplication.h>
#include <kwin.h>

#include <qpushbutton.h>

KOAlarmClient::KOAlarmClient( QObject *parent, const char *name )
  : DCOPObject( "ac" ), QObject( parent, name )
{
  kdDebug(5890) << "KOAlarmClient::KOAlarmClient()" << endl;

  mDocker = new AlarmDockWindow;
  mDocker->show();
  connect( this, SIGNAL( reminderCount( int ) ), mDocker, SLOT( slotUpdate( int ) ) );
  connect( mDocker, SIGNAL( quitSignal() ), SLOT( slotQuit() ) );
  connect( mDocker, SIGNAL( suspendAllSignal() ), SLOT( slotSuspendAll() ) );
  connect( mDocker, SIGNAL( dismissAllSignal() ), SLOT( slotDismissAll() ) );

  KConfig c( locate( "config", "korganizerrc" ) );
  c.setGroup( "Time & Date" );
  QString tz = c.readEntry( "TimeZoneId" );
  kdDebug(5890) << "TimeZone: " << tz << endl;

  mCalendar = new CalendarResources( tz );
  mCalendar->readConfig();
  mCalendar->load();

  connect( &mCheckTimer, SIGNAL( timeout() ), SLOT( checkAlarms() ) );

  KConfig *cfg = KGlobal::config();
  cfg->setGroup( "Alarms" );
  int interval = cfg->readNumEntry( "Interval", 60 );
  kdDebug(5890) << "KOAlarmClient check interval: " << interval << " seconds."
                << endl;

  // load reminders that were active when quitting
  KConfig *config = kapp->config();
  config->setGroup( "General" );
  int numReminders = config->readNumEntry( "Reminders", 0 );
  for ( int i=1; i<=numReminders; ++i )
  {
    QString group( QString( "Incidence-%1" ).arg( i ) );
    config->setGroup( group );
    QString uid = config->readEntry( "UID" );
    QDateTime dt = config->readDateTimeEntry( "RemindAt" );
    if ( !uid.isEmpty() )
      createReminder( mCalendar->incidence( uid ), dt );
    config->deleteGroup( group );
  }
  config->setGroup( "General" );
  config->writeEntry( "Reminders", 0 );
  config->sync();

  mCheckTimer.start( 1000 * interval );  // interval in seconds
}

KOAlarmClient::~KOAlarmClient()
{
  delete mCalendar;
  delete mDocker;
}

void KOAlarmClient::checkAlarms()
{
  KConfig *cfg = KGlobal::config();

  cfg->setGroup( "General" );
  if ( !cfg->readBoolEntry( "Enabled", true ) ) return;

  cfg->setGroup( "Alarms" );
  QDateTime lastChecked = cfg->readDateTimeEntry( "CalendarsLastChecked" );
  QDateTime from = lastChecked.addSecs( 1 );
  QDateTime to = QDateTime::currentDateTime();

  kdDebug(5891) << "Check: " << from.toString() << " - " << to.toString() << endl;

  QValueList<Alarm *> alarms = mCalendar->alarms( from, to );

  QValueList<Alarm *>::ConstIterator it;
  for( it = alarms.begin(); it != alarms.end(); ++it ) {
    kdDebug(5891) << "ALARM: " << (*it)->parent()->summary() << endl;
    Incidence *incidence = mCalendar->incidence( (*it)->parent()->uid() );
    createReminder( incidence, QDateTime::currentDateTime() );
  }

  cfg->writeEntry( "CalendarsLastChecked", to );

  cfg->sync();
}

void KOAlarmClient::createReminder( KCal::Incidence *incidence, QDateTime dt )
{
  AlarmDialog *dialog = new AlarmDialog();
  dialog->setIncidence( incidence );
  dialog->setRemindAt( dt );
  mReminders.append( dialog );
  connect( dialog, SIGNAL( finishedSignal( AlarmDialog *) ), SLOT( slotRemove( AlarmDialog *) ) );
  connect( mDocker, SIGNAL( suspendAllSignal() ), dialog, SLOT( slotUser1() ) );
  connect( mDocker, SIGNAL( dismissAllSignal() ), dialog, SLOT( slotOk() ) );
  connect( this, SIGNAL( saveAllSignal() ), dialog, SLOT( slotSave() ) );
  emit reminderCount( mReminders.count() );
  dialog->wakeUp();
}

void KOAlarmClient::slotQuit()
{
  emit saveAllSignal();
  quit();
}

void KOAlarmClient::quit()
{
  kdDebug(5890) << "KOAlarmClient::quit()" << endl;
  kapp->quit();
}

void KOAlarmClient::slotRemove( AlarmDialog *d )
{
  mReminders.remove( d );
  delete d;
  emit reminderCount( mReminders.count() );
}

bool KOAlarmClient::commitData( QSessionManager& )
{
  emit saveAllSignal();
  return true;
}

void KOAlarmClient::forceAlarmCheck()
{
  checkAlarms();
}

void KOAlarmClient::dumpDebug()
{
  KConfig *cfg = KGlobal::config();

  cfg->setGroup( "Alarms" );
  QDateTime lastChecked = cfg->readDateTimeEntry( "CalendarsLastChecked" );

  kdDebug(5890) << "Last Check: " << lastChecked << endl;
}

QStringList KOAlarmClient::dumpAlarms()
{
  QDateTime start = QDateTime( QDateTime::currentDateTime().date(),
                               QTime( 0, 0 ) );
  QDateTime end = start.addDays( 1 ).addSecs( -1 );

  QStringList lst;
  // Don't translate, this is for debugging purposes.
  lst << QString("AlarmDeamon::dumpAlarms() from ") + start.toString()+ " to " +
         end.toString();

  QValueList<Alarm*> alarms = mCalendar->alarms( start, end );
  QValueList<Alarm*>::ConstIterator it;
  for( it = alarms.begin(); it != alarms.end(); ++it ) {
    Alarm *a = *it;
    lst << QString("  ") + a->parent()->summary() + " ("
              + a->time().toString() + ")";
  }

  return lst;
}

void KOAlarmClient::debugShowDialog()
{
//   showAlarmDialog();
}

#include "koalarmclient.moc"
