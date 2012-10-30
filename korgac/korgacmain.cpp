/*
  This file is part of the KDE reminder agent.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "koalarmclient.h"
#include "kdepim-version.h"

#include <KAboutData>
#include <KCmdLineArgs>
#include <KUniqueApplication>

#include <stdlib.h>

#ifdef SERIALIZER_PLUGIN_STATIC
#include <QtPlugin>

Q_IMPORT_PLUGIN(akonadi_serializer_kcalcore)
#endif

class ReminderDaemonApp : public KUniqueApplication
{
  public:
    ReminderDaemonApp() : mClient( 0 )
    {
      // ensure the Quit dialog's Cancel response does not close the app
      setQuitOnLastWindowClosed( false );
    }

    int newInstance()
    {
      // Check if we already have a running alarm daemon widget
      if ( mClient ) {
        return 0;
      }

      mClient = new KOAlarmClient;

      return 0;
    }

  private:
    KOAlarmClient *mClient;
};

static const char korgacVersion[] = KDEPIM_VERSION;

int main( int argc, char **argv )
{
  KAboutData aboutData( "korgac", "korganizer", ki18n( "KOrganizer Reminder Daemon" ),
                        korgacVersion, ki18n( "KOrganizer Reminder Daemon" ),
                        KAboutData::License_GPL,
                        ki18n( "(c) 2003 Cornelius Schumacher" ),
                        KLocalizedString(), "http://pim.kde.org" );
  aboutData.addAuthor( ki18n( "Cornelius Schumacher" ), ki18n( "Former Maintainer" ),
                       "schumacher@kde.org" );
  aboutData.addAuthor( ki18n( "Reinhold Kainhofer" ), ki18n ( "Former Maintainer" ),
                       "kainhofer@kde.org" );
  aboutData.addAuthor( ki18n( "Allen Winter" ),ki18n( "Janitorial Staff" ),
                       "winter@kde.org" );

  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
  KCmdLineArgs::addCmdLineOptions( options );
  KUniqueApplication::addCmdLineOptions();

  if ( !ReminderDaemonApp::start() ) {
    exit( 0 );
  }

  ReminderDaemonApp app;
#if !defined(Q_WS_WINCE)
  app.disableSessionManagement();
#endif

  return app.exec();
}
