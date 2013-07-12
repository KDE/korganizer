/*
  This file is part of the KDE reminder agent.

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
#if !defined(KORGAC_AKONADI_AGENT)
#include "alarmdialog.h"
#include "alarmdockwindow.h"
#else
#include <KNotification>
#endif
#include "korgacadaptor.h"

#include <calendarsupport/utils.h>

#include <Akonadi/Item>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/Session>
#include <Akonadi/Collection>
#include <Akonadi/ItemFetchScope>
#include <akonadi/dbusconnectionpool.h> // fix when forwarding header is there

#include <KCalCore/Calendar>

#include <KApplication>
#include <KCheckableProxyModel>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KStandardDirs>
#include <KSystemTimeZones>

#ifdef Q_WS_MAEMO_5
#include <QtMaemo5/QMaemo5InformationBox>
#endif

using namespace KCalCore;

KOAlarmClient::KOAlarmClient( QObject *parent )
  : QObject( parent ), mDocker( 0 ), mDialog( 0 )
{
  new KOrgacAdaptor( this );
  Akonadi::DBusConnectionPool::threadConnection().registerObject( "/ac", this );
  kDebug();

#if !defined(KORGAC_AKONADI_AGENT)
  if ( dockerEnabled() ) {
    mDocker = new AlarmDockWindow;
    connect( this, SIGNAL(reminderCount(int)), mDocker, SLOT(slotUpdate(int)) );
    connect( mDocker, SIGNAL(quitSignal()), SLOT(slotQuit()) );
  }
#endif
  QStringList mimeTypes;
  mimeTypes << Event::eventMimeType() << Todo::todoMimeType();
  mCalendar = Akonadi::ETMCalendar::Ptr( new Akonadi::ETMCalendar( mimeTypes ) );
  mCalendar->setObjectName( "KOrgac's calendar" );
  mETM = mCalendar->entityTreeModel();

  connect( &mCheckTimer, SIGNAL(timeout()), SLOT(checkAlarms()) );
  connect( mETM, SIGNAL(collectionPopulated(Akonadi::Collection::Id)),
           SLOT(deferredInit()) );
  connect( mETM, SIGNAL(collectionTreeFetched(Akonadi::Collection::List)),
           SLOT(deferredInit()) );

  KConfigGroup alarmGroup( KGlobal::config(), "Alarms" );
  const int interval = alarmGroup.readEntry( "Interval", 60 );
  kDebug() << "KOAlarmClient check interval:" << interval << "seconds.";
  mLastChecked = alarmGroup.readEntry( "CalendarsLastChecked", QDateTime() );

  checkAlarms();
  mCheckTimer.start( 1000 * interval );  // interval in seconds
}

KOAlarmClient::~KOAlarmClient()
{
#if !defined(KORGAC_AKONADI_AGENT)
  delete mDocker;
  delete mDialog;
#endif
}

void checkAllItems( KCheckableProxyModel *model, const QModelIndex &parent = QModelIndex() )
{
  const int rowCount = model->rowCount( parent );
  for ( int row = 0; row < rowCount; row++ ) {
    QModelIndex index = model->index( row, 0, parent );
    model->setData( index, Qt::Checked, Qt::CheckStateRole );

    if ( model->rowCount( index ) > 0 ) {
      checkAllItems( model, index );
    }
  }
}

void KOAlarmClient::deferredInit()
{
  if ( !collectionsAvailable() ) {
    return;
  }

  kDebug(5891) << "Performing delayed initialization.";

  // load reminders that were active when quitting
  KConfigGroup genGroup( KGlobal::config(), "General" );
  const int numReminders = genGroup.readEntry( "Reminders", 0 );

  for ( int i=1; i<=numReminders; ++i ) {
    const QString group( QString( "Incidence-%1" ).arg( i ) );
    const KConfigGroup incGroup( KGlobal::config(), group );

    const KUrl url = incGroup.readEntry( "AkonadiUrl" );
    Akonadi::Item::Id akonadiItemId = -1;
    if ( !url.isValid() ) {
      // logic to migrate old KOrganizer incidence uid's to a Akonadi item.
      const QString uid = incGroup.readEntry( "UID" );
      if ( !uid.isEmpty() ) {
        akonadiItemId = mCalendar->item( uid ).id();
      }
    } else {
      akonadiItemId = Akonadi::Item::fromUrl( url ).id();
    }

    if ( akonadiItemId >= 0 ) {
      const QDateTime dt = incGroup.readEntry( "RemindAt", QDateTime() );
      Akonadi::Item i = mCalendar->item( Akonadi::Item::fromUrl( url ).id() );
      if ( CalendarSupport::hasIncidence( i ) && !CalendarSupport::incidence( i )->alarms().isEmpty() ) {
        createReminder( mCalendar, i, dt, QString() );
      }
    }
  }

  KCheckableProxyModel *checkableModel = mCalendar->checkableProxyModel();
  checkAllItems( checkableModel );

  // Now that everything is set up, a first check for reminders can be performed.
  checkAlarms();
}

bool KOAlarmClient::dockerEnabled()
{
  KConfig korgConfig( KStandardDirs::locate( "config", "korganizerrc" ) );
  KConfigGroup generalGroup( &korgConfig, "System Tray" );
  return generalGroup.readEntry( "ShowReminderDaemon", true );
}

bool KOAlarmClient::collectionsAvailable() const
{
  // The list of collections must be available.
  if ( !mETM->isCollectionTreeFetched() ) {
    return false;
  }

  // All collections must be populated.
  const int rowCount = mETM->rowCount();
  for ( int row = 0; row < rowCount; ++row ) {
    static const int column = 0;
    const QModelIndex index = mETM->index( row, column );
    bool haveData =
      mETM->data( index, Akonadi::EntityTreeModel::IsPopulatedRole ).toBool();
    if ( !haveData ) {
      return false;
    }
  }

  return true;
}

void KOAlarmClient::checkAlarms()
{
  KConfigGroup cfg( KGlobal::config(), "General" );

  if ( !cfg.readEntry( "Enabled", true ) ) {
    return;
  }

  // We do not want to miss any reminders, so don't perform check unless
  // the collections are available and populated.
  if ( !collectionsAvailable() ) {
    kDebug(5891) << "Collections are not available; aborting check.";
    return;
  }

  QDateTime from = mLastChecked.addSecs( 1 );
  mLastChecked = QDateTime::currentDateTime();

  kDebug(5891) << "Check:" << from.toString() << " -" << mLastChecked.toString();

  const Alarm::List alarms = mCalendar->alarms( KDateTime( from, KDateTime::LocalZone ),
                                                KDateTime( mLastChecked, KDateTime::LocalZone ),
                                                true /* exclude blocked alarms */);

  foreach ( const Alarm::Ptr &alarm, alarms ) {
    const QString uid = alarm->customProperty( "ETMCalendar", "parentUid" );
    const Akonadi::Item::Id id = mCalendar->item( uid ).id();
    const Akonadi::Item item = mCalendar->item( id );

    createReminder( mCalendar, item, from, alarm->text() );
  }
}

