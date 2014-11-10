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

#ifndef KORG_PLUGINS_HEBREW_HOLIDAY_H
#define KORG_PLUGINS_HEBREW_HOLIDAY_H

#include "converter.h"

#include <QStringList>

/**
  @author Jonathan Singer
*/
class Holiday
{
public:
    /**
      Given a day of a Hebrew month, figures out all the interesting holidays
      that correspond to that date.
      @p showParsha, @p showOmer, and @p showChol determine respectively whether
      we should give information about the Parsha of the week, the Sfira, and
      Chol Hamoed.

      We are also influenced by the @p useIsraelSettings flag, which determines
      whether we will use the settings corresponding to Israel or to the
      diaspora.
    */
    static QStringList findHoliday(HebrewDate hd, bool useIsraelSettings,
                                   bool showParsha, bool showChol,
                                   bool showOmer);

private:
    /**
      Return a string corresponding to the nth day of the Omer (the seven weeks
      from the end of Passover to Shavuot).
    */
    static QString sfirah(int);

    enum HebrewMonths {
        Nissan = 1,
        Iyar = 2,
        Sivan = 3,
        Tamuz = 4,
        Ab = 5,
        Elul = 6,
        Tishrei = 7,
        Cheshvan = 8,
        Kislev = 9,
        Tevet = 10,
        Shvat = 11,
        Adar = 12,
        AdarII = 13,
        AdarI = 12
    };

    static QStringList findHoliday(int month, int day, int weekday,
                                   int kvia, bool leap_year_p,
                                   bool useIsraelSettings, int day_number,
                                   int year, bool showParsha, bool showChol,
                                   bool showOmer);
};

#endif
