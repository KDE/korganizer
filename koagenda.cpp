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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

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

#include "koagenda.h"
#include "koagenda.moc"

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
	    return QApplication::reverseLayout() ?
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
    int y = mins*agenda->gridSpacingY()/minutesPerCell;
    int today = recalculate ? todayColumn() : oldToday;
    int x = agenda->gridSpacingX()*today;
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
	setFixedSize(agenda->gridSpacingX(),1);
    agenda->moveChild(this, x, y);
    raise();

    if(recalculate)
	mTimeBox->setFont(KOPrefs::instance()->mMarcusBainsFont);

    mTimeBox->setText(KGlobal::locale()->formatTime(tim, KOPrefs::instance()->mMarcusBainsShowSeconds));
    mTimeBox->adjustSize();
    // the -2 below is there because there is a bug in this program
    // somewhere, where the last column of this widget is a few pixels
    // narrower than the other columns.
    int offs = (today==agenda->columns()-1) ? -4 : 0;
    agenda->moveChild(mTimeBox,
		      x+agenda->gridSpacingX()-mTimeBox->width()+offs,
		      y-mTimeBox->height());
    mTimeBox->raise();
    mTimeBox->setAutoMask(true);

    minutes->start(1000,true);
}


////////////////////////////////////////////////////////////////////////////


/*
  Create an agenda widget with rows rows and columns columns.
*/
KOAgenda::KOAgenda(int columns,int rows,int rowSize,QWidget *parent,
                   const char *name,WFlags f) :
  QScrollView(parent,name,f)
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
KOAgenda::KOAgenda(int columns,QWidget *parent,const char *name,WFlags f) :
  QScrollView(parent,name,f)
{
  mColumns = columns;
  mRows = 1;
  mGridSpacingY = 24;
  mAllDayMode = true;

  init();
}


KOAgenda::~KOAgenda()
{
    if(mMarcusBains) delete mMarcusBains;
}


void KOAgenda::init()
{
  mGridSpacingX = 100;

  mResizeBorderWidth = 8;
  mScrollBorderWidth = 8;
  mScrollDelay = 30;
  mScrollOffset = 10;

  enableClipper(true);

  // Grab key strokes for keyboard navigation of agenda. Seems to have no
  // effect. Has to be fixed.
  setFocusPolicy(WheelFocus);

  connect(&mScrollUpTimer,SIGNAL(timeout()),SLOT(scrollUp()));
  connect(&mScrollDownTimer,SIGNAL(timeout()),SLOT(scrollDown()));

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

  mItems.setAutoDelete(true);

  resizeContents( mGridSpacingX * mColumns + 1 , mGridSpacingY * mRows + 1 );

  viewport()->update();

  setMinimumSize(30, mGridSpacingY + 1);
//  setMaximumHeight(mGridSpacingY * mRows + 5);

  // Disable horizontal scrollbar. This is a hack. The geometry should be
  // controlled in a way that the contents horizontally always fits. Then it is
  // not necessary to turn off the scrollbar.
  setHScrollBarMode(AlwaysOff);

  setStartHour(KOPrefs::instance()->mDayBegins);

  calculateWorkingHours();

  connect(verticalScrollBar(),SIGNAL(valueChanged(int)),
          SLOT(checkScrollBoundaries(int)));

  // Create the Marcus Bains line.
  if(mAllDayMode)
      mMarcusBains = 0;
  else {
      mMarcusBains = new MarcusBains(this);
      addChild(mMarcusBains);
  }
}


