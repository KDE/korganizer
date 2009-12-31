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
#include "akonadicalendar.h"

#include <Akonadi/Item>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/Session>
#include <Akonadi/ItemFetchScope>

#include <akonadi/kcal/calendarmodel.h>
#include <kcal/mimetypevisitor.h>
#include <akonadi/kcal/utils.h>

#include <KCal/CalendarResources>

#include <KApplication>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KStandardDirs>
#include <KSystemTimeZones>
#include "kdescendantsproxymodel_p.h" // fix when forwarding header is there

using namespace Akonadi;
using namespace KCal;

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
  Session *session = new Session( "KOAlarmClient", this );
  ChangeRecorder *monitor = new ChangeRecorder( this );
  monitor->setSession( session );
  monitor->itemFetchScope().fetchFullPayload( true );
  monitor->setCollectionMonitored( Collection::root() );
  monitor->fetchCollection( true );
  monitor->setMimeTypeMonitored( "text/calendar", true ); // FIXME: this one should not be needed, in fact it might cause the inclusion of free/busy, notes or other unwanted stuff
  monitor->setMimeTypeMonitored( KCal::MimeTypeVisitor::eventMimeType(), true );
  monitor->setMimeTypeMonitored( KCal::MimeTypeVisitor::todoMimeType(), true );
  monitor->setMimeTypeMonitored( KCal::MimeTypeVisitor::journalMimeType(), true );
  CalendarModel *calendarModel = new CalendarModel( monitor, this );
  //mCalendarModel->setItemPopulationStrategy( EntityTreeModel::LazyPopulation );

  KDescendantsProxyModel *flattener = new KDescendantsProxyModel(this);
  flattener->setSourceModel( calendarModel );

  mCalendar = new KOrg::AkonadiCalendar( calendarModel, flattener, zone.isValid() ? KDateTime::Spec( zone ) : KDateTime::ClockTime );

  connect( &mCheckTimer, SIGNAL(timeout()), SLOT(checkAlarms()) );

  KConfigGroup alarmGroup( KGlobal::config(), "Alarms" );
  int interval = alarmGroup.readEntry( "Interval", 60 );
  kDebug() << "KOAlarmClient check interval:" << interval << "seconds.";
  mLastChecked = alarmGroup.readEntry( "CalendarsLastChecked", QDateTime() );

  // load reminders that were active when quitting
  KConfigGroup genGroup( KGlobal::config(), "General" );
  int numReminders = genGroup.readEntry( "Reminders", 0 );
  for ( int i=1; i<=numReminders; ++i ) {
    const QString group( QString( "Incidence-%1" ).arg( i ) );
    const KConfigGroup incGroup( KGlobal::config(), group );

    const KUrl url = incGroup.readEntry( "AkonadiUrl" );
    Akonadi::Item::Id akonadiItemId = -1;
    if( ! url.isValid() ) {
      // logic to migrate old KOrganizer incidence uid's to a Akonadi item.
      const QString uid = incGroup.readEntry( "UID" );
      if( ! uid.isEmpty() ) {
        akonadiItemId = mCalendar->itemIdForIncidenceUid(uid);
      }
    } else {
      akonadiItemId = Item::fromUrl( url ).id();
    }

    if ( akonadiItemId >= 0 ) {
      const QDateTime dt = incGroup.readEntry( "RemindAt", QDateTime() );
      createReminder( mCalendar, mCalendar->incidence( Item::fromUrl( url ).id() ), dt, QString() );
    }
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

  Alarm::List alarms = mCalendar->alarms( KDateTime( from, KDateTime::LocalZone ),
                                            KDateTime( mLastChecked, KDateTime::LocalZone ) );

  foreach(Alarm* a, alarms) {
    const QString uid = a->parent()->uid();
    const Akonadi::Item::Id itemId = mCalendar->itemIdForIncidenceUid( uid );
    const Akonadi::Item incidence = mCalendar->incidence( itemId );
    createReminder( mCalendar, incidence, from, a->text() );
  }
}

void KOAlarmClient::createReminder( KOrg::AkonadiCalendar *calendar,
                                    const Item &aitem,
                                    const QDateTime &dt,
                                    const QString &displayText )
{
  if ( !Akonadi::hasIncidence( aitem ) ) {
    return;
  }

  if ( !mDialog ) {
    mDialog = new AlarmDialog( calendar );
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

  mDialog->addIncidence( aitem, dt, displayText );
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

  Alarm::List alarms = mCalendar->alarms( start, end );
  foreach(Alarm* a, alarms) {
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
