/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2007 Loïc Corbasson <loic.corbasson@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "thisdayinhistory.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(ThisDayInHistory, "thisdayinhistory.json")

ThisDayInHistory::ThisDayInHistory(QObject *parent, const QVariantList &args)
    : Decoration(parent, args)
{
    KConfig _config(QStringLiteral("korganizerrc"));
    KConfigGroup config(&_config, QStringLiteral("This Day in History Plugin"));
}

QString ThisDayInHistory::info() const
{
    return i18n(
        "This plugin provides links to Wikipedia's "
        "'This Day in History' pages.");
}

Element::List ThisDayInHistory::createDayElements(const QDate &date)
{
    Element::List elements;

    auto element = new StoredElement(QStringLiteral("Wikipedia link"), i18n("This day in history"));

    element->setUrl(QUrl(i18nc("Localized Wikipedia website", "https://en.wikipedia.org/wiki/")
                         + date.toString(i18nc("Qt date format used by the localized Wikipedia", "MMMM_d"))));

    elements.append(element);

    return elements;
}

Element::List ThisDayInHistory::createMonthElements(const QDate &date)
{
    Element::List elements;

    auto element = new StoredElement(QStringLiteral("Wikipedia link"), i18n("This month in history"));

    element->setUrl(QUrl(i18nc("Localized Wikipedia website", "https://en.wikipedia.org/wiki/")
                         + date.toString(i18nc("Qt date format used by the localized Wikipedia", "MMMM_yyyy"))));

    elements.append(element);

    return elements;
}

#include "thisdayinhistory.moc"
