/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

// $Id$

#include <qhbox.h>
#include <qvbox.h>
#include <qlabel.h>
#include <qframe.h>
#include <qlayout.h>
#ifndef KORG_NOSPLITTER
#include <qsplitter.h>
#endif
#include <qfont.h>
#include <qfontmetrics.h>
#include <qpopupmenu.h>
#include <qtooltip.h>
#include <qpainter.h>
#include <qpushbutton.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>

#include <libkcal/calendar.h>
#include <libkcal/vcaldrag.h>
#include <libkcal/dndfactory.h>

#ifndef KORG_NOPLUGINS
#include "kocore.h"
#endif
#include "koprefs.h"
#include "koagenda.h"
#include "koagendaitem.h"
#ifndef KORG_NOPRINTER
#include "calprinter.h"
#endif

#include "koagendaview.h"
#include "koagendaview.moc"

using namespace KOrg;

TimeLabels::TimeLabels(int rows,QWidget *parent,const char *name,WFlags f) :
  QScrollView(parent,name,f)
{
  mRows = rows;

  mCellHeight = KOPrefs::instance()->mHourSize*4;

  enableClipper(true);

  setHScrollBarMode(AlwaysOff);
  setVScrollBarMode(AlwaysOff);

  resizeContents(50,mRows * mCellHeight);

  viewport()->setBackgroundMode( PaletteBackground );
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
  int borderWidth = 4;

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
  
  // update HourSize
  mCellHeight = KOPrefs::instance()->mHourSize*4;
  resizeContents(50,mRows * mCellHeight);
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
      int xOffset = QApplication::reverseLayout() ? 
               (mColumns - 1 - i)*cellWidth + cellWidth/2 -mPixmap.width()/2 :
               i*cellWidth + cellWidth/2 -mPixmap.width()/2;
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
  mStartHour = 8;
  mSelectedDates.append(QDate::currentDate());

  mLayoutDayLabels = 0;
  mDayLabelsFrame = 0;
  mDayLabels = 0;

  bool isRTL = QApplication::reverseLayout();

  if ( KOPrefs::instance()->mVerticalScreen ) {
    mExpandedPixmap = SmallIcon( "1downarrow" );
    mNotExpandedPixmap = SmallIcon( "1uparrow" );
  } else {
    mExpandedPixmap = SmallIcon( isRTL ? "1leftarrow" : "1rightarrow" );
    mNotExpandedPixmap = SmallIcon( isRTL ? "1rightarrow" : "1leftarrow" );
  }

  QBoxLayout *topLayout = new QVBoxLayout(this);

  // Create day name labels for agenda columns
  mDayLabelsFrame = new QHBox(this);
  topLayout->addWidget(mDayLabelsFrame);

  // Create agenda splitter
#ifndef KORG_NOSPLITTER
  mSplitterAgenda = new QSplitter(Vertical,this);
  topLayout->addWidget(mSplitterAgenda);
  mSplitterAgenda->setOpaqueResize();
  
  mAllDayFrame = new QHBox(mSplitterAgenda);

  QWidget *agendaFrame = new QWidget(mSplitterAgenda);
#else
  QVBox *mainBox = new QVBox( this );
  topLayout->addWidget( mainBox );
  
  mAllDayFrame = new QHBox(mainBox);

  QWidget *agendaFrame = new QWidget(mainBox);
#endif

  // Create all-day agenda widget
  mDummyAllDayLeft = new QVBox( mAllDayFrame );

  mExpandButton = new QPushButton(mDummyAllDayLeft);
  mExpandButton->setPixmap( mNotExpandedPixmap );
  mExpandButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed,
                                QSizePolicy::Fixed ) );
  connect( mExpandButton, SIGNAL( clicked() ), SIGNAL( toggleExpand() ) );
 
  mAllDayAgenda = new KOAgenda(1,mAllDayFrame);
  QWidget *dummyAllDayRight = new QWidget(mAllDayFrame);

  // Create event context menu for all day agenda
  mAllDayAgendaPopup = eventPopup();
  connect(mAllDayAgenda,SIGNAL(showEventPopupSignal(Event *)),
          mAllDayAgendaPopup,SLOT(showEventPopup(Event *)));

  // Create agenda frame
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
  mAgenda = new KOAgenda(1,96,KOPrefs::instance()->mHourSize,agendaFrame);
  agendaLayout->addMultiCellWidget(mAgenda,1,1,1,2);
  agendaLayout->setColStretch(1,1);

  // Create event context menu for agenda
  mAgendaPopup = eventPopup();
  mAgendaPopup->addAdditionalItem(QIconSet(SmallIcon("bell")),
                                  i18n("Toggle Alarm"),mAgenda,
                                  SLOT(popupAlarm()),true);
  connect(mAgenda,SIGNAL(showEventPopupSignal(Event *)),
          mAgendaPopup,SLOT(showEventPopup(Event *)));

  // make connections between dependent widgets
  mTimeLabels->setAgenda(mAgenda);

  // Update widgets to reflect user preferences
