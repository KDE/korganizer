// $Id$

#include "koagenda.h"
#include "koagenda.moc"

#include "koagendaitem.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kconfig.h>

#include <qintdict.h>
#include <qdatetime.h>
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qcursor.h>

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
  mGridSpacingY = 40;
  mAllDayMode = true;

  init();
}


KOAgenda::~KOAgenda()
{
}


void KOAgenda::clear()
{
//  qDebug("KOAgenda::clear()");

  KOAgendaItem *item;
  for ( item=mItems.first(); item != 0; item=mItems.next() ) {
    removeChild(item);
  }
  mItems.clear();
}


void KOAgenda::changeColumns(int columns)
{
  if (columns == 0) {
    qDebug("KOAgenda::changeColumns() called with argument 0");
    return;
  }

  clear();
  mColumns = columns;
//  setMinimumSize(mColumns * 10, mGridSpacingY + 1);
//  init();
//  update();
  QApplication::sendEvent(this,new QResizeEvent(size(),size()));
}

void KOAgenda::init()
{
  mGridSpacingX = 100;

  mResizeBorderWidth = 8;
  mScrollBorderWidth = 8;
  mScrollDelay = 30;
  mScrollOffset = 10;

  enableClipper(true);

  QObject::connect(&mScrollUpTimer,SIGNAL(timeout()),SLOT(scrollUp()));
  QObject::connect(&mScrollDownTimer,SIGNAL(timeout()),SLOT(scrollDown()));

  mStartCellX = 0;
  mStartCellY = 0;
  mCurrentCellX = 0;
  mCurrentCellY = 0;
  
  mClickedItem = 0;
  
  mActionItem = 0;
  mActionType = NOP;
  mItemMoved = false;

  mItems.setAutoDelete(true);
  
  resizeContents( mGridSpacingX * mColumns + 1 , mGridSpacingY * mRows + 1 );

  viewport()->update();

  setMinimumSize(30, mGridSpacingY + 1);
//  setMaximumHeight(mGridSpacingY * mRows + 5);

  // Disable horizontal scrollbar. This is a hack. The geometry should be
  // controlled in a way that the contents horizontally always fits. Then it is
  // not necessary to turn off the scrollbar.
  setHScrollBarMode(AlwaysOff);

  // item popup menu
  mItemPopup = new QPopupMenu();
  mItemPopup->insertItem (i18n("&Show"),this,SLOT(popupShow()));
  mItemPopup->insertItem (i18n("&Edit"),this, SLOT(popupEdit()));
  mItemPopup->insertItem (QIconSet(BarIcon("delete")),i18n("&Delete"),
                          this, SLOT(popupDelete()));
  mItemPopup->insertItem (QIconSet(UserIcon("bell")),i18n("ToggleAlarm"),
                          this, SLOT(popupAlarm()));
}

/*
  This is the eventFilter function, which gets all events from the KOAgendaItems
  contained in the agenda. It has to handle moving and resizing for all items.
*/
bool KOAgenda::eventFilter ( QObject *object, QEvent *event )
{
//  qDebug("KOAgenda::eventFilter");

  QMouseEvent *me;
  QPoint viewportPos;

  switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
      me = (QMouseEvent *)event;
      if (object != viewport()) {
        viewportPos = ((QWidget *)object)->mapToParent(me->pos());
      } else {
        viewportPos = me->pos();
      }

      if (me->type() == QEvent::MouseButtonPress) {
//        qDebug("koagenda: filtered button press");
      	if (object != viewport()) {
          if (me->button() == RightButton) {
            mClickedItem = (KOAgendaItem *)object;
            mItemPopup->popup(QCursor::pos());
          } else {
            mActionItem = (KOAgendaItem *)object;
            startItemAction(viewportPos);
          }
        } else {
          mActionItem = 0;
	  mActionType = NOP;
          setCursor(arrowCursor);
	}
      } else if (me->type() == QEvent::MouseButtonRelease) {
        if (mActionItem) {
	  endItemAction();
	}
      } else if (me->type() == QEvent::MouseMove) {
	KOAgendaItem *moveItem = (KOAgendaItem *)object;
        if (!mActionItem) {
      	  setNoActionCursor(moveItem,viewportPos);
	} else {
      	  performItemAction(viewportPos);
	}
      } else if (me->type() == QEvent::MouseButtonDblClick) {
        if (object == viewport()) {
          int x,y;
          viewportToContents(viewportPos.x(),viewportPos.y(),x,y);
          int gx,gy;
          contentsToGrid(x,y,gx,gy);
	  emit newEventSignal(gx,gy);
        } else {
	  emit editEventSignal(((KOAgendaItem *)object)->itemEvent());
	}
      }
      break;
    
    case (QEvent::Leave):
      if (!mActionItem)
        setCursor(arrowCursor);
      break;

    default:
      break;
  }

  return QScrollView::eventFilter(object,event);
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
      setCursor(sizeAllCursor);
    }
  }
}