void KOAgenda::clear()
{
//  kdDebug() << "KOAgenda::clear()" << endl;

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
    kdDebug() << "KOAgenda::changeColumns() called with argument 0" << endl;
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


Event *KOAgenda::selectedEvent()
{
  if (mSelectedItem) {
    return mSelectedItem->itemEvent();
  } else {
    return 0;
  }
}

QDate KOAgenda::selectedEventDate()
{
  QDate qd;
  if (mSelectedItem) {
    qd = mSelectedItem->itemDate();
  }
  return qd;
}

/*
  This is the eventFilter function, which gets all events from the KOAgendaItems
  contained in the agenda. It has to handle moving and resizing for all items.
*/
bool KOAgenda::eventFilter ( QObject *object, QEvent *event )
{
//  kdDebug() << "KOAgenda::eventFilter" << endl;

  switch(event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
      return eventFilter_mouse(object, static_cast<QMouseEvent *>(event));

    case (QEvent::Leave):
      if (!mActionItem)
        setCursor(arrowCursor);
      return true;

    default:
      return QScrollView::eventFilter(object,event);
  }
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
//        kdDebug() << "koagenda: filtered button press" << endl;
      if (object != viewport()) {
        if (me->button() == RightButton) {
          mClickedItem = (KOAgendaItem *)object;
          if (mClickedItem) {
            selectItem(mClickedItem);
            emit showEventPopupSignal(mClickedItem->itemEvent());
          }
    //            mItemPopup->popup(QCursor::pos());
        } else {
          mActionItem = (KOAgendaItem *)object;
          if (mActionItem) {
            selectItem(mActionItem);
            Event *event = mActionItem->itemEvent();
            if ( event->isReadOnly() || event->recurrence()->doesRecur() ) {
              mActionItem = 0;
            } else {
              startItemAction(viewportPos);
            }
          }
        }
      } else {
        selectItem(0);
        mActionItem = 0;
        setCursor(arrowCursor);
        startSelectAction(viewportPos);
      }
      break;

    case QEvent::MouseButtonRelease:
      if (mActionItem) {
        endItemAction();
      } else if ( mActionType == SELECT ) {
        endSelectAction();
      }
      break;

    case QEvent::MouseMove:
      if (object != viewport()) {
        KOAgendaItem *moveItem = (KOAgendaItem *)object;
        if (!moveItem->itemEvent()->isReadOnly() &&
            !moveItem->itemEvent()->recurrence()->doesRecur() )
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
        emit editEventSignal(doubleClickedItem->itemEvent());
      }
      break;

    default:
      break;
  }

  return true;
}

void KOAgenda::startSelectAction(QPoint viewportPos)
{
  emit newStartSelectSignal();

  mActionType = SELECT;

  int x,y;
  viewportToContents(viewportPos.x(),viewportPos.y(),x,y);
  int gx,gy;
  contentsToGrid(x,y,gx,gy);

  mStartCellX = gx;
  mStartCellY = gy;
  mCurrentCellX = gx;
  mCurrentCellY = gy;

  // Store coordinates of old selection
  int selectionX = mSelectionCellX * mGridSpacingX;
  int selectionYTop = mSelectionYTop;
  int selectionHeight = mSelectionHeight;

  // Store new selection
  mSelectionCellX = gx;
  mSelectionYTop = gy * mGridSpacingY;
  mSelectionHeight = mGridSpacingY;
  
  // Clear old selection
  repaintContents( selectionX, selectionYTop,
                   mGridSpacingX, selectionHeight );

  // Paint new selection
  repaintContents( mSelectionCellX * mGridSpacingX, mSelectionYTop,
                   mGridSpacingX, mSelectionHeight );
}

