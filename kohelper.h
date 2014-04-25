/*
  This file is part of KOrganizer.

  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef KORG_KOHELPER_H
#define KORG_KOHELPER_H

#include "korganizer_export.h"

#include <AkonadiCore/Entity>

#include <KCalCore/Incidence>

namespace Akonadi {
  class Collection;
  class Item;
}

class QColor;
class QDate;

// Provides static methods that are useful to all views.

namespace KOHelper {
  /**
    Returns a nice QColor for text, give the input color &c.
  */
  KORGANIZERPRIVATE_EXPORT QColor getTextColor( const QColor &c );

  /**
    This method returns the proper resource / subresource color for the view.
    @return The resource color for the incidence. If the incidence belongs
    to a subresource, the color for the subresource is returned (if set).
    @param calendar the calendar for which the resource color should be obtained
    @param incidence the incidence for which the color is needed (to
                     determine which  subresource needs to be used)
  */
  KORGANIZERPRIVATE_EXPORT QColor resourceColor( const Akonadi::Item &incidence );

  KORGANIZERPRIVATE_EXPORT QColor resourceColor( const Akonadi::Collection &collection );

  /**
    Returns the number of years between the @p start QDate and the @p end QDate
    (i.e. the difference in the year number of both dates)
  */
  KORGANIZERPRIVATE_EXPORT int yearDiff( const QDate &start, const QDate &end );

  /**
    Return true if it's the standard calendar
  */
  KORGANIZERPRIVATE_EXPORT bool isStandardCalendar( const Akonadi::Entity::Id &id );

  KORGANIZERPRIVATE_EXPORT void showSaveIncidenceErrorMsg(
    QWidget *parent, const KCalCore::Incidence::Ptr &incidence );
}

#endif
