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
#include <KConfig>
#include <KStandardDirs>

using namespace KOrg::CalendarDecoration;

class DatenumsFactory : public DecorationFactory {
  public:
    Decoration *create() { return new Datenums; }
};

K_EXPORT_COMPONENT_FACTORY( libkorg_datenums, DatenumsFactory )


Datenums::Datenums()
  : mDisplayedInfo( DayOfYear | DaysRemaining )
{
  KConfig _config( "korganizerrc", KConfig::NoGlobals );
  KConfigGroup config(&_config, "Calendar/Datenums Plugin");
  mDisplayedInfo = (DayNumbers)config.readEntry( "DayNumbers",
                                                 int(DayOfYear | DaysRemaining) );
}

void Datenums::configure( QWidget *parent )
{
  ConfigDialog dlg( parent );
}

QString Datenums::info()
{
  return i18n("This plugin shows information on a day's position in the year.");
}

Element::List Datenums::createDayElements( const QDate &date )
{
  Element::List result;

  const KCalendarSystem *calsys = KOGlobals::self()->calendarSystem();
  int dayOfYear = calsys->dayOfYear(date);
  int remainingDays = calsys->daysInYear(date) - dayOfYear;

  StoredElement *e;
  switch ( mDisplayedInfo ) {
    case DayOfYear: // only day of year
      e = new StoredElement( QString::number( dayOfYear ) );
    case DaysRemaining: // only days until end of year
      e = new StoredElement( QString::number( remainingDays ),
                             i18np("1 day before the end of the year",
                                   "%1 days before the end of the year",
                                   remainingDays) );
      break;
    case DayOfYear + DaysRemaining: // both day of year and days till end of year
    default:
      e = new StoredElement( QString::number( dayOfYear ),
                             i18nc("dayOfYear / daysTillEndOfYear", "%1 / %2",
                                   dayOfYear, remainingDays),
                             i18np("1 day since the beginning of the year,\n",
                                   "%1 days since the beginning of the year,\n",
                                   dayOfYear)
                             + i18np("1 day until the end of the year",
                                     "%1 days until the end of the year",
                                     remainingDays) );
      break;
  }
  result.append( e );

  return result;
}
