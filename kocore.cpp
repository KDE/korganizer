// $Id$

#include <qwidget.h>

#include <klibloader.h>
#include <kdebug.h>

#include <calendar/plugin.h>
#include <korganizer/part.h>

#include <calendar.h>

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
  mHolidaysLoaded(false)
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
    kdDebug() << "KOCore::loadPlugin(): Cast failed" << endl; 
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

KOrg::Part *KOCore::loadPart(KService::Ptr service, CalendarView *view,
                             QWidget *parent)
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
  
  return pluginFactory->create(view,parent,0);
}

KOrg::Part *KOCore::loadPart(const QString &name,CalendarView *view,
                             QWidget *parent)
{
  KTrader::OfferList list = availablePlugins("KOrganizer/Part");
  KTrader::OfferList::ConstIterator it;
  for(it = list.begin(); it != list.end(); ++it) {
    if ((*it)->desktopEntryName() == name) {
      return loadPart(*it,view,parent);
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
