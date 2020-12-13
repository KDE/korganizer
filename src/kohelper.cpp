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

QColor KOHelper::getTextColor(const QColor &c)
{
    const double luminance = (c.red() * 0.299) + (c.green() * 0.587) + (c.blue() * 0.114);
    return (luminance > 128.0) ? QColor(0, 0, 0) : QColor(255, 255, 255);
}

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

QColor KOHelper::resourceColor(const Akonadi::Item &item)
{
    return EventViews::resourceColor(item, KOPrefs::instance()->eventViewsPreferences());
}

int KOHelper::yearDiff(QDate start, QDate end)
{
    return end.year() - start.year();
}

bool KOHelper::isStandardCalendar(Akonadi::Collection::Id id)
{
    return id == CalendarSupport::KCalPrefs::instance()->defaultCalendarId();
}

void KOHelper::showSaveIncidenceErrorMsg(QWidget *parent, const KCalendarCore::Incidence::Ptr &incidence)
{
    KMessageBox::sorry(
        parent,
        i18n("Unable to save %1 \"%2\".",
             i18n(incidence->typeStr().constData()), incidence->summary()));
}
