/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    Marcus Bains line.
    Copyright (c) 2001 Ali Rahimi

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

#include <qintdict.h>
#include <qdatetime.h>
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qpainter.h>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>

#include "koagendaitem.h"
#include "koprefs.h"
#include "koglobals.h"

#include "koagenda.h"
#include "koagenda.moc"

#include <libkcal/event.h>
#include <libkcal/todo.h>
#include <libkcal/dndfactory.h>
#include <libkcal/icaldrag.h>
#include <libkcal/vcaldrag.h>

////////////////////////////////////////////////////////////////////////////
MarcusBains::MarcusBains(KOAgenda *_agenda,const char *name)
    : QFrame(_agenda->viewport(),name), agenda(_agenda)
{
  setLineWidth(0);
  setMargin(0);
  setBackgroundColor(Qt::red);
  minutes = new QTimer(this);
  connect(minutes, SIGNAL(timeout()), this, SLOT(updateLocation()));
  minutes->start(0, true);

  mTimeBox = new QLabel(this);
  mTimeBox->setAlignment(Qt::AlignRight | Qt::AlignBottom);
  QPalette pal = mTimeBox->palette();
  pal.setColor(QColorGroup::Foreground, Qt::red);
  mTimeBox->setPalette(pal);
  mTimeBox->setAutoMask(true);

  agenda->addChild(mTimeBox);

  oldToday = -1;
}

MarcusBains::~MarcusBains()
{
  delete minutes;
}

int MarcusBains::todayColumn()
{
  QDate currentDate = QDate::currentDate();

  DateList dateList = agenda->dateList();
  DateList::ConstIterator it;
  int col = 0;
  for(it = dateList.begin(); it != dateList.end(); ++it) {
    if((*it) == currentDate)
      return KOGlobals::self()->reverseLayout() ?
             agenda->columns() - 1 - col : col;
      ++col;
  }

  return -1;
}

void MarcusBains::updateLocation(bool recalculate)
{
  QTime tim = QTime::currentTime();
  if((tim.hour() == 0) && (oldTime.hour()==23))
    recalculate = true;

  int mins = tim.hour()*60 + tim.minute();
  int minutesPerCell = 24 * 60 / agenda->rows();
  int y = (int)(mins*agenda->gridSpacingY()/minutesPerCell);
  int today = recalculate ? todayColumn() : oldToday;
  int x = (int)( agenda->gridSpacingX()*today );
  bool disabled = !(KOPrefs::instance()->mMarcusBainsEnabled);

  oldTime = tim;
  oldToday = today;

  if(disabled || (today<0)) {
    hide(); mTimeBox->hide();
    return;
  } else {
    show(); mTimeBox->show();
  }

  if(recalculate)
    setFixedSize((int)(agenda->gridSpacingX()),1);
  agenda->moveChild(this, x, y);
  raise();

  if(recalculate)
    mTimeBox->setFont(KOPrefs::instance()->mMarcusBainsFont);

  mTimeBox->setText(KGlobal::locale()->formatTime(tim, KOPrefs::instance()->mMarcusBainsShowSeconds));
  mTimeBox->adjustSize();
  agenda->moveChild(mTimeBox,
                    (int)(x+agenda->gridSpacingX()-mTimeBox->width()-1),
                    y-mTimeBox->height());
  mTimeBox->raise();
  mTimeBox->setAutoMask(true);

  minutes->start(1000,true);
}


////////////////////////////////////////////////////////////////////////////


/*
  Create an agenda widget with rows rows and columns columns.
*/
KOAgenda::KOAgenda( int columns, int rows, int rowSize, QWidget *parent,
                    const char *name, WFlags f )
  : QScrollView( parent, name, f )
{
  mColumns = columns;
  mRows = rows;
  mGridSpacingY = rowSize;
  mAllDayMode = false;

  init();
}

/*
  Create an agenda widget with columns columns and one row. This is used for
  all-day events.
*/
KOAgenda::KOAgenda( int columns, QWidget *parent, const char *name, WFlags f )
  : QScrollView( parent, name, f )
{
  mColumns = columns;
  mRows = 1;
  mGridSpacingY = 24;
  mAllDayMode = true;

  init();
}


KOAgenda::~KOAgenda()
{
  delete mMarcusBains;
}


Incidence *KOAgenda::selectedIncidence() const
{
  return ( mSelectedItem ? mSelectedItem->incidence() : 0 );
}


QDate KOAgenda::selectedIncidenceDate() const
{
  return ( mSelectedItem ? mSelectedItem->itemDate() : QDate() );
}


void KOAgenda::init()
{
  mGridSpacingX = 100;

  mResizeBorderWidth = 8;
  mScrollBorderWidth = 8;
  mScrollDelay = 30;
  mScrollOffset = 10;

  enableClipper( true );

  // Grab key strokes for keyboard navigation of agenda. Seems to have no
  // effect. Has to be fixed.
  setFocusPolicy( WheelFocus );

  connect( &mScrollUpTimer, SIGNAL( timeout() ), SLOT( scrollUp() ) );
  connect( &mScrollDownTimer, SIGNAL( timeout() ), SLOT( scrollDown() ) );

  mStartCellX = 0;
  mStartCellY = 0;
  mCurrentCellX = 0;
  mCurrentCellY = 0;

  mSelectionCellX = 0;
  mSelectionYTop = 0;
  mSelectionHeight = 0;

  mOldLowerScrollValue = -1;
  mOldUpperScrollValue = -1;

  mClickedItem = 0;

  mActionItem = 0;
  mActionType = NOP;
  mItemMoved = false;

  mSelectedItem = 0;

  setAcceptDrops( true );
  installEventFilter( this );
  mItems.setAutoDelete( true );

//  resizeContents( (int)(mGridSpacingX * mColumns + 1) , (int)(mGridSpacingY * mRows + 1) );
  resizeContents( int( mGridSpacingX * mColumns ),
                  int( mGridSpacingY * mRows ) );

  viewport()->update();
  viewport()->setBackgroundMode( NoBackground );
  viewport()->setFocusPolicy( WheelFocus );

  setMinimumSize( 30, int( mGridSpacingY + 1 ) );
//  setMaximumHeight(mGridSpacingY * mRows + 5);

  // Disable horizontal scrollbar. This is a hack. The geometry should be
  // controlled in a way that the contents horizontally always fits. Then it is
  // not necessary to turn off the scrollbar.
  setHScrollBarMode( AlwaysOff );

  setStartHour( KOPrefs::instance()->mDayBegins );

  calculateWorkingHours();

  connect( verticalScrollBar(), SIGNAL( valueChanged( int ) ),
           SLOT( checkScrollBoundaries( int ) ) );

  // Create the Marcus Bains line.
  if( mAllDayMode ) {
    mMarcusBains = 0;
  } else {
    mMarcusBains = new MarcusBains( this );
    addChild( mMarcusBains );
  }

  mTypeAhead = false;
  mTypeAheadReceiver = 0;

  mReturnPressed = false;
}


