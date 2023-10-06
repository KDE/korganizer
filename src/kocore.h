/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "korganizer_core_export.h"

#include <KPluginMetaData>
#include <KXMLGUIClient>

#include <EventViews/CalendarDecoration>

namespace KIdentityManagementCore
{
class IdentityManager;
}

class KORGANIZER_CORE_EXPORT KOCore
{
public:
    ~KOCore();

    static KOCore *self();

    [[nodiscard]] QList<KPluginMetaData> availableCalendarDecorations();

    EventViews::CalendarDecoration::Decoration *loadCalendarDecoration(const KPluginMetaData &service);
    EventViews::CalendarDecoration::Decoration::List loadCalendarDecorations();

    void addXMLGUIClient(QWidget *, KXMLGUIClient *guiclient);
    void removeXMLGUIClient(QWidget *);
    KXMLGUIClient *xmlguiClient(QWidget *) const;

    void unloadPlugins();

    void reloadPlugins();

    KIdentityManagementCore::IdentityManager *identityManager();

protected:
    KOCore();

private:
    static KOCore *mSelf;

    EventViews::CalendarDecoration::Decoration::List mCalendarDecorations;
    bool mCalendarDecorationsLoaded = false;

    QMap<QWidget *, KXMLGUIClient *> mXMLGUIClients;
};
