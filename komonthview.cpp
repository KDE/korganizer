/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
*/

// $Id$

#include <qpopupmenu.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qkeycode.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qpainter.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kiconloader.h>

#ifndef KORG_NOPRINTER
#include "calprinter.h"
#endif
#include "koprefs.h"
#ifndef KORG_NOPLUGINS
#include "kocore.h"
#endif

#include "komonthview.h"
#include "komonthview.moc"

KNoScrollListBox::KNoScrollListBox(QWidget *parent,const char *name)
  : QListBox(parent, name)
{
//      clearTableFlags();
//      setTableFlags(Tbl_clipCellPainting | Tbl_cutCellsV | Tbl_snapToVGrid |
//		    Tbl_scrollLastHCell| Tbl_smoothHScrolling);
}

void KNoScrollListBox::keyPressEvent(QKeyEvent *e)
{
  switch(e->key()) {
  case Key_Right:
    scrollBy(4,0);
    break;
  case Key_Left:
    scrollBy(-4,0);
    break;
  case Key_Up:
    if(!count()) break;
    setCurrentItem((currentItem()+count()-1)%count());
    if(!itemVisible(currentItem())) {
      if((unsigned int) currentItem() == (count()-1)) {
        setTopItem(currentItem()-numItemsVisible()+1);
      } else {
        setTopItem(topItem()-1);
      }
    }
    break;
  case Key_Down:
    if(!count()) break;
    setCurrentItem((currentItem()+1)%count());
    if(!itemVisible(currentItem())) {
      if(currentItem() == 0) {
        setTopItem(0);
      } else {
        setTopItem(topItem()+1);
      }
    }
  case Key_Shift:
    emit shiftDown();
    break;
  default:
    break;
  }
}

void KNoScrollListBox::keyReleaseEvent(QKeyEvent *e)
{
  switch(e->key()) {
  case Key_Shift:
    emit shiftUp();
    break;
  default:
    break;
  }
}

void KNoScrollListBox::mousePressEvent(QMouseEvent *e)
{
  QListBox::mousePressEvent(e);

  if(e->button() == RightButton) {
    emit rightClick();
  }
}


EventListBoxItem::EventListBoxItem(const QString & s)
  : QListBoxItem()
{
  setText(s);
  alarmPxmp = SmallIcon("bell");
  recurPxmp = SmallIcon("recur");
  replyPxmp = SmallIcon("mail_reply");
  recur = false;
  alarm = false;
  reply = false;
}

void EventListBoxItem::paint(QPainter *p)
{
  int x = 3;
  if(recur) {
    p->drawPixmap(x, 0, recurPxmp);
    x += recurPxmp.width()+2;
  }
  if(alarm) {
    p->drawPixmap(x, 0, alarmPxmp);
    x += alarmPxmp.width()+2;
  }
  if(reply) {
    p->drawPixmap(x, 0, replyPxmp);
    x += replyPxmp.width()+2;
  }
  QFontMetrics fm = p->fontMetrics();
  int yPos;
  int pmheight = QMAX(recurPxmp.height(), QMAX(alarmPxmp.height(),replyPxmp.height()) );
  if(pmheight < fm.height())
    yPos = fm.ascent() + fm.leading()/2;
  else
    yPos = pmheight/2 - fm.height()/2  + fm.ascent();


  // currently disabled because selected items drawn in black!
  //p->setPen(palette().normal().text());
  p->drawText(x, yPos, text());
}

int EventListBoxItem::height(const QListBox *lb) const
{
  return QMAX(
        QMAX(recurPxmp.height(), replyPxmp.height()),
	      QMAX(alarmPxmp.height(), lb->fontMetrics().lineSpacing()+1));
}

int EventListBoxItem::width(const QListBox *lb) const
{
  int x = 3;
  if(recur) {
    x += recurPxmp.width()+2;
  }
  if(alarm) {
    x += alarmPxmp.width()+2;
  }
  if(reply) {
    x += replyPxmp.width()+2;
  }

  return(x + lb->fontMetrics().width(text())+1);
}


