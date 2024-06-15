/*
  This file is part of KOrganizer.
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2007 Loïc Corbasson <loic.corbasson@gmail.com>
  SPDX-FileCopyrightText: 2021 Friedrich W. H. Kossebau <kossebau@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "picoftheday.h"
#include "configdialog.h"
#include "element.h"

#include "korganizer_picoftheday_plugin_debug.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QCache>

K_PLUGIN_CLASS_WITH_JSON(Picoftheday, "picoftheday.json")

// TODO: add also disc cache to avoid even more network traffic
using Cache = QCache<QDate, ElementData>;
constexpr int cacheElementMaxSize = 6 * 7; // rows by weekdays, a full gregorian month's view
Q_GLOBAL_STATIC_WITH_ARGS(Cache, s_cache, (cacheElementMaxSize))

// https://www.mediawiki.org/wiki/API:Picture_of_the_day_viewer
Picoftheday::Picoftheday(QObject *parent, const QVariantList &args)
    : Decoration(parent, args)
{
    KConfig _config(QStringLiteral("korganizerrc"));
    KConfigGroup config(&_config, QStringLiteral("Picture of the Day Plugin"));
    mThumbSize = config.readEntry("InitialThumbnailSize", QSize(120, 60));
}

void Picoftheday::configure(QWidget *parent)
{
    ConfigDialog dlg(parent);
    dlg.exec();
}

QString Picoftheday::info() const
{
    return i18n(
        "<qt>This plugin provides the Wikipedia "
        "<i>Picture of the Day</i>.</qt>");
}

Element::List Picoftheday::createDayElements(const QDate &date)
{
    Element::List elements;

    auto data = s_cache->take(date);
    qCDebug(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << date << ": taking from cache" << data;
    if (!data) {
        data = new ElementData;
        data->mThumbSize = mThumbSize;
    }

    auto element = new POTDElement(QStringLiteral("main element"), date, data);
    elements.append(element);

    return elements;
}

void Picoftheday::cacheData(QDate date, ElementData *data)
{
    if (data->mState < DataLoaded) {
        delete data;
        return;
    }
    qCDebug(KORGANIZERPICOFTHEDAYPLUGIN_LOG) << date << ": adding to cache" << data;
    s_cache->insert(date, data);
}

#include "picoftheday.moc"
