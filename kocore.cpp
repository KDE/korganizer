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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kocore.h"

#include "koprefs.h"
#include "koglobals.h"
#include "koidentitymanager.h"

#include <calendar/plugin.h>
#include <korganizer/part.h>

#include <klibloader.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kxmlguifactory.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include <qwidget.h>

KOCore *KOCore::mSelf = 0;

KOCore *KOCore::self()
{
  if ( !mSelf ) {
    mSelf = new KOCore;
  }

  return mSelf;
}

KOCore::KOCore()
  : mCalendarDecorationsLoaded( false ), mIdentityManager( 0 )
{
}

KOCore::~KOCore()
{
  mSelf = 0;
}

KTrader::OfferList KOCore::availablePlugins( const QString &type, int version )
{
  QString constraint;
  if ( version >= 0 ) {
    constraint = QString("[X-KDE-PluginInterfaceVersion] == %1")
                 .arg( QString::number( version ) );
  }

  return KTrader::self()->query( type, constraint );
}

KTrader::OfferList KOCore::availablePlugins()
{
  return availablePlugins( KOrg::Plugin::serviceType(),
                           KOrg::Plugin::interfaceVersion() );
}

KTrader::OfferList KOCore::availableCalendarDecorations()
{
  return availablePlugins( KOrg::CalendarDecoration::serviceType(),
                           KOrg::CalendarDecoration::interfaceVersion() );
}

KTrader::OfferList KOCore::availableParts()
{
  return availablePlugins( KOrg::Part::serviceType(),
                           KOrg::Part::interfaceVersion() );
}

KTrader::OfferList KOCore::availablePrintPlugins()
{
  return availablePlugins( KOrg::PrintPlugin::serviceType(),
                           KOrg::PrintPlugin::interfaceVersion() );
}

KOrg::Plugin *KOCore::loadPlugin( KService::Ptr service )
{
  kdDebug(5850) << "loadPlugin: library: " << service->library() << endl;

  if ( !service->hasServiceType( KOrg::Plugin::serviceType() ) ) {
    return 0;
  }

  KLibFactory *factory = KLibLoader::self()->factory(
      service->library().latin1() );

  if ( !factory ) {
    kdDebug(5850) << "KOCore::loadPlugin(): Factory creation failed" << endl;
    return 0;
  }

  KOrg::PluginFactory *pluginFactory =
      static_cast<KOrg::PluginFactory *>( factory );

  if ( !pluginFactory ) {
    kdDebug(5850) << "KOCore::loadPlugin(): Cast to KOrg::PluginFactory failed" << endl;
    return 0;
  }

  return pluginFactory->create();
}

KOrg::Plugin *KOCore::loadPlugin( const QString &name )
{
  KTrader::OfferList list = availablePlugins();
  KTrader::OfferList::ConstIterator it;
  for( it = list.begin(); it != list.end(); ++it ) {
    if ( (*it)->desktopEntryName() == name ) {
      return loadPlugin( *it );
    }
  }
  return 0;
}

KOrg::CalendarDecoration *KOCore::loadCalendarDecoration(KService::Ptr service)
{
  kdDebug(5850) << "loadCalendarDecoration: library: " << service->library() << endl;

  KLibFactory *factory = KLibLoader::self()->factory(service->library().latin1());

  if (!factory) {
    kdDebug(5850) << "KOCore::loadCalendarDecoration(): Factory creation failed" << endl;
    return 0;
  }

  KOrg::CalendarDecorationFactory *pluginFactory =
      static_cast<KOrg::CalendarDecorationFactory *>(factory);

  if (!pluginFactory) {
    kdDebug(5850) << "KOCore::loadCalendarDecoration(): Cast failed" << endl;
    return 0;
  }

  return pluginFactory->create();
}

KOrg::CalendarDecoration *KOCore::loadCalendarDecoration( const QString &name )
{
  KTrader::OfferList list = availableCalendarDecorations();
  KTrader::OfferList::ConstIterator it;
  for( it = list.begin(); it != list.end(); ++it ) {
    if ( (*it)->desktopEntryName() == name ) {
      return loadCalendarDecoration( *it );
    }
  }
  return 0;
}

KOrg::Part *KOCore::loadPart( KService::Ptr service, KOrg::MainWindow *parent )
{
  kdDebug(5850) << "loadPart: library: " << service->library() << endl;

  if ( !service->hasServiceType( KOrg::Part::serviceType() ) ) {
    return 0;
  }

  KLibFactory *factory = KLibLoader::self()->factory(
      service->library().latin1() );

  if ( !factory ) {
    kdDebug(5850) << "KOCore::loadPart(): Factory creation failed" << endl;
    return 0;
  }

  KOrg::PartFactory *pluginFactory =
      static_cast<KOrg::PartFactory *>( factory );

  if ( !pluginFactory ) {
    kdDebug(5850) << "KOCore::loadPart(): Cast failed" << endl;
    return 0;
  }

  return pluginFactory->create( parent );
}

KOrg::PrintPlugin *KOCore::loadPrintPlugin( KService::Ptr service )
{
  kdDebug(5850) << "loadPart: print plugin in library: " << service->library() << endl;

  if ( !service->hasServiceType( KOrg::PrintPlugin::serviceType() ) ) {
    return 0;
  }

  KLibFactory *factory = KLibLoader::self()->factory(
      service->library().latin1() );

  if ( !factory ) {
    kdDebug(5850) << "KOCore::loadPrintPlugin(): Factory creation failed" << endl;
    return 0;
  }

  KOrg::PrintPluginFactory *pluginFactory =
      static_cast<KOrg::PrintPluginFactory *>( factory );

  if ( !pluginFactory ) {
    kdDebug(5850) << "KOCore::loadPrintPlugins(): Cast failed" << endl;
    return 0;
  }

  return pluginFactory->create();
}

