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
#include "converter.h"
#include <klocale.h>

Converter::Converter()
{

}

Converter::~Converter()
{
}

long Converter::absolute_from_gregorian(int year, int month, int day)
{
  int xyear, day_number;

  xyear = year - 1;
  day_number = day + 31 * (month - 1);
  if (month > 2)
      {
        day_number -= (23 + (4 * month)) / 10;
        if (gregorian_leap_year_p(year))
          day_number++;
      }
  return day_number +     /* the day number within the current year */
    365L * xyear +        /* days in prior years */
    (xyear / 4) +         /* Julian leap years */
    (-(xyear / 100)) +    /* deduct century years */
    (xyear / 400);        /* add Gregorian leap years */
}

/* Given a Hebrew date, calculate the number of days since
 * January 0, 0001, Gregorian
 */
long Converter::absolute_from_hebrew(int year, int month, int day)
{
  long sum = day + hebrew_elapsed_days(year) - 1373429L;
  int i;

  if (month < 7)
      {
        int months = hebrew_months_in_year(year);

        for (i = 7; i <= months; ++i)
          sum += hebrew_month_length(year, i);
        for (i = 1; i < month; ++i)
          sum += hebrew_month_length(year, i);
      }
  else
      {
        for (i = 7; i < month; ++i)
          sum += hebrew_month_length(year, i);
      }
  return sum;
}

/* Given an absolute date, calculate the gregorian date  */
void
  Converter::gregorian_from_absolute(long date, int *yearp,
                                     int *monthp, int *dayp)
{
  int year, month, day;

  for (year = date / 366;
       date >= absolute_from_gregorian(year + 1, 1, 1); ++year) ;
  for (month = 1;
       (month <= 11)
       && (date >= absolute_from_gregorian(year, 1 + month, 1));
       ++month ) ;
  day = 1 + date - absolute_from_gregorian(year, month, 1);
  *yearp = year;
  *monthp = month;
  *dayp = day;
}

/* Given an absolute date, calculate the Hebrew date */
void
  Converter::hebrew_from_absolute(long date, int *yearp, int *monthp,
                                  int *dayp)
{
  int year, month, day, gyear, gmonth, gday, months;

  gregorian_from_absolute(date, &gyear, &gmonth, &gday);
  year = gyear + 3760;
  while (date >= absolute_from_hebrew(1 + year, 7, 1))
    year++;
  months = hebrew_months_in_year(year);
  for (month = 7;
       date > absolute_from_hebrew(year, month,
                                   hebrew_month_length(year, month));
       month = 1 + (month % months)) ;
  day = 1 + date - absolute_from_hebrew(year, month, 1);
  *yearp = year;
  *monthp = month;
  *dayp = day;
}

/* Number of months in a Hebrew year */
int Converter::hebrew_months_in_year(int year)
{
  if (hebrew_leap_year_p(year))
    return 13;
  else
    return 12;
}

enum
{ Nissan =
    1, Iyar, Sivan, Tamuz, Ab, Elul, Tishrei, Cheshvan, Kislev, Tevet,
  Shvat, Adar, AdarII, AdarI = 12
};

enum
{ January =
    1, February, March, April, May, June, July, August, September,
  October, November, December
};

/* Number of days in a Hebrew month */
int Converter::hebrew_month_length(int year, int month)
{
  switch (month)
      {
      case Tishrei:
      case Shvat:
      case Nissan:
      case Sivan:
      case Ab:
        return 30;

      case Tevet:
      case Iyar:
      case Tamuz:
      case Elul:
      case AdarII:
        return 29;

      case Cheshvan:
        // 29 days, unless it's a long year.
        if ((hebrew_year_length(year) % 10) == 5)
          return 30;
        else
          return 29;

      case Kislev:
        // 30 days, unless it's a short year.
        if ((hebrew_year_length(year) % 10) == 3)
          return 29;
        else
          return 30;

      case Adar:
        // Adar (non-leap year) has 29 days.  Adar I has 30 days.
        if (hebrew_leap_year_p(year))
          return 30;
        else
          return 29;

      default:
        return 0;
      }
}

/* Number of days in a Julian or gregorian month */
int
  Converter::secular_month_length(int year,
                                  int month /*, bool julianp */ )
{
  switch (month)
      {
      case January:
      case March:
      case May:
      case July:
      case August:
      case October:
      case December:
        return 31;
      case April:
      case June:
      case September:
      case November:
        return 30;
      case February:
        if (gregorian_leap_year_p(year))
          return 29;
        else
          return 28;
      default:
        return 0;
      }
}

