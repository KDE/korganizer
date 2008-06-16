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
#include "monthview.h"
#include "koprefs.h"
#include "koglobals.h"

#include <kcal/incidence.h>
#include <kcalendarsystem.h>

#include <QPaintEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneMouseEvent>

using namespace KOrg;

MonthGraphicsView::MonthGraphicsView( MonthView *parent, Calendar *calendar )
  : QGraphicsView( parent ), mMonthView( parent )
{
  setMouseTracking( true );
}

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
    mMovingMonthGraphicsItem( 0 ),
    mStartHeight( 0 ),
    mActionType( None )

{
  installEventFilter( this );
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
  return ( rowHeight() - MonthCell::topMargin() ) / itemHeight();
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
  return sceneRect().width();
}

int MonthScene::availableHeight() const
{
  return sceneRect().height() - headerHeight();
}

int MonthScene::columnWidth() const
{
  return ( availableWidth() - 1 ) / 7.;
}

int MonthScene::rowHeight() const
{
  return ( availableHeight() - 1 ) / 6.;
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
                      mScene->sceneRect().width(), mScene->headerHeight() - dayLabelsHeight ),
               Qt::AlignCenter,
               i18nc( "monthname year", "%1 %2",
                      calSys->monthName( mMonthView->averageDate() ),
                      calSys->yearString( mMonthView->averageDate() ) ) );

  font.setPixelSize( dayLabelsHeight - 10 );
  p->setFont( font );
  for ( QDate d = mMonthView->mStartDate;
        d <= mMonthView->mStartDate.addDays( 6 ); d = d.addDays( 1 ) ) {
    MonthCell *cell = mScene->mMonthCellMap[ d ];
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
  QFontMetrics fm = p->fontMetrics();
  int numberWidth = fm.boundingRect( "OO" ).width(); // biggest width

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

    if ( mScene->startHeight() != 0 && cell->hasEventBelow( mScene->startHeight() ) ) {
      int arrowCenter = mScene->cellHorizontalPos( cell ) + 5;
      int arrowHead = MonthCell::topMargin() / 2 - 3;
      QPolygon upArrow( 3 );
      upArrow.setPoint( 0, arrowCenter, mScene->cellVerticalPos( cell ) + arrowHead );
      upArrow.setPoint( 1, arrowCenter + 2, mScene->cellVerticalPos( cell ) + arrowHead + 2 );
      upArrow.setPoint( 2, arrowCenter - 2, mScene->cellVerticalPos( cell )+ arrowHead + 2 );
      p->setBrush( Qt::black );
      p->drawPolygon( upArrow );
    }

    if ( !mScene->lastItemFit( cell ) ) {
      int arrowCenter = mScene->cellHorizontalPos( cell ) + 5;
      int arrowHead = MonthCell::topMargin() / 2 + 3;
      QPolygon downArrow( 3 );
      downArrow.setPoint( 0, arrowCenter, mScene->cellVerticalPos( cell ) + arrowHead );
      downArrow.setPoint( 1, arrowCenter + 2, mScene->cellVerticalPos( cell ) + arrowHead - 2 );
      downArrow.setPoint( 2, arrowCenter - 2, mScene->cellVerticalPos( cell ) + arrowHead - 2 );
      p->setBrush( Qt::black );
      p->drawPolygon( downArrow );
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
  mMovingMonthGraphicsItem = 0;
  mActionItem = 0;
  mClickedItem = 0;
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

bool MonthScene::eventFilter( QObject *object, QEvent *event )
{
  switch( event->type() ) {
  case QEvent::GraphicsSceneMouseMove:
  case QEvent::GraphicsSceneMouseRelease:
  case QEvent::GraphicsSceneMousePress:
  case QEvent::GraphicsSceneMouseDoubleClick:
    return eventFilterMouse( object, static_cast<QGraphicsSceneMouseEvent*>( event ) );
  case QEvent::GraphicsSceneWheel:
    return eventFilterWheel( object, static_cast<QGraphicsSceneWheelEvent *>( event ) );
  default:
    return false;
  }
}

bool MonthScene::eventFilterWheel( QObject *object, QGraphicsSceneWheelEvent *event )
{
  int numDegrees = -event->delta() / 8;
  int numSteps = numDegrees / 15;

  if ( startHeight() + numSteps < 0 ) {
    return true;
  }

  int newHeight;
  int maxStartHeight = qMax( 0, totalHeight() - maxRowCount() );
  if ( numSteps > 0  && startHeight() + numSteps >= maxStartHeight ) {
    newHeight = maxStartHeight;
  } else {
    newHeight = startHeight() + numSteps;
  }

  setStartHeight( newHeight );

  foreach ( MonthItem *manager, mManagerList ) {
    manager->updateGeometry();
  }

  invalidate( QRectF(), BackgroundLayer );

  return true;
}

bool MonthScene::eventFilterMouse( QObject *object, QGraphicsSceneMouseEvent *event )
{
  QPointF pos = event->scenePos();

  // Check the type and do the correct action
  switch ( event->type() )  {
  case QEvent::GraphicsSceneMouseDoubleClick:
    if ( itemAt( pos ) ) {
      MonthGraphicsItem *iItem = dynamic_cast<MonthGraphicsItem*>( itemAt( pos ) );
      if ( iItem->monthItem() ) {
        selectItem( iItem->monthItem() );
        mMonthView->defaultAction( iItem->monthItem()->incidence() );
      }
    }
    return true;
  case QEvent::GraphicsSceneMousePress:
    if ( itemAt( pos ) ) {
      MonthGraphicsItem *iItem = dynamic_cast<MonthGraphicsItem*>( itemAt( pos ) );
      mClickedItem = 0;
      if ( iItem ) {
        mClickedItem = iItem->monthItem();
      }

      selectItem( mClickedItem ); // if clickedItem is null, the item will be deselected
      if ( event->button() == Qt::RightButton ) {
        emit showIncidencePopupSignal( mClickedItem->incidence(),
                                       mClickedItem->startDate() ); // FIXME ?
      }

      if ( event->button() == Qt::LeftButton ) {
        // Basic initialization for resize and move
        mActionItem = mClickedItem;
        mActionMonthGraphicsItem = iItem;
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
        } else {
          mActionType = Move;
        }
      }
      return true;
    } else {
      MonthCell *cell = getCellFromPos( pos );
      if ( cell ) {
        mSelectedCellDate = cell->date();
        update();
        if ( event->button() == Qt::RightButton ) {
          emit showNewEventPopupSignal();
        }
      }
      return true;
    }
    break;
  case QEvent::GraphicsSceneMouseRelease:
    static_cast<MonthGraphicsView*>( views().first() )->setActionCursor( None );

    if ( mActionItem ) {
      MonthCell *currentCell = getCellFromPos( pos );
      if ( currentCell && currentCell != mStartCell ) { // We want to act if a move really happened
        mMovingMonthGraphicsItem = 0;
        //mActionItem->move( false );
        if ( mActionType == Resize ) {
          mActionItem->resize( false );
        } else if ( mActionType == Move ) {
          mActionItem->move( false );
        }
      }

      mActionItem = 0;
      mActionType = None;
      mActionMonthGraphicsItem = 0;
      mStartCell = 0;

      mMonthView->reloadIncidences();
    }
    return true;
    break;
  case QEvent::GraphicsSceneMouseMove:

    // Change cursor depending on the part of the item it hovers to inform
    // the user that he can resize the item.
    if ( mActionType == None ) {
      if ( itemAt( pos ) ) {
        MonthGraphicsItem *iItem = dynamic_cast<MonthGraphicsItem*>( itemAt( pos ) );

        if ( iItem ) {
          if ( iItem->monthItem()->isResizable() &&
               iItem->isBeginItem() && iItem->mapFromScene( pos ).x() <= 10 ) {
            static_cast<MonthGraphicsView*>( views().first() )->setActionCursor( Resize );
          } else if ( iItem->monthItem()->isResizable() &&
                      iItem->isEndItem() &&
                      iItem->mapFromScene( pos ).x() >= iItem->boundingRect().width() - 10 ) {
            static_cast<MonthGraphicsView*>( views().first() )->setActionCursor( Resize );
          } else {
            static_cast<MonthGraphicsView*>( views().first() )->setActionCursor( None );
          }
          return true;
        }
      } else {
        static_cast<MonthGraphicsView*>( views().first() )->setActionCursor( None );
        return true;
      }
    }

    // If an item was selected during the click, we maybe have an item to move !
    if ( mActionItem ) {
      MonthCell *currentCell = getCellFromPos( pos );

      // Initiate action if not already done
      if ( !mActionInitiated && mActionType != None ) {
        mMovingMonthGraphicsItem = mActionMonthGraphicsItem; // todo rename used by both actions
        if ( mActionType == Move ) {
          mActionItem->move( true );
        } else if ( mActionType == Resize ) {
          mActionItem->resize( true );
        }
        mActionInitiated = true;
      }
      static_cast<MonthGraphicsView*>( views().first() )->setActionCursor( mActionType );

      // Move or resize action
      if ( currentCell && currentCell != mPreviousCell ) {

        if ( mActionType == Move ) {
          mActionItem->moving( mPreviousCell->date().daysTo( currentCell->date() ) );
        } else if ( mActionType == Resize ) {
          mActionItem->resizing( mPreviousCell->date().daysTo( currentCell->date() ) );
        }

        mPreviousCell = currentCell;
        mActionItem->updateGeometry();
        update();
      }
      return true;
    }

    return false;
    break;
  default:
    return false;
  }
  return false;
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
  int y = sceneYToMonthGridY( pos.y() );
  int x = sceneXToMonthGridX( pos.x() );
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

  if ( item == 0 ) {
    mSelectedItem = 0;
    emit incidenceSelected( 0 );
    return;
  }

  mSelectedItem = item;
  Q_ASSERT( mSelectedItem->incidence() );

  emit incidenceSelected( mSelectedItem->incidence() );
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
  mScene->setSceneRect( 0, 0, width(), height() );
  mScene->updateGeometry();
}
