/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOCORE_H
#define KOCORE_H

#include <calendar/calendardecoration.h>
#include <korganizer/part.h>
#include <korganizer/printplugin.h>

#include <ktrader.h>

namespace KPIM { class IdentityManager; }

class KOCore
{
  public:
    ~KOCore();

    static KOCore *self();

    KTrader::OfferList availablePlugins();
    KTrader::OfferList availableCalendarDecorations();
    KTrader::OfferList availableParts();
    KTrader::OfferList availablePrintPlugins();

    KOrg::Plugin *loadPlugin( KService::Ptr service );
    KOrg::Plugin *loadPlugin( const QString & );

    KOrg::CalendarDecoration *loadCalendarDecoration( KService::Ptr service );
    KOrg::CalendarDecoration *loadCalendarDecoration( const QString & );

    KOrg::Part *loadPart( KService::Ptr, KOrg::MainWindow *parent );
    KOrg::Part *loadPart( const QString &, KOrg::MainWindow *parent );
    
    KOrg::PrintPlugin *loadPrintPlugin( KService::Ptr service );
    KOrg::PrintPlugin *loadPrintPlugin( const QString & );

    KOrg::CalendarDecoration::List calendarDecorations();
    KOrg::PrintPlugin::List loadPrintPlugins();
    KOrg::Part::List loadParts( KOrg::MainWindow *parent );

    void addXMLGUIClient( QWidget*, KXMLGUIClient *guiclient );
    void removeXMLGUIClient( QWidget* );
    KXMLGUIClient *xmlguiClient( QWidget* ) const;

    /**
      Unload the parts in &p parts for this main window. Clears
      parts.
    */
    void unloadParts( KOrg::MainWindow *parent, KOrg::Part::List &parts );
    void unloadPlugins();

    void reloadPlugins();

    /**
      Unloads the parts from the main window. Loads the parts that
      are listed in KOPrefs and returns a list of these parts.
    */
    KOrg::Part::List reloadParts( KOrg::MainWindow *parent,
                                  KOrg::Part::List &parts );

    QString holiday( const QDate & );
    bool isWorkDay( const QDate & );

    KPIM::IdentityManager* identityManager();

  protected:
    KOCore();

    KTrader::OfferList availablePlugins( const QString &type,
                                         int pluginInterfaceVersion = -1 );

  private:
    static KOCore *mSelf;

    KOrg::CalendarDecoration::List mCalendarDecorations;
    bool mCalendarDecorationsLoaded;

    KOrg::CalendarDecoration *mHolidays;

    QMap<QWidget*, KXMLGUIClient*> mXMLGUIClients;

    KPIM::IdentityManager *mIdentityManager;
};

#endif
