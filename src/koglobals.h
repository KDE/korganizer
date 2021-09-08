/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
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

    Q_REQUIRED_RESULT QMap<QDate, QStringList> holiday(const QDate &start, const QDate &end) const;

    Q_REQUIRED_RESULT int firstDayOfWeek() const;

    Q_REQUIRED_RESULT int getWorkWeekMask();

    /**
       Set which holiday regions the user wants to use.
       @param regions a list of Holiday Regions strings.
    */
    void setHolidays(const QStringList &regions);

    /** return the HolidayRegion object or 0 if none has been defined
     */
    Q_REQUIRED_RESULT QList<KHolidays::HolidayRegion *> holidays() const;

protected:
    KOGlobals();

private:
    QList<KHolidays::HolidayRegion *> mHolidayRegions;
};