KSummaries::KSummaries(QWidget    *parent,
                       Calendar  *cal,
                       QDate       qd,
                       int         index,
                       const char *name)
  : KNoScrollListBox(parent, name)
{
  idx = index;
  myCal = cal;
  myDate = qd;

  setFont(QFont("Helvetica", 10));
  currIdxs = new QIntDict<Event>(101); /* nobody should have more
                                             than 101 events on any
                                             given day. */
  connect(this, SIGNAL(highlighted(int)), this, SLOT(itemHighlighted(int)));
  connect(this, SIGNAL(selected(int)), this, SLOT(itemSelected(int)));
//  calUpdated();
}

void KSummaries::calUpdated()
{
//  kdDebug() << "KSummaries::calUpdated()" << endl;

  setBackgroundMode(PaletteBase);
  clear();
  currIdxs->clear();

  QPtrList<Event> events;

  // 2nd arg is TRUE because we want the events to be sorted.
  events = myCal->getEventsForDate(myDate, TRUE);

  // add new listitems if neccessary.
  EventListBoxItem *elitem;
  QString sumString;
  unsigned int i = 0;
  Event *anEvent;
  for(anEvent = events.first(); anEvent; anEvent = events.next()) {
    if (anEvent->isMultiDay()) {
      if (myDate == anEvent->dtStart().date()) {
        sumString = "(---- " + anEvent->summary();
      } else if (myDate == anEvent->dtEnd().date()) {
        sumString = anEvent->summary() + " ----)";
      } else if (!(anEvent->dtStart().date().daysTo(myDate) % 7)) {
        sumString = "---- " + anEvent->summary() + "----";
      } else {
        sumString = "----------------";
      }
    } else {
      if (anEvent->doesFloat())
        sumString = anEvent->summary();
      else {
        sumString = KGlobal::locale()->formatTime(anEvent->dtStart().time());
        sumString += " " + anEvent->summary();
      }
    }

    elitem = new EventListBoxItem(sumString);
    elitem->setRecur(anEvent->recurrence()->doesRecur());
    elitem->setAlarm(anEvent->isAlarmEnabled());

    Attendee *me = anEvent->attendeeByMail(KOPrefs::instance()->email());
    if (me!=0) {
      if (me->status()==Attendee::NeedsAction && me->RSVP()) elitem->setReply(true);
      else elitem->setReply(false);
    }
    else elitem->setReply(false);

    insertItem(elitem);
    currIdxs->insert(i++, anEvent);
  }

  // insert due todos
  QPtrList<Todo> todos=myCal->getTodosForDate(myDate);
  Todo *todo;
  for(todo = todos.first(); todo; todo = todos.next()) {
    sumString = "";
    if (todo->hasDueDate()) {
      if (!todo->doesFloat()) {
        sumString += KGlobal::locale()->formatTime(todo->dtDue().time());
        sumString += " ";
      }
    }
    sumString += i18n("To-Do: ") + todo->summary();

    elitem = new EventListBoxItem(sumString);
    insertItem(elitem);
// TODO: check, how to index todos
//    currIdxs->insert(i++, anEvent);
  }

  repaint();

//  kdDebug() << "KSummaries::calUpdated() done" << endl;
}

Event *KSummaries::getSelected()
{
  if (currentItem() < 0) return 0;
  else return currIdxs->find(currentItem());
};

// QScrollView assumes a minimum size of (100,100). That's too much for us.
QSize KSummaries::minimumSizeHint() const
{
  return QSize(10+frameWidth()*2,
               10+frameWidth()*2);
}

void KSummaries::setDate(QDate qd)
{
  myDate = qd;
  calUpdated();
}

void KSummaries::itemHighlighted(int index)
{
  if (index < 0)
    kdDebug() << "KSummaries::itemHighlighted(int) called with argument " << index << endl;
  else {
    itemIndex = index;
    emit daySelected(idx);
  }
}

void KSummaries::itemSelected(int index)
{
    Event *anEvent;

    anEvent = currIdxs->find(index);
    if (!anEvent)
      kdDebug() << "error, event not found in dictionary" << endl;
    else
      emit editEventSignal(anEvent);
}

