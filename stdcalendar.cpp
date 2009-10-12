/*
  This file is part of KOrganizer.

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
#include "akonadicalendar.h"

#include <KDebug>
#include <K3StaticDeleter>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KUrl>
#include <ksystemtimezone.h>

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
  : AkonadiCalendar( KSystemTimeZones::local() )
{
}

StdCalendar::~StdCalendar()
{
  mSelf = 0;
}
