#include "akonadicalendar.h"
#include "akonadicalendar_p.h"

#include <kcal/incidence.h>
#include <kcal/event.h>
#include <kcal/todo.h>
#include <kcal/journal.h>
#include <kcal/filestorage.h>

#include <QtCore/QDate>
#include <QtCore/QHash>
#include <QtCore/QMultiHash>
#include <QtCore/QString>

#include <kdebug.h>
#include <kdatetime.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <akonadi/collection.h>
#include <akonadi/collectionview.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/monitor.h>
#include <akonadi/session.h>

using namespace KCal;

#if 0
namespace {
template <typename T> void removeIncidenceFromMultiHashByUID( QMultiHash< QString, T >& container, const QString &key, const QString &uid )
{
  const QList<T> values = container.values( key );
  QListIterator<T> it(values);
  while ( it.hasNext() ) {
    T const inc = it.next();
    if ( inc->uid() == uid ) container.remove( key, inc );
  }
}
}
#endif

AkonadiCalendar::AkonadiCalendar( const KDateTime::Spec &timeSpec )
  : KCal::Calendar( timeSpec )
  , d( new AkonadiCalendar::Private(this) )
{
}

AkonadiCalendar::~AkonadiCalendar()
{
  close();
  delete d;
}

bool AkonadiCalendar::hasCollection( const Akonadi::Collection &collection ) const
{
  return d->m_collectionMap.contains( collection.id() );
}

void AkonadiCalendar::addCollection( const Akonadi::Collection &collection )
{
  kDebug();
  Q_ASSERT( ! d->m_collectionMap.contains( collection.id() ) );
  AkonadiCalendarCollection *c = new AkonadiCalendarCollection( this, collection );
  d->m_collectionMap[ collection.id() ] = c; //TODO remove again if failed!

  d->m_monitor->setCollectionMonitored( collection, true );

  // start a new job and fetch all items
  Akonadi::ItemFetchJob* job = new Akonadi::ItemFetchJob( collection, d->m_session );
  job->setFetchScope( d->m_monitor->itemFetchScope() );
  connect( job, SIGNAL(result(KJob*)), d, SLOT(listingDone(KJob*)) );
}

void AkonadiCalendar::removeCollection( const Akonadi::Collection &collection )
{
  kDebug();
  if ( !d->m_collectionMap.contains( collection.id() ) )
    return;
  Q_ASSERT( d->m_collectionMap.contains( collection.id() ) );
  d->m_monitor->setCollectionMonitored( collection, false );
  AkonadiCalendarCollection *c = d->m_collectionMap.take( collection.id() );
  delete c;

  //d->clear();
  //d->m_session->clear();

  QHash<QString, AkonadiCalendarItem*>::Iterator it( d->m_itemMap.begin() ), end( d->m_itemMap.end() );
  while( it != end) {
    if( it.value()->m_item.storageCollectionId() == collection.id() ) {
      AkonadiCalendarItem* i = *it;
      it = d->m_itemMap.erase(it);
      delete i;
    } else {
      ++it;
    }
  }

  emit calendarChanged();
}

Akonadi::Item AkonadiCalendar::itemForIncidence(Incidence *incidence) const
{
  const QString uid = incidence->uid();
  kDebug()<<uid;  
  if( ! d->m_itemMap.contains( uid ) )
    return Akonadi::Item();
  AkonadiCalendarItem *aci = d->m_itemMap[ uid ];
  return aci->m_item;
}

bool AkonadiCalendar::reload()
{
  kDebug();
#if 0
  const QString filename = d->mFileName;
  save();
  close();
  d->mFileName = filename;
  FileStorage storage( this, d->mFileName );
  return storage.load();
#else
  return true;
#endif
}

bool AkonadiCalendar::save()
{
  kDebug();
#if 0
  if ( d->mFileName.isEmpty() ) {
    return false;
  }
  if ( isModified() ) {
    FileStorage storage( this, d->mFileName, d->mFormat );
    return storage.save();
  } else {
    return true;
  }
#else
  return true;
#endif
}

