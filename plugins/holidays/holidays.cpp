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

#include <qfile.h>

#include <kapplication.h>
//#include <kconfig.h>
#include <kstandarddirs.h>

#include "configdialog.h"

#include "holidaySettings.h"
#include "holidays.h"


class HolidaysFactory : public CalendarDecorationFactory {
  public:
    CalendarDecoration *create() { return new Holidays; }
};

K_EXPORT_COMPONENT_FACTORY( libkorg_holidays, HolidaysFactory );


extern "C" {
  char *parse_holidays(const char *, int year, short force);
  /** \internal */
  struct holiday {
    char            *string;        /* name of holiday, 0=not a holiday */
    unsigned short  dup;            /* reference count */
  };
  extern struct holiday holiday[366];
}


Holidays::Holidays()
{
  HolidaySettings::self()->readConfig();
  QString holiday( HolidaySettings::holidays() );

  mHolidayFile = locate("data","korganizer/holiday_" + holiday);

  yearLast = 0;
}

Holidays::~Holidays()
{
}

QString Holidays::shortText(const QDate &date)
{
  return getHoliday(date);
}

QString Holidays::info()
{
  return i18n("This plugin provides holidays.");
}

void Holidays::configure(QWidget *parent)
{
  ConfigDialog *dlg = new ConfigDialog(parent);
  dlg->exec();
  delete dlg;
}

QString Holidays::getHoliday(const QDate &qd)
{
  //static int lastYear = 0;
  int lastYear = 0;

  if (mHolidayFile.isEmpty() || qd.isNull()) return QString::null;

  //if ((lastYear == 0) || (qd.year() != lastYear)) {
  if ((yearLast == 0) || (qd.year() != yearLast)) {
      yearLast = qd.year();
      lastYear = qd.year() - 1900; // silly parse_year takes 2 digit year...
      parse_holidays(QFile::encodeName(mHolidayFile), lastYear, 1);
  }

  if (holiday[qd.dayOfYear()-1].string) {
    QString holidayname = QString::fromUtf8(holiday[qd.dayOfYear()-1].string);
    return holidayname;
  } else {
    return QString::null;
  }
}
