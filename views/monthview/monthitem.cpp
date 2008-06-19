/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Bruno Virlet <bruno.virlet@gmail.com>

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "monthitem.h"
#include "monthscene.h"
#include "monthview.h"
#include "koprefs.h"
#include "kohelper.h"

#include <kcal/todo.h>
#include <kcal/journal.h>
#include <kcal/event.h>
#include <kcal/incidence.h>

#include <QDate>
#include <QObject>
#include <QDebug>

using namespace KOrg;

static const int ft = 1; // frame thickness

//-------------------------------------------------------------
ScrollIndicator::ScrollIndicator( ScrollIndicator::ArrowDirection dir )
  : mDirection( dir )
{
  setZValue( 200 ); // on top of everything
  hide();
}

QRectF ScrollIndicator::boundingRect() const
{
  return QRectF( - mWidth / 2, - mHeight / 2, mWidth, mHeight );
}

void ScrollIndicator::paint( QPainter *painter, const QStyleOptionGraphicsItem *option,
                             QWidget *widget )
{
  painter->setRenderHint( QPainter::Antialiasing );

  QPolygon arrow( 3 );
  if ( mDirection == ScrollIndicator::UpArrow ) {
    arrow.setPoint( 0, 0, - mHeight / 2 );
    arrow.setPoint( 1, mWidth / 2, mHeight / 2 );
    arrow.setPoint( 2, - mWidth / 2, mHeight / 2 );
  } else if ( mDirection == ScrollIndicator::DownArrow ) { // down
    arrow.setPoint( 1, mWidth / 2, - mHeight / 2 );
    arrow.setPoint( 2, - mWidth / 2,  - mHeight / 2 );
    arrow.setPoint( 0, 0, mHeight / 2 );
  }
  QColor color( Qt::black );
  color.setAlpha( 155 );
  painter->setBrush( color );
  painter->setPen( color );
  painter->drawPolygon( arrow );
}

//-------------------------------------------------------------
MonthCell::MonthCell( int id, QDate date, QGraphicsScene *scene )
  : mId( id ), mDate( date ), mScene( scene )
{
  mUpArrow = new ScrollIndicator( ScrollIndicator::UpArrow );
  mDownArrow = new ScrollIndicator( ScrollIndicator::DownArrow );
  mScene->addItem( mUpArrow );
  mScene->addItem( mDownArrow );
}

MonthCell::~MonthCell()
{
  mScene->removeItem( mUpArrow );
  mScene->removeItem( mDownArrow );
  delete mUpArrow; // we've taken ownership, so this is safe
  delete mDownArrow;
}

bool MonthCell::hasEventBelow( int height )
{
  if ( mHeightHash.isEmpty() ) {
    return false;
  }

  for ( int i=0; i<height; i++ ) {
    if ( mHeightHash.value( i ) != 0 ) {
      return true;
    }
  }

  return false;
}

int MonthCell::topMargin()
{
  return 18;
}

void MonthCell::addMonthItem( MonthItem *manager, int height )
{
  mHeightHash[ height ] = manager;
}

int MonthCell::firstFreeSpace()
{
  MonthItem *manager = 0;
  int i = 0;
  while ( true ) {
    manager = mHeightHash[ i ];
    if ( manager == 0 ) {
      return i;
    }
    i++;
  }
}

MonthItem::MonthItem( MonthScene *monthScene, Incidence *incidence )
  : mMonthScene( monthScene ),
    mIncidence( incidence ),
    mSelected( false ),
    mMoving( false ),
    mResizing( false )
{
  connect( mMonthScene, SIGNAL(incidenceSelected(Incidence* )),
           this, SLOT(updateSelection(Incidence* )) );

  installEventFilter( monthScene );

}

QRectF MonthGraphicsItem::boundingRect() const
{
  return QRectF( 0,  0, ( daySpan() + 1 ) * mMonthScene->columnWidth(),
                 mMonthScene->itemHeight() );
}