void KOAgenda::performSelectAction(QPoint viewportPos)
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

  if ( gy > mCurrentCellY ) {
    mSelectionHeight = ( gy + 1 ) * mGridSpacingY - mSelectionYTop;

#if 0
    // FIXME: Repaint only the newly selected region
    repaintContents( mSelectionCellX * mGridSpacingX,
                     mCurrentCellY + mGridSpacingY,
                     mGridSpacingX,
                     mSelectionHeight - ( gy - mCurrentCellY - 1 ) * mGridSpacingY );
#else
    repaintContents( (QApplication::reverseLayout() ?
                     mColumns - 1 - mSelectionCellX : mSelectionCellX) *
                     mGridSpacingX, mSelectionYTop,
                     mGridSpacingX, mSelectionHeight );
#endif

    mCurrentCellY = gy;
  } else if ( gy < mCurrentCellY ) {
    if ( gy >= mStartCellY ) {
      int selectionHeight = mSelectionHeight;
      mSelectionHeight = ( gy + 1 ) * mGridSpacingY - mSelectionYTop;

      repaintContents( (QApplication::reverseLayout() ?
                       mColumns - 1 - mSelectionCellX : mSelectionCellX) *
                       mGridSpacingX, mSelectionYTop,
                       mGridSpacingX, selectionHeight );
    
      mCurrentCellY = gy;
    } else {
    }
  }
}

void KOAgenda::endSelectAction()
{
  mActionType = NOP;
  mScrollUpTimer.stop();
  mScrollDownTimer.stop();

  emit newTimeSpanSignal(mStartCellX,mStartCellY,mCurrentCellX,mCurrentCellY);
}

void KOAgenda::startItemAction(QPoint viewportPos)
{
  int x,y;
  viewportToContents(viewportPos.x(),viewportPos.y(),x,y);
  int gx,gy;
  contentsToGrid(x,y,gx,gy);

  mStartCellX = gx;
  mStartCellY = gy;
  mCurrentCellX = gx;
  mCurrentCellY = gy;

  if (mAllDayMode) {
    int gridDistanceX = (x - gx * mGridSpacingX);
    if (gridDistanceX < mResizeBorderWidth &&
        mActionItem->cellX() == mCurrentCellX) {
      mActionType = RESIZELEFT;
      setCursor(sizeHorCursor);
    } else if ((mGridSpacingX - gridDistanceX) < mResizeBorderWidth &&
               mActionItem->cellXWidth() == mCurrentCellX) {
      mActionType = RESIZERIGHT;
      setCursor(sizeHorCursor);
    } else {
      mActionType = MOVE;
      mActionItem->startMove();
      setCursor(sizeAllCursor);
    }
  } else {
    int gridDistanceY = (y - gy * mGridSpacingY);
    if (gridDistanceY < mResizeBorderWidth &&
        mActionItem->cellYTop() == mCurrentCellY &&
        !mActionItem->firstMultiItem()) {
      mActionType = RESIZETOP;
      setCursor(sizeVerCursor);
    } else if ((mGridSpacingY - gridDistanceY) < mResizeBorderWidth &&
               mActionItem->cellYBottom() == mCurrentCellY &&
	       !mActionItem->lastMultiItem())  {
      mActionType = RESIZEBOTTOM;
      setCursor(sizeVerCursor);
    } else {
      mActionType = MOVE;
      mActionItem->startMove();
      setCursor(sizeAllCursor);
    }
  }
}

