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

#include <qlayout.h>

#include <kconfig.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <libkcal/calendar.h>

#include "timespanwidget.h"
#include "koglobals.h"

#include "kotimespanview.h"
#include "kotimespanview.moc"

KOTimeSpanView::KOTimeSpanView(Calendar *calendar, QWidget *parent, 
               const char *name) :
  KOEventView( calendar, parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  
  mTimeSpanWidget = new TimeSpanWidget( this );
  topLayout->addWidget( mTimeSpanWidget );

  connect( mTimeSpanWidget, SIGNAL( dateRangeChanged() ), SLOT( updateView() ) );
}

KOTimeSpanView::~KOTimeSpanView()
{
}

void KOTimeSpanView::readSettings()
{
  kdDebug(5850) << "KOTimeSpanView::readSettings()" << endl;

  KConfig config( "korganizerrc", true, false); // Open read-only, no kdeglobals
  config.setGroup("Views");

  QValueList<int> sizes = config.readIntListEntry("Separator TimeSpanView");
  if (sizes.count() == 2) {
    mTimeSpanWidget->setSplitterSizes(sizes);
  }
}

void KOTimeSpanView::writeSettings(KConfig *config)
{
//  kdDebug(5850) << "KOTimeSpanView::writeSettings()" << endl;

  config->setGroup("Views");

  QValueList<int> list = mTimeSpanWidget->splitterSizes();
  config->writeEntry("Separator TimeSpanView",list);
}

int KOTimeSpanView::maxDatesHint()
{
  return 0;
}

int KOTimeSpanView::currentDateCount()
{
  return 0;
}

Incidence::List KOTimeSpanView::selectedIncidences()
{
  Incidence::List selected;
  
  return selected;
}

void KOTimeSpanView::updateView()
{
  insertItems( mTimeSpanWidget->startDateTime().date(),
               mTimeSpanWidget->endDateTime().date() );
}

void KOTimeSpanView::showDates(const QDate &start, const QDate &end)
{
  QDate s = start.addDays( -2 );
  QDate e = end.addDays( 2 );

  insertItems( s, e );
}

void KOTimeSpanView::insertItems(const QDate &start, const QDate &end)
{
  mTimeSpanWidget->clear();
  mTimeSpanWidget->setDateRange( start, end );

  Event::List events = calendar()->events( start, end );
  Event::List::ConstIterator it;
  for( it = events.begin(); it != events.end(); ++it ) {
    mTimeSpanWidget->addItem( *it );
  }
  
  mTimeSpanWidget->updateView();
}

void KOTimeSpanView::showIncidences( const Incidence::List & )
{
}

void KOTimeSpanView::changeIncidenceDisplay(Incidence *, int)
{
}
