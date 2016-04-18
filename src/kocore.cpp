/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kocore.h"
#include "prefs/koprefs.h"

#include <CalendarSupport/IdentityManager>

#include "korganizer_debug.h"
#include <KServiceTypeTrader>
#include <KXMLGUIFactory>

KOCore *KOCore::mSelf = Q_NULLPTR;

KOCore *KOCore::self()
{
    if (!mSelf) {
        mSelf = new KOCore;
    }

    return mSelf;
}

KOCore::KOCore()
    : mCalendarDecorationsLoaded(false), mIdentityManager(Q_NULLPTR)
{
}

KOCore::~KOCore()
{
    mSelf = Q_NULLPTR;
}

KService::List KOCore::availablePlugins(const QString &type, int version)
{
    QString constraint;
    if (version >= 0) {
        constraint =
            QStringLiteral("[X-KDE-PluginInterfaceVersion] == %1").arg(QString::number(version));
    }

    return KServiceTypeTrader::self()->query(type, constraint);
}

KService::List KOCore::availablePlugins()
{
    return availablePlugins(CalendarSupport::Plugin::serviceType(),
                            CalendarSupport::Plugin::interfaceVersion());
}

KService::List KOCore::availableCalendarDecorations()
{
    return availablePlugins(EventViews::CalendarDecoration::Decoration::serviceType(),
                            EventViews::CalendarDecoration::Decoration::interfaceVersion());
}

KService::List KOCore::availableParts()
{
    return availablePlugins(KOrg::Part::serviceType(), KOrg::Part::interfaceVersion());
}

CalendarSupport::Plugin *KOCore::loadPlugin(const KService::Ptr &service)
{
    qCDebug(KORGANIZER_LOG) << service->library();

    if (!service->hasServiceType(CalendarSupport::Plugin::serviceType())) {
        return Q_NULLPTR;
    }

    KPluginLoader loader(*service);
    auto factory = loader.instance();

    if (!factory) {
        qCDebug(KORGANIZER_LOG) << "Factory creation failed";
        return Q_NULLPTR;
    }

    auto pluginFactory = qobject_cast<CalendarSupport::PluginFactory *>(factory);

    if (!pluginFactory) {
        qCDebug(KORGANIZER_LOG) << "Cast failed";
        return Q_NULLPTR;
    }

    return pluginFactory->createPluginFactory();
}

CalendarSupport::Plugin *KOCore::loadPlugin(const QString &name)
{
    KService::List list = availablePlugins();
    KService::List::ConstIterator it;
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
        if ((*it)->desktopEntryName() == name) {
            return loadPlugin(*it);
        }
    }
    return Q_NULLPTR;
}

EventViews::CalendarDecoration::Decoration *KOCore::loadCalendarDecoration(const KService::Ptr &service)
{
    KPluginLoader loader(*service);
    auto factory = loader.instance();

    if (!factory) {
        qCDebug(KORGANIZER_LOG) << "Factory creation failed";
        return Q_NULLPTR;
    }

    auto pluginFactory = qobject_cast<EventViews::CalendarDecoration::DecorationFactory *>(factory);

    if (!pluginFactory) {
        qCDebug(KORGANIZER_LOG) << "Cast failed";
        return Q_NULLPTR;
    }

    return pluginFactory->createPluginFactory();
}

EventViews::CalendarDecoration::Decoration *KOCore::loadCalendarDecoration(const QString &name)
{
    KService::List list = availableCalendarDecorations();
    KService::List::ConstIterator it;
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
        if ((*it)->desktopEntryName() == name) {
            return loadCalendarDecoration(*it);
        }
    }
    return Q_NULLPTR;
}

KOrg::Part *KOCore::loadPart(const KService::Ptr &service, KOrg::MainWindow *parent)
{
    qCDebug(KORGANIZER_LOG) << service->library();

    if (!service->hasServiceType(KOrg::Part::serviceType())) {
        return Q_NULLPTR;
    }

    KPluginLoader loader(*service);
    KPluginFactory *factory = loader.factory();

    if (!factory) {
        qCDebug(KORGANIZER_LOG) << "Factory creation failed";
        return Q_NULLPTR;
    }

    KOrg::PartFactory *pluginFactory =
        static_cast<KOrg::PartFactory *>(factory);

    if (!pluginFactory) {
        qCDebug(KORGANIZER_LOG) << "Cast failed";
        return Q_NULLPTR;
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
        return Q_NULLPTR;
    }

    QWidget *topLevel = wdg->topLevelWidget();
    QMap<QWidget *, KXMLGUIClient *>::ConstIterator it = mXMLGUIClients.find(topLevel);
    if (it != mXMLGUIClients.constEnd()) {
        return it.value();
    }

    return Q_NULLPTR;
}

KOrg::Part *KOCore::loadPart(const QString &name, KOrg::MainWindow *parent)
{
    KService::List list = availableParts();
    KService::List::ConstIterator it;
    for (it = list.constBegin(); it != list.constEnd(); ++it) {
        if ((*it)->desktopEntryName() == name) {
            return loadPart(*it, parent);
        }
    }
    return Q_NULLPTR;
}

EventViews::CalendarDecoration::Decoration::List KOCore::loadCalendarDecorations()
{
    if (!mCalendarDecorationsLoaded) {
        QStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;

        mCalendarDecorations.clear();
        KService::List plugins = availableCalendarDecorations();
        KService::List::ConstIterator it;
        for (it = plugins.constBegin(); it != plugins.constEnd(); ++it) {
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

    QStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;

    KService::List plugins = availableParts();
    KService::List::ConstIterator it;
    for (it = plugins.constBegin(); it != plugins.constEnd(); ++it) {
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
    KOrg::Part::List list = loadParts(parent);

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
    if (!mIdentityManager) {
        mIdentityManager = new CalendarSupport::IdentityManager;
    }
    return mIdentityManager;
}