KOMonthView::KOMonthView(Calendar *cal,
                           QWidget    *parent,
                           const char *name,
                           QDate       qd)
    : KOEventView(cal,parent, name)
{
    KIconLoader *loader = KGlobal::iconLoader();
    QPixmap pixmap;
    int i;

    // top layout of monthview
    QBoxLayout *topLayout = new QVBoxLayout(this);

    // frame for day and navigation button frames
    QFrame *mainFrame = new QHBox(this,"monthview main frame");
    mainFrame->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);

    // frame for days
    QFrame *vFrame = new QFrame(mainFrame);
    vFrame->setFrameStyle(QFrame::NoFrame);
    vFrame->setBackgroundColor(QColor(black));

    // frame for navigation buttons
    QVBox *cFrame = new QVBox(mainFrame);
    cFrame->setFrameStyle(QFrame::Panel|QFrame::Raised);

    // Create navigation buttons
    pixmap = loader->loadIcon("3uparrow",KIcon::Small);
    KONavButton *upYear = new KONavButton(pixmap,cFrame);
    QToolTip::add(upYear, i18n("Go back one year"));

    pixmap = loader->loadIcon("2uparrow",KIcon::Small);
    KONavButton *upMonth = new KONavButton(pixmap, cFrame);
    QToolTip::add(upMonth, i18n("Go back one month"));

    pixmap = loader->loadIcon("1uparrow",KIcon::Small);
    KONavButton *upWeek = new KONavButton(pixmap, cFrame);
    QToolTip::add(upWeek, i18n("Go back one week"));

    pixmap = loader->loadIcon("1downarrow",KIcon::Small);
    KONavButton *downWeek = new KONavButton(pixmap, cFrame);
    QToolTip::add(downWeek, i18n("Go forward one week"));

    pixmap = loader->loadIcon("2downarrow",KIcon::Small);
    KONavButton *downMonth = new KONavButton(pixmap, cFrame);
    QToolTip::add(downMonth, i18n("Go forward one month"));

    pixmap = loader->loadIcon("3downarrow",KIcon::Small);
    KONavButton *downYear = new KONavButton(pixmap, cFrame);
    QToolTip::add(downYear, i18n("Go forward one year"));

    connect(upYear,    SIGNAL(clicked()), this, SLOT(goBackYear()));
    connect(upMonth,   SIGNAL(clicked()), this, SLOT(goBackMonth()));
    connect(upWeek,    SIGNAL(clicked()), this, SLOT(goBackWeek()));
    connect(downWeek,  SIGNAL(clicked()), this, SLOT(goForwardWeek()));
    connect(downMonth, SIGNAL(clicked()), this, SLOT(goForwardMonth()));
    connect(downYear,  SIGNAL(clicked()), this, SLOT(goForwardYear()));

    myDate = qd.addDays(-(qd.dayOfWeek()) - 7);

    QFont bfont = font();
    bfont.setBold(TRUE);

    // make a 13 row, 7 column grid. 1 column for each day of the week,
    // one row for the headers and two rows
    // for each of 6 weeks (one for the day header and one for the
    // summaries.
    QGridLayout *dayLayout = new QGridLayout(vFrame, 19, 14);

    // create the day of the week labels (Sun, Mon, etc) and add them to
    // the layout.
    shortdaynames = TRUE;
    for(i=0; i<7; i++) {
        dayNames[i] = new QLabel(vFrame);
        dayNames[i]->setText("FOO"); // will be replaced by updateConfig();
        dayNames[i]->setFont(bfont);
        dayNames[i]->setFrameStyle(QFrame::Panel|QFrame::Raised);
        dayNames[i]->setLineWidth(1);
        dayNames[i]->adjustSize();
        dayNames[i]->setAlignment(AlignCenter);
        dayNames[i]->setMinimumHeight(dayNames[i]->height());
        dayLayout->addMultiCellWidget(dayNames[i], 0, 0, i*2, i*2+1);
        dayLayout->setColStretch(i*2, 1);
        dayLayout->setColStretch(i*2+1, 0);
        dayLayout->addColSpacing(i*2+1, 1);
    }

    QDate date = myDate;

    // create the days, and add them to the layout.
    for(i=0; i < 42; i++, date = date.addDays(1)) {
        // text is irrelevant here, just needs to be something to set the
        // initial size on (to reduce flicker on initial resize).
        QString daynum;
        daynum.setNum(date.day());
        if(date.day() == 1) {
            daynum.prepend(" ");
            daynum.prepend(KGlobal::locale()->monthName(date.month(), true));
        }
        dayHeaders[i] = new KSelLabel(vFrame, daynum, i);
        dayHeaders[i]->setFont(bfont);
        dayHeaders[i]->adjustSize();
        dayHeaders[i]->setMinimumHeight(dayHeaders[i]->height());
        connect(dayHeaders[i], SIGNAL(labelActivated(int)),
                this, SLOT(daySelected(int)));
        connect(dayHeaders[i], SIGNAL(newEventSignal(int)),
                this, SLOT(newEventSlot(int)));

        daySummaries[i] = new KSummaries(vFrame, calendar(), date, i);
        daySummaries[i]->setFrameStyle(QFrame::NoFrame);
        connect(daySummaries[i], SIGNAL(daySelected(int)),
                this, SLOT(daySelected(int)));
        connect(daySummaries[i], SIGNAL(editEventSignal(Event *)),
                this, SIGNAL(editEventSignal(Event *)));
        connect(daySummaries[i], SIGNAL(rightClick()),
                this, SLOT(doRightClickMenu()));

        dayLayout->addWidget(dayHeaders[i], i/7*3+1, (i%7)*2);
        dayLayout->addWidget(daySummaries[i], i/7*3+2, (i%7)*2);
    }
    dayLayout->setRowStretch(0, 0);
    for(i=0; i<6; i++) {
        dayLayout->setRowStretch(i*3+1, 0);
        dayLayout->setRowStretch(i*3+2, 1);
        dayLayout->setRowStretch(i*3+3, 0);
        dayLayout->addRowSpacing(i*3+3, 1);
    }

    dispLabel = new QLabel(this);
    dispLabel->setFrameStyle(QFrame::Panel|QFrame::Raised);
    dispLabel->setAlignment(AlignCenter);
    QString tstring = KGlobal::locale()->formatDate(daySummaries[0]->getDate())
        + " - " + KGlobal::locale()->formatDate(daySummaries[41]->getDate());
    dispLabel->setText(tstring);
    dispLabel->setFont(bfont);

    topLayout->addWidget(dispLabel);
    topLayout->addWidget(mainFrame);

    rightClickMenu = eventPopup();

    updateConfig();

    emit eventsSelected(false);
}