void MonthGraphicsItem::paint( QPainter *p, const QStyleOptionGraphicsItem *, QWidget * )
{
  if ( !mMonthScene->initialized() ) {
    return;
  }

  p->setRenderHint( QPainter::Antialiasing );

  int textMargin = 10;

  QColor bgColor = QColor();
  if ( mMonthItem->incidence()->type() == "Todo" &&
       !KOPrefs::instance()->todosUseCategoryColors() ) {
    if ( static_cast<Todo*>( mMonthItem->incidence() )->isOverdue() ) {
      bgColor = KOPrefs::instance()->agendaCalendarItemsToDosOverdueBackgroundColor();
    } else if ( static_cast<Todo*>( mMonthItem->incidence() )->dtDue().date() ==
                QDateTime::currentDateTime().date() ) {
      bgColor = KOPrefs::instance()->agendaCalendarItemsToDosDueTodayBackgroundColor();
    }
  }

  if ( !bgColor.isValid() ) {
    QStringList categories = mMonthItem->incidence()->categories();
    QString cat;
    if ( !categories.isEmpty() ) {
      cat = categories.first();
    }
    if ( cat.isEmpty() ) {
      bgColor = KOPrefs::instance()->monthCalendarItemsEventsBackgroundColor();
    } else {
      bgColor = KOPrefs::instance()->categoryColor(cat);
    }
  }
  QColor resourceColor = KOHelper::resourceColor( mMonthScene->calendar(),
                                                  mMonthItem->incidence() );
  QColor frameColor = Qt::black;
  if (/* KOPrefs::instance()->agendaViewUsesResourceColor()
         && */ resourceColor.isValid() ) {
    frameColor = mMonthItem->selected() ? QColor( 85 + resourceColor.red() * 2 / 3,
                                     85 + resourceColor.green() * 2 / 3,
                                     85 + resourceColor.blue() * 2 / 3 )
                 : resourceColor;
  } else {
    frameColor = mMonthItem->selected() ? QColor( 85 + bgColor.red() * 2 / 3,
                                     85 + bgColor.green() * 2 / 3,
                                     85 + bgColor.blue() * 2 / 3 )
                 : bgColor.dark(115);
  }
  QColor textColor = getTextColor(bgColor);

  QPen pen( frameColor );
  pen.setWidth( ft );
  p->setPen( pen );

  // Add a gradient at extremities to show whether the item continues on a new line or not.
  QColor gradientCenterColor = mMonthItem->selected() ? bgColor.lighter( 130 ) : bgColor;
  QLinearGradient bgGradient( QPointF( 0, 0 ), QPointF( boundingRect().width(), 0 ) ) ;
  if ( !isBeginItem() ) {
    bgGradient.setColorAt( 0, frameColor );
    bgGradient.setColorAt( 0.05, gradientCenterColor );
  } else {
    bgGradient.setColorAt( 0, gradientCenterColor );
  }
  if ( !isEndItem() ) {
    bgGradient.setColorAt( 0.95, gradientCenterColor );
    bgGradient.setColorAt( 1, frameColor );
  } else {
    bgGradient.setColorAt( 1, gradientCenterColor );
  }
  p->setBrush( bgGradient );

  // Rounded rect
  p->drawPath( widgetPath() );

  p->setPen( textColor );
//  p->drawText( textMargin, 10, mMonthItem->incidence()->summary() );

  int alignFlag = Qt::AlignVCenter;
  if ( isBeginItem() ) {
    alignFlag |= Qt::AlignLeft;
  } else if ( isEndItem() ) {
    alignFlag |= Qt::AlignRight;
  } else {
    alignFlag |= Qt::AlignCenter;
  }

  QString textHour;
  if ( !incidence()->allDay() ) { // Prepend the time str to the text
    QTime time;
    if ( mMonthItem->incidence()->type() == "Todo" ) {
      Todo *todo = static_cast<Todo*>( mMonthItem->incidence() );
      time = todo->dtDue().time();
    } else {
      if ( isBeginItem() ) {
        time = mMonthItem->incidence()->dtStart().time();
      } else {
        time = mMonthItem->incidence()->dtEnd().time();
      }
    }
    textHour = KGlobal::locale()->formatTime( time );
  }
  QString text;
  if ( isBeginItem() ) {
    text = textHour + ' ' + mMonthItem->incidence()->summary();
  } else {
    text = mMonthItem->incidence()->summary() + ' ' + textHour;
  }

  QFont font  = p->font();
  font.setPixelSize( boundingRect().height() - 7 );
  p->setFont( font );
  QRect textRect = QRect( textMargin, 1,
                          boundingRect().width() - 2 * textMargin, mMonthScene->itemHeight() - 2 );
  text = p->fontMetrics().elidedText( text, Qt::ElideRight, textRect.width() );
  // Uncomment to draw text bounding rect
  //  p->drawRect( textRect );
  p->drawText( textRect, alignFlag, text );

}

