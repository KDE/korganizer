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

#include "monthscene.h"
#include "monthitem.h"
#include "monthgraphicsitems.h"
#include "monthview.h"
#include "koprefs.h"
#include "koglobals.h"

#include <kcal/incidence.h>
#include <kcalendarsystem.h>

#include <QPaintEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneMouseEvent>

using namespace KOrg;

MonthScene::MonthScene( MonthView *parent, Calendar *calendar )
  : QGraphicsScene( parent ),
    mMonthView( parent ),
    mInitialized( false ),
    mCalendar( calendar ),
    mClickedItem( 0 ),
    mActionItem( 0 ),
    mActionInitiated( false ),
    mSelectedItem( 0 ),
    mStartCell( 0 ),
    mActionType( None ),
    mStartHeight( 0 )
{
  mEventPixmap     = KOGlobals::self()->smallIcon( "view-calendar-day" );
  mTodoPixmap      = KOGlobals::self()->smallIcon( "view-calendar-tasks" );
  mTodoDonePixmap  = KOGlobals::self()->smallIcon( "task-complete" );
  mJournalPixmap   = KOGlobals::self()->smallIcon( "view-pim-journal" );
  mAlarmPixmap     = KOGlobals::self()->smallIcon( "appointment-reminder" );
  mRecurPixmap     = KOGlobals::self()->smallIcon( "appointment-recurring" );
  mReadonlyPixmap  = KOGlobals::self()->smallIcon( "object-locked" );
  mReplyPixmap     = KOGlobals::self()->smallIcon( "mail-reply-sender" );
  mHolidayPixmap   = KOGlobals::self()->smallIcon( "favorites" );
}

MonthScene::~MonthScene()
{
  qDeleteAll( mMonthCellMap );
  qDeleteAll( mManagerList );
}

MonthCell *MonthScene::selectedCell() const
{
  return mMonthCellMap.value( mSelectedCellDate );
}

int MonthScene::getRightSpan( const QDate &date ) const
{
  MonthCell *cell = mMonthCellMap.value( date );
  if ( !cell ) {
    return 0;
  }

  return 7 - cell->x() - 1;
}

int MonthScene::getLeftSpan( const QDate &date ) const
{
  MonthCell *cell = mMonthCellMap.value( date );
  if ( !cell ) {
    return 0;
  }

  return cell->x();
}

int MonthScene::maxRowCount()
{
  return ( rowHeight() - MonthCell::topMargin() ) / itemHeightIncludingSpacing();
}

int MonthScene::itemHeightIncludingSpacing()
{
  return MonthCell::topMargin() + 2;
}

int MonthScene::itemHeight()
{
  return MonthCell::topMargin();
}

MonthCell *MonthScene::firstCellForMonthItem( MonthItem *manager )
{
  for ( QDate d = manager->startDate(); d <= manager->endDate(); d = d.addDays( 1 ) ) {
    MonthCell *monthCell = mMonthCellMap.value( d );
    if ( monthCell ) {
      return monthCell;
    }
  }

  return 0;
}

void MonthScene::updateGeometry()
{
  foreach ( MonthItem *manager, mManagerList ) {
    manager->updateGeometry();
  }
}

int MonthScene::availableWidth() const
{
  return static_cast<int> ( sceneRect().width() );
}

int MonthScene::availableHeight() const
{
  return static_cast<int> ( sceneRect().height() - headerHeight() );
}

int MonthScene::columnWidth() const
{
  return static_cast<int> ( ( availableWidth() - 1 ) / 7. );
}

int MonthScene::rowHeight() const
{
  return static_cast<int> ( ( availableHeight() - 1 ) / 6. );
}

int MonthScene::headerHeight() const
{
  return 50;
}

int MonthScene::cellVerticalPos( const MonthCell *cell ) const
{
  return headerHeight() + cell->y() * rowHeight();
}

int MonthScene::cellHorizontalPos( const MonthCell *cell ) const
{
  return cell->x() * columnWidth();
}

int MonthScene::sceneYToMonthGridY( int yScene )
{
  return yScene - headerHeight();
}

int MonthScene::sceneXToMonthGridX( int xScene )
{
  return xScene;
}

