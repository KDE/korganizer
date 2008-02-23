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

#define protected public
#include <kdgantt/KDGanttViewSubwidgets.h>
#undef public

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
  item->setOriginalStart( start );
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

void TimelineItem::moveItems(KCal::Incidence * incidence, int delta, int duration)
{
  typedef QValueList<TimelineSubItem*> ItemList;
  ItemList list = mItemMap[incidence];
  for ( ItemList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it ) {
    QDateTime start = (*it)->originalStart();
    start = start.addSecs( delta );
    (*it)->setStartTime( start );
    (*it)->setOriginalStart( start );
    (*it)->setEndTime( start.addSecs( duration ) );
  }
}


TimelineSubItem::TimelineSubItem(KCal::Incidence * incidence, TimelineItem * parent) :
    KDGanttViewTaskItem( parent ),
    mIncidence( incidence ),
    mLeft( 0 ),
    mRight( 0 ),
    mMarkerWidth( 0 )
{
  setTooltipText( IncidenceFormatter::toolTipString( incidence ) );
  if ( !incidence->isReadOnly() ) {
    setMoveable( true );
    setResizeable( true );
  }
}

TimelineSubItem::~TimelineSubItem()
{
  delete mLeft;
  delete mRight;
}

void TimelineSubItem::showItem(bool show, int coordY)
{
  KDGanttViewTaskItem::showItem( show, coordY );
  int y;
  if ( coordY != 0 )
    y = coordY;
  else
    y = getCoordY();
  int startX = myGanttView->timeHeaderWidget()->getCoordX(myStartTime);
  int endX = myGanttView->timeHeaderWidget()->getCoordX(myEndTime);

  const int mw = QMAX( 1, QMIN( 4, endX - startX ) );
  if ( !mLeft || mw != mMarkerWidth ) {
    if ( !mLeft ) {
      mLeft = new KDCanvasPolygon( myGanttView->timeTableWidget(), this, Type_is_KDGanttViewItem );
      mLeft->setBrush( Qt::black );
    }
    QPointArray a = QPointArray( 4 );
    a.setPoint( 0, 0, -mw -myItemSize/2 - 2 );
    a.setPoint( 1, mw, -myItemSize/2 - 2 );
    a.setPoint( 2, mw, myItemSize/2 + 2 );
    a.setPoint( 3, 0, myItemSize/2 + mw + 2 );
    mLeft->setPoints( a );
  }
  if ( !mRight || mw != mMarkerWidth ) {
    if ( !mRight ) {
      mRight = new KDCanvasPolygon( myGanttView->timeTableWidget(), this, Type_is_KDGanttViewItem );
      mRight->setBrush( Qt::black );
    }
    QPointArray a = QPointArray( 4 );
    a.setPoint( 0, -mw, -myItemSize/2 - 2 );
    a.setPoint( 1, 0, -myItemSize/2 - mw - 2 );
    a.setPoint( 2, 0, myItemSize/2 + mw + 2 );
    a.setPoint( 3, -mw, myItemSize/2 + 2 );
    mRight->setPoints( a );
  }
  mMarkerWidth = mw;
  mLeft->setX( startX );
  mLeft->setY( y );
  mLeft->setZ( startShape->z() - 1 );
  mLeft->show();
  mRight->setX( endX );
  mRight->setY( y );
  mRight->setZ( startShape->z() - 1 );
  mRight->show();
}
