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

#include <QString>

/**
  @author Jonathan Singer
*/
class Parsha
{
public:
    /**
      Find the parsha for a given day of the year.
      @p dayNumber is the day of the year.
      @p kvia and @p isLeapYear refer to the year type.
    */
    static QString findParshaName(int dayNumber, int kvia, bool isLeapYear, bool useIsraelSettings);
};
