/*
  This file is part of KOrganizer.

  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "koapp.h"
#include "korganizer.h"
#include "koprefs.h"
#include "version.h"
#include "koglobals.h"
#include "actionmanager.h"
#include "importdialog.h"
#include "kocore.h"
#include "calendarview.h"
#include "stdcalendar.h"

#include <libkdepim/reminderclient.h>

#include <kcal/calformat.h>
#include <kcal/calendarresources.h>

#include <kstartupinfo.h>
#include <kglobal.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kwindowsystem.h>
#include <kurl.h>
#include <kstandarddirs.h>

#include <stdlib.h>
#include <iostream>

using namespace std;

KOrganizerApp::KOrganizerApp() : KPIM::PimApplication()
{
  QString prodId = "-//K Desktop Environment//NONSGML KOrganizer %1//EN";
  CalFormat::setApplication( "KOrganizer", prodId.arg( korgVersion ) );

  // icons shared by the KDE PIM applications
  KGlobal::dirs()->addResourceType( "appicon", "data", "/kdepim/icons/" );
}

KOrganizerApp::~KOrganizerApp()
{
}

int KOrganizerApp::newInstance()
{
  kDebug();
  static bool first = true;
  if ( isSessionRestored() && first ) {
     KOrg::MainWindow *korg = ActionManager::findInstance( KUrl() );
     if ( korg ) {
       KOrg::StdCalendar::self()->load();
       korg->view()->updateCategories();
       korg->view()->updateView();
     }
     first = false;
     return 0;
  }
  first = false;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

  KOGlobals::self()->reminderClient()->startDaemon();

  // No filenames given => all other args are meaningless, show main Window
  if ( args->count() <= 0 ) {
    processCalendar( KUrl() );
    return 0;
  }

  // If filenames were given as arguments, load them as calendars, one per window.
  if ( args->isSet( "open" ) ) {
    for ( int i = 0; i < args->count(); ++i ) {
      processCalendar( args->url( i ) );
    }
  } else {
    // Import, merge, or ask => we need the resource calendar window anyway.
    processCalendar( KUrl() );
    KOrg::MainWindow *korg = ActionManager::findInstance( KUrl() );
    if ( !korg ) {
      kError() << "Unable to find default calendar resources view.";
      return -1;
    }
    // Check for import, merge or ask
    if ( args->isSet( "import" ) ) {
      for ( int i = 0; i < args->count(); ++i ) {
        korg->actionManager()->addResource( args->url( i ) );
      }
    } else if ( args->isSet( "merge" ) ) {
      for ( int i = 0; i < args->count(); ++i ) {
        korg->actionManager()->mergeURL( args->url( i ).url() );
      }
    } else {
      for ( int i = 0; i < args->count(); ++i ) {
        korg->actionManager()->importCalendar( args->url( i ) );
      }
    }
  }

  kDebug() << "done";

  return 0;
}

void KOrganizerApp::processCalendar( const KUrl &url )
{
  KOrg::MainWindow *korg = ActionManager::findInstance( url );
  if ( !korg ) {
    bool hasDocument = !url.isEmpty();
    korg = new KOrganizer();
    korg->init( hasDocument );
    korg->topLevelWidget()->show();

    kDebug() << url.url();

    if ( hasDocument ) {
      korg->openURL( url );
    } else {
      KOrg::StdCalendar::self()->load();
      korg->view()->updateCategories();
      korg->view()->updateView();
    }
  } else {
    korg->topLevelWidget()->show();
  }

  // Handle window activation
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
  KStartupInfo::setNewStartupId( korg->topLevelWidget(), startupId() );
#endif
}

#include "koapp.moc"