void AkonadiCalendar::close()
{
  kDebug();
#if 0
  setObserversEnabled( false );
  d->mFileName.clear();
  deleteAllEvents();
  deleteAllTodos();
  deleteAllJournals();
  d->mDeletedIncidences.clearAll();
#endif
  setModified( false );
#if 0
  setObserversEnabled( true );
#endif
}

bool AkonadiCalendar::addAgent( const KUrl &mUrl )
{
  kDebug()<<mUrl;
  Akonadi::AgentType type = Akonadi::AgentManager::self()->type( "akonadi_ical_resource" );
  Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob( type );
  job->setProperty("path", mUrl.path());
  connect( job, SIGNAL( result( KJob * ) ), d, SLOT( agentCreated( KJob * ) ) );
  job->start();
  return true;
}

bool AkonadiCalendar::addIncidence( Incidence *incidence )
{
  kDebug();
  // dispatch to addEvent/addTodo/addJournal
  return Calendar::addIncidence( incidence );
}

bool AkonadiCalendar::deleteIncidence( Incidence *incidence )
{
  kDebug();
  // dispatch to deleteEvent/deleteTodo/deleteJournal
  return Calendar::deleteIncidence( incidence );
}

void AkonadiCalendar::incidenceUpdated( IncidenceBase *incidence )
{
  kDebug();

  KDateTime nowUTC = KDateTime::currentUtcDateTime();
  incidence->setLastModified( nowUTC );
  // we should probably update the revision number here,
  // or internally in the Event itself when certain things change.
  // need to verify with ical documentation.
#if 0
  if ( incidence->type() == "Event" ) {
    Event *event = static_cast<Event*>( incidence );
    removeIncidenceFromMultiHashByUID<Event *>(
      d->mEventsForDate, event->dtStart().date().toString(), event->uid() );
    if ( !event->recurs() && !event->isMultiDay() ) {
      d->mEventsForDate.insert( event->dtStart().date().toString(), event );
    }
  } else if ( incidence->type() == "Todo" ) {
    Todo *todo = static_cast<Todo*>( incidence );
    removeIncidenceFromMultiHashByUID<Todo *>(
      d->mTodosForDate, todo->dtDue().date().toString(), todo->uid() );
    if ( todo->hasDueDate() ) {
      d->mTodosForDate.insert( todo->dtDue().date().toString(), todo );
    }
  } else if ( incidence->type() == "Journal" ) {
    Journal *journal = static_cast<Journal*>( incidence );
    removeIncidenceFromMultiHashByUID<Journal *>(
      d->mJournalsForDate, journal->dtStart().date().toString(), journal->uid() );
    d->mJournalsForDate.insert( journal->dtStart().date().toString(), journal );
  } else {
    Q_ASSERT( false );
  }
  // The static_cast is ok as the AkonadiCalendar only observes Incidence objects
  notifyIncidenceChanged( static_cast<Incidence *>( incidence ) );
  setModified( true );
#else
  KCal::Incidence* i = dynamic_cast<KCal::Incidence*>( incidence );
  Q_ASSERT( i );
  Q_ASSERT( d->m_itemMap.contains( i->uid() ) );
  kDebug() << "Updated uid=" << i->uid() << "summary=" << i->summary() << "type=" << i->type();
  Akonadi::Item item = d->m_itemMap[ i->uid() ]->m_item;
  Q_ASSERT( item.isValid() );
  Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob( item, d->m_session );
  connect( job, SIGNAL( result( KJob* ) ), d, SLOT( modifyDone( KJob* ) ) );
#endif
}

bool AkonadiCalendar::addEvent( Event *event )
{
  kDebug();
  /*
  d->mEvents.insert( uid, event );
  if ( !event->recurs() && !event->isMultiDay() ) {
      mEventsForDate.insert( event->dtStart().date().toString(), event );
  }
  */
  return d->addIncidence(event);
}

// this is e.g. called by pimlibs/kcal/icalformat_p.cpp on import to replace
// existing events with newer ones. We probably like to just update in that
// case rather then to delete+create...
bool AkonadiCalendar::deleteEvent( Event *event )
{
  kDebug();
  /*
  d->mDeletedIncidences.append( event );
  if ( !event->recurs() ) {
    removeIncidenceFromMultiHashByUID<Event *>(
      d->mEventsForDate, event->dtStart().date().toString(), event->uid() );
  }
  */
  return d->deleteIncidence(event);
}

