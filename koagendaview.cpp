// $Id$

#include "koagendaview.h"
#include "koagendaview.moc"

#include "koagenda.h"
#include "koagendaitem.h"
#include "calobject.h"
#include "calprinter.h"

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qframe.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qfont.h>
#include <qfontmetrics.h>

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>

#include "koprefs.h"

TimeLabels::TimeLabels(int rows,QWidget *parent,const char *name,WFlags f) :
  QScrollView(parent,name,f)
{
  mTimeFormat = 0;
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
    switch (mTimeFormat)
    {
      // am/pm format
      case 1:
      {
        //debug("am/pm format");
        if (cell > 11) suffix = "pm";
        if (cell == 0) hour.setNum(12);
        if (cell > 12) hour.setNum(cell - 12);
        break;
      }

      // 24h time
      case 0:
      {
        //debug("24h format");
        suffix = ":00";
        break;
      }

      default:
        debug("Error:  time format not recognised.");
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

void TimeLabels::setTimeFormat(int format)
{
  mTimeFormat = format;
}

/** updates widget's internal state */
void TimeLabels::updateConfig(KConfig *config)
{
  // set the font
//  config->setGroup("Fonts");
//  QFont font = config->readFontEntry("TimeBar Font");
  setFont(KOPrefs::instance()->mTimeBarFont);

  // set the time format
  config->setGroup("Time & Date");
  int fmt = config->readNumEntry("Time Format", 1);
  // fmt = 0 for 24h, or 1 for am/pm
  setTimeFormat(fmt);

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
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

KOAgendaView::KOAgendaView(CalObject *cal,QWidget *parent,const char *name) :
  KOBaseView (cal,parent,name)
{
  mStartDate = QDate::currentDate();
  mWeekStartsMonday = true;
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

  // Create agenda frame
  QHBox *agendaFrame = new QHBox(splitterAgenda);

  // Create time labels
  mTimeLabels = new TimeLabels(24,agendaFrame);

  // Create agenda
  mAgenda = new KOAgenda(1,48,20,agendaFrame);

  // Create day name labels for agenda columns
  mDayLabelsFrame = new QHBox(this);
  createDayLabels();

  // make connections between dependent widgets
  mTimeLabels->setAgenda(mAgenda);

  // Update widgets to reflect user preferences
//  updateConfig();

  // these blank widgets make the All Day Event box line up with the agenda
  dummyAllDayRight->setFixedWidth(mAgenda->verticalScrollBar()->width());
  mDummyAllDayLeft->setFixedWidth(mTimeLabels->width());

  QBoxLayout *layoutTop = new QVBoxLayout(this);
  layoutTop->addWidget(mDayLabelsFrame);
  layoutTop->addWidget(splitterAgenda);

  //connect signals and slots
  QObject::connect(mAgenda->verticalScrollBar(),SIGNAL(valueChanged(int)),
                   mTimeLabels, SLOT(positionChanged()));
  QObject::connect(mAgenda,SIGNAL(newEventSignal(int,int)),
                           SLOT(newEvent(int,int)));
  QObject::connect(mAllDayAgenda,SIGNAL(newEventSignal(int,int)),
                                 SLOT(newEventAllDay(int,int)));
  QObject::connect(mAgenda,SIGNAL(editEventSignal(KOEvent *)),
                           SIGNAL(editEventSignal(KOEvent *)));
  QObject::connect(mAllDayAgenda,SIGNAL(editEventSignal(KOEvent *)),
                                 SIGNAL(editEventSignal(KOEvent *)));
  QObject::connect(mAgenda,SIGNAL(showEventSignal(KOEvent *)),
                           SIGNAL(showEventSignal(KOEvent *)));
  QObject::connect(mAllDayAgenda,SIGNAL(showEventSignal(KOEvent *)),
                                 SIGNAL(showEventSignal(KOEvent *)));
  QObject::connect(mAgenda,SIGNAL(deleteEventSignal(KOEvent *)),
                           SIGNAL(deleteEventSignal(KOEvent *)));
  QObject::connect(mAllDayAgenda,SIGNAL(deleteEventSignal(KOEvent *)),
                                 SIGNAL(deleteEventSignal(KOEvent *)));
  QObject::connect(mAgenda,SIGNAL(itemModified(KOAgendaItem *)),
                           SLOT(updateEventDates(KOAgendaItem *)));
  QObject::connect(mAllDayAgenda,SIGNAL(itemModified(KOAgendaItem *)),
                                 SLOT(updateEventDates(KOAgendaItem *)));
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
    dayLabel->setText(QString("%1 %2").arg(date.dayName(date.dayOfWeek()))
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


QList<KOEvent> KOAgendaView::getSelected()
{
  QList<KOEvent> selectedEvents;
  
  return selectedEvents;
}


void KOAgendaView::updateView()
{
  qDebug("KOAgendaView::updateView()\n\n");
  fillAgenda();
}


/*
  Update configuration settings for the agenda view. This method is not
  complete.
*/
void KOAgendaView::updateConfig()
{
//  debug("KOAgendaView::updateConfig()");

  KConfig config(locate("config","korganizerrc"));

  // update koagendaview members
  mWeekStartsMonday = config.readBoolEntry("Week Starts Monday", true);

  // update config for children
  mTimeLabels->updateConfig(&config);
  mAgenda->updateConfig(&config);

  // widget synchronization
  //TODO: find a better way, maybe signal/slot
  mTimeLabels->positionChanged();

  // for some reason, this needs to be called explicitly
  mTimeLabels->repaint();

  mDummyAllDayLeft->setFixedWidth(mTimeLabels->width());

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
  
//  qDebug("  B StartDt: %s, EndDt: %s",startDt.toString().latin1(),
//                                    endDt.toString().latin1());
  
  item->itemEvent()->setDtStart(startDt);
  item->itemEvent()->setDtEnd(endDt);

//  qDebug("  A StartDt: %s, EndDt: %s",
//         item->itemEvent()->getDtStart().toString().latin1(),
//         item->itemEvent()->getDtEnd().toString().latin1());
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
	     (mWeekStartsMonday ? 1 : 7)) &&
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


void KOAgendaView::selectEvents(QList<KOEvent> eventList)
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
    mStartDate = mStartDate.addDays( 7 );
    mSelectedDates.clear();
    for( count = 0; count < 7; count++ )
      mSelectedDates.append(new QDate(mStartDate.addDays(count)));
    break;
  case LIST:
    datenum = mSelectedDates.count();
    mStartDate = mSelectedDates.last()->addDays( 1 );
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
      if (mWeekStartsMonday)
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
      if (mWeekStartsMonday)
        mStartDate = mStartDate.addDays(-6);
    } else if (mWeekStartsMonday) {
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

  createDayLabels();

  QList<KOEvent> dayEvents;
  int curCol;  // current column of agenda, i.e. the X coordinate
  QDate currentDate = mStartDate;
  for(curCol=0;curCol<int(mSelectedDates.count());++curCol) {
    dayEvents = calendar->getEventsForDate(currentDate,false);    

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
        }
      } else {
        int startY = mAgenda->timeToY(event->getDtStart().time());
        int endY = mAgenda->timeToY(event->getDtEnd().time()) - 1;
	if (endY < startY) endY = startY;
	mAgenda->insertItem(event,curCol,startY,endY);
      }
    }
//    if (numEvent == 0) qDebug(" No events");
    
    currentDate = currentDate.addDays(1);
  }

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