// Return the height of the item in the cell.
int MonthItem::height() const
{
  return mHeight;
}

void MonthItem::updateSelection( Incidence *incidence )
{
  if ( mSelected || incidence == mIncidence ) {
    if ( mSelected ) {
      mSelected = false;
    }
    if ( incidence == mIncidence && !mSelected ) {
      mSelected = true;
    }
  }
}

bool MonthItem::isResizable() const
{
  return incidence()->type() != "Todo"
    && incidence()->type() != "Journal";
}

void MonthItem::deleteAll()
{
  foreach ( MonthGraphicsItem *item, mMonthGraphicsItemList ) {
    delete item;
  }

  mMonthGraphicsItemList.clear();
}

void MonthItem::updateMonthGraphicsItems()
{
  MonthGraphicsItem *movingItem = mMonthScene->movingMonthGraphicsItem();
  // Remove all items but the moving one
  foreach ( MonthGraphicsItem *item, mMonthGraphicsItemList ) {

    if ( item != movingItem ) {
      delete item;
    }

  }

  mMonthGraphicsItemList.clear();
  bool movingItemAdded = false; // if we don't add movingItem to the list, don't forget to delete it

  // For each row of the month view, create an item or use the movingItem to build the whole
  // MonthItem's MonthGraphicsItems.
  for ( QDate d = mMonthScene->mMonthView->startDate();
        d < mMonthScene->mMonthView->endDate(); d = d.addDays( 7 ) ) {
    QDate end = d.addDays( 6 );

    int span;
    QDate start;
    if ( startDate() <= d && endDate() >= end ) { // MonthItem takes the whole line
      span = 6;
      start = d;
    } else if ( startDate() >= d && endDate() <= end ) { // starts and ends on this line
      start = startDate();
      span = daySpan();
    } else if ( d <= endDate() && endDate() <= end ) { // MonthItem ends on this line
      span = mMonthScene->getLeftSpan( endDate() );
      start = d;
    } else if ( d <= startDate() && startDate() <= end ) { // MonthItem begins on this line
      span = mMonthScene->getRightSpan( startDate() );
      start = startDate();
    } else { // MonthItem is not on the line
      continue;
    }

    if ( movingItem && ( ( movingItem->startDate() >= d && movingItem->startDate() <= end ) ||
                         ( movingItem->endDate() >= d && movingItem->endDate() <= end ) ) ) {
      movingItem->setStartDate( start );
      movingItem->setDaySpan( span );
      mMonthGraphicsItemList << movingItem;
      movingItemAdded = true;
    } else {
      // A new item needs to be created
      MonthGraphicsItem *newItem = new MonthGraphicsItem( mMonthScene, this );
      mMonthGraphicsItemList << newItem;
      newItem->setStartDate( start );
      newItem->setDaySpan( span );
    }
  }

  if ( movingItem && movingItem->monthItem() == this ) {
    setZValue( 100 );
  } else {
    setZValue( 0 );
  }

  if ( movingItem && !movingItemAdded ) {
    qDebug() << "DELETING MOVING ITEM - SHOULD NOT HAPPEN";
    delete mMonthScene->movingMonthGraphicsItem();
  }
}