// hmmm...
void AkonadiCalendar::deleteAllEvents()
{
  kDebug();
  Q_ASSERT(false);
#if 0
  QHashIterator<QString, Event *>i( d->mEvents );
  while ( i.hasNext() ) {
    i.next();
    notifyIncidenceDeleted( i.value() );
    // suppress update notifications for the relation removal triggered
    // by the following deletions
    i.value()->startUpdates();
  }
  qDeleteAll( d->mEvents );
  d->mEvents.clear();
  d->mEventsForDate.clear();
#endif
}

Event *AkonadiCalendar::event( const QString &uid )
{
  kDebug();
  if( ! d->m_itemMap.contains( uid ) )
    return 0;
  AkonadiCalendarItem *aci = d->m_itemMap[ uid ];
  Event *event = dynamic_cast<Event*>( aci->incidence().get() );
  return event;
}

bool AkonadiCalendar::addTodo( Todo *todo )
{
  kDebug();
  /*
  d->mTodos.insert( uid, todo );
  if ( todo->hasDueDate() ) {
    mTodosForDate.insert( todo->dtDue().date().toString(), todo );
  }
  todo->registerObserver( this );
  setupRelations( todo ); // Set up sub-to-do relations
  setModified( true );
  notifyIncidenceAdded( todo );
  */
  return d->addIncidence(todo);
}

bool AkonadiCalendar::deleteTodo( Todo *todo )
{
  kDebug();
  /*
  // Handle orphaned children
  removeRelations( todo );
  if ( d->mTodos.remove( todo->uid() ) ) {
    setModified( true );
    notifyIncidenceDeleted( todo );
    d->mDeletedIncidences.append( todo );
    if ( todo->hasDueDate() ) {
      removeIncidenceFromMultiHashByUID<Todo *>(
        d->mTodosForDate, todo->dtDue().date().toString(), todo->uid() );
    }
    return true;
  } else {
    kWarning() << "AkonadiCalendar::deleteTodo(): Todo not found.";
    return false;
  }
  */
  return d->deleteIncidence(todo);
}

void AkonadiCalendar::deleteAllTodos()
{
  kDebug();
  Q_ASSERT(false);
#if 0
  QHashIterator<QString, Todo *>i( d->mTodos );
  while ( i.hasNext() ) {
    i.next();
    notifyIncidenceDeleted( i.value() );
    // suppress update notifications for the relation removal triggered
    // by the following deletions
    i.value()->startUpdates();
  }
  qDeleteAll( d->mTodos );
  d->mTodos.clear();
  d->mTodosForDate.clear();
#endif
}

Todo *AkonadiCalendar::todo( const QString &uid )
{
  kDebug();
  if( ! d->m_itemMap.contains( uid ) )
    return 0;
  AkonadiCalendarItem *aci = d->m_itemMap[ uid ];
  Todo *todo = dynamic_cast<Todo*>( aci->incidence().get() );
  return todo;
}

Todo::List AkonadiCalendar::rawTodos( TodoSortField sortField, SortDirection sortDirection )
{
  kDebug()<<sortField<<sortDirection;
  Todo::List todoList;
  QHashIterator<QString, AkonadiCalendarItem*>i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if( Todo *todo = dynamic_cast<Todo*>(i.value()->incidence().get()) )
      todoList.append( todo );
  }
  return sortTodos( &todoList, sortField, sortDirection );
}

Todo::List AkonadiCalendar::rawTodosForDate( const QDate &date )
{
  kDebug()<<date.toString();
  Todo::List todoList;
#if 0
  Todo *t;
  QString dateStr = date.toString();
  QMultiHash<QString, Todo *>::const_iterator it = d->mTodosForDate.constFind( dateStr );
  while ( it != d->mTodosForDate.constEnd() && it.key() == dateStr ) {
    t = it.value();
    todoList.append( t );
    ++it;
  }
#else
  QHashIterator<QString, AkonadiCalendarItem*>i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if( Todo *todo = dynamic_cast<Todo*>(i.value()->incidence().get()) )
      if( todo->dtDue().date().toString() == date.toString() )
        todoList.append( todo );
  }