void KOAgenda::performItemAction(QPoint viewportPos)
{
//  kdDebug() << "viewportPos: " << viewportPos.x() << "," << viewportPos.y() << endl;
//  QPoint point = viewport()->mapToGlobal(viewportPos);
//  kdDebug() << "Global: " << point.x() << "," << point.y() << endl;
//  point = clipper()->mapFromGlobal(point);
//  kdDebug() << "clipper: " << point.x() << "," << point.y() << endl;
//  kdDebug() << "visible height: " << visibleHeight() << endl;
  int x,y;
  viewportToContents(viewportPos.x(),viewportPos.y(),x,y);
//  kdDebug() << "contents: " << x << "," << y << "\n" << endl;
  int gx,gy;
  contentsToGrid(x,y,gx,gy);
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
      emit startDragSignal( mActionItem->itemEvent() );
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
      KOAgendaItem *moveItem = mActionItem->firstMultiItem();
      bool isMultiItem = (moveItem || mActionItem->lastMultiItem());
      if (!moveItem) moveItem = mActionItem;
      while (moveItem) {
        int dy;
        if (isMultiItem) dy = 0;
        else dy = gy - mCurrentCellY;
        moveItem->moveRelative(gx - mCurrentCellX,dy);
        int x,y;
        gridToContents(moveItem->cellX(),moveItem->cellYTop(),x,y);
        moveItem->resize(mGridSpacingX * moveItem->cellWidth(),
	                 mGridSpacingY * moveItem->cellHeight());
        moveChild(moveItem,x,y);
        moveItem = moveItem->nextMultiItem();
      }
    } else if (mActionType == RESIZETOP) {
      if (mCurrentCellY <= mActionItem->cellYBottom()) {
        mActionItem->expandTop(gy - mCurrentCellY);
        mActionItem->resize(mActionItem->width(),
                            mGridSpacingY * mActionItem->cellHeight());
        int x,y;
        gridToContents(mCurrentCellX,mActionItem->cellYTop(),x,y);
        moveChild(mActionItem,childX(mActionItem),y);
      }
    } else if (mActionType == RESIZEBOTTOM) {
      if (mCurrentCellY >= mActionItem->cellYTop()) {
        mActionItem->expandBottom(gy - mCurrentCellY);
        mActionItem->resize(mActionItem->width(),
                            mGridSpacingY * mActionItem->cellHeight());
      }
    } else if (mActionType == RESIZELEFT) {
       if (mCurrentCellX <= mActionItem->cellXWidth()) {
         mActionItem->expandLeft(gx - mCurrentCellX);
         mActionItem->resize(mGridSpacingX * mActionItem->cellWidth(),
                             mActionItem->height());
        int x,y;
        gridToContents(mActionItem->cellX(),mActionItem->cellYTop(),x,y);
        moveChild(mActionItem,x,childY(mActionItem));
       }
    } else if (mActionType == RESIZERIGHT) {
       if (mCurrentCellX >= mActionItem->cellX()) {
         mActionItem->expandRight(gx - mCurrentCellX);
         mActionItem->resize(mGridSpacingX * mActionItem->cellWidth(),
                             mActionItem->height());
       }
    }
    mCurrentCellX = gx;
    mCurrentCellY = gy;
  }
}

