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
#include <assert.h>

#include <qintdict.h>
#include <qdatetime.h>
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qlabel.h>

#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kmessagebox.h>

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
#include <libkcal/calendar.h>

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
  int y = int( mins * agenda->gridSpacingY() / minutesPerCell );
  int today = recalculate ? todayColumn() : oldToday;
  int x = int( agenda->gridSpacingX() * today );
  bool disabled = !(KOPrefs::instance()->mMarcusBainsEnabled);

  oldTime = tim;
  oldToday = today;

  if(disabled || (today<0)) {
    hide();
    mTimeBox->hide();
    return;
  } else {
    show();
    mTimeBox->show();
  }

  if ( recalculate ) setFixedSize( int( agenda->gridSpacingX() ), 1 );
  agenda->moveChild( this, x, y );
  raise();

  if(recalculate)
    mTimeBox->setFont(KOPrefs::instance()->mMarcusBainsFont);

  mTimeBox->setText(KGlobal::locale()->formatTime(tim, KOPrefs::instance()->mMarcusBainsShowSeconds));
  mTimeBox->adjustSize();
  if (y-mTimeBox->height()>=0) y-=mTimeBox->height(); else y++;
  if (x-mTimeBox->width()+agenda->gridSpacingX() > 0)
    x += int( agenda->gridSpacingX() - mTimeBox->width() - 1 );
  else x++;
  agenda->moveChild(mTimeBox,x,y);
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

const QString KOAgenda::lastSelectedUid() const
{
  return mSelectedUid;
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

  mStartCell = QPoint( 0, 0 );
  mEndCell = QPoint( 0, 0 );

  mHasSelection = false;
  mSelectionStartPoint = QPoint( 0, 0 );
  mSelectionStartCell = QPoint( 0, 0 );
  mSelectionEndCell = QPoint( 0, 0 );

  mOldLowerScrollValue = -1;
  mOldUpperScrollValue = -1;

  mClickedItem = 0;

  mActionItem = 0;
  mActionType = NOP;
  mItemMoved = false;

  mSelectedItem = 0;
  mSelectedUid = QString::null;

  setAcceptDrops( true );
  installEventFilter( this );
  mItems.setAutoDelete( true );
  mItemsToDelete.setAutoDelete( true );

//  resizeContents( int(mGridSpacingX * mColumns + 1) , int(mGridSpacingY * mRows + 1) );
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

  setStartTime( KOPrefs::instance()->mDayBegins.time() );

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
  for ( item = mItems.first(); item != 0; item = mItems.next() ) {
    removeChild( item );
  }
  mItems.clear();
  mItemsToDelete.clear();

  mSelectedItem = 0;

  clearSelection();
}


void KOAgenda::clearSelection()
{
  mHasSelection = false;
  mActionType = NOP;
  updateContents();
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

bool KOAgenda::eventFilter_drag( QObject *object, QDropEvent *de )
{
#ifndef KORG_NODND
  QPoint viewportPos;
  if ( object != viewport() && object != this ) {
    viewportPos = static_cast<QWidget *>( object )->mapToParent( de->pos() );
  } else {
    viewportPos = de->pos();
  }

  switch ( de->type() ) {
    case QEvent::DragEnter:
    case QEvent::DragMove:
      if ( ICalDrag::canDecode( de ) || VCalDrag::canDecode( de ) ) {

        DndFactory factory( mCalendar );
        Todo *todo = factory.createDropTodo( de );
        if ( todo ) {
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
          QPoint pos;
          // FIXME: This is a bad hack, as the viewportToContents seems to be off by
          // 2000 (which is the left upper corner of the viewport). It works correctly
          // for agendaItems.
          if ( object == this  ) {
            pos = viewportPos + QPoint( contentsX(), contentsY() );
          } else {
            pos = viewportToContents( viewportPos );
          }
          QPoint gpos = contentsToGrid( pos );
          emit droppedToDo( todo, gpos, mAllDayMode );
          return true;
        }
      }
      break;

    case QEvent::DragResponse:
    default:
      break;
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
        }
        return true;
    }
  }
  return false;
}

void KOAgenda::emitNewEventForSelection()
{
  emit newEventSignal();
}

