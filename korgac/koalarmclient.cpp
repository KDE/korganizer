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

KOAlarmClient::KOAlarmClient( QObject *parent, const char *name )
  : DCOPObject( "ac" ), QObject( parent, name ),
    mSuspendTimer( this )
{
  kdDebug(5890) << "KOAlarmClient::KOAlarmClient()" << endl;

  mDocker = new AlarmDockWindow;
  mDocker->show();

  mAlarmDialog = new AlarmDialog;
  connect( mAlarmDialog, SIGNAL( suspendSignal( int ) ),
           SLOT( suspend( int ) ) );

  mCalendar = new CalendarResources();

  KConfig c( locate( "config", "korganizerrc" ) );
  c.setGroup( "Time & Date" );
  QString tz = c.readEntry( "TimeZoneId" );
  kdDebug(5890) << "TimeZone: " << tz << endl;
  mCalendar->setTimeZoneId( tz );

  connect( &mCheckTimer, SIGNAL( timeout() ), SLOT( checkAlarms() ) );

  KConfig *cfg = KGlobal::config();
  cfg->setGroup( "Alarms" );
  int interval = cfg->readNumEntry( "Interval", 60 );
  kdDebug(5890) << "KOAlarmClient check interval: " << interval << " seconds."
                << endl;

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
  
  bool newEvents = false;
  QValueList<Alarm *>::ConstIterator it;
  for( it = alarms.begin(); it != alarms.end(); ++it ) {
    kdDebug(5891) << "ALARM: " << (*it)->parent()->summary() << endl;
    Event *event = mCalendar->event( (*it)->parent()->uid() );
    if ( event ) {
      mAlarmDialog->appendEvent( event );
      newEvents = true;
    }
  }
  if ( newEvents ) {
    mAlarmDialog->show();
    mAlarmDialog->eventNotification();
  }

  cfg->writeEntry( "CalendarsLastChecked", to );

  cfg->sync();
}

void KOAlarmClient::suspend( int minutes )
{
//  kdDebug(5890) << "KOAlarmClient::suspend() " << minutes << " minutes" << endl;
  connect( &mSuspendTimer, SIGNAL( timeout() ), SLOT( showAlarmDialog() ) );
  mSuspendTimer.start( 1000 * 60 * minutes, true );
}

void KOAlarmClient::showAlarmDialog()
{
  mAlarmDialog->show();
  mAlarmDialog->eventNotification();
}

void KOAlarmClient::quit()
{
  kdDebug(5890) << "KOAlarmClient::quit()" << endl;
  kapp->quit();
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

#include "koalarmclient.moc"
