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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "stdcalendar.h"

#include <libkcal/resourcecalendar.h>
#include <libkcal/resourcelocal.h>
#include <kresources/remote/resourceremote.h>
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
  : CalendarResources( KPimPrefs::timezone() )
{
  readConfig();

  KCal::CalendarResourceManager *manager = resourceManager();
  if ( manager->isEmpty() ) {
    KConfig config( "korganizerrc" );
    config.setGroup( "General" );
    QString fileName = config.readPathEntry( "Active Calendar" );

    QString resourceName;
    KCal::ResourceCalendar *defaultResource = 0;
    if ( fileName.isEmpty() ) {
      fileName = locateLocal( "data", "korganizer/std.ics" );
      resourceName = i18n( "Default Calendar" );
      defaultResource = new KCal::ResourceLocal( fileName );
    } else {
      KURL url( fileName );
      if ( url.isLocalFile() ) {
        defaultResource = new KCal::ResourceLocal( url.path() );
      } else {
        kdDebug(5850) << "Remote Resource" << endl;
        defaultResource = new KCal::ResourceRemote( url );
      }
      resourceName = i18n( "Active Calendar" );
    }

    defaultResource->setTimeZoneId( KPimPrefs::timezone() );
    defaultResource->setResourceName( resourceName );

    manager->add( defaultResource );
    manager->setStandardResource( defaultResource );
  }
}

StdCalendar::~StdCalendar()
{
  mSelf = 0;
}
