/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2003 Jonathan Singer <jsinger@leeta.net>
  SPDX-FileCopyrightText: 2007 Loïc Corbasson <loic.corbasson@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "hebrew.h"
#include "configdialog.h"
#include "converter.h"
#include "holiday.h"

#include <KCalendarSystem>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QLocale>

K_PLUGIN_CLASS_WITH_JSON(Hebrew, "hebrew.json")

using namespace EventViews::CalendarDecoration;

Hebrew::Hebrew(QObject *parent, const QVariantList &args)
    : Decoration(parent, args)
{
    KConfig config(QStringLiteral("korganizerrc"), KConfig::NoGlobals);

    KConfigGroup group(&config, QStringLiteral("Hebrew Calendar Plugin"));
    areWeInIsrael = group.readEntry("UseIsraelSettings", QLocale::territoryToString(QLocale().territory()) == QLatin1StringView(".il"));
    showParsha = group.readEntry("ShowParsha", true);
    showChol = group.readEntry("ShowChol_HaMoed", true);
    showOmer = group.readEntry("ShowOmer", true);
}

void Hebrew::configure(QWidget *parent)
{
    ConfigDialog dlg(parent);
    dlg.exec();
}

Element::List Hebrew::createDayElements(const QDate &date)
{
    Element::List el;
    QString text;
    HebrewDate hd = HebrewDate::fromSecular(date.year(), date.month(), date.day());

    const QStringList holidays = Holiday::findHoliday(hd, areWeInIsrael, showParsha, showChol, showOmer);

    KCalendarSystem *cal = KCalendarSystem::create(KLocale::HebrewCalendar);

    text = cal->formatDate(date, KLocale::Day, KLocale::LongNumber) + QLatin1Char(' ') + cal->monthName(date);

    for (const QString &holiday : holidays) {
        text += QLatin1StringView("<br/>\n") + holiday;
    }

    text = i18nc("Change the next two strings if emphasis is done differently in your language.", "<qt><p align=\"center\"><i>\n%1\n</i></p></qt>", text);
    el.append(new StoredElement(QStringLiteral("main element"), text));
    return el;
}

QString Hebrew::info() const
{
    return i18n("This plugin provides the date in the Jewish calendar.");
}

#include "hebrew.moc"
