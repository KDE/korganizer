/*
    This file is part of KOrganizer.
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
*/

#include <qfile.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>
#include <kglobal.h>

//#include <korganizer/calendarviewbase.h">

#include "kotimespanview.h"

#include "timespanview.h"
using namespace KOrg;
#include "timespanview.moc"

class TimespanViewFactory : public KOrg::PartFactory {
  public:
    KOrg::Part *create( KOrg::MainWindow *parent, const char *name )
    {
      return new TimespanView( parent, name );
    }
};

extern "C" {
  void *init_libkorg_timespanview()
  {
    return ( new TimespanViewFactory );
  }
}


TimespanView::TimespanView(KOrg::MainWindow *parent, const char *name) :
  KOrg::Part(parent,name), mView(0)
{
  setInstance( new KInstance( "korganizer" ) );

  setXMLFile( "plugins/timespanviewui.rc" );

  new KAction( i18n("&Timespan"), "timespan", 0, this, SLOT( showView() ),
              actionCollection(), "view_timespan" );
}

TimespanView::~TimespanView()
{
}

QString TimespanView::info()
{
  return i18n("This plugin provides a gantt-like Timespan view.");
}

QString TimespanView::shortInfo()
{
  return i18n( "Timespan View Plugin" );
}

void TimespanView::showView()
{
  if (!mView) {
    mView = new KOTimeSpanView( mainWindow()->view()->calendar(),
                                mainWindow()->view() );
    mainWindow()->view()->addView( mView );
  }
  mainWindow()->view()->showView( mView );
}
