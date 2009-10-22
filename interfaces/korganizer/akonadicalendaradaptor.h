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

#ifndef AKONADICALENDARADAPTOR_H
#define AKONADICALENDARADAPTOR_H

#include "incidencechangerbase.h"

#include <akonadi/kcal/akonadicalendar.h>

#include <KCal/CalendarResources>
#include <KCal/CalFilter>
#include <KCal/DndFactory>
#include <KCal/FileStorage>
#include <KCal/FreeBusy>
#include <KCal/ICalDrag>
#include <KCal/ICalFormat>
#include <KCal/VCalFormat>
#include <kcal/listbase.h>

namespace KOrg {

template<class T> inline T* itemToIncidence(const Akonadi::Item &item) {
  return item.payload< boost::shared_ptr<T> >().get();
}

template<class T> inline KCal::ListBase<T> itemsToIncidences(QList<Akonadi::Item> items) {
  KCal::ListBase<T> list;
  foreach(const Akonadi::Item &item, items)
    list.append( itemToIncidence<T>(item) );
  return list;
}

template<class T> inline Akonadi::Item incidenceToItem(T* incidence) {
  Akonadi::Item item;
  item.setPayload< boost::shared_ptr<T> >( boost::shared_ptr<T>(incidence->clone()) );
  return item;
}

class KORGANIZER_INTERFACES_EXPORT AkonadiCalendarAdaptor : public KCal::Calendar
{
  public:
    explicit AkonadiCalendarAdaptor(AkonadiCalendar *calendar, IncidenceChangerBase* changer)
      : KCal::Calendar( KOPrefs::instance()->timeSpec() )
      , mCalendar( calendar ), mChanger( changer )
    {
      Q_ASSERT(mCalendar);
      Q_ASSERT(mChanger);
    }

    virtual ~AkonadiCalendarAdaptor() {}
    
    virtual bool save() { kDebug(); return true; } //unused
    virtual bool reload() { kDebug(); return true; } //unused
    virtual void close() { kDebug(); } //unused
    
    virtual bool addEvent( Event *event )
    {
      return mChanger->addIncidence( Incidence::Ptr( event->clone() ) );
    }
    
    virtual bool deleteEvent( Event *event )
    {
      return mChanger->deleteIncidence( incidenceToItem( event ) );
    }
    
    virtual void deleteAllEvents() { Q_ASSERT(false); } //unused
    
    virtual Event::List rawEvents(KCal::EventSortField sortField = KCal::EventSortUnsorted, KCal::SortDirection sortDirection = KCal::SortDirectionAscending )
    {
      return itemsToIncidences<Event>( mCalendar->rawEvents( (KOrg::EventSortField)sortField, (KOrg::SortDirection)sortDirection ) );
    }
    
    virtual Event::List rawEventsForDate( const KDateTime &dt )
    {
      return itemsToIncidences<Event>( mCalendar->rawEventsForDate( dt ) );
    }
    
    virtual Event::List rawEvents( const QDate &start, const QDate &end, const KDateTime::Spec &timeSpec = KDateTime::Spec(), bool inclusive = false )
    {
      return itemsToIncidences<Event>( mCalendar->rawEvents( start, end, timeSpec, inclusive ) );
    }
    
    virtual Event::List rawEventsForDate( const QDate &date, const KDateTime::Spec &timeSpec = KDateTime::Spec(), KCal::EventSortField sortField = KCal::EventSortUnsorted, KCal::SortDirection sortDirection = KCal::SortDirectionAscending )
    {
      return itemsToIncidences<Event>( mCalendar->rawEventsForDate( date, timeSpec, (KOrg::EventSortField)sortField, (KOrg::SortDirection)sortDirection ) );
    }
    
    virtual Event *event( const QString &uid )
    {
      return itemToIncidence<Event>( mCalendar->event( mCalendar->itemIdForIncidenceUid(uid) ) );
    }
    
    virtual bool addTodo( Todo *todo )
    {
      return mChanger->addIncidence( Incidence::Ptr( todo->clone() ) );
    }
    
    virtual bool deleteTodo( Todo *todo )
    {
      return mChanger->deleteIncidence( incidenceToItem( todo ) );
    }
    
    virtual void deleteAllTodos() { Q_ASSERT(false); } //unused
    
    virtual Todo::List rawTodos( KCal::TodoSortField sortField = KCal::TodoSortUnsorted, KCal::SortDirection sortDirection = KCal::SortDirectionAscending )
    {
      return itemsToIncidences<Todo>( mCalendar->rawTodos( (KOrg::TodoSortField)sortField, (KOrg::SortDirection)sortDirection ) );
    }
    
    virtual Todo::List rawTodosForDate( const QDate &date )
    {
      return itemsToIncidences<Todo>( mCalendar->rawTodosForDate( date ) );
    }
    
    virtual Todo *todo( const QString &uid )
    {
      return itemToIncidence<Todo>( mCalendar->todo( mCalendar->itemIdForIncidenceUid(uid) ) );
    }
    
    virtual bool addJournal( Journal *journal )
    {
      return mChanger->addIncidence( Incidence::Ptr( journal->clone() ) );
    }
    
    virtual bool deleteJournal( Journal *journal )
    {
      return mChanger->deleteIncidence( incidenceToItem( journal ) );
    }
    
    virtual void deleteAllJournals() { Q_ASSERT(false); } //unused

    virtual Journal::List rawJournals( KCal::JournalSortField sortField = KCal::JournalSortUnsorted, KCal::SortDirection sortDirection = KCal::SortDirectionAscending )
    {
      return itemsToIncidences<Journal>( mCalendar->rawJournals( (KOrg::JournalSortField)sortField, (KOrg::SortDirection)sortDirection ) );
    }
    
    virtual Journal::List rawJournalsForDate( const QDate &dt )
    {
      return itemsToIncidences<Journal>( mCalendar->rawJournalsForDate( dt ) );
    }
    
    virtual Journal *journal( const QString &uid )
    {
      return itemToIncidence<Journal>( mCalendar->journal( mCalendar->itemIdForIncidenceUid(uid) ) );
    }
    
    virtual Alarm::List alarms( const KDateTime &from, const KDateTime &to )
    {
      return itemsToIncidences<Alarm>( mCalendar->alarms( from, to ) );
    }

  private:
    AkonadiCalendar *mCalendar;
    IncidenceChangerBase *mChanger;
};

}

#endif
