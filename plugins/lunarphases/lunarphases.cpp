/*
  SPDX-FileCopyrightText: Allen Winter <winter@kde.org>
  SPDX-FileCopyrightText: 2021 Friedrich W. H. Kossebau <kossebau@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "lunarphases.h"
#include "configdialog.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(Lunarphases, "lunarphases.json")

static QIcon phaseIcon(KHolidays::LunarPhase::Phase phase, Lunarphases::Hemisphere hemisphere)
{
    QString iconName;
    switch (phase) {
    case KHolidays::LunarPhase::NewMoon:
        iconName = QStringLiteral("realmoon-new");
        break;
    case KHolidays::LunarPhase::FullMoon:
        iconName = QStringLiteral("realmoon-full");
        break;
    case KHolidays::LunarPhase::FirstQuarter:
        iconName = QStringLiteral("realmoon-waxing-first-quarter");
        break;
    case KHolidays::LunarPhase::LastQuarter:
        iconName = QStringLiteral("realmoon-waning-last-quarter");
        break;
        // TODO find/create matching icons for these.
        // any new icon set must also look good in dark-mode.
    // case KHolidays::LunarPhase::WaxingCrescent:
    //     iconName = QStringLiteral("realmoon-waxing-crescent");
    //     break;
    // case KHolidays::LunarPhase::WaningCrescent:
    //    iconName = QStringLiteral("realmoon-waning-crescent");
    //    break;
    // case KHolidays::LunarPhase::WaxingGibbous:
    //    iconName = QStringLiteral("realmoon-waxing-gibbous");
    //    break;
    // case KHolidays::LunarPhase::WaningGibbous:
    //    iconName = QStringLiteral("realmoon-waning-gibbous");
    //    break;
    default:
        break;
    }
    if (!iconName.isEmpty() && iconName != QStringLiteral("realmoon-new") && iconName != QStringLiteral("realmoon-full")) {
        if (hemisphere == Lunarphases::NorthernHemisphere) {
            iconName += QStringLiteral("-north");
        } else {
            iconName += QStringLiteral("-south");
        }
    }
    return iconName.isEmpty() ? QIcon() : QIcon::fromTheme(iconName);
}

LunarphasesElement::LunarphasesElement(KHolidays::LunarPhase::Phase phase, Lunarphases::Hemisphere hemisphere)
    : Element(QStringLiteral("main element"))
    , mName(KHolidays::LunarPhase::phaseName(phase))
    , mIcon(phaseIcon(phase, hemisphere))
{
}

QString LunarphasesElement::shortText() const
{
    // don't clutter with phase name texts if we don't have an icon
    if (!mIcon.isNull()) {
        return mName;
    }
    return {};
}

QString LunarphasesElement::longText() const
{
    // don't clutter with phase name tooltips if we don't have an icon
    if (!mIcon.isNull()) {
        return mName;
    }
    return {};
}

QPixmap LunarphasesElement::newPixmap(const QSize &size)
{
    return mIcon.pixmap(size * 3 / 4); // I think a bit smaller than 48 pixels looks better
}

Lunarphases::Lunarphases(QObject *parent, const QVariantList &args)
    : Decoration(parent, args)
{
    KConfig _config(QStringLiteral("korganizerrc"));
    KConfigGroup const config(&_config, QStringLiteral("Calendar/Lunar Phases Plugin"));
    mHemisphere = (Hemisphere)config.readEntry("Hemisphere", int(NorthernHemisphere));
}

void Lunarphases::configure(QWidget *parent)
{
    ConfigDialog dlg(parent);
    dlg.exec();
}

QString Lunarphases::info() const
{
    return i18n(
        "This plugin displays the day's lunar phase (New, First, Last, Full). "
        "The phase is computed for noon in the system timezone.");
}

Element::List Lunarphases::createDayElements(const QDate &date)
{
    Element::List result;

    KHolidays::LunarPhase::Phase const phase = KHolidays::LunarPhase::phaseAtDate(date);
    if (phase != KHolidays::LunarPhase::None) {
        auto e = new LunarphasesElement(phase, mHemisphere);
        result.append(e);
    }

    return result;
}

#include "lunarphases.moc"

#include "moc_lunarphases.cpp"