void KOAgenda::finishTypeAhead()
{
//  kdDebug() << "KOAgenda::finishTypeAhead()" << endl;
  if ( typeAheadReceiver() ) {
    for( QEvent *e = mTypeAheadEvents.first(); e;
         e = mTypeAheadEvents.next() ) {
//      kdDebug() << "sendEvent() " << int( typeAheadReceiver() ) << endl;
      QApplication::sendEvent( typeAheadReceiver(), e );
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
          mClickedItem = dynamic_cast<KOAgendaItem *>(object);
          if (mClickedItem) {
            selectItem(mClickedItem);
            emit showIncidencePopupSignal( mClickedItem->incidence(),
                                           mClickedItem->itemDate() );
          }
        } else {
          mActionItem = dynamic_cast<KOAgendaItem *>(object);
          if (mActionItem) {
            selectItem(mActionItem);
            Incidence *incidence = mActionItem->incidence();
// OLD_RK:            if ( incidence->isReadOnly() || incidence->doesRecur() ) {
            if ( incidence->isReadOnly() ) {
              mActionItem = 0;
            } else {
              startItemAction(viewportPos);
            }
          }
        }
      } else {
        if (me->button() == RightButton)
        {
          // if mouse pointer is not in selection, select the cell below the cursor
          QPoint gpos = contentsToGrid( viewportToContents( viewportPos ) );
          if ( !ptInSelection( gpos ) ) {
            mSelectionStartCell = gpos;
            mSelectionEndCell = gpos;
            mHasSelection = true;
            emit newStartSelectSignal();
            emit newTimeSpanSignal( mSelectionStartCell, mSelectionEndCell );
            updateContents();
          }
          showNewEventPopupSignal();
        }
        else
        {
          // if mouse pointer is in selection, don't change selection
          QPoint gpos = contentsToGrid( viewportToContents( viewportPos ) );
          if ( !ptInSelection( gpos ) ) {
            selectItem(0);
            mActionItem = 0;
            setCursor(arrowCursor);
            startSelectAction(viewportPos);
          }
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
        KOAgendaItem *moveItem = dynamic_cast<KOAgendaItem *>(object);
// OLD_RK:        if (moveItem && !moveItem->incidence()->isReadOnly() &&
//            !moveItem->incidence()->doesRecur() )
        if (moveItem && !moveItem->incidence()->isReadOnly() )
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
        emit newEventSignal();
      } else {
        KOAgendaItem *doubleClickedItem = dynamic_cast<KOAgendaItem *>(object);
        if (doubleClickedItem) {
          selectItem(doubleClickedItem);
          emit editIncidenceSignal(doubleClickedItem->incidence());
        }
      }
      break;

    default:
      break;
  }

  return true;
}

bool KOAgenda::ptInSelection( QPoint gpos ) const
{
  if ( !mHasSelection ) {
    return false;
  } else if ( gpos.x()<mSelectionStartCell.x() || gpos.x()>mSelectionEndCell.x() ) {
    return false;
  } else if ( (gpos.x()==mSelectionStartCell.x()) && (gpos.y()<mSelectionStartCell.y()) ) {
    return false;
  } else if ( (gpos.x()==mSelectionEndCell.x()) && (gpos.y()>mSelectionEndCell.y()) ) {
    return false;
  }
  return true;
}

void KOAgenda::startSelectAction( const QPoint &viewportPos )
{
  emit newStartSelectSignal();

  mActionType = SELECT;
  mSelectionStartPoint = viewportPos;
  mHasSelection = true;

  QPoint pos = viewportToContents( viewportPos );
  QPoint gpos = contentsToGrid( pos );

  // Store new selection
  mStartCell = gpos;
  mEndCell = gpos;
  mSelectionStartCell = gpos;
  mSelectionEndCell = gpos;

  updateContents();
}

void KOAgenda::performSelectAction(const QPoint& viewportPos)
{
  QPoint pos = viewportToContents( viewportPos );
  QPoint gpos = contentsToGrid( pos );

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

  if ( gpos != mEndCell ) {
    mEndCell = gpos;
    if ( mStartCell.x()>mEndCell.x() ||
         ( mStartCell.x()==mEndCell.x() && mStartCell.y()>mEndCell.y() ) ) {
      // backward selection
      mSelectionStartCell = mEndCell;
      mSelectionEndCell = mStartCell;
    } else {
      mSelectionStartCell = mStartCell;
      mSelectionEndCell = mEndCell;
    }

    updateContents();
  }
}

void KOAgenda::endSelectAction( const QPoint &currentPos )
{
  mScrollUpTimer.stop();
  mScrollDownTimer.stop();

  emit newTimeSpanSignal( mSelectionStartCell, mSelectionEndCell );

  if ( KOPrefs::instance()->mSelectionStartsEditor ) {
    if ( ( mSelectionStartPoint - currentPos ).manhattanLength() >
         QApplication::startDragDistance() ) {
       emitNewEventForSelection();
    }
  }
}

KOAgenda::MouseActionType KOAgenda::isInResizeArea( bool horizontal,
    const QPoint &pos, KOAgendaItem*item )
{
  if (!item) return NOP;
  QPoint gridpos = contentsToGrid( pos );
  QPoint contpos = gridToContents( gridpos +
      QPoint( (KOGlobals::self()->reverseLayout())?1:0, 0 ) );

//kdDebug()<<"contpos="<<contpos<<", pos="<<pos<<", gpos="<<gpos<<endl;
//kdDebug()<<"clXLeft="<<clXLeft<<", clXRight="<<clXRight<<endl;

  if ( horizontal ) {
    int clXLeft = item->cellXLeft();
    int clXRight = item->cellXRight();
    if ( KOGlobals::self()->reverseLayout() ) {
      int tmp = clXLeft;
      clXLeft = clXRight;
      clXRight = tmp;
    }
    int gridDistanceX = int( pos.x() - contpos.x() );
    if (gridDistanceX < mResizeBorderWidth && clXLeft == gridpos.x() ) {
      if ( KOGlobals::self()->reverseLayout() ) return RESIZERIGHT;
      else return RESIZELEFT;
    } else if ((mGridSpacingX - gridDistanceX) < mResizeBorderWidth &&
               clXRight == gridpos.x() ) {
      if ( KOGlobals::self()->reverseLayout() ) return RESIZELEFT;
      else return RESIZERIGHT;
    } else {
      return MOVE;
    }
  } else {
    int gridDistanceY = int( pos.y() - contpos.y() );
    if (gridDistanceY < mResizeBorderWidth &&
        item->cellYTop() == gridpos.y() &&
        !item->firstMultiItem() ) {
      return RESIZETOP;
    } else if ((mGridSpacingY - gridDistanceY) < mResizeBorderWidth &&
               item->cellYBottom() == gridpos.y() &&
               !item->lastMultiItem() )  {
      return RESIZEBOTTOM;
    } else {
      return MOVE;
    }
  }
}