void MonthGraphicsView::drawBackground( QPainter *p, const QRectF & rect )
{
  p->setFont( KOPrefs::instance()->mMonthViewFont );
  p->fillRect( rect, Qt::white );

  /*
    Headers
  */
  QFont font = KOPrefs::instance()->mMonthViewFont;
  font.setBold( true );
  font.setPointSize( 15 );
  p->setFont( font );
  const int dayLabelsHeight = 20;
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
  p->drawText( QRect( 0,  0, // top right
                      static_cast<int> ( mScene->sceneRect().width() ),
                      static_cast<int> ( mScene->headerHeight() - dayLabelsHeight ) ),
               Qt::AlignCenter,
               i18nc( "monthname year", "%1 %2",
                      calSys->monthName( mMonthView->averageDate() ),
                      calSys->yearString( mMonthView->averageDate() ) ) );

  font.setPixelSize( dayLabelsHeight - 10 );
  p->setFont( font );
  for ( QDate d = mMonthView->mStartDate;
        d <= mMonthView->mStartDate.addDays( 6 ); d = d.addDays( 1 ) ) {
    MonthCell *cell = mScene->mMonthCellMap[ d ];

    if ( !cell ) {
      // This means drawBackground() is being called before reloadIncidences(). Can happen with some
      // themes. Bug  #190191
      return;
    }

    p->drawText( QRect( mScene->cellHorizontalPos( cell ),
                        mScene->cellVerticalPos( cell ) - 15,
                        mScene->columnWidth(),
                        15 ),
                 Qt::AlignCenter,
                 calSys->weekDayName( d, KCalendarSystem::LongDayName ) );
  }

  /*
    Month grid
  */
  int columnWidth = mScene->columnWidth();
  int rowHeight = mScene->rowHeight();

  for ( QDate d = mMonthView->mStartDate; d <= mMonthView->mEndDate; d = d.addDays( 1 ) ) {
    MonthCell *cell = mScene->mMonthCellMap[ d ];

    QColor color;
    if ( KOGlobals::self()->isWorkDay( d ) ) {
      color = KOPrefs::instance()->monthGridWorkHoursBackgroundColor();
    } else {
      color = KOPrefs::instance()->monthGridBackgroundColor();
    }
    if ( cell == mScene->selectedCell() ) {
      color = color.dark( 115 );
    }
    if ( cell->date() == QDate::currentDate() ) {
      color = color.dark( 140 );
    }

    // Draw cell
    p->setPen( KOPrefs::instance()->monthGridBackgroundColor().dark( 150 ) );
    p->setBrush( color );
    p->drawRect( QRect( mScene->cellHorizontalPos( cell ), mScene->cellVerticalPos( cell ),
                        columnWidth, rowHeight ) );

    // Draw cell header
    int cellHeaderX = mScene->cellHorizontalPos( cell ) + 1;
    int cellHeaderY = mScene->cellVerticalPos( cell ) + 1;
    int cellHeaderWidth = columnWidth - 2;
    int cellHeaderHeight = cell->topMargin() - 2;
    QLinearGradient bgGradient( QPointF( cellHeaderX, cellHeaderY ),
                                QPointF( cellHeaderX + cellHeaderWidth,
                                         cellHeaderY + cellHeaderHeight ) );
    bgGradient.setColorAt( 0, color.dark( 105 ) );
    bgGradient.setColorAt( 0.7, color.dark( 105 ) );
    bgGradient.setColorAt( 1, color );
    p->setBrush( bgGradient );

    p->setPen( Qt::NoPen );
    p->drawRect( QRect( cellHeaderX, cellHeaderY,
                        cellHeaderWidth, cellHeaderHeight ) );
  }

  font = KOPrefs::instance()->mMonthViewFont;
  font.setPixelSize( MonthCell::topMargin() - 4 );

  p->setFont( font );

  QPen oldPen =  KOPrefs::instance()->monthGridBackgroundColor().dark( 150 );

  // Draw dates
  for ( QDate d = mMonthView->mStartDate; d <= mMonthView->mEndDate; d = d.addDays( 1 ) ) {
    MonthCell *cell = mScene->mMonthCellMap.value( d );

    QFont font = p->font();
    if ( cell->date() == QDate::currentDate() ) {
      font.setBold( true );
    } else {
      font.setBold( false );
    }
    p->setFont( font );

    if ( d.month() == mMonthView->mCurrentMonth ) {
      p->setPen( QPalette::Text );
    } else {
      p->setPen( oldPen );
    }

    /*
      Draw arrows if all items won't fit
    */

    // Up arrow if first item is above cell top
    if ( mScene->startHeight() != 0 && cell->hasEventBelow( mScene->startHeight() ) ) {
      cell->upArrow()->setPos(
        mScene->cellHorizontalPos( cell ) + columnWidth / 2,
        mScene->cellVerticalPos( cell ) + cell->upArrow()->boundingRect().height() / 2 + 2 );
      cell->upArrow()->show();
    } else {
      cell->upArrow()->hide();
    }

    // Down arrow if last item is below cell bottom
    if ( !mScene->lastItemFit( cell ) ) {
      cell->downArrow()->setPos(
        mScene->cellHorizontalPos( cell ) + columnWidth / 2,
        mScene->cellVerticalPos( cell ) + rowHeight -
        cell->downArrow()->boundingRect().height() / 2 - 2 );
      cell->downArrow()->show();
    } else {
      cell->downArrow()->hide();
    }

    const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

    QString dayText;
    // Prepend month name if d is the first or last day of month
    if ( calSys->day( d ) == 1 ||                // d is the first day of month
         calSys->day( d.addDays( 1 ) ) == 1 ) {  // d is the last day of month
      dayText = i18nc( "'Month day' for month view cells", "%1 %2",
                  calSys->monthName( d, KCalendarSystem::ShortName ),
                  calSys->day( d ) );
    } else {
      dayText = QString::number( calSys->day( d ) );
    }

    p->drawText( QRect( mScene->cellHorizontalPos( cell ), // top right
                        mScene->cellVerticalPos( cell ),     // of the cell
                        mScene->columnWidth() - 2,
                        cell->topMargin() ),
                 Qt::AlignRight,
                 dayText );
  }

  // ...
}