KOMonthView::~KOMonthView()
{
  delete rightClickMenu;
}

int KOMonthView::maxDatesHint()
{
  // 6 weeks = 42 days. This is enough to display all weeks of one month,
  // regardless of the starting day of the week.
  return 42;  // What was the question? :-)
}

int KOMonthView::currentDateCount()
{
  return 42;
}

QPtrList<Incidence> KOMonthView::selectedIncidences()
{
  QPtrList<Incidence> selectedEvents;

  uint i;
  for(i=0; i<selDateIdxs.count(); ++i) {
    Event *event = daySummaries[selDateIdxs[i]]->getSelected();
    if (event) selectedEvents.append(event);
  }

  return selectedEvents;
}

void KOMonthView::printPreview(CalPrinter *calPrinter, const QDate &fd,
                               const QDate &td)
{
#ifndef KORG_NOPRINTER
  calPrinter->preview(CalPrinter::Month, fd, td);
#endif
}

void KOMonthView::updateConfig()
{
  weekStartsMonday = KGlobal::locale()->weekStartsMonday();

  for (int i = 0; i < 7; i++) {
    if (weekStartsMonday) {
      dayNames[i]->setText(KGlobal::locale()->weekDayName(i+1));
    } else {
      if (i==0) dayNames[i]->setText(KGlobal::locale()->weekDayName(7));
      else dayNames[i]->setText(KGlobal::locale()->weekDayName(i));
    }
  }

  holidayPalette = palette();
  holidayPalette.setColor(QColorGroup::Foreground,
                          KOPrefs::instance()->mHolidayColor);
  holidayPalette.setColor(QColorGroup::Text,
                          KOPrefs::instance()->mHolidayColor);

  QFont newFont = KOPrefs::instance()->mMonthViewFont;
  newFont.setBold(false);

  QFont hfont(newFont);
  hfont.setBold(true);

  for (int i = 0; i < 42; i++) {
    dayHeaders[i]->setFont(hfont);
    daySummaries[i]->setFont(newFont);
    if (KOPrefs::instance()->mEnableMonthScroll) {
      daySummaries[i]->setVScrollBarMode(QScrollView::Auto);
      daySummaries[i]->setHScrollBarMode(QScrollView::Auto);
    } else {
      daySummaries[i]->setVScrollBarMode(QScrollView::AlwaysOff);
      daySummaries[i]->setHScrollBarMode(QScrollView::AlwaysOff);
    }
  }

  viewChanged();
}