void KOAgenda::clear()
{
//  kdDebug(5850) << "KOAgenda::clear()" << endl;

  KOAgendaItem *item;
  for ( item=mItems.first(); item != 0; item=mItems.next() ) {
    removeChild(item);
  }
  mItems.clear();

  mSelectedItem = 0;

  clearSelection();
}


void KOAgenda::clearSelection()
{
  mSelectionCellX = 0;
  mSelectionYTop = 0;
  mSelectionHeight = 0;
}

void KOAgenda::marcus_bains()
{
    if(mMarcusBains) mMarcusBains->updateLocation(true);
}


void KOAgenda::changeColumns(int columns)
{
  if (columns == 0) {
    kdDebug(5850) << "KOAgenda::changeColumns() called with argument 0" << endl;
    return;
  }

  clear();
  mColumns = columns;
//  setMinimumSize(mColumns * 10, mGridSpacingY + 1);
//  init();
//  update();

  QResizeEvent event( size(), size() );

  QApplication::sendEvent( this, &event );
}

/*
  This is the eventFilter function, which gets all events from the KOAgendaItems
  contained in the agenda. It has to handle moving and resizing for all items.
*/
bool KOAgenda::eventFilter ( QObject *object, QEvent *event )
{
//  kdDebug(5850) << "KOAgenda::eventFilter() " << int( event->type() ) << endl;

  switch( event->type() ) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
      return eventFilter_mouse( object, static_cast<QMouseEvent *>( event ) );

    case QEvent::KeyPress:
    case QEvent::KeyRelease:
      return eventFilter_key( object, static_cast<QKeyEvent *>( event ) );

    case ( QEvent::Leave ):
      if ( !mActionItem )
        setCursor( arrowCursor );
      return true;

#ifndef KORG_NODND
    case QEvent::DragEnter:
    case QEvent::DragMove:
    case QEvent::DragLeave:
    case QEvent::Drop:
 //   case QEvent::DragResponse:
      return eventFilter_drag(object, static_cast<QDropEvent*>(event));
#endif

    default:
      return QScrollView::eventFilter( object, event );
  }
}

bool KOAgenda::eventFilter_drag(QObject *object, QDropEvent *de)
{
#ifndef KORG_NODND
  QPoint viewportPos;
  if (object != viewport() && object != this ) {
    viewportPos = ((QWidget *)object)->mapToParent(de->pos());
  } else {
    viewportPos = de->pos();
  }

  switch (de->type())  {
    case QEvent::DragEnter:
    case QEvent::DragMove:
      if ( ICalDrag::canDecode( de ) || VCalDrag::canDecode( de ) ) {

        DndFactory factory( mCalendar );
        Todo *todo = factory.createDropTodo( de );
        if (todo) {
          de->accept();
          delete todo;
        } else {
          de->ignore();
        }
        return true;
      } else return false;
      break;
    case QEvent::DragLeave:
      return false;
      break;
    case QEvent::Drop:
      {
        if ( !ICalDrag::canDecode( de ) && !VCalDrag::canDecode( de ) ) {
          return false;
        }

        DndFactory factory( mCalendar );
        Todo *todo = factory.createDropTodo( de );

        if ( todo ) {
          de->acceptAction();
          int x, y;
          // FIXME: This is a bad hack, as the viewportToContents seems to be off by
          // 2000 (which is the left upper corner of the viewport). It works correctly
          // for agendaItems.
          if ( object == this  ) {
            x=viewportPos.x()+contentsX();
            y=viewportPos.y()+contentsY();
          } else {
            viewportToContents( viewportPos.x(), viewportPos.y(), x, y );
          }
          int gx, gy;
          contentsToGrid( x, y, gx, gy );
          emit droppedToDo( todo, gx, gy, mAllDayMode );
          return true;
        }
      }
      break;

    case QEvent::DragResponse:
    default: break;
  }
#endif

  return false;
}

bool KOAgenda::eventFilter_key( QObject *, QKeyEvent *ke )
{
//  kdDebug() << "KOAgenda::eventFilter_key() " << ke->type() << endl;

  // If Return is pressed bring up an editor for the current selected time span.
  if ( ke->key() == Key_Return ) {
    if ( ke->type() == QEvent::KeyPress ) mReturnPressed = true;
    else if ( ke->type() == QEvent::KeyRelease ) {
      if ( mReturnPressed ) {
        emitNewEventForSelection();
        mReturnPressed = false;
        return true;
      } else {
        mReturnPressed = false;
      }
    }
  }

  // Ignore all input that does not produce any output
  if ( ke->text().isEmpty() ) return false;

  if ( ke->type() == QEvent::KeyPress || ke->type() == QEvent::KeyRelease ) {
    switch ( ke->key() ) {
      case Key_Escape:
      case Key_Return:
      case Key_Enter:
      case Key_Tab:
      case Key_Backtab:
      case Key_Left:
      case Key_Right:
      case Key_Up:
      case Key_Down:
      case Key_Backspace:
      case Key_Delete:
      case Key_Prior:
      case Key_Next:
      case Key_Home:
      case Key_End:
      case Key_Control:
      case Key_Meta:
      case Key_Alt:
        break;
      default:
        mTypeAheadEvents.append( new QKeyEvent( ke->type(), ke->key(),
                                                ke->ascii(), ke->state(),
                                                ke->text(), ke->isAutoRepeat(),
                                                ke->count() ) );
        if ( !mTypeAhead ) {
          mTypeAhead = true;
          emitNewEventForSelection();
          return true;
        }
        break;
    }
  }
  return false;
}

void KOAgenda::emitNewEventForSelection()
{
  if ( mSelectionHeight > 0 ) {
    // Subtract 1 from the bottom, because the ybottom describes the last cell in
    // the event, not the one after the item
    emit newEventSignal( mSelectionCellX, (int)(mSelectionYTop / mGridSpacingY),
                         mSelectionCellX,
                         (int)( ( mSelectionYTop + mSelectionHeight ) /
                         mGridSpacingY ) -1 );
  } else {
    emit newEventSignal();
  }
}

void KOAgenda::finishTypeAhead()
{
//  kdDebug() << "KOAgenda::finishTypeAhead()" << endl;
  if ( typeAheadReceiver() ) {
    for( QEvent *e = mTypeAheadEvents.first(); e;
         e = mTypeAheadEvents.next() ) {
//      kdDebug() << "postEvent() " << int( typeAheadReceiver() ) << endl;
      QApplication::postEvent( typeAheadReceiver(), e );
    }
  }
  mTypeAheadEvents.clear();
  mTypeAhead = false;
}

