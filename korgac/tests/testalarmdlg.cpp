/*
  This file is part of KOrganizer.

  Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include "alarmdialog.h"

#include <kcal/event.h>
#include <kcal/todo.h>
using namespace KCal;

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <QWidget>

int main( int argc, char **argv )
{
  KAboutData aboutData( "testkabc", 0, ki18n( "TestKabc" ), "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );

  KApplication app;

  Event *e1 = new Event;
  e1->setSummary( "This is a summary." );
  KDateTime now = KDateTime::currentLocalDateTime();
  e1->setDtStart( now );
  e1->setDtEnd( now.addDays( 1 ) );
  Alarm *a = e1->newAlarm();
//  a->setProcedureAlarm( "/usr/X11R6/bin/xeyes" );
  a->setAudioAlarm( "/opt/kde/share/apps/korganizer/sounds/spinout.wav" );

  Todo *t1 = new Todo;
  t1->setSummary( "To-do A" );
  t1->setDtDue( now );
  t1->newAlarm();

  Event *e2 = new Event;
  e2->setSummary( "This is another summary. But it is a very long summary of total sillyness for no good reason" );
  e2->setDtStart( now );
  e2->setDtEnd( now.addDays( 1 ) );
  e2->newAlarm();

  Event *e3 = new Event;
  e3->setSummary( "Meet with Fred" );
  e3->setDtStart( now );
  e3->setDtEnd( now.addDays( 2 ) );
  e3->newAlarm();

  Todo *t2 = new Todo;
  t2->setSummary( "Something big is due today" );
  t2->setDtDue( now );
  t2->newAlarm();

  Todo *t3 = new Todo;
  t3->setSummary( "Be lazy" );
  t3->setDtDue( now );
  t3->newAlarm();

  AlarmDialog dlg;
  dlg.addIncidence( e1, QDateTime::currentDateTime(), QString() );
  dlg.addIncidence( t1, QDateTime::currentDateTime(),
                    QString( "THIS IS DISPLAY TEXT" ) );
  dlg.addIncidence( e2, QDateTime::currentDateTime(), QString() );
  dlg.addIncidence( e3, QDateTime::currentDateTime(), QString() );
  dlg.addIncidence( t2, QDateTime::currentDateTime(),
                    QString( "THIS IS DISPLAY TEXT" ) );
  dlg.addIncidence( t3, QDateTime::currentDateTime(), QString() );
  dlg.show();
  dlg.eventNotification();

  app.exec();
}
