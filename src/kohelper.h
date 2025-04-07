/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "korganizerprivate_export.h"

#include <Akonadi/Collection>

#include <KCalendarCore/Incidence>

namespace Akonadi
{
class Collection;
}

class QColor;
class QDate;

// Provides static methods that are useful to all views.

namespace KOHelper
{
/**
  This method returns the proper resource / subresource color for the view.
  @return The resource color for the incidence. If the incidence belongs
  to a subresource, the color for the subresource is returned (if set).
  @param calendar the calendar for which the resource color should be obtained
  @param incidence the incidence for which the color is needed (to
                   determine which  subresource needs to be used)
*/
[[nodiscard]] KORGANIZERPRIVATE_EXPORT QColor resourceColor(const Akonadi::Collection &collection);
[[nodiscard]] KORGANIZERPRIVATE_EXPORT QColor resourceColorKnown(const Akonadi::Collection &collection);
KORGANIZERPRIVATE_EXPORT void setResourceColor(const Akonadi::Collection &collection, const QColor &color);

/**
  Return true if it's the standard (that is, the current default) calendar.
*/
[[nodiscard]] KORGANIZERPRIVATE_EXPORT bool isStandardCalendar(Akonadi::Collection::Id id);

}
