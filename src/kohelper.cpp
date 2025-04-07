/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kohelper.h"
#include "prefs/koprefs.h"

#include <EventViews/Helper>

#include <CalendarSupport/KCalPrefs>
#include <KLocalizedString>
#include <KMessageBox>

QColor KOHelper::resourceColor(const Akonadi::Collection &coll)
{
    return EventViews::resourceColor(coll, KOPrefs::instance()->eventViewsPreferences());
}

QColor KOHelper::resourceColorKnown(const Akonadi::Collection &coll)
{
    return EventViews::resourceColor(coll, KOPrefs::instance()->eventViewsPreferences());
}

void KOHelper::setResourceColor(const Akonadi::Collection &collection, const QColor &color)
{
    EventViews::setResourceColor(collection, color, KOPrefs::instance()->eventViewsPreferences());
    KOPrefs::instance()->eventViewsPreferences()->writeConfig();
}

bool KOHelper::isStandardCalendar(Akonadi::Collection::Id id)
{
    return id == CalendarSupport::KCalPrefs::instance()->defaultCalendarId();
}
