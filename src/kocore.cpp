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

KService::List KOCore::availablePlugins()
{
    return availablePlugins(CalendarSupport::Plugin::serviceType(), CalendarSupport::Plugin::interfaceVersion());
}

KService::List KOCore::availableCalendarDecorations()
{
    return availablePlugins(EventViews::CalendarDecoration::Decoration::serviceType(), EventViews::CalendarDecoration::Decoration::interfaceVersion());
}

KService::List KOCore::availableParts()
{
    return availablePlugins(KOrg::Part::serviceType(), KOrg::Part::interfaceVersion());
}

CalendarSupport::Plugin *KOCore::loadPlugin(const KService::Ptr &service)
{
    qCDebug(KORGANIZER_LOG) << service->library();

    if (!service->hasServiceType(CalendarSupport::Plugin::serviceType())) {
        return nullptr;
    }

    KPluginLoader loader(*service);
    auto factory = loader.instance();

    if (!factory) {
        qCDebug(KORGANIZER_LOG) << "Factory creation failed";
        return nullptr;
    }

    auto pluginFactory = qobject_cast<CalendarSupport::PluginFactory *>(factory);

    if (!pluginFactory) {
        qCDebug(KORGANIZER_LOG) << "Cast failed";
        return nullptr;
    }

    return pluginFactory->createPluginFactory();
}

CalendarSupport::Plugin *KOCore::loadPlugin(const QString &name)
{
    KService::List list = availablePlugins();
    KService::List::ConstIterator it;
    KService::List::ConstIterator end(list.constEnd());
    for (it = list.constBegin(); it != end; ++it) {
        if ((*it)->desktopEntryName() == name) {
            return loadPlugin(*it);
        }
    }
    return nullptr;
}

EventViews::CalendarDecoration::Decoration *KOCore::loadCalendarDecoration(const KService::Ptr &service)
{
    KPluginLoader loader(*service);
    auto factory = loader.instance();

    if (!factory) {
        qCDebug(KORGANIZER_LOG) << "Factory creation failed";
        return nullptr;
    }

    auto pluginFactory = qobject_cast<EventViews::CalendarDecoration::DecorationFactory *>(factory);

    if (!pluginFactory) {
        qCDebug(KORGANIZER_LOG) << "Cast failed";
        return nullptr;
    }

    return pluginFactory->createPluginFactory();
}

EventViews::CalendarDecoration::Decoration *KOCore::loadCalendarDecoration(const QString &name)
{
    const KService::List list = availableCalendarDecorations();
    KService::List::ConstIterator it;
    KService::List::ConstIterator end(list.constEnd());
    for (it = list.constBegin(); it != end; ++it) {
        if ((*it)->desktopEntryName() == name) {
            return loadCalendarDecoration(*it);
        }
    }
    return nullptr;
}

KOrg::Part *KOCore::loadPart(const KService::Ptr &service, KOrg::MainWindow *parent)
{
    qCDebug(KORGANIZER_LOG) << service->library();

    if (!service->hasServiceType(KOrg::Part::serviceType())) {
        return nullptr;
    }

    KPluginLoader loader(*service);
    KPluginFactory *factory = loader.factory();

    if (!factory) {
        qCDebug(KORGANIZER_LOG) << "Factory creation failed";
        return nullptr;
    }

    auto pluginFactory = static_cast<KOrg::PartFactory *>(factory);

    if (!pluginFactory) {
        qCDebug(KORGANIZER_LOG) << "Cast failed";
        return nullptr;
    }

    return pluginFactory->createPluginFactory(parent);
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

KOrg::Part *KOCore::loadPart(const QString &name, KOrg::MainWindow *parent)
{
    const KService::List list = availableParts();
    KService::List::ConstIterator end(list.constEnd());
    for (KService::List::ConstIterator it = list.constBegin(); it != end; ++it) {
        if ((*it)->desktopEntryName() == name) {
            return loadPart(*it, parent);
        }
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

KOrg::Part::List KOCore::loadParts(KOrg::MainWindow *parent)
{
    KOrg::Part::List parts;

    const QStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;

    const KService::List plugins = availableParts();
    const KService::List::ConstIterator end(plugins.constEnd());
    for (KService::List::ConstIterator it = plugins.constBegin(); it != end; ++it) {
        if (selectedPlugins.contains((*it)->desktopEntryName())) {
            KOrg::Part *part = loadPart(*it, parent);
            if (part) {
                if (!parent->mainGuiClient()) {
                    qCCritical(KORGANIZER_LOG) << "parent has no mainGuiClient.";
                } else {
                    parent->mainGuiClient()->insertChildClient(part);
                    parts.append(part);
                }
            }
        }
    }
    return parts;
}

void KOCore::unloadPlugins()
{
    qDeleteAll(mCalendarDecorations);
    mCalendarDecorations.clear();
    mCalendarDecorationsLoaded = false;
}

void KOCore::unloadParts(KOrg::MainWindow *parent, KOrg::Part::List &parts)
{
    foreach (KOrg::Part *part, parts) {
        parent->mainGuiClient()->removeChildClient(part);
        delete part;
    }
    parts.clear();
}

KOrg::Part::List KOCore::reloadParts(KOrg::MainWindow *parent, KOrg::Part::List &parts)
{
    KXMLGUIFactory *factory = parent->mainGuiClient()->factory();
    factory->removeClient(parent->mainGuiClient());

    unloadParts(parent, parts);
    const KOrg::Part::List list = loadParts(parent);

    factory->addClient(parent->mainGuiClient());

    return list;
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
