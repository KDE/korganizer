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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qwidget.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "alarmdialog.h"

int main(int argc,char **argv)
{
  KAboutData aboutData("testkabc",I18N_NOOP("TestKabc"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;

  Event *e = new Event;
  e->setSummary( "This is a summary." );
  e->setDtStart( QDateTime::currentDateTime() );
  e->setDtEnd( QDateTime::currentDateTime().addDays( 1 ) );

  Alarm *a = e->newAlarm();
//  a->setProcedureAlarm( "/usr/X11R6/bin/xeyes" );
  a->setAudioAlarm( "/opt/kde/share/apps/korganizer/sounds/spinout.wav" );

  AlarmDialog dlg;
  app.setMainWidget( &dlg );
  dlg.appendEvent( e );
  dlg.show();
  dlg.eventNotification();
    
  app.exec();
}
