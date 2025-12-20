/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "kocore.h"
#include "prefs/koprefs.h"

#include <KPluginFactory>

#include <QDBusConnectionInterface>

KOCore *KOCore::mSelf = nullptr;

KOCore *KOCore::self()
{
    if (!mSelf) {
        mSelf = new KOCore;
    }

    return mSelf;
}

KOCore::KOCore()
{
    // fallback reminder daemon startup
    // this should be started by autostart and session management already under normal
    // circumstances, but another safety net doesn't hurt
    QDBusConnection::sessionBus().interface()->startService(QStringLiteral("org.kde.kalendarac"));
}

KOCore::~KOCore()
{
    mSelf = nullptr;
}

QList<KPluginMetaData> KOCore::availableCalendarDecorations()
{
    return KPluginMetaData::findPlugins(QStringLiteral("pim6/korganizer"));
}

EventViews::CalendarDecoration::Decoration *KOCore::loadCalendarDecoration(const KPluginMetaData &service)
{
    return KPluginFactory::instantiatePlugin<EventViews::CalendarDecoration::Decoration>(service).plugin;
}

void KOCore::addXMLGUIClient(QWidget *wdg, KXMLGUIClient *guiclient) /* cppcheck-suppress constParameterPointer */
{
    mXMLGUIClients.insert(wdg, guiclient);
}

void KOCore::removeXMLGUIClient(QWidget *wdg) /* cppcheck-suppress constParameterPointer */
{
    mXMLGUIClients.remove(wdg);
}

KXMLGUIClient *KOCore::xmlguiClient(QWidget *wdg) const
{
    if (!wdg) {
        return nullptr;
    }

    /* cppcheck-suppress constVariablePointer */
    QWidget *topLevel = wdg->topLevelWidget(); // NOLINT(misc-const-correctness)
    QMap<QWidget *, KXMLGUIClient *>::ConstIterator const it = mXMLGUIClients.find(topLevel);
    if (it != mXMLGUIClients.constEnd()) {
        return it.value();
    }

    return nullptr;
}

EventViews::CalendarDecoration::Decoration::List KOCore::loadCalendarDecorations()
{
    if (!mCalendarDecorationsLoaded) {
        const QStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;

        mCalendarDecorations.clear();
        const QList<KPluginMetaData> plugins = availableCalendarDecorations();
        for (const auto &plugin : plugins) {
            QString const name = plugin.pluginId();
            if (selectedPlugins.contains(name)) {
                EventViews::CalendarDecoration::Decoration *d = loadCalendarDecoration(plugin);
                mCalendarDecorations.append(d);
            }
        }
        mCalendarDecorationsLoaded = true;
    }

    return mCalendarDecorations;
}

void KOCore::unloadPlugins()
{
    qDeleteAll(mCalendarDecorations);
    mCalendarDecorations.clear();
    mCalendarDecorationsLoaded = false;
}

void KOCore::reloadPlugins()
{
    // TODO: does this still apply?
    // Plugins should be unloaded, but e.g. komonthview keeps using the old ones
    unloadPlugins();
    loadCalendarDecorations();
}