bool KOAgenda::eventFilter_mouse(QObject *object, QMouseEvent *me)
{
  QPoint viewportPos;
  if (object != viewport()) {
    viewportPos = ((QWidget *)object)->mapToParent(me->pos());
  } else {
    viewportPos = me->pos();
  }

  switch (me->type())  {
    case QEvent::MouseButtonPress:
//        kdDebug(5850) << "koagenda: filtered button press" << endl;
      if (object != viewport()) {
        if (me->button() == RightButton) {
          mClickedItem = (KOAgendaItem *)object;
          if (mClickedItem) {
            selectItem(mClickedItem);
            emit showIncidencePopupSignal(mClickedItem->incidence());
          }
    //            mItemPopup->popup(QCursor::pos());
        } else {
          mActionItem = (KOAgendaItem *)object;
          if (mActionItem) {
            selectItem(mActionItem);
            Incidence *incidence = mActionItem->incidence();
            if ( incidence->isReadOnly() || incidence->doesRecur() ) {
              mActionItem = 0;
            } else {
              startItemAction(viewportPos);
            }
          }
        }
      } else {
        if (me->button() == RightButton)
        {
          showNewEventPopupSignal();
        }
        else
        {
          selectItem(0);
          mActionItem = 0;
          setCursor(arrowCursor);
          startSelectAction(viewportPos);
        }
      }
      break;

    case QEvent::MouseButtonRelease:
      if (mActionItem) {
        endItemAction();
      } else if ( mActionType == SELECT ) {
        endSelectAction( viewportPos );
      }
      break;

    case QEvent::MouseMove:
      if (object != viewport()) {
        KOAgendaItem *moveItem = (KOAgendaItem *)object;
        if (!moveItem->incidence()->isReadOnly() &&
            !moveItem->incidence()->recurrence()->doesRecur() )
          if (!mActionItem)
            setNoActionCursor(moveItem,viewportPos);
          else
            performItemAction(viewportPos);
        } else {
          if ( mActionType == SELECT ) {
            performSelectAction( viewportPos );
          }
        }
      break;

    case QEvent::MouseButtonDblClick:
      if (object == viewport()) {
        selectItem(0);
        int x,y;
        viewportToContents(viewportPos.x(),viewportPos.y(),x,y);
        int gx,gy;
        contentsToGrid(x,y,gx,gy);
        emit newEventSignal(gx,gy);
      } else {
        KOAgendaItem *doubleClickedItem = (KOAgendaItem *)object;
        selectItem(doubleClickedItem);
        emit editIncidenceSignal(doubleClickedItem->incidence());
      }
      break;

    default:
      break;
  }

  return true;
}

void KOAgenda::startSelectAction( const QPoint &viewportPos )
{
  emit newStartSelectSignal();

  mActionType = SELECT;
  mSelectionStartPoint = viewportPos;

  int x,y;
  viewportToContents( viewportPos.x(), viewportPos.y(), x, y );
  int gx,gy;
  contentsToGrid( x, y, gx, gy );

  mStartCellX = gx;
  mStartCellY = gy;
  mCurrentCellX = gx;
  mCurrentCellY = gy;

  // Store coordinates of old selection
  int selectionX = mSelectionCellX;
  int selectionYTop = mSelectionYTop;
  int selectionHeight = mSelectionHeight;

  // Store new selection
  mSelectionCellX = gx;
  mSelectionYTop = int( gy * mGridSpacingY );
  mSelectionHeight = int( mGridSpacingY );

  // Clear old selection
  repaintContents( int( selectionX*mGridSpacingX ), selectionYTop,
                   int( (selectionX+1)*mGridSpacingX ) - int( selectionX*mGridSpacingX ), selectionHeight );

  // Paint new selection
  repaintContents( int( mSelectionCellX * mGridSpacingX ), mSelectionYTop,
                   int( (mSelectionCellX+1)*mGridSpacingX ) - int( mSelectionCellX*mGridSpacingX ),
                   mSelectionHeight );

}

void KOAgenda::performSelectAction(const QPoint& viewportPos)
{
  int x,y;
  viewportToContents(viewportPos.x(),viewportPos.y(),x,y);
  int gx,gy;
  contentsToGrid(x,y,gx,gy);

  QPoint clipperPos = clipper()->
                      mapFromGlobal(viewport()->mapToGlobal(viewportPos));

  // Scroll if cursor was moved to upper or lower end of agenda.
  if (clipperPos.y() < mScrollBorderWidth) {
    mScrollUpTimer.start(mScrollDelay);
  } else if (visibleHeight() - clipperPos.y() <
             mScrollBorderWidth) {
    mScrollDownTimer.start(mScrollDelay);
  } else {
    mScrollUpTimer.stop();
    mScrollDownTimer.stop();
  }

  if ( gy != mCurrentCellY && gy >= mStartCellY) {
    int selectionHeight = mSelectionHeight;

    // FIXME: Repaint only the newly (de)selected region
    int x, x1, y;
    gridToContents( mSelectionCellX, 0, x, y );
    gridToContents( mSelectionCellX + 1, gy+1, x1, y );
    mSelectionHeight = y - mSelectionYTop;
    repaintContents( x, mSelectionYTop, x1-x,
        (mSelectionHeight>selectionHeight)?mSelectionHeight:selectionHeight );

    mCurrentCellY = gy;
  }
}

void KOAgenda::endSelectAction( const QPoint &currentPos )
{
  mActionType = NOP;
  mScrollUpTimer.stop();
  mScrollDownTimer.stop();

  emit newTimeSpanSignal(mStartCellX,mStartCellY,mCurrentCellX,mCurrentCellY);

  if ( KOPrefs::instance()->mSelectionStartsEditor ) {
    if ( ( mSelectionStartPoint - currentPos ).manhattanLength() >
         QApplication::startDragDistance() ) {
       emitNewEventForSelection();
    }
  }
}

void KOAgenda::startItemAction(const QPoint& viewportPos)
{
  int x,y;
  viewportToContents(viewportPos.x(),viewportPos.y(),x,y);
  int gx,gy;
  contentsToGrid(x,y,gx,gy);

  mStartCellX = gx;
  mStartCellY = gy;
  mCurrentCellX = gx;
  mCurrentCellY = gy;
  bool noResize = ( mActionItem->incidence()->type() == "Todo");


  if (mAllDayMode) {
    int gridDistanceX = (int)(x - gx * mGridSpacingX);
    if (gridDistanceX < mResizeBorderWidth &&
        mActionItem->cellX() == mCurrentCellX &&
        !noResize ) {
      mActionType = RESIZELEFT;
      setCursor(sizeHorCursor);
    } else if ((mGridSpacingX - gridDistanceX) < mResizeBorderWidth &&
               mActionItem->cellXWidth() == mCurrentCellX &&
               !noResize ) {
      mActionType = RESIZERIGHT;
      setCursor(sizeHorCursor);
    } else {
      mActionType = MOVE;
      mActionItem->startMove();
      setCursor(sizeAllCursor);
    }
  } else {
    int gridDistanceY = (int)(y - gy * mGridSpacingY);
    if (gridDistanceY < mResizeBorderWidth &&
        mActionItem->cellYTop() == mCurrentCellY &&
        !mActionItem->firstMultiItem() &&
        !noResize ) {
      mActionType = RESIZETOP;
      setCursor(sizeVerCursor);
    } else if ((mGridSpacingY - gridDistanceY) < mResizeBorderWidth &&
               mActionItem->cellYBottom() == mCurrentCellY &&
               !mActionItem->lastMultiItem() &&
               !noResize )  {
      mActionType = RESIZEBOTTOM;
      setCursor(sizeVerCursor);
    } else {
      mActionType = MOVE;
      mActionItem->startMove();
      setCursor(sizeAllCursor);
    }
  }
}