void KOAgenda::endItemAction()
{
//  kdDebug() << "KOAgenda::endItemAction()" << endl;

  if ( mItemMoved ) {
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

//  kdDebug() << "KOAgenda::endItemAction() done" << endl;
}

void KOAgenda::setNoActionCursor(KOAgendaItem *moveItem,QPoint viewportPos)
{
//  kdDebug() << "viewportPos: " << viewportPos.x() << "," << viewportPos.y() << endl;
//  QPoint point = viewport()->mapToGlobal(viewportPos);
//  kdDebug() << "Global: " << point.x() << "," << point.y() << endl;
//  point = clipper()->mapFromGlobal(point);
//  kdDebug() << "clipper: " << point.x() << "," << point.y() << endl;

  int x,y;
  viewportToContents(viewportPos.x(),viewportPos.y(),x,y);
//  kdDebug() << "contents: " << x << "," << y << "\n" << endl;
  int gx,gy;
  contentsToGrid(x,y,gx,gy);

  // Change cursor to resize cursor if appropriate
  if (mAllDayMode) {
    int gridDistanceX = (x - gx * mGridSpacingX);
    if (gridDistanceX < mResizeBorderWidth &&
        moveItem->cellX() == gx) {
      setCursor(sizeHorCursor);
    } else if ((mGridSpacingX - gridDistanceX) < mResizeBorderWidth &&
             moveItem->cellXWidth() == gx) {
      setCursor(sizeHorCursor);
    } else {
      setCursor(arrowCursor);
    }
  } else {
    int gridDistanceY = (y - gy * mGridSpacingY);
    if (gridDistanceY < mResizeBorderWidth &&
        moveItem->cellYTop() == gy &&
        !moveItem->firstMultiItem()) {
      setCursor(sizeVerCursor);
    } else if ((mGridSpacingY - gridDistanceY) < mResizeBorderWidth &&
               moveItem->cellYBottom() == gy &&
	       !moveItem->lastMultiItem()) {
      setCursor(sizeVerCursor);
    } else {
      setCursor(arrowCursor);
    }
  }
}


/*
  Place item in cell and take care that multiple items using the same cell do
  not overlap. This method is not yet optimal. It doesn�t use the maximum space
  it can get in all cases.
  At the moment the method has a bug: When an item is placed only the sub cell
  widths of the items are changed, which are within the Y region the item to
  place spans. When the sub cell width change of one of this items affects a
  cell, where other items are, which do not overlap in Y with the item to place,
  the display gets corrupted, although the corruption looks quite nice.
*/
void KOAgenda::placeSubCells(KOAgendaItem *placeItem)
{
#if 0
  kdDebug() << "KOAgenda::placeSubCells()" << endl;
  if ( placeItem ) {
    Event *event = placeItem->itemEvent();
    if ( !event ) {
      kdDebug() << "  event is 0" << endl;
    } else {
      kdDebug() << "  event: " << event->summary() << endl;
    }
  } else {
    kdDebug() << "  placeItem is 0" << endl;
  }
  kdDebug() << "KOAgenda::placeSubCells()..." << endl;
#endif

  QPtrList<KOAgendaItem> conflictItems;
  int maxSubCells = 0;
  QIntDict<KOAgendaItem> subCellDict(5);

  KOAgendaItem *item;
  for ( item=mItems.first(); item != 0; item=mItems.next() ) {
    if (item != placeItem) {
      if (placeItem->cellX() <= item->cellXWidth() &&
          placeItem->cellXWidth() >= item->cellX()) {
        if ((placeItem->cellYTop() <= item->cellYBottom()) &&
            (placeItem->cellYBottom() >= item->cellYTop())) {
          conflictItems.append(item);
          if (item->subCells() > maxSubCells)
            maxSubCells = item->subCells();
          subCellDict.insert(item->subCell(),item);
        }
      }
    }
  }

  if (conflictItems.count() > 0) {
    // Look for unused sub cell and insert item
    int i;
    for(i=0;i<maxSubCells;++i) {
      if (!subCellDict.find(i)) {
        placeItem->setSubCell(i);
	break;
      }
    }
    if (i == maxSubCells) {
      placeItem->setSubCell(maxSubCells);
      maxSubCells++;  // add new item to number of sub cells
    }

    // Prepare for sub cell geometry adjustment
    int newSubCellWidth;
    if (mAllDayMode) newSubCellWidth = mGridSpacingY / maxSubCells;
    else newSubCellWidth = mGridSpacingX / maxSubCells;
    conflictItems.append(placeItem);

//    kdDebug() << "---Conflict items: " << conflictItems.count() << endl;

    // Adjust sub cell geometry of all items
    for ( item=conflictItems.first(); item != 0;
          item=conflictItems.next() ) {
//      kdDebug() << "---Placing item: " << item->itemEvent()->getSummary() << endl;
      item->setSubCells(maxSubCells);
      if (mAllDayMode) {
        item->resize(item->cellWidth() * mGridSpacingX, newSubCellWidth);
      } else {
        item->resize(newSubCellWidth, item->cellHeight() * mGridSpacingY);
      }
      int x,y;
      gridToContents(item->cellX(),item->cellYTop(),x,y);
      if (mAllDayMode) {
        y += item->subCell() * newSubCellWidth;
      } else {
        x += item->subCell() * newSubCellWidth;
      }
      moveChild(item,x,y);
    }
  } else {
    placeItem->setSubCell(0);
    placeItem->setSubCells(1);
    if (mAllDayMode) placeItem->resize(placeItem->width(),mGridSpacingY);
    else placeItem->resize(mGridSpacingX,placeItem->height());
    int x,y;
    gridToContents(placeItem->cellX(),placeItem->cellYTop(),x,y);
    moveChild(placeItem,x,y);
  }
  placeItem->setConflictItems(conflictItems);
}

/*
  Draw grid in the background of the agenda.
*/
void KOAgenda::drawContents(QPainter* p, int cx, int cy, int cw, int ch)
{
//  kdDebug() << "KOAgenda::drawContents()" << endl;
  int lGridSpacingY = mGridSpacingY*2;
  
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
      int gxStart = x1/mGridSpacingX;
      int gxEnd = x2/mGridSpacingX;
      while(gxStart <= gxEnd) {
        if (gxStart < int(mHolidayMask->count()) &&
            !mHolidayMask->at(gxStart)) {
          int xStart = QApplication::reverseLayout() ?
                                    (mColumns - 1 - gxStart)*mGridSpacingX :
                                     gxStart*mGridSpacingX;
          if (xStart < x1) xStart = x1;
          int xEnd = QApplication::reverseLayout() ?
                                    (mColumns - gxStart)*mGridSpacingX-1 :
                                    (gxStart+1)*mGridSpacingX-1;
          if (xEnd > x2) xEnd = x2;
          p->fillRect(xStart,y1,xEnd-xStart+1,y2-y1+1,
                      KOPrefs::instance()->mWorkingHoursColor);
        }
        ++gxStart;
      }
    }
  }

  int selectionX = QApplication::reverseLayout() ?
                   (mColumns - 1 - mSelectionCellX) * mGridSpacingX : 
                    mSelectionCellX * mGridSpacingX;

  // Draw selection
  if ( ( cx + cw ) >= selectionX && cx <= ( selectionX + mGridSpacingX ) &&
       ( cy + ch ) >= mSelectionYTop && cy <= ( mSelectionYTop + mSelectionHeight ) ) {
    // TODO: paint only part within cx,cy,cw,ch
    p->fillRect( selectionX, mSelectionYTop, mGridSpacingX,
                 mSelectionHeight, KOPrefs::instance()->mHighlightColor );
  }

  // Draw vertical lines of grid
  //  kdDebug() << "drawContents cx: " << cx << " cy: " << cy << " cw: " << cw << " ch: " << ch << endl;
  int x = ((int)(cx/mGridSpacingX))*mGridSpacingX;
  while (x < cx + cw) {
    p->drawLine(x,cy,x,cy+ch);
    x+=mGridSpacingX;
  }

  // Draw horizontal lines of grid
  int y = ((int)(cy/lGridSpacingY))*lGridSpacingY;
  while (y < cy + ch) {
//    kdDebug() << " y: " << y << endl;
    p->drawLine(cx,y,cx+cw,y);
    y+=lGridSpacingY;
  }
}

