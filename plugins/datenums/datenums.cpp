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

// $Id$

#include "datenums.h"
#include "koglobals.h"

#include <kcalendarsystem.h>

class DatenumsFactory : public CalendarDecorationFactory {
  public:
    CalendarDecoration *create() { return new Datenums; }
};

extern "C" {
  void *init_libkorg_datenums()
  {
    return (new DatenumsFactory);
  }
}


QString Datenums::shortText(const QDate &date)
{
  return QString::number(KOGlobals::self()->calendarSystem()->dayOfYear(date));
}

QString Datenums::info()
{
  return i18n("This plugin provides numbers of days and weeks.");
}