void KOAlarmClient::createReminder( const Akonadi::ETMCalendar::Ptr &calendar,
                                    const Akonadi::Item &aitem,
                                    const QDateTime &remindAtDate,
                                    const QString &displayText )
{
  if ( !CalendarSupport::hasIncidence( aitem ) ) {
    return;
  }

#if !defined(Q_WS_MAEMO_5) && !defined(Q_WS_WINCE) && !defined(KORGAC_AKONADI_AGENT)
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

  mDialog->addIncidence( aitem, remindAtDate, displayText );
  mDialog->wakeUp();
#else
  const Incidence::Ptr incidence = CalendarSupport::incidence( aitem );
  Q_UNUSED( calendar );
  Q_UNUSED( remindAtDate );
  Q_UNUSED( displayText );

#if defined(Q_WS_MAEMO_5)
  QMaemo5InformationBox::information( 0, incidence->summary(), QMaemo5InformationBox::NoTimeout );
#else
  KNotification *notify = new KNotification( "reminder", 0, KNotification::Persistent );
  notify->setText( incidence->summary() );
  notify->sendEvent();
#endif

#endif
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
#if !defined(KORGAC_AKONADI_AGENT)
  kapp->quit();
#endif
}

#if !defined(Q_WS_WINCE)
bool KOAlarmClient::commitData( QSessionManager & )
{
  emit saveAllSignal();
  saveLastCheckTime();
  return true;
}
#endif

void KOAlarmClient::forceAlarmCheck()
{
  checkAlarms();
  saveLastCheckTime();
}

void KOAlarmClient::dumpDebug()
{
  KConfigGroup cfg( KGlobal::config(), "Alarms" );
  const QDateTime lastChecked = cfg.readEntry( "CalendarsLastChecked", QDateTime() );
  kDebug() << "Last Check:" << lastChecked;
}

QStringList KOAlarmClient::dumpAlarms()
{
  const KDateTime start = KDateTime( QDateTime::currentDateTime().date(),
                                     QTime( 0, 0 ), KDateTime::LocalZone );
  const KDateTime end = start.addDays( 1 ).addSecs( -1 );

  QStringList lst;
  // Don't translate, this is for debugging purposes.
  lst << QString( "AlarmDeamon::dumpAlarms() from " ) + start.toString() + " to " +
         end.toString();

  Alarm::List alarms = mCalendar->alarms( start, end );
  foreach( Alarm::Ptr a, alarms ) {
    const Incidence::Ptr parentIncidence = mCalendar->incidence( a->parentUid() );
    lst << QString( "  " ) + parentIncidence->summary() + " (" + a->time().toString() + ')';
  }

  return lst;
}

void KOAlarmClient::debugShowDialog()
{
//   showAlarmDialog();
}

void KOAlarmClient::hide()
{
#if !defined(KORGAC_AKONADI_AGENT)
  delete mDocker;
  mDocker = 0;
#endif
}

void KOAlarmClient::show()
{
#if !defined(KORGAC_AKONADI_AGENT)
  if ( !mDocker ) {
    if ( dockerEnabled() ) {
      mDocker = new AlarmDockWindow;
      connect( this, SIGNAL(reminderCount(int)), mDocker, SLOT(slotUpdate(int)) );
      connect( mDocker, SIGNAL(quitSignal()), SLOT(slotQuit()) );
    }

    if ( mDialog ) {
      connect( mDialog, SIGNAL(reminderCount(int)),
               mDocker, SLOT(slotUpdate(int)) );
      connect( mDocker, SIGNAL(suspendAllSignal()),
               mDialog, SLOT(suspendAll()) );
      connect( mDocker, SIGNAL(dismissAllSignal()),
               mDialog, SLOT(dismissAll()) );
    }
  }
#endif
}

#include "koalarmclient.moc"
