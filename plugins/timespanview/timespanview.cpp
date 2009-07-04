/*
  This file is part of KOrganizer.

  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
*/

#include "timespanview.h"
#include "kotimespanview.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>
#include <kglobal.h>

#include <QFile>

using namespace KOrg;

K_PLUGIN_FACTORY( TimespanViewFactory, registerPlugin<TimespanView>(); )
K_EXPORT_PLUGIN( TimespanViewFactory( "korg_timespanview" ) )

TimespanView::TimespanView( MainWindow *parent )
  : Part( parent ), mView( 0 )
{
  setComponentData( KComponentData( "korganizer" ) );

  setXMLFile( "plugins/timespanviewui.rc" );

  new KAction( i18n( "&Timespan" ), "timespan", 0, this, SLOT(showView()),
               actionCollection(), "view_timespan" );
}

TimespanView::~TimespanView()
{
}

QString TimespanView::info()
{
  return i18n( "This plugin provides a Gantt-like Timespan view." );
}

QString TimespanView::shortInfo()
{
  return i18n( "Timespan View Plugin" );
}

void TimespanView::showView()
{
  if ( !mView ) {
    mView = new KOTimeSpanView( mainWindow()->view()->calendar(),
                                mainWindow()->view() );
    mainWindow()->view()->addView( mView );
  }
  mainWindow()->view()->showView( mView );
}

#include "timespanview.moc"