void KOAgenda::startItemAction(const QPoint& viewportPos)
{
  QPoint pos = viewportToContents( viewportPos );
  mStartCell = contentsToGrid( pos );
  mEndCell = mStartCell;

  bool noResize = ( mActionItem->incidence()->type() == "Todo");

  mActionType = MOVE;
  if ( !noResize ) {
    mActionType = isInResizeArea( mAllDayMode, pos, mActionItem );
  }

  mActionItem->startMove();
  setActionCursor( mActionType, true );
}

void KOAgenda::performItemAction(const QPoint& viewportPos)
{
//  kdDebug(5850) << "viewportPos: " << viewportPos.x() << "," << viewportPos.y() << endl;
//  QPoint point = viewport()->mapToGlobal(viewportPos);
//  kdDebug(5850) << "Global: " << point.x() << "," << point.y() << endl;
//  point = clipper()->mapFromGlobal(point);
//  kdDebug(5850) << "clipper: " << point.x() << "," << point.y() << endl;
//  kdDebug(5850) << "visible height: " << visibleHeight() << endl;
  QPoint pos = viewportToContents( viewportPos );
//  kdDebug(5850) << "contents: " << x << "," << y << "\n" << endl;
  QPoint gpos = contentsToGrid( pos );
  QPoint clipperPos = clipper()->
                      mapFromGlobal(viewport()->mapToGlobal(viewportPos));

  // Cursor left active agenda area.
  // This starts a drag.
  if ( clipperPos.y() < 0 || clipperPos.y() > visibleHeight() ||
       clipperPos.x() < 0 || clipperPos.x() > visibleWidth() ) {
    if ( mActionType == MOVE ) {
      mScrollUpTimer.stop();
      mScrollDownTimer.stop();
      mActionItem->resetMove();
      placeSubCells( mActionItem );
      emit startDragSignal( mActionItem->incidence() );
      setCursor( arrowCursor );
      mActionItem = 0;
      mActionType = NOP;
      mItemMoved = false;
      return;
    }
  } else {
    setActionCursor( mActionType );
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
  if ( mEndCell != gpos ) {
    mItemMoved = true;
    mActionItem->raise();
    if (mActionType == MOVE) {
      // Move all items belonging to a multi item
      KOAgendaItem *firstItem = mActionItem->firstMultiItem();
      if (!firstItem) firstItem = mActionItem;
      KOAgendaItem *lastItem = mActionItem->lastMultiItem();
      if (!lastItem) lastItem = mActionItem;
      QPoint deltapos = gpos - mEndCell;
      KOAgendaItem *moveItem = firstItem;
      while (moveItem) {
        bool changed=false;
        if ( deltapos.x()!=0 ) {
          moveItem->moveRelative( deltapos.x(), 0 );
          changed=true;
        }
        // in agenda's all day view don't try to move multi items, since there are none
        if ( moveItem==firstItem && !mAllDayMode ) { // is the first item
          int newY = deltapos.y() + moveItem->cellYTop();
          // If event start moved earlier than 0:00, it starts the previous day
          if ( newY<0 ) {
            moveItem->expandTop( -moveItem->cellYTop() );
            // prepend a new item at ( x-1, rows()+newY to rows() )
            KOAgendaItem *newFirst = firstItem->prevMoveItem();
            // cell's y values are first and last cell of the bar, so if newY=-1, they need to be the same
            if (newFirst) {
              newFirst->setCellXY(moveItem->cellXLeft()-1, rows()+newY, rows()-1);
              mItems.append( newFirst );
              moveItem->resize( int( mGridSpacingX * newFirst->cellWidth() ),
                                int( mGridSpacingY * newFirst->cellHeight() ));
              QPoint cpos = gridToContents( QPoint( newFirst->cellXLeft(), newFirst->cellYTop() ) );
              addChild( newFirst, cpos.x(), cpos.y() );
            } else {
              newFirst = insertItem( moveItem->incidence(), moveItem->itemDate(),
                moveItem->cellXLeft()-1, rows()+newY, rows()-1 ) ;
            }
            if (newFirst) newFirst->show();
            moveItem->prependMoveItem(newFirst);
            firstItem=newFirst;
          } else if ( newY>=rows() ) {
            // If event start is moved past 24:00, it starts the next day
            // erase current item (i.e. remove it from the multiItem list)
            firstItem = moveItem->nextMultiItem();
            moveItem->hide();
            mItems.take( mItems.find( moveItem ) );
            removeChild( moveItem );
            mActionItem->removeMoveItem(moveItem);
            moveItem=firstItem;
            // adjust next day's item
            if (moveItem) moveItem->expandTop( rows()-newY );
          } else {
            moveItem->expandTop(deltapos.y());
          }
          changed=true;
        }
        if ( !moveItem->lastMultiItem() && !mAllDayMode ) { // is the last item
          int newY = deltapos.y()+moveItem->cellYBottom();
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
              newLast->setCellXY( moveItem->cellXLeft()+1, 0, newY-rows()-1 );
              mItems.append(newLast);
              moveItem->resize( int( mGridSpacingX * newLast->cellWidth() ),
                                int( mGridSpacingY * newLast->cellHeight() ));
              QPoint cpos = gridToContents( QPoint( newLast->cellXLeft(), newLast->cellYTop() ) ) ;
              addChild( newLast, cpos.x(), cpos.y() );
            } else {
              newLast = insertItem( moveItem->incidence(), moveItem->itemDate(),
                moveItem->cellXLeft()+1, 0, newY-rows()-1 ) ;
            }
            moveItem->appendMoveItem( newLast );
            newLast->show();
            lastItem = newLast;
          } else {
            moveItem->expandBottom( deltapos.y() );
          }
          changed=true;
        }
        if (changed) {
          adjustItemPosition( moveItem );
        }
        moveItem = moveItem->nextMultiItem();
      }
    } else if (mActionType == RESIZETOP) {
      if (mEndCell.y() <= mActionItem->cellYBottom()) {
        mActionItem->expandTop(gpos.y() - mEndCell.y());
        adjustItemPosition( mActionItem );
      }
    } else if (mActionType == RESIZEBOTTOM) {
      if (mEndCell.y() >= mActionItem->cellYTop()) {
        mActionItem->expandBottom(gpos.y() - mEndCell.y());
        adjustItemPosition( mActionItem );
      }
    } else if (mActionType == RESIZELEFT) {
      if (mEndCell.x() <= mActionItem->cellXRight()) {
        mActionItem->expandLeft( gpos.x() - mEndCell.x() );
        adjustItemPosition( mActionItem );
      }
    } else if (mActionType == RESIZERIGHT) {
      if (mEndCell.x() >= mActionItem->cellXLeft()) {
        mActionItem->expandRight(gpos.x() - mEndCell.x());
        adjustItemPosition( mActionItem );
      }
    }
    mEndCell = gpos;
  }
}

