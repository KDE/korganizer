/*
  This file is part of libkcal.

  Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "stdcalendar.h"

#include <kcal/resourcecalendar.h>
#include <libkdepim/kpimprefs.h>

#include <k3staticdeleter.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurl.h>

using namespace KOrg;

static K3StaticDeleter<StdCalendar> selfDeleter;

StdCalendar *StdCalendar::mSelf = 0;

StdCalendar *StdCalendar::self()
{
  if ( !mSelf ) {
    selfDeleter.setObject( mSelf, new StdCalendar() );
  }
  return mSelf;
}

StdCalendar::StdCalendar()
  : CalendarResources( KPIM::KPimPrefs::timeSpec() )
{
  readConfig();

  KCal::CalendarResourceManager *manager = resourceManager();
  if ( manager->isEmpty() ) {
    KConfig _config( "korganizerrc" );
    KConfigGroup config(&_config, "General" );
    QString fileName = config.readPathEntry( "Active Calendar", QString() );

    QString resourceName;
    QString resoruceType;
    KCal::ResourceCalendar *defaultResource = 0;
    if ( !fileName.isEmpty() ) {
      KUrl url( fileName );
      if ( url.isLocalFile() ) {
        kDebug() << "Local resource at" << url;
        defaultResource = manager->createResource( "file" );
        if ( defaultResource ) {
          defaultResource->setValue( "File", url.toLocalFile() );
        }
      } else {
        kDebug() << "Remote Resource at" << url;
        defaultResource = manager->createResource( "remote" );
        if ( defaultResource ) {
          defaultResource->setValue( "URL", url.url() );
        }
      }
      resourceName = i18n( "Active Calendar" );
    }
    // No resource created, i.e. no path found in config => use default path
    if ( !defaultResource ) {
      fileName = KStandardDirs::locateLocal( "data", "korganizer/std.ics" );
      kDebug() << "Creating new default local resource at" << fileName;
      defaultResource = manager->createResource( "file" );
      if ( defaultResource ) {
        defaultResource->setValue( "File", fileName );
      }
      resourceName = i18n( "Default Calendar" );
    }

    if ( defaultResource ) {
      defaultResource->setTimeSpec( KPIM::KPimPrefs::timeSpec() );
      defaultResource->setResourceName( resourceName );
      manager->add( defaultResource );
      manager->setStandardResource( defaultResource );
    }

    // By default, also create a birthday resource
    KCal::ResourceCalendar *bdayResource = manager->createResource( "birthdays" );
    if ( bdayResource ) {
      kDebug() << "Adding Birthdays resource";
      bdayResource->setTimeSpec( KPIM::KPimPrefs::timeSpec() );
      bdayResource->setResourceName( i18n( "Birthdays" ) );
      manager->add( bdayResource );
    } else {
      kDebug() << "Unable to add a Birthdays resource";
    }
  }
}

StdCalendar::~StdCalendar()
{
  mSelf = 0;
}
