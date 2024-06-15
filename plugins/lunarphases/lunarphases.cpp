/*
  SPDX-FileCopyrightText: 2018 Allen Winter <winter@kde.org>
  SPDX-FileCopyrightText: 2021 Friedrich W. H. Kossebau <kossebau@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "lunarphases.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(Lunarphases, "lunarphases.json")

static QIcon phaseIcon(KHolidays::LunarPhase::Phase phase)
{
    const QString iconName = (phase == KHolidays::LunarPhase::NewMoon) ? QStringLiteral("moon-phase-new")
        : (phase == KHolidays::LunarPhase::FullMoon)                   ? QStringLiteral("moon-phase-full")
        : (phase == KHolidays::LunarPhase::FirstQuarter)               ? QStringLiteral("moon-phase-first-quarter")
        : (phase == KHolidays::LunarPhase::LastQuarter)                ? QStringLiteral("moon-phase-last-quarter")
                                                                       :
                                                        /* else */ QString();
    return iconName.isEmpty() ? QIcon() : QIcon::fromTheme(iconName);
}

LunarphasesElement::LunarphasesElement(KHolidays::LunarPhase::Phase phase)
    : Element(QStringLiteral("main element"))
    , mName(KHolidays::LunarPhase::phaseName(phase))
    , mIcon(phaseIcon(phase))
{
}

QString LunarphasesElement::shortText() const
{
    return mName;
}

QString LunarphasesElement::longText() const
{
    return mName;
}

QPixmap LunarphasesElement::newPixmap(const QSize &size)
{
    // TODO: support south hemisphere & equator by rotating by 90 and 180 degrees
    return mIcon.pixmap(size);
}

Lunarphases::Lunarphases(QObject *parent, const QVariantList &args)
    : Decoration(parent, args)
{
    KConfig _config(QStringLiteral("korganizerrc"));
    KConfigGroup config(&_config, QStringLiteral("Calendar/Lunar Phases Plugin"));
}

QString Lunarphases::info() const
{
    return i18n(
        "This plugin displays the day's lunar phase (New, First, Last, Full). "
        "Currently, the phase is computed for noon at UTC; therefore, you should "
        "expect variations by 1 day in either direction.");
}

Element::List Lunarphases::createDayElements(const QDate &date)
{
    Element::List result;

    KHolidays::LunarPhase::Phase phase = KHolidays::LunarPhase::phaseAtDate(date);
    if (phase != KHolidays::LunarPhase::None) {
        auto e = new LunarphasesElement(phase);
        result.append(e);
    }

    return result;
}

#include "lunarphases.moc"

#include "moc_lunarphases.cpp"
