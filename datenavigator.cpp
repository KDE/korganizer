/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "datenavigator.h"

#include "koglobals.h"

#include <kcalendarsystem.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

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
    kdDebug(5850) << "DateNavigator::selectDates(QDate): an invalid date was passed as a parameter!" << endl;
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
  if ( weekDay == 1 && dateCount == 5 ) selectWorkWeek( d );
  else if ( weekStart && dateCount == 7 ) selectWeek( d );
  else selectDates( d, dateCount );
}

void DateNavigator::selectWeek()
{
  selectWeek( mSelectedDates.first() );
}

void DateNavigator::selectWeek( const QDate &d )
{
  int dayOfWeek = KOGlobals::self()->calendarSystem()->dayOfWeek( d );

  int weekStart = KGlobal::locale()->weekStartDay();

  QDate firstDate = d.addDays( weekStart - dayOfWeek );

  if ( weekStart != 1 && dayOfWeek < weekStart ) {
    firstDate = firstDate.addDays( -7 );
  }

  selectDates( firstDate, 7 );
}

void DateNavigator::selectWorkWeek()
{
  selectWorkWeek( mSelectedDates.first() );
}

void DateNavigator::selectWorkWeek( const QDate &d )
{
  int dayOfWeek = KOGlobals::self()->calendarSystem()->dayOfWeek( d );

  QDate firstDate = d.addDays( 1 - dayOfWeek );

  int weekStart = KGlobal::locale()->weekStartDay();
  if ( weekStart != 1 && dayOfWeek >= weekStart ) {
    firstDate = firstDate.addDays( 7 );
  }

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
  firstSelected = KOGlobals::self()->calendarSystem()->addYears( firstSelected, -1 );

  selectWeekByDay( weekDay, firstSelected );
}

void DateNavigator::selectPreviousMonth()
{
  QDate firstSelected = mSelectedDates.first();
  int weekDay = firstSelected.dayOfWeek();
  firstSelected = KOGlobals::self()->calendarSystem()->addMonths( firstSelected, -1 );

  selectWeekByDay( weekDay, firstSelected );
}

void DateNavigator::selectPreviousWeek()
{
  QDate firstSelected = mSelectedDates.first();
  int weekDay = firstSelected.dayOfWeek();
  firstSelected = KOGlobals::self()->calendarSystem()->addDays( firstSelected, -7 );

  selectWeekByDay( weekDay, firstSelected );
}

void DateNavigator::selectNextWeek()
{
  QDate firstSelected = mSelectedDates.first();
  int weekDay = firstSelected.dayOfWeek();

  firstSelected = KOGlobals::self()->calendarSystem()->addDays( firstSelected, 7 );

  selectWeekByDay( weekDay, firstSelected );
}

void DateNavigator::selectNextMonth()
{
  QDate firstSelected = mSelectedDates.first();
  int weekDay = firstSelected.dayOfWeek();

  firstSelected = KOGlobals::self()->calendarSystem()->addMonths( firstSelected, 1 );

  selectWeekByDay( weekDay, firstSelected );
}

void DateNavigator::selectNextYear()
{
  QDate firstSelected = mSelectedDates.first();
  int weekDay = firstSelected.dayOfWeek();
  firstSelected = KOGlobals::self()->calendarSystem()->addYears( firstSelected, 1 );

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

void DateNavigator::selectMonth(int month)
{
  QDate firstSelected = mSelectedDates.first();
  int weekDay = firstSelected.dayOfWeek();

  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
  calSys->setYMD( firstSelected, calSys->year(firstSelected), month, calSys->day(firstSelected) );

  selectWeekByDay( weekDay, firstSelected );
}

void DateNavigator::emitSelected()
{
  emit datesSelected( mSelectedDates );
}

#include "datenavigator.moc"
