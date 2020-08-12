/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2002, 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koglobals.h"
#include "prefs/koprefs.h"

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

KOGlobals::KOGlobals()
{
}

KOGlobals::~KOGlobals()
{
    qDeleteAll(mHolidayRegions);
}

bool KOGlobals::reverseLayout()
{
    return QApplication::isRightToLeft();
}

QMap<QDate, QStringList> KOGlobals::holiday(const QDate &start, const QDate &end) const
{
    QMap<QDate, QStringList> holidaysByDate;

    if (mHolidayRegions.isEmpty()) {
        return holidaysByDate;
    }

    for (const KHolidays::HolidayRegion *region : qAsConst(mHolidayRegions)) {
        if (region && region->isValid()) {
            const KHolidays::Holiday::List list = region->holidays(start, end);
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

int KOGlobals::firstDayOfWeek() const
{
    return KOPrefs::instance()->mWeekStartDay + 1;
}

QList<QDate> KOGlobals::workDays(QDate startDate, QDate endDate) const
{
    QList<QDate> result;

    const int mask(~(KOPrefs::instance()->mWorkWeekMask));
    const qint64 numDays = startDate.daysTo(endDate) + 1;

    for (int i = 0; i < numDays; ++i) {
        const QDate date = startDate.addDays(i);
        if (!(mask & (1 << (date.dayOfWeek() - 1)))) {
            result.append(date);
        }
    }

    if (KOPrefs::instance()->mExcludeHolidays) {
        for (const KHolidays::HolidayRegion *region : qAsConst(mHolidayRegions)) {
            const KHolidays::Holiday::List list = region->holidays(startDate, endDate);
            for (int i = 0; i < list.count(); ++i) {
                const KHolidays::Holiday &h = list.at(i);
                if (h.dayType() == KHolidays::Holiday::NonWorkday) {
                    result.removeAll(h.observedStartDate());
                }
            }
        }
    }

    return result;
}

int KOGlobals::getWorkWeekMask()
{
    return KOPrefs::instance()->mWorkWeekMask;
}

void KOGlobals::setHolidays(const QStringList &regions)
{
    qDeleteAll(mHolidayRegions);
    mHolidayRegions.clear();
    for (const QString &regionStr : regions) {
        KHolidays::HolidayRegion *region = new KHolidays::HolidayRegion(regionStr);
        if (region->isValid()) {
            mHolidayRegions.append(region);
        } else {
            delete region;
        }
    }
}

QList<KHolidays::HolidayRegion *> KOGlobals::holidays() const
{
    return mHolidayRegions;
}
