/*
    This file is part of Akonadi.

    Copyright (c) 2009 KDAB
    Author: Sebastian Sauer <sebsauer@kdab.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "akonadicalendar.h"
#include "akonadicalendar_p.h"

#include <kcal/incidence.h>
#include <kcal/event.h>
#include <kcal/todo.h>
#include <kcal/journal.h>
#include <kcal/filestorage.h>
#include <kcal/comparisonvisitor.h>

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

using namespace Akonadi;
using namespace KCal;
using namespace KOrg;

AkonadiCalendar::AkonadiCalendar( const KDateTime::Spec &timeSpec )
  : KOrg::CalendarBase( timeSpec )
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
  d->assertInvariants();
  if ( !d->m_collectionMap.contains( collection.id() ) )
    return;
  Q_ASSERT( d->m_collectionMap.contains( collection.id() ) );
  d->m_monitor->setCollectionMonitored( collection, false );
  AkonadiCalendarCollection *c = d->m_collectionMap.take( collection.id() );
  delete c;

  QHash<Akonadi::Item::Id, Akonadi::Item>::Iterator it( d->m_itemMap.begin() ), end( d->m_itemMap.end() );
  while( it != end) {
    if( it.value().storageCollectionId() == collection.id() ) {
      Akonadi::Item i = *it;
      it = d->m_itemMap.erase(it);
      Q_ASSERT( i.hasPayload<KCal::Incidence::Ptr>() );
      const KCal::Incidence::Ptr incidence = i.payload<KCal::Incidence::Ptr>();
      Q_ASSERT( incidence.get() );
      const QString uid = incidence->uid();
      d->m_uidToItemId.remove( uid );
    } else {
      ++it;
    }
  }
  d->assertInvariants();

  emit calendarChanged();
}

Akonadi::Item AkonadiCalendar::itemForIncidence(Incidence *incidence) const
{
  const QString uid = incidence->uid();
  if( ! d->m_uidToItemId.contains( uid ) ) {
    kDebug()<<"uid="<<uid<<"UNKNOWN_UID";
    return Akonadi::Item();
  }
  const Akonadi::Item aci = d->itemForUid( uid );
  kDebug()<<"uid="<<uid<<"storageCollectionId="<<aci.storageCollectionId();
  return aci;
}

bool AkonadiCalendar::beginChange( Incidence *incidence )
{
  if( ! CalendarBase::beginChange( incidence ) ) {
    return false;
  }
  Q_ASSERT( ! d->m_changes.contains( incidence->uid() ) ); //no nested changes, right?
  d->m_changes << incidence->uid();
  d->m_incidenceBeingChanged = KCal::Incidence::Ptr( incidence->clone() );
  return true;
}

bool AkonadiCalendar::beginChangeFORAKONADI( const Item &incidence )
{
#ifdef AKONADI_PORT_DISABLED
  if( ! CalendarBase::beginChange( incidence ) ) {
    return false;
  }
  Q_ASSERT( ! d->m_changes.contains( incidence->uid() ) ); //no nested changes, right?
  d->m_changes << incidence->uid();
  d->m_incidenceBeingChanged = KCal::Incidence::Ptr( incidence->clone() );
  return true;
#endif // AKONADI_PORT_DISABLED
}

bool AkonadiCalendar::endChange( Incidence *incidence )
{
  Q_ASSERT( incidence );

  const bool isModification = d->m_changes.removeAll( incidence->uid() ) >= 1;
  const QString uid = incidence->uid();

  if( ! CalendarBase::endChange( incidence ) ) {
    // should not happen, but well...
    kDebug() << "Abort modify uid=" << uid << "summary=" << incidence->summary() << "type=" << incidence->type();
    return false;
  }

  if( ! isModification || !d->m_incidenceBeingChanged ) {
    // only if beginChange() with the incidence was called then this is a modification else it
    // is e.g. a new event/todo/journal that was not added yet or an existing one got deleted.
    kDebug() << "Skipping modify uid=" << uid << "summary=" << incidence->summary() << "type=" << incidence->type();
    return false;
  }

  Q_ASSERT( d->m_uidToItemId.contains( uid ) );
  Akonadi::Item item = d->itemForUid( uid );
  Q_ASSERT( item.isValid() );

  // check if there was an actual change to the incidence since beginChange
  // if not, don't kick off a modify job. The reason this is useful is that
  // begin/endChange is used for locking as well, so it is called quite often
  // without any actual changes happening. Nested modify jobs confuse the
  // conflict detection in Akonadi, so let's avoid them.
  ComparisonVisitor v;
  KCal::Incidence::Ptr incidencePtr( d->m_incidenceBeingChanged );
  d->m_incidenceBeingChanged = KCal::Incidence::Ptr();
  if ( v.compare( incidence, incidencePtr.get() ) )
    return true;

  kDebug() << "modify uid=" << uid << "summary=" << incidence->summary() << "type=" << incidence->type() << "storageCollectionId=" << item.storageCollectionId();
  Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob( item, d->m_session );

  connect( job, SIGNAL( result( KJob* ) ), d, SLOT( modifyDone( KJob* ) ) );
  return true;
}

bool AkonadiCalendar::endChangeFORAKONADI( const Item &incidence )
{
#ifdef AKONADI_PORT_DISABLED
  Q_ASSERT( incidence );

  const bool isModification = d->m_changes.removeAll( incidence->uid() ) >= 1;
  const QString uid = incidence->uid();

  if( ! CalendarBase::endChange( incidence ) ) {
    // should not happen, but well...
    kDebug() << "Abort modify uid=" << uid << "summary=" << incidence->summary() << "type=" << incidence->type();
    return false;
  }

  if( ! isModification || !d->m_incidenceBeingChanged ) {
    // only if beginChange() with the incidence was called then this is a modification else it
    // is e.g. a new event/todo/journal that was not added yet or an existing one got deleted.
    kDebug() << "Skipping modify uid=" << uid << "summary=" << incidence->summary() << "type=" << incidence->type();
    return false;
  }

  Q_ASSERT( d->m_uidToItemId.contains( uid ) );
  Akonadi::Item item = d->itemForUid( uid )->m_item;
  Q_ASSERT( item.isValid() );

  // check if there was an actual change to the incidence since beginChange
  // if not, don't kick off a modify job. The reason this is useful is that
  // begin/endChange is used for locking as well, so it is called quite often
  // without any actual changes happening. Nested modify jobs confuse the
  // conflict detection in Akonadi, so let's avoid them.
  ComparisonVisitor v;
  KCal::Incidence::Ptr incidencePtr( d->m_incidenceBeingChanged );
  d->m_incidenceBeingChanged = KCal::Incidence::Ptr();
  if ( v.compare( incidence, incidencePtr.get() ) )
    return true;

  kDebug() << "modify uid=" << uid << "summary=" << incidence->summary() << "type=" << incidence->type() << "storageCollectionId=" << item.storageCollectionId();
  Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob( item, d->m_session );

  connect( job, SIGNAL( result( KJob* ) ), d, SLOT( modifyDone( KJob* ) ) );
  return true;
#else // AKONADI_PORT_DISABLED
  return false;
#endif // AKONADI_PORT_DISABLED
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
  if ( d->mFileName.isEmpty() ) return false;
  if ( ! isModified() ) return true;
  FileStorage storage( this, d->mFileName, d->mFormat );
  return storage.save();
#else
  return true;
#endif
}

void AkonadiCalendar::close()
{
  kDebug();
  setObserversEnabled( false );
#if 0
  d->mFileName.clear();
  deleteAllEvents();
  deleteAllTodos();
  deleteAllJournals();
  d->mDeletedIncidences.clearAll();
#endif
  setModified( false );
  setObserversEnabled( true );
}

bool AkonadiCalendar::addAgent( const KUrl &mUrl )
{
  kDebug()<<mUrl;
  Akonadi::AgentType type = Akonadi::AgentManager::self()->type( "akonadi_ical_resource" );
  Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob( type, d->m_session );
  job->setProperty("path", mUrl.path());
  connect( job, SIGNAL( result( KJob * ) ), d, SLOT( agentCreated( KJob * ) ) );
  job->start();
  return true;
}

bool AkonadiCalendar::addIncidence( Incidence *incidence )
{
  kDebug();
  // dispatch to addEvent/addTodo/addJournal
  return CalendarBase::addIncidence( incidence );
}


bool AkonadiCalendar::addIncidenceFORAKONADI( const Item &incidence )
{
  kDebug();
  // dispatch to addEvent/addTodo/addJournal
  return CalendarBase::addIncidenceFORAKONADI( incidence );
}

bool AkonadiCalendar::deleteIncidence( Incidence *incidence )
{
  kDebug();
  // dispatch to deleteEvent/deleteTodo/deleteJournal
  return CalendarBase::deleteIncidence( incidence );
}

bool AkonadiCalendar::deleteIncidenceFORAKONADI( const Item &incidence )
{
  kDebug();
  // dispatch to deleteEvent/deleteTodo/deleteJournal
  return CalendarBase::deleteIncidenceFORAKONADI( incidence );
}

// This method will be called probably multiple times if a series of changes where done. One finished the endChange() method got called.
void AkonadiCalendar::incidenceUpdated( IncidenceBase *incidence )
{
  KDateTime nowUTC = KDateTime::currentUtcDateTime();
  incidence->setLastModified( nowUTC );
  KCal::Incidence* i = dynamic_cast<KCal::Incidence*>( incidence );
  Q_ASSERT( i );
  Q_ASSERT( d->m_uidToItemId.contains( i->uid() ) );
  Akonadi::Item item = d->itemForUid( i->uid() );
  Q_ASSERT( item.isValid() );
  kDebug() << "Updated uid=" << i->uid() << "summary=" << i->summary() << "type=" << i->type() << "storageCollectionId=" << item.storageCollectionId();
}

bool AkonadiCalendar::addEvent( Event *event )
{
  kDebug();
  return d->addIncidence(event);
}

bool AkonadiCalendar::addEventFORAKONADI( const Item &event )
{
  kDebug();
  return d->addIncidenceFORAKONADI(event);
}

// this is e.g. called by pimlibs/kcal/icalformat_p.cpp on import to replace
// existing events with newer ones. We probably like to just update in that
// case rather then to delete+create...
bool AkonadiCalendar::deleteEvent( Event *event )
{
  kDebug();
  return d->deleteIncidence(event);
}

bool AkonadiCalendar::deleteEventFORAKONADI( const Item &event )
{
  kDebug();
  return d->deleteIncidenceFORAKONADI(event);
}

Event *AkonadiCalendar::event( const QString &uid )
{
  kDebug();
  if( !d->m_uidToItemId.contains( uid ) )
    return 0;
  Akonadi::Item aci = d->itemForUid( uid );
  return KOHelper::event( aci ).get();
}

Item AkonadiCalendar::eventFORAKONADI( const Item::Id &id )
{
  const Akonadi::Item item = d->m_itemMap.value( id );
  if ( KOHelper::event( item ) )
    return item;
  else
    return Akonadi::Item();
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

bool AkonadiCalendar::addTodoFORAKONADI( const Item &todo )
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
  return d->addIncidenceFORAKONADI(todo);
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
      removeIncidenceFromMultiHashByUID<Todo *>( d->mTodosForDate, todo->dtDue().date().toString(), todo->uid() );
    }
    return true;
  } else {
    kWarning() << "AkonadiCalendar::deleteTodo(): Todo not found.";
    return false;
  }
  */
  return d->deleteIncidence(todo);
}

