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
#ifndef KOCORE_H
#define KOCORE_H

#include "korganizer_export.h"

#include <calendar/calendardecoration.h>
#include <korganizer/part.h>
#include <korganizer/printplugin.h>

#include <kservice.h>

namespace KPIMIdentities { class IdentityManager; }

class KORGANIZER_EXPORT KOCore
{
  public:
    ~KOCore();

    static KOCore *self();

    KService::List availablePlugins();
    KService::List availableCalendarDecorations();
    KService::List availableParts();
    KService::List availablePrintPlugins();

    KOrg::Plugin *loadPlugin( KService::Ptr service );
    KOrg::Plugin *loadPlugin( const QString & );

    KOrg::CalendarDecoration::Decoration *loadCalendarDecoration( KService::Ptr service );
    KOrg::CalendarDecoration::Decoration *loadCalendarDecoration( const QString & );

    KOrg::Part *loadPart( KService::Ptr, KOrg::MainWindow *parent );
    KOrg::Part *loadPart( const QString &, KOrg::MainWindow *parent );

    KOrg::PrintPlugin *loadPrintPlugin( KService::Ptr service );
    KOrg::PrintPlugin *loadPrintPlugin( const QString & );

    KOrg::CalendarDecoration::Decoration::List calendarDecorations();
    KOrg::PrintPlugin::List loadPrintPlugins();
    KOrg::Part::List loadParts( KOrg::MainWindow *parent );

    void addXMLGUIClient( QWidget*, KXMLGUIClient *guiclient );
    void removeXMLGUIClient( QWidget* );
    KXMLGUIClient *xmlguiClient( QWidget* ) const;

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
    KOrg::Part::List reloadParts( KOrg::MainWindow *parent,
                                  KOrg::Part::List &parts );

    KPIMIdentities::IdentityManager* identityManager();

  protected:
    KOCore();

    KService::List availablePlugins( const QString &type,
                                         int pluginInterfaceVersion = -1 );

  private:
    static KOCore *mSelf;

    KOrg::CalendarDecoration::Decoration::List mCalendarDecorations;
    bool mCalendarDecorationsLoaded;

    QMap<QWidget*, KXMLGUIClient*> mXMLGUIClients;

    KPIMIdentities::IdentityManager *mIdentityManager;
};

#endif
