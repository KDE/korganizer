// $Id$

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qframe.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qpopupmenu.h>
#include <qtooltip.h>

#include <kapp.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include <klocale.h>

#include "koprefs.h"
#include "koagenda.h"
#include "koagendaitem.h"
#include "calobject.h"
#include "calprinter.h"
#include "vcaldrag.h"

#include "koagendaview.h"
#include "koagendaview.moc"

TimeLabels::TimeLabels(int rows,QWidget *parent,const char *name,WFlags f) :
  QScrollView(parent,name,f)
{
  mRows = rows;

  mCellHeight = 40;

  enableClipper(true);

  setHScrollBarMode(AlwaysOff);
  setVScrollBarMode(AlwaysOff);

  resizeContents(50,mRows * mCellHeight);
}

void TimeLabels::setCellHeight(int height)
{
  mCellHeight = height;
}

/*
  Optimization so that only the "dirty" portion of the scroll view
  is redrawn.  Unfortunately, this is not called by default paintEvent() method.
*/
void TimeLabels::drawContents(QPainter *p,int cx, int cy, int cw, int ch)
{
  // bug:  the parameters cx, cy, cw, ch are the areas that need to be
  //       redrawn, not the area of the widget.  unfortunately, this
  //       code assumes the latter...

  // now, for a workaround...
  // these two assignments fix the weird redraw bug
  cx = contentsX() + 2;
  cw = contentsWidth() - 2;
  // end of workaround

  int cell = ((int)(cy/mCellHeight));
  int y = cell * mCellHeight;
  QFontMetrics fm = fontMetrics();
  QString hour;
  QString suffix;
  QString fullTime;

  while (y < cy + ch) {
    p->drawLine(cx,y,cx+cw,y);
    hour.setNum(cell);
    suffix = "am";

    // handle 24h and am/pm time formats
    if (KGlobal::locale()->use12Clock()) {
      if (cell > 11) suffix = "pm";
      if (cell == 0) hour.setNum(12);
      if (cell > 12) hour.setNum(cell - 12);
    } else {
      suffix = ":00";
    }

    // create string in format of "XX:XX" or "XXpm/am"
    fullTime = hour + suffix;

    // center and draw the time label
    int timeWidth = fm.width(fullTime);
    int offset = this->width() - timeWidth;
    int borderWidth = 5;
    p->drawText(cx -borderWidth + offset, y+15, fullTime);

    // increment indices
    y += mCellHeight;
    cell++;
  }
}

/**
   Calculates the minimum width.
*/
int TimeLabels::minimumWidth() const
{
  QFontMetrics fm = fontMetrics();

  //TODO: calculate this value
  int borderWidth = 8;

  // the maximum width possible
  int width = fm.width("88:88") + borderWidth;

  return width;
}

/** updates widget's internal state */
void TimeLabels::updateConfig()
{
  // set the font
//  config->setGroup("Fonts");
//  QFont font = config->readFontEntry("TimeBar Font");
  setFont(KOPrefs::instance()->mTimeBarFont);

  // update geometry restrictions based on new settings
  setFixedWidth(minimumWidth());
}

/** update time label positions */
void TimeLabels::positionChanged()
{
  int adjustment = mAgenda->contentsY();
  setContentsPos(0, adjustment);
}

/**  */
void TimeLabels::setAgenda(KOAgenda* agenda)
{
  mAgenda = agenda;
}


/** This is called in response to repaint() */
void TimeLabels::paintEvent(QPaintEvent*)
{
//  debug("paintevent...");
  // this is another hack!
//  QPainter painter(this);
  //QString c
  repaintContents(contentsX(), contentsY(), visibleWidth(), visibleHeight());
}

////////////////////////////////////////////////////////////////////////////

EventIndicator::EventIndicator(Location loc,QWidget *parent,const char *name)
  : QFrame(parent,name)
{
  mColumns = 1;
  mTopBox = 0;
  mLocation = loc;
  mTopLayout = 0;

  if (mLocation == Top) mPixmap = UserIcon("1uparrow");
  else mPixmap = UserIcon("1downarrow");

  setMinimumHeight(mPixmap.height());
}

EventIndicator::~EventIndicator()
{
}