bool AkonadiCalendar::deleteTodoFORAKONADI( const Item &todo )
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
      removeIncidenceFromMultiHashByUID<Todo *>( d->mTodosForDate, todo->dtDue().date().toString(), todo->uid() );
    }
    return true;
  } else {
    kWarning() << "AkonadiCalendar::deleteTodo(): Todo not found.";
    return false;
  }
  */
  return d->deleteIncidenceFORAKONADI(todo);
}

Todo *AkonadiCalendar::todo( const QString &uid )
{
  kDebug();
  if( ! d->m_uidToItemId.contains( uid ) )
    return 0;
  Akonadi::Item aci = d->itemForUid( uid );
  return KOHelper::todo( aci ).get();
}

Item AkonadiCalendar::todoFORAKONADI( const Item::Id &id )
{
  const Akonadi::Item item = d->m_itemMap.value( id );
  if ( KOHelper::todo( item ) )
    return item;
  else
    return Akonadi::Item();
}

Todo::List AkonadiCalendar::rawTodos( TodoSortField sortField, SortDirection sortDirection )
{
  kDebug()<<sortField<<sortDirection;
  Todo::List todoList;
  QHashIterator<Akonadi::Item::Id, Akonadi::Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if( Todo::Ptr todo = KOHelper::todo( i.value() ) )
      todoList.append( todo.get() );
  }
  return sortTodos( &todoList, sortField, sortDirection );
}

