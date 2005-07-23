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

#include "koapp.h"
#include "aboutdata.h"

#include <kglobal.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "korganizer.h"
#include "korganizer_options.h"

int main ( int argc, char **argv )
{
  KOrg::AboutData aboutData;

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( korganizer_options );
  KUniqueApplication::addCmdLineOptions();

  if ( !KOrganizerApp::start() ) return 0;

  KOrganizerApp app;

  KGlobal::locale()->insertCatalogue( "libkcal" );
  KGlobal::locale()->insertCatalogue( "libkdepim" );
  KGlobal::locale()->insertCatalogue( "kdgantt" );

  if ( app.isRestored() ) {
    RESTORE( KOrganizer )
  }

  return app.exec();
}