void MonthItem::resize( bool resize )
{
  if ( resize ) {
    mResizingDaySpan = daySpan();
    mResizingStartDate = startDate();
    mResizing = true;
    setZValue( 100 );
  } else {
    setZValue( 0 );
    mResizing = false; // startDate() and daySpan() are real ones again !
    int oldDaySpan = daySpan();
    if ( mResizingStartDate != startDate() || mResizingDaySpan != daySpan() ) {

      // Modify the incidence end date
      if ( mIncidence->type() == "Event" ) {
        Event *event = static_cast<Event *>( mIncidence );

        if ( mMonthScene->resizeType() == MonthScene::ResizeLeft ) {
          event->setDtStart( event->dtStart().addDays(
                               realStartDate().daysTo( mResizingStartDate ) ) );
        }

        if ( mMonthScene->resizeType() == MonthScene::ResizeRight ) {
          event->setDtEnd( event->dtEnd().addDays( mResizingDaySpan - oldDaySpan ) );
        }
      } // a todo can't be resized
    }
  }
}

void MonthItem::setZValue( qreal z )
{
  foreach ( MonthGraphicsItem *item, mMonthGraphicsItemList ) {
    item->setZValue( z );
  }
}

void MonthItem::move( bool move )
{
  if ( move ) {
    mMovingStartDate = startDate();
    mMoving = true;
    setZValue( 100 );
  } else {
    setZValue( 0 );
    mMonthScene->setMovingMonthGraphicsItem( 0 );

    if ( !mMovingStartDate.isValid() ) {
      return;
    }

    mMoving = false;
    if ( mMovingStartDate != realStartDate() ) {
      int offset = realStartDate().daysTo( mMovingStartDate );
      // Modify the incidence dates
      if ( mIncidence->type() == "Todo" ) {
        Todo *todo = static_cast<Todo*>( mIncidence );
        todo->setDtDue( todo->dtDue().addDays( offset ) );
      } else if ( mIncidence->type() == "Journal" ) {
        Journal *journal = static_cast<Journal*>( mIncidence );
        journal->setDtStart( journal->dtStart().addDays( offset ) );
      } else if ( mIncidence->type() == "Event" ) {
        Event *event = static_cast<Event*>( mIncidence );
        event->setDtStart( event->dtStart().addDays( offset ) );
        event->setDtEnd( event->dtEnd().addDays( offset ) );
        KDateTime dtTest = event->dtStart();
        startDate();
      }
      mMovingStartDate = QDate();
    }
    updateMonthGraphicsItems();
  }
}

// If @p begin is true, the resizing is done at the beginning of the item, else
// it is done at the other extremity.
void MonthItem::resizing( int offsetToPreviousDate )
{
  // save dayspan
  int span = daySpan();
  QDate start = startDate();

  movingOrResizing( offsetToPreviousDate );

  if ( mMonthScene->resizeType() == MonthScene::ResizeLeft ) {
    mResizingStartDate = start.addDays( offsetToPreviousDate );
    setResizingDaySpan( span - offsetToPreviousDate );
  } else if ( mMonthScene->resizeType() == MonthScene::ResizeRight ) {
    setResizingDaySpan( span + offsetToPreviousDate );
  }

  updateMonthGraphicsItems();
}

