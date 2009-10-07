/*
  This file is part of the kcal library.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the CalendarNull class.

  @brief
  Represents a null calendar class; that is, a calendar which contains
  no information and provides no capabilities.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/
#include "calendarbase.h"
#include "calendarnull.h"

using namespace Akonadi;
using namespace KCal;
using namespace KOrg;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KOrg::CalendarNull::Private
{
};
static CalendarNull *mSelf = 0;
//@endcond

CalendarNull::CalendarNull( const KDateTime::Spec &timeSpec )
  : CalendarBase( timeSpec ),
    d( new KOrg::CalendarNull::Private )
{}

CalendarNull::CalendarNull( const QString &timeZoneId )
  : CalendarBase( timeZoneId ),
    d( new KOrg::CalendarNull::Private )
{}

CalendarNull::~CalendarNull()
{
  delete d;
}

CalendarNull *CalendarNull::self()
{
  if ( !mSelf ) {
    mSelf = new CalendarNull( KDateTime::UTC );
  }

  return mSelf;
}

void CalendarNull::close()
{
}

bool CalendarNull::save()
{
  return true;
}

bool CalendarNull::reload()
{
  return true;
}

bool CalendarNull::addEvent( Event *event )
{
  Q_UNUSED ( event );
  return false;
}

bool CalendarNull::addEventFORAKONADI( const Item &event )
{
  Q_UNUSED ( event );
  return false;
}

bool CalendarNull::deleteEvent( Event *event )
{
  Q_UNUSED( event );
  return false;
}

bool CalendarNull::deleteEventFORAKONADI( const Item &event )
{
  Q_UNUSED( event );
  return false;
}

void CalendarNull::deleteAllEvents()
{
}

void CalendarNull::deleteAllEventsFORAKONADI()
{
}

Event::List CalendarNull::rawEvents( KOrg::EventSortField sortField,
                                     KOrg::SortDirection sortDirection )
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Event::List();
}

Item::List CalendarNull::rawEventsFORAKONADI( KOrg::EventSortField sortField,
                                     KOrg::SortDirection sortDirection )
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Item::List();
}

Event::List CalendarNull::rawEvents( const QDate &start, const QDate &end,
                                     const KDateTime::Spec &timeSpec,
                                     bool inclusive )
{
  Q_UNUSED( start );
  Q_UNUSED( end );
  Q_UNUSED( timeSpec );
  Q_UNUSED( inclusive );
  return Event::List();
}

Item::List CalendarNull::rawEventsFORAKONADI( const QDate &start, const QDate &end,
                                     const KDateTime::Spec &timeSpec,
                                     bool inclusive )
{
  Q_UNUSED( start );
  Q_UNUSED( end );
  Q_UNUSED( timeSpec );
  Q_UNUSED( inclusive );
  return Item::List();
}

Event::List CalendarNull::rawEventsForDate( const QDate &date,
                                            const KDateTime::Spec &timeSpec,
                                            KOrg::EventSortField sortField,
                                            KOrg::SortDirection sortDirection )
{
  Q_UNUSED( date );
  Q_UNUSED( timeSpec );
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Event::List();
}

Item::List CalendarNull::rawEventsForDateFORAKONADI( const QDate &date,
                                            const KDateTime::Spec &timeSpec,
                                            KOrg::EventSortField sortField,
                                            KOrg::SortDirection sortDirection )
{
  Q_UNUSED( date );
  Q_UNUSED( timeSpec );
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Item::List();
}

Event::List CalendarNull::rawEventsForDate( const KDateTime &dt )
{
  Q_UNUSED( dt );
  return Event::List();
}

Item::List CalendarNull::rawEventsForDateFORAKONADI( const KDateTime &dt )
{
  Q_UNUSED( dt );
  return Item::List();
}

Event *CalendarNull::event( const QString &uid )
{
  Q_UNUSED( uid );
  return 0;
}


Item CalendarNull::eventFORAKONADI( const Item::Id &uid )
{
  Q_UNUSED( uid );
  return Item();
}

bool CalendarNull::addTodo( Todo *todo )
{
  Q_UNUSED( todo );
  return false;
}

bool CalendarNull::addTodoFORAKONADI( const Item &todo )
{
  Q_UNUSED( todo );
  return false;
}

bool CalendarNull::deleteTodo( Todo *todo )
{
  Q_UNUSED( todo );
  return false;
}

bool CalendarNull::deleteTodoFORAKONADI( const Item &todo )
{
  Q_UNUSED( todo );
  return false;
}

void CalendarNull::deleteAllTodos()
{
}

void CalendarNull::deleteAllTodosFORAKONADI()
{
}

Todo::List CalendarNull::rawTodos( KOrg::TodoSortField sortField,
                                   KOrg::SortDirection sortDirection )
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Todo::List();
}

Item::List CalendarNull::rawTodosFORAKONADI( KOrg::TodoSortField sortField,
                                   KOrg::SortDirection sortDirection )
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Item::List();
}

Todo::List CalendarNull::rawTodosForDate( const QDate &date )
{
  Q_UNUSED( date );
  return Todo::List();
}

Item::List CalendarNull::rawTodosForDateFORAKONADI( const QDate &date )
{
  Q_UNUSED( date );
  return Item::List();
}

Todo *CalendarNull::todo( const QString &uid )
{
  Q_UNUSED( uid );
  return 0;
}

Item CalendarNull::todoFORAKONADI( const Item::Id &uid )
{
  Q_UNUSED( uid );
  return Item();
}

bool CalendarNull::addJournal( Journal *journal )
{
  Q_UNUSED( journal );
  return false;
}

bool CalendarNull::addJournalFORAKONADI( const Item &journal )
{
  Q_UNUSED( journal );
  return false;
}

bool CalendarNull::deleteJournal( Journal *journal )
{
  Q_UNUSED( journal );
  return false;
}

bool CalendarNull::deleteJournalFORAKONADI( const Item &journal )
{
  Q_UNUSED( journal );
  return false;
}

void CalendarNull::deleteAllJournals()
{
}

void CalendarNull::deleteAllJournalsFORAKONADI()
{
}

Journal::List CalendarNull::rawJournals( KOrg::JournalSortField sortField,
                                         KOrg::SortDirection sortDirection )
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Journal::List();
}

Item::List CalendarNull::rawJournalsFORAKONADI( KOrg::JournalSortField sortField,
                                         KOrg::SortDirection sortDirection )
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Item::List();
}

Journal::List CalendarNull::rawJournalsForDate( const QDate &date )
{
  Q_UNUSED( date );
  return Journal::List();
}

Item::List CalendarNull::rawJournalsForDateFORAKONADI( const QDate &date )
{
  Q_UNUSED( date );
  return Item::List();
}

Journal *CalendarNull::journal( const QString &uid )
{
  Q_UNUSED( uid );
  return 0;
}

Item CalendarNull::journalFORAKONADI( const Item::Id &uid )
{
  Q_UNUSED( uid );
  return Item();
}

Alarm::List CalendarNull::alarms( const KDateTime &from, const KDateTime &to )
{
  Q_UNUSED( from );
  Q_UNUSED( to );
  return Alarm::List();
}

void CalendarNull::incidenceUpdated( IncidenceBase *incidenceBase )
{
  Q_UNUSED( incidenceBase );
}
