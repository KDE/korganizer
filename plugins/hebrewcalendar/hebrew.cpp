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

static QString stringFromInteger(int number)
{
    if (QLocale().language() == QLocale::Hebrew) {
        // Hebrew numbers are composed of combinations of normal letters which have a numeric value.
        // This is a non-positional system, the numeric values are simply added together, however
        // convention is for a RTL highest to lowest value ordering. There is also a degree of
        // ambiguity due to the lack of a letter for 0, hence 5 and 5000 are written the same.
        // Hebrew numbers are only used in dates.
        // See https://en.wikipedia.org/wiki/Hebrew_numerals for more explanation

        /*
         *        Ref table for numbers to Hebrew chars
         *
         *        Value     1       2       3        4        5       6         7        8      9
         *
         *        x 1    Alef א  Bet  ב  Gimel ג  Dalet ד  He   ה  Vav  ו    Zayen ז  Het  ח  Tet  ט
         *               0x05D0  0x05D1  0x05D2   0x05D3   0x05D4  0x05D5    0x05D6   0x05D7  0x05D8
         *
         *        x 10   Yod  י  Kaf  כ  Lamed ל  Mem  מ   Nun  נ  Samekh ס  Ayin ע   Pe   פ  Tzadi צ
         *               0x05D9  0x05DB  0x05DC   0x05DE   0x05E0  0x05E1    0x05E2   0x05E4  0x05E6
         *
         *        x 100  Qof  ק  Resh ר  Shin ש   Tav  ת
         *               0x05E7  0x05E8  0x05E9   0x05EA
         *
         *        Note special cases 15 = 9 + 6 = 96 טו and 16 = 9 + 7 = 97 טז
         */

        const unsigned int decade[] = {//  Tet = ט,    Yod = י,    Kaf = כ,    Lamed = ל,  Mem = מ
                                       //  Nun = נ,    Samekh = ס, Ayin = ע,   Pe = פ,     Tsadi = צ
                                       0x05D8,
                                       0x05D9,
                                       0x05DB,
                                       0x05DC,
                                       0x05DE,
                                       0x05E0,
                                       0x05E1,
                                       0x05E2,
                                       0x05E4,
                                       0x05E6};

        QString result;

        // We have no rules for coping with numbers outside this range
        if (number < 1 || number > 9999) {
            return QString::number(number);
        }

        // Translate the thousands digit, just uses letter for number 1..9 ( א to ט, Alef to Tet )
        // Years 5001-5999 do not have the thousands by convention
        if (number >= 1000) {
            if (number <= 5000 || number >= 6000) {
                result += QChar(0x05D0 - 1 + number / 1000); // Alef א to Tet ט
            }
            number %= 1000;
        }

        // Translate the hundreds digit
        // Use traditional method where we only have letters assigned values for 100, 200, 300 and 400
        // so may need to repeat 400 twice to make up the required number
        if (number >= 100) {
            while (number >= 500) {
                result += QChar(0x05EA); // Tav = ת
                number -= 400;
            }
            result += QChar(0x05E7 - 1 + number / 100); // Qof = ק to xxx
            number %= 100;
        }

        // Translate the tens digit
        // The numbers 15 and 16 translate to letters that spell out the name of God which is
        // forbidden, so require special treatment where 15 = 9 + 6 and 1 = 9 + 7.
        if (number >= 10) {
            if (number == 15 || number == 16) {
                number -= 9;
            }
            result += static_cast<QChar>(decade[number / 10]);
            number %= 10;
        }

        // Translate the ones digit, uses letter for number 1..9 ( א to ט, Alef to Tet )
        if (number > 0) {
            result += QChar(0x05D0 - 1 + number); // Alef = א to xxx
        }

        // When used in a string with mixed names and numbers the numbers need special chars to
        // distinguish them from words composed of the same letters.
        // Single digit numbers are followed by a geresh symbol ? (Unicode = 0x05F3), but we use
        // single quote for convenience.
        // Multiple digit numbers have a gershayim symbol ? (Unicode = 0x05F4) as second-to-last
        // char, but we use double quote for convenience.
        if (result.length() == 1) {
            result += QLatin1Char('\'');
        } else {
            result.insert(result.length() - 1, QLatin1Char('\"'));
        }

        return result;
    } else {
        return QString::number(number);
    }
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
    text = i18nc("1. day of the month 2. hebrew month", "%1 %2", stringFromInteger(hd.day()), monthName(hd.month(), hd.year()));

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
