/*
    This file is part of the KOrganizer alarm client.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <stdlib.h>

#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kuniqueapplication.h>

#include "koalarmclient.h"

class MyApp : public KUniqueApplication
{
  public:
    MyApp() : mClient( 0 ) {}
    int newInstance()
    {
      // Check if we already have a running alarm daemon widget
      if ( mClient ) return 0;

      mClient = new KOAlarmClient;

      return 0;
    }

  private:
    KOAlarmClient *mClient;
};


static const char korgacVersion[] = "0.9";

static const KCmdLineOptions options[] =
{
  { 0, 0, 0 }
};

int main( int argc, char **argv )
{
  KLocale::setMainCatalogue( "korgac" );
  KAboutData aboutData( "korgac", I18N_NOOP("KOrganizer Alarm Daemon"),
                        korgacVersion, I18N_NOOP("KOrganizer Alarm Daemon"),
                        KAboutData::License_GPL,
                        "(c) 2003 Cornelius Schumacher",
                        0, "http://pim.kde.org" );
  aboutData.addAuthor( "Cornelius Schumacher", I18N_NOOP("Maintainer"),
                       "schumacher@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );
  KUniqueApplication::addCmdLineOptions();

  if ( !MyApp::start() ) exit( 0 );

  MyApp app;
  app.disableSessionManagement();

  return app.exec();
}
