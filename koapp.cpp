/*
    This file is part of KOrganizer.
    Copyright (c) 1999 Preston Brown
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
#include <iostream>

#include <kglobal.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kwin.h>
#include <kurl.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/filestorage.h>
#include <libkcal/calformat.h>

#include "korganizer.h"
#include "koprefs.h"
#include "version.h"
#include "alarmclient.h"
#include "koglobals.h"
#include "actionmanager.h"

#include "koapp.h"
#include "koapp.moc"

using namespace std;

KOrganizerApp::KOrganizerApp() : KUniqueApplication()
{
  QString prodId = "-//K Desktop Environment//NONSGML KOrganizer %1//EN";
  CalFormat::setApplication( "KOrganizer", prodId.arg( korgVersion ) );
}

KOrganizerApp::~KOrganizerApp()
{
}

void KOrganizerApp::displayImminent( const KURL &url, int numdays )
{
  if (!url.isLocalFile()) {
    cerr << i18n("Unable to handle remote calendar.").local8Bit() << endl;
    return;
  }

  CalendarLocal cal( KOPrefs::instance()->mTimeZoneId );

  QDate currDate(QDate::currentDate());
  Event *currEvent;

  FileStorage storage( &cal, url.path() );

  if ( !storage.load() ) {
    cerr << i18n("Could not load calendar '%1'.").arg(url.path()).local8Bit()
         << endl;
    exit(0);
  }

  for (int i = 1; i <= numdays; i++) {
    cout << KGlobal::locale()->formatDate(currDate).local8Bit() << endl;

    QPtrList<Event> tmpList( cal.events( currDate, true ) );
    cout << "---------------------------------------------------------------"
         << endl;
    if (tmpList.count() > 0) {
      for (currEvent = tmpList.first(); currEvent; currEvent = tmpList.next()) {
        cout << currEvent->summary().local8Bit() << endl;
        if (!currEvent->doesFloat()) {
          cout << " (" << currEvent->dtStartStr().local8Bit() << " - "
               << currEvent->dtEndStr().local8Bit() << ")" << endl;
        }
        cout << endl;
      }
    } else {
      cout << i18n("(no events)").local8Bit() << endl;
    }

    cout << "---------------------------------------------------------------"
         << endl;
    QPtrList<Todo> tmpList2 = cal.todos(currDate);
    Todo *currTodo;
    if (tmpList.count() > 0) {
      for (currTodo = tmpList2.first(); currTodo; currTodo = tmpList2.next()) {
        cout << currTodo->summary().local8Bit() << endl;
        if (!currTodo->doesFloat()) {
          cout << " (" << currTodo->dtDueStr().local8Bit() << ")" << endl;
        }
        cout << endl;
      }
    } else {
      cout << i18n("(no todos)").local8Bit() << endl;
    }

    cout << endl;
    currDate = currDate.addDays(1);
  }
}


int KOrganizerApp::newInstance()
{
  kdDebug(5850) << "KOApp::newInstance()" << endl;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  
  // process command line options
  int numDays = 0;
  if (args->isSet("list")) {
    numDays = 1;
  } else if (args->isSet("show")) {
    numDays = args->getOption("show").toInt();
  } else {
    KOGlobals::self()->alarmClient()->startDaemon();
  }

  // If filenames was given as argument load this as calendars, one per window.
  if (args->count() > 0) {
    int i;
    for(i=0;i<args->count();++i) {
      processCalendar( args->url(i), numDays ); 
    }
  } else {
// Don't load active calendar. This is now handled by the CalendarResources.
// TODO: Fix alarm daemon to also use CalendarResources.
#if 0
    KConfig *config = KOGlobals::config();
    config->setGroup("General");
    QString urlString = config->readEntry("Active Calendar");

    // Force alarm daemon to load active calendar
    KOGlobals::self()->alarmClient()->addCalendar( urlString );
#endif
    processCalendar( KURL(), numDays ); 
  }
  
  kdDebug(5850) << "KOApp::newInstance() done" << endl;
  return 0;
}


void KOrganizerApp::processCalendar( const KURL &url, int numDays )
{
  if (numDays > 0) {
    displayImminent( url, numDays );
  } else {
    if (isRestored()) {
      RESTORE( KOrganizer( true ) )
    } else {
      KOrg::MainWindow *korg=ActionManager::findInstance(url);
      if (0 == korg) {
        bool hasDocument = !url.isEmpty();
        korg = new KOrganizer( hasDocument, "KOrganizer MainWindow" );
        korg->topLevelWidget()->show();

        kdDebug(5850) << "KOrganizerApp::processCalendar(): '" << url.url()
                  << "'" << endl;

        if ( hasDocument ) korg->openURL(url);
      } else
          KWin::setActiveWindow(korg->topLevelWidget()->winId());
    }
  }
}
