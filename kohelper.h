/*
    This file is part of KOrganizer. It provides several static methods
    that are useful to all views.

    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOHELPER_H
#define KOHELPER_H

#include <kdepimmacros.h>

namespace KCal {
  class Calendar;
  class Incidence;
}

class KDE_EXPORT KOHelper
{
  public:
    /**
      This method returns the proper resource / subresource color for the
      view.
      @return The resource color for the incidence. If the incidence belongs
      to a subresource, the color for the subresource is returned (if set).
    */
    static QColor resourceColor( KCal::Calendar*calendar, KCal::Incidence*incidence );

    /**
      Returns the resource label the given incidence belongs to.
    */
    static QString resourceLabel( KCal::Calendar *calendar, KCal::Incidence *incidence );

};

#endif
