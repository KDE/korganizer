/***************************************************************************
 *   Copyright (C) 2003 by Jonathan Singer                                             *
 *   jsinger@leeta.net                                                                                *
 *   Calendar routines from Hebrew Calendar by Frank Yellin                     *
 *                                                                                                             *
 *   This program is free software; you can redistribute it and/or modify      *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or      *
 *   (at your option) any later version.                                                       *
 ***************************************************************************/
#ifndef HOLIDAY_H
#define HOLIDAY_H

#include <qstring.h>
#include <qstringlist.h>
#include <parsha.h>
/**
@author Jonathan Singer
*/
class Holiday
{
public:

  Holiday();
  ~Holiday();

  static QStringList FindHoliday(int month, int day, int weekday,
                                 int kvia, bool leap_year_p,
                                 bool israel_p, int day_number,
                                 int year);

  static QString Sfirah(int);

  static bool CholP;
  static bool OmerP;
  static bool ParshaP;

private:

  static QStringList holidays;
  static int HolidayFlags;        //supposed to be extern

//parsha Parsha_lookup;
};

#endif
