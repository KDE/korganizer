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
#include "holiday.h"
#include <klocale.h>

bool Holiday::CholP;
bool Holiday::OmerP;
bool Holiday::ParshaP;

QStringList Holiday::holidays;
int Holiday::HolidayFlags;

Holiday::Holiday()
{

}

Holiday::~Holiday()
{
}

/* Given a day of the Hebrew month, figuring out all the interesting holidays that
* correspond to that date.  ParshaP, OmerP, and CholP determine whether we should
* given info about the Parsha of the week, the Sfira, or Chol Hamoed.
*
* We are also influenced by the IsraelP flag
*/

QStringList
  Holiday::FindHoliday(int month, int day, int weekday, int kvia,
                       bool leap_year_p, bool israel_p,
                       int day_number, int year)
{

  enum
  { Sunday = 1, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday
  };

  holidays.clear();
  bool shabbat_p = (weekday == Saturday);        // Is it a Saturday?

  // Treat Adar in a non-leap year as if it were Adar II.
  if ((month == 12) && !leap_year_p)
    month = 13;
  switch (month)
      {
      case 1:                        /* Nissan */
        switch (day)
            {
            case 1:
              if (shabbat_p)
                holidays <<
                  i18n
                  ("These are Jewish holidays and mostly don't have translations. They may have different  spellings in your language; otherwise just translate the sound to your characters",
                   "Sh. HaHodesh");
              break;
            case 14:
              if (!shabbat_p)
                // If it's Shabbat, we have three pieces of info.
                // This is the least important.
                holidays << i18n("Erev Pesach");
              /* Fall thru */
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
              // The Saturday before Pesach (8th-14th)
              if (shabbat_p)
                holidays << i18n("Sh. HaGadol");
              break;
            case 15:
            case 16:
            case 21:
            case 22:
              if (!israel_p || (day == 15) || (day == 21))
                  {
                    holidays << i18n("Pesach");
                    break;
                  }
              else if (day == 22)
                break;
              /* else fall through */
            case 17:
            case 18:
            case 19:
            case 20:
              if (CholP)
                holidays << i18n("Chol Hamoed");
              break;
            case 27:
              // Yom HaShoah only exists since Israel was established.
              if (year > 1948 + 3760)
                holidays << i18n("Yom HaShoah");
              break;
            }
        if ((day > 15) && OmerP)
          // Count the Omer, starting after the first day of Pesach.
          holidays << Sfirah(day - 15);
        break;

      case 2:                        /* Iyar */
        switch (day)
            {
            case 2:
            case 3:
            case 4:
            case 5:
              // Yom HaAtzmaut is on the 5th, unless that's a Saturday, in which
              // case it is moved back two days to Thursday.  Yom HaZikaron is
              // the day before Yom HaAtzmaut.
              if (year >= 1948 + 3760)
                  {                // only after Israel established.
                    switch (weekday)
                        {
                        case Wednesday:
                          if (day == 5)
                            holidays << i18n("Yom HaAtzmaut");
                          else
                            holidays << i18n("Yom HaZikaron");
                          break;
                        case Thursday:
                          // This can't be 2 Iyar.
                          holidays << i18n("Yom HaAtzmaut");
                          break;
                        case Friday:
                        case Saturday:
                          // These are never either of them.
                          break;
                        default:
                          // All other days follow the normal rules.
                          if (day == 4)
                            holidays << i18n("Yom HaZikaron");
                          else if (day == 5)
                            holidays << i18n("Yom HaAtzmaut");
                        }
                  }
              break;
            case 28:
              // only since the 1967 war
              if (year > 1967 + 3760)
                holidays << i18n("Yom Yerushalayim");
              break;
            case 18:
              holidays << i18n("Lag BaOmer");
              break;
            }
        if ((day != 18) && OmerP)
          // Sfirah the whole month.  But Lag BaOmer is already mentioned.
          holidays << Sfirah(day + 15);

        break;

      case 3:                        /* Sivan */
        switch (day)
            {
            case 1:
            case 2:
            case 3:
            case 4:
              // Sfirah until Shavuot
              if (OmerP)
                holidays << Sfirah(day + 44);
              break;
            case 5:
              // Don't need to mention Sfira(49) if there's already two other
              // pieces of information
              if (OmerP && !shabbat_p)
                holidays << Sfirah(49);
              holidays << i18n("Erev Shavuot");
              break;
            case 6:
            case 7:
              if (!israel_p || (day == 6))
                holidays << i18n("Shavuot");
              break;
            }
        break;

      case 4:                        /* Tamuz */
        // 17th of Tamuz, except Shabbat pushes it to Sunday.
        if ((!shabbat_p && (day == 17))
            || ((weekday == 1) && (day == 18)))
          holidays << i18n("Tzom Tammuz");
        break;

      case 5:                        /* Ab */
        if (shabbat_p && (3 <= day) && (day <= 16))
          // The shabbat before and after Tisha B'Av are special
          if (day <= 9)
            holidays << i18n("Sh. Hazon");
          else
            holidays << i18n("Sh. Nahamu");
        else if ((!shabbat_p && (day == 9))
                 || ((weekday == 1) && (day == 10)))
          // 9th of Av, except Shabbat pushes it to Sunday.
          holidays << i18n("Tisha B'Av");
        break;

      case 6:                        /* Elul */
        if ((day >= 20) && (day <= 26) && shabbat_p)
          holidays << i18n("S'lichot");
        else if (day == 29)
          holidays << i18n("Erev R.H.");
        break;

      case 7:                        /* Tishrei */
        switch (day)
            {
            case 1:
            case 2:
              holidays << i18n("Rosh Hashana");
              break;
            case 3:
              if (shabbat_p)
                holidays << i18n("Sh. Shuvah");
              else
                holidays << i18n("Tzom Gedalia");
              break;
            case 4:
              if (weekday == 1)
                holidays << i18n("Tzom Gedalia");
              /* fall through */
            case 5:
            case 6:
            case 7:
            case 8:
              if (shabbat_p)
                holidays << i18n("Sh. Shuvah");
              break;
            case 9:
              holidays << i18n("Erev Y.K.");
              break;
            case 10:
              holidays << i18n("Yom Kippur");
              break;
            case 14:
              holidays << i18n("Erev Sukkot");
              break;
            case 15:
            case 16:
              if (!israel_p || (day == 15))
                  {
                    holidays << i18n("Sukkot");
                    break;
                  }
              /* else fall through */
            case 17:
            case 18:
            case 19:
            case 20:
              if (CholP)
                holidays << i18n("Chol Hamoed");
              break;
            case 21:
              holidays << i18n("Hoshana Rabah");
              break;
            case 22:
              holidays << i18n("Shmini Atzeret");
              break;
            case 23:
              if (!israel_p)
                holidays << i18n("Simchat Torah");
              break;
            }
        break;
      case 8:                        /* Cheshvan */
        break;

      case 9:                        /* Kislev */
        if (day == 24)
          holidays << i18n("Erev Hanukah");
        else if (day >= 25)
          holidays << i18n("Hanukah");
        break;

      case 10:                        /* Tevet */
        if (day <= (kvia == 0 ? 3 : 2))
          // Need to know length of Kislev to determine last day of Chanukah
          holidays << i18n("Hanukah");
        else if (((day == 10) && !shabbat_p)
                 || ((day == 11) && (weekday == 1)))
          // 10th of Tevet.  Shabbat pushes it to Sunday
          holidays << i18n("Tzom Tevet");
        break;

      case 11:                        /* Shvat */
        switch (day)
            {
              // The info for figuring out Shabbat Shirah is from the Gnu code.  I
              // assume it's correct.
//                              static char *song = i18n("Sh. Shirah";
            case 10:
              if ((kvia != 0) && shabbat_p)
                holidays << i18n("Sh. Shirah");
              break;
            case 11:
            case 12:
            case 13:
            case 14:
            case 16:
              if (shabbat_p)
                holidays << i18n("Sh. Shirah");
              break;
            case 15:
              if (shabbat_p)
                holidays << i18n("Sh. Shirah");
              holidays << i18n("Tu B'Shvat");
            case 17:
              if ((kvia == 0) && shabbat_p)
                holidays << i18n("Sh. Shirah");
              break;
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
            case 30:
              // The last shabbat on or before 1 Adar or 1 AdarII
              if (shabbat_p && !leap_year_p)
                holidays << i18n("Sh. Shekalim");
              break;
            }
        break;

      case 12:                        /* Adar I */
        if (day == 14)
          // Eat Purim Katan Candy
          holidays << i18n("Purim Katan");
        else if ((day >= 25) && shabbat_p)
          // The last shabbat on or before 1 Adar II.
          holidays << i18n("Sh. Shekalim");
        break;

      case 13:                        /* Adar II or Adar */
        switch (day)
            {
            case 1:
              if (shabbat_p)
                holidays << i18n("Sh. Shekalim");
              break;
            case 11:
            case 12:
              // Ta'anit ester is on the 13th.  But shabbat moves it back to
              // Thursday.
              if (weekday == Thursday)
                holidays << i18n("Ta'anit Ester");
              /* Fall thru */
            case 7:
            case 8:
            case 9:
            case 10:
              // The Shabbat before purim is Shabbat Zachor
              if (shabbat_p)
                holidays << i18n("Sh. Zachor");
              break;
            case 13:
              if (shabbat_p)
                holidays << i18n("Sh. Zachor");
              else
                holidays << i18n("Erev Purim");
              // It's Ta'anit Esther, unless it's a Friday or Saturday
              if (weekday < Friday)
                holidays << i18n("Ta'anit Ester");
              break;
            case 14:
              holidays << i18n("Purim");
              break;
            case 15:
              if (!shabbat_p)
                holidays << i18n("Shushan Purim");
              break;
            case 16:
              if (weekday == 1)
                holidays << i18n("Shushan Purim");
              break;
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 23:
              if (shabbat_p)
                holidays << i18n("Sh. Parah");
              break;
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
              if (shabbat_p)
                holidays << i18n("Sh. HaHodesh");
              break;
            }
        break;
      }
  if (shabbat_p && ParshaP)
    // Find the Parsha on Shabbat.
    holidays << Parsha::FindParshaName(day_number, kvia, leap_year_p,
                                       israel_p);
  return holidays;
}

/* Return a string corresponding to the nth day of the Omer */
QString Holiday::Sfirah(int day)
{
  /*static char buffer[40];
     char *endings[] = {"th", "st", "nd", "rd"};
     int remainder = day % 10;
     // 11-19 and anything not ending with 1, 2, or 3 uses -th as suffix.
     if ( ((day >= 11) && (day <= 19)) || (remainder > 3)) remainder = 0;
     sprintf(buffer, "%d%s day Omer", day, endings[remainder]);
     return buffer; */
  QString buffer;

  buffer.setNum(day);
  buffer + i18n(" Omer");        // Fix this to original function
  return buffer;

}