void MonthItem::movingOrResizing( int offsetToPreviousDate )
{
  MonthGraphicsItem *movingItem = mMonthScene->movingMonthGraphicsItem();

  /*
   * Set the moving item startDate so that it will be on the same line as it has been moved.
   */
  if ( movingItem ) {
    movingItem->setStartDate( movingItem->startDate().addDays( offsetToPreviousDate ) );

    // Check if the item begin and end are on the same week
    MonthCell *startDateCell = mMonthScene->mMonthCellMap.value( movingItem->startDate() );
    MonthCell *endDateCell = mMonthScene->mMonthCellMap.value( movingItem->endDate() );

    if ( offsetToPreviousDate < 0 ) {
      if ( !startDateCell || startDateCell->y() != endDateCell->y() ) {
        // Item has been moved to the left
        movingItem->setStartDate( mMonthScene->firstDateOnRow( endDateCell->y() ) );
        movingItem->setDaySpan( 0 );
      }
    } else if ( offsetToPreviousDate > 0 ) {
      if ( !endDateCell || endDateCell->y() != startDateCell->y() ) {
        // Item has been moved to the right, start date is ok
        movingItem->setDaySpan( 0 );
      }
    }
  }

  mMovingStartDate = startDate().addDays( offsetToPreviousDate );
}

// While in a move, calling this function will change the detachedStartDate
void MonthItem::moving( int offsetToPreviousDate )
{
  movingOrResizing( offsetToPreviousDate );
  updateMonthGraphicsItems();
}

void MonthItem::updateGeometry()
{
  foreach ( MonthGraphicsItem *item, mMonthGraphicsItemList ) {
      item->updateGeometry();
  }
}

MonthItem::~MonthItem()
{
  deleteAll();
}

QDate MonthItem::startDate() const
{
  if ( isMoving() ) {
    return mMovingStartDate;
  } else if ( isResizing() ) {
    return mResizingStartDate;
  } else {
    return realStartDate();
  }
}

QDate MonthItem::realStartDate() const
{
  KDateTime dt;
  if ( mIncidence->type() == "Todo" ) {
    Todo *todo = static_cast<Todo*>( mIncidence );
    dt = todo->dtDue();
  } else {
    dt =  mIncidence->dtStart();
  }

  if ( dt.isDateOnly() ) {
    return dt.date();
  } else {
    return dt.toTimeSpec( KOPrefs::instance()->timeSpec() ).date();
  }
}

QDate MonthItem::endDate() const
{
  return startDate().addDays( daySpan() );
}

bool MonthItem::greaterThan( const MonthItem *e1, const MonthItem *e2 )
{
  if ( e1->startDate() == e2->startDate() ) {
    if ( e1->daySpan() == e2->daySpan() ) {
      if ( e1->incidence()->allDay() ) {
        return true;
      }
      if ( e2->incidence()->allDay() ) {
        return false;
      }
      return e1->incidence()->dtStart().time() < e2->incidence()->dtStart().time();
    } else {
      return e1->daySpan() >  e2->daySpan();
    }
  }

  return e1->startDate() < e2->startDate();
}

int MonthItem::daySpan() const
{
  if ( mResizing ) {
    return mResizingDaySpan;
  }

  if ( !mIncidence->dtEnd().isValid() ) {
    return 0; // For Todo items
  }

  KDateTime start = mIncidence->dtStart();
  KDateTime end = mIncidence->dtEnd();

  if ( !mIncidence->allDay() ) {
    end = end.addSecs( -1 );
  }

  if ( end.isValid() ) {
    return start.daysTo( end );
  }

  return 0;
}

// Find the smaller possible height for this item
void MonthItem::updateHeight()
{
  int firstFreeSpace = 0;
  for ( QDate d = startDate(); d <= endDate(); d = d.addDays( 1 ) ) {
    MonthCell *cell = mMonthScene->mMonthCellMap.value( d );
    if ( !cell ) {
      continue; // cell can be null if the item begins outside the month
    }
    int firstFreeSpaceTmp = cell->firstFreeSpace();
    if ( firstFreeSpaceTmp > firstFreeSpace ) {
      firstFreeSpace = firstFreeSpaceTmp;
    }
  }

  for ( QDate d = startDate(); d <= endDate(); d = d.addDays( 1 ) ) {
    MonthCell *cell = mMonthScene->mMonthCellMap.value( d );
    if ( !cell ) {
      continue;
    }
    cell->addMonthItem( this, firstFreeSpace );
  }

  mHeight = firstFreeSpace;
}
bool MonthGraphicsItem::isMoving() const
{
  return mMonthItem->isMoving();
}