#endif
  return todoList;
}

Alarm::List AkonadiCalendar::alarmsTo( const KDateTime &to )
{
  kDebug();
  return alarms( KDateTime( QDate( 1900, 1, 1 ) ), to );
}

Alarm::List AkonadiCalendar::alarms( const KDateTime &from, const KDateTime &to )
{
  kDebug();
  Alarm::List alarmList;
#if 0
  QHashIterator<QString, Event *>ie( d->mEvents );
  Event *e;
  while ( ie.hasNext() ) {
    ie.next();
    e = ie.value();
    if ( e->recurs() ) {
      appendRecurringAlarms( alarmList, e, from, to );
    } else {
      appendAlarms( alarmList, e, from, to );
    }
  }

  QHashIterator<QString, Todo *>it( d->mTodos );
  Todo *t;
  while ( it.hasNext() ) {
    it.next();
    t = it.value();
    if (! t->isCompleted() ) {
      appendAlarms( alarmList, t, from, to );
    }
  }
#endif
  return alarmList;
}

Event::List AkonadiCalendar::rawEventsForDate( const QDate &date, const KDateTime::Spec &timespec, EventSortField sortField, SortDirection sortDirection )
{
  kDebug()<<date.toString();
  Event::List eventList;
#if 0
  Event *ev;
  // Find the hash for the specified date
  QString dateStr = date.toString();
  QMultiHash<QString, Event *>::const_iterator it = d->mEventsForDate.constFind( dateStr );
  // Iterate over all non-recurring, single-day events that start on this date
  KDateTime::Spec ts = timespec.isValid() ? timespec : timeSpec();
  KDateTime kdt( date, ts );
  while ( it != d->mEventsForDate.constEnd() && it.key() == dateStr ) {
    ev = it.value();
    KDateTime end( ev->dtEnd().toTimeSpec( ev->dtStart() ) );
    if ( ev->allDay() ) {
      end.setDateOnly( true );
    } else {
      end = end.addSecs( -1 );
    }
    if ( end >= kdt ) eventList.append( ev );
    ++it;
  }
  // Iterate over all events. Look for recurring events that occur on this date
  QHashIterator<QString, Event *>i( d->mEvents );
  while ( i.hasNext() ) {
    i.next();
    ev = i.value();
    if ( ev->recurs() ) {
      if ( ev->isMultiDay() ) {
        int extraDays = ev->dtStart().date().daysTo( ev->dtEnd().date() );
        for ( int i = 0; i <= extraDays; ++i ) {
          if ( ev->recursOn( date.addDays( -i ), ts ) ) {
            eventList.append( ev );
            break;
          }
        }
      } else {
        if ( ev->recursOn( date, ts ) ) eventList.append( ev );
      }
    } else {
      if ( ev->isMultiDay() ) {
        if ( ev->dtStart().date() <= date && ev->dtEnd().date() >= date ) eventList.append( ev );
      }
    }
  }
#endif
  return sortEvents( &eventList, sortField, sortDirection );
}

Event::List AkonadiCalendar::rawEvents( const QDate &start, const QDate &end, const KDateTime::Spec &timespec, bool inclusive )
{
  kDebug()<<start.toString()<<end.toString()<<inclusive;
  Event::List eventList;
#if 0
  KDateTime::Spec ts = timespec.isValid() ? timespec : timeSpec();
  KDateTime st( start, ts );
  KDateTime nd( end, ts );
  KDateTime yesterStart = st.addDays( -1 );
  // Get non-recurring events
  QHashIterator<QString, Event *>i( d->mEvents );
  Event *event;
  while ( i.hasNext() ) {
    i.next();
    event = i.value();
    KDateTime rStart = event->dtStart();
    if ( nd < rStart ) continue;
    if ( inclusive && rStart < st ) continue;
    if ( !event->recurs() ) { // non-recurring events
      KDateTime rEnd = event->dtEnd();
      if ( rEnd < st ) continue;
      if ( inclusive && nd < rEnd ) continue;
    } else { // recurring events
      switch( event->recurrence()->duration() ) {
      case -1: // infinite
        if ( inclusive ) continue;
        break;
      case 0: // end date given
      default: // count given
        KDateTime rEnd( event->recurrence()->endDate(), ts );
        if ( !rEnd.isValid() ) continue;
        if ( rEnd < st ) continue;
        if ( inclusive && nd < rEnd ) continue;
        break;
      } // switch(duration)
    } //if(recurs)
    eventList.append( event );
  }
#endif
  return eventList;
}