void KOAgenda::performItemAction(const QPoint& viewportPos)
{
//  kdDebug(5850) << "viewportPos: " << viewportPos.x() << "," << viewportPos.y() << endl;
//  QPoint point = viewport()->mapToGlobal(viewportPos);
//  kdDebug(5850) << "Global: " << point.x() << "," << point.y() << endl;
//  point = clipper()->mapFromGlobal(point);
//  kdDebug(5850) << "clipper: " << point.x() << "," << point.y() << endl;
//  kdDebug(5850) << "visible height: " << visibleHeight() << endl;
  int x,y;
  viewportToContents(viewportPos.x(),viewportPos.y(),x,y);
//  kdDebug(5850) << "contents: " << x << "," << y << "\n" << endl;
  int gx,gy;
  contentsToGrid(x,y,gx,gy);
  QPoint clipperPos = clipper()->
                      mapFromGlobal(viewport()->mapToGlobal(viewportPos));

  // Cursor left active agenda area.
  // This starts a drag.
  if ( clipperPos.y() < 0 || clipperPos.y() > visibleHeight() ||
       clipperPos.x() < 0 || clipperPos.x() > visibleWidth() ) {
    if ( mActionType == MOVE ) {
kdDebug(5850)<<"Resetting move"<<endl;
      mScrollUpTimer.stop();
      mScrollDownTimer.stop();
      mActionItem->resetMove();
      placeSubCells( mActionItem );
      emit startDragSignal( mActionItem->incidence() );
      setCursor( arrowCursor );
      mActionItem = 0;
      mActionType = NOP;
      mItemMoved = 0;
      return;
    }
  } else {
    switch ( mActionType ) {
      case MOVE:
        setCursor( sizeAllCursor );
        break;
      case RESIZETOP:
      case RESIZEBOTTOM:
        setCursor( sizeVerCursor );
        break;
      case RESIZELEFT:
      case RESIZERIGHT:
        setCursor( sizeHorCursor );
        break;
      default:
        setCursor( arrowCursor );
    }
  }

  // Scroll if item was moved to upper or lower end of agenda.
  if (clipperPos.y() < mScrollBorderWidth) {
    mScrollUpTimer.start(mScrollDelay);
  } else if (visibleHeight() - clipperPos.y() <
             mScrollBorderWidth) {
    mScrollDownTimer.start(mScrollDelay);
  } else {
    mScrollUpTimer.stop();
    mScrollDownTimer.stop();
  }

  // Move or resize item if necessary
  if (mCurrentCellX != gx || mCurrentCellY != gy) {
    mItemMoved = true;
    mActionItem->raise();
    if (mActionType == MOVE) {
      // Move all items belonging to a multi item
      KOAgendaItem *firstItem = mActionItem->firstMultiItem();
      if (!firstItem) firstItem = mActionItem;
      KOAgendaItem *lastItem = mActionItem->lastMultiItem();
      if (!lastItem) lastItem = mActionItem;
      int dy = gy - mCurrentCellY;
      int dx = gx - mCurrentCellX;
      int x,y;
      KOAgendaItem *moveItem = firstItem;
      while (moveItem) {
        bool changed=false;
        if (dx!=0) {
          moveItem->moveRelative( dx, 0 );
          changed=true;
        }
        // in agenda's all day view don't try to move multi items, since there are none
        if ( moveItem==firstItem && !mAllDayMode ) { // is the first item
          int newY = dy+moveItem->cellYTop();
          // If event start moved earlier than 0:00, it starts the previous day
          if (newY<0) {
            moveItem->expandTop( -moveItem->cellYTop() );
            // prepend a new item at ( x-1, rows()+newY to rows() )
            KOAgendaItem *newFirst = firstItem->prevMoveItem();
            // cell's y values are first and last cell of the bar, so if newY=-1, they need to be the same
            if (newFirst) {
              newFirst->setCellXY(moveItem->cellX()-1, rows()+newY, rows()-1);
              mItems.append(newFirst);
              moveItem->resize( (int)( mGridSpacingX * newFirst->cellWidth() ),
                                (int)( mGridSpacingY * newFirst->cellHeight() ));
              gridToContents(newFirst->cellX(), newFirst->cellYTop(),x,y);
              addChild( newFirst, x, y );
            } else {
              newFirst = insertItem( moveItem->incidence(), moveItem->itemDate(),
                moveItem->cellX()-1, rows()+newY, rows()-1 ) ;
            }
            if (newFirst) newFirst->show();
            moveItem->prependMoveItem(newFirst);
            firstItem=newFirst;
          } else if ( newY>=rows() ) {
            // If event start is moved past 24:00, it starts the next day
            // erase current item (i.e. remove it from the multiItem list)
            firstItem = moveItem->nextMultiItem();
            moveItem->hide();
            mItems.take(mItems.find(moveItem));
            removeChild( moveItem );
            mActionItem->removeMoveItem(moveItem);
            moveItem=firstItem;
            // adjust next day's item
            if (moveItem) moveItem->expandTop( rows()-newY );
          } else {
            moveItem->expandTop(dy);
          }
          changed=true;
        }
        if ( !moveItem->lastMultiItem() && !mAllDayMode ) { // is the last item
          int newY = dy+moveItem->cellYBottom();
          if (newY<0) {
            // erase current item
            lastItem = moveItem->prevMultiItem();
            moveItem->hide();
            mItems.take( mItems.find(moveItem) );
            removeChild( moveItem );
            moveItem->removeMoveItem( moveItem );
            moveItem = lastItem;
            moveItem->expandBottom(newY+1);
          } else if (newY>=rows()) {
            moveItem->expandBottom( rows()-moveItem->cellYBottom()-1 );
            // append item at ( x+1, 0 to newY-rows() )
            KOAgendaItem *newLast = lastItem->nextMoveItem();
            if (newLast) {
              newLast->setCellXY( moveItem->cellX()+1, 0, newY-rows()-1 );
              mItems.append(newLast);
              moveItem->resize( (int)( mGridSpacingX * newLast->cellWidth() ),
                                (int)( mGridSpacingY * newLast->cellHeight() ));
              gridToContents( newLast->cellX(), newLast->cellYTop(), x, y) ;
              addChild( newLast, x, y );
            } else {
              newLast = insertItem( moveItem->incidence(), moveItem->itemDate(),
                moveItem->cellX()+1, 0, newY-rows()-1 ) ;
            }
            moveItem->appendMoveItem( newLast );
            newLast->show();
            lastItem = newLast;
          } else {
            moveItem->expandBottom( dy );
          }
          changed=true;
        }
        if (changed) {
          moveItem->resize((int)( mGridSpacingX * moveItem->cellWidth() ),
                         (int)( mGridSpacingY * moveItem->cellHeight() ));
          gridToContents(moveItem->cellX(),moveItem->cellYTop(),x,y);
          moveChild(moveItem,x,y);
        }
        moveItem = moveItem->nextMultiItem();
      }
    } else if (mActionType == RESIZETOP) {
      if (mCurrentCellY <= mActionItem->cellYBottom()) {
        mActionItem->expandTop(gy - mCurrentCellY);
        mActionItem->resize(mActionItem->width(),
                            (int)( mGridSpacingY * mActionItem->cellHeight() ));
        int x,y;
        gridToContents(mCurrentCellX,mActionItem->cellYTop(),x,y);
        moveChild(mActionItem,childX(mActionItem),y);
      }
    } else if (mActionType == RESIZEBOTTOM) {
      if (mCurrentCellY >= mActionItem->cellYTop()) {
        mActionItem->expandBottom(gy - mCurrentCellY);
        mActionItem->resize(mActionItem->width(),
                            (int)( mGridSpacingY * mActionItem->cellHeight() ));
      }
    } else if (mActionType == RESIZELEFT) {
       if (mCurrentCellX <= mActionItem->cellXWidth()) {
         mActionItem->expandLeft(gx - mCurrentCellX);
         mActionItem->resize((int)(mGridSpacingX * mActionItem->cellWidth()),
                             mActionItem->height());
        int x,y;
        gridToContents(mActionItem->cellX(),mActionItem->cellYTop(),x,y);
        moveChild(mActionItem,x,childY(mActionItem));
       }
    } else if (mActionType == RESIZERIGHT) {
       if (mCurrentCellX >= mActionItem->cellX()) {
         mActionItem->expandRight(gx - mCurrentCellX);
         mActionItem->resize((int)(mGridSpacingX * mActionItem->cellWidth()),
                             mActionItem->height());
       }
    }
    mCurrentCellX = gx;
    mCurrentCellY = gy;
  }
}

