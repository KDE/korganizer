/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2002, 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "koglobals.h"
#include "prefs/koprefs.h"

#include <CalendarSupport/KCalPrefs>

#include <KHolidays/HolidayCategories>
#include <KHolidays/HolidayRegion>

#include <QApplication>

class KOGlobalsSingletonPrivate
{
public:
    KOGlobals instance;
};

Q_GLOBAL_STATIC(KOGlobalsSingletonPrivate, sKOGlobalsSingletonPrivate)

KOGlobals *KOGlobals::self()
{
    return &sKOGlobalsSingletonPrivate->instance;
}

KOGlobals::KOGlobals() = default;

KOGlobals::~KOGlobals()
{
    qDeleteAll(mHolidayRegions);
    mHolidayCategories.clear();
}

bool KOGlobals::reverseLayout()
{
    return QApplication::isRightToLeft();
}

QMap<QDate, QStringList> KOGlobals::holiday(const QDate &start, const QDate &end)
{
    QMap<QDate, QStringList> holidaysByDate;

    if (mHolidayRegions.isEmpty()) {
        return holidaysByDate;
    }

    setHolidayCategories(CalendarSupport::KCalPrefs::instance()->holidayCategories());
    for (KHolidays::HolidayRegion *region : std::as_const(mHolidayRegions)) {
        if (region && region->isValid()) {
            region->setCategories(holidayCategories());
            const KHolidays::Holiday::List list = region->rawHolidaysWithAstroSeasons(start, end);
            const int listCount(list.count());
            for (int i = 0; i < listCount; ++i) {
                const KHolidays::Holiday &h = list.at(i);
                // dedupe, since we support multiple holiday regions which may have similar holidays
                if (!holidaysByDate[h.observedStartDate()].contains(h.name())) {
                    holidaysByDate[h.observedStartDate()].append(h.name());
                }
            }
        }
    }

    return holidaysByDate;
}

/* cppcheck-suppress functionStatic */
int KOGlobals::firstDayOfWeek() const
{
    return KOPrefs::instance()->mWeekStartDay + 1;
}

/* cppcheck-suppress functionStatic */
int KOGlobals::getWorkWeekMask()
{
    return KOPrefs::instance()->mWorkWeekMask;
}

void KOGlobals::setHolidays(const QStringList &regions)
{
    qDeleteAll(mHolidayRegions);
    mHolidayRegions.clear();
    for (const QString &regionStr : regions) {
        auto region = new KHolidays::HolidayRegion(regionStr);
        if (region->isValid()) {
            mHolidayRegions.append(region);
        } else {
            delete region;
        }
    }
}

QList<KHolidays::HolidayRegion *> &KOGlobals::holidays()
{
    return mHolidayRegions;
}

void KOGlobals::setHolidayCategories(const QStringList &categories)
{
    mHolidayCategories.clear();
    for (const QString &category : categories) {
        if (KHolidays::isHolidayCategoryValid(category)) {
            mHolidayCategories.append(category);
        }
    }
}

QStringList KOGlobals::holidayCategories() const
{
    return mHolidayCategories;
}
