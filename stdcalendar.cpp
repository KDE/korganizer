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

#include <kstaticdeleter.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurl.h>

using namespace KOrg;

static KStaticDeleter<StdCalendar> selfDeleter;

StdCalendar *StdCalendar::mSelf = 0;

StdCalendar *StdCalendar::self()
{
  if ( !mSelf ) {
    selfDeleter.setObject( mSelf, new StdCalendar() );
  }
  return mSelf;
}

StdCalendar::StdCalendar()
  : CalendarResources( KPimPrefs::timeSpec() )
{
  readConfig();

  KCal::CalendarResourceManager *manager = resourceManager();
  if ( manager->isEmpty() ) {
    KConfig _config( "korganizerrc" );
    KConfigGroup config(&_config, "General" );
    QString fileName = config.readPathEntry( "Active Calendar" );

    QString resourceName;
    QString resoruceType;
    KCal::ResourceCalendar *defaultResource = 0;
    if ( !fileName.isEmpty() ) {
      KUrl url( fileName );
      if ( url.isLocalFile() ) {
        kDebug(5850) << "Local resource at " << url << endl;
        defaultResource = manager->createResource( "file" );
        if ( defaultResource )
          defaultResource->setValue( "File", url.path() );
      } else {
        kDebug(5850) << "Remote Resource at " << url << endl;
        defaultResource = manager->createResource( "remote" );
        if ( defaultResource )
          defaultResource->setValue( "URL", url.url() );
      }
      resourceName = i18n( "Active Calendar" );
    }
    // No resource created, i.e. no path found in config => use default path
    if ( !defaultResource ) {
      fileName = KStandardDirs::locateLocal( "data", "korganizer/std.ics" );
      kDebug(5850) << "Creating new default local resource at " << fileName << endl;
      defaultResource = manager->createResource( "file" );
      if ( defaultResource )
        defaultResource->setValue( "File", fileName );
      resourceName = i18n( "Default Calendar" );
    }

    if ( defaultResource ) {
      defaultResource->setTimeSpec( KPimPrefs::timeSpec() );
      defaultResource->setResourceName( resourceName );
      manager->add( defaultResource );
      manager->setStandardResource( defaultResource );
    }

    // By default, also create a birthday resource
    KCal::ResourceCalendar *bdayResource = manager->createResource( "birthdays" );
    if ( bdayResource ) {
      kDebug(5850) << "Adding Birthdays resource" << endl;
      bdayResource->setTimeSpec( KPimPrefs::timeSpec() );
      bdayResource->setResourceName( i18n("Birthdays") );
      manager->add( bdayResource );
    } else {
      kDebug(5850) << "Unable to add a Birthdays resource" << endl;
    }
  }
}

StdCalendar::~StdCalendar()
{
  mSelf = 0;
}
