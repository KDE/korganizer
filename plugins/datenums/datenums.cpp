/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2007 Loïc Corbasson <loic.corbasson@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "datenums.h"
#include "configdialog.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(Datenums, "datenums.json")

Datenums::Datenums(QObject *parent, const QVariantList &args)
    : Decoration(parent, args)
{
    KConfig _config(QStringLiteral("korganizerrc"), KConfig::NoGlobals);
    KConfigGroup config(&_config, QStringLiteral("Calendar/Datenums Plugin"));
    mDisplayedInfo = (DayNumbers)config.readEntry("ShowDayNumbers", int(DayOfYear | DaysRemaining));
}

void Datenums::configure(QWidget *parent)
{
    ConfigDialog dlg(parent);
    dlg.exec();
}

QString Datenums::info() const
{
    return i18n("This plugin shows information on a day's position in the year.");
}

Element::List Datenums::createDayElements(const QDate &date)
{
    Element::List result;

    int dayOfYear = date.dayOfYear();
    int remainingDays = date.daysInYear() - dayOfYear;

    StoredElement *e = nullptr;
    switch (mDisplayedInfo) {
    case DayOfYear: // only day of year
        e = new StoredElement(QStringLiteral("main element"), QString::number(dayOfYear));
        break;
    case DaysRemaining: // only days until end of year
        e = new StoredElement(QStringLiteral("main element"),
                              QString::number(remainingDays),
                              i18np("1 day before the end of the year", "%1 days before the end of the year", remainingDays));
        break;
    default:
        e = new StoredElement(QStringLiteral("main element"),
                              QString::number(dayOfYear),
                              i18nc("dayOfYear / daysTillEndOfYear", "%1 / %2", dayOfYear, remainingDays),
                              i18np("1 day since the beginning of the year,\n", "%1 days since the beginning of the year,\n", dayOfYear)
                                  + i18np("1 day until the end of the year", "%1 days until the end of the year", remainingDays));
        break;
    }
    result.append(e);

    return result;
}

static int weeksInYear(int year)
{
    QDate date(year, 12, 31);
    for (int i = 0; i < 6; ++i) {
        if (date.weekNumber() > 1) {
            break;
        }
        date = date.addDays(-1);
    }
    return date.weekNumber();
}

Element::List Datenums::createWeekElements(const QDate &date)
{
    Element::List result;

    int *yearOfTheWeek = nullptr;
    yearOfTheWeek = nullptr;
    int remainingWeeks;
    const int weekOfYear = date.weekNumber(yearOfTheWeek);

    QString weekOfYearShort;
    QString weekOfYearLong;
    QString weekOfYearExtensive;
    QString remainingWeeksShort;
    QString remainingWeeksLong;
    QString remainingWeeksExtensive;
    QString weekOfYearAndRemainingWeeksShort;

    // Usual case: the week belongs to this year
    remainingWeeks = weeksInYear(date.year()) - weekOfYear;

    weekOfYearShort = QString::number(weekOfYear);
    weekOfYearLong = i18nc("Week weekOfYear", "Week %1", weekOfYear);
    weekOfYearExtensive = i18np("1 week since the beginning of the year", "%1 weeks since the beginning of the year", weekOfYear);

    if (yearOfTheWeek) { // The week does not belong to this year
        weekOfYearShort = i18nc("weekOfYear (year)", "%1 (%2)", weekOfYear, *yearOfTheWeek);
        weekOfYearLong = i18nc("Week weekOfYear (year)", "Week %1 (%2)", weekOfYear, *yearOfTheWeek);

        if (*yearOfTheWeek == date.year() + 1) {
            // The week belongs to next year
            remainingWeeks = 0;

            weekOfYearExtensive = i18np("1 week since the beginning of the year", "%1 weeks since the beginning of the year", weekOfYear);
        } else {
            // The week belongs to last year
            remainingWeeks = weeksInYear(date.year());

            weekOfYearExtensive = i18np("1 week since the beginning of the year", "%1 weeks since the beginning of the year", 0);
        }
    }

    remainingWeeksShort = i18np("1 week remaining", "%1 weeks remaining", remainingWeeks);
    remainingWeeksExtensive = i18np("1 week until the end of the year", "%1 weeks until the end of the year", remainingWeeks);
    weekOfYearAndRemainingWeeksShort = i18nc("weekOfYear / weeksTillEndOfYear", "%1 / %2", weekOfYear, remainingWeeks);

    StoredElement *e = nullptr;
    switch (mDisplayedInfo) {
    case DayOfYear: // only week of year
        e = new StoredElement(QStringLiteral("main element"), weekOfYearShort, weekOfYearLong, weekOfYearExtensive);
        break;
    case DaysRemaining: // only weeks until end of year
        e = new StoredElement(QStringLiteral("main element"), remainingWeeksShort, remainingWeeksLong, remainingWeeksExtensive);
        break;
    default:
        e = new StoredElement(QStringLiteral("main element"),
                              weekOfYearShort,
                              weekOfYearAndRemainingWeeksShort,
                              i18nc("n weeks since the beginning of the year\n"
                                    "n weeks until the end of the year",
                                    "%1\n%2",
                                    weekOfYearExtensive,
                                    remainingWeeksExtensive));
        break;
    }
    result.append(e);

    return result;
}

#include "datenums.moc"
