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
class Item;
}

class QColor;
class QDate;

// Provides static methods that are useful to all views.

namespace KOHelper
{
/**
  Returns a nice QColor for text, give the input color &c.
*/
KORGANIZERPRIVATE_EXPORT QColor getTextColor(const QColor &c);

/**
  This method returns the proper resource / subresource color for the view.
  @return The resource color for the incidence. If the incidence belongs
  to a subresource, the color for the subresource is returned (if set).
  @param calendar the calendar for which the resource color should be obtained
  @param incidence the incidence for which the color is needed (to
                   determine which  subresource needs to be used)
*/
KORGANIZERPRIVATE_EXPORT Q_REQUIRED_RESULT QColor resourceColor(const Akonadi::Item &incidence);

KORGANIZERPRIVATE_EXPORT Q_REQUIRED_RESULT QColor resourceColor(const Akonadi::Collection &collection);
KORGANIZERPRIVATE_EXPORT Q_REQUIRED_RESULT QColor resourceColorKnown(const Akonadi::Collection &collection);
KORGANIZERPRIVATE_EXPORT void setResourceColor(const Akonadi::Collection &collection, const QColor &color);

/**
  Returns the number of years between the @p start QDate and the @p end QDate
  (i.e. the difference in the year number of both dates)
*/
KORGANIZERPRIVATE_EXPORT Q_REQUIRED_RESULT int yearDiff(QDate start, QDate end);

/**
  Return true if it's the standard (that is, the current default) calendar.
*/
KORGANIZERPRIVATE_EXPORT Q_REQUIRED_RESULT bool isStandardCalendar(Akonadi::Collection::Id id);

KORGANIZERPRIVATE_EXPORT void showSaveIncidenceErrorMsg(QWidget *parent, const KCalendarCore::Incidence::Ptr &incidence);
}
