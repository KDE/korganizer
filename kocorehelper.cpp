/*
  This file is part of KOrganizer.

  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "kocorehelper.h"

#include <calendarsupport/kcalprefs.h>

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
    QStringList lst(KOGlobals::self()->holiday(dt, dt)[dt]);
    return lst.join(i18nc("@item:intext delimiter for joining holiday names", ","));
}