void KOMonthView::goBackYear()
{
  int which;

  if (selDateIdxs.count() == 0)
    which = 0;
  else
    which = selDateIdxs.first();

  QDate date = daySummaries[which]->getDate();
  date.setYMD(date.year()-1, date.month(), date.day());
  myDate = date.addDays(-(date.dayOfWeek()));

  while (!myDate.isValid())
    myDate = myDate.addDays(-1);

  viewChanged();
  daySelected(myDate.daysTo(date));
}

void KOMonthView::goForwardYear()
{
  int which;

  if (selDateIdxs.count() == 0)
    which = 0;
  else
    which = selDateIdxs.first();
  QDate date = daySummaries[which]->getDate();
  date.setYMD(date.year()+1, date.month(), date.day());
  myDate = date.addDays(-(date.dayOfWeek()));
  while (!myDate.isValid())
    myDate = myDate.addDays(-1);
  viewChanged();
  daySelected(myDate.daysTo(date));
}

void KOMonthView::goBackMonth()
{
  int which;

  if (selDateIdxs.count() == 0)
    which = 0;
  else
    which = selDateIdxs.first();
  QDate date = daySummaries[which]->getDate();
  if (date.month() == 1) {
    date.setYMD(date.year()-1, 12, 1);
    myDate = date;
    //while (!myDate.isValid())
    //  myDate = myDate.addDays(-1);
  } else {
    date.setYMD(date.year(), date.month()-1, date.day());
    myDate = date.addDays(-(date.dayOfWeek()));
    while (!myDate.isValid())
      myDate = myDate.addDays(-1);
  }
  myDate = date.addDays(-(date.dayOfWeek()));
  viewChanged();
  daySelected(myDate.daysTo(date));
}

void KOMonthView::goForwardMonth()
{
  int which;

  if (selDateIdxs.count() == 0)
    which = 0;
  else
    which = selDateIdxs.first();
  QDate date = daySummaries[which]->getDate();
  if (date.month() == 12) {

    date.setYMD(date.year()+1, 1, 1);
    myDate = date;
    /*
    myDate = date.addDays(-(date.dayOfWeek()));
    while (!myDate.isValid())
    myDate = myDate.addDays(-1);*/
  } else {
      date.setYMD(date.year(), date.month()+1, date.day());
      myDate = date.addDays(-(date.dayOfWeek()));
      while (!myDate.isValid())
              myDate = myDate.addDays(-1);
  }
  myDate = date.addDays(-(date.dayOfWeek()));

  viewChanged();
  daySelected(myDate.daysTo(date));
}

void KOMonthView::goBackWeek()
{
  myDate = myDate.addDays(-7);
  while (!myDate.isValid())
    myDate = myDate.addDays(-1);
  viewChanged();
}

void KOMonthView::goForwardWeek()
{
  myDate = myDate.addDays(7);
  while (!myDate.isValid())
    myDate = myDate.addDays(-1);
  viewChanged();
}

void KOMonthView::showDates(const QDate &start, const QDate &)
{
  QDate qd = start;

  // check to see if we're going to a currently selected date.
  // commented out because it is causing problems where the view isn't
  // refreshed when a different calendar is opened with the same date
  // selected.  bad news...
  /*unsigned int i;
  int *idx;
  for(i=0, idx = selDateIdxs.first();
      i < selDateIdxs.count();
      i++, idx = selDateIdxs.next()) {
    if(daySummaries[*idx]->getDate() == qd) return;
    }*/

  // nope, go to the date.
  if(qd < myDate || qd > myDate.addDays(42)) {
    // the view has to change to accomodate this action.
    myDate = qd.addDays(-(qd.dayOfWeek()) - 7);
  } else {
    // otherwise, just set the seleted day in this view.
    daySelected(myDate.daysTo(qd) - (weekStartsMonday ? 1 : 0));
  }
  viewChanged();
}

void KOMonthView::showEvents(QPtrList<Event>)
{
  kdDebug() << "KOMonthView::selectEvents is not implemented yet." << endl;
}

void KOMonthView::changeEventDisplay(Event *, int)
{
  // this should be re-written to be much more efficient, but this
  // quick-and-dirty-hack gets the job done for right now.
  updateView();
}

