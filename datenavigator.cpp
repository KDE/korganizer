/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "datenavigator.h"

#include <calendarsystem/kcalendarsystem.h>

#include "kocore.h"

#include <kdebug.h>

using namespace KCal;

DateNavigator::DateNavigator( QObject *parent, const char *name )
  : QObject( parent, name )
{
  mSelectedDates.append( QDate::currentDate() );
}

DateNavigator::~DateNavigator()
{
}

DateList DateNavigator::selectedDates()
{
  return mSelectedDates;
}

int DateNavigator::datesCount() const
{
  return mSelectedDates.count();
}

void DateNavigator::selectDates( const DateList& dateList )
{
  if (dateList.count() > 0) {
    mSelectedDates = dateList;
    
    emitSelected();
  }
}

void DateNavigator::selectDate( const QDate &date )
{
  QDate d = date;

  if ( !d.isValid() ) {
    kdDebug() << "DateNavigator::selectDates(QDate): an invalid date was passed as a parameter!" << endl;
    d = QDate::currentDate();
  }

  mSelectedDates.clear();
  mSelectedDates.append( d );

  emitSelected();
}

void DateNavigator::selectDates( int count )
{
  selectDates( mSelectedDates.first(), count );
}

void DateNavigator::selectDates( const QDate &d, int count )
{
  DateList dates;

  int i;
  for( i = 0; i < count; ++i ) {
    dates.append( d.addDays( i ) );
  }
  
  mSelectedDates = dates;
  
  emitSelected();
}

void DateNavigator::selectWeek()
{
  selectWeek( mSelectedDates.first() );
}

void DateNavigator::selectWeek( const QDate &d )
{
  QDate firstDate;

  int dayOfWeek = KOCore::self()->calendarSystem()->dayOfTheWeek( d );

  if ( dayOfWeek == 7 ) {
    if ( KGlobal::locale()->weekStartsMonday() ) {
      firstDate = d.addDays( -6 );
    } else {
      firstDate = d.addDays( 1 );
    }
  } else if ( dayOfWeek > 1 ) {
    firstDate = d.addDays( dayOfWeek * -1 + 1 );
  } else  {
    firstDate = d;
  }

  selectDates( firstDate, 7 );
}

void DateNavigator::selectWorkWeek()
{
  selectWorkWeek( mSelectedDates.first() );
}

void DateNavigator::selectWorkWeek( const QDate &d )
{
  QDate firstDate;

  int dayOfWeek = KOCore::self()->calendarSystem()->dayOfTheWeek( d );

  // find the beginning of this week (could be monday or sunday)
  if( dayOfWeek == 7 ) {
    if ( KGlobal::locale()->weekStartsMonday() ) {
      firstDate = d.addDays( -6 );
    }
  } else if ( KGlobal::locale()->weekStartsMonday() ) {
    firstDate = d.addDays( dayOfWeek * -1 + 1 );
  } else {
    firstDate = d.addDays( dayOfWeek * -1 );
  }

  selectDates( firstDate, 5 );
}

void DateNavigator::selectToday()
{
  mSelectedDates.clear();
  mSelectedDates.append( QDate::currentDate() );
  
  emitSelected();
}

void DateNavigator::selectPreviousYear()
{
  DateList::Iterator it;
  for( it = mSelectedDates.begin(); it != mSelectedDates.end(); ++it ) {
    KOCore::self()->calendarSystem()->previousYearDate( *it );
  }

  emitSelected();
}

void DateNavigator::selectPreviousMonth()
{
  DateList::Iterator it;
  for( it = mSelectedDates.begin(); it != mSelectedDates.end(); ++it ) {
    KOCore::self()->calendarSystem()->previousMonthDate( *it );
  }

  emitSelected();
}

void DateNavigator::selectNextMonth()
{
  DateList::Iterator it;
  for( it = mSelectedDates.begin(); it != mSelectedDates.end(); ++it ) {
    KOCore::self()->calendarSystem()->nextMonthDate( *it );
  }

  emitSelected();
}

void DateNavigator::selectNextYear()
{
  DateList::Iterator it;
  for( it = mSelectedDates.begin(); it != mSelectedDates.end(); ++it ) {
    KOCore::self()->calendarSystem()->nextYearDate( *it );
  }

  emitSelected();
}

void DateNavigator::selectPrevious()
{
  int offset = -7;
  if ( datesCount() == 1 ) {
    offset = -1;
  }

  selectDates( mSelectedDates.first().addDays( offset ), datesCount() );
}

void DateNavigator::selectNext()
{
  int offset = 7;
  if ( datesCount() == 1 ) {
    offset = 1;
  }

  selectDates( mSelectedDates.first().addDays( offset ), datesCount() );
}

void DateNavigator::emitSelected()
{
  emit datesSelected( mSelectedDates );
}

#include "datenavigator.moc"
