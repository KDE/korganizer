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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <stdlib.h>
#include <iostream>

#include <kglobal.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kwin.h>
#include <kurl.h>

#include <libkcal/calformat.h>
#include <libkcal/calendarresources.h>

#include "korganizer.h"
#include "koprefs.h"
#include "version.h"
#include "alarmclient.h"
#include "koglobals.h"
#include "actionmanager.h"
#include "importdialog.h"
#include "kocore.h"
#include "calendarview.h"
#include "stdcalendar.h"

#include "koapp.h"
#include <kstartupinfo.h>

using namespace std;

KOrganizerApp::KOrganizerApp() : KUniqueApplication()
{
  QString prodId = "-//K Desktop Environment//NONSGML KOrganizer %1//EN";
  CalFormat::setApplication( "KOrganizer", prodId.arg( korgVersion ) );
}

KOrganizerApp::~KOrganizerApp()
{
}

int KOrganizerApp::newInstance()
{
  kdDebug(5850) << "KOApp::newInstance()" << endl;
  static bool first = true;
  if ( isRestored() && first ) {
     KOrg::MainWindow *korg = ActionManager::findInstance( KURL() );
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

  KOGlobals::self()->alarmClient()->startDaemon();

  // No filenames given => all other args are meaningless, show main Window
  if ( args->count() <= 0 ) {
    processCalendar( KURL() );
    return 0;
  }
  
  // If filenames wer given as arguments, load them as calendars, one per window.
  if ( args->isSet( "open" ) ) {
    for( int i = 0; i < args->count(); ++i ) {
      processCalendar( args->url( i ) );
    }
  } else {
    // Import, merge, or ask => we need the resource calendar window anyway.
    processCalendar( KURL() );
    KOrg::MainWindow *korg = ActionManager::findInstance( KURL() );
    if ( !korg ) {
      kdError() << "Unable to find default calendar resources view." << endl;
      return -1;
    }
    // Check for import, merge or ask
    if ( args->isSet( "import" ) ) {
      for( int i = 0; i < args->count(); ++i ) {
        korg->actionManager()->addResource( args->url( i ) );
      }
    } else if ( args->isSet( "merge" ) ) {
      for( int i = 0; i < args->count(); ++i ) {
        korg->actionManager()->mergeURL( args->url( i ).url() );
      }
    } else {
      for( int i = 0; i < args->count(); ++i ) {
        korg->actionManager()->importCalendar( args->url( i ) );
      }
    }
  }

  kdDebug(5850) << "KOApp::newInstance() done" << endl;

  return 0;
}


void KOrganizerApp::processCalendar( const KURL &url )
{
  KOrg::MainWindow *korg = ActionManager::findInstance( url );
  if ( !korg ) {
    bool hasDocument = !url.isEmpty();
    korg = new KOrganizer( "KOrganizer MainWindow" );
    korg->init( hasDocument );
    korg->topLevelWidget()->show();

    kdDebug(5850) << "KOrganizerApp::processCalendar(): '" << url.url()
                  << "'" << endl;

    if ( hasDocument )
      korg->openURL( url );
    else {
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
