/*
  KOrganizer Alarm Daemon Client.

  This file is part of KOrganizer.

  Copyright (c) 2002,2003 Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
//krazy:excludeall=kdebug because we use the korgac(check) debug area in here

#include "koalarmclient.h"
#include "alarmdialog.h"
#include "alarmdockwindow.h"
#include "korgacadaptor.h"

#include <KCal/CalendarResources>
using namespace KCal;

#include <KApplication>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KStandardDirs>
#include <KSystemTimeZones>

KOAlarmClient::KOAlarmClient( QObject *parent )
  : QObject( parent ), mDocker( 0 ), mDialog( 0 )
{
  new KOrgacAdaptor( this );
  QDBusConnection::sessionBus().registerObject( "/ac", this );
  kDebug();

  KConfig korgConfig( KStandardDirs::locate( "config", "korganizerrc" ) );
  KConfigGroup generalGroup( &korgConfig, "General" );
  bool showDock = generalGroup.readEntry( "ShowReminderDaemon", true );

  if ( showDock ) {
    mDocker = new AlarmDockWindow;

    connect( this, SIGNAL(reminderCount(int)), mDocker, SLOT(slotUpdate(int)) );
    connect( mDocker, SIGNAL(quitSignal()), SLOT(slotQuit()) );
  }

  const KTimeZone zone = KSystemTimeZones::local();
  mCalendar = new CalendarResources( zone.isValid() ?
                                     KDateTime::Spec( zone ) :
                                     KDateTime::ClockTime );
  mCalendar->readConfig();
  mCalendar->load();

  connect( &mCheckTimer, SIGNAL(timeout()), SLOT(checkAlarms()) );

  KConfigGroup alarmGroup( KGlobal::config(), "Alarms" );
  int interval = alarmGroup.readEntry( "Interval", 60 );
  kDebug() << "KOAlarmClient check interval:" << interval << "seconds.";
  mLastChecked = alarmGroup.readEntry( "CalendarsLastChecked", QDateTime() );

  // load reminders that were active when quitting
  KConfigGroup genGroup( KGlobal::config(), "General" );
  int numReminders = genGroup.readEntry( "Reminders", 0 );
  for ( int i=1; i<=numReminders; ++i ) {
    QString group( QString( "Incidence-%1" ).arg( i ) );
    KConfigGroup *incGroup = new KConfigGroup( KGlobal::config(), group );
    QString uid = incGroup->readEntry( "UID" );
    QDateTime dt = incGroup->readEntry( "RemindAt", QDateTime() );
    if ( !uid.isEmpty() ) {
      createReminder( mCalendar->incidence( uid ), dt, QString() );
    }
    delete incGroup;
  }
  if ( numReminders ) {
     genGroup.writeEntry( "Reminders", 0 );
     genGroup.sync();
  }

  checkAlarms();
  mCheckTimer.start( 1000 * interval );  // interval in seconds
}

KOAlarmClient::~KOAlarmClient()
{
  delete mCalendar;
  delete mDocker;
  delete mDialog;
}

void KOAlarmClient::checkAlarms()
{
  KConfigGroup cfg( KGlobal::config(), "General" );

  if ( !cfg.readEntry( "Enabled", true ) ) {
    return;
  }

  QDateTime from = mLastChecked.addSecs( 1 );
  mLastChecked = QDateTime::currentDateTime();

  kDebug(5891) << "Check:" << from.toString() << " -" << mLastChecked.toString();

  QList<Alarm *>alarms = mCalendar->alarms( KDateTime( from, KDateTime::LocalZone ),
                                            KDateTime( mLastChecked, KDateTime::LocalZone ) );

  QList<Alarm *>::ConstIterator it;
  for ( it = alarms.constBegin(); it != alarms.constEnd(); ++it ) {
    kDebug(5891) << "REMINDER:" << (*it)->parent()->summary();
    Incidence *incidence = mCalendar->incidence( (*it)->parent()->uid() );
    createReminder( incidence, QDateTime::currentDateTime(), (*it)->text() );
  }
}

void KOAlarmClient::createReminder( KCal::Incidence *incidence,
                                    const QDateTime &dt,
                                    const QString &displayText )
{
  if ( !incidence ) {
    return;
  }

  if ( !mDialog ) {
    mDialog = new AlarmDialog();
    connect( this, SIGNAL(saveAllSignal()), mDialog, SLOT(slotSave()) );
    if ( mDocker ) {
      connect( mDialog, SIGNAL(reminderCount(int)),
               mDocker, SLOT(slotUpdate(int)) );
      connect( mDocker, SIGNAL(suspendAllSignal()),
               mDialog, SLOT(suspendAll()) );
      connect( mDocker, SIGNAL(dismissAllSignal()),
               mDialog, SLOT(dismissAll()) );
    }
  }

  mDialog->addIncidence( incidence, dt, displayText );
  mDialog->wakeUp();
  saveLastCheckTime();
}

void KOAlarmClient::slotQuit()
{
  emit saveAllSignal();
  saveLastCheckTime();
  quit();
}

void KOAlarmClient::saveLastCheckTime()
{
  KConfigGroup cg( KGlobal::config(), "Alarms" );
  cg.writeEntry( "CalendarsLastChecked", mLastChecked );
  KGlobal::config()->sync();
}

void KOAlarmClient::quit()
{
  kDebug();
  kapp->quit();
}

bool KOAlarmClient::commitData( QSessionManager & )
{
  emit saveAllSignal();
  saveLastCheckTime();
  return true;
}

void KOAlarmClient::forceAlarmCheck()
{
  checkAlarms();
  saveLastCheckTime();
}

void KOAlarmClient::dumpDebug()
{
  KConfigGroup cfg( KGlobal::config(), "Alarms" );

  QDateTime lastChecked = cfg.readEntry( "CalendarsLastChecked", QDateTime() );

  kDebug() << "Last Check:" << lastChecked;
}

QStringList KOAlarmClient::dumpAlarms()
{
  KDateTime start = KDateTime( QDateTime::currentDateTime().date(),
                               QTime( 0, 0 ), KDateTime::LocalZone );
  KDateTime end = start.addDays( 1 ).addSecs( -1 );

  QStringList lst;
  // Don't translate, this is for debugging purposes.
  lst << QString( "AlarmDeamon::dumpAlarms() from " ) + start.toString() + " to " +
         end.toString();

  QList<Alarm *> alarms = mCalendar->alarms( start, end );
  QList<Alarm *>::ConstIterator it;
  for ( it = alarms.constBegin(); it != alarms.constEnd(); ++it ) {
    Alarm *a = *it;
    lst << QString( "  " ) + a->parent()->summary() + " (" + a->time().toString() + ')';
  }

  return lst;
}

void KOAlarmClient::debugShowDialog()
{
//   showAlarmDialog();
}

void KOAlarmClient::hide()
{
  delete mDocker;
  mDocker = 0;
}

void KOAlarmClient::show()
{
  if ( !mDocker ) {
    mDocker = new AlarmDockWindow;

    connect( this, SIGNAL(reminderCount(int)), mDocker, SLOT(slotUpdate(int)) );
    connect( mDocker, SIGNAL(quitSignal()), SLOT(slotQuit()) );

    if ( mDialog ) {
      connect( mDialog, SIGNAL(reminderCount(int)),
               mDocker, SLOT(slotUpdate(int)) );
      connect( mDocker, SIGNAL(suspendAllSignal()),
               mDialog, SLOT(suspendAll()) );
      connect( mDocker, SIGNAL(dismissAllSignal()),
               mDialog, SLOT(dismissAll()) );
    }
  }
}

#include "koalarmclient.moc"