void MonthScene::resetAll()
{
  qDeleteAll( mMonthCellMap );
  mMonthCellMap.clear();

  qDeleteAll( mManagerList );
  mManagerList.clear();

  mSelectedItem = 0;
  mActionItem = 0;
  mClickedItem = 0;
}

IncidenceChangerBase *MonthScene::incidenceChanger() const
{
  return mMonthView->mChanger;
}

QDate MonthScene::firstDateOnRow( int row ) const
{
  return mMonthView->startDate().addDays( 7 * row );
}

bool MonthScene::lastItemFit( MonthCell *cell )
{
  if ( cell->firstFreeSpace() > maxRowCount() + startHeight() ) {
    return false;
  } else {
    return true;
  }
}

int MonthScene::totalHeight()
{
  int max = 0;
  for ( QDate d = mMonthView->mStartDate; d <= mMonthView->mEndDate; d = d.addDays( 1 ) ) {
    int c = mMonthCellMap[ d ]->firstFreeSpace();
    if ( c > max ) {
      max = c;
    }
  }

  return max;
}

void MonthScene::wheelEvent( QGraphicsSceneWheelEvent *event )
{
  Q_UNUSED( event ); // until we figure out what to do in here

/*  int numDegrees = -event->delta() / 8;
  int numSteps = numDegrees / 15;

  if ( startHeight() + numSteps < 0 ) {
    numSteps = -startHeight();
  }

  int cellHeight = 0;

  MonthCell *currentCell = getCellFromPos( event->scenePos() );
  if ( currentCell ) {
    cellHeight = currentCell->firstFreeSpace();
  }
  if ( cellHeight == 0 ) {
    // no items in this cell, there's no point to scroll
    return;
  }

  int newHeight;
  int maxStartHeight = qMax( 0, cellHeight - maxRowCount() );
  if ( numSteps > 0  && startHeight() + numSteps >= maxStartHeight ) {
    newHeight = maxStartHeight;
  } else {
    newHeight = startHeight() + numSteps;
  }

  if ( newHeight == startHeight() ) {
    return;
  }

  setStartHeight( newHeight );

  foreach ( MonthItem *manager, mManagerList ) {
    manager->updateGeometry();
  }

  invalidate( QRectF(), BackgroundLayer );

  event->accept();
*/
}

void MonthScene::scrollCellsDown()
{
  int newHeight = startHeight() + 1;
  setStartHeight( newHeight );

  foreach ( MonthItem *manager, mManagerList ) {
    manager->updateGeometry();
  }

  invalidate( QRectF(), BackgroundLayer );
}

void MonthScene::scrollCellsUp()
{
  int newHeight = startHeight() - 1;
  setStartHeight( newHeight );

  foreach ( MonthItem *manager, mManagerList ) {
    manager->updateGeometry();
  }

  invalidate( QRectF(), BackgroundLayer );
}