/*
  Convert srcollview contents coordinates to agenda grid coordinates.
*/
void KOAgenda::contentsToGrid (int x, int y, int& gx, int& gy)
{
  gx = QApplication::reverseLayout() ? mColumns - 1 - x/mGridSpacingX :
                                                        x/mGridSpacingX;
  gy = y/mGridSpacingY;
}

/*
  Convert agenda grid coordinates to scrollview contents coordinates.
*/
void KOAgenda::gridToContents (int gx, int gy, int& x, int& y)
{
  x = QApplication::reverseLayout() ? (mColumns - 1 - gx)*mGridSpacingX:
                                                         gx*mGridSpacingX;
  y = gy*mGridSpacingY;
}


/*
  Return Y coordinate corresponding to time. Coordinates are rounded to fit into
  the grid.
*/
int KOAgenda::timeToY(const QTime &time)
{
//  kdDebug() << "Time: " << time.toString() << endl;
  int minutesPerCell = 24 * 60 / mRows;
//  kdDebug() << "minutesPerCell: " << minutesPerCell << endl;
  int timeMinutes = time.hour() * 60 + time.minute();
//  kdDebug() << "timeMinutes: " << timeMinutes << endl;
  int Y = (timeMinutes + (minutesPerCell / 2)) / minutesPerCell;
//  kdDebug() << "y: " << Y << endl;
//  kdDebug() << "\n" << endl;
  return Y;
}


