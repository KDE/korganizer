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
#include "koprefs.h"

#include <calendarsupport/identitymanager.h>

#include <KDebug>
#include <KServiceTypeTrader>
#include <KXMLGUIFactory>

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

KService::List KOCore::availablePlugins( const QString &type, int version )
{
  QString constraint;
  if ( version >= 0 ) {
    constraint =
      QString::fromLatin1( "[X-KDE-PluginInterfaceVersion] == %1" ).arg( QString::number( version ) );
  }

  return KServiceTypeTrader::self()->query( type, constraint );
}

KService::List KOCore::availablePlugins()
{
  return availablePlugins( CalendarSupport::Plugin::serviceType(),
                           CalendarSupport::Plugin::interfaceVersion() );
}

KService::List KOCore::availableCalendarDecorations()
{
  return availablePlugins( EventViews::CalendarDecoration::Decoration::serviceType(),
                           EventViews::CalendarDecoration::Decoration::interfaceVersion() );
}

KService::List KOCore::availableParts()
{
  return availablePlugins( KOrg::Part::serviceType(), KOrg::Part::interfaceVersion() );
}

KService::List KOCore::availablePrintPlugins()
{
  return
    availablePlugins( KOrg::PrintPlugin::serviceType(), KOrg::PrintPlugin::interfaceVersion() );
}

CalendarSupport::Plugin *KOCore::loadPlugin( KService::Ptr service )
{
  kDebug() << service->library();

  if ( !service->hasServiceType( CalendarSupport::Plugin::serviceType() ) ) {
    return 0;
  }

  KPluginLoader loader( *service );
  KPluginFactory *factory = loader.factory();

  if ( !factory ) {
    kDebug() << "Factory creation failed";
    return 0;
  }

  CalendarSupport::PluginFactory *pluginFactory =
    static_cast<CalendarSupport::PluginFactory *>( factory );

  if ( !pluginFactory ) {
    kDebug() << "Cast failed";
    return 0;
  }

  return pluginFactory->createPluginFactory();
}

CalendarSupport::Plugin *KOCore::loadPlugin( const QString &name )
{
  KService::List list = availablePlugins();
  KService::List::ConstIterator it;
  for ( it = list.constBegin(); it != list.constEnd(); ++it ) {
    if ( (*it)->desktopEntryName() == name ) {
      return loadPlugin( *it );
    }
  }
  return 0;
}

EventViews::CalendarDecoration::Decoration *KOCore::loadCalendarDecoration( KService::Ptr service )
{
  KPluginLoader loader( *service );
  KPluginFactory *factory = loader.factory();

  if ( !factory ) {
    kDebug() << "Factory creation failed";
    return 0;
  }

  EventViews::CalendarDecoration::DecorationFactory *pluginFactory =
      static_cast<EventViews::CalendarDecoration::DecorationFactory *>( factory );

  if ( !pluginFactory ) {
    kDebug() << "Cast failed";
    return 0;
  }

  return pluginFactory->createPluginFactory();
}

EventViews::CalendarDecoration::Decoration *KOCore::loadCalendarDecoration( const QString &name )
{
  KService::List list = availableCalendarDecorations();
  KService::List::ConstIterator it;
  for ( it = list.constBegin(); it != list.constEnd(); ++it ) {
    if ( (*it)->desktopEntryName() == name ) {
      return loadCalendarDecoration( *it );
    }
  }
  return 0;
}

KOrg::Part *KOCore::loadPart( KService::Ptr service, KOrg::MainWindow *parent )
{
  kDebug() << service->library();

  if ( !service->hasServiceType( KOrg::Part::serviceType() ) ) {
    return 0;
  }

  KPluginLoader loader( *service );
  KPluginFactory *factory = loader.factory();

  if ( !factory ) {
    kDebug() << "Factory creation failed";
    return 0;
  }

  KOrg::PartFactory *pluginFactory =
      static_cast<KOrg::PartFactory *>( factory );

  if ( !pluginFactory ) {
    kDebug() << "Cast failed";
    return 0;
  }

  return pluginFactory->createPluginFactory( parent );
}

KOrg::PrintPlugin *KOCore::loadPrintPlugin( KService::Ptr service )
{
  kDebug() << service->library();

  if ( !service->hasServiceType( KOrg::PrintPlugin::serviceType() ) ) {
    return 0;
  }

  KPluginLoader loader( *service );
  KPluginFactory *factory = loader.factory();

  if ( !factory ) {
    kDebug() << "Factory creation failed";
    return 0;
  }

  KOrg::PrintPluginFactory *pluginFactory =
      static_cast<KOrg::PrintPluginFactory *>( factory );

  if ( !pluginFactory ) {
    kDebug() << "Cast failed";
    return 0;
  }

  return pluginFactory->createPluginFactory();
}

