/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kocorehelper.h"

#include <CalendarSupport/KCalPrefs>

#include <KLocalizedString>

QColor KOCoreHelper::categoryColor(const QStringList &categories)
{
    if (categories.isEmpty()) {
        return CalendarSupport::KCalPrefs::instance()->unsetCategoryColor();
    }
    // FIXME: Correctly treat events with multiple categories
    const QString cat = categories.first();
    QColor bgColor;
    if (cat.isEmpty()) {
        bgColor = CalendarSupport::KCalPrefs::instance()->unsetCategoryColor();
    } else {
        bgColor = CalendarSupport::KCalPrefs::instance()->categoryColor(cat);
    }
    return bgColor;
}

QString KOCoreHelper::holidayString(const QDate &dt)
{
    QStringList lst(KOGlobals::self()->holiday(dt, dt).value(dt));
    return lst.join(i18nc("@item:intext delimiter for joining holiday names", ","));
}
