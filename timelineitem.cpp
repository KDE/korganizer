/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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
*/

#include "timelineitem.h"

#include "kohelper.h"

#include <libkcal/calendar.h>
#include <libkcal/incidenceformatter.h>
#include <libkcal/resourcecalendar.h>

using namespace KOrg;
using namespace KCal;

TimelineItem::TimelineItem( const QString &label, KDGanttView * parent) :
    KDGanttViewTaskItem( parent )
{
  setListViewText( 0, label );
  setDisplaySubitemsAsGroup( true );
  if ( listView() )
    listView()->setRootIsDecorated( false );
}

void TimelineItem::insertIncidence(KCal::Incidence * incidence, const QDateTime & start, const QDateTime & end)
{
  TimelineSubItem * item = new TimelineSubItem( incidence, this );
  QColor c1, c2, c3;
  colors( c1, c2, c3 );
  item->setColors( c1, c2, c3 );
  if ( start.isValid() )
    item->setStartTime( start );
  if ( end.isValid() )
    item->setEndTime( end );
  mItemMap[incidence] = item;
}

void TimelineItem::removeIncidence(KCal::Incidence * incidence)
{
  delete mItemMap[incidence];
  mItemMap.remove( incidence );
}

TimelineSubItem::TimelineSubItem(KCal::Incidence * incidence, TimelineItem * parent) :
    KDGanttViewTaskItem( parent ),
    mIncidence( incidence )
{
  setStartTime( incidence->dtStart() );
  QDateTime end = incidence->dtEnd();
  if ( incidence->doesFloat() ) {
    end = end.addDays( 1 );
  }
  setEndTime( end );

  setTooltipText( IncidenceFormatter::toolTipString( incidence ) );
}
