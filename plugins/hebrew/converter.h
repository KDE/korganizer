/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Jonathan Singer <jsinger@leeta.net>
  Copyright (C) 2007 Loïc Corbasson <loic.corbasson@gmail.com>
  Calendar routines from Hebrew Calendar by Frank Yellin.
  Copyright (C) 1994-2006 Danny Sadinoff <danny@sadinoff.com>.

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
#ifndef HEBREW_CONVERTER_H
#define HEBREW_CONVERTER_H

#include <QString>
#include <QStringList>

struct DateResult
{
  int year;
  int month;
  int day;
  int day_of_week;

  int hebrew_month_length, secular_month_length;
  bool hebrew_leap_year_p, secular_leap_year_p;
  int kvia;
  int hebrew_day_number;
};

/**
  This class converts dates between the Hebrew and Gregorian (secular)
  calendars.

  @author Loïc Corbasson
 */
class HebrewDate
{
  public:
    HebrewDate( struct DateResult );
    ~HebrewDate();

    static HebrewDate fromSecular( int year, int month, int day );
    static HebrewDate fromHebrew( int year, int month, int day );

    int year();
    int month();
    int day();
    int dayOfWeek();

    int hebrewMonthLength();
    int secularMonthLength();
    bool isOnHebrewLeapYear();
    bool isOnSecularLeapYear();
    int kvia();
    int hebrewDayNumber();

  private:
    int mYear, mMonth, mDay, mDayOfWeek;
    int mHebrewMonthLength, mSecularMonthLength;
    bool mOnHebrewLeapYear, mOnSecularLeapYear;
    int mKvia, mHebrewDayNumber;
};

/**
  This class is used internally to convert dates between the Hebrew and
  Gregorian (secular) calendars.

  Calendar routines from Hebrew Calendar by Frank Yellin.

  For more information, see “The Comprehensive Hebrew Calendar” by Arthur Spier
  and “Calendrical Calculations” by E. M. Reingold and Nachum Dershowitz,
  or the documentation of Remind by Roaring Penguin Software Inc.

  @author Jonathan Singer
*/
class Converter
{
  friend class HebrewDate;

  public:
    enum HebrewMonths {
      Nissan = 1,
      Iyar,
      Sivan,
      Tamuz,
      Ab,
      Elul,
      Tishrei,
      Cheshvan,
      Kislev,
      Tevet,
      Shvat,
      Adar,
      AdarII,
      AdarI = 12
    };

    enum SecularMonths {
      January = 1,
      February,
      March,
      April,
      May,
      June,
      July,
      August,
      September,
      October,
      November,
      December
    };

  private:
    static bool hebrew_leap_year_p( int year );
    static bool gregorian_leap_year_p( int year );

    static long absolute_from_gregorian( int year, int month, int day );
    static long absolute_from_hebrew( int year, int month, int day );

    static void gregorian_from_absolute( long date, int *yearp,
                                         int *monthp, int *dayp );
    static void hebrew_from_absolute( long date, int *yearp, int *monthp,
                                      int *dayp);

    static int hebrew_months_in_year( int year );
    static int hebrew_month_length( int year, int month );
    static int secular_month_length( int year, int month );

    static long hebrew_elapsed_days( int year );
    static long hebrew_elapsed_days2( int year );
    static int hebrew_year_length( int year );

    static void finish_up( long absolute, int hyear, int hmonth,
                           int syear, int smonth,
                           struct DateResult *result );

    static void secularToHebrewConversion( int year, int month, int day,
                                           struct DateResult *result );
    static void hebrewToSecularConversion( int year, int month, int day,
                                           struct DateResult *result );
};

#endif
