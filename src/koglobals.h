/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once

#include "korganizerprivate_export.h"

#include <QDate>
#include <QList>
#include <QMap>
#include <QString>

namespace KHolidays
{
class HolidayRegion;
}

class KORGANIZERPRIVATE_EXPORT KOGlobals
{
    friend class KOGlobalsSingletonPrivate;

public:
    static KOGlobals *self();

    static bool reverseLayout();

    ~KOGlobals();

    [[nodiscard]] QMap<QDate, QStringList> holiday(const QDate &start, const QDate &end);

    [[nodiscard]] int firstDayOfWeek() const;

    [[nodiscard]] int getWorkWeekMask();

    /**
       Set which holiday regions the user wants to use.
       @param regions a list of Holiday Regions strings.
    */
    void setHolidays(const QStringList &regions);

    /**
     * Returns the HolidayRegion object or 0 if none has been defined
     */
    [[nodiscard]] QList<KHolidays::HolidayRegion *> &holidays();

    /**
     * Set which holiday categories the user wants to use.
     * @param categories a list of Holiday Category strings.
     */
    void setHolidayCategories(const QStringList &categories);

    /**
     * Returns the holiday categories list
     */
    [[nodiscard]] QStringList holidayCategories() const;

protected:
    KOGlobals();

private:
    QList<KHolidays::HolidayRegion *> mHolidayRegions;
    QStringList mHolidayCategories;
};
