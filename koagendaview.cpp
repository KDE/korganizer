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

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>

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

void TimeLabels::drawContents(QPainter *p,int cx, int cy, int cw, int ch)
{
  int cell = ((int)(cy/mCellHeight));
  int y = cell * mCellHeight;
  QString hour;
  while (y < cy + ch) {
    p->drawLine(cx,y,cx+cw,y);
    hour.setNum(cell++);
    p->drawText(cx+5,y+15,hour);
    y += mCellHeight;
  }
}


KOAgendaView::KOAgendaView(CalObject *cal,QWidget *parent,const char *name) :
  KOBaseView (cal,parent,name)
{
  mStartDate = QDate::currentDate();
  mWeekStartsMonday = true;
  mStartHour = 8;

  mConfig = new KConfig(locate("config","korganizerrc"));
                         
  mLayoutDayLabels = 0;
  mDayLabelsFrame = 0;
  mDayLabels = 0;
//  mDayLabelsFrame = new QFrame(this);
  
  // Create agenda splitter
  QSplitter *splitterAgenda = new QSplitter(Vertical,this);
  splitterAgenda->setOpaqueResize();

  // Create all-day agenda widget
  mAllDayFrame = new QHBox(splitterAgenda);
  QWidget *dummyAllDayLeft = new QWidget(mAllDayFrame);
  mAllDayAgenda = new KOAgenda(1,mAllDayFrame);
  QWidget *dummyAllDayRight = new QWidget(mAllDayFrame);

  // Create agenda frame
  QHBox *agendaFrame = new QHBox(splitterAgenda);
      
  // Create time labels
  mTimeLabels = new TimeLabels(24,agendaFrame);
//  mTimeLabels->setFrameStyle(QFrame::Box|QFrame::Raised);
  mTimeLabels->setFixedWidth(30);

  // Create agenda
  mAgenda = new KOAgenda(1,48,20,agendaFrame);

  // Create day name labels for agenda columns
//  QWidget *dummyDaysLeft = new QWidget(mDayLabelsFrame);
//  mDayLabels = 0;
  mDayLabelsFrame = new QHBox(this);
  createDayLabels();
//  QWidget *dummyDaysRight = new QWidget(mDayLabelsFrame);

  // Create layout for KOAgendaView 
//  QGridLayout *layoutTop = new QGridLayout(this,3,3);
//  layoutTop->addWidget(mDayLabels,0,1);
//  layoutTop->addWidget(mTimeLabels,2,0);
//  layoutTop->addMultiCellWidget(splitterAgenda,1,2,1,2);
//  layoutTop->addColSpacing(2,mAgenda->verticalScrollBar()->width());
//  dummyDaysRight->setFixedWidth(mAgenda->verticalScrollBar()->width());
  dummyAllDayRight->setFixedWidth(mAgenda->verticalScrollBar()->width());
//  dummyDaysLeft->setFixedWidth(mTimeLabels->width());
  dummyAllDayLeft->setFixedWidth(mTimeLabels->width());

  QBoxLayout *layoutTop = new QVBoxLayout(this);
  layoutTop->addWidget(mDayLabelsFrame);
  layoutTop->addWidget(splitterAgenda);

  QValueList<int> splitterSizes;
  splitterSizes.append(mAllDayFrame->minimumHeight());
  splitterSizes.append(300);  // don´t know what this should be, works anyway
  splitterAgenda->setSizes(splitterSizes);

  QObject::connect(mAgenda->verticalScrollBar(),SIGNAL(valueChanged(int)),
                                                SLOT(adjustTimeLabels()));

  QObject::connect(mAgenda,SIGNAL(newEventSignal(int,int)),
                           SLOT(newEvent(int,int)));
  QObject::connect(mAllDayAgenda,SIGNAL(newEventSignal(int,int)),
                                 SLOT(newEventAllDay(int,int)));
  QObject::connect(mAgenda,SIGNAL(editEventSignal(KOEvent *)),
                           SIGNAL(editEventSignal(KOEvent *)));
  QObject::connect(mAllDayAgenda,SIGNAL(editEventSignal(KOEvent *)),
                                 SIGNAL(editEventSignal(KOEvent *)));
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
  delete mConfig;
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
//  qDebug("KOAgendaView::updateView()");
  fillAgenda();
}


/*
  Update configuration settings for the agenda view. This method is not
  complete. It currently only sets the start day of the week.
*/
void KOAgendaView::updateConfig()
{
  int fmt;

  mConfig->setGroup("Time & Date");
  fmt = mConfig->readNumEntry("Time Format", 1);
//  agendaSheet->setTimeStyle((fmt == 1 ? KAgendaSheet::Time_12 :
//                                        KAgendaSheet::Time_24));
  mWeekStartsMonday = mConfig->readBoolEntry("Week Starts Monday", true);
  
  mConfig->setGroup("Views");
  QString startStr(mConfig->readEntry("Day Begins", "8:00"));
  // handle case where old config files are in format "8" instead of "8:00".
  int colonPos = startStr.find(':');
  if (colonPos >= 0)
    startStr.truncate(colonPos);
  int mStartHour = startStr.toUInt();
  mAgenda->setStartHour(mStartHour);
//  startScrollVal = scrollBar->minValue() +
//                   (agendaSheet->rowSize() * startHour);

  mConfig->setGroup("Fonts");
//  agendaSheet->setTimeFont( mConfig->readFontEntry( "TimeBar Font" ) );

/*
  QListIterator<EventWidget> iter(WidgetList);
  EventWidget *tmpWidget;
  iter.toFirst();
  while ((tmpWidget = iter.current()))
  {
  	  tmpWidget->updateConfig();
  	  ++iter;
  }

  agendaSheet->updateConfig();

  update();
*/
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

void KOAgendaView::adjustTimeLabels()
{
  mTimeLabels->setContentsPos(0,mAgenda->contentsY());
}


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