//  updateConfig();

  createDayLabels();

  // these blank widgets make the All Day Event box line up with the agenda
  dummyAllDayRight->setFixedWidth(mAgenda->verticalScrollBar()->width());
  dummyAgendaRight->setFixedWidth(mAgenda->verticalScrollBar()->width());
  mDummyAllDayLeft->setFixedWidth(mTimeLabels->width());

  // Scrolling
  connect(mAgenda->verticalScrollBar(),SIGNAL(valueChanged(int)),
          mTimeLabels, SLOT(positionChanged()));
  connect(mTimeLabels->verticalScrollBar(),SIGNAL(valueChanged(int)),
          SLOT(setContentsPos(int)));

  // Create/Show/Edit/Delete Event
  connect(mAgenda,SIGNAL(newEventSignal(int,int)),
                  SLOT(newEvent(int,int)));
  connect(mAgenda,SIGNAL(newEventSignal(int,int,int,int)),
                  SLOT(newEvent(int,int,int,int)));
  connect(mAllDayAgenda,SIGNAL(newEventSignal(int,int)),
                        SLOT(newEventAllDay(int,int)));
  connect(mAllDayAgenda,SIGNAL(newEventSignal(int,int,int,int)),
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
  connect( mAgenda, SIGNAL( incidenceSelected( Incidence * ) ),
           mAllDayAgenda, SLOT( deselectItem() ) );
  connect( mAllDayAgenda, SIGNAL( incidenceSelected( Incidence * ) ),
           mAgenda, SLOT( deselectItem() ) );
  connect( mAgenda, SIGNAL( incidenceSelected( Incidence * ) ),
           SIGNAL( incidenceSelected( Incidence * ) ) );
  connect( mAllDayAgenda, SIGNAL( incidenceSelected( Incidence * ) ),
           SIGNAL( incidenceSelected( Incidence * ) ) );
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

  DateList::ConstIterator dit;
  for( dit = mSelectedDates.begin(); dit != mSelectedDates.end(); ++dit ) {
    QDate date = *dit;
    QBoxLayout *dayLayout = new QVBoxLayout(mLayoutDayLabels);

    QLabel *dayLabel = new QLabel(mDayLabels);
    QString str = QString("%1 %2")
        .arg(KGlobal::locale()->weekDayName(date.dayOfWeek(),true))
        .arg(date.day());
    dayLabel->setText(str);
    dayLabel->setAlignment(QLabel::AlignHCenter);
    if (date == QDate::currentDate()) {
      QFont font = dayLabel->font();
      font.setBold(true);
      dayLabel->setFont(font);
    }
    dayLayout->addWidget(dayLabel);

#ifndef KORG_NOPLUGINS
    CalendarDecoration::List cds = KOCore::self()->calendarDecorations();
    CalendarDecoration *it;
    for(it = cds.first(); it; it = cds.next()) {
      QString text = it->shortText( date );
      if ( !text.isEmpty() ) {
        QLabel *label = new QLabel(text,mDayLabels);
        label->setAlignment(AlignCenter);
        dayLayout->addWidget(label);
      }
    }

    for(it = cds.first(); it; it = cds.next()) {
      QWidget *wid = it->smallWidget(mDayLabels,date);
      if ( wid ) {
//      wid->setHeight(20);
        dayLayout->addWidget(wid);
      }
    }
#endif
  }

  mLayoutDayLabels->addSpacing(mAgenda->verticalScrollBar()->width());
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

QPtrList<Incidence> KOAgendaView::selectedIncidences()
{
  QPtrList<Incidence> selectedEvents;
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

  if ( ( mViewType == NEXTX ) &&
      ( int( mSelectedDates.count() ) != KOPrefs::instance()->mNextXDays) ) {
    int count;
    QDate firstDate = mSelectedDates.first();
    mSelectedDates.clear();
    for( count = 0; count < KOPrefs::instance()->mNextXDays; count++ )
      mSelectedDates.append(firstDate.addDays(count));
  }
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

  createDayLabels();

  updateView();
}


void KOAgendaView::updateEventDates(KOAgendaItem *item)
{
//  kdDebug() << "KOAgendaView::updateEventDates(): " << item->text() << endl;

  QDateTime startDt,endDt;
  QDate startDate;

  if (item->cellX() < 0) {
    startDate = (mSelectedDates.first()).addDays(item->cellX());
  } else {
    startDate = mSelectedDates[item->cellX()];
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

//  kdDebug() << "KOAgendaView::updateEventDates(): now setting dates" << endl;

  QDate olddate = item->itemEvent()->dtStart().date();

  item->itemEvent()->setDtStart(startDt);
  item->itemEvent()->setDtEnd(endDt);
  item->itemEvent()->setRevision(item->itemEvent()->revision()+1);

  emit shiftedEvent(olddate, startDate);

//  kdDebug() << "KOAgendaView::updateEventDates() done " << endl;
}


void KOAgendaView::showDates( const QDate &start, const QDate &end )
{
//  kdDebug() << "KOAgendaView::selectDates" << endl;

  mSelectedDates.clear();

  QDate d = start;
  while (d <= end) {
    mSelectedDates.append(d);
    d = d.addDays( 1 );
  }

  // if there are 5 dates and the first is a monday, we have a workweek.
  if ((mSelectedDates.count() == 5) &&
      (mSelectedDates.first().dayOfWeek() == 1) &&
      (mSelectedDates.first().daysTo(mSelectedDates.last()) == 4)) {
    setView(WORKWEEK);

  // if there are 7 dates and the first is a monday, we have a regular week.
  } else if ((mSelectedDates.count() == 7) &&
             (mSelectedDates.first().dayOfWeek() ==
	     (KGlobal::locale()->weekStartsMonday() ? 1 : 7)) &&
             (mSelectedDates.first().daysTo(mSelectedDates.last()) == 6)) {
    setView(WEEK);

  } else if (mSelectedDates.count() == 1) {
    setView(DAY);

  } else if (((int)mSelectedDates.count() == KOPrefs::instance()->mNextXDays) &&
             (mSelectedDates.first() == QDate::currentDate()) ) {
    setView(NEXTX);
  } else {
    // for sanity, set viewtype to LIST for now...
    setView(LIST);
  }

  // and update the view
  fillAgenda();
}


void KOAgendaView::showEvents(QPtrList<Event>)
{
  kdDebug() << "KOAgendaView::showEvents() is not yet implemented" << endl;
}

void KOAgendaView::setView(int view)
{
  // change view type if valid
  if( mSelectedDates.count() ) {
    if ((view >= DAY) && (view <= NEXTX))
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
  shiftDates(-1);
}

void KOAgendaView::slotNextDates()
{
  shiftDates(+1);
}

void KOAgendaView::shiftDates(int multiplier)
{
  int shift = 0;

  switch( mViewType ) {
    case DAY:
      shift = 1;
      break;
    case WEEK:
    case WORKWEEK:
    case LIST:
      shift = 7;
      break;
    case NEXTX:
      shift = KOPrefs::instance()->mNextXDays;
      break;
    default:
      break;
  }

  DateList::Iterator it;
  for( it = mSelectedDates.begin(); it != mSelectedDates.end(); ++it ) {
    *it = (*it).addDays(multiplier*shift);
  }
  
  emit datesSelected(mSelectedDates);

  fillAgenda();
}


void KOAgendaView::slotViewChange()
{
  slotViewChange(mViewType + 1 != LIST ? mViewType + 1 : DAY);
}


void KOAgendaView::slotViewChange(int newView)
{
  kdDebug() << "KOAgendaView::slotViewChange(): " << newView << endl;

  int count;
  QDate firstDate = mSelectedDates.first();

  switch (newView) {
  case DAY:
    mSelectedDates.clear();
    mSelectedDates.append(firstDate);
    break;

  case WORKWEEK:
    // find monday for this week
    if (firstDate.dayOfWeek() == 7) {
      if (KGlobal::locale()->weekStartsMonday())
        firstDate = firstDate.addDays(-6);
      else
        firstDate = firstDate.addDays(1);
    } else if (firstDate.dayOfWeek() > 1) {
      firstDate = firstDate.addDays(firstDate.dayOfWeek() * -1 + 1);
    }

    mSelectedDates.clear();
    for (count = 0; count < 5; count++)
      mSelectedDates.append(firstDate.addDays(count));
    break;

  case WEEK:
    // find the beginning of this week (could be monday or sunday)
    if (firstDate.dayOfWeek() == 7) {
      if (KGlobal::locale()->weekStartsMonday())
        firstDate = firstDate.addDays(-6);
    } else if (KGlobal::locale()->weekStartsMonday()) {
      firstDate = firstDate.addDays(firstDate.dayOfWeek() * -1 + 1);
    } else {
      firstDate = firstDate.addDays(firstDate.dayOfWeek() * -1);
    }

    mSelectedDates.clear();
    for( count = 0; count < 7; count++ )
      mSelectedDates.append(firstDate.addDays(count));

    break;

  case LIST:
    /* Is this unnecessary? I believe the mSelectedDates is already
       correctly set and does not need to be reset like this. --ali@mit.edu
    */
    /*
      datenum = mSelectedDates.count();
    mSelectedDates.clear();
    for (count = 0; count < datenum; count++)
      mSelectedDates.append(new QDate(mStartDate.addDays(count)));
    */
    break;

  case NEXTX:
    firstDate = QDate::currentDate();
    mSelectedDates.clear();
    for( count = 0; count < KOPrefs::instance()->mNextXDays; count++ )
      mSelectedDates.append(firstDate.addDays(count));
  
    break;
  
  default:
    mSelectedDates.clear();
    mSelectedDates.append(QDate::currentDate());
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


void KOAgendaView::fillAgenda(const QDate &)
{
  fillAgenda();
}


void KOAgendaView::fillAgenda()
{
//  clearView();

  mAllDayAgenda->changeColumns(mSelectedDates.count());
  mAgenda->changeColumns(mSelectedDates.count());
  mEventIndicatorTop->changeColumns(mSelectedDates.count());
  mEventIndicatorBottom->changeColumns(mSelectedDates.count());

  createDayLabels();
  setHolidayMasks();

  mMinY.resize(mSelectedDates.count());
  mMaxY.resize(mSelectedDates.count());

  QPtrList<Event> dayEvents;

  mAgenda->setDateList(mSelectedDates);

  DateList::ConstIterator dit;
  int curCol = 0;
  for( dit = mSelectedDates.begin(); dit != mSelectedDates.end(); ++dit ) {
    QDate currentDate = *dit;
//    kdDebug() << "KOAgendaView::fillAgenda(): " << currentDate.toString()
//              << endl;

    dayEvents = calendar()->getEventsForDate(currentDate,true);

    // Default values, which can never be reached
    mMinY[curCol] = mAgenda->timeToY(QTime(23,59)) + 1;
    mMaxY[curCol] = mAgenda->timeToY(QTime(0,0)) - 1;

    unsigned int numEvent;
    for(numEvent=0;numEvent<dayEvents.count();++numEvent) {
      Event *event = dayEvents.at(numEvent);
//      kdDebug() << " Event: " << event->summary() << endl;

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
    ++curCol;
  }

  mAgenda->checkScrollBoundaries();

//  mAgenda->viewport()->update();
//  mAllDayAgenda->viewport()->update();

  emit incidenceSelected( 0 );

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
#ifndef KORG_NOPRINTER
  if (fd == td)
    calPrinter->preview(CalPrinter::Day, fd, td);
  else
    calPrinter->preview(CalPrinter::Week, fd, td);
#endif
}


void KOAgendaView::newEvent(int gx, int gy)
{
  if (!mSelectedDates.count()) return;
  
  QDate day = mSelectedDates[gx];

  QTime time = mAgenda->gyToTime(gy);
  QDateTime dt(day,time);

  emit newEventSignal(dt);
}

void KOAgendaView::newEvent(int gxStart, int gyStart, int gxEnd, int gyEnd)
{
  if (!mSelectedDates.count()) return;
  
  QDate dayStart = mSelectedDates[gxStart];
  QDate dayEnd = mSelectedDates[gxEnd];

  QTime timeStart = mAgenda->gyToTime(gyStart);
  QTime timeEnd = mAgenda->gyToTime( gyEnd + 1 );

  QDateTime dtStart(dayStart,timeStart);
  QDateTime dtEnd(dayEnd,timeEnd);

  emit newEventSignal(dtStart,dtEnd);
}

void KOAgendaView::newEventAllDay(int gx, int )
{
  if (!mSelectedDates.count()) return;

  QDate day = mSelectedDates[gx];

  emit newEventSignal(day);
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
#ifndef KORG_NODND
  DndFactory factory( calendar() );
  VCalDrag *vd = factory.createDrag(event,this);
  if (vd->drag()) {
    kdDebug() << "KOTodoListView::contentsMouseMoveEvent(): Delete drag source" << endl;
  }
#endif
}

void KOAgendaView::readSettings()
{
  readSettings(KGlobal::config());
}

void KOAgendaView::readSettings(KConfig *config)
{
//  kdDebug() << "KOAgendaView::readSettings()" << endl;

  config->setGroup("Views");

#ifndef KORG_NOSPLITTER
  QValueList<int> sizes = config->readIntListEntry("Separator AgendaView");
  if (sizes.count() == 2) {
    mSplitterAgenda->setSizes(sizes);
  }
#endif

  slotViewChange(config->readNumEntry("Agenda View", KOAgendaView::WORKWEEK));

  updateConfig();
}

void KOAgendaView::writeSettings(KConfig *config)
{
//  kdDebug() << "KOAgendaView::writeSettings()" << endl;

  config->setGroup("Views");

#ifndef KORG_NOSPLITTER
  QValueList<int> list = mSplitterAgenda->sizes();
  config->writeEntry("Separator AgendaView",list);
#endif

  config->writeEntry("Agenda View",currentView());
}

void KOAgendaView::setHolidayMasks()
{
  mHolidayMask.resize(mSelectedDates.count());

  uint i;
  for(i=0;i<mSelectedDates.count();++i) {
    QDate date = mSelectedDates[i];
    bool showSaturday = KOPrefs::instance()->mExcludeSaturdays && (date.dayOfWeek() == 6);
    bool showSunday = KOPrefs::instance()->mExcludeHolidays && (date.dayOfWeek() == 7);
#ifndef KORG_NOPLUGINS
    bool showHoliday = KOPrefs::instance()->mExcludeHolidays &&
                       !KOCore::self()->holiday(date).isEmpty();
    bool showDay = showSaturday || showSunday || showHoliday;
#else
    bool showDay = showSaturday || showSunday;
#endif
    if (showDay) {
      mHolidayMask[i] = true;
    } else {
      mHolidayMask[i] = false;
    }
  }

  mAgenda->setHolidayMask(&mHolidayMask);
  mAllDayAgenda->setHolidayMask(&mHolidayMask);
}

void KOAgendaView::setContentsPos(int y)
{
  mAgenda->setContentsPos(0,y);
}

void KOAgendaView::setExpandedButton( bool expanded )
{
  if ( expanded ) {
    mExpandButton->setPixmap( mExpandedPixmap );
  } else {
    mExpandButton->setPixmap( mNotExpandedPixmap );
  }
}

void KOAgendaView::clearSelection()
{
  mAgenda->deselectItem();
  mAllDayAgenda->deselectItem();
}