void EventIndicator::drawContents(QPainter *p)
{
//  qDebug("======== top: %d  bottom %d  left %d  right %d",contentsRect().top(),
//         contentsRect().bottom(),contentsRect().left(),contentsRect().right());

  int i;
  for(i=0;i<mColumns;++i) {
    if (mEnabled[i]) {
      int cellWidth = contentsRect().right()/mColumns;
      int xOffset = i*cellWidth + cellWidth/2 -mPixmap.width()/2;
      p->drawPixmap(QPoint(xOffset,0),mPixmap);
    }
  }
}

void EventIndicator::changeColumns(int columns)
{
  mColumns = columns;
  mEnabled.resize(mColumns);

  update();
}

void EventIndicator::enableColumn(int column, bool enable)
{
  mEnabled[column] = enable;
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

KOAgendaView::KOAgendaView(CalObject *cal,QWidget *parent,const char *name) :
  KOBaseView (cal,parent,name)
{
  mStartDate = QDate::currentDate();
  mStartHour = 8;
                         
  mLayoutDayLabels = 0;
  mDayLabelsFrame = 0;
  mDayLabels = 0;
  
  // Create agenda splitter
  QSplitter *splitterAgenda = new QSplitter(Vertical,this);
  splitterAgenda->setOpaqueResize();

  // Create all-day agenda widget
  mAllDayFrame = new QHBox(splitterAgenda);
  mDummyAllDayLeft = new QWidget(mAllDayFrame);
  mAllDayAgenda = new KOAgenda(1,mAllDayFrame);
  QWidget *dummyAllDayRight = new QWidget(mAllDayFrame);

  // Create event context menu for all day agenda
  mAllDayAgendaPopup = eventPopup();
  connect(mAllDayAgenda,SIGNAL(showEventPopupSignal(KOEvent *)),
          SLOT(showAllDayAgendaPopup(KOEvent *)));

  // Create agenda frame
  QWidget *agendaFrame = new QWidget(splitterAgenda);
  QGridLayout *agendaLayout = new QGridLayout(agendaFrame,3,3);
//  QHBox *agendaFrame = new QHBox(splitterAgenda);

  // create event indicator bars
  mEventIndicatorTop = new EventIndicator(EventIndicator::Top,agendaFrame);
  agendaLayout->addWidget(mEventIndicatorTop,0,1);
  mEventIndicatorBottom = new EventIndicator(EventIndicator::Bottom,
                                             agendaFrame);
  agendaLayout->addWidget(mEventIndicatorBottom,2,1);
  QWidget *dummyAgendaRight = new QWidget(agendaFrame);
  agendaLayout->addWidget(dummyAgendaRight,0,2);

  // Create time labels
  mTimeLabels = new TimeLabels(24,agendaFrame);
  agendaLayout->addWidget(mTimeLabels,1,0);

  // Create agenda
  mAgenda = new KOAgenda(1,48,20,agendaFrame);
  agendaLayout->addMultiCellWidget(mAgenda,1,1,1,2);
  agendaLayout->setColStretch(1,1);

  // Create event context menu for agenda
  mAgendaPopup = eventPopup();
  mAgendaPopup->insertSeparator();
  mAgendaPopup->insertItem(QIconSet(UserIcon("bell")),i18n("ToggleAlarm"),
                           mAgenda,SLOT(popupAlarm()));
  connect(mAgenda,SIGNAL(showEventPopupSignal(KOEvent *)),
          SLOT(showAgendaPopup(KOEvent *)));

  // Create day name labels for agenda columns
  mDayLabelsFrame = new QHBox(this);
  createDayLabels();

  // make connections between dependent widgets
  mTimeLabels->setAgenda(mAgenda);

  // Update widgets to reflect user preferences
//  updateConfig();

  // these blank widgets make the All Day Event box line up with the agenda
  dummyAllDayRight->setFixedWidth(mAgenda->verticalScrollBar()->width());
  dummyAgendaRight->setFixedWidth(mAgenda->verticalScrollBar()->width());
  mDummyAllDayLeft->setFixedWidth(mTimeLabels->width());

  QBoxLayout *layoutTop = new QVBoxLayout(this);
  layoutTop->addWidget(mDayLabelsFrame);
  layoutTop->addWidget(splitterAgenda);

  //connect signals and slots
  connect(mAgenda->verticalScrollBar(),SIGNAL(valueChanged(int)),
          mTimeLabels, SLOT(positionChanged()));
  connect(mAgenda,SIGNAL(newEventSignal(int,int)),
                  SLOT(newEvent(int,int)));
  connect(mAllDayAgenda,SIGNAL(newEventSignal(int,int)),
                        SLOT(newEventAllDay(int,int)));
  connect(mAgenda,SIGNAL(editEventSignal(KOEvent *)),
                  SIGNAL(editEventSignal(KOEvent *)));
  connect(mAllDayAgenda,SIGNAL(editEventSignal(KOEvent *)),
                        SIGNAL(editEventSignal(KOEvent *)));
  connect(mAgenda,SIGNAL(showEventSignal(KOEvent *)),
                  SIGNAL(showEventSignal(KOEvent *)));
  connect(mAllDayAgenda,SIGNAL(showEventSignal(KOEvent *)),
                        SIGNAL(showEventSignal(KOEvent *)));
  connect(mAgenda,SIGNAL(deleteEventSignal(KOEvent *)),
                  SIGNAL(deleteEventSignal(KOEvent *)));
  connect(mAllDayAgenda,SIGNAL(deleteEventSignal(KOEvent *)),
                        SIGNAL(deleteEventSignal(KOEvent *)));
  connect(mAgenda,SIGNAL(itemModified(KOAgendaItem *)),
                  SLOT(updateEventDates(KOAgendaItem *)));
  connect(mAllDayAgenda,SIGNAL(itemModified(KOAgendaItem *)),
                        SLOT(updateEventDates(KOAgendaItem *)));

  // event indicator update
  connect(mAgenda,SIGNAL(lowerYChanged(int)),
          SLOT(updateEventIndicatorTop(int)));
  connect(mAgenda,SIGNAL(upperYChanged(int)),
          SLOT(updateEventIndicatorBottom(int)));

  // drag signals
  connect(mAgenda,SIGNAL(startDragSignal(KOEvent *)),
          SLOT(startDrag(KOEvent *)));
  connect(mAllDayAgenda,SIGNAL(startDragSignal(KOEvent *)),
          SLOT(startDrag(KOEvent *)));
}


KOAgendaView::~KOAgendaView()
{
}

void KOAgendaView::createDayLabels()
{
  delete mDayLabels;

  mDayLabels = new QFrame (mDayLabelsFrame);
  mLayoutDayLabels = new QHBoxLayout(mDayLabels);
  mLayoutDayLabels->addSpacing(mTimeLabels->width());
  
  QLabel *dayLabel;
  unsigned int i;
  QDate date;
  for(i=0;i<mSelectedDates.count();++i) {
    date = mStartDate.addDays(i);
    dayLabel = new QLabel(mDayLabels);
    dayLabel->setText(QString("%1 %2").arg(KGlobal::locale()->weekDayName(date.dayOfWeek()))
                                      .arg(date.day()));
    dayLabel->setAlignment(QLabel::AlignHCenter);
    mLayoutDayLabels->addWidget(dayLabel,1);
  }
  
  mLayoutDayLabels->addSpacing(mAgenda->verticalScrollBar()->width());
//  mDayLabels->updateGeometry();
//  mDayLabelsFrame->updateGeometry();
//  mDayLabelsFrame->show();
  mDayLabels->show();
}

int KOAgendaView::maxDatesHint()
{
  // Not sure about the max number of events, so return 0 for now.
  return 0;
}

int KOAgendaView::currentDateCount()
{
  return mSelectedDates.count();
}

QList<KOEvent> KOAgendaView::getSelected()
{
  QList<KOEvent> selectedEvents;
  
  return selectedEvents;
}


void KOAgendaView::updateView()
{
//  qDebug("KOAgendaView::updateView()");
  fillAgenda();
}


/*
  Update configuration settings for the agenda view. This method is not
  complete.
*/
void KOAgendaView::updateConfig()
{
//  qDebug("KOAgendaView::updateConfig()");

  // update config for children
  mTimeLabels->updateConfig();
  mAgenda->updateConfig();
  mAllDayAgenda->updateConfig();

  // widget synchronization
  //TODO: find a better way, maybe signal/slot
  mTimeLabels->positionChanged();

  // for some reason, this needs to be called explicitly
  mTimeLabels->repaint();

  mDummyAllDayLeft->setFixedWidth(mTimeLabels->width());

  // ToolTips displaying summary of events
  KOAgendaItem::toolTipGroup()->setEnabled(KOPrefs::instance()
                                           ->mEnableToolTips);

  updateView();
}


void KOAgendaView::updateEventDates(KOAgendaItem *item)
{
//  qDebug("updateEventDates %s",item->text().latin1());
  QDateTime startDt,endDt;
  QDate startDate;

  if (item->cellX() < 0) {
    startDate = (*mSelectedDates.first()).addDays(item->cellX());
  } else {
    startDate = *mSelectedDates.at(item->cellX());
  }
  startDt.setDate(startDate);
  
  if (item->itemEvent()->doesFloat()) {
    endDt.setDate(startDate.addDays(item->cellWidth() - 1));
  } else {
    startDt.setTime(mAgenda->gyToTime(item->cellYTop()));
    if (item->lastMultiItem()) {
      endDt.setTime(mAgenda->gyToTime(item->lastMultiItem()->cellYBottom()+1));
      endDt.setDate(startDate.
                    addDays(item->lastMultiItem()->cellX() - item->cellX()));
    } else {
      endDt.setTime(mAgenda->gyToTime(item->cellYBottom()+1));
      endDt.setDate(startDate);
    }
  }
  
  item->itemEvent()->setDtStart(startDt);
  item->itemEvent()->setDtEnd(endDt);
}


void KOAgendaView::selectDates(const QDateList list)
{
//  qDebug("KOAgendaView::selectDates");
  
  mSelectedDates.clear();
  mSelectedDates = list;
  mStartDate = *mSelectedDates.first();

  // if there are 5 dates and the first is a monday, we have a workweek.
  if ((mSelectedDates.count() == 5) &&
      (mSelectedDates.first()->dayOfWeek() == 1) &&
      (mSelectedDates.first()->daysTo(*mSelectedDates.last()) == 4)) {
    setView(WORKWEEK);
    
  // if there are 7 dates and the first is a monday, we have a regular week.
  } else if ((mSelectedDates.count() == 7) &&
             (mSelectedDates.first()->dayOfWeek() ==
	     (KGlobal::locale()->weekStartsMonday() ? 1 : 7)) &&
             (mSelectedDates.first()->daysTo(*mSelectedDates.last()) == 6)) {
    setView(WEEK);

  } else if (mSelectedDates.count() == 1) {
    setView(DAY);

  } else {
    // for sanity, set viewtype to LIST for now...
    setView(LIST);
  }

  // and update the view
  fillAgenda();
}


void KOAgendaView::selectEvents(QList<KOEvent>)
{
  qDebug("KOAgendaView::selectEvents() is not yet implemented");
}

void KOAgendaView::setView(int view)
{
  // change view type if valid
  if( mSelectedDates.first() ) {
    if ((view >= DAY) && (view <= LIST))
      mViewType = view;
    else 
      mViewType = DAY;
  } else
    mViewType = DAY;
}

void KOAgendaView::changeEventDisplay(KOEvent *, int)
{
//  qDebug("KOAgendaView::changeEventDisplay");
  // this should be re-written to be MUCH smarter.  Right now we
  // are just playing dumb.
  fillAgenda();
}

void KOAgendaView::slotPrevDates()
{
  int datenum, count;

  switch( mViewType ) {
  case DAY:
    mStartDate = mStartDate.addDays(-1);
    mSelectedDates.clear();
    mSelectedDates.append(new QDate(mStartDate));
    break;
  case WORKWEEK:
    mStartDate = mStartDate.addDays(-7);
    mSelectedDates.clear();
    for (count = 0; count < 5; count++)
      mSelectedDates.append(new QDate(mStartDate.addDays(count)));
    break;
  case WEEK:
    mStartDate = mStartDate.addDays(-7);
    mSelectedDates.clear();
    for( count = 0; count < 7; count++ )
      mSelectedDates.append(new QDate(mStartDate.addDays(count)));
    break;
  case LIST:
    datenum = mSelectedDates.count();
    mStartDate = mStartDate.addDays( -datenum );
    mSelectedDates.clear();
    for (count = 0; count < datenum; count++)
      mSelectedDates.append(new QDate(mStartDate.addDays(count)));
    break;
  }
  emit datesSelected(mSelectedDates);
  setView(mViewType);
  fillAgenda();
}

void KOAgendaView::slotNextDates()
{
  int datenum, count;

  switch(mViewType) {
  case DAY:
    mStartDate = mStartDate.addDays(1);
    mSelectedDates.clear();
    mSelectedDates.append(new QDate(mStartDate));
    break;
  case WORKWEEK:
    mStartDate = mStartDate.addDays(7);
    mSelectedDates.clear();
    for (count = 0; count < 5; count++)
      mSelectedDates.append(new QDate(mStartDate.addDays(count)));
    break;
  case WEEK:
    mStartDate = mStartDate.addDays(7);
    mSelectedDates.clear();
    for( count = 0; count < 7; count++ )
      mSelectedDates.append(new QDate(mStartDate.addDays(count)));
    break;
  case LIST:
    datenum = mSelectedDates.count();
    mStartDate = mSelectedDates.last()->addDays(1);
    mSelectedDates.clear();
    for (count = 0; count < datenum; count++)
      mSelectedDates.append(new QDate(mStartDate.addDays(count)));
    break;
  }
  emit datesSelected(mSelectedDates);
  setView(mViewType);
  fillAgenda();
}


void KOAgendaView::slotViewChange()
{
  slotViewChange(mViewType + 1 != LIST ? mViewType + 1 : DAY);
}


void KOAgendaView::slotViewChange(int newView)
{
//  qDebug("KOAgendaView::slotViewChange(): %d",newView);

  int datenum, count;

  switch (newView) {
  case DAY:
    mSelectedDates.clear();
    mSelectedDates.append(new QDate(mStartDate));

    break;

  case WORKWEEK:
    // find monday for this week
    if (mStartDate.dayOfWeek() == 7) {
      if (KGlobal::locale()->weekStartsMonday())
        mStartDate = mStartDate.addDays(-6);
      else
        mStartDate = mStartDate.addDays(1);
    } else if (mStartDate.dayOfWeek() > 1) {
      mStartDate = mStartDate.addDays(mStartDate.dayOfWeek() * -1 + 1);
    }

    mSelectedDates.clear();
    for (count = 0; count < 5; count++)
      mSelectedDates.append(new QDate(mStartDate.addDays(count)));

    break;

  case WEEK:
    // find the beginning of this week (could be monday or sunday)
    if (mStartDate.dayOfWeek() == 7) {
      if (KGlobal::locale()->weekStartsMonday())
        mStartDate = mStartDate.addDays(-6);
    } else if (KGlobal::locale()->weekStartsMonday()) {
      mStartDate = mStartDate.addDays(mStartDate.dayOfWeek() * -1 + 1);
    } else {
      mStartDate = mStartDate.addDays(mStartDate.dayOfWeek() * -1);
    }

    mSelectedDates.clear();
    for( count = 0; count < 7; count++ )
      mSelectedDates.append(new QDate(mStartDate.addDays(count)));

    break;

  case LIST:
    datenum = mSelectedDates.count();
    mSelectedDates.clear();
    for (count = 0; count < datenum; count++)
      mSelectedDates.append(new QDate(mStartDate.addDays(count)));

    break;

  default:
    mSelectedDates.clear();
    mSelectedDates.append(new QDate(QDate::currentDate()));
    newView = DAY;
    break;
  }

  emit datesSelected(mSelectedDates);
  setView(newView);
  fillAgenda();
}

//void KOAgendaView::adjustTimeLabels()
//{
  //mTimeLabels->setContentsPos(0,mAgenda->contentsY());
//}


void KOAgendaView::fillAgenda(const QDate &startDate)
{
  mStartDate = startDate;
  fillAgenda();
}


void KOAgendaView::fillAgenda()
{
//  qDebug("Fill Agenda beginning with date %s",mStartDate.toString().latin1());
//  qDebug(" number of dates: %d",mSelectedDates.count());

//  clearView();

  mAllDayAgenda->changeColumns(mSelectedDates.count());
  mAgenda->changeColumns(mSelectedDates.count());
  mEventIndicatorTop->changeColumns(mSelectedDates.count());
  mEventIndicatorBottom->changeColumns(mSelectedDates.count());

  createDayLabels();

  mMinY.resize(mSelectedDates.count());
  mMaxY.resize(mSelectedDates.count());

  QList<KOEvent> dayEvents;
  int curCol;  // current column of agenda, i.e. the X coordinate
  QDate currentDate = mStartDate;
  for(curCol=0;curCol<int(mSelectedDates.count());++curCol) {
    dayEvents = mCalendar->getEventsForDate(currentDate,false);

    // Default values, which can never be reached
    mMinY[curCol] = mAgenda->timeToY(QTime(23,59)) + 1;
    mMaxY[curCol] = mAgenda->timeToY(QTime(0,0)) - 1;

    unsigned int numEvent;
    for(numEvent=0;numEvent<dayEvents.count();++numEvent) {
      KOEvent *event = dayEvents.at(numEvent);
//      qDebug(" Event: %s",event->getSummary().latin1());

      int beginX = currentDate.daysTo(event->getDtStart().date()) + curCol;
      int endX = currentDate.daysTo(event->getDtEnd().date()) + curCol;

//      qDebug("  beginX: %d  endX: %d",beginX,endX);      
      
      if (event->doesFloat()) {
      	if (beginX <= 0 && curCol == 0) {     
          mAllDayAgenda->insertAllDayItem(event,beginX,endX);
	} else if (beginX == curCol) {
          mAllDayAgenda->insertAllDayItem(event,beginX,endX);
	}
      } else if (event->isMultiDay()) {
        if ((beginX <= 0 && curCol == 0) || beginX == curCol) {
          int startY = mAgenda->timeToY(event->getDtStart().time());
          int endY = mAgenda->timeToY(event->getDtEnd().time()) - 1;  
          mAgenda->insertMultiItem(event,beginX,endX,startY,endY);
          if (startY < mMinY[curCol]) mMinY[curCol] = startY;
          if (endY > mMaxY[curCol]) mMaxY[curCol] = endY;
        }
      } else {
        int startY = mAgenda->timeToY(event->getDtStart().time());
        int endY = mAgenda->timeToY(event->getDtEnd().time()) - 1;
	if (endY < startY) endY = startY;
	mAgenda->insertItem(event,curCol,startY,endY);
        if (startY < mMinY[curCol]) mMinY[curCol] = startY;
        if (endY > mMaxY[curCol]) mMaxY[curCol] = endY;
      }
    }
//    if (numEvent == 0) qDebug(" No events");
    
    currentDate = currentDate.addDays(1);
  }

  mAgenda->checkScrollBoundaries();

//  mAgenda->viewport()->update();
//  mAllDayAgenda->viewport()->update();

//  qDebug("Fill Agenda done");
}

void KOAgendaView::clearView()
{
//  qDebug("ClearView");
  mAllDayAgenda->clear();
  mAgenda->clear();
}

void KOAgendaView::printPreview(CalPrinter *calPrinter, const QDate &fd,
                               const QDate &td)
{
  if (fd == td)
    calPrinter->preview(CalPrinter::Day, fd, td);
  else
    calPrinter->preview(CalPrinter::Week, fd, td);
}


void KOAgendaView::newEvent(int gx, int gy)
{
  QDate day = mStartDate.addDays(gx);
  QTime time = mAgenda->gyToTime(gy);
  QDateTime dt(day,time);
  emit newEventSignal(dt);
}

void KOAgendaView::newEventAllDay(int gx, int )
{
  emit newEventSignal(mStartDate.addDays(gx));
}

void KOAgendaView::showAgendaPopup(KOEvent *event)
{
  showEventPopup(mAgendaPopup,event);
}

void KOAgendaView::showAllDayAgendaPopup(KOEvent *event)
{
  showEventPopup(mAllDayAgendaPopup,event);
}

void KOAgendaView::updateEventIndicatorTop(int newY)
{
  uint i;
  for(i=0;i<mMinY.size();++i) {
    if (newY >= mMinY[i]) mEventIndicatorTop->enableColumn(i,true);
    else mEventIndicatorTop->enableColumn(i,false);
  }
  
  mEventIndicatorTop->update();
}

void KOAgendaView::updateEventIndicatorBottom(int newY)
{
  uint i;
  for(i=0;i<mMaxY.size();++i) {
    if (newY <= mMaxY[i]) mEventIndicatorBottom->enableColumn(i,true);
    else mEventIndicatorBottom->enableColumn(i,false);
  }

  mEventIndicatorBottom->update();
}

void KOAgendaView::startDrag(KOEvent *event)
{
  VCalDrag *vd = mCalendar->createDrag(event,this);
  if (vd->drag()) {
    qDebug("KOTodoListView::contentsMouseMoveEvent(): Delete drag source");
  }
}
