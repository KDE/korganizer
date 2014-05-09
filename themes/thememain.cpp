/*
    This file is part of KOrganizer.

    Copyright © 2007 Loïc Corbasson <loic.corbasson@gmail.com>

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

#include <KApplication>
#include <K4AboutData>
#include <KCmdLineArgs>
#include <KLocalizedString>
#include <QDebug>

#include "theme.h"

int main ( int argc, char **argv )
{
  K4AboutData aboutData( "korganizer",
                        0,
                        ki18n( "KOrganizer Theming Stub" ),
                        "0.001",
                        ki18n( "DO NOT USE - Stub doing various things with KOrganizer themes" ),
                        K4AboutData::License_GPL,
                        ki18n( "© 2007 Loïc Corbasson" ),
                        KLocalizedString(),
                        "http://blog.loic.corbasson.fr/",
                        "loic.corbasson@gmail.com" );

  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
  options.add( "+[url]", ki18n( "Theme to use" ) );
  KCmdLineArgs::addCmdLineOptions( options );

  KApplication app( false );   // no GUI

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if ( args->count() > 0 ) {
    for ( int i=0; i < args->count(); ++i ) {
      KOrg::Theme::useThemeFrom( args->url( i ) );
    }
  }
  args->clear();
}