/*
  Return time corresponding to cell y coordinate. Coordinates are rounded to
  fit into the grid.
*/
QTime KOAgenda::gyToTime(int gy)
{
//  kdDebug() << "gyToTime: " << gy << endl;
  int secondsPerCell = 24 * 60 * 60/ mRows;

  int timeSeconds = secondsPerCell * gy;

  QTime time( 0, 0, 0 );
  if ( timeSeconds < 24 * 60 * 60 ) {
    time = time.addSecs(timeSeconds);
  } else {
    time.setHMS( 23, 59, 59 );
  }
//  kdDebug() << "  gyToTime: " << time.toString() << endl;

  return time;
}

void KOAgenda::setStartHour(int startHour)
{
  int startCell = startHour * mRows / 24;
  setContentsPos(0,startCell * gridSpacingY());
}


/*
  Insert KOAgendaItem into agenda.
*/
KOAgendaItem *KOAgenda::insertItem (Event *event,QDate qd,int X,int YTop,int YBottom)
{
//  kdDebug() << "KOAgenda::insertItem" << endl;

  if (mAllDayMode) {
    kdDebug() << "KOAgenda: calling insertItem in all-day mode is illegal." << endl;
    return 0;
  }

  KOAgendaItem *agendaItem = new KOAgendaItem (event,qd,viewport());
  agendaItem->setFrameStyle(WinPanel|Raised);

  int YSize = YBottom - YTop + 1;
  if (YSize < 0) {
    kdDebug() << "KOAgenda::insertItem(): Text: " << agendaItem->text() << " YSize<0" << endl;
    YSize = 1;
  }

  agendaItem->resize(mGridSpacingX,mGridSpacingY * YSize);
  agendaItem->setCellXY(X,YTop,YBottom);
  agendaItem->setCellXWidth(X);

  agendaItem->installEventFilter(this);

  addChild(agendaItem,X*mGridSpacingX,YTop*mGridSpacingY);
  mItems.append(agendaItem);

  placeSubCells(agendaItem);

  agendaItem->show();

  marcus_bains();

  return agendaItem;
}


/*
  Insert all-day KOAgendaItem into agenda.
*/
KOAgendaItem *KOAgenda::insertAllDayItem (Event *event,QDate qd,int XBegin,int XEnd)
{
   if (!mAllDayMode) {
    kdDebug() << "KOAgenda: calling insertAllDayItem in non all-day mode is illegal." << endl;
    return 0;
  }

  KOAgendaItem *agendaItem = new KOAgendaItem (event,qd,viewport());
  agendaItem->setFrameStyle(WinPanel|Raised);

  agendaItem->setCellXY(XBegin,0,0);
  agendaItem->setCellXWidth(XEnd);
  agendaItem->resize(mGridSpacingX * agendaItem->cellWidth(),mGridSpacingY);

  agendaItem->installEventFilter(this);

  addChild(agendaItem,XBegin*mGridSpacingX,0);
  mItems.append(agendaItem);

  placeSubCells(agendaItem);

  agendaItem->show();

  return agendaItem;
}


void KOAgenda::insertMultiItem (Event *event,QDate qd,int XBegin,int XEnd,
                                int YTop,int YBottom)
{
  if (mAllDayMode) {
    kdDebug() << "KOAgenda: calling insertMultiItem in all-day mode is illegal." << endl;
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

    current->setMultiItem(setFirst,next,setLast);
    current = next;
  }

  marcus_bains();
}


