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
#ifndef CONVERTER_H
#define CONVERTER_H

#include <qstring.h>
#include <qstringlist.h>

struct DateResult
{
  int year;
  int month;
  int day;
  int day_of_week;

  int hebrew_month_length, secular_month_length;
  bool hebrew_leap_year_p, secular_leap_year_p;
  QString hebrew_month_name, secular_month_name;
  int kvia;
  int hebrew_day_number;
};

/**
@author Jonathan Singer
*/
class Converter
{
public:

  Converter();
  ~Converter();

  static bool hebrew_leap_year_p(int year);
  static bool gregorian_leap_year_p(int year);

  static long absolute_from_gregorian(int year, int month, int day);
  static long absolute_from_hebrew(int year, int month, int day);

  static void gregorian_from_absolute(long date, int *yearp,
                                      int *monthp, int *dayp);
  static void hebrew_from_absolute(long date, int *yearp, int *monthp,
                                   int *dayp);

  static int hebrew_months_in_year(int year);
  static int hebrew_month_length(int year, int month);
  static int secular_month_length(int year, int month);

  static long hebrew_elapsed_days(int year);
  static long hebrew_elapsed_days2(int year);
  static int hebrew_year_length(int year);

  static void finish_up(long absolute, int hyear, int hmonth,
                        int syear, int smonth,
                        struct DateResult *result);

  static void SecularToHebrewConversion(int year, int month, int day,
                                        struct DateResult *result);
  static void HebrewToSecularConversion(int year, int month, int day,
                                        struct DateResult *result);

private:

  static QStringList HebrewMonthNames;
  static QStringList SecularMonthNames;

};

#endif
