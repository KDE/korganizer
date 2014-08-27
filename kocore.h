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

#ifndef KORG_KOCORE_H
#define KORG_KOCORE_H

#include "korganizer_core_export.h"
#include "korganizer/part.h"

#include <calendarviews/agenda/calendardecoration.h>

#include <KService>

namespace KIdentityManagement {
  class IdentityManager;
}

class KORGANIZER_CORE_EXPORT KOCore
{
  public:
    ~KOCore();

    static KOCore *self();

    KService::List availablePlugins();
    KService::List availableCalendarDecorations();
    KService::List availableParts();

    CalendarSupport::Plugin *loadPlugin( KService::Ptr service );
    CalendarSupport::Plugin *loadPlugin( const QString & );

    EventViews::CalendarDecoration::Decoration *loadCalendarDecoration( KService::Ptr service );
    EventViews::CalendarDecoration::Decoration *loadCalendarDecoration( const QString & );

    KOrg::Part *loadPart( KService::Ptr, KOrg::MainWindow *parent );
    KOrg::Part *loadPart( const QString &, KOrg::MainWindow *parent );

    EventViews::CalendarDecoration::Decoration::List loadCalendarDecorations();
    KOrg::Part::List loadParts( KOrg::MainWindow *parent );

    void addXMLGUIClient( QWidget *, KXMLGUIClient *guiclient );
    void removeXMLGUIClient( QWidget * );
    KXMLGUIClient *xmlguiClient( QWidget * ) const;

    /**
      Unload the parts in &p parts for this main window. Clears
      parts.
        @param parent the parent main window for all parts
        @param parts the list of parts to be undloaded
    */
    void unloadParts( KOrg::MainWindow *parent, KOrg::Part::List &parts );
    void unloadPlugins();

    void reloadPlugins();

    /**
      Unloads the parts from the main window. Loads the parts that
      are listed in KOPrefs and returns a list of these parts.
        @param parent the parent main window for all parts
        @param parts the list of parts to be reloaded
    */
    KOrg::Part::List reloadParts( KOrg::MainWindow *parent, KOrg::Part::List &parts );

    KIdentityManagement::IdentityManager *identityManager();

  protected:
    KOCore();
    KService::List availablePlugins( const QString &type, int pluginInterfaceVersion = -1 );

  private:
    static KOCore *mSelf;

    EventViews::CalendarDecoration::Decoration::List mCalendarDecorations;
    bool mCalendarDecorationsLoaded;

    QMap<QWidget*, KXMLGUIClient*> mXMLGUIClients;

    KIdentityManagement::IdentityManager *mIdentityManager;
};

#endif
