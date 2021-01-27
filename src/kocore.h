/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KORG_KOCORE_H
#define KORG_KOCORE_H

#include "korganizer_core_export.h"
#include "part.h"

#include <EventViews/CalendarDecoration>

#include <KService>

namespace KIdentityManagement
{
class IdentityManager;
}

class KORGANIZER_CORE_EXPORT KOCore
{
public:
    ~KOCore();

    static KOCore *self();

    Q_REQUIRED_RESULT KService::List availablePlugins();
    Q_REQUIRED_RESULT KService::List availableCalendarDecorations();
    Q_REQUIRED_RESULT KService::List availableParts();

    CalendarSupport::Plugin *loadPlugin(const KService::Ptr &service);
    CalendarSupport::Plugin *loadPlugin(const QString &);

    EventViews::CalendarDecoration::Decoration *loadCalendarDecoration(const KService::Ptr &service);
    EventViews::CalendarDecoration::Decoration *loadCalendarDecoration(const QString &);

    KOrg::Part *loadPart(const KService::Ptr &, KOrg::MainWindow *parent);
    KOrg::Part *loadPart(const QString &, KOrg::MainWindow *parent);

    Q_REQUIRED_RESULT EventViews::CalendarDecoration::Decoration::List loadCalendarDecorations();
    Q_REQUIRED_RESULT KOrg::Part::List loadParts(KOrg::MainWindow *parent);

    void addXMLGUIClient(QWidget *, KXMLGUIClient *guiclient);
    void removeXMLGUIClient(QWidget *);
    KXMLGUIClient *xmlguiClient(QWidget *) const;

    /**
      Unload the parts in &p parts for this main window. Clears
      parts.
        @param parent the parent main window for all parts
        @param parts the list of parts to be undloaded
    */
    void unloadParts(KOrg::MainWindow *parent, KOrg::Part::List &parts);
    void unloadPlugins();

    void reloadPlugins();

    /**
      Unloads the parts from the main window. Loads the parts that
      are listed in KOPrefs and returns a list of these parts.
        @param parent the parent main window for all parts
        @param parts the list of parts to be reloaded
    */
    KOrg::Part::List reloadParts(KOrg::MainWindow *parent, KOrg::Part::List &parts);

    KIdentityManagement::IdentityManager *identityManager();

protected:
    KOCore();
    KService::List availablePlugins(const QString &type, int pluginInterfaceVersion = -1);

private:
    static KOCore *mSelf;

    EventViews::CalendarDecoration::Decoration::List mCalendarDecorations;
    bool mCalendarDecorationsLoaded = false;

    QMap<QWidget *, KXMLGUIClient *> mXMLGUIClients;
};

#endif