Event::List AkonadiCalendar::rawEventsForDate( const KDateTime &kdt )
{
  kDebug();
  return rawEventsForDate( kdt.date(), kdt.timeSpec() );
}

Event::List AkonadiCalendar::rawEvents( EventSortField sortField, SortDirection sortDirection )
{
  kDebug()<<sortField<<sortDirection;
  Event::List eventList;
  QHashIterator<QString, AkonadiCalendarItem*>i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if( Event *event = dynamic_cast<Event*>(i.value()->incidence().get()) )
      eventList.append( event );
  }
  return sortEvents( &eventList, sortField, sortDirection );
}

bool AkonadiCalendar::addJournal( Journal *journal )
{
  kDebug();
  /*
  d->mJournals.insert( uid, journal );
  mJournalsForDate.insert( journal->dtStart().date().toString(), journal );
  journal->registerObserver( this );
  setModified( true );
  notifyIncidenceAdded( journal );
  */
  return d->addIncidence(journal);
}

bool AkonadiCalendar::deleteJournal( Journal *journal )
{
  kDebug();
  //Q_ASSERT( d->mJournals.contains( journal->uid() ) );
  /*
  if ( d->mJournals.remove( journal->uid() ) ) {
    setModified( true );
    notifyIncidenceDeleted( journal );
    d->mDeletedIncidences.append( journal );
    removeIncidenceFromMultiHashByUID<Journal *>(
      d->mJournalsForDate, journal->dtStart().date().toString(), journal->uid() );
    return true;
  } else {
    kWarning() << "AkonadiCalendar::deleteJournal(): Journal not found.";
    return false;
  }
  */
  return d->deleteIncidence(journal);
}

void AkonadiCalendar::deleteAllJournals()
{
  kDebug();
  Q_ASSERT(false);
#if 0
  QHashIterator<QString, Journal *>i( d->mJournals );
  while ( i.hasNext() ) {
    i.next();
    notifyIncidenceDeleted( i.value() );
    // suppress update notifications for the relation removal triggered
    // by the following deletions
    i.value()->startUpdates();
  }
  qDeleteAll( d->mJournals );
  d->mJournals.clear();
  d->mJournalsForDate.clear();
#endif
}

Journal *AkonadiCalendar::journal( const QString &uid )
{
  kDebug();
  if( ! d->m_itemMap.contains( uid ) )
    return 0;
  AkonadiCalendarItem *aci = d->m_itemMap[ uid ];
  Journal *journal = dynamic_cast<Journal*>( aci->incidence().get() );
  return journal;
}

Journal::List AkonadiCalendar::rawJournals( JournalSortField sortField, SortDirection sortDirection )
{
  kDebug()<<sortField<<sortDirection;
  Journal::List journalList;
  QHashIterator<QString, AkonadiCalendarItem*>i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if( Journal *journal = dynamic_cast<Journal*>(i.value()->incidence().get()) )
      journalList.append( journal );
  }
  return sortJournals( &journalList, sortField, sortDirection );
}

Journal::List AkonadiCalendar::rawJournalsForDate( const QDate &date )
{
  kDebug()<<date.toString();
  Journal::List journalList;
#if 0
  Journal *j;
  QString dateStr = date.toString();
  QMultiHash<QString, Journal *>::const_iterator it = d->mJournalsForDate.constFind( dateStr );
  while ( it != d->mJournalsForDate.constEnd() && it.key() == dateStr ) {
    j = it.value();
    journalList.append( j );
    ++it;
  }
#else
  QHashIterator<QString, AkonadiCalendarItem*>i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if( Journal *journal = dynamic_cast<Journal*>(i.value()->incidence().get()) )
      if( journal->dtStart().date().toString() == date.toString() )
        journalList.append( journal );
  }
#endif
  return journalList;
}
