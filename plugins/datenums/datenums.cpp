/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
*/

#include "datenums.h"
#include "koglobals.h"
#include <kconfig.h>
#include <kstandarddirs.h>

#include "configdialog.h"
#include <kcalendarsystem.h>

class DatenumsFactory : public CalendarDecorationFactory {
  public:
    CalendarDecoration *create() { return new Datenums; }
};

K_EXPORT_COMPONENT_FACTORY( libkorg_datenums, DatenumsFactory )


Datenums::Datenums()
{
  KConfig config( "korganizerrc", true, false); // Open read-only, no kdeglobals
  config.setGroup("Calendar/DateNum Plugin");
  mDateNum = config.readNumEntry( "ShowDayNumbers", 0 );
}

void Datenums::configure(QWidget *parent)
{
  ConfigDialog *dlg = new ConfigDialog(parent);
  dlg->exec();
  delete dlg;
}


QString Datenums::shortText(const QDate &date)
{
  int doy = KOGlobals::self()->calendarSystem()->dayOfYear(date);
  switch (mDateNum) {
    case 1: // only days until end of year
        return QString::number( KOGlobals::self()->calendarSystem()->daysInYear(date) - doy );
        break;
    case 2: // both day of year and days till end of year
        return i18n("dayOfYear / daysTillEndOfYear", "%1 / %2").arg( doy )
               .arg(KOGlobals::self()->calendarSystem()->daysInYear(date) - doy);
        break;
    case 0: // only day of year
    default:
        return QString::number( doy );
  }
  return QString::number( doy );
}

QString Datenums::info()
{
  return i18n("This plugin provides numbers of days and weeks.");
}