void KOCore::addXMLGUIClient( QWidget *wdg, KXMLGUIClient *guiclient )
{
  mXMLGUIClients.insert( wdg, guiclient );
}

void KOCore::removeXMLGUIClient( QWidget *wdg )
{
  mXMLGUIClients.remove( wdg );
}

KXMLGUIClient *KOCore::xmlguiClient( QWidget *wdg ) const
{
  if ( !wdg ) {
    return 0;
  }

  QWidget *topLevel = wdg->topLevelWidget();
  QMap<QWidget*, KXMLGUIClient*>::ConstIterator it = mXMLGUIClients.find( topLevel );
  if ( it != mXMLGUIClients.constEnd() ) {
    return it.value();
  }

  return 0;
}

KOrg::Part *KOCore::loadPart( const QString &name, KOrg::MainWindow *parent )
{
  KService::List list = availableParts();
  KService::List::ConstIterator it;
  for ( it = list.constBegin(); it != list.constEnd(); ++it ) {
    if ( (*it)->desktopEntryName() == name ) {
      return loadPart( *it, parent );
    }
  }
  return 0;
}

KOrg::PrintPlugin *KOCore::loadPrintPlugin( const QString &name )
{
  KService::List list = availablePrintPlugins();
  KService::List::ConstIterator it;
  for ( it = list.constBegin(); it != list.constEnd(); ++it ) {
    if ( (*it)->desktopEntryName() == name ) {
      return loadPrintPlugin( *it );
    }
  }
  return 0;
}

EventViews::CalendarDecoration::Decoration::List KOCore::loadCalendarDecorations()
{
  if ( !mCalendarDecorationsLoaded ) {
    QStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;

    mCalendarDecorations.clear();
    KService::List plugins = availableCalendarDecorations();
    KService::List::ConstIterator it;
    for ( it = plugins.constBegin(); it != plugins.constEnd(); ++it ) {
      if ( (*it)->hasServiceType( EventViews::CalendarDecoration::Decoration::serviceType() ) ) {
        QString name = (*it)->desktopEntryName();
        if ( selectedPlugins.contains( name ) ) {
          EventViews::CalendarDecoration::Decoration *d = loadCalendarDecoration(*it);
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

  KService::List plugins = availableParts();
  KService::List::ConstIterator it;
  for ( it = plugins.constBegin(); it != plugins.constEnd(); ++it ) {
    if ( selectedPlugins.contains( (*it)->desktopEntryName() ) ) {
      KOrg::Part *part = loadPart( *it, parent );
      if ( part ) {
        if ( !parent->mainGuiClient() ) {
          kError() << "parent has no mainGuiClient.";
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

  EventViews::PrefsPtr viewPrefs = KOPrefs::instance()->eventViewsPreferences();
  QStringList selectedPlugins = viewPrefs->selectedPlugins();

  KService::List plugins = availablePrintPlugins();
  KService::List::ConstIterator it;
  for ( it = plugins.constBegin(); it != plugins.constEnd(); ++it ) {
    if ( selectedPlugins.contains( (*it)->desktopEntryName() ) ) {
      KOrg::PrintPlugin *part = loadPrintPlugin( *it );
      if ( part ) {
        loadedPlugins.append( part );
      }
    }
  }
  return loadedPlugins;
}

void KOCore::unloadPlugins()
{
  qDeleteAll( mCalendarDecorations );
  mCalendarDecorations.clear();
  mCalendarDecorationsLoaded = false;
}

void KOCore::unloadParts( KOrg::MainWindow *parent, KOrg::Part::List &parts )
{
  foreach ( KOrg::Part *part, parts ) {
    parent->mainGuiClient()->removeChildClient( part );
    delete part;
  }
  parts.clear();
}

KOrg::Part::List KOCore::reloadParts( KOrg::MainWindow *parent, KOrg::Part::List &parts )
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
  // TODO: does this still apply?
  // Plugins should be unloaded, but e.g. komonthview keeps using the old ones
  unloadPlugins();
  loadCalendarDecorations();
}

KPIMIdentities::IdentityManager *KOCore::identityManager()
{
  if ( !mIdentityManager ) {
    mIdentityManager = new CalendarSupport::IdentityManager;
  }
  return mIdentityManager;
}