void KOMonthView::viewChanged()
{
  QDate date = myDate.addDays(weekStartsMonday ? 1 : 0);

  unsigned int i;
  for(i=0; i<42; i++, date = date.addDays(1)) {
    QString daynum;
    daynum.setNum(date.day());
    if(date.day() == 1) {
      daynum.prepend(" ");
      daynum.prepend(KGlobal::locale()->monthName(date.month(), true));
    }

#ifndef KORG_NOPLUGINS
    // add holiday, if present
    QString hstring(KOCore::self()->holiday(date));
    if (!hstring.isEmpty()) {
      daynum.prepend(" ");
      daynum.prepend(hstring);
      dayHeaders[i]->setPalette(holidayPalette);
    } else {
      dayHeaders[i]->setPalette(palette());
    }
#endif

    dayHeaders[i]->setText(daynum);
    daySummaries[i]->setDate(date);
    if (date.month() % 2) {
      dayHeaders[i]->setBackgroundMode(PaletteBackground);
      daySummaries[i]->setBackgroundMode(PaletteBackground);
    } else {
      dayHeaders[i]->setBackgroundMode(PaletteLight);
      daySummaries[i]->setBackgroundMode(PaletteLight);
    }
  }
  QString tstring = KGlobal::locale()->formatDate(daySummaries[0]->getDate())
       + " - " + KGlobal::locale()->formatDate(daySummaries[41]->getDate());
  dispLabel->setText(tstring);
  for(i=0; i < selDateIdxs.count(); i++) {
    //kdDebug() << "selDateIdxs.count(): " << selDateIdxs.count() << endl;
    daySelected(selDateIdxs[i]);
  }

  processSelectionChange();
}

void KOMonthView::updateView()
{
  int i;
  //  QDate date = myDate.addDays(weekStartsMonday ? 1 : 0);

  for(i=0; i<42; i++/*, date = date.addDays(1)*/) {
    /*    if (date.month() % 2)
      daySummaries[i]->setBackgroundMode(PaletteBackground);
    else
    daySummaries[i]->setBackgroundMode(PaletteLight);*/
    daySummaries[i]->calUpdated();
  }

  processSelectionChange();
}

void KOMonthView::resizeEvent(QResizeEvent *)
{
  QFontMetrics fontmetric(dayNames[0]->font());
  unsigned int i;

  // select the appropriate heading string size. "Wednesday" or "Wed".
  // note this only changes the text if the requested size crosses the
  // threshold between big enough to support the full name and not big
  // enough.
  if(dayNames[0]->width() < fontmetric.width("Wednesday")+4) {
    if (!shortdaynames) {
      updateConfig();
      for(i=0; i<7; i++) {
        QString tmpStr = dayNames[i]->text();
        tmpStr.truncate(3);
        dayNames[i]->setText(tmpStr);
      }
      shortdaynames = TRUE;
    }
  } else {
    if (shortdaynames) {
      updateConfig();
      shortdaynames = FALSE;
    }
  }
}


void KOMonthView::daySelected(int index)
{
  unsigned int i;
  for(i=0; i < selDateIdxs.count(); i++ ) {
    int idx = selDateIdxs[i];
    if(idx != index) {
      dayHeaders[idx]->setActivated(FALSE);
      daySummaries[idx]->clearSelection();  // calls daySelected
    }
  }
  selDateIdxs.clear();
  dayHeaders[index]->setActivated(TRUE);
  daySummaries[index]->setFocus();  // calls daySelected
  selDateIdxs.append(index);

  DateList dateList;
  dateList.append(daySummaries[index]->getDate());

  emit datesSelected(dateList);

  processSelectionChange();
}

void KOMonthView::newEventSlot(int index)
{
  emit newEventSignal(daySummaries[index]->getDate());
}

void KOMonthView::doRightClickMenu()
{
  Incidence *incidence = selectedIncidences().first();
  if( incidence->type() == "Event" ) {
    Event *event = static_cast<Event *>(incidence);
    rightClickMenu->showEventPopup(event);
  } else {
    kdDebug() << "MonthView::doRightClickMenu(): cast failed." << endl;
  }
}

void KOMonthView::processSelectionChange()
{
  QPtrList<Incidence> events = selectedIncidences();
  if (events.count() > 0) {
    emit eventsSelected(true);
//    kdDebug() << "KOMonthView::processSelectionChange() true" << endl;
  } else {
    emit eventsSelected(false);
//    kdDebug() << "KOMonthView::processSelectionChange() false" << endl;
  }
}