void KOAgenda::endItemAction()
{
//  kdDebug(5850) << "KOAgenda::endItemAction()" << endl;

  if ( mItemMoved ) {
    if ( mActionType == MOVE ) {
      mActionItem->endMove();
    }
    KOAgendaItem *placeItem = mActionItem->firstMultiItem();
    if ( !placeItem ) {
      placeItem = mActionItem;
    }
    emit itemModified( placeItem );
    QPtrList<KOAgendaItem> oldconflictItems = placeItem->conflictItems();
    KOAgendaItem *item;
    for ( item=oldconflictItems.first(); item != 0;
          item=oldconflictItems.next() ) {
      placeSubCells(item);
    }
    while ( placeItem ) {
      placeSubCells( placeItem );
      placeItem = placeItem->nextMultiItem();
    }
  }

  mScrollUpTimer.stop();
  mScrollDownTimer.stop();
  setCursor( arrowCursor );
  mActionItem = 0;
  mActionType = NOP;
  mItemMoved = 0;

//  kdDebug(5850) << "KOAgenda::endItemAction() done" << endl;
}

void KOAgenda::setNoActionCursor( KOAgendaItem *moveItem, const QPoint& viewportPos )
{
//  kdDebug(5850) << "viewportPos: " << viewportPos.x() << "," << viewportPos.y() << endl;
//  QPoint point = viewport()->mapToGlobal(viewportPos);
//  kdDebug(5850) << "Global: " << point.x() << "," << point.y() << endl;
//  point = clipper()->mapFromGlobal(point);
//  kdDebug(5850) << "clipper: " << point.x() << "," << point.y() << endl;

  int x,y;
  viewportToContents(viewportPos.x(),viewportPos.y(),x,y);
//  kdDebug(5850) << "contents: " << x << "," << y << endl  << endl;
  int gx,gy;
  contentsToGrid(x,y,gx,gy);
  bool noResize = (moveItem && moveItem->incidence() &&
      moveItem->incidence()->type() == "Todo");

  // Change cursor to resize cursor if appropriate
  if (mAllDayMode) {
    int gridDistanceX = (int)(x - gx * mGridSpacingX);
    if ( !noResize &&
         gridDistanceX < mResizeBorderWidth &&
         moveItem->cellX() == gx ) {
      setCursor(sizeHorCursor);
    } else if ( !noResize &&
                (mGridSpacingX - gridDistanceX) < mResizeBorderWidth &&
                moveItem->cellXWidth() == gx ) {
      setCursor(sizeHorCursor);
    } else {
      setCursor(arrowCursor);
    }
  } else {
    int gridDistanceY = (int)(y - gy * mGridSpacingY);
    if ( !noResize &&
         gridDistanceY < mResizeBorderWidth &&
         moveItem->cellYTop() == gy &&
         !moveItem->firstMultiItem() ) {
      setCursor(sizeVerCursor);
    } else if ( !noResize &&
                (mGridSpacingY - gridDistanceY) < mResizeBorderWidth &&
                moveItem->cellYBottom() == gy &&
                !moveItem->lastMultiItem()) {
      setCursor(sizeVerCursor);
    } else {
      setCursor(arrowCursor);
    }
  }
}


/** calculate the width of the column subcells of the given item
*/
double KOAgenda::calcSubCellWidth( KOAgendaItem *item )
{
  int x, y, x1, y1;
  gridToContents( item->cellX(), item->cellYTop(), x, y );
  gridToContents( item->cellX()+1, item->cellYTop()+1, x1, y1 );
  int maxSubCells = item->subCells();
  double newSubCellWidth;
  if ( mAllDayMode ) {
    newSubCellWidth = double( y1 - y ) / maxSubCells;
  } else {
    newSubCellWidth = double( x1 - x ) / maxSubCells;
  }
  return newSubCellWidth;
}

