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

void TimelineItem::insertIncidence(KCal::Incidence * incidence, const QDateTime & _start, const QDateTime & _end)
{
  QDateTime start = incidence->dtStart(), end = incidence->dtEnd();
  if ( _start.isValid() )
    start = _start;
  if ( _end.isValid() )
    end = _end;
  if ( incidence->doesFloat() )
    end = end.addDays( 1 );

  typedef QValueList<TimelineSubItem*> ItemList;
  ItemList list = mItemMap[incidence];
  for ( ItemList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it )
    if ( (*it)->startTime() == start && (*it)->endTime() == end )
      return;

  TimelineSubItem * item = new TimelineSubItem( incidence, this );
  QColor c1, c2, c3;
  colors( c1, c2, c3 );
  item->setColors( c1, c2, c3 );

  item->setStartTime( start );
  item->setEndTime( end );

  mItemMap[incidence].append( item );
}

void TimelineItem::removeIncidence(KCal::Incidence * incidence)
{
  typedef QValueList<TimelineSubItem*> ItemList;
  ItemList list = mItemMap[incidence];
  for ( ItemList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it )
    delete *it;
  mItemMap.remove( incidence );
}

TimelineSubItem::TimelineSubItem(KCal::Incidence * incidence, TimelineItem * parent) :
    KDGanttViewTaskItem( parent ),
    mIncidence( incidence )
{
  setTooltipText( IncidenceFormatter::toolTipString( incidence ) );
  if ( !incidence->isReadOnly() ) {
    setMoveable( true );
    setResizeable( true );
  }
}
