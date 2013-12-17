/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Jonathan Singer <jsinger@leeta.net>
  Copyright (C) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

  Calendar routines from Hebrew Calendar by Frank Yellin.
  Based on some GNU Emacs code (lisp/calendar/cal-hebrew.el),
  copyright (C) 1995, 1997 Free Software Foundation, Inc.,
  authors: Nachum Dershowitz <nachum@cs.uiuc.edu>
           Edward M. Reingold <reingold@cs.uiuc.edu>

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

#include "holiday.h"
#include "parsha.h"

#include <KLocalizedString>

QStringList Holiday::findHoliday( HebrewDate hd, bool useIsraelSettings,
                                  bool showParsha, bool showChol,
                                  bool showOmer )
{
  return findHoliday( hd.month(), hd.day(), hd.dayOfWeek() + 1, hd.kvia(),
                      hd.isOnHebrewLeapYear(), useIsraelSettings,
                      hd.hebrewDayNumber(), hd.year(),
                      showParsha, showChol, showOmer );
}

QStringList Holiday::findHoliday( int month, int day, int weekday, int kvia,
                                  bool isLeapYear, bool useIsraelSettings,
                                  int dayNumber, int year,
                                  bool showParsha, bool showChol,
                                  bool showOmer )
{
  enum {
    Sunday = 1,
    Monday,
    Tuesday,
    Wednesday,
    Thursday,
    Friday,
    Saturday
  };

  QStringList holidays;
  bool isAShabbat = ( weekday == Saturday );

  // Treat Adar in a non-leap year as if it were Adar II.
  if ( ( month == Adar ) && !isLeapYear ) {
    month = AdarII;
  }
  switch ( month ) {
  case Nissan:
    switch ( day ) {
    case 1:
      if ( isAShabbat ) {
        holidays << i18nc( "These are Jewish holidays and mostly do not "
                           "have translations. They may have different "
                           "spellings in your language; otherwise, just "
                           "translate the sound to your characters.",
                           "Sh. HaHodesh" );
      }
      break;
    case 14:
      if ( !isAShabbat ) {
        // If it's Shabbat, we have three pieces of info.
        // This is the least important, so we skip it on Shabbat as we only
        // really want the two most important ones.
        holidays << i18n( "Erev Pesach" );
      }
      /* fall through */
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
      // The Saturday before Pesach (8th-14th)
      if ( isAShabbat ) {
        holidays << i18n( "Sh. HaGadol" );
      }
      break;
    case 15:
    case 16:
    case 21:
    case 22:
      if ( !useIsraelSettings || ( day == 15 ) || ( day == 21 ) ) {
        holidays << i18n( "Pesach" );
        break;
      } else if ( day == 22 ) {
        break;
      }
      /* else fall through */
    case 17:
    case 18:
    case 19:
    case 20:
      if ( showChol ) {
        holidays << i18n( "Chol Hamoed" );
      }
      break;
    case 26:
    case 27:
    case 28:
      // Yom HaShoah only exists since Israel was established.
      if ( year > 1948 + 3760 ) {
        switch( weekday ) {
        case Thursday:
          if ( day == 26 || day == 27 ) {
            holidays << i18n( "Yom HaShoah" );
          }
          break;
        case Monday:
          if ( day == 28 || day == 27 ) {
            holidays << i18n( "Yom HaShoah" );
          }
          break;
        case Sunday:
        case Friday:
          // These are never either of them.
          break;
        default:
          if ( day == 27 ) {
            holidays << i18n( "Yom HaShoah" );
          }
          break;
        }
      }
      break;
    }
    if ( ( day > 15 ) && showOmer ) {
      // Count the Omer, starting after the first day of Pesach.
      holidays << sfirah( day - 15 );
    }
    break;

  case Iyar:
    switch ( day ) {
    case 2:
    case 3:
    case 4:
    case 5:
      // Yom HaAtzmaut is on the 5th, unless that's a Saturday, in which
      // case it is moved back two days to Thursday. Yom HaZikaron is the
      // day before Yom HaAtzmaut.
      if ( year >= 1948 + 3760 ) { // only after Israel was established
        switch ( weekday ) {
        case Wednesday:
          if ( day == 5 ) {
            holidays << i18n( "Yom HaAtzmaut" );
          } else {
            holidays << i18n( "Yom HaZikaron" );
          }
          break;
        case Thursday:
          // This can't be 2 Iyar.
          holidays << i18n( "Yom HaAtzmaut" );
          break;
        case Friday:
        case Saturday:
          // These are never either of them.
          break;
        default:
          // All other days follow the normal rules.
          if ( day == 4 ) {
            holidays << i18n( "Yom HaZikaron" );
          } else if ( day == 5 ) {
            holidays << i18n( "Yom HaAtzmaut" );
          }
        }
      }
      break;
    case 28:
      if ( year > 1967 + 3760 ) {
        // only since the 1967 war
        holidays << i18n( "Yom Yerushalayim" );
      }
      break;
    case 18:
      holidays << i18n( "Lag BaOmer" );
      break;
    }
    if ( ( day != 18 ) && showOmer ) {
      // Sfirah the whole month, Lag BaOmer is already mentioned.
      holidays << sfirah( day + 15 );
    }
    break;

  case Sivan:
    switch ( day ) {
    case 1:
    case 2:
    case 3:
    case 4:
      // Sfirah until Shavuot
      if ( showOmer ) {
        holidays << sfirah( day + 44 );
      }
      break;
    case 5:
      // Don't need to mention Sfira(49) if there's already two other and
      // more important pieces of information.
      if ( showOmer && !isAShabbat ) {
        holidays << sfirah( 49 );
      }
      holidays << i18n( "Erev Shavuot" );
      break;
    case 6:; case 7:
      if ( !useIsraelSettings || ( day == 6 ) ) {
        holidays << i18n( "Shavuot" );
      }
      break;
    }
    break;

  case Tamuz:
    // 17th of Tamuz, except Shabbat pushes it to Sunday.
    if ( ( !isAShabbat && ( day == 17 ) ) ||
         ( ( weekday == Sunday ) && ( day == 18 ) ) ) {
      holidays << i18n( "Tzom Tammuz" );
    }
    break;

  case Ab:
    if ( isAShabbat && ( 3 <= day ) && ( day <= 16 ) ) {
      // The shabbat before and after Tisha B'Av are special
      if ( day <= 9 ) {
        holidays << i18n( "Sh. Hazon" );
      } else {
        holidays << i18n( "Sh. Nahamu" );
      }
    } else if ( ( !isAShabbat && ( day == 9 ) ) ||
                ( ( weekday == Sunday ) && ( day == 10 ) ) ) {
      // 9th of Av, except Shabbat pushes it to Sunday.
      holidays << i18n( "Tisha B'Av" );
    }
    break;

  case Elul:
    if ( ( day >= 20 ) && ( day <= 26 ) && isAShabbat ) {
      holidays << i18n( "S'lichot" );
    } else if ( day == 29 ) {
      holidays << i18n( "Erev R.H." );
    }
    break;

  case Tishrei:
    switch ( day ) {
    case 1:; case 2:
      holidays << i18n( "Rosh Hashana" );
      break;
    case 3:
      if ( isAShabbat ) {
        holidays << i18n( "Sh. Shuvah" );
      } else {
        holidays << i18n( "Tzom Gedalia" );
      }
      break;
    case 4:
      if ( weekday == Sunday ) {
        holidays << i18n( "Tzom Gedalia" );
      }
      /* fall through */
    case 5:
    case 6:
    case 7:
    case 8:
      if ( isAShabbat ) {
        holidays << i18n( "Sh. Shuvah" );
      }
      break;
    case 9:
      holidays << i18n( "Erev Y.K." );
      break;
    case 10:
      holidays << i18n( "Yom Kippur" );
      break;
    case 14:
      holidays << i18n( "Erev Sukkot" );
      break;
    case 15:
    case 16:
      if ( !useIsraelSettings || ( day == 15 ) ) {
        holidays << i18n( "Sukkot" );
        break;
      }
      /* else fall through */
    case 17:
    case 18:
    case 19:
    case 20:
      if ( showChol ) {
        holidays << i18n( "Chol Hamoed" );
      }
      break;
    case 21:
      holidays << i18n( "Hoshana Rabah" );
      break;
    case 22:
      holidays << i18n( "Shmini Atzeret" );
      break;
    case 23:
      if ( !useIsraelSettings ) {
        holidays << i18n( "Simchat Torah" );
      }
      break;
    }
    break;

  case Cheshvan:
    break;

  case Kislev:
    if ( day == 24 ) {
      holidays << i18n( "Erev Hanukah" );
    } else if ( day >= 25 ) {
      holidays << i18n( "Hanukah" );
    }
    break;

  case Tevet:
    if ( day <= ( kvia == 0 ? 3 : 2 ) ) {
      // We need to know the length of Kislev to determine the last day of
      // Chanukah.
      holidays << i18n( "Hanukah" );
    } else if ( ( ( day == 10 ) && !isAShabbat ) ||
                ( ( day == 11 ) && ( weekday == Sunday ) ) ) {
      // 10th of Tevet; Shabbat pushes it to Sunday.
      holidays << i18n( "Tzom Tevet" );
    }
    break;

  case Shvat:
    switch ( day ) {
      // The info for figuring out Shabbat Shirah is from the Gnu code. I
      // assume it's correct.
    case 10:
      if ( ( kvia != 0 ) && isAShabbat ) {
        holidays << i18n( "Sh. Shirah" );
      }
      break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 16:
      if ( isAShabbat ) {
        holidays << i18n( "Sh. Shirah" );
      }
      break;
    case 15:
      if ( isAShabbat ) {
        holidays << i18n( "Sh. Shirah" );
      }
      holidays << i18n( "Tu B'Shvat" );
      break;
    case 17:
      if ( ( kvia == 0 ) && isAShabbat ) {
        holidays << i18n( "Sh. Shirah" );
      }
      break;
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
      // The last shabbat on or before 1 Adar or 1 AdarII
      if ( isAShabbat && !isLeapYear ) {
        holidays << i18n( "Sh. Shekalim" );
      }
      break;
    }
    break;

  case AdarI:
    if ( day == 14 ) {
      // Eat Purim Katan Candy
      holidays << i18n( "Purim Katan" );
    } else if ( ( day >= 25 ) && isAShabbat ) {
      // The last shabbat on or before 1 Adar II.
      holidays << i18n( "Sh. Shekalim" );
    }
    break;

  case AdarII: /* Adar II or non-leap year Adar */
    switch (day) {
    case 1:
      if ( isAShabbat ) {
        holidays << i18n( "Sh. Shekalim" );
      }
      break;
    case 11:
    case 12:
      // Ta'anit ester is on the 13th.  But shabbat moves it back to
      // Thursday.
      if ( weekday == Thursday ) {
        holidays << i18n( "Ta'anit Ester" );
      }
      /* fall through */
    case 7:
    case 8:
    case 9:
    case 10:
      // The Shabbat before purim is Shabbat Zachor
      if ( isAShabbat ) {
        holidays << i18n( "Sh. Zachor" );
      }
      break;
    case 13:
      if ( isAShabbat ) {
        holidays << i18n( "Sh. Zachor" );
      } else {
        holidays << i18n( "Erev Purim" );
      }
      // It's Ta'anit Esther, unless it's a Friday or Saturday
      if ( weekday < Friday ) {
        holidays << i18n( "Ta'anit Ester" );
      }
      break;
    case 14:
      holidays << i18n( "Purim" );
      break;
    case 15:
      if ( !isAShabbat ) {
        holidays << i18n( "Shushan Purim" );
      }
      break;
    case 16:
      if ( weekday == Sunday ) {
        holidays << i18n( "Shushan Purim" );
      }
      break;
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
      if ( isAShabbat ) {
        holidays << i18n( "Sh. Parah" );
      }
      break;
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
      if ( isAShabbat ) {
        holidays << i18n( "Sh. HaHodesh" );
      }
      break;
    }
    break;
  }
  if ( isAShabbat && showParsha ) {
    // Find the Parsha on Shabbat.
    holidays << Parsha::findParshaName( dayNumber, kvia, isLeapYear,
                                        useIsraelSettings );
  }
  return holidays;
}

QString Holiday::sfirah( int day )
{
  QString buffer = QString::number( day );
  buffer + i18n( " Omer" ); // TODO: Find a way to write 1st instead of 1,
                            //                           2nd instead of 2, etc.
  return buffer;
}
