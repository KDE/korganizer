/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <qwidget.h>

#include <klibloader.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kxmlguifactory.h>

#include <calendar/plugin.h>
#include <korganizer/part.h>

#include "koprefs.h"

#include "kocore.h"
#include "koglobals.h"

KOCore *KOCore::mSelf = 0;

KOCore *KOCore::self()
{
  if (!mSelf) {
    mSelf = new KOCore;
  }
  
  return mSelf;
}

KOCore::KOCore() :
  mCalendarDecorationsLoaded( false ), mHolidays( 0 )
{
}

KTrader::OfferList KOCore::availablePlugins(const QString &type)
{
  return KTrader::self()->query(type);
}

KOrg::Plugin *KOCore::loadPlugin(KService::Ptr service)
{
  kdDebug() << "loadPlugin: library: " << service->library() << endl;

  if ( !service->hasServiceType( "Calendar/Plugin" ) ) {
    return 0;
  }

  KLibFactory *factory = KLibLoader::self()->factory(service->library());

  if (!factory) {
    kdDebug() << "KOCore::loadPlugin(): Factory creation failed" << endl;
    return 0;
  }
  
  KOrg::PluginFactory *pluginFactory = static_cast<KOrg::PluginFactory *>(factory);
  
  if (!pluginFactory) {
    kdDebug() << "KOCore::loadPlugin(): Cast to KOrg::PluginFactory failed" << endl;
    return 0;
  }
  
  return pluginFactory->create();
}

KOrg::Plugin *KOCore::loadPlugin(const QString &name)
{
  KTrader::OfferList list = availablePlugins("Calendar/Plugin");
  KTrader::OfferList::ConstIterator it;
  for(it = list.begin(); it != list.end(); ++it) {
    if ((*it)->desktopEntryName() == name) {
      return loadPlugin(*it);
    }
  }
  return 0;
}

KOrg::CalendarDecoration *KOCore::loadCalendarDecoration(KService::Ptr service)
{
  kdDebug() << "loadCalendarDecoration: library: " << service->library() << endl;

  KLibFactory *factory = KLibLoader::self()->factory(service->library());

  if (!factory) {
    kdDebug() << "KOCore::loadCalendarDecoration(): Factory creation failed" << endl;
    return 0;
  }
  
  KOrg::CalendarDecorationFactory *pluginFactory =
      static_cast<KOrg::CalendarDecorationFactory *>(factory);
  
  if (!pluginFactory) {
    kdDebug() << "KOCore::loadCalendarDecoration(): Cast failed" << endl;
    return 0;
  }
  
  return pluginFactory->create();
}

KOrg::CalendarDecoration *KOCore::loadCalendarDecoration(const QString &name)
{
  KTrader::OfferList list = availablePlugins("Calendar/Decoration");
  KTrader::OfferList::ConstIterator it;
  for(it = list.begin(); it != list.end(); ++it) {
    if ((*it)->desktopEntryName() == name) {
      return loadCalendarDecoration(*it);
    }
  }
  return 0;  
}

KOrg::Part *KOCore::loadPart(KService::Ptr service, KOrg::MainWindow *parent)
{
  kdDebug() << "loadPart: library: " << service->library() << endl;

  if ( !service->hasServiceType( "KOrganizer/Part" ) ) {
    return 0;
  }

  KLibFactory *factory = KLibLoader::self()->factory(service->library());

  if (!factory) {
    kdDebug() << "KOCore::loadPart(): Factory creation failed" << endl;
    return 0;
  }
  
  KOrg::PartFactory *pluginFactory =
      static_cast<KOrg::PartFactory *>(factory);
  
  if (!pluginFactory) {
    kdDebug() << "KOCore::loadPart(): Cast failed" << endl;
    return 0;
  }
  
  return pluginFactory->create(parent);
}

KOrg::Part *KOCore::loadPart(const QString &name,KOrg::MainWindow *parent)
{
  KTrader::OfferList list = availablePlugins("KOrg::MainWindow/Part");
  KTrader::OfferList::ConstIterator it;
  for(it = list.begin(); it != list.end(); ++it) {
    if ((*it)->desktopEntryName() == name) {
      return loadPart(*it,parent);
    }
  }
  return 0;  
}

KOrg::CalendarDecoration::List KOCore::calendarDecorations()
{
  if (!mCalendarDecorationsLoaded) {
    QStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;

    mCalendarDecorations.clear();
    KTrader::OfferList plugins = availablePlugins("Calendar/Decoration");
    KTrader::OfferList::ConstIterator it;
    for(it = plugins.begin(); it != plugins.end(); ++it) {
      if ((*it)->hasServiceType("Calendar/Decoration")) {
        QString name = (*it)->desktopEntryName();
        if ( selectedPlugins.find( name ) != selectedPlugins.end() ) {
          KOrg::CalendarDecoration *d = loadCalendarDecoration(*it);
          mCalendarDecorations.append( d );
          if ( name == "holidays" ) mHolidays = d;
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

  KTrader::OfferList plugins = availablePlugins("KOrganizer/Part");
  KTrader::OfferList::ConstIterator it;
  for(it = plugins.begin(); it != plugins.end(); ++it) {
    if (selectedPlugins.find((*it)->desktopEntryName()) != selectedPlugins.end()) {
      KOrg::Part *part = loadPart(*it,parent);
      if ( part ) {
        parent->mainGuiFactory()->addClient( part );
        parts.append( part );
      }
    }
  }
  return parts;
}

void KOCore::unloadPlugins()
{
  KOrg::CalendarDecoration *plugin;
  for( plugin=mCalendarDecorations.first(); plugin; plugin=mCalendarDecorations.next() ) {    
    delete plugin;
  }
  mCalendarDecorations.clear();
  mCalendarDecorationsLoaded = false;
  mHolidays = 0;
}

void KOCore::unloadParts( KOrg::MainWindow *parent, KOrg::Part::List& parts )
{
  KOrg::Part *part;
  for( part=parts.first(); part; part=parts.next() ) {    
    parent->mainGuiFactory()->removeClient( part );
    delete part;
  }
  parts.clear();
}

KOrg::Part::List KOCore::reloadParts( KOrg::MainWindow *parent, KOrg::Part::List& parts )
{
  unloadParts( parent, parts );
  return loadParts( parent );
}

void KOCore::reloadPlugins()
{
  mCalendarDecorationsLoaded = false;
// Plugins should be unloaded, but e.g. komonthview keeps using the old ones
  unloadPlugins();
  calendarDecorations();
}

QString KOCore::holiday(const QDate &date)
{
  calendarDecorations();
  if (mHolidays)
    return mHolidays->shortText(date);
  else
    return QString::null;
}
