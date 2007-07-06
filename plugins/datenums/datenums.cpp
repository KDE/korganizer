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

#include <kconfig.h>
#include <kstandarddirs.h>
#include <kcalendarsystem.h>

using namespace KOrg::CalendarDecoration;

class DatenumsFactory : public DecorationFactory {
  public:
    Decoration *create() { return new Datenums; }
};

K_EXPORT_COMPONENT_FACTORY( libkorg_datenums, DatenumsFactory )


Datenums::Datenums()
  : mDateNums( 0 )
{
  KConfig _config( "korganizerrc", KConfig::NoGlobals );
  KConfigGroup config(&_config, "Calendar/Datenums Plugin");
  mDateNums = config.readEntry( "ShowDayNumbers", 0 );
}

void Datenums::configure(QWidget *parent)
{
  ConfigDialog *dlg = new ConfigDialog(parent);
  dlg->exec();
  delete dlg;
}

QString Datenums::info()
{
  return i18n("This plugin shows information on a day's position in the year.");
}

Element::List Datenums::createDayElements( const QDate &date )
{
  Element::List result;

  int doy = KOGlobals::self()->calendarSystem()->dayOfYear(date);
  switch (mDateNums) {
    case 1: // only days until end of year
      result.append( new StoredElement(
        QString::number(
          KOGlobals::self()->calendarSystem()->daysInYear(date) - doy ) ) );
      break;
    case 2: // both day of year and days till end of year
      result.append( new StoredElement(
        i18nc("dayOfYear / daysTillEndOfYear", "%1 / %2", doy ,
          KOGlobals::self()->calendarSystem()->daysInYear(date) - doy) ) );
      break;
    case 0: // only day of year
    default:
      result.append( new StoredElement( QString::number( doy ) ) );
  }

  return result;
}

