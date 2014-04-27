/*
  This file is part of KOrganizer.

  Copyright (c) 1997-1999 Preston Brown <pbrown@kde.org>
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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "aboutdata.h"
#include "koapp.h"
#include "korganizer.h"
#include "korganizer_options.h"

#include <KGlobal>

int main( int argc, char **argv )
{
  KOrg::AboutData aboutData;

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( korganizer_options() );
  KUniqueApplication::addCmdLineOptions();

  KUniqueApplication::StartFlags flags;
  //flags |= KUniqueApplication::NonUniqueInstance;
  if ( !KOrganizerApp::start(flags) ) {
    return 0;
  }

  KOrganizerApp app;

  KGlobal::locale()->insertCatalog( QLatin1String("libkcalutils") );
  KGlobal::locale()->insertCatalog( QLatin1String("calendarsupport") );
  KGlobal::locale()->insertCatalog( QLatin1String("libkdepim") );
  KGlobal::locale()->insertCatalog( QLatin1String("kdgantt2") );
  KGlobal::locale()->insertCatalog( QLatin1String("libakonadi") );
  KGlobal::locale()->insertCatalog( QLatin1String("libincidenceeditors") );
  KGlobal::locale()->insertCatalog( QLatin1String("libkpimutils") );
  KGlobal::locale()->insertCatalog( QLatin1String("libpimcommon") );

  if ( app.isSessionRestored() ) {
    RESTORE( KOrganizer )
  }

  return app.exec();
}
