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

void DateNavigator::selectWeekByDay( int weekDay, const QDate &d )
{
  int dateCount = mSelectedDates.count();
  bool weekStart = ( weekDay == KGlobal::locale()->weekStartDay() );
  if ( weekStart && dateCount == 5 ) selectWorkWeek( d );
  else if ( weekStart && dateCount == 7 ) selectWeek( d );
  else selectDates( d, dateCount );
}

void DateNavigator::selectWeek()
{
  selectWeek( mSelectedDates.first() );
}

void DateNavigator::selectWeek( const QDate &d )
{
  QDate firstDate;

  int dayOfWeek = KOCore::self()->calendarSystem()->dayOfTheWeek( d );

  firstDate = d.addDays( KGlobal::locale()->weekStartDay() - dayOfWeek );

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

  firstDate = d.addDays( KGlobal::locale()->weekStartDay() - dayOfWeek );

  selectDates( firstDate, 5 );
}

void DateNavigator::selectToday()
{
  QDate d = QDate::currentDate();

  int dateCount = mSelectedDates.count();

  if ( dateCount == 5 ) selectWorkWeek( d );
  else if ( dateCount == 7 ) selectWeek( d );
  else selectDates( d, dateCount );
}

void DateNavigator::selectPreviousYear()
{
  QDate firstSelected = mSelectedDates.first();
  int weekDay = firstSelected.dayOfWeek();
  KOCore::self()->calendarSystem()->previousYearDate( firstSelected );

  selectWeekByDay( weekDay, firstSelected );
}

void DateNavigator::selectPreviousMonth()
{
  QDate firstSelected = mSelectedDates.first();
  int weekDay = firstSelected.dayOfWeek();
  KOCore::self()->calendarSystem()->previousMonthDate( firstSelected );

  selectWeekByDay( weekDay, firstSelected );
}

void DateNavigator::selectNextMonth()
{
  QDate firstSelected = mSelectedDates.first();
  int weekDay = firstSelected.dayOfWeek();

  KOCore::self()->calendarSystem()->nextMonthDate( firstSelected );

  selectWeekByDay( weekDay, firstSelected );
}

void DateNavigator::selectNextYear()
{
  QDate firstSelected = mSelectedDates.first();
  int weekDay = firstSelected.dayOfWeek();
  KOCore::self()->calendarSystem()->nextYearDate( firstSelected );

  selectWeekByDay( weekDay, firstSelected );
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
