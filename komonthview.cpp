// 	$Id$	

#include <qpopupmenu.h>
#include <qfont.h>
#include <qfontmet.h>
#include <qkeycode.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qpushbutton.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <kiconloader.h>

#include "calprinter.h"

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
  EventListBoxItem *elitem;
  QString sumstring, ampm, tmpstring;
  KOEvent *anEvent;
  QTime t1;
  int h;
  unsigned int i;

  setAutoUpdate(FALSE);
  setBackgroundMode(PaletteBase);
  clear();
  currIdxs->clear();
  // 2nd arg is TRUE because we want the events to be sorted.
  events = myCal->getEventsForDate(myDate, TRUE);

  // add new listitems if neccessary.
  for(i = 0, anEvent = events.first();
      anEvent != 0; i++, anEvent = events.next()) {
    t1 = anEvent->getDtStart().time();
    h = t1.hour();
    if(h == 0) {
      h = 12;
      ampm = "a";
    } else if(h > 11) {
      ampm = "p";
      if(h != 12) {
        h -= 12;
      }
    } else {
      ampm = "a";
    }
    if (anEvent->isMultiDay()) {
      if (myDate == anEvent->getDtStart().date()) {
        sumstring.sprintf("(---- %s", anEvent->getSummary().data());
      } else if (myDate == anEvent->getDtEnd().date()) {
        sumstring.sprintf("%s ----)", anEvent->getSummary().data());
      } else if (!(anEvent->getDtStart().date().daysTo(myDate) % 7)) {
        sumstring.sprintf("---- %s ----", anEvent->getSummary().data());
      } else {
        sumstring.sprintf("----------------");
      }
    } else {
      if (anEvent->doesFloat())
        sumstring = anEvent->getSummary();
      else {
        if (timeAmPm) {
          sumstring.sprintf("%2d:%02d%s %s", h, t1.minute(), ampm.data(),
                            anEvent->getSummary().data());
        } else {
          sumstring.sprintf("%2d:%02d %s", t1.hour(), t1.minute(),
                            anEvent->getSummary().data());
        }
      }
    }

    //    sumstring.detach();
    elitem = new EventListBoxItem(sumstring);
    elitem->setRecur(anEvent->doesRecur());
    elitem->setAlarm(anEvent->getAlarmRepeatCount() > 0);
    insertItem(elitem);
    currIdxs->insert(i, anEvent);
  }
  setAutoUpdate(TRUE);
  repaint();
}

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
    qDebug("KSummaries::itemHighlighted(int) called with argument %d",
           index);
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
    pixmap = loader->loadIcon("2uparrow",KIcon::User);
    KONavButton *upMonth = new KONavButton(pixmap, cFrame);
    pixmap = loader->loadIcon("1uparrow",KIcon::User);
    KONavButton *upWeek = new KONavButton(pixmap, cFrame);
    pixmap = loader->loadIcon("1downarrow",KIcon::User);
    KONavButton *downWeek = new KONavButton(pixmap, cFrame);    
    pixmap = loader->loadIcon("2downarrow",KIcon::User);
    KONavButton *downMonth = new KONavButton(pixmap, cFrame);    
    pixmap = loader->loadIcon("3downarrow",KIcon::User);
    KONavButton *downYear = new KONavButton(pixmap, cFrame);

    connect(upYear,    SIGNAL(clicked()), this, SLOT(goBackYear()));
    connect(upMonth,   SIGNAL(clicked()), this, SLOT(goBackMonth()));
    connect(upWeek,    SIGNAL(clicked()), this, SLOT(goBackWeek()));
    connect(downWeek,  SIGNAL(clicked()), this, SLOT(goForwardWeek()));
    connect(downMonth, SIGNAL(clicked()), this, SLOT(goForwardMonth()));
    connect(downYear,  SIGNAL(clicked()), this, SLOT(goForwardYear()));

    
    calendar = cal;
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
            daynum.prepend(date.monthName(date.month()));
        }
        dayHeaders[i] = new KSelLabel(vFrame, daynum, i);
        dayHeaders[i]->setFont(bfont);
        dayHeaders[i]->adjustSize();
        dayHeaders[i]->setMinimumHeight(dayHeaders[i]->height());
        connect(dayHeaders[i], SIGNAL(labelActivated(int)),
                this, SLOT(daySelected(int)));
        connect(dayHeaders[i], SIGNAL(newEventSignal(int)),
                this, SLOT(newEventSlot(int)));

        daySummaries[i] = new KSummaries(vFrame, calendar, date, i);
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
    QString tstring = (daySummaries[0]->getDate().toString()+" - "+
                       daySummaries[41]->getDate().toString());
    dispLabel->setText(tstring);
    dispLabel->setFont(bfont);

    topLayout->addWidget(dispLabel);
    topLayout->addWidget(mainFrame);


    rightClickMenu = new QPopupMenu;
    rightClickMenu->insertItem(i18n("New Event"), this,
                               SLOT(newEventSelected()));
    rightClickMenu->insertItem(i18n("&Edit"), this,
                               SLOT(editSelected()));
    rightClickMenu->insertItem(i18n("&Delete"), this,
                               SLOT(deleteSelected()));

    updateConfig();
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
  bool fmt;
  const QString longDayNames[] = { i18n("Sunday"), i18n("Monday"),
                                   i18n("Tuesday"), i18n("Wednesday"),
                                   i18n("Thursday"),
                                   i18n("Friday"), i18n("Saturday") };
  const QString longDayNames2[] = { i18n("Monday"), i18n("Tuesday"),
                                    i18n("Wednesday"), i18n("Thursday"),
                                    i18n("Friday"), i18n("Saturday"),
                                    i18n("Sunday") };
  
  KConfig config(locate("config", "korganizerrc")); 
  config.setGroup("Time & Date");
  weekStartsMonday = config.readBoolEntry("Week Starts Monday", FALSE);
  
  fmt = (config.readNumEntry("Time Format", 1) ? TRUE : FALSE);

  QColor tmpColor("#cc3366");
  config.setGroup("Colors");
  QColor hiliteColor = config.readColorEntry("Holiday Color", &tmpColor);

  holidayPalette = palette();
  QColorGroup myGroup = QColorGroup(palette().normal().foreground(),
                                    palette().normal().background(),
                                    palette().normal().light(),
                                    palette().normal().dark(),
                                    palette().normal().mid(),
                                    hiliteColor,
                                    palette().normal().base());
  holidayPalette.setNormal(myGroup);
  
  for (int i = 0; i < 42; i++)
    daySummaries[i]->setAmPm(fmt);

  for (int i = 0; i < 7; i++)
    dayNames[i]->setText((weekStartsMonday ? longDayNames2[i] :
                          longDayNames[i]));
  
  // set font
  config.setGroup("Fonts");
  QFont defaultFont = font();
  QFont newFont(config.readFontEntry("Month Font", &defaultFont));
  QFont hfont(newFont);
  hfont.setBold(TRUE);
  hfont.setPointSize(newFont.pointSize() + 2);
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

void KOMonthView::selectEvents(QList<KOEvent> eventList)
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
      daynum.prepend(date.monthName(date.month()));
    }

    // add holiday, if present
    QString hstring(calendar->getHolidayForDate(date));;
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
  QString tstring = (daySummaries[0]->getDate().toString()+" - "+
                     daySummaries[41]->getDate().toString());
  dispLabel->setText(tstring);
  for(i=0, idx = selDateIdxs.first(); 
      i < selDateIdxs.count(), idx != 0;
      i++, idx = selDateIdxs.next()) {
    //debug("selDateIdxs.count(): %d",selDateIdxs.count());
    daySelected(*idx);
  }
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
}

void KOMonthView::newEventSlot(int index)
{
  emit newEventSignal(daySummaries[index]->getDate());
}

void KOMonthView::doRightClickMenu()
{
  rightClickMenu->popup(QCursor::pos());
}
