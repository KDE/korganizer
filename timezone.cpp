/*
    This file is part of KOrganizer.

    Copyright (c) 2000 Cornelius Schumacher <schumacher@kde.org>

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
#include <time.h>

#include <qdatetime.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kglobal.h>

#include "koprefs.h"

int main(int argc,char **argv)
{
  KAboutData aboutData("timezone",I18N_NOOP("KOrganizer Timezone Test"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;

  kdDebug(5850) << "KOrganizer TimezoneId: " << KOPrefs::instance()->mTimeZoneId
            << endl;
  
  time_t ltime;
  ::time( &ltime );
  tm *t = localtime( &ltime );

  kdDebug(5850) << "localtime: " << t->tm_hour << ":" << t->tm_min << endl;

  kdDebug(5850) << "tzname: " << tzname[0] << " " << tzname[1] << endl;
  kdDebug(5850) << "timezone: " << timezone/3600 << endl;
  
  QTime qtime = QTime::currentTime();
  
  kdDebug(5850) << "QDateTime::currentTime(): "
            << qtime.toString( Qt::ISODate ) << endl;

  kdDebug(5850) << "KLocale::formatTime(): "
            << KGlobal::locale()->formatTime( qtime ) << endl;
}