Item::List AkonadiCalendar::rawTodosFORAKONADI( TodoSortField sortField, SortDirection sortDirection )
{
  kDebug()<<sortField<<sortDirection;
  Item::List todoList;
  QHashIterator<Akonadi::Item::Id, Akonadi::Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if( KOHelper::todo( i.value() ) )
      todoList.append( i.value() );
  }
  return sortTodosFORAKONADI( todoList, sortField, sortDirection );
}


Todo::List AkonadiCalendar::rawTodosForDate( const QDate &date )
{
  kDebug()<<date.toString();
  Todo::List todoList;
  QString dateStr = date.toString();
  QMultiHash<QString, KCal::Incidence::Ptr>::const_iterator it = d->m_incidenceForDate.constFind( dateStr );
  while ( it != d->m_incidenceForDate.constEnd() && it.key() == dateStr ) {
    if( Todo *t = dynamic_cast<Todo*>(it.value().get()) )
      todoList.append( t );
    ++it;
  }
  return todoList;
}

Item::List AkonadiCalendar::rawTodosForDateFORAKONADI( const QDate &date )
{
#ifdef AKONADI_PORT_DISABLED
  kDebug()<<date.toString();
  Todo::List todoList;
  QString dateStr = date.toString();
  QMultiHash<QString, KCal::Incidence::Ptr>::const_iterator it = d->m_incidenceForDate.constFind( dateStr );
  while ( it != d->m_incidenceForDate.constEnd() && it.key() == dateStr ) {
    if( Todo *t = dynamic_cast<Todo*>(it.value().get()) )
      todoList.append( t );
    ++it;
  }
  return todoList;
#else // AKONADI_PORT_DISABLED
  return Item::List();
#endif // AKONADI_PORT_DISABLED
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
    if ( e->recurs() ) appendRecurringAlarms( alarmList, e, from, to ); else appendAlarms( alarmList, e, from, to );
  }

  QHashIterator<QString, Todo *>it( d->mTodos );
  Todo *t;
  while ( it.hasNext() ) {
    it.next();
    t = it.value();
    if (! t->isCompleted() ) appendAlarms( alarmList, t, from, to );
  }