void KOAgenda::placeAgendaItem( KOAgendaItem *item, double subCellWidth )
{
  int x, y, x1, y1;
  // left upper corner, no subcells yet
  // right lower corner
  gridToContents( item->cellX(), item->cellYTop(), x, y );
  gridToContents( item->cellX() + item->cellWidth(),
                  item->cellYBottom()+1, x1, y1 );

  double subCellPos = item->subCell() * subCellWidth;

  // we need to add 0.01 to make sure we don't loose one pixed due to
  // numerics (i.e. if it would be x.9998, we want the integer, not rounded down.
  if (mAllDayMode) {
    item->resize( x1-x, int( subCellPos + subCellWidth + 0.01 ) - int( subCellPos ) );
    y += int( subCellPos );
  } else {
    item->resize( int( subCellPos + subCellWidth + 0.01 ) - int( subCellPos ), y1-y );
    x += int( subCellPos );
  }
  moveChild( item, x, y );
}

/*
  Place item in cell and take care that multiple items using the same cell do
  not overlap. This method is not yet optimal. It doesn't use the maximum space
  it can get in all cases.
  At the moment the method has a bug: When an item is placed only the sub cell
  widths of the items are changed, which are within the Y region the item to
  place spans. When the sub cell width change of one of this items affects a
  cell, where other items are, which do not overlap in Y with the item to place,
  the display gets corrupted, although the corruption looks quite nice.
*/
void KOAgenda::placeSubCells( KOAgendaItem *placeItem )
{
#if 0
  kdDebug(5850) << "KOAgenda::placeSubCells()" << endl;
  if ( placeItem ) {
    Event *event = placeItem->itemEvent();
    if ( !event ) {
      kdDebug(5850) << "  event is 0" << endl;
    } else {
      kdDebug(5850) << "  event: " << event->summary() << endl;
    }
  } else {
    kdDebug(5850) << "  placeItem is 0" << endl;
  }
  kdDebug(5850) << "KOAgenda::placeSubCells()..." << endl;
#endif

  QPtrList<KOAgendaItem> conflictItems;
  int maxSubCells = 0;
  QIntDict<KOAgendaItem> subCellDict( 5 );

  // Find all items which are in the same cell.
  KOAgendaItem *item;
  for ( item=mItems.first(); item != 0; item = mItems.next() ) {
    if ( item != placeItem ) {
      if ( placeItem->cellX() <= item->cellXWidth() &&
           placeItem->cellXWidth() >= item->cellX() ) {
        if ( ( placeItem->cellYTop() <= item->cellYBottom() ) &&
             ( placeItem->cellYBottom() >= item->cellYTop() ) ) {
          conflictItems.append( item );
          if ( item->subCells() > maxSubCells )
            maxSubCells = item->subCells();
          subCellDict.insert( item->subCell(), item );
        }
      }
    }
  }

  if ( conflictItems.count() > 0 ) {
    // Look for unused sub cell and insert item
    int i;
    for( i = 0; i < maxSubCells; ++i ) {
      if ( !subCellDict.find( i ) ) {
        placeItem->setSubCell( i );
        break;
      }
    }
    if ( i == maxSubCells ) {
      placeItem->setSubCell( maxSubCells );
      maxSubCells++;  // add new item to number of sub cells
    }

    conflictItems.append( placeItem );

//    kdDebug(5850) << "---Conflict items: " << conflictItems.count() << endl;

    // Adjust sub cell geometry of all items
    placeItem->setSubCells( maxSubCells );
    double newSubCellWidth = calcSubCellWidth( placeItem );
    for ( item = conflictItems.first(); item != 0; item = conflictItems.next() ) {
      item->setSubCells( maxSubCells );
      placeAgendaItem( item, newSubCellWidth );
    }
  } else {
    // There is only one item in this cell
    placeItem->setSubCell( 0 );
    placeItem->setSubCells( 1 );
    double newSubCellWidth = calcSubCellWidth( placeItem );
    placeAgendaItem( placeItem, newSubCellWidth );
  }
  placeItem->setConflictItems( conflictItems );
  placeItem->update();
}

/*
  Draw grid in the background of the agenda.
*/
void KOAgenda::drawContents(QPainter* p, int cx, int cy, int cw, int ch)
{

  QPixmap db(cw, ch);
  db.fill(KOPrefs::instance()->mAgendaBgColor);
  QPainter dbp(&db);
  dbp.translate(-cx,-cy);

//  kdDebug(5850) << "KOAgenda::drawContents()" << endl;
  double lGridSpacingY = mGridSpacingY*2;

  // Highlight working hours
  if (mWorkingHoursEnable) {
    int x1 = cx;
    int y1 = mWorkingHoursYTop;
    if (y1 < cy) y1 = cy;
    int x2 = cx+cw-1;
    //  int x2 = mGridSpacingX * 5 - 1;
    //  if (x2 > cx+cw-1) x2 = cx + cw - 1;
    int y2 = mWorkingHoursYBottom;
    if (y2 > cy+ch-1) y2=cy+ch-1;

    if (x2 >= x1 && y2 >= y1) {
      int gxStart = (int)(x1/mGridSpacingX);
      int gxEnd = (int)(x2/mGridSpacingX);
      while(gxStart <= gxEnd) {
        if (gxStart < int(mHolidayMask->count()) &&
            !mHolidayMask->at(gxStart)) {
          int xStart = (int)( KOGlobals::self()->reverseLayout() ?
                                    (mColumns - 1 - gxStart)*mGridSpacingX :
                              gxStart*mGridSpacingX );
          if (xStart < x1) xStart = x1;
          int xEnd = (int)( KOGlobals::self()->reverseLayout() ?
                                    (mColumns - gxStart)*mGridSpacingX-1 :
                            (gxStart+1)*mGridSpacingX-1 );
          if (xEnd > x2) xEnd = x2;
          dbp.fillRect(xStart,y1,xEnd-xStart+1,y2-y1+1,
                      KOPrefs::instance()->mWorkingHoursColor);
        }
        ++gxStart;
      }
    }
  }

  int selectionX, selectionX1, selectionY;
  gridToContents( mSelectionCellX, 0, selectionX, selectionY );
  gridToContents( mSelectionCellX+1, 0, selectionX1, selectionY );

  // Draw selection
  if ( ( cx + cw ) >= selectionX && cx <= ( selectionX1 ) &&
       ( cy + ch ) >= mSelectionYTop && cy <= ( mSelectionYTop + mSelectionHeight ) ) {
    // TODO: paint only part within cx,cy,cw,ch
    dbp.fillRect( selectionX, mSelectionYTop, selectionX1-selectionX,
                 mSelectionHeight, KOPrefs::instance()->mHighlightColor );
  }

  dbp.setPen( KOPrefs::instance()->mAgendaBgColor.dark(150) );

  // Draw vertical lines of grid, start with the last line not yet visible
  //  kdDebug(5850) << "drawContents cx: " << cx << " cy: " << cy << " cw: " << cw << " ch: " << ch << endl;
  double x = ((int)(cx/mGridSpacingX))*mGridSpacingX;
  while (x < cx + cw) {
    dbp.drawLine((int)x,cy,(int)x,cy+ch);
    x+=mGridSpacingX;
  }

  // Draw horizontal lines of grid
  double y = ((int)(cy/lGridSpacingY))*lGridSpacingY;
  while (y < cy + ch) {
//    kdDebug(5850) << " y: " << y << endl;
    dbp.drawLine(cx,(int)y,cx+cw,(int)y);
    y+=lGridSpacingY;
  }
  p->drawPixmap(cx,cy, db);
}