void KOCore::addXMLGUIClient( QWidget *wdg, KXMLGUIClient *guiclient )
{
  mXMLGUIClients.insert( wdg, guiclient );
}

void KOCore::removeXMLGUIClient( QWidget *wdg )
{
  mXMLGUIClients.remove( wdg );
}

KXMLGUIClient* KOCore::xmlguiClient( QWidget *wdg ) const
{
  QWidget *topLevel = wdg->topLevelWidget();
  QMap<QWidget*, KXMLGUIClient*>::ConstIterator it = mXMLGUIClients.find( topLevel );
  if ( it != mXMLGUIClients.end() )
    return it.data();

  return 0;
}

KOrg::Part *KOCore::loadPart( const QString &name, KOrg::MainWindow *parent )
{
  KTrader::OfferList list = availableParts();
  KTrader::OfferList::ConstIterator it;
  for( it = list.begin(); it != list.end(); ++it ) {
    if ( (*it)->desktopEntryName() == name ) {
      return loadPart( *it, parent );
    }
  }
  return 0;
}

KOrg::PrintPlugin *KOCore::loadPrintPlugin( const QString &name )
{
  KTrader::OfferList list = availablePrintPlugins();
  KTrader::OfferList::ConstIterator it;
  for( it = list.begin(); it != list.end(); ++it ) {
    if ( (*it)->desktopEntryName() == name ) {
      return loadPrintPlugin( *it );
    }
  }
  return 0;
}

KOrg::CalendarDecoration::List KOCore::calendarDecorations()
{
  if ( !mCalendarDecorationsLoaded ) {
    QStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;

    mCalendarDecorations.clear();
    KTrader::OfferList plugins = availableCalendarDecorations();
    KTrader::OfferList::ConstIterator it;
    for( it = plugins.begin(); it != plugins.end(); ++it ) {
      if ( (*it)->hasServiceType("Calendar/Decoration") ) {
        QString name = (*it)->desktopEntryName();
        if ( selectedPlugins.find( name ) != selectedPlugins.end() ) {
          KOrg::CalendarDecoration *d = loadCalendarDecoration(*it);
          mCalendarDecorations.append( d );
        }
      }
    }
    mCalendarDecorationsLoaded = true;
  }

  return mCalendarDecorations;
}

KOrg::Part::List KOCore::loadParts( KOrg::MainWindow *parent )
{
  KOrg::Part::List parts;

  QStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;

  KTrader::OfferList plugins = availableParts();
  KTrader::OfferList::ConstIterator it;
  for( it = plugins.begin(); it != plugins.end(); ++it ) {
    if ( selectedPlugins.find( (*it)->desktopEntryName() ) !=
                               selectedPlugins.end() ) {
      KOrg::Part *part = loadPart( *it, parent );
      if ( part ) {
        if ( !parent->mainGuiClient() ) {
          kdError() << "KOCore::loadParts(): parent has no mainGuiClient."
                    << endl;
        } else {
          parent->mainGuiClient()->insertChildClient( part );
          parts.append( part );
        }
      }
    }
  }
  return parts;
}

KOrg::PrintPlugin::List KOCore::loadPrintPlugins()
{
  KOrg::PrintPlugin::List loadedPlugins;

  QStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;

  KTrader::OfferList plugins = availablePrintPlugins();
  KTrader::OfferList::ConstIterator it;
  for( it = plugins.begin(); it != plugins.end(); ++it ) {
    if ( selectedPlugins.find( (*it)->desktopEntryName() ) !=
                               selectedPlugins.end() ) {
      KOrg::PrintPlugin *part = loadPrintPlugin( *it );
      if ( part ) loadedPlugins.append( part );
    }
  }
  return loadedPlugins;
}

void KOCore::unloadPlugins()
{
  KOrg::CalendarDecoration *plugin;
  for( plugin = mCalendarDecorations.first(); plugin;
       plugin = mCalendarDecorations.next() ) {
    delete plugin;
  }
  mCalendarDecorations.clear();
  mCalendarDecorationsLoaded = false;
}

void KOCore::unloadParts( KOrg::MainWindow *parent, KOrg::Part::List &parts )
{
  KOrg::Part *part;
  for( part = parts.first(); part; part = parts.next() ) {
    parent->mainGuiClient()->removeChildClient( part );
    delete part;
  }
  parts.clear();
}

KOrg::Part::List KOCore::reloadParts( KOrg::MainWindow *parent,
                                      KOrg::Part::List &parts )
{
  KXMLGUIFactory *factory = parent->mainGuiClient()->factory();
  factory->removeClient( parent->mainGuiClient() );

  unloadParts( parent, parts );
  KOrg::Part::List list = loadParts( parent );

  factory->addClient( parent->mainGuiClient() );

  return list;
}

void KOCore::reloadPlugins()
{
  mCalendarDecorationsLoaded = false;
// Plugins should be unloaded, but e.g. komonthview keeps using the old ones
  unloadPlugins();
  calendarDecorations();
}

KPIM::IdentityManager* KOCore::identityManager()
{
  if ( !mIdentityManager )
    mIdentityManager = new KOrg::IdentityManager;
  return mIdentityManager;
}