/* Is it a Leap year in the gregorian calendar */
bool Converter::gregorian_leap_year_p(int year)
{
  if ((year % 4) != 0)
    return 0;
  if ((year % 400) == 0)
    return 1;
  if ((year % 100) == 0)
    return 0;
  return 1;
}

/* Is it a leap year in the Jewish Calendar */
bool Converter::hebrew_leap_year_p(int year)
{
  switch (year % 19)
      {
      case 0:
      case 3:
      case 6:
      case 8:
      case 11:
      case 14:
      case 17:
        return 1;
      default:
        return 0;
      }
}

/* Return the number of days from 1 Tishrei 0001 to the beginning of the given year.
 * Since this routine gets called frequently with the same year arguments, we cache
 * the most recent values.
 */
#define MEMORY 5
long Converter::hebrew_elapsed_days(int year)
{
  static int saved_year[MEMORY] = { -1, -1, -1, -1, -1 };
  static long saved_value[MEMORY];
  int i;

  for (i = 0; i < MEMORY; ++i)
    if (year == saved_year[i])
      return saved_value[i];
  for (i = 0; i < MEMORY; ++i)
    saved_year[i] = saved_year[1 + i], saved_value[i] =
      saved_value[1 + i];
  saved_year[MEMORY - 1] = year;
  saved_value[MEMORY - 1] = hebrew_elapsed_days2(year);
  return saved_value[MEMORY - 1];
}

long Converter::hebrew_elapsed_days2(int year)
{
  long prev_year = year - 1;
  long months_elapsed = 235L * (prev_year / 19)  /* months in complete cycles so far */
    + 12L * (prev_year % 19)                     /* regular months in this cycle */
    + (((prev_year % 19) * 7 + 1) / 19);         /* leap months this cycle */
  long parts_elapsed = 5604 + 13753 * months_elapsed;
  long day = 1 + 29 * months_elapsed + parts_elapsed / 25920;
  long parts = parts_elapsed % 25920;
  int weekday = (day % 7);
  long alt_day = ((parts >= 19440)
                  || (weekday == 2 && (parts >= 9924)
                      && !hebrew_leap_year_p(year)) || (weekday == 1
                                                        && (parts >=
                                                            16789)
                                                        &&
                                                        hebrew_leap_year_p
                                                        (prev_year)))
    ? day + 1 : day;
  switch (alt_day % 7)
      {
      case 0:
      case 3:
      case 5:
        return 1 + alt_day;
      default:
        return alt_day;
      }
}

/* Number of days in the given Hebrew year */
int Converter::hebrew_year_length(int year)
{
  return hebrew_elapsed_days(1 + year) - hebrew_elapsed_days(year);
}

/* Fill in the DateResult structure based on the given secular date */
void
  Converter::SecularToHebrewConversion(int syear, int smonth,
                                       int sday,
                                       struct DateResult *result)
{
  int hyear, hmonth, hday;
  long absolute;

  absolute = absolute_from_gregorian(syear, smonth, sday);

  hebrew_from_absolute(absolute, &hyear, &hmonth, &hday);

  result->year = hyear;
  result->month = hmonth;
  result->day = hday;
  finish_up(absolute, hyear, hmonth, syear, smonth, result);
}

/* Fill in the DateResult structure based on the given Hebrew date */
void
  Converter::HebrewToSecularConversion(int hyear, int hmonth,
                                       int hday,
                                       struct DateResult *result)
{
  int syear, smonth, sday;
  long absolute;

  absolute = absolute_from_hebrew(hyear, hmonth, hday);
  gregorian_from_absolute(absolute, &syear, &smonth, &sday);
  result->year = hyear;
  result->month = hmonth;
  result->day = hday;
  finish_up(absolute, hyear, hmonth, syear, smonth, result);
}

/* This is common code for filling up the DateResult structure */
void
  Converter::finish_up(long absolute, int hyear, int hmonth,
                       int syear, int smonth,
                       struct DateResult *result)
{
  result->hebrew_month_length = hebrew_month_length(hyear, hmonth);
  result->secular_month_length = secular_month_length(syear, smonth);
  result->hebrew_leap_year_p = hebrew_leap_year_p(hyear);
  result->secular_leap_year_p = gregorian_leap_year_p(syear);
  result->kvia = (hebrew_year_length(hyear) % 10) - 3;
  // absolute is -1 on 1/1/0001 Julian
  result->day_of_week = (7 + absolute) % 7;
  result->hebrew_day_number =
    absolute - absolute_from_hebrew(hyear, 7, 1) + 1;

}
