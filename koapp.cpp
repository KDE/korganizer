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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <stdio.h>
#include <stdlib.h>

#include <qfile.h>

#include <kstandarddirs.h>
#include <kglobal.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kwin.h>
#include <kurl.h>
#include <kprocess.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/filestorage.h>
#include <libkcal/calformat.h>

#include "kalarmd/alarmdaemoniface_stub.h"

#include "korganizer.h"
#include "koprefs.h"
#include "version.h"

#include "koapp.h"
#include "koapp.moc"

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
    printf(i18n("Unable to handle remote calendar.\n").local8Bit());
    return;
  }

  Calendar *cal = new CalendarLocal(KOPrefs::instance()->mTimeZoneId.local8Bit());

  QDate currDate(QDate::currentDate());
  Event *currEvent;

  FileStorage storage( cal, url.path() );

  if ( !storage.load() ) {
    printf(i18n("Could not load calendar '%1'.\n").arg(url.path()).local8Bit());
    exit(0);
  }

  for (int i = 1; i <= numdays; i++) {
    printf("%s\n",(const char *)KGlobal::locale()->formatDate(currDate).local8Bit());

    QPtrList<Event> tmpList( cal->events( currDate, true ) );
    printf("---------------------------------------------------------------\n");
    if (tmpList.count() > 0) {
      for (currEvent = tmpList.first(); currEvent; currEvent = tmpList.next()) {
        printf("%s",(const char *)currEvent->summary().local8Bit());
        if (!currEvent->doesFloat()) {
          printf(" (%s - %s)",(const char *)currEvent->dtStartStr().local8Bit(),
                 (const char *)currEvent->dtEndStr().local8Bit());
        }
        printf("\n");
      }
    } else {
      printf(i18n("(no events)\n").local8Bit());
    }

    printf("---------------------------------------------------------------\n");
    QPtrList<Todo> tmpList2 = cal->todos(currDate);
    Todo *currTodo;
    if (tmpList.count() > 0) {
      for (currTodo = tmpList2.first(); currTodo; currTodo = tmpList2.next()) {
        printf("%s",(const char *)currTodo->summary().local8Bit());
        if (!currTodo->doesFloat()) {
          printf(" (%s)",(const char *)currTodo->dtDueStr().local8Bit());
        }
        printf("\n");
      }
    } else {
      printf(i18n("(no todos)\n").local8Bit());
    }

    printf("\n");
    currDate = currDate.addDays(1);
  }
}


void KOrganizerApp::startAlarmDaemon()
{
  kdDebug() << "Starting alarm daemon" << endl;

  // Start alarmdaemon. It is a KUniqueApplication, that means it is
  // automatically made sure that there is only one instance of the alarm daemon
  // running.
  QString execStr = locate("exe","kalarmd");
  kapp->kdeinitExecWait(execStr);

  kdDebug() << "Starting alarm daemon done" << endl;
}

void KOrganizerApp::startAlarmClient()
{
  kdDebug() << "Starting alarm client" << endl;

  KProcess *proc = new KProcess;
  *proc << "korgac";
  *proc << "--miniicon" <<  "korganizer";
  connect( proc, SIGNAL( processExited( KProcess * ) ),
           SLOT( startCompleted( KProcess * ) ) );
  if (!proc->start())
      delete proc;
}

void KOrganizerApp::startCompleted( KProcess *process )
{
  delete process;
}

int KOrganizerApp::newInstance()
{
  kdDebug() << "KOApp::newInstance()" << endl;

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  
  // process command line options
  int numDays = 0;
  if (args->isSet("list")) {
    numDays = 1;
  } else if (args->isSet("show")) {
    numDays = args->getOption("show").toInt();
  } else {
    if (!dcopClient()->isApplicationRegistered("kalarmd")) {
      startAlarmDaemon();
    }
    if (!dcopClient()->isApplicationRegistered("korgac")) {
      startAlarmClient();
    }

    kdDebug() << "KOApp::newInstance() registerApp" << endl;
    // Register this application with the alarm daemon
    AlarmDaemonIface_stub stub( "kalarmd", "ad" );
    stub.registerApp( "korgac", "KOrganizer", "ac", 3, true );
    if( !stub.ok() ) {
      kdDebug() << "KOrganizerApp::newInstance(): dcop send failed" << endl;
    }
  }

  // If filenames was given as argument load this as calendars, one per window.
  if (args->count() > 0) {
    int i;
    for(i=0;i<args->count();++i) {
      processCalendar( args->url(i), numDays ); 
    }
  } else {
    KGlobal::config()->setGroup("General");
    QString urlString = KGlobal::config()->readEntry("Active Calendar");

    // Force alarm daemon to load active calendar
    AlarmDaemonIface_stub stub( "kalarmd", "ad" );
    stub.addCal( "korgac", urlString );

    processCalendar( urlString, numDays ); 
  }
  
  kdDebug() << "KOApp::newInstance() done" << endl;
  return 0;
}


void KOrganizerApp::processCalendar( const KURL &url, int numDays )
{
  if (numDays > 0) {
    displayImminent( url, numDays );
  } else {
    if (isRestored()) {
      RESTORE(KOrganizer)
    } else {
      KOrganizer *korg=KOrganizer::findInstance(url);
      if (0 == korg) {
        korg = new KOrganizer( "KOrganizer MainWindow" ); 
        korg->show();
        
        kdDebug() << "KOrganizerApp::processCalendar(): " << url.url() << endl;
        
        if (!url.isEmpty()) {
          korg->openURL(url);
        }
      } else
          KWin::setActiveWindow(korg->winId());
    }
  }
}
