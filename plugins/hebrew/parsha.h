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

#ifndef KORG_PLUGINS_HEBREW_PARSHA_H
#define KORG_PLUGINS_HEBREW_PARSHA_H

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
    static QString findParshaName(int dayNumber, int kvia, bool isLeapYear,
                                  bool useIsraelSettings);
};

#endif
