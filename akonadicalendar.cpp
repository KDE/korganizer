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

Akonadi::Collection AkonadiCalendar::collection() const
{
  return d->m_collection;
}

void AkonadiCalendar::setCollection( const Akonadi::Collection &collection )
{
  d->m_monitor->setCollectionMonitored( d->m_collection, false );
  d->m_collection = collection;
  d->m_monitor->setCollectionMonitored( d->m_collection, true );

  // the query changed, thus everything we have already is invalid
  d->clear();
  //reset();

  // stop all running jobs
  d->m_session->clear();
  // start a new job and fetch all items
  Akonadi::ItemFetchJob* job = new Akonadi::ItemFetchJob( d->m_collection, d->m_session );
  job->setFetchScope( d->m_monitor->itemFetchScope() );
  connect( job, SIGNAL(itemsReceived(Akonadi::Item::List)), d, SLOT(itemsAdded(Akonadi::Item::List)) );
  connect( job, SIGNAL(result(KJob*)), d, SLOT(listingDone(KJob*)) );
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

bool AkonadiCalendar::addIncidence( Incidence *incidence )
{
  // first dispatch to addEvent/addTodo/addJournal
  if ( ! Calendar::addIncidence( incidence ) ) {
    return false;
  }
  // then try to add the incidence
  Akonadi::Item item;
  item.setMimeType( "text/calendar" );
  KCal::Incidence::Ptr incidencePtr( incidence ); //no clone() needed
  item.setPayload<KCal::Incidence::Ptr>( incidencePtr );
  Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, d->m_collection );
  connect( job, SIGNAL( result( KJob* ) ), d, SLOT( createDone( KJob* ) ) );
  return true;
}

bool AkonadiCalendar::deleteIncidence( Incidence *incidence )
{
  // first dispatch to deleteEvent/deleteTodo/deleteJournal
  if ( ! Calendar::deleteIncidence( incidence ) ) {
    return false;
  }
  // then try to delete the incidence
  Akonadi::Item item;
  foreach(const Akonadi::Item& i, d->m_items)
    if(i.payload<KCal::Incidence::Ptr>()->uid() == incidence->uid()) {
      item = i;
      break;
    }
  Q_ASSERT(item.isValid());
  Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob( item );
  connect( job, SIGNAL( result( KJob* ) ), d, SLOT( deleteDone( KJob* ) ) );
  return true;
}

bool AkonadiCalendar::addEvent( Event *event )
{
  kDebug();
  QString uid = event->uid();
  Q_ASSERT( ! d->mEvents.contains( uid ) );
/*
  Akonadi::Item item;
  item.setMimeType( "text/calendar" );
  KCal::Incidence* incidence = event;
  KCal::Incidence::Ptr incidencePtr( incidence ); //clone?
  item.setPayload<KCal::Incidence::Ptr>( incidencePtr );
  Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, d->m_collection );
  connect( job, SIGNAL( result( KJob* ) ), d, SLOT( createDone( KJob* ) ) );
#if 0
  d->mEvents.insert( uid, event );
  if ( !event->recurs() && !event->isMultiDay() ) {
      mEventsForDate.insert( event->dtStart().date().toString(), event );
  }
  event->registerObserver( this );
  setModified( true );
  notifyIncidenceAdded( event );
#endif
*/
  return true;
}

bool AkonadiCalendar::deleteEvent( Event *event )
{
  kDebug();
  const QString uid = event->uid();
  Q_ASSERT( d->mEvents.contains( uid ) );
  /*
#if 0
    setModified( true );
    notifyIncidenceDeleted( event );
    d->mDeletedIncidences.append( event );
    if ( !event->recurs() ) {
      removeIncidenceFromMultiHashByUID<Event *>(
        d->mEventsForDate, event->dtStart().date().toString(), event->uid() );
    }
#else
  Akonadi::Item item;
  foreach(const Akonadi::Item& i, d->m_items)
    if(i.payload<KCal::Incidence::Ptr>()->uid() == uid) { item = i; break; }
  Q_ASSERT(item.isValid());
  Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob( item );
  connect( job, SIGNAL( result( KJob* ) ), d, SLOT( deleteDone( KJob* ) ) );
#endif
  */
  return true;
}

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
  return d->mEvents.value( uid );
}

bool AkonadiCalendar::addTodo( Todo *todo )
{
  kDebug();
  QString uid = todo->uid();
  Q_ASSERT( ! d->mTodos.contains( uid ) );
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
  return true;
}


bool AkonadiCalendar::deleteTodo( Todo *todo )
{
  kDebug();
  QString uid = todo->uid();
  Q_ASSERT( d->mTodos.contains( uid ) );
#if 0
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
#else
  return true;
#endif
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
  return d->mTodos.value( uid );
}

Todo::List AkonadiCalendar::rawTodos( TodoSortField sortField,
                                    SortDirection sortDirection )
{
  kDebug();
  Todo::List todoList;
  QHashIterator<QString, Todo *>i( d->mTodos );
  while ( i.hasNext() ) {
    i.next();
    todoList.append( i.value() );
  }
  return sortTodos( &todoList, sortField, sortDirection );
}

Todo::List AkonadiCalendar::rawTodosForDate( const QDate &date )
{
  kDebug();
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

void AkonadiCalendar::incidenceUpdated( IncidenceBase *incidence )
{
  kDebug();
#if 0
  KDateTime nowUTC = KDateTime::currentUtcDateTime();
  incidence->setLastModified( nowUTC );
  // we should probably update the revision number here,
  // or internally in the Event itself when certain things change.
  // need to verify with ical documentation.

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
#endif
}

Event::List AkonadiCalendar::rawEventsForDate( const QDate &date,
                                             const KDateTime::Spec &timespec,
                                             EventSortField sortField,
                                             SortDirection sortDirection )
{
  kDebug();
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

Event::List AkonadiCalendar::rawEvents( const QDate &start, const QDate &end,
                                      const KDateTime::Spec &timespec, bool inclusive )
{
  kDebug();
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

Event::List AkonadiCalendar::rawEvents( EventSortField sortField,
                                      SortDirection sortDirection )
{
  kDebug();
  Event::List eventList;
  QHashIterator<QString, Event *>i( d->mEvents );
  while ( i.hasNext() ) {
    i.next();
    eventList.append( i.value() );
  }
  return sortEvents( &eventList, sortField, sortDirection );
}

bool AkonadiCalendar::addJournal( Journal *journal )
{
  kDebug();
  QString uid = journal->uid();
  Q_ASSERT( ! d->mJournals.contains( uid ) );
#if 0
  d->mJournals.insert( uid, journal );
  mJournalsForDate.insert( journal->dtStart().date().toString(), journal );
  journal->registerObserver( this );
  setModified( true );
  notifyIncidenceAdded( journal );
#endif
  return true;
}

bool AkonadiCalendar::deleteJournal( Journal *journal )
{
  kDebug();
  QString uid = journal->uid();
  Q_ASSERT( d->mJournals.contains( uid ) );
#if 0
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
#endif
  return true;
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
  return d->mJournals.value( uid );
}

Journal::List AkonadiCalendar::rawJournals( JournalSortField sortField,
                                          SortDirection sortDirection )
{
  kDebug();
  Journal::List journalList;
  QHashIterator<QString, Journal *>i( d->mJournals );
  while ( i.hasNext() ) {
    i.next();
    journalList.append( i.value() );
  }
  return sortJournals( &journalList, sortField, sortDirection );
}

Journal::List AkonadiCalendar::rawJournalsForDate( const QDate &date )
{
  kDebug();
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
#endif
  return journalList;
}
