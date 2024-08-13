/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kocorehelper.h"

#include <Akonadi/TagCache>

#include <KLocalizedString>

QColor KOCoreHelper::categoryColor(const QStringList &categories)
{
    // FIXME: Correctly treat events with multiple categories
    QColor bgColor;
    if (!categories.isEmpty()) {
        bgColor = Akonadi::TagCache::instance()->tagColor(categories.at(0));
    }
    return bgColor.isValid() ? bgColor : CalendarSupport::KCalPrefs::instance()->unsetCategoryColor();
}

QString KOCoreHelper::holidayString(const QDate &dt)
{
    QStringList lst(KOGlobals::self()->holiday(dt, dt).value(dt));
    return lst.join(i18nc("@item:intext delimiter for joining holiday names", ","));
}
