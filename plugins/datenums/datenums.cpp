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
#include "koglobals.h"
#include <kconfig.h>
#include <kstandarddirs.h>

#include "configdialog.h"
#include <kcalendarsystem.h>

using namespace KOrg::CalendarDecoration;

class DatenumsFactory : public DecorationFactory {
  public:
    Decoration *create() { return new Datenums; }
};

K_EXPORT_COMPONENT_FACTORY( libkorg_datenums, DatenumsFactory )


Datenums::Datenums()
{
  KConfig _config( "korganizerrc", KConfig::NoGlobals );
  KConfigGroup config(&_config, "Calendar/Datenums Plugin");

  DatenumsAgenda* a = new DatenumsAgenda();
  agendaElements += a;
}

QString Datenums::info()
{
  return i18n("This plugin shows information on a day's position in the year.");
}

DatenumsAgenda::DatenumsAgenda()
{
  KConfig _config( "korganizerrc", KConfig::NoGlobals );
  KConfigGroup config(&_config, "Calendar/Datenums Plugin/Agenda");
  mDateNum = config.readEntry( "ShowDayNumbers", 0 );

  // TODO: read the position from the config
  m_position = DayTopT;
}

void DatenumsAgenda::configure(QWidget *parent)
{
  ConfigDialog *dlg = new ConfigDialog(parent);
  dlg->exec();
  delete dlg;
}

QString DatenumsAgenda::shortText( const QDate &date )
{
  int doy = KOGlobals::self()->calendarSystem()->dayOfYear(date);
  switch (mDateNum) {
    case 1: // only days until end of year
        return QString::number( KOGlobals::self()->calendarSystem()->daysInYear(date) - doy );
        break;
    case 2: // both day of year and days till end of year
        return i18nc("dayOfYear / daysTillEndOfYear", "%1 / %2", doy ,
                KOGlobals::self()->calendarSystem()->daysInYear(date) - doy);
        break;
    case 0: // only day of year
    default:
        return QString::number( doy );
  }
  return QString::number( doy );
}

