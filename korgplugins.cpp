/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include <calendar/plugin.h>

#include "kocore.h"

int main(int argc,char **argv)
{
  KAboutData aboutData("korgplugins",I18N_NOOP("KOrgPlugins"),"0.1");
  KCmdLineArgs::init(argc,argv,&aboutData);

  KApplication app;
  
  KTrader::OfferList plugins = KOCore::self()->availablePlugins();
  KTrader::OfferList::ConstIterator it;
  for(it = plugins.begin(); it != plugins.end(); ++it) {
    kdDebug(5850) << "Plugin: " << (*it)->desktopEntryName() << " ("
              << (*it)->name() << ")" << endl;
    KOrg::Plugin *p = KOCore::self()->loadPlugin(*it);
    if (!p) {
      kdDebug(5850) << "Plugin loading failed." << endl;
    } else {
      kdDebug(5850) << "PLUGIN INFO: " << p->info() << endl;
    }
  }
  
  plugins = KOCore::self()->availableParts();
  for(it = plugins.begin(); it != plugins.end(); ++it) {
    kdDebug(5850) << "Part: " << (*it)->desktopEntryName() << " ("
              << (*it)->name() << ")" << endl;
    KOrg::Part *p = KOCore::self()->loadPart(*it,0);
    if (!p) {
      kdDebug(5850) << "Plugin loading failed." << endl;
    } else {
      kdDebug(5850) << "PLUGIN INFO: " << p->info() << endl;
    }
  }
  
  plugins = KOCore::self()->availableCalendarDecorations();
  for(it = plugins.begin(); it != plugins.end(); ++it) {
    kdDebug(5850) << "CalendarDecoration: " << (*it)->desktopEntryName() << " ("
              << (*it)->name() << ")" << endl;
#if 0
    // FIXME: Update this to calendar decorations
    KOrg::Part *p = KOCore::self()->loadPart(*it,0,0);
    if (!p) {
      kdDebug(5850) << "Plugin loading failed." << endl;
    } else {
      kdDebug(5850) << "PLUGIN INFO: " << p->info() << endl;
    }
#endif
  }

#if 0  
  KOrg::TextDecoration::List tdl = KOCore::self()->textDecorations();
  KOrg::TextDecoration *td = tdl.first();
  while(td) {
    kdDebug(5850) << "TEXT DECORATION INFO: " << td->info() << endl;
    td = tdl.next();
  }

  KOrg::WidgetDecoration *moon = KOCore::self()->loadWidgetDecoration("moon");
  if (moon) {
    QWidget *wid = moon->daySmall(0,QDate::currentDate());
    app.setMainWidget(wid);
    wid->show();
    app.exec();
  }
#endif
}