#else
  kWarning()<<"TODO";
#endif
  return alarmList;
}

Event::List AkonadiCalendar::rawEventsForDate( const QDate &date, const KDateTime::Spec &timespec, EventSortField sortField, SortDirection sortDirection )
{
  kDebug()<<date.toString();
  Event::List eventList;
  // Find the hash for the specified date
  QString dateStr = date.toString();
  // Iterate over all non-recurring, single-day events that start on this date
  QMultiHash<QString, KCal::Incidence::Ptr>::const_iterator it = d->m_incidenceForDate.constFind( dateStr );
  KDateTime::Spec ts = timespec.isValid() ? timespec : timeSpec();
  KDateTime kdt( date, ts );
  while ( it != d->m_incidenceForDate.constEnd() && it.key() == dateStr ) {
    if( Event *ev = dynamic_cast<Event*>(it.value().get()) ) {
      KDateTime end( ev->dtEnd().toTimeSpec( ev->dtStart() ) );
      if ( ev->allDay() )
        end.setDateOnly( true ); else end = end.addSecs( -1 );
      if ( end >= kdt )
        eventList.append( ev );
    }
    ++it;
  }
  // Iterate over all events. Look for recurring events that occur on this date
  QHashIterator<Akonadi::Item::Id, Akonadi::Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if( Event::Ptr ev = KOHelper::event( i.value() ) ) {
      if ( ev->recurs() ) {
        if ( ev->isMultiDay() ) {
          int extraDays = ev->dtStart().date().daysTo( ev->dtEnd().date() );
          for ( int i = 0; i <= extraDays; ++i ) {
            if ( ev->recursOn( date.addDays( -i ), ts ) ) {
              eventList.append( ev.get() );
              break;
            }
          }
        } else {
          if ( ev->recursOn( date, ts ) )
            eventList.append( ev.get() );
        }
      } else {
        if ( ev->isMultiDay() ) {
          if ( ev->dtStart().date() <= date && ev->dtEnd().date() >= date )
            eventList.append( ev.get() );
        }
      }
    }
  }
  return sortEvents( &eventList, sortField, sortDirection );
}