void KOAgenda::performItemAction(QPoint viewportPos)
{
//  qDebug("viewportPos: %d,%d",viewportPos.x(),viewportPos.y());
//  QPoint point = viewport()->mapToGlobal(viewportPos);
//  qDebug("Global: %d,%d",point.x(),point.y());
//  point = clipper()->mapFromGlobal(point);
//  qDebug("clipper: %d,%d",point.x(),point.y());
//  qDebug("visible height: %d",visibleHeight());
  int x,y;
  viewportToContents(viewportPos.x(),viewportPos.y(),x,y);
//  qDebug("contents: %d,%d\n",x,y);
  int gx,gy;
  contentsToGrid(x,y,gx,gy);
  QPoint clipperPos = clipper()->
                      mapFromGlobal(viewport()->mapToGlobal(viewportPos));

  // Cursor left active agenda area.
  // This should trigger drag/drop mode, at the moment is only sets the
  // cursor.
  if (clipperPos.y() < 0 || clipperPos.y() > visibleHeight() ||
      clipperPos.x() < 0 || clipperPos.x() > visibleWidth()) {
    setCursor(waitCursor);
    mScrollUpTimer.stop();
    mScrollDownTimer.stop();
    return;
  } else {
    switch (mActionType) {
      case MOVE:
        setCursor(sizeAllCursor);
        break;
      case RESIZETOP:
      case RESIZEBOTTOM:
        setCursor(sizeVerCursor);
	break;
      case RESIZELEFT:
      case RESIZERIGHT:
        setCursor(sizeHorCursor);
        break;
      default:
        setCursor(arrowCursor);
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
  if (mItemMoved) {
    KOAgendaItem *placeItem = mActionItem->firstMultiItem();
    if (!placeItem) placeItem = mActionItem;
    emit itemModified(placeItem);
    while (placeItem) {
      placeSubCells(placeItem);
      placeItem = placeItem->nextMultiItem();
    }
  }
  mScrollUpTimer.stop();
  mScrollDownTimer.stop();
  setCursor(arrowCursor);
  mActionItem = 0;
  mActionType = NOP;
  mItemMoved = 0;
}

void KOAgenda::setNoActionCursor(KOAgendaItem *moveItem,QPoint viewportPos)
{
//  qDebug("viewportPos: %d,%d",viewportPos.x(),viewportPos.y());
//  QPoint point = viewport()->mapToGlobal(viewportPos);
//  qDebug("Global: %d,%d",point.x(),point.y());
//  point = clipper()->mapFromGlobal(point);
//  qDebug("clipper: %d,%d",point.x(),point.y());

  int x,y;
  viewportToContents(viewportPos.x(),viewportPos.y(),x,y);
//  qDebug("contents: %d,%d\n",x,y);
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
  not overlap. This method is not yet optimal. It doesn´t use the maximum space
  it can get in all cases.
  At the moment the method has a bug: When an item is placed only the sub cell
  widths of the items are changed, which are within the Y region the item to
  place spans. When the sub cell width change of one of this items affects a
  cell, where other items are, which do not overlap in Y with the item to place,
  the display gets corrupted, although the corruption looks quite nice.
*/
void KOAgenda::placeSubCells(KOAgendaItem *placeItem)
{
  QList<KOAgendaItem> conflictItems;
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
    
    // Adjust sub cell geometry of all items
    for ( item=conflictItems.first(); item != 0;
          item=conflictItems.next() ) {
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
}

/*
  Draw grid in the background of the agenda.
*/
void KOAgenda::drawContents(QPainter* p, int cx, int cy, int cw, int ch)
{
//  qDebug("drawContents cx: %d cy: %d cw: %d ch: %d",cx,cy,cw,ch);
  int x = ((int)(cx/mGridSpacingX))*mGridSpacingX;
  while (x < cx + cw) {
    p->drawLine(x,cy,x,cy+ch);
    x+=mGridSpacingX;
  }  

  int y = ((int)(cy/mGridSpacingY))*mGridSpacingY;
  while (y < cy + ch) {
//    qDebug(" y: %d",y);
    p->drawLine(cx,y,cx+cw,y);
    y+=mGridSpacingY;
  }
}

/*
  Convert srcollview contents coordinates to agenda grid coordinates.
*/ 
void KOAgenda::contentsToGrid (int x, int y, int& gx, int& gy)
{
  gx = x/mGridSpacingX;
  gy = y/mGridSpacingY;
}

/*
  Convert agenda grid coordinates to scrollview contents coordinates.
*/
void KOAgenda::gridToContents (int gx, int gy, int& x, int& y)
{
  x = gx*mGridSpacingX;
  y = gy*mGridSpacingY;
}


/*
  Return Y coordinate corresponding to time. Coordinates are rounded to fit into
  the grid.
*/  
int KOAgenda::timeToY(const QTime &time)
{
//  qDebug("Time: %s",time.toString().latin1());
  int minutesPerCell = 24 * 60 / mRows;  
//  qDebug("minutesPerCell: %d",minutesPerCell);
  int timeMinutes = time.hour() * 60 + time.minute();
//  qDebug("timeMinutes: %d",timeMinutes);  
  int Y = (timeMinutes + (minutesPerCell / 2)) / minutesPerCell;
//  qDebug("y: %d",Y);
//  qDebug("\n");  
  return Y;
}


/*
  Return time corresponding to cell y coordinate. Coordinates are rounded to
  fit into the grid.
*/  
QTime KOAgenda::gyToTime(int gy)
{
//  qDebug("gyToTime: %d",gy);
  int secondsPerCell = 24 * 60 * 60/ mRows;  
  
  int timeSeconds = secondsPerCell * gy;

  QTime time;
  time = time.addSecs(timeSeconds);
//  qDebug("  gyToTime: %s",time.toString().latin1());

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
KOAgendaItem *KOAgenda::insertItem (KOEvent *event,int X,int YTop,int YBottom)
{
//  qDebug("KOAgenda::insertItem");

  if (mAllDayMode) {
    qDebug("KOAgenda: calling insertItem in all-day mode is illegal.");
    return 0;
  }
  
  KOAgendaItem *agendaItem = new KOAgendaItem (event,viewport());
  agendaItem->setFrameStyle(WinPanel|Raised);

  int YSize = YBottom - YTop + 1;
  if (YSize < 0) {
    qDebug ("KOAgenda::insertItem(): Text: %s YSize<0",
            agendaItem->text().latin1());
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
  
  return agendaItem;
}


/*
  Insert all-day KOAgendaItem into agenda.
*/
KOAgendaItem *KOAgenda::insertAllDayItem (KOEvent *event,int XBegin,int XEnd)
{
   if (!mAllDayMode) {
    qDebug("KOAgenda: calling insertAllDayItem in non all-day mode is illegal.");
    return 0;
  }

  KOAgendaItem *agendaItem = new KOAgendaItem (event,viewport());
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


void KOAgenda::insertMultiItem (KOEvent *event,int XBegin,int XEnd,
                                int YTop,int YBottom)
{
  if (mAllDayMode) {
    qDebug("KOAgenda: calling insertMultiItem in all-day mode is illegal.");
    return;
  }
  
  int cellX,cellYTop,cellYBottom;
  QString newtext;
  int width = XEnd - XBegin + 1;
  int count = 0;
  KOAgendaItem *current = 0;
  QList<KOAgendaItem> multiItems;
  for (cellX = XBegin;cellX <= XEnd;++cellX) {
    if (cellX == XBegin) cellYTop = YTop;
    else cellYTop = 0;
    if (cellX == XEnd) cellYBottom = YBottom;
    else cellYBottom = rows() - 1;
    newtext = QString("(%1/%2): ").arg(++count).arg(width);
    newtext.append(event->getSummary());
    current = insertItem(event,cellX,cellYTop,cellYBottom);
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
//  qDebug("KOAgenda::resizeEvent");
  if (mAllDayMode) {
    mGridSpacingX = width() / mColumns;
//    qDebug("Frame %d",frameWidth());
    mGridSpacingY = height() - 2 * frameWidth() - 1;
    resizeContents( mGridSpacingX * mColumns + 1 , mGridSpacingY + 1);
//    mGridSpacingY = height();
//    resizeContents( mGridSpacingX * mColumns + 1 , mGridSpacingY * mRows + 1 );

    KOAgendaItem *item;
    int subCellWidth;
    for ( item=mItems.first(); item != 0; item=mItems.next() ) {
      subCellWidth = mGridSpacingY / item->subCells();
      item->resize(mGridSpacingX * item->cellWidth(),subCellWidth);
      moveChild(item,item->cellX() * mGridSpacingX,
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
      moveChild(item,item->cellX() * mGridSpacingX +
                item->subCell() * subCellWidth,childY(item));
    }
  }
  
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

void KOAgenda::popupShow()
{
  if (!mClickedItem) {
    qDebug("KOAgenda::itemPopup() called without having a clicked item");
    return;
  }
  
  emit showEventSignal(mClickedItem->itemEvent());
}

void KOAgenda::popupEdit()
{
  if (!mClickedItem) {
    qDebug("KOAgenda::itemPopup() called without having a clicked item");
    return;
  }
  
  emit editEventSignal(mClickedItem->itemEvent());
}

void KOAgenda::popupDelete()
{
  if (!mClickedItem) {
    qDebug("KOAgenda::itemPopup() called without having a clicked item");
    return;
  }

  emit deleteEventSignal(mClickedItem->itemEvent());
}

void KOAgenda::popupAlarm()
{
  if (!mClickedItem) {
    qDebug("KOAgenda::itemPopup() called without having a clicked item");
    return;
  }

  mClickedItem->itemEvent()->toggleAlarm();
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

/**  */
void KOAgenda::updateConfig(KConfig* config)
{
  // set start hour
  config->setGroup("Views");
  QString startStr(config->readEntry("Day Begins", "8:00"));
  //TODO: handle case where old config files are in format "8" instead of "8:00".
  int colonPos = startStr.find(':');
  if (colonPos >= 0)
    startStr.truncate(colonPos);
  int mStartHour = startStr.toUInt();
  setStartHour(mStartHour);
}
