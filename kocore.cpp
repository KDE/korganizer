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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$

#include <qwidget.h>

#include <klibloader.h>
#include <kdebug.h>

#include <calendar/plugin.h>
#include <korganizer/part.h>

#include "koprefs.h"

#include "kocore.h"

KOCore *KOCore::mSelf = 0;

KOCore *KOCore::self()
{
  if (!mSelf) {
    mSelf = new KOCore;
  }
  
  return mSelf;
}

KOCore::KOCore() :
  mTextDecorationsLoaded(false), mWidgetDecorationsLoaded(false),
  mPartsLoaded(false), mHolidaysLoaded(false)
{
}

KTrader::OfferList KOCore::availablePlugins(const QString &type)
{
  return KTrader::self()->query(type);
}

KOrg::Plugin *KOCore::loadPlugin(KService::Ptr service)
{
  kdDebug() << "loadPlugin: library: " << service->library() << endl;

  KLibFactory *factory = KLibLoader::self()->factory(service->library());

  if (!factory) {
    kdDebug() << "KOCore::loadPlugin(): Factory creation failed" << endl;
    return 0;
  }
  
  KOrg::PluginFactory *pluginFactory = dynamic_cast<KOrg::PluginFactory *>(factory);
  
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

KOrg::TextDecoration *KOCore::loadTextDecoration(KService::Ptr service)
{
  kdDebug() << "loadTextDecoration: library: " << service->library() << endl;

  KLibFactory *factory = KLibLoader::self()->factory(service->library());

  if (!factory) {
    kdDebug() << "KOCore::loadTextDecoration(): Factory creation failed" << endl;
    return 0;
  }
  
  KOrg::TextDecorationFactory *pluginFactory =
      dynamic_cast<KOrg::TextDecorationFactory *>(factory);
  
  if (!pluginFactory) {
    kdDebug() << "KOCore::loadTextDecoration(): Cast failed" << endl;
    return 0;
  }
  
  return pluginFactory->create();
}

KOrg::WidgetDecoration *KOCore::loadWidgetDecoration(KService::Ptr service)
{
  kdDebug() << "loadWidgetDecoration: library: " << service->library() << endl;

  KLibFactory *factory = KLibLoader::self()->factory(service->library());

  if (!factory) {
    kdDebug() << "KOCore::loadWidgetDecoration(): Factory creation failed" << endl;
    return 0;
  }
  
  KOrg::WidgetDecorationFactory *pluginFactory =
      dynamic_cast<KOrg::WidgetDecorationFactory *>(factory);
  
  if (!pluginFactory) {
    kdDebug() << "KOCore::loadWidgetDecoration(): Cast failed" << endl;
    return 0;
  }
  
  return pluginFactory->create();
}

KOrg::WidgetDecoration *KOCore::loadWidgetDecoration(const QString &name)
{
  KTrader::OfferList list = availablePlugins("Calendar/WidgetDecoration");
  KTrader::OfferList::ConstIterator it;
  for(it = list.begin(); it != list.end(); ++it) {
    if ((*it)->desktopEntryName() == name) {
      return loadWidgetDecoration(*it);
    }
  }
  return 0;  
}

KOrg::Part *KOCore::loadPart(KService::Ptr service, KOrg::MainWindow *parent)
{
  kdDebug() << "loadPart: library: " << service->library() << endl;

  KLibFactory *factory = KLibLoader::self()->factory(service->library());

  if (!factory) {
    kdDebug() << "KOCore::loadPart(): Factory creation failed" << endl;
    return 0;
  }
  
  KOrg::PartFactory *pluginFactory =
      dynamic_cast<KOrg::PartFactory *>(factory);
  
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

KOrg::TextDecoration::List KOCore::textDecorations()
{
  if (!mTextDecorationsLoaded) {
    QStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;

    mTextDecorations.clear();
    KTrader::OfferList plugins = availablePlugins("Calendar/TextDecoration");
    KTrader::OfferList::ConstIterator it;
    for(it = plugins.begin(); it != plugins.end(); ++it) {
      if ((*it)->hasServiceType("Calendar/TextDecoration")) {
        if (selectedPlugins.find((*it)->desktopEntryName()) != selectedPlugins.end()) {
          mTextDecorations.append(loadTextDecoration(*it));
        }
      }
    }
    mTextDecorationsLoaded = true;
  }
  
  return mTextDecorations;
}

KOrg::WidgetDecoration::List KOCore::widgetDecorations()
{
  if (!mWidgetDecorationsLoaded) {
    QStringList selectedPlugins = KOPrefs::instance()->mSelectedPlugins;

    mWidgetDecorations.clear();
    KTrader::OfferList plugins = availablePlugins("Calendar/WidgetDecoration");
    KTrader::OfferList::ConstIterator it;
    for(it = plugins.begin(); it != plugins.end(); ++it) {
      if ((*it)->hasServiceType("Calendar/WidgetDecoration")) {
        if (selectedPlugins.find((*it)->desktopEntryName()) != selectedPlugins.end()) {
          mWidgetDecorations.append(loadWidgetDecoration(*it));
        }
      }
    }
    mWidgetDecorationsLoaded = true;
  }
  
  return mWidgetDecorations;
}

KOrg::Part::List KOCore::parts(KOrg::MainWindow *parent)
{
  if (!mPartsLoaded) {
    mParts.clear();
    KTrader::OfferList plugins = availablePlugins("KOrganizer/Part");
    KTrader::OfferList::ConstIterator it;
    for(it = plugins.begin(); it != plugins.end(); ++it) {
      mParts.append(loadPart(*it,parent));
    }
    mPartsLoaded = true;
  }
  
  return mParts;
}

void KOCore::reloadPlugins()
{
  mTextDecorationsLoaded = false;
  mWidgetDecorationsLoaded = false;
  textDecorations();
  widgetDecorations();
}

QString KOCore::holiday(const QDate &date)
{
  if (!mHolidaysLoaded) {
    mHolidays = dynamic_cast<KOrg::TextDecoration *>(loadPlugin("holidays"));
    mHolidaysLoaded = true;
  }
  
  if (mHolidays)
    return mHolidays->dayShort(date);
  else
    return QString::null;
}