//QSizePolicy KOAgenda::sizePolicy() const
//{
  // Thought this would make the all-day event agenda minimum size and the
  // normal agenda take the remaining space. But it doesn�t work. The QSplitter
  // don�t seem to think that an Expanding widget needs more space than a
  // Preferred one.
  // But it doesn�t hurt, so it stays.
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
//  kdDebug() << "KOAgenda::resizeEvent" << endl;
  if (mAllDayMode) {
    mGridSpacingX = width() / mColumns;
//    kdDebug() << "Frame " << frameWidth() << endl;
    mGridSpacingY = height() - 2 * frameWidth() - 1;
    resizeContents( mGridSpacingX * mColumns + 1 , mGridSpacingY + 1);
//    mGridSpacingY = height();
//    resizeContents( mGridSpacingX * mColumns + 1 , mGridSpacingY * mRows + 1 );

    KOAgendaItem *item;
    int subCellWidth;
    for ( item=mItems.first(); item != 0; item=mItems.next() ) {
      subCellWidth = mGridSpacingY / item->subCells();
      item->resize(mGridSpacingX * item->cellWidth(),subCellWidth);
      moveChild(item,QApplication::reverseLayout() ?
                     (mColumns - 1 - item->cellX()) * mGridSpacingX :
                      item->cellX() * mGridSpacingX,
                      item->subCell() * subCellWidth);
    }
  } else {
    mGridSpacingX = (width() - verticalScrollBar()->width())/mColumns;
    resizeContents( mGridSpacingX * mColumns + 1 , mGridSpacingY * mRows + 1 );

    KOAgendaItem *item;
    int subCellWidth;
    for ( item=mItems.first(); item != 0; item=mItems.next() ) {
      subCellWidth = mGridSpacingX / item->subCells();
      item->resize(subCellWidth,item->height());
      moveChild(item,(QApplication::reverseLayout() ?
                     (mColumns - 1 - item->cellX()) * mGridSpacingX :
                     item->cellX() * mGridSpacingX) +
                     item->subCell() * subCellWidth,childY(item));
    }
  }

  checkScrollBoundaries();

  marcus_bains();

  viewport()->update();
  QScrollView::resizeEvent(ev);
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
    kdDebug() << "KOAgenda::itemPopup() called without having a clicked item" << endl;
    return;
  }
// TODO: deal correctly with multiple alarms
  Alarm* alarm;
  QPtrList<Alarm> list(mClickedItem->itemEvent()->alarms());
  for(alarm=list.first();alarm;alarm=list.next())
      alarm->toggleAlarm();

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
  viewport()->setBackgroundColor(KOPrefs::instance()->mAgendaBgColor);

  mGridSpacingY = KOPrefs::instance()->mHourSize;
  
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
  int yMin = v/mGridSpacingY;
  int yMax = (v+visibleHeight())/mGridSpacingY;

//  kdDebug() << "--- yMin: " << yMin << "  yMax: " << yMax << endl;

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
  if (mSelectedItem == 0) return;
  mSelectedItem->select(false);
  mSelectedItem = 0;
}

void KOAgenda::selectItem(KOAgendaItem *item)
{
  if (mSelectedItem == item) return;
  deselectItem();
  if (item == 0) {
    emit incidenceSelected( 0 );
    return;
  }
  mSelectedItem = item;
  mSelectedItem->select();
  emit incidenceSelected( mSelectedItem->itemEvent() );
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

  mWorkingHoursYTop = mGridSpacingY *
                      KOPrefs::instance()->mWorkingHoursStart * 4;
  mWorkingHoursYBottom = mGridSpacingY *
                         KOPrefs::instance()->mWorkingHoursEnd * 4 - 1;
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
  kdDebug() << "HolidayMask: ";
  for(uint i=0;i<mask->count();++i) {
    kdDebug() << (mask->at(i) ? "*" : "o");
  }
  kdDebug() << endl;
*/
}

void KOAgenda::contentsMousePressEvent ( QMouseEvent *event )
{
  kdDebug() << "KOagenda::contentsMousePressEvent(): type: " << event->type() << endl;
  QScrollView::contentsMousePressEvent(event);
}