void MonthScene::clickOnScrollIndicator( ScrollIndicator *scrollItem )
{
  if ( scrollItem->direction() == ScrollIndicator::UpArrow ) {
    scrollCellsUp();
  } else if ( scrollItem->direction() == ScrollIndicator::DownArrow ) {
    scrollCellsDown();
  }
}

void MonthScene::mouseDoubleClickEvent ( QGraphicsSceneMouseEvent *mouseEvent )
{
  QPointF pos = mouseEvent->scenePos();

  MonthGraphicsItem *iItem;
  if ( ( iItem = dynamic_cast<MonthGraphicsItem*>( itemAt( pos ) ) ) ) {
    if ( iItem->monthItem() ) {
      IncidenceMonthItem *tmp = dynamic_cast<IncidenceMonthItem *>( iItem->monthItem() );
      if ( tmp ) {
        selectItem( iItem->monthItem() );
        mMonthView->defaultAction( tmp->incidence() );

        mouseEvent->accept();
      }
    }
  } else {
    emit newEventSignal();
  }
}

void MonthScene::mouseMoveEvent ( QGraphicsSceneMouseEvent *mouseEvent )
{
  QPointF pos = mouseEvent->scenePos();

  MonthGraphicsView *view = static_cast<MonthGraphicsView*>( views().first() );

  // Change cursor depending on the part of the item it hovers to inform
  // the user that he can resize the item.
  if ( mActionType == None ) {
    MonthGraphicsItem *iItem;
    if ( ( iItem = dynamic_cast<MonthGraphicsItem*>( itemAt( pos ) ) ) ) {
      if ( iItem->monthItem()->isResizable() &&
            iItem->isBeginItem() && iItem->mapFromScene( pos ).x() <= 10 ) {
        view->setActionCursor( Resize );
      } else if ( iItem->monthItem()->isResizable() &&
                    iItem->isEndItem() &&
                    iItem->mapFromScene( pos ).x() >= iItem->boundingRect().width() - 10 ) {
        view->setActionCursor( Resize );
      } else {
        view->setActionCursor( None );
      }
    } else {
      view->setActionCursor( None );
    }
    mouseEvent->accept();
    return;
  }

  // If an item was selected during the click, we maybe have an item to move !
  if ( mActionItem ) {
    MonthCell *currentCell = getCellFromPos( pos );

    // Initiate action if not already done
    if ( !mActionInitiated && mActionType != None ) {
      if ( mActionType == Move ) {
        mActionItem->beginMove();
      } else if ( mActionType == Resize ) {
        mActionItem->beginResize();
      }
      mActionInitiated = true;
    }
    view->setActionCursor( mActionType );

    // Move or resize action
    if ( currentCell && currentCell != mPreviousCell ) {

      bool ok = true;
      if ( mActionType == Move ) {
        mActionItem->moveBy( mPreviousCell->date().daysTo( currentCell->date() ) );
      } else if ( mActionType == Resize ) {
        ok = mActionItem->resizeBy( mPreviousCell->date().daysTo( currentCell->date() ) );
      }

      if ( ok ) {
        mPreviousCell = currentCell;
      }
      mActionItem->updateGeometry();
      update();
    }
    mouseEvent->accept();
  }
}