/*
  Convert srcollview contents coordinates to agenda grid coordinates.
*/
void KOAgenda::contentsToGrid (int x, int y, int& gx, int& gy)
{
  gx = (int)( KOGlobals::self()->reverseLayout() ?
        mColumns - 1 - x/mGridSpacingX : x/mGridSpacingX );
  gy = (int)( y/mGridSpacingY );
}

/*
  Convert agenda grid coordinates to scrollview contents coordinates.
*/
void KOAgenda::gridToContents (int gx, int gy, int& x, int& y)
{
  x = (int)( KOGlobals::self()->reverseLayout() ?
             (mColumns - 1 - gx)*mGridSpacingX : gx*mGridSpacingX );
  y = (int)( gy*mGridSpacingY );
}


/*
  Return Y coordinate corresponding to time. Coordinates are rounded to fit into
  the grid.
*/
int KOAgenda::timeToY(const QTime &time)
{
//  kdDebug(5850) << "Time: " << time.toString() << endl;
  int minutesPerCell = 24 * 60 / mRows;
//  kdDebug(5850) << "minutesPerCell: " << minutesPerCell << endl;
  int timeMinutes = time.hour() * 60 + time.minute();
//  kdDebug(5850) << "timeMinutes: " << timeMinutes << endl;
  int Y = (timeMinutes + (minutesPerCell / 2)) / minutesPerCell;
//  kdDebug(5850) << "y: " << Y << endl;
//  kdDebug(5850) << "\n" << endl;
  return Y;
}


/*
  Return time corresponding to cell y coordinate. Coordinates are rounded to
  fit into the grid.
*/
QTime KOAgenda::gyToTime(int gy)
{
//  kdDebug(5850) << "gyToTime: " << gy << endl;
  int secondsPerCell = 24 * 60 * 60/ mRows;

  int timeSeconds = secondsPerCell * gy;

  QTime time( 0, 0, 0 );
  if ( timeSeconds < 24 * 60 * 60 ) {
    time = time.addSecs(timeSeconds);
  } else {
    time.setHMS( 23, 59, 59 );
  }
//  kdDebug(5850) << "  gyToTime: " << time.toString() << endl;

  return time;
}

void KOAgenda::setStartHour(int startHour)
{
  int startCell = startHour * mRows / 24;
  setContentsPos(0, (int)(startCell * gridSpacingY()));
}


/*
  Insert KOAgendaItem into agenda.
*/
KOAgendaItem *KOAgenda::insertItem (Incidence *event,QDate qd,int X,int YTop,int YBottom)
{
  //kdDebug(5850) << "KOAgenda::insertItem:" << event->summary() << "-" << qd.toString() << " ;top, bottom:" << YTop << "," << YBottom << endl;

  if (mAllDayMode) {
    kdDebug(5850) << "KOAgenda: calling insertItem in all-day mode is illegal." << endl;
    return 0;
  }

  KOAgendaItem *agendaItem = new KOAgendaItem (event,qd,viewport());

  int YSize = YBottom - YTop + 1;
  if (YSize < 0) {
    kdDebug(5850) << "KOAgenda::insertItem(): Text: " << agendaItem->text() << " YSize<0" << endl;
    YSize = 1;
  }

  agendaItem->resize((int)mGridSpacingX, (int)( mGridSpacingY * YSize ));
  agendaItem->setCellXY(X,YTop,YBottom);
  agendaItem->setCellXWidth(X);

  agendaItem->installEventFilter(this);

  addChild(agendaItem,(int)( X*mGridSpacingX ), (int)( YTop*mGridSpacingY ));
  mItems.append(agendaItem);

  placeSubCells(agendaItem);

  agendaItem->show();

  marcus_bains();

  return agendaItem;
}


/*
  Insert all-day KOAgendaItem into agenda.
*/
KOAgendaItem *KOAgenda::insertAllDayItem (Incidence *event,QDate qd,int XBegin,int XEnd)
{
   if (!mAllDayMode) {
    kdDebug(5850) << "KOAgenda: calling insertAllDayItem in non all-day mode is illegal." << endl;
    return 0;
  }

  KOAgendaItem *agendaItem = new KOAgendaItem (event,qd,viewport());

  agendaItem->setCellXY(XBegin,0,0);
  agendaItem->setCellXWidth(XEnd);

  double startIt = mGridSpacingX * (agendaItem->cellX());
  double endIt = mGridSpacingX * (agendaItem->cellWidth()+agendaItem->cellX());

  agendaItem->resize( int(endIt) - int(startIt), int(mGridSpacingY));

  agendaItem->installEventFilter(this);

  addChild(agendaItem,(int)( XBegin*mGridSpacingX ), 0);
  mItems.append(agendaItem);

  placeSubCells(agendaItem);

  agendaItem->show();

  return agendaItem;
}


void KOAgenda::insertMultiItem (Event *event,QDate qd,int XBegin,int XEnd,
                                int YTop,int YBottom)
{
  if (mAllDayMode) {
    kdDebug(5850) << "KOAgenda: calling insertMultiItem in all-day mode is illegal." << endl;
    return;
  }

  int cellX,cellYTop,cellYBottom;
  QString newtext;
  int width = XEnd - XBegin + 1;
  int count = 0;
  KOAgendaItem *current = 0;
  QPtrList<KOAgendaItem> multiItems;
  for (cellX = XBegin;cellX <= XEnd;++cellX) {
    if (cellX == XBegin) cellYTop = YTop;
    else cellYTop = 0;
    if (cellX == XEnd) cellYBottom = YBottom;
    else cellYBottom = rows() - 1;
    newtext = QString("(%1/%2): ").arg(++count).arg(width);
    newtext.append(event->summary());
    current = insertItem(event,qd,cellX,cellYTop,cellYBottom);
    current->setText(newtext);
    multiItems.append(current);
  }

  KOAgendaItem *next = 0;
  KOAgendaItem *prev = 0;
  KOAgendaItem *last = multiItems.last();
  KOAgendaItem *first = multiItems.first();
  KOAgendaItem *setFirst,*setLast;
  current = first;
  while (current) {
    next = multiItems.next();
    if (current == first) setFirst = 0;
    else setFirst = first;
    if (current == last) setLast = 0;
    else setLast = last;

    current->setMultiItem(setFirst, prev, next, setLast);
    prev=current;
    current = next;
  }

  marcus_bains();
}