Item::List AkonadiCalendar::rawEventsForDateFORAKONADI( const QDate &date, const KDateTime::Spec &timespec, EventSortField sortField, SortDirection sortDirection )
{
#ifdef AKONADI_PORT_DISABLED
  kDebug()<<date.toString();
  Event::List eventList;
  // Find the hash for the specified date
  QString dateStr = date.toString();
  // Iterate over all non-recurring, single-day events that start on this date
  QMultiHash<QString, KCal::Incidence::Ptr>::const_iterator it = d->m_incidenceForDate.constFind( dateStr );
  KDateTime::Spec ts = timespec.isValid() ? timespec : timeSpec();
  KDateTime kdt( date, ts );
  while ( it != d->m_incidenceForDate.constEnd() && it.key() == dateStr ) {
    if( Event *ev = dynamic_cast<Event*>(it.value().get()) ) {
      KDateTime end( ev->dtEnd().toTimeSpec( ev->dtStart() ) );
      if ( ev->allDay() )
        end.setDateOnly( true ); else end = end.addSecs( -1 );
      if ( end >= kdt )
        eventList.append( ev );
    }
    ++it;
  }
  // Iterate over all events. Look for recurring events that occur on this date
  QHashIterator<Akonadi::Item::Id, AkonadiCalendarItem*>i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if( Event *ev = dynamic_cast<Event*>(i.value()->incidence().get()) ) {
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
          if ( ev->recursOn( date, ts ) )
            eventList.append( ev );
        }
      } else {
        if ( ev->isMultiDay() ) {
          if ( ev->dtStart().date() <= date && ev->dtEnd().date() >= date )
            eventList.append( ev );
        }
      }
    }
  }
  return sortEvents( &eventList, sortField, sortDirection );
#else
  return Item::List();
#endif
}

Event::List AkonadiCalendar::rawEvents( const QDate &start, const QDate &end, const KDateTime::Spec &timespec, bool inclusive )
{
  kDebug()<<start.toString()<<end.toString()<<inclusive;
  Event::List eventList;
  KDateTime::Spec ts = timespec.isValid() ? timespec : timeSpec();
  KDateTime st( start, ts );
  KDateTime nd( end, ts );
  KDateTime yesterStart = st.addDays( -1 );
  // Get non-recurring events
  QHashIterator<Akonadi::Item::Id, Akonadi::Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if( Event::Ptr event = KOHelper::event( i.value() ) ) {
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
      eventList.append( event.get() );
    }
  }
  return eventList;
}

Item::List AkonadiCalendar::rawEventsFORAKONADI( const QDate &start, const QDate &end, const KDateTime::Spec &timespec, bool inclusive )
{
  kDebug()<<start.toString()<<end.toString()<<inclusive;
  Item::List eventList;
  KDateTime::Spec ts = timespec.isValid() ? timespec : timeSpec();
  KDateTime st( start, ts );
  KDateTime nd( end, ts );
  KDateTime yesterStart = st.addDays( -1 );
  // Get non-recurring events
  QHashIterator<Akonadi::Item::Id, Akonadi::Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if( Event::Ptr event = KOHelper::event( i.value() ) ) {
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
      eventList.append( i.value() );
    }
  }
  return eventList;
}

Event::List AkonadiCalendar::rawEventsForDate( const KDateTime &kdt )
{
  kDebug();
  return rawEventsForDate( kdt.date(), kdt.timeSpec() );
}

Item::List AkonadiCalendar::rawEventsForDateFORAKONADI( const KDateTime &kdt )
{
  kDebug();
  return rawEventsForDateFORAKONADI( kdt.date(), kdt.timeSpec() );
}