Incidence *MonthGraphicsItem::incidence() const
{
  return mMonthItem->incidence();
}

bool MonthGraphicsItem::isEndItem() const
{
  return startDate().addDays( daySpan() ) == mMonthItem->endDate();
}

bool MonthGraphicsItem::isBeginItem() const
{
  return startDate() == mMonthItem->startDate();
}

QPainterPath MonthGraphicsItem::shape() const
{
  return widgetPath( false );
}

// TODO: remove this method.
QPainterPath MonthGraphicsItem::widgetPath( bool mask ) const
{
  // If this is the mask, we draw it one pixel bigger
  int m = mask ? 1 : 0;
  int x0 = ft / 2 - m;
  int y0 = ft / 2 - m;
  int height = boundingRect().height() - ft / 2 + 2 * m;
  int width = boundingRect().width() - 1 - ft / 2 + 2 * m;
  int x1 = boundingRect().width() - 1 - ft / 2 + m;
  int y1 = boundingRect().height() - ft / 2 + m;
  int beginRound = boundingRect().height() / 3 + m;

  QPainterPath path( QPoint( beginRound, 0 ) );
  if ( isBeginItem() ) {
    path.arcTo( QRect( x0, y0, beginRound * 2 + m, height ), +90, +180 );
  } else {
    path.lineTo( x0, y0 );
    path.lineTo( x0, y1 );
    path.lineTo( x0 + beginRound, y1 );
  }

  if ( isEndItem() ) {
    path.lineTo( x1 - beginRound, y1 );
    path.arcTo( QRect( x1 - 2 * beginRound - m, y0, beginRound * 2 + m, height ), -90, +180 );
    path.lineTo( x0 + beginRound, y0 );
  } else {
    path.lineTo( x1, y1 );
    path.lineTo( x1, y0 );
    path.lineTo( x0 + beginRound, y0 );
  }
  path.closeSubpath();

  return path;
}

void MonthGraphicsItem::setStartDate( const QDate &date )
{
  mStartDate = date;
}

QDate MonthGraphicsItem::endDate() const
{
  return startDate().addDays( daySpan() );
}

QDate MonthGraphicsItem::startDate() const
{
  return mStartDate;
}

void MonthGraphicsItem::setDaySpan( int span )
{
  mDaySpan = span;
}

int MonthGraphicsItem::daySpan() const
{
  return mDaySpan;
}

void MonthGraphicsItem::updateGeometry()
{
  MonthCell *cell =  mMonthScene->mMonthCellMap.value( startDate() );

  // If the item is moving and this one is moved outside the view, cell
  // will be null
  if ( mMonthItem->isMoving() && !cell ) {
    hide();
    return;
  }

  Q_ASSERT( cell );

  prepareGeometryChange();

  int beginX = 1 + mMonthScene->cellHorizontalPos( cell );
  int beginY = 1 + cell->topMargin() + mMonthScene->cellVerticalPos( cell );

  beginY += mMonthItem->height() * mMonthScene->itemHeightIncludingSpacing() -
            mMonthScene->startHeight() * mMonthScene->itemHeightIncludingSpacing(); // scrolling

  setPos( beginX, beginY );

  if ( mMonthItem->height() < mMonthScene->startHeight() ||
       mMonthItem->height() - mMonthScene->startHeight() >= mMonthScene->maxRowCount() ) {
    hide();
  } else {
    show();
    update();
  }
}

MonthGraphicsItem::MonthGraphicsItem( MonthScene *monthScene, MonthItem *manager )
  : QGraphicsItem( 0, monthScene ),
    mMonthItem( manager ),
    mMonthScene( monthScene )
{
  QTransform transform;
  transform = transform.translate( 0.5, 0.5 );
  setTransform( transform );
}

MonthGraphicsItem::~MonthGraphicsItem()
{
}
