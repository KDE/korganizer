/*
    This file is part of KOrganizer.
    Copyright (c) 2003 Jonathan Singer <jsinger@leeta.net>

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

#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kcalendarsystem.h>
#include <kcalendarsystemfactory.h>
#include "hebrew.h"
#include "configdialog.h"
#include "parsha.h"
#include "converter.h"
#include "holiday.h"

bool Hebrew::IsraelP;

class HebrewFactory:public CalendarDecorationFactory
{
public:
  CalendarDecoration * create()
  {
    return new Hebrew;
  }
};

K_EXPORT_COMPONENT_FACTORY( libkorg_hebrew, HebrewFactory );


QString Hebrew::shortText(const QDate & date)
{

  KConfig config(locateLocal("config", "korganizerrc"));

  config.setGroup("Calendar/Hebrew Calendar Plugin");
  IsraelP =
    config.readBoolEntry("Israel",
                         (KGlobal::locale()->country() == ".il"));
  Holiday::ParshaP = config.readBoolEntry("Parsha", true);
  Holiday::CholP = config.readBoolEntry("Chol_HaMoed", true);
  Holiday::OmerP = config.readBoolEntry("Omer", true);
  QString *label_text = new QString();

  int day = date.day();
  int month = date.month();
  int year = date.year();

  // core calculations!!
  struct DateResult result;

  Converter::SecularToHebrewConversion(year, month, day, /*0, */
                                       &result);
  int hebrew_day = result.day;
  int hebrew_month = result.month;
  int hebrew_year = result.year;
  int hebrew_day_of_week = result.day_of_week;
  bool hebrew_leap_year_p = result.hebrew_leap_year_p;
  int hebrew_kvia = result.kvia;
  int hebrew_day_number = result.hebrew_day_number;

  QStringList holidays =
    Holiday::FindHoliday(hebrew_month, hebrew_day,
                         hebrew_day_of_week + 1, hebrew_kvia,
                         hebrew_leap_year_p, IsraelP,
                         hebrew_day_number, hebrew_year);

  KCalendarSystem *cal = KCalendarSystemFactory::create("hebrew");
  *label_text = QString("%1 %2").arg(cal->dayString(date, false))
                                .arg(cal->monthName(date));

  if (holidays.count())
      {
        int count = holidays.count();

        for (int h = 0; h <= count; ++h)
            {
              *label_text += "\n" + holidays[h];
            }
      }

  return *label_text;
}

QString Hebrew::info()
{
  return
    i18n("This plugin provides the date in the Jewish calendar.");
}

void Hebrew::configure(QWidget * parent)
{
  ConfigDialog *dlg = new ConfigDialog(parent);        //parent?

  dlg->exec();
}