void KOAgenda::endItemAction()
{
//  kdDebug(5850) << "KOAgenda::endItemAction() " << endl;
  mScrollUpTimer.stop();
  mScrollDownTimer.stop();
  setCursor( arrowCursor );
  bool multiModify = false;

  if ( mItemMoved ) {
    bool modify = true;
    if ( mActionItem->incidence()->doesRecur() ) {
      int res = KMessageBox::questionYesNoCancel( this, 
          i18n("The item you try to change is a recurring item. Shall the changes "
               "be applied to all items in the recurrence, "/*"only the future items, "*/
               "or just to this single occurrence?"), 
          i18n("Changing a recurring item"), 
          i18n("&All occurrences"), i18n("Only &this item") );
      switch ( res ) {
        case KMessageBox::Yes: // All occurences
            // Moving the whole sequene of events is handled by the itemModified below.
            modify = true;
            break;
        case KMessageBox::No: { // Just this occurence
            // Dissociate this occurence: 
            // create clone of event, set relation to old event, set cloned event 
            // for mActionItem, add exception date to old event, emit incidenceChanged 
            // for the old event, remove the recurrence from the new copy and then just 
            // go on with the newly adjusted mActionItem and let the usual code take 
            // care of the new time!
            modify = true;
            multiModify = true;
            emit startMultiModify( i18n("Dissociate event from recurrence") );
            Incidence* oldInc = mActionItem->incidence()->clone();
            Incidence* newInc = mCalendar->dissociateOccurrence( 
                mActionItem->incidence(), mActionItem->itemDate() );
            if ( newInc ) {
              // don't recreate items, they already have the correct position
              emit enableAgendaUpdate( false );
              emit incidenceChanged( oldInc, mActionItem->incidence() );
              mActionItem->setIncidence( newInc );
              emit incidenceAdded( newInc );
              emit enableAgendaUpdate( true );
            } else {
              KMessageBox::sorry( this, i18n("Unable to add the exception item to the "
                  "calendar. No change will be done."), i18n("Error Occurred") );
            }
            delete oldInc;
            break; }
        case KMessageBox::Continue/*Future*/: { // All future occurences
            // Dissociate this occurence: 
            // create clone of event, set relation to old event, set cloned event 
            // for mActionItem, add recurrence end date to old event, emit incidenceChanged 
            // for the old event, adjust the recurrence for the new copy and then just 
            // go on with the newly adjusted mActionItem and let the usual code take 
            // care of the new time!
            modify = true;
            multiModify = true;
            emit startMultiModify( i18n("Split future recurrences") );
            Incidence* oldInc = mActionItem->incidence()->clone();
            Incidence* newInc = mCalendar->dissociateOccurrence( 
                mActionItem->incidence(), mActionItem->itemDate(), true );
            if ( newInc ) {
              emit incidenceChanged( oldInc, mActionItem->incidence() );
              emit enableAgendaUpdate( false );
              mActionItem->setIncidence( newInc );
              emit incidenceAdded( newInc );
              emit enableAgendaUpdate( true );
            } else {
              KMessageBox::sorry( this, i18n("Unable to add the future items to the "
                  "calendar. No change will be done."), i18n("Error Occurred") );
            }
            delete oldInc;
            break; }
        default:
          modify = false;
          mActionItem->resetMove();
          placeSubCells( mActionItem );
      }
    }

    if ( modify ) {
      mActionItem->endMove();
      KOAgendaItem *placeItem = mActionItem->firstMultiItem();
      if  ( !placeItem ) {
        placeItem = mActionItem;
      }

      KOAgendaItem *modif = placeItem;

      QPtrList<KOAgendaItem> oldconflictItems = placeItem->conflictItems();
      KOAgendaItem *item;
      for ( item = oldconflictItems.first(); item != 0;
            item = oldconflictItems.next() ) {
        placeSubCells( item );
      }
      while ( placeItem ) {
        placeSubCells( placeItem );
        placeItem = placeItem->nextMultiItem();
      }

      // Notify about change, so that agenda view can update the event data
      emit itemModified( modif );
    }
  }

  mActionItem = 0;
  mActionType = NOP;
  mItemMoved = false;

  if ( multiModify ) emit endMultiModify();

  kdDebug(5850) << "KOAgenda::endItemAction() done" << endl;
}

