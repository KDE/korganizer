/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kocore.h"
#include "prefs/koprefs.h"

#include <CalendarSupport/IdentityManager>

#include "korganizer_debug.h"
#include <KServiceTypeTrader>
#include <KXMLGUIFactory>

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

KService::List KOCore::availablePlugins(const QString &type, int version)
{
    QString constraint;
    if (version >= 0) {
        constraint = QStringLiteral("[X-KDE-PluginInterfaceVersion] == %1").arg(QString::number(version));
    }

    return KServiceTypeTrader::self()->query(type, constraint);
}

KService::List KOCore::availableCalendarDecorations()
{
    return availablePlugins(EventViews::CalendarDecoration::Decoration::serviceType(), EventViews::CalendarDecoration::Decoration::interfaceVersion());
}

EventViews::CalendarDecoration::Decoration *KOCore::loadCalendarDecoration(const KService::Ptr &service)
{
    KPluginFactory *factory = KPluginFactory::loadFactory(KPluginMetaData(service->library())).plugin;
    if (!factory) {
        qCDebug(KORGANIZER_LOG) << "Factory creation failed";
        return nullptr;
    }

    return factory->create<EventViews::CalendarDecoration::Decoration>();
}

void KOCore::addXMLGUIClient(QWidget *wdg, KXMLGUIClient *guiclient)
{
    mXMLGUIClients.insert(wdg, guiclient);
}

void KOCore::removeXMLGUIClient(QWidget *wdg)
{
    mXMLGUIClients.remove(wdg);
}

KXMLGUIClient *KOCore::xmlguiClient(QWidget *wdg) const
{
    if (!wdg) {
        return nullptr;
    }

    QWidget *topLevel = wdg->topLevelWidget();
    QMap<QWidget *, KXMLGUIClient *>::ConstIterator it = mXMLGUIClients.find(topLevel);
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
        const KService::List plugins = availableCalendarDecorations();
        KService::List::ConstIterator it;
        const KService::List::ConstIterator end(plugins.constEnd());
        for (it = plugins.constBegin(); it != end; ++it) {
            if ((*it)->hasServiceType(EventViews::CalendarDecoration::Decoration::serviceType())) {
                QString name = (*it)->desktopEntryName();
                if (selectedPlugins.contains(name)) {
                    EventViews::CalendarDecoration::Decoration *d = loadCalendarDecoration(*it);
                    mCalendarDecorations.append(d);
                }
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

KIdentityManagement::IdentityManager *KOCore::identityManager()
{
    return CalendarSupport::IdentityManager::self();
}