//QSizePolicy KOAgenda::sizePolicy() const
//{
  // Thought this would make the all-day event agenda minimum size and the
  // normal agenda take the remaining space. But it doesn´t work. The QSplitter
  // don´t seem to think that an Expanding widget needs more space than a
  // Preferred one.
  // But it doesn´t hurt, so it stays.
//  if (mAllDayMode) {
//    return QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
//  } else {
//    return QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
//  }
//}


/*
  Overridden from QScrollView to provide proper resizing of KOAgendaItems.
*/
void KOAgenda::resizeEvent ( QResizeEvent *ev )
{
//  kdDebug(5850) << "KOAgenda::resizeEvent" << endl;
  double subCellWidth;
  KOAgendaItem *item;
  if (mAllDayMode) {
    mGridSpacingX = double( width() - 2 * frameWidth() ) / (double)mColumns;
//    kdDebug(5850) << "Frame " << frameWidth() << endl;
    mGridSpacingY = height() - 2 * frameWidth();
    resizeContents( (int)( mGridSpacingX * mColumns ), (int)(mGridSpacingY ));

    for ( item=mItems.first(); item != 0; item=mItems.next() ) {
      subCellWidth = calcSubCellWidth( item );
      placeAgendaItem( item, subCellWidth );
    }
  } else {
    mGridSpacingX = double(width() - verticalScrollBar()->width() - 2 * frameWidth()) / double(mColumns);
    // make sure that there are not more than 24 per day
    mGridSpacingY = double(height() - 2 * frameWidth()) / double(mRows);
    if ( mGridSpacingY < mDesiredGridSpacingY ) mGridSpacingY = mDesiredGridSpacingY;

    resizeContents( int( mGridSpacingX * mColumns ), int( mGridSpacingY * mRows ));

    for ( item=mItems.first(); item != 0; item=mItems.next() ) {
      subCellWidth = calcSubCellWidth( item );
      placeAgendaItem( item, subCellWidth );
    }
  }

  checkScrollBoundaries();
  calculateWorkingHours();

  marcus_bains();

  QScrollView::resizeEvent(ev);
  viewport()->update();
}


void KOAgenda::scrollUp()
{
  scrollBy(0,-mScrollOffset);
}


void KOAgenda::scrollDown()
{
  scrollBy(0,mScrollOffset);
}

void KOAgenda::popupAlarm()
{
  if (!mClickedItem) {
    kdDebug(5850) << "KOAgenda::popupAlarm() called without having a clicked item" << endl;
    return;
  }
// TODO: deal correctly with multiple alarms
  Alarm::List alarms = mClickedItem->incidence()->alarms();
  Alarm::List::ConstIterator it;
  for( it = alarms.begin(); it != alarms.end(); ++it )
    (*it)->toggleAlarm();
  if (alarms.isEmpty()) {
    // Add an alarm if it didn't have one
    Alarm*alm = mClickedItem->incidence()->newAlarm();
    alm->setEnabled(true);
  }

  mClickedItem->updateIcons();
}

/*
  Calculates the minimum width
*/
int KOAgenda::minimumWidth() const
{
  // TODO:: develop a way to dynamically determine the minimum width
  int min = 100;

  return min;
}

void KOAgenda::updateConfig()
{
  mDesiredGridSpacingY = KOPrefs::instance()->mHourSize;
 // make sure that there are not more than 24 per day
  mGridSpacingY = (double)height()/(double)mRows;
  if (mGridSpacingY<mDesiredGridSpacingY) mGridSpacingY=mDesiredGridSpacingY;

  calculateWorkingHours();

  marcus_bains();
}

void KOAgenda::checkScrollBoundaries()
{
  // Invalidate old values to force update
  mOldLowerScrollValue = -1;
  mOldUpperScrollValue = -1;

  checkScrollBoundaries(verticalScrollBar()->value());
}

void KOAgenda::checkScrollBoundaries(int v)
{
  int yMin = (int)(v/mGridSpacingY);
  int yMax = (int)((v+visibleHeight())/mGridSpacingY);

//  kdDebug(5850) << "--- yMin: " << yMin << "  yMax: " << yMax << endl;

  if (yMin != mOldLowerScrollValue) {
    mOldLowerScrollValue = yMin;
    emit lowerYChanged(yMin);
  }
  if (yMax != mOldUpperScrollValue) {
    mOldUpperScrollValue = yMax;
    emit upperYChanged(yMax);
  }
}

void KOAgenda::deselectItem()
{
  if (mSelectedItem.isNull()) return;
  mSelectedItem->select(false);
  mSelectedItem = 0;
}

void KOAgenda::selectItem(KOAgendaItem *item)
{
  if ((KOAgendaItem *)mSelectedItem == item) return;
  deselectItem();
  if (item == 0) {
    emit incidenceSelected( 0 );
    return;
  }
  mSelectedItem = item;
  mSelectedItem->select();
  emit incidenceSelected( mSelectedItem->incidence() );
}

// This function seems never be called.
void KOAgenda::keyPressEvent( QKeyEvent *kev )
{
  switch(kev->key()) {
    case Key_PageDown:
      verticalScrollBar()->addPage();
      break;
    case Key_PageUp:
      verticalScrollBar()->subtractPage();
      break;
    case Key_Down:
      verticalScrollBar()->addLine();
      break;
    case Key_Up:
      verticalScrollBar()->subtractLine();
      break;
    default:
      ;
  }
}

void KOAgenda::calculateWorkingHours()
{
//  mWorkingHoursEnable = KOPrefs::instance()->mEnableWorkingHours;
  mWorkingHoursEnable = !mAllDayMode;

  mWorkingHoursYTop = (int)(mGridSpacingY *
                      KOPrefs::instance()->mWorkingHoursStart * 4);
  mWorkingHoursYBottom = (int)(mGridSpacingY *
                         KOPrefs::instance()->mWorkingHoursEnd * 4 - 1);
}


DateList KOAgenda::dateList() const
{
    return mSelectedDates;
}

void KOAgenda::setDateList(const DateList &selectedDates)
{
    mSelectedDates = selectedDates;
    marcus_bains();
}

void KOAgenda::setHolidayMask(QMemArray<bool> *mask)
{
  mHolidayMask = mask;

/*
  kdDebug(5850) << "HolidayMask: ";
  for(uint i=0;i<mask->count();++i) {
    kdDebug(5850) << (mask->at(i) ? "*" : "o");
  }
  kdDebug(5850) << endl;
*/
}

void KOAgenda::contentsMousePressEvent ( QMouseEvent *event )
{
  kdDebug(5850) << "KOagenda::contentsMousePressEvent(): type: " << event->type() << endl;
  QScrollView::contentsMousePressEvent(event);
}

void KOAgenda::setTypeAheadReceiver( QObject *o )
{
  mTypeAheadReceiver = o;
}

QObject *KOAgenda::typeAheadReceiver() const
{
  return mTypeAheadReceiver;
}