void KOAgenda::setActionCursor( int actionType, bool acting )
{
  switch ( actionType ) {
    case MOVE:
      if (acting) setCursor( sizeAllCursor );
      else setCursor( arrowCursor );
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

void KOAgenda::setNoActionCursor( KOAgendaItem *moveItem, const QPoint& viewportPos )
{
//  kdDebug(5850) << "viewportPos: " << viewportPos.x() << "," << viewportPos.y() << endl;
//  QPoint point = viewport()->mapToGlobal(viewportPos);
//  kdDebug(5850) << "Global: " << point.x() << "," << point.y() << endl;
//  point = clipper()->mapFromGlobal(point);
//  kdDebug(5850) << "clipper: " << point.x() << "," << point.y() << endl;

  QPoint pos = viewportToContents( viewportPos );
  bool noResize = (moveItem && moveItem->incidence() &&
      moveItem->incidence()->type() == "Todo");

  KOAgenda::MouseActionType resizeType = MOVE;
  if ( !noResize ) resizeType = isInResizeArea( mAllDayMode, pos , moveItem);
  setActionCursor( resizeType );
}


/** calculate the width of the column subcells of the given item
*/
double KOAgenda::calcSubCellWidth( KOAgendaItem *item )
{
  QPoint pt, pt1;
  pt = gridToContents( QPoint( item->cellXLeft(), item->cellYTop() ) );
  pt1 = gridToContents( QPoint( item->cellXLeft(), item->cellYTop() ) +
                        QPoint( 1, 1 ) );
  pt1 -= pt;
  int maxSubCells = item->subCells();
  double newSubCellWidth;
  if ( mAllDayMode ) {
    newSubCellWidth = double( pt1.y() ) / maxSubCells;
  } else {
    newSubCellWidth = double( pt1.x() ) / maxSubCells;
  }
  return newSubCellWidth;
}

void KOAgenda::adjustItemPosition( KOAgendaItem *item )
{
  if (!item) return;
  item->resize( int( mGridSpacingX * item->cellWidth() ),
                int( mGridSpacingY * item->cellHeight() ) );
  int clXLeft = item->cellXLeft();
  if ( KOGlobals::self()->reverseLayout() )
    clXLeft = item->cellXRight() + 1;
  QPoint cpos = gridToContents( QPoint( clXLeft, item->cellYTop() ) );
  moveChild( item, cpos.x(), cpos.y() );
}

void KOAgenda::placeAgendaItem( KOAgendaItem *item, double subCellWidth )
{
//  kdDebug() << "KOAgenda::placeAgendaItem(): " << item->incidence()->summary()
//            << " subCellWidth: " << subCellWidth << endl;

  // "left" upper corner, no subcells yet, RTL layouts have right/left switched, widths are negative then
  QPoint pt = gridToContents( QPoint( item->cellXLeft(), item->cellYTop() ) );
  // right lower corner
  QPoint pt1 = gridToContents( QPoint( item->cellXLeft() + item->cellWidth(),
      item->cellYBottom()+1 ) );

  double subCellPos = item->subCell() * subCellWidth;

  // we need to add 0.01 to make sure we don't loose one pixed due to
  // numerics (i.e. if it would be x.9998, we want the integer, not rounded down.
  double delta=0.01;
  if (subCellWidth<0) delta=-delta;
  int height, width, xpos, ypos;
  if (mAllDayMode) {
    width = pt1.x()-pt.x();
    height = int( subCellPos + subCellWidth + delta ) - int( subCellPos );
    xpos = pt.x();
    ypos = pt.y() + int( subCellPos );
  } else {
    width = int( subCellPos + subCellWidth + delta ) - int( subCellPos );
    height = pt1.y()-pt.y();
    xpos = pt.x() + int( subCellPos );
    ypos = pt.y();
  }
  if ( KOGlobals::self()->reverseLayout() ) { // RTL language/layout
    xpos += width;
    width = -width;
  }
  if ( height<0 ) { // BTT (bottom-to-top) layout ?!?
    ypos += height;
    height = -height;
  }
  item->resize( width, height );
  moveChild( item, xpos, ypos );
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
    Incidence *event = placeItem->incidence();
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

  QPtrList<KOrg::CellItem> cells;
  KOAgendaItem *item;
  for ( item = mItems.first(); item != 0; item = mItems.next() ) {
    cells.append( item );
  }

  QPtrList<KOrg::CellItem> items = KOrg::CellItem::placeItem( cells,
                                                              placeItem );

  placeItem->setConflictItems( QPtrList<KOAgendaItem>() );
  double newSubCellWidth = calcSubCellWidth( placeItem );
  KOrg::CellItem *i;
  for ( i = items.first(); i; i = items.next() ) {
    item = static_cast<KOAgendaItem *>( i );
    placeAgendaItem( item, newSubCellWidth );
    item->addConflictItem( placeItem );
    placeItem->addConflictItem( item );
  }
  if ( items.isEmpty() ) {
    placeAgendaItem( placeItem, newSubCellWidth );
  }
  placeItem->update();
}

int KOAgenda::columnWidth( int column )
{
  int start = gridToContents( QPoint( column, 0 ) ).x();
  if (KOGlobals::self()->reverseLayout() )
    column--;
  else
    column++;
  int end = gridToContents( QPoint( column, 0 ) ).x();
  return end - start;
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
    QPoint pt1( cx, mWorkingHoursYTop );
    QPoint pt2( cx+cw, mWorkingHoursYBottom );
    if ( pt2.x() >= pt1.x() /*&& pt2.y() >= pt1.y()*/) {
      int gxStart = contentsToGrid( pt1 ).x();
      int gxEnd = contentsToGrid( pt2 ).x();
      // correct start/end for rtl layouts
      if ( gxStart > gxEnd ) {
        int tmp = gxStart;
        gxStart = gxEnd;
        gxEnd = tmp;
      }
      int xoffset = ( KOGlobals::self()->reverseLayout()?1:0 );
      while( gxStart <= gxEnd ) {
        int xStart = gridToContents( QPoint( gxStart+xoffset, 0 ) ).x();
        int xWidth = columnWidth( gxStart ) + 1;
        if ( pt2.y() < pt1.y() ) {
          // overnight working hours
          if ( ( (gxStart==0) && !mHolidayMask->at(mHolidayMask->count()-1) ) ||
               ( (gxStart>0) && (gxStart<int(mHolidayMask->count())) && (!mHolidayMask->at(gxStart-1) ) ) ) {
            if ( pt2.y() > cy ) {
              dbp.fillRect( xStart, cy, xWidth, pt2.y() - cy + 1,
                            KOPrefs::instance()->mWorkingHoursColor);
            }
          }
          if ( (gxStart < int(mHolidayMask->count()-1)) && (!mHolidayMask->at(gxStart)) ) {
            if ( pt1.y() < cy + ch - 1 ) {
              dbp.fillRect( xStart, pt1.y(), xWidth, cy + ch - pt1.y() + 1,
                            KOPrefs::instance()->mWorkingHoursColor);
            }
          }
        } else {
          // last entry in holiday mask denotes the previous day not visible (needed for overnight shifts)
          if ( gxStart < int(mHolidayMask->count()-1) && !mHolidayMask->at(gxStart)) {
            dbp.fillRect( xStart, pt1.y(), xWidth, pt2.y() - pt1.y() + 1,
                          KOPrefs::instance()->mWorkingHoursColor );
          }
        }
        ++gxStart;
      }
    }
  }

  // draw selection
  if ( mHasSelection ) {
    QPoint pt, pt1;

    if ( mSelectionEndCell.x() > mSelectionStartCell.x() ) { // multi day selection
      // draw start day
      pt = gridToContents( mSelectionStartCell );
      pt1 = gridToContents( QPoint( mSelectionStartCell.x() + 1, mRows + 1 ) );
      dbp.fillRect( QRect( pt, pt1 ), KOPrefs::instance()->mHighlightColor );
      // draw all other days between the start day and the day of the selection end
      for ( int c = mSelectionStartCell.x() + 1; c < mSelectionEndCell.x(); ++c ) {
        pt = gridToContents( QPoint( c, 0 ) );
        pt1 = gridToContents( QPoint( c + 1, mRows + 1 ) );
        dbp.fillRect( QRect( pt, pt1 ), KOPrefs::instance()->mHighlightColor );
      }
      // draw end day
      pt = gridToContents( QPoint( mSelectionEndCell.x(), 0 ) );
      pt1 = gridToContents( mSelectionEndCell + QPoint(1,1) );
      dbp.fillRect( QRect( pt, pt1), KOPrefs::instance()->mHighlightColor );
    }  else { // single day selection
      pt = gridToContents( mSelectionStartCell );
      pt1 = gridToContents( mSelectionEndCell + QPoint(1,1) );
      dbp.fillRect( QRect( pt, pt1 ), KOPrefs::instance()->mHighlightColor );
    }
  }

  dbp.setPen( KOPrefs::instance()->mAgendaBgColor.dark(150) );

  // Draw vertical lines of grid, start with the last line not yet visible
  //  kdDebug(5850) << "drawContents cx: " << cx << " cy: " << cy << " cw: " << cw << " ch: " << ch << endl;
  double x = ( int( cx / mGridSpacingX ) ) * mGridSpacingX;
  while (x < cx + cw) {
    dbp.drawLine( int( x ), cy, int( x ), cy + ch );
    x+=mGridSpacingX;
  }

  // Draw horizontal lines of grid
  double y = ( int( cy / lGridSpacingY ) ) * lGridSpacingY;
  while (y < cy + ch) {
//    kdDebug(5850) << " y: " << y << endl;
    dbp.drawLine( cx, int( y ), cx + cw, int( y ) );
    y+=lGridSpacingY;
  }
  p->drawPixmap(cx,cy, db);
}

/*
  Convert srcollview contents coordinates to agenda grid coordinates.
*/
QPoint KOAgenda::contentsToGrid ( const QPoint &pos ) const
{
  int gx = int( KOGlobals::self()->reverseLayout() ?
        mColumns - pos.x()/mGridSpacingX : pos.x()/mGridSpacingX );
  int gy = int( pos.y()/mGridSpacingY );
  return QPoint( gx, gy );
}

/*
  Convert agenda grid coordinates to scrollview contents coordinates.
*/
QPoint KOAgenda::gridToContents( const QPoint &gpos ) const
{
  int x = int( KOGlobals::self()->reverseLayout() ?
             (mColumns - gpos.x())*mGridSpacingX : gpos.x()*mGridSpacingX );
  int y = int( gpos.y()*mGridSpacingY );
  return QPoint( x, y );
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

QMemArray<int> KOAgenda::minContentsY()
{
  QMemArray<int> minArray;
  minArray.fill( timeToY( QTime(23, 59) ), mSelectedDates.count() );
  for ( KOAgendaItem *item = mItems.first();
        item != 0;
        item = mItems.next() ) {
    int timeY = timeToY( item->incidence()->dtStart().time() );
    int index = mSelectedDates.findIndex( item->incidence()->dtStart().date() );
    if( timeY < minArray[index] && mItemsToDelete.findRef( item ) == -1 )
      minArray[index] = timeY;
  }

  return minArray;
}

QMemArray<int> KOAgenda::maxContentsY()
{
  QMemArray<int> maxArray;
  maxArray.fill( timeToY( QTime(0, 0) ), mSelectedDates.count() );
  for ( KOAgendaItem *item = mItems.first();
        item != 0;
        item = mItems.next() ) {
    QDateTime dtEnd;
    if ( item->incidence()->type() == "Todo" )
      dtEnd = static_cast<Todo *>( item->incidence() )->dtDue();
    else
      dtEnd = item->incidence()->dtEnd();
    int timeY = timeToY( dtEnd.time() );
    int index = mSelectedDates.findIndex( dtEnd.date() );
    if( timeY > maxArray[index] && mItemsToDelete.findRef( item ) == -1 )
      maxArray[index] = timeY - 1;
  }

  return maxArray;
}

void KOAgenda::setStartTime( QTime startHour )
{
  double startPos = ( startHour.hour()/24. + startHour.minute()/1440. +
                      startHour.second()/86400. ) * mRows * gridSpacingY();
  setContentsPos( 0, int( startPos ) );
}


/*
  Insert KOAgendaItem into agenda.
*/
KOAgendaItem *KOAgenda::insertItem( Incidence *incidence, QDate qd, int X,
                                    int YTop, int YBottom )
{
#if 0
  kdDebug(5850) << "KOAgenda::insertItem:" << event->summary() << "-"
                << qd.toString() << " ;top, bottom:" << YTop << "," << YBottom
                << endl;
#endif

  if ( mAllDayMode ) {
    kdDebug(5850) << "KOAgenda: calling insertItem in all-day mode is illegal." << endl;
    return 0;
  }
  mActionType = NOP;

  KOAgendaItem *agendaItem = new KOAgendaItem( incidence, qd, viewport() );
  connect( agendaItem, SIGNAL( removeAgendaItem( KOAgendaItem * ) ),
           SLOT( removeAgendaItem( KOAgendaItem * ) ) );
  connect( agendaItem, SIGNAL( showAgendaItem( KOAgendaItem * ) ),
           SLOT( showAgendaItem( KOAgendaItem * ) ) );

  if ( YBottom <= YTop ) {
    kdDebug(5850) << "KOAgenda::insertItem(): Text: " << agendaItem->text() << " YSize<0" << endl;
    YBottom = YTop;
  }

  agendaItem->resize( int( ( X + 1 ) * mGridSpacingX ) -
                      int( X * mGridSpacingX ),
                      int( YTop * mGridSpacingY ) -
                      int( ( YBottom + 1 ) * mGridSpacingY ) );
  agendaItem->setCellXY( X, YTop, YBottom );
  agendaItem->setCellXRight( X );

  agendaItem->installEventFilter( this );

  addChild( agendaItem, int( X * mGridSpacingX ), int( YTop * mGridSpacingY ) );
  mItems.append( agendaItem );

  placeSubCells( agendaItem );

  agendaItem->show();

  marcus_bains();

  return agendaItem;
}

/*
  Insert all-day KOAgendaItem into agenda.
*/
KOAgendaItem *KOAgenda::insertAllDayItem( Incidence *event, QDate qd,
                                          int XBegin, int XEnd )
{
  if ( !mAllDayMode ) {
    kdDebug(5850) << "KOAgenda: calling insertAllDayItem in non all-day mode is illegal." << endl;
    return 0;
  }
  mActionType = NOP;

  KOAgendaItem *agendaItem = new KOAgendaItem( event, qd, viewport() );
  connect( agendaItem, SIGNAL( removeAgendaItem( KOAgendaItem* ) ),
           SLOT( removeAgendaItem( KOAgendaItem* ) ) );
  connect( agendaItem, SIGNAL( showAgendaItem( KOAgendaItem* ) ),
           SLOT( showAgendaItem( KOAgendaItem* ) ) );

  agendaItem->setCellXY( XBegin, 0, 0 );
  agendaItem->setCellXRight( XEnd );

  double startIt = mGridSpacingX * ( agendaItem->cellXLeft() );
  double endIt = mGridSpacingX * ( agendaItem->cellWidth() +
                                   agendaItem->cellXLeft() );

  agendaItem->resize( int( endIt ) - int( startIt ), int( mGridSpacingY ) );

  agendaItem->installEventFilter( this );

  addChild( agendaItem, int( XBegin * mGridSpacingX ), 0 );
  mItems.append( agendaItem );

  placeSubCells( agendaItem );

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
  mActionType = NOP;

  int cellX,cellYTop,cellYBottom;
  QString newtext;
  int width = XEnd - XBegin + 1;
  int count = 0;
  KOAgendaItem *current = 0;
  int visibleCount = mSelectedDates.first().daysTo(mSelectedDates.last());
  QPtrList<KOAgendaItem> multiItems;
  for ( cellX = XBegin; cellX <= XEnd; ++cellX ) {
    ++count;
    //Only add the items that are visible.
    if( cellX >=0 && cellX <= visibleCount ) {
      if ( cellX == XBegin ) cellYTop = YTop;
      else cellYTop = 0;
      if ( cellX == XEnd ) cellYBottom = YBottom;
      else cellYBottom = rows() - 1;
      newtext = QString("(%1/%2): ").arg( count ).arg( width );
      newtext.append( event->summary() );
      current = insertItem( event, qd, cellX, cellYTop, cellYBottom );
      current->setText( newtext );
      multiItems.append( current );
    }
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

void KOAgenda::removeIncidence( Incidence *incidence )
{
  // First find all items to be deleted and store them
  // in its own list. Otherwise removeAgendaItem will reset
  // the current position and mess this up.
  QPtrList<KOAgendaItem> itemsToRemove;

  KOAgendaItem *item = mItems.first();
  while ( item ) {
    if ( item->incidence() == incidence ) {
      itemsToRemove.append( item );
    }
    item = mItems.next();
  }
  item = itemsToRemove.first();
  while ( item ) {
    removeAgendaItem( item );
    item = itemsToRemove.next();
  }
}

void KOAgenda::showAgendaItem( KOAgendaItem *agendaItem )
{
  if ( !agendaItem ) return;
  agendaItem->hide();
  addChild( agendaItem );
  if ( !mItems.containsRef( agendaItem ) )
    mItems.append( agendaItem );
  placeSubCells( agendaItem );
  agendaItem->show();
}

bool KOAgenda::removeAgendaItem( KOAgendaItem *item )
{
  // we found the item. Let's remove it and update the conflicts
  bool taken = false;
  KOAgendaItem *thisItem = item;
  QPtrList<KOAgendaItem> conflictItems = thisItem->conflictItems();
  removeChild( thisItem );
  int pos = mItems.find( thisItem );
  if ( pos>=0 ) {
    mItems.take( pos );
    taken = true;
  }

  KOAgendaItem *confitem;
  for ( confitem = conflictItems.first(); confitem != 0;
        confitem = conflictItems.next() ) {
    // the item itself is also in its own conflictItems list!
    if ( confitem != thisItem ) placeSubCells(confitem);

  }
  mItemsToDelete.append( thisItem );
  QTimer::singleShot( 0, this, SLOT( deleteItemsToDelete() ) );
  return taken;
}

void KOAgenda::deleteItemsToDelete()
{
  mItemsToDelete.clear();
}

/*QSizePolicy KOAgenda::sizePolicy() const
{
  // Thought this would make the all-day event agenda minimum size and the
  // normal agenda take the remaining space. But it doesnt work. The QSplitter
  // dont seem to think that an Expanding widget needs more space than a
  // Preferred one.
  // But it doesnt hurt, so it stays.
  if (mAllDayMode) {
    return QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
  } else {
    return QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  }
}
*/

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
    resizeContents( int( mGridSpacingX * mColumns ), int( mGridSpacingY ) );

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
  int yMin = int( v / mGridSpacingY );
  int yMax = int( ( v + visibleHeight() ) / mGridSpacingY );

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
  assert( mSelectedItem->incidence() );
  mSelectedUid = mSelectedItem->incidence()->uid();
  emit incidenceSelected( mSelectedItem->incidence() );
}

void KOAgenda::selectItemByUID( const QString& uid )
{
  KOAgendaItem *item;
  for ( item = mItems.first(); item != 0; item = mItems.next() ) {
    if( item->incidence() && item->incidence()->uid() == uid ) {
      selectItem( item );
      break;
    }
  }
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
  mWorkingHoursEnable = !mAllDayMode;

  QTime tmp = KOPrefs::instance()->mWorkingHoursStart.time();
  mWorkingHoursYTop = int( 4 * mGridSpacingY *
                           ( tmp.hour() + tmp.minute() / 60. +
                             tmp.second() / 3600. ) );
  tmp = KOPrefs::instance()->mWorkingHoursEnd.time();
  mWorkingHoursYBottom = int( 4 * mGridSpacingY *
                              ( tmp.hour() + tmp.minute() / 60. +
                                tmp.second() / 3600. ) - 1 );
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
