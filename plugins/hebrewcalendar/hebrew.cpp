/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2003 Jonathan Singer <jsinger@leeta.net>
  SPDX-FileCopyrightText: 2007 Loïc Corbasson <loic.corbasson@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "hebrew.h"
#include "configdialog.h"
#include "holiday.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QLocale>

#include <KHolidays/HebrewConverter>

K_PLUGIN_CLASS_WITH_JSON(Hebrew, "hebrew.json")

using namespace EventViews::CalendarDecoration;

Hebrew::Hebrew(QObject *parent, const QVariantList &args)
    : Decoration(parent, args)
{
    KConfig config(QStringLiteral("korganizerrc"), KConfig::NoGlobals);

    const KConfigGroup group(&config, QStringLiteral("Hebrew Calendar Plugin"));
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

static bool isLeapYear(int year)
{
    return ((((7 * year) + 1) % 19) < 7);
}

static int monthNumberToMonthIndex(int year, int month)
{
    if (isLeapYear(year)) {
        if (month == 6) {
            return 13; // Adar I
        } else if (month == 7) {
            return 14; // Adar II
        } else if (month > 7) {
            return month - 1; // Because of Adar II
        }
    }

    return month;
}

static QString monthName(int month, int year)
{
    // month and year are in the hebrew calendar
    // month number is according to the Biblical reckoning
    // See https://en.wikipedia.org/wiki/Hebrew_calendar

    // Convert Biblical to Civil month numbering
    if (month == 12) {
        month = 6;
    } else {
        if (month <= 6) {
            month += 6;
        } else {
            month -= 6;
        }
    }

    // We must map month number to month index (to account for leap years)
    const int monthIndex = monthNumberToMonthIndex(year, month);

    // Civil
    switch (monthIndex) {
    case 1:
        return i18nc("Hebrew month 1 - KLocale::LongName", "Tishrey");
    case 2:
        return i18nc("Hebrew month 2 - KLocale::LongName", "Heshvan");
    case 3:
        return i18nc("Hebrew month 3 - KLocale::LongName", "Kislev");
    case 4:
        return i18nc("Hebrew month 4 - KLocale::LongName", "Tevet");
    case 5:
        return i18nc("Hebrew month 5 - KLocale::LongName", "Shvat");
    case 6:
        return i18nc("Hebrew month 6 - KLocale::LongName", "Adar");
    case 7:
        return i18nc("Hebrew month 7 - KLocale::LongName", "Nisan");
    case 8:
        return i18nc("Hebrew month 8 - KLocale::LongName", "Iyar");
    case 9:
        return i18nc("Hebrew month 9 - KLocale::LongName", "Sivan");
    case 10:
        return i18nc("Hebrew month 10 - KLocale::LongName", "Tamuz");
    case 11:
        return i18nc("Hebrew month 11 - KLocale::LongName", "Av");
    case 12:
        return i18nc("Hebrew month 12 - KLocale::LongName", "Elul");
    case 13:
        return i18nc("Hebrew month 13 - KLocale::LongName", "Adar I");
    case 14:
        return i18nc("Hebrew month 14 - KLocale::LongName", "Adar II");
    default:
        return QString();
    }
}

Element::List Hebrew::createDayElements(const QDate &date)
{
    Element::List el;
    QString text;
    const KHolidays::HebrewDate hd = KHolidays::HebrewDate::fromSecular(date.year(), date.month(), date.day());
    const QStringList holidays = Holiday::findHoliday(hd, areWeInIsrael, showParsha, showChol, showOmer);
    text = i18nc("1. day of the month 2. hebrew month", "%1 %2", hd.day(), monthName(hd.month(), hd.year()));

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
