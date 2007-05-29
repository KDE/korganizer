/*
    This file is part of KOrganizer.

    Copyright (c) 2007 Till Adam <adam@kde.org>

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


#include <libkcal/calendar.h>

#include <qlayout.h>

#include <kdgantt/KDGanttView.h>
#include <kdgantt/KDGanttViewTaskItem.h>
#include <kdgantt/KDGanttViewSubwidgets.h>

#include "koglobals.h"
#include "koprefs.h"

#include "kotimelineview.h"

using namespace KOrg;

KOTimelineView::KOTimelineView(Calendar *calendar, QWidget *parent,
                                 const char *name)
  : KOrg::BaseView(calendar, parent, name)
{
    QVBoxLayout* vbox = new QVBoxLayout(this);
    mGantt = new KDGanttView(this);
    mGantt->setCalendarMode( true );
    mGantt->setShowLegendButton( false );
    mGantt->setScale( KDGanttView::Hour );
    vbox->addWidget( mGantt );
}

KOTimelineView::~KOTimelineView()
{
}

/*virtual*/ 
KCal::ListBase<KCal::Incidence> KOTimelineView::selectedIncidences()
{
    return KCal::ListBase<KCal::Incidence>();
}

/*virtual*/
KCal::DateList KOTimelineView::selectedDates()
{
    return KCal::DateList();
}

/*virtual*/
int KOTimelineView::currentDateCount()
{
    return 0;
}

/*virtual*/
void KOTimelineView::showDates(const QDate& start, const QDate& end)
{
  mGantt->setHorizonStart( QDateTime(start) );
  mGantt->setHorizonEnd( QDateTime(end) );
  mGantt->zoomToFit();
}

/*virtual*/
void KOTimelineView::showIncidences(const KCal::ListBase<KCal::Incidence>&)
{
}

/*virtual*/
void KOTimelineView::updateView()
{
}

/*virtual*/
void KOTimelineView::changeIncidenceDisplay(KCal::Incidence*, int)
{
}

#include "kotimelineview.moc"
