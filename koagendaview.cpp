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
#include <kdebug.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>

#include "koprefs.h"
#include "koagenda.h"
#include "koagendaitem.h"
#include "calendar.h"
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
//  kdDebug() << "paintevent..." << endl;
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

  if (mLocation == Top) mPixmap = SmallIcon("1uparrow");
  else mPixmap = SmallIcon("1downarrow");

  setMinimumHeight(mPixmap.height());
}

EventIndicator::~EventIndicator()
{
}

void EventIndicator::drawContents(QPainter *p)
{
//  kdDebug() << "======== top: " << contentsRect().top() << "  bottom " << //         contentsRect().bottom() << "  left " << contentsRect().left() << "  right " << contentsRect().right() << endl;

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

KOAgendaView::KOAgendaView(Calendar *cal,QWidget *parent,const char *name) :
  KOEventView (cal,parent,name)
{
  mStartDate = QDate::currentDate();
  mStartHour = 8;
                         
  mLayoutDayLabels = 0;
  mDayLabelsFrame = 0;
  mDayLabels = 0;
  
  // Create agenda splitter
  mSplitterAgenda = new QSplitter(Vertical,this);
  mSplitterAgenda->setOpaqueResize();

  // Create all-day agenda widget
  mAllDayFrame = new QHBox(mSplitterAgenda);
  mDummyAllDayLeft = new QWidget(mAllDayFrame);
  mAllDayAgenda = new KOAgenda(1,mAllDayFrame);
  QWidget *dummyAllDayRight = new QWidget(mAllDayFrame);

  // Create event context menu for all day agenda
  mAllDayAgendaPopup = eventPopup();
  connect(mAllDayAgenda,SIGNAL(showEventPopupSignal(Event *)),
          mAllDayAgendaPopup,SLOT(showEventPopup(Event *)));

  // Create agenda frame
  QWidget *agendaFrame = new QWidget(mSplitterAgenda);
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
  mAgendaPopup->addAdditionalItem(QIconSet(SmallIcon("bell")),
                                  i18n("ToggleAlarm"),mAgenda,
                                  SLOT(popupAlarm()),true);
  connect(mAgenda,SIGNAL(showEventPopupSignal(Event *)),
          mAgendaPopup,SLOT(showEventPopup(Event *)));

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
  layoutTop->addWidget(mSplitterAgenda);

  //connect signals and slots
  connect(mAgenda->verticalScrollBar(),SIGNAL(valueChanged(int)),
          mTimeLabels, SLOT(positionChanged()));
  connect(mAgenda,SIGNAL(newEventSignal(int,int)),
                  SLOT(newEvent(int,int)));
  connect(mAllDayAgenda,SIGNAL(newEventSignal(int,int)),
                        SLOT(newEventAllDay(int,int)));
  connect(mAgenda,SIGNAL(editEventSignal(Event *)),
                  SIGNAL(editEventSignal(Event *)));
  connect(mAllDayAgenda,SIGNAL(editEventSignal(Event *)),
                        SIGNAL(editEventSignal(Event *)));
  connect(mAgenda,SIGNAL(showEventSignal(Event *)),
                  SIGNAL(showEventSignal(Event *)));
  connect(mAllDayAgenda,SIGNAL(showEventSignal(Event *)),
                        SIGNAL(showEventSignal(Event *)));
  connect(mAgenda,SIGNAL(deleteEventSignal(Event *)),
                  SIGNAL(deleteEventSignal(Event *)));
  connect(mAllDayAgenda,SIGNAL(deleteEventSignal(Event *)),
                        SIGNAL(deleteEventSignal(Event *)));
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
  connect(mAgenda,SIGNAL(startDragSignal(Event *)),
          SLOT(startDrag(Event *)));
  connect(mAllDayAgenda,SIGNAL(startDragSignal(Event *)),
          SLOT(startDrag(Event *)));

  // synchronize selections
  connect(mAgenda,SIGNAL(itemSelected(bool)),
          mAllDayAgenda,SLOT(deselectItem()));
  connect(mAllDayAgenda,SIGNAL(itemSelected(bool)),
          mAgenda,SLOT(deselectItem()));
  connect(mAgenda,SIGNAL(itemSelected(bool)),
          SIGNAL(eventsSelected(bool)));
  connect(mAllDayAgenda,SIGNAL(itemSelected(bool)),
          SIGNAL(eventsSelected(bool)));
}


KOAgendaView::~KOAgendaView()
{
  delete mAgendaPopup;
  delete mAllDayAgendaPopup;
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
    QString str = QString("%1 %2")
        .arg(KGlobal::locale()->weekDayName(date.dayOfWeek(),true))
        .arg(date.day());
    QString holiday = mCalendar->getHolidayForDate(date);
    if (!holiday.isEmpty()) str.append("\n" + holiday);
    dayLabel->setText(str);
    dayLabel->setAlignment(QLabel::AlignHCenter);
    if (date == QDate::currentDate()) {
      QFont font = dayLabel->font();
      font.setBold(true);
      dayLabel->setFont(font);
    }
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

QList<Incidence> KOAgendaView::getSelected()
{
  QList<Incidence> selectedEvents;
  Event *event;

  event = mAgenda->selectedEvent();
  if (event) selectedEvents.append(event);

  event = mAllDayAgenda->selectedEvent();
  if (event) selectedEvents.append(event);  

  return selectedEvents;
}


void KOAgendaView::updateView()
{
//  kdDebug() << "KOAgendaView::updateView()" << endl;
  fillAgenda();
}


/*
  Update configuration settings for the agenda view. This method is not
  complete.
*/
void KOAgendaView::updateConfig()
{
//  kdDebug() << "KOAgendaView::updateConfig()" << endl;

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

  setHolidayMasks();

  updateView();
}


void KOAgendaView::updateEventDates(KOAgendaItem *item)
{
//  kdDebug() << "updateEventDates(): " << item->text() << endl;

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
  
//  kdDebug() << "updateEventDates(): now setting dates" << endl;

  item->itemEvent()->setDtStart(startDt);
  item->itemEvent()->setDtEnd(endDt);

//  kdDebug() << "updateEventDates() done " << endl;
}


void KOAgendaView::selectDates(const QDateList list)
{
//  kdDebug() << "KOAgendaView::selectDates" << endl;
  
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


void KOAgendaView::selectEvents(QList<Event>)
{
  kdDebug() << "KOAgendaView::selectEvents() is not yet implemented" << endl;
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

void KOAgendaView::changeEventDisplay(Event *, int)
{
//  kdDebug() << "KOAgendaView::changeEventDisplay" << endl;
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
//  kdDebug() << "KOAgendaView::slotViewChange(): " << newView << endl;

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
//  kdDebug() << "Fill Agenda beginning with date " << mStartDate.toString() << endl;
//  kdDebug() << " number of dates: " << mSelectedDates.count() << endl;

//  clearView();

  mAllDayAgenda->changeColumns(mSelectedDates.count());
  mAgenda->changeColumns(mSelectedDates.count());
  mEventIndicatorTop->changeColumns(mSelectedDates.count());
  mEventIndicatorBottom->changeColumns(mSelectedDates.count());

  createDayLabels();
  setHolidayMasks();

  mMinY.resize(mSelectedDates.count());
  mMaxY.resize(mSelectedDates.count());

  QList<Event> dayEvents;
  int curCol;  // current column of agenda, i.e. the X coordinate
  QDate currentDate = mStartDate;
  for(curCol=0;curCol<int(mSelectedDates.count());++curCol) {
//    kdDebug() << "KOAgendaView::fillAgenda(): " << currentDate.toString()
//              << endl;

    dayEvents = mCalendar->getEventsForDate(currentDate,false);

    // Default values, which can never be reached
    mMinY[curCol] = mAgenda->timeToY(QTime(23,59)) + 1;
    mMaxY[curCol] = mAgenda->timeToY(QTime(0,0)) - 1;

    unsigned int numEvent;
    for(numEvent=0;numEvent<dayEvents.count();++numEvent) {
      Event *event = dayEvents.at(numEvent);
//      kdDebug() << " Event: " << event->getSummary() << endl;

      int beginX = currentDate.daysTo(event->dtStart().date()) + curCol;
      int endX = currentDate.daysTo(event->dtEnd().date()) + curCol;

//      kdDebug() << "  beginX: " << beginX << "  endX: " << endX << endl;
      
      if (event->doesFloat()) {
        if (event->recurrence()->doesRecur()) {
          mAllDayAgenda->insertAllDayItem(event,curCol,curCol);
        } else {
          if (beginX <= 0 && curCol == 0) {     
            mAllDayAgenda->insertAllDayItem(event,beginX,endX);
          } else if (beginX == curCol) {
            mAllDayAgenda->insertAllDayItem(event,beginX,endX);
          }
        }
      } else if (event->isMultiDay()) {
        int startY = mAgenda->timeToY(event->dtStart().time());
        int endY = mAgenda->timeToY(event->dtEnd().time()) - 1;  
        if ((beginX <= 0 && curCol == 0) || beginX == curCol) {
          mAgenda->insertMultiItem(event,beginX,endX,startY,endY);
        }
        if (beginX == curCol) {
          mMaxY[curCol] = mAgenda->timeToY(QTime(23,59));
          if (startY < mMinY[curCol]) mMinY[curCol] = startY;
        } else if (endX == curCol) {
          mMinY[curCol] = mAgenda->timeToY(QTime(0,0));
          if (endY > mMaxY[curCol]) mMaxY[curCol] = endY;
        } else {
          mMinY[curCol] = mAgenda->timeToY(QTime(0,0));
          mMaxY[curCol] = mAgenda->timeToY(QTime(23,59));
        }
      } else {
        int startY = mAgenda->timeToY(event->dtStart().time());
        int endY = mAgenda->timeToY(event->dtEnd().time()) - 1;
	if (endY < startY) endY = startY;
	mAgenda->insertItem(event,curCol,startY,endY);
        if (startY < mMinY[curCol]) mMinY[curCol] = startY;
        if (endY > mMaxY[curCol]) mMaxY[curCol] = endY;
      }
    }
//    if (numEvent == 0) kdDebug() << " No events" << endl;
    
    currentDate = currentDate.addDays(1);
  }

  mAgenda->checkScrollBoundaries();

//  mAgenda->viewport()->update();
//  mAllDayAgenda->viewport()->update();

  emit eventsSelected(false);

//  kdDebug() << "Fill Agenda done" << endl;
}

void KOAgendaView::clearView()
{
//  kdDebug() << "ClearView" << endl;
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

void KOAgendaView::showAgendaPopup(Event *event)
{
  showEventPopup(mAgendaPopup,event);
}

void KOAgendaView::showAllDayAgendaPopup(Event *event)
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

void KOAgendaView::startDrag(Event *event)
{
  VCalDrag *vd = mCalendar->createDrag(event,this);
  if (vd->drag()) {
    kdDebug() << "KOTodoListView::contentsMouseMoveEvent(): Delete drag source" << endl;
  }
}

void KOAgendaView::readSettings()
{
  readSettings(kapp->config());
}

void KOAgendaView::readSettings(KConfig *config)
{
  config->setGroup("Views");
    
  QValueList<int> sizes = config->readIntListEntry("Separator AgendaView");
  if (sizes.count() == 2) {
    mSplitterAgenda->setSizes(sizes);
  }

  setView(config->readNumEntry("Agenda View", KOAgendaView::WEEK));
}

void KOAgendaView::writeSettings(KConfig *config)
{
  config->setGroup("Views");
    
  QValueList<int> list = mSplitterAgenda->sizes();
  config->writeEntry("Separator AgendaView",list);

  config->writeEntry("Agenda View",currentView());
}

void KOAgendaView::setHolidayMasks()
{
  mHolidayMask.resize(mSelectedDates.count());

  uint i;
  for(i=0;i<mSelectedDates.count();++i) {
    QDate date = *(mSelectedDates.at(i));
    if ((KOPrefs::instance()->mExcludeSaturdays &&
         date.dayOfWeek() == 6) ||
        (KOPrefs::instance()->mExcludeHolidays && 
         (!mCalendar->getHolidayForDate(date).isEmpty() ||
          date.dayOfWeek() == 7))) {
      mHolidayMask[i] = true;
    } else {
      mHolidayMask[i] = false;
    }
  }
  
  mAgenda->setHolidayMask(&mHolidayMask);
  mAllDayAgenda->setHolidayMask(&mHolidayMask);
}
