// 	$Id$	

#include <qpopupmenu.h>
#include <qfont.h>
#include <qfontmet.h>
#include <qkeycode.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <kiconloader.h>

#include "calprinter.h"
#include "koprefs.h"

#include "komonthview.h"
#include "komonthview.moc"

void KNoScrollListBox::keyPressEvent(QKeyEvent *e) 
{
  switch(e->key()) {
  case Key_Right:
//    setXOffset(xOffset()+2);
    break; 
  case Key_Left:
//    setXOffset(xOffset()-2);
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

EventListBoxItem::EventListBoxItem(const char *s)
  : QListBoxItem()
{ 
  setText(s); 
  alarmPxmp = UserIcon("bell");
  recurPxmp = UserIcon("recur");
  recur = false;
  alarm = false;
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
  QFontMetrics fm = p->fontMetrics();
  int yPos;
  int pmheight = QMAX(recurPxmp.height(), alarmPxmp.height());
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
  return QMAX(recurPxmp.height(), 
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
  
  return(x + lb->fontMetrics().width(text())+1);
}

KSummaries::KSummaries(QWidget    *parent, 
                       CalObject  *cal,
                       QDate       qd,
                       int         index,
                       const char *name)
  :KNoScrollListBox(parent, name)
{
  idx = index;
  myCal = cal;
  myDate = qd;

  setFont(QFont("Helvetica", 10));
  currIdxs = new QIntDict<KOEvent>(101); /* nobody should have more
                                             than 101 events on any
                                             given day. */
  connect(this, SIGNAL(highlighted(int)), this, SLOT(itemHighlighted(int)));
  connect(this, SIGNAL(selected(int)), this, SLOT(itemSelected(int)));
//  calUpdated();
}

void KSummaries::calUpdated()
{
  setAutoUpdate(FALSE);
  setBackgroundMode(PaletteBase);
  clear();
  currIdxs->clear();

  QList<KOEvent> events;

  // 2nd arg is TRUE because we want the events to be sorted.
  events = myCal->getEventsForDate(myDate, TRUE);

  // add new listitems if neccessary.
  EventListBoxItem *elitem;
  QString sumString;
  unsigned int i = 0;
  KOEvent *anEvent;
  for(anEvent = events.first(); anEvent; anEvent = events.next()) {
    if (anEvent->isMultiDay()) {
      if (myDate == anEvent->getDtStart().date()) {
        sumString = "(---- " + anEvent->getSummary();
      } else if (myDate == anEvent->getDtEnd().date()) {
        sumString = anEvent->getSummary() + " ----)";
      } else if (!(anEvent->getDtStart().date().daysTo(myDate) % 7)) {
        sumString = "---- " + anEvent->getSummary() + "----";
      } else {
        sumString = "----------------";
      }
    } else {
      if (anEvent->doesFloat())
        sumString = anEvent->getSummary();
      else {
        sumString = KGlobal::locale()->formatTime(anEvent->getDtStart().time());
        sumString += " " + anEvent->getSummary();
      }
    }

    elitem = new EventListBoxItem(sumString);
    elitem->setRecur(anEvent->doesRecur());
    elitem->setAlarm(anEvent->getAlarmRepeatCount() > 0);
    insertItem(elitem);
    currIdxs->insert(i++, anEvent);
  }

  // insert due todos
  events=myCal->getTodosForDate(myDate);
  for(anEvent = events.first(); anEvent; anEvent = events.next()) {
    sumString = "";
    if (anEvent->hasDueDate()) {
      if (!anEvent->doesFloat()) {
        sumString += KGlobal::locale()->formatTime(anEvent->getDtDue().time());
        sumString += " ";
      }
    }
    sumString += i18n("Todo: ") + anEvent->getSummary();

    elitem = new EventListBoxItem(sumString);
    insertItem(elitem);
    currIdxs->insert(i++, anEvent);
  }

  setAutoUpdate(TRUE);
  repaint();
}

KOEvent *KSummaries::getSelected()
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
    qDebug("KSummaries::itemHighlighted(int) called with argument %d",index);
  else {
    itemIndex = index;
    emit daySelected(idx);
  }
}

void KSummaries::itemSelected(int index)
{
    KOEvent *anEvent;
    
    anEvent = currIdxs->find(index);
    if (!anEvent)
      debug("error, event not found in dictionary");
    else
      emit editEventSignal(anEvent);
}

KOMonthView::KOMonthView(CalObject *cal,
                           QWidget    *parent,
                           const char *name,
                           QDate       qd)
    : KOBaseView(cal,parent, name)
{
    KIconLoader *loader = KGlobal::iconLoader();
    QPixmap pixmap;
    int i;
    
    selDateIdxs.setAutoDelete(TRUE);
    selDates.setAutoDelete(TRUE);
    
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
    pixmap = loader->loadIcon("3uparrow",KIcon::User);
    KONavButton *upYear = new KONavButton(pixmap,cFrame);
    QToolTip::add(upYear, i18n("Go back one year"));    

    pixmap = loader->loadIcon("2uparrow",KIcon::User);
    KONavButton *upMonth = new KONavButton(pixmap, cFrame);
    QToolTip::add(upMonth, i18n("Go back one month"));    

    pixmap = loader->loadIcon("1uparrow",KIcon::User);
    KONavButton *upWeek = new KONavButton(pixmap, cFrame);
    QToolTip::add(upWeek, i18n("Go back one week"));

    pixmap = loader->loadIcon("1downarrow",KIcon::User);
    KONavButton *downWeek = new KONavButton(pixmap, cFrame); 
    QToolTip::add(downWeek, i18n("Go forward one week"));  

    pixmap = loader->loadIcon("2downarrow",KIcon::User);
    KONavButton *downMonth = new KONavButton(pixmap, cFrame);    
    QToolTip::add(downMonth, i18n("Go forward one month"));

    pixmap = loader->loadIcon("3downarrow",KIcon::User);
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

        daySummaries[i] = new KSummaries(vFrame, mCalendar, date, i);
        daySummaries[i]->setFrameStyle(QFrame::NoFrame);
        connect(daySummaries[i], SIGNAL(daySelected(int)),
                this, SLOT(daySelected(int)));
        connect(daySummaries[i], SIGNAL(editEventSignal(KOEvent *)),
                this, SIGNAL(editEventSignal(KOEvent *)));
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

QList<KOEvent> KOMonthView::getSelected()
{
  QList<KOEvent> selectedEvents;

  uint i;
  for(i=0; i<selDateIdxs.count(); ++i) {
    KOEvent *event = daySummaries[*selDateIdxs.at(i)]->getSelected();
    if (event) selectedEvents.append(event);
  }
  
  return selectedEvents;

/*
  int which;

  if (selDateIdxs.count() == 0)
    which = 0;
  else
    which = *selDateIdxs.first();
  
  if (which) {
    return daySummaries[which]->getSelected();
  } else {
    return (KOEvent *) 0L;
  }
*/
}

void KOMonthView::printPreview(CalPrinter *calPrinter, const QDate &fd,
                               const QDate &td)
{
  calPrinter->preview(CalPrinter::Month, fd, td);
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
  }
  
  viewChanged();
}

void KOMonthView::goBackYear()
{
  int which;

  if (selDateIdxs.count() == 0)
    which = 0;
  else
    which = *selDateIdxs.first();

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
    which = *selDateIdxs.first();
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
    which = *selDateIdxs.first();
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
    which = *selDateIdxs.first();
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

void KOMonthView::selectDates(const QDateList dateList)
{
  QDateList tmpList(FALSE);

  tmpList = dateList;

  QDate qd = *(tmpList.first());


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

void KOMonthView::selectEvents(QList<KOEvent>)
{
  qDebug("KOMonthView::selectEvents is not implemented yet.");
}

void KOMonthView::changeEventDisplay(KOEvent *, int)
{
  // this should be re-written to be much more efficient, but this
  // quick-and-dirty-hack gets the job done for right now.
  updateView();
}

void KOMonthView::viewChanged()
{
  unsigned int i;
  int *idx;
  QDate date = myDate.addDays(weekStartsMonday ? 1 : 0);

  for(i=0; i<42; i++, date = date.addDays(1)) {
    QString daynum;
    daynum.setNum(date.day());
    if(date.day() == 1) {
      daynum.prepend(" ");
      daynum.prepend(KGlobal::locale()->monthName(date.month(), true));
    }

    // add holiday, if present
    QString hstring(mCalendar->getHolidayForDate(date));
    if (!hstring.isEmpty()) {
      daynum.prepend(" ");
      daynum.prepend(hstring);
      dayHeaders[i]->setPalette(holidayPalette);
    } else {
      dayHeaders[i]->setPalette(palette());
    }

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
  for(i=0, idx = selDateIdxs.first(); 
      i < selDateIdxs.count(), idx != 0;
      i++, idx = selDateIdxs.next()) {
    //debug("selDateIdxs.count(): %d",selDateIdxs.count());
    daySelected(*idx);
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
  int *idx;
  QDateList dateList;

  for(i=0, idx = selDateIdxs.first();
      i < selDateIdxs.count();
      i++, idx = selDateIdxs.next()) {
    if(*idx != index) {
      dayHeaders[*idx]->setActivated(FALSE);
      daySummaries[*idx]->clearSelection();  // calls daySelected
    }
  }
  selDateIdxs.clear();
  dayHeaders[index]->setActivated(TRUE);
  daySummaries[index]->setFocus();  // calls daySelected
  selDateIdxs.append(new int(index));
  
  dateList.setAutoDelete(TRUE);
  dateList.append(new QDate(daySummaries[index]->getDate()));

  emit datesSelected(dateList);
  dateList.clear();

  processSelectionChange();
}

void KOMonthView::newEventSlot(int index)
{
  emit newEventSignal(daySummaries[index]->getDate());
}

void KOMonthView::doRightClickMenu()
{
  showEventPopup(rightClickMenu,getSelected().first());
}

void KOMonthView::processSelectionChange()
{
  QList<KOEvent> events = getSelected();
  if (events.count() > 0) {
    emit eventsSelected(true);
//    qDebug("KOMonthView::processSelectionChange() true");
  } else {
    emit eventsSelected(false);
//    qDebug("KOMonthView::processSelectionChange() false");
  }
}