void MonthScene::mousePressEvent ( QGraphicsSceneMouseEvent *mouseEvent )
{
  QPointF pos = mouseEvent->scenePos();

  mClickedItem = 0;

  MonthGraphicsItem *iItem;
  if ( ( iItem = dynamic_cast<MonthGraphicsItem*>( itemAt( pos ) ) ) ) {
    mClickedItem = iItem->monthItem();

    selectItem( mClickedItem );
    if ( mouseEvent->button() == Qt::RightButton ) {
      IncidenceMonthItem *tmp = dynamic_cast<IncidenceMonthItem *>( mClickedItem );
      if ( tmp ) {
        emit showIncidencePopupSignal( mCalendar,
                                       tmp->incidence(), tmp->realStartDate() );
      }
    }

    if ( mouseEvent->button() == Qt::LeftButton ) {
      // Basic initialization for resize and move
      mActionItem = mClickedItem;
      mStartCell = getCellFromPos( pos );
      mPreviousCell = mStartCell;
      mActionInitiated = false;

      // Move or resize ?
      if ( iItem->monthItem()->isResizable() &&
            iItem->isBeginItem() && iItem->mapFromScene( pos ).x() <= 10 ) {
        mActionType = Resize;
        mResizeType = ResizeLeft;
      } else if ( iItem->monthItem()->isResizable() &&
                    iItem->isEndItem() &&
                    iItem->mapFromScene( pos ).x() >= iItem->boundingRect().width() - 10 ) {
        mActionType = Resize;
        mResizeType = ResizeRight;
      } else if ( iItem->monthItem()->isMoveable() ) {
        mActionType = Move;
      }
    }
    mouseEvent->accept();
  } else if ( ScrollIndicator *scrollItem = dynamic_cast<ScrollIndicator*>( itemAt( pos ) ) ) {
    clickOnScrollIndicator( scrollItem );
  } else {
    // unselect items when clicking somewhere else
    selectItem( 0 );

    MonthCell *cell = getCellFromPos( pos );
    if ( cell ) {
      mSelectedCellDate = cell->date();
      update();
      if ( mouseEvent->button() == Qt::RightButton ) {
        emit showNewEventPopupSignal();
      }
      mouseEvent->accept();
    }
  }
}

void MonthScene::mouseReleaseEvent ( QGraphicsSceneMouseEvent *mouseEvent )
{
  QPointF pos = mouseEvent->scenePos();

  static_cast<MonthGraphicsView*>( views().first() )->setActionCursor( None );

  if ( mActionItem ) {
    MonthCell *currentCell = getCellFromPos( pos );
    if ( currentCell && currentCell != mStartCell ) { // We want to act if a move really happened
      if ( mActionType == Resize ) {
        mActionItem->endResize();
      } else if ( mActionType == Move ) {
        mActionItem->endMove();
      }
    }

    mActionItem = 0;
    mActionType = None;
    mStartCell = 0;

    // FIXME: WOW, quite heavy if only a move happened...
    // and BTW, when changing an incidence, we should get an event anyway,
    // which should trigger the reload...
    mMonthView->reloadIncidences();

    mouseEvent->accept();
  }
}

// returns true if the point is in the monthgrid (allows to avoid selecting a cell when
// a click is outside the month grid
bool MonthScene::isInMonthGrid( int x, int y ) {
  return x >= 0 && y >= 0 && x <= availableWidth() && y <= availableHeight();
}

// The function converts the coordinates to the month grid coordinates to
// be able to locate the cell.
MonthCell *MonthScene::getCellFromPos( const QPointF &pos )
{
  int y = sceneYToMonthGridY( static_cast<int> ( pos.y() ) );
  int x = sceneXToMonthGridX( static_cast<int> ( pos.x() ) );
  if ( !isInMonthGrid( x, y ) ) {
    return 0;
  }
  int id = ( int )( y / rowHeight() ) * 7 + ( int )( x / columnWidth() );

  return mMonthCellMap.value( mMonthView->mStartDate.addDays( id ) );
}

void MonthScene::selectItem( MonthItem *item )
{
  if ( mSelectedItem == item ) {
    return;
  }

  IncidenceMonthItem *tmp = dynamic_cast<IncidenceMonthItem *>( item );

  if ( !tmp ) {
    mSelectedItem = 0;
    emit incidenceSelected( 0 );
    return;
  }

  mSelectedItem = item;
  Q_ASSERT( tmp->incidence() );

  emit incidenceSelected( tmp->incidence() );
}

//----------------------------------------------------------
MonthGraphicsView::MonthGraphicsView( MonthView *parent )
  : QGraphicsView( parent ), mMonthView( parent )
{
  setMouseTracking( true );
}

void MonthGraphicsView::setActionCursor( MonthScene::ActionType actionType )
{
   switch ( actionType ) {
   case MonthScene::Move:
     setCursor( Qt::ArrowCursor );
     break;
   case MonthScene::Resize:
     setCursor( Qt::SizeHorCursor );
     break;
   default:
     setCursor( Qt::ArrowCursor );
   }
}

void MonthGraphicsView::setScene( MonthScene *scene )
{
  mScene = scene;
  QGraphicsView::setScene( scene );
}

void MonthGraphicsView::resizeEvent( QResizeEvent *event )
{
  mScene->setSceneRect( 0, 0, event->size().width(), event->size().height() );
  mScene->updateGeometry();
}
