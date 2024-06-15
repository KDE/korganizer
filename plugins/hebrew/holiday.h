/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2003 Jonathan Singer <jsinger@leeta.net>
  SPDX-FileCopyrightText: 2007 Loïc Corbasson <loic.corbasson@gmail.com>

  Calendar routines from Hebrew Calendar by Frank Yellin.
  Based on some GNU Emacs code (lisp/calendar/cal-hebrew.el),
  SPDX-FileCopyrightText: 1995, 1997 Free Software Foundation, Inc.
  SPDX-FileContributor: Nachum Dershowitz <nachum@cs.uiuc.edu>
  SPDX-FileContributor: Edward M. Reingold <reingold@cs.uiuc.edu>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

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
    static QStringList findHoliday(const HebrewDate &hd, bool useIsraelSettings, bool showParsha, bool showChol, bool showOmer);

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
        AdarI = 12,
    };

    static QStringList findHoliday(int month,
                                   int day,
                                   int weekday,
                                   int kvia,
                                   bool leap_year_p,
                                   bool useIsraelSettings,
                                   int day_number,
                                   int year,
                                   bool showParsha,
                                   bool showChol,
                                   bool showOmer);
};
