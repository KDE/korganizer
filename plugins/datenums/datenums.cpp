/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

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
*/

#include "datenums.h"
#include "configdialog.h"
#include "koglobals.h"

#include <KCalendarSystem>

Datenums::Datenums()
  : mDisplayedInfo( DayOfYear | DaysRemaining )
{
  KConfig _config( QLatin1String("korganizerrc"), KConfig::NoGlobals );
  KConfigGroup config( &_config, "Calendar/Datenums Plugin" );
  mDisplayedInfo = (DayNumbers)config.readEntry(
    "DayNumbers", int( DayOfYear | DaysRemaining ) );
}

void Datenums::configure( QWidget *parent )
{
  ConfigDialog dlg( parent );
  dlg.exec();
}

QString Datenums::info() const
{
  return i18n( "This plugin shows information on a day's position in the year." );
}

Element::List Datenums::createDayElements( const QDate &date )
{
  Element::List result;

  const KCalendarSystem *calsys = KOGlobals::self()->calendarSystem();
  int dayOfYear = calsys->dayOfYear( date );
  int remainingDays = calsys->daysInYear( date ) - dayOfYear;

  StoredElement *e;
  switch ( mDisplayedInfo ) {
  case DayOfYear: // only day of year
    e = new StoredElement( QLatin1String("main element"), QString::number( dayOfYear ) );
    break;
  case DaysRemaining: // only days until end of year
    e = new StoredElement( QLatin1String("main element"), QString::number( remainingDays ),
                           i18np( "1 day before the end of the year",
                                  "%1 days before the end of the year",
                                  remainingDays ) );
    break;
  case DayOfYear + DaysRemaining: // both day of year and days till end of year
  default:
    e = new StoredElement( QLatin1String("main element"), QString::number( dayOfYear ),
                           i18nc( "dayOfYear / daysTillEndOfYear", "%1 / %2",
                                  dayOfYear, remainingDays ),
                           i18np( "1 day since the beginning of the year,\n",
                                  "%1 days since the beginning of the year,\n",
                                  dayOfYear ) +
                           i18np( "1 day until the end of the year",
                                  "%1 days until the end of the year",
                                  remainingDays ) );
    break;
  }
  result.append( e );

  return result;
}

Element::List Datenums::createWeekElements( const QDate &date )
{
  Element::List result;

  const KCalendarSystem *calsys = KOGlobals::self()->calendarSystem();
  int *yearOfTheWeek;
  yearOfTheWeek = 0;
  int remainingWeeks;
  const int weekOfYear = calsys->week( date, yearOfTheWeek );

  QString weekOfYearShort;
  QString weekOfYearLong;
  QString weekOfYearExtensive;
  QString remainingWeeksShort;
  QString remainingWeeksLong;
  QString remainingWeeksExtensive;
  QString weekOfYearAndRemainingWeeksShort;

   // Usual case: the week belongs to this year
  remainingWeeks = calsys->weeksInYear( date.year() ) - weekOfYear;

  weekOfYearShort = QString::number( weekOfYear );
  weekOfYearLong = i18nc( "Week weekOfYear", "Week %1", weekOfYear );
  weekOfYearExtensive = i18np( "1 week since the beginning of the year",
                               "%1 weeks since the beginning of the year",
                               weekOfYear );

  if ( yearOfTheWeek ) {  // The week does not belong to this year

    weekOfYearShort = i18nc( "weekOfYear (year)",
                             "%1 (%2)", weekOfYear, *yearOfTheWeek );
    weekOfYearLong = i18nc( "Week weekOfYear (year)",
                            "Week %1 (%2)", weekOfYear, *yearOfTheWeek );

    if ( *yearOfTheWeek == date.year() + 1 ) {
      // The week belongs to next year
      remainingWeeks = 0;

      weekOfYearExtensive = i18np( "1 week since the beginning of the year",
                                   "%1 weeks since the beginning of the year",
                                   weekOfYear );

    } else {
      // The week belongs to last year
      remainingWeeks = calsys->weeksInYear( date.year() );

      weekOfYearExtensive = i18np( "1 week since the beginning of the year",
                                   "%1 weeks since the beginning of the year",
                                   0 );
    }
  }

  remainingWeeksShort = QString::number( remainingWeeks );
  remainingWeeksShort = i18np( "1 week remaining",
                               "%1 weeks remaining",
                               remainingWeeks );
  remainingWeeksExtensive = i18np( "1 week until the end of the year",
                                   "%1 weeks until the end of the year",
                                   remainingWeeks );
  weekOfYearAndRemainingWeeksShort = i18nc( "weekOfYear / weeksTillEndOfYear",
                                            "%1 / %2", weekOfYear,
                                            remainingWeeks );

  StoredElement *e;
  switch ( mDisplayedInfo ) {
    case DayOfYear: // only week of year
      e = new StoredElement( QLatin1String("main element"), weekOfYearShort, weekOfYearLong,
                              weekOfYearExtensive );
      break;
    case DaysRemaining: // only weeks until end of year
      e = new StoredElement( QLatin1String("main element"), remainingWeeksShort,
                             remainingWeeksLong, remainingWeeksExtensive );
      break;
    case DayOfYear + DaysRemaining: // both week of year and weeks till end of year
    default:
      e = new StoredElement( QLatin1String("main element"), weekOfYearShort,
                             weekOfYearAndRemainingWeeksShort,
                             i18nc( "n weeks since the beginning of the year\n"
                                    "n weeks until the end of the year",
                                    "%1\n%2", weekOfYearExtensive,
                                    remainingWeeksExtensive ) );
      break;
  }
  result.append( e );

  return result;
}