Event::List AkonadiCalendar::rawEvents( EventSortField sortField, SortDirection sortDirection )
{
  kDebug()<<sortField<<sortDirection;
  Event::List eventList;
  QHashIterator<Akonadi::Item::Id, Akonadi::Item>i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if( Event::Ptr event = KOHelper::event( i.value() ) )
      eventList.append( event.get() );
  }
  return sortEvents( &eventList, sortField, sortDirection );
}

Item::List AkonadiCalendar::rawEventsFORAKONADI( EventSortField sortField, SortDirection sortDirection )
{
  kDebug()<<sortField<<sortDirection;
  Item::List eventList;
  QHashIterator<Akonadi::Item::Id, Akonadi::Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if( KOHelper::event( i.value() ) )
      eventList.append( i.value() );
  }
  return sortEventsFORAKONADI( eventList, sortField, sortDirection );
}

bool AkonadiCalendar::addJournal( Journal *journal )
{
  kDebug();
  return d->addIncidence(journal);
}

bool AkonadiCalendar::addJournalFORAKONADI( const Item &journal )
{
  kDebug();
  return d->addIncidenceFORAKONADI(journal);
}

bool AkonadiCalendar::deleteJournal( Journal *journal )
{
  kDebug();
  return d->deleteIncidence(journal);
}

bool AkonadiCalendar::deleteJournalFORAKONADI( const Item &journal )
{
  kDebug();
  return d->deleteIncidenceFORAKONADI(journal);
}


Journal *AkonadiCalendar::journal( const QString &uid )
{
  kDebug();
  if( ! d->m_uidToItemId.contains( uid ) )
    return 0;
  Akonadi::Item aci = d->itemForUid( uid );
  return KOHelper::journal( aci ).get();
}


Item AkonadiCalendar::journalFORAKONADI( const Item::Id &id )
{
  const Akonadi::Item item = d->m_itemMap.value( id );
  if ( KOHelper::journal( item ) )
    return item;
  else
    return Akonadi::Item();
}

Journal::List AkonadiCalendar::rawJournals( JournalSortField sortField, SortDirection sortDirection )
{
  kDebug()<<sortField<<sortDirection;
  Journal::List journalList;
  QHashIterator<Akonadi::Item::Id, Akonadi::Item>i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if( Journal::Ptr journal = KOHelper::journal( i.value() ) )
      journalList.append( journal.get() );
  }
  return sortJournals( &journalList, sortField, sortDirection );
}

Item::List AkonadiCalendar::rawJournalsFORAKONADI( JournalSortField sortField, SortDirection sortDirection )
{
  kDebug()<<sortField<<sortDirection;
  Item::List journalList;
  QHashIterator<Akonadi::Item::Id, Akonadi::Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if( KOHelper::journal( i.value() ) )
      journalList.append( i.value() );
  }
  return sortJournalsFORAKONADI( journalList, sortField, sortDirection );
}

Journal::List AkonadiCalendar::rawJournalsForDate( const QDate &date )
{
  kDebug()<<date.toString();
  Journal::List journalList;
  QString dateStr = date.toString();
  
  QMultiHash<QString, KCal::Incidence::Ptr>::const_iterator it = d->m_incidenceForDate.constFind( dateStr );
  while ( it != d->m_incidenceForDate.constEnd() && it.key() == dateStr ) {
    if( Journal *j = dynamic_cast<Journal*>(it.value().get()) )
      journalList.append( j );
    ++it;
  }
  return journalList;
}

Item::List AkonadiCalendar::rawJournalsForDateFORAKONADI( const QDate &date )
{
#ifdef AKONADI_PORT_DISABLED
  kDebug()<<date.toString();
  Journal::List journalList;
  QString dateStr = date.toString();

  QMultiHash<QString, KCal::Incidence::Ptr>::const_iterator it = d->m_incidenceForDate.constFind( dateStr );
  while ( it != d->m_incidenceForDate.constEnd() && it.key() == dateStr ) {
    if( Journal *j = dynamic_cast<Journal*>(it.value().get()) )
      journalList.append( j );
    ++it;
  }
  return journalList;
#else // AKONADI_PORT_DISABLED
  return Item::List();
#endif // AKONADI_PORT_DISABLED
}
