// 	$Id$	

#include <qstring.h>
#include <qtooltip.h>
#include <qkeycode.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kstddirs.h>

#include "kdatenav.h"
#include "kdatenav.moc"

KDateNavigator::KDateNavigator(QWidget *parent, 
			       CalObject *calendar, 
			       bool show_week_nums,
			       const char *name,
			       QDate startDate)
  : QFrame(parent, name)
{
  QPixmap pixmap;
  KDateNavigator::calendar = calendar;
  int i;
  
  setFrameStyle(QFrame::NoFrame);
  setBackgroundMode(PaletteBase);

  if (! startDate.isValid()) {
    debug("KDateNavigator::KDateNavigator(): an invalid date was passed as a parameter!");
    startDate = QDate::currentDate();
  }

  selectedDates.setAutoDelete(TRUE);
  selectedDates.append(new QDate(startDate));
  m_MthYr = startDate;
  m_bShowWeekNums = show_week_nums;

  // Set up the control buttons and date label
  ctrlFrame = new QFrame(this, "KDateNavigator::CtrlFrame");
  ctrlFrame->setFrameStyle(QFrame::Panel|QFrame::Raised);
  ctrlFrame->setLineWidth(1);
  
  QFont tfont = font();
  tfont.setPointSize(10);
  tfont.setBold(FALSE);

  KIconLoader *loader = KGlobal::iconLoader();

  pixmap = loader->loadIcon("2leftarrow",KIcon::User);
  prevYear  = new KPButton(pixmap, ctrlFrame);
  prevYear->setFont(tfont);
  prevYear->resize(16,16);
  QToolTip::add(prevYear, i18n("Previous Year"));

  pixmap = loader->loadIcon("1leftarrow",KIcon::User);
  prevMonth = new KPButton(pixmap, ctrlFrame);
  prevMonth->setFont(tfont);
  prevMonth->setFixedSize(prevYear->size());
  QToolTip::add(prevMonth, i18n("Previous Month"));

  pixmap = loader->loadIcon("1rightarrow",KIcon::User);
  nextMonth = new KPButton(pixmap, ctrlFrame);
  nextMonth->setFont(tfont);
  nextMonth->setFixedSize(prevYear->size());
  QToolTip::add(nextMonth, i18n("Next Month"));

  pixmap = loader->loadIcon("2rightarrow",KIcon::User);
  nextYear  = new KPButton(pixmap, ctrlFrame);
  nextYear->setFont(tfont);
  nextYear->setFixedSize(prevYear->size());
  QToolTip::add(nextYear, i18n("Next Year"));
  
  dateLabel = new QLabel(ctrlFrame);
  dateLabel->setFont(tfont);
  dateLabel->setAlignment(AlignCenter);

  const QString months[] = { i18n("January"), i18n("February"), i18n("March"),
                             i18n("April"),   i18n("May"),      i18n("June"),
                             i18n("July"),    i18n("August"),   i18n("September"),
                             i18n("October"), i18n("November"), i18n("December")
                           };

  QString dtstr = months[startDate.month()-1];
  QString yrstr;
  yrstr.setNum(startDate.year());
  dtstr += " " + yrstr;

  dateLabel->setText(dtstr);
  dateLabel->adjustSize();
  dateLabel->setFixedHeight(prevYear->height()-2);

  connect(prevYear, SIGNAL(clicked()), this, SLOT(goPrevYear()));
  connect(prevMonth, SIGNAL(clicked()), this, SLOT(goPrevMonth()));
  connect(nextMonth, SIGNAL(clicked()), this, SLOT(goNextMonth()));
  connect(nextYear, SIGNAL(clicked()), this, SLOT(goNextYear()));

  viewFrame = new QFrame(this, "KDateNavigator::ViewFrame");
  viewFrame->setFrameStyle(QFrame::NoFrame);
  viewFrame->setLineWidth(0);
  //viewFrame->setBackgroundColor(PaletteBase);

  // get the day of the week on the first day
  QDate dayone(m_MthYr.year(), 
	       m_MthYr.month(), 1);
  m_fstDayOfWk = dayone.dayOfWeek();

  // Set up the heading fields.
  for(i=0; i<7; i++) {
    headings[i] = new QLabel("", viewFrame);
    headings[i]->setFont(QFont("Arial", 10, QFont::Bold));
    headings[i]->setAlignment(AlignCenter);
  }

  headingSep = new QFrame(viewFrame, "KDateNavigator::HeadingSep");
  headingSep->setFrameStyle(QFrame::HLine|QFrame::Plain);
  headingSep->setLineWidth(1);
  headingSep->setFixedHeight(1);

  // Create the weeknumber labels

  QPalette wkno_Palette = palette();
  QColorGroup my_Group = QColorGroup(palette().active().base(),
				     palette().active().base(),
				     palette().active().base(),
				     palette().active().base(),
				     palette().active().base(),
				     palette().active().mid(),
				     palette().active().base());

// Temporarily disabled.
//  wkno_Palette.setActive(my_Group);
//  wkno_Palette.setInactive(my_Group);
  
  for(i=0; i<7; i++) {
    weeknos[i] = new QLabel(viewFrame);
    weeknos[i]->setPalette(wkno_Palette);
    weeknos[i]->setAlignment(AlignRight | AlignVCenter);
    weeknos[i]->setFont(QFont("Arial", 10));
    if(!show_week_nums) {
      weeknos[i]->hide();
    }
  }
  weeknumSep = new QFrame(viewFrame, "KDateNavigator::WeeknumSep");
  weeknumSep->setFrameStyle(QFrame::VLine|QFrame::Plain);
  weeknumSep->setLineWidth(1);
  weeknumSep->setFixedWidth(1);
  weeknumSep->setBackgroundColor(PaletteMid);
  if(!show_week_nums) {
    weeknumSep->hide();
  }
  // If month begins on Monday and Monday is first day of week,
  // month should begin on second line. Sunday doesn't have this problem.
  int nextLine = ((m_fstDayOfWk == 1) && (weekStartsMonday == 1)) ? 7 : 0;
  int index = 0;
  //  int daysInMonth = m_MthYr.daysInMonth();

  showDailyRecurrences = FALSE;

  for(i = 0; i < 42; i++) {
    index = i + (weekStartsMonday ? 1 : 0) - m_fstDayOfWk - nextLine;
    buttons[i] = new KDateButton(dayone.addDays(index), i, 
				 calendar, viewFrame);
    connect(buttons[i], SIGNAL(selected(QDate, int, bool)),
            SLOT(addSelection(QDate, int, bool)));
    connect(buttons[i], SIGNAL(updateMe(int)),
	    SLOT(updateButton(int)));
    /*    if (index >= 0 && index <= daysInMonth - 1)
      buttons[i]->setItalics(FALSE);
    else
    buttons[i]->setItalics(TRUE);*/
  }

  // read settings from configuration file.
  updateConfig();
}


KDateNavigator::~KDateNavigator()
{
  // clean up widgets
  delete prevMonth;
  delete prevYear;
  delete nextYear;
  delete nextMonth;
  delete dateLabel;
  selectedDates.clear();
}

void KDateNavigator::resizeEvent(QResizeEvent *e) 
{
  int column_width, row_height, i, start_x, wnos_width;
  
  setUpdatesEnabled(FALSE);
  // these shouldn't be hardcoded, and they don't look "quite right..."
  ctrlFrame->setFixedSize(e->size().width(), 18);
  viewFrame->setFixedSize(e->size().width()+3, e->size().height() - 
			  ctrlFrame->size().height() + 4);

  ctrlFrame->move(0,0);

  prevYear->move(5,1);
  prevMonth->move(prevYear->geometry().topRight().x()+2, 1);
  nextYear->move(ctrlFrame->width()-nextYear->width()-5, 1);
  nextMonth->move(nextYear->geometry().topLeft().x()-nextMonth->width()-2, 1);
  dateLabel->setFixedSize(ctrlFrame->width()-prevYear->width()*4-14,
			  dateLabel->height());
  dateLabel->move(ctrlFrame->width()/2-dateLabel->width()/2, 1);

  viewFrame->move(0, ctrlFrame->size().height());

  // 1 for the upper separator line and 2 for the lower margin
  row_height = (viewFrame->height() - 4) / 7;
  if(m_bShowWeekNums) {
    weeknos[0]->setText("99");
    wnos_width = weeknos[0]->sizeHint().width() + 1;
    column_width = (viewFrame->width() - wnos_width) / 7;
    start_x = wnos_width + 1;
    // plus 1 for the line
    weeknos[0]->setText(""); // clear it out;
    weeknos[0]->setFixedSize(wnos_width + 1, row_height + 1);
    weeknos[0]->move(0,0);
    for(i = 1; i < 7; i++) {
      weeknos[i]->setFixedSize(wnos_width, row_height);
      weeknos[i]->move(0, i * row_height + 1);
    }
    weeknumSep->move(start_x-1, row_height+2);
  } else {
    column_width = (viewFrame->width())/7;
    start_x = 0;
  }
  for(i=0; i<7; i++) {
    headings[i]->setFixedSize(column_width, row_height);
    headings[i]->move(start_x+i*column_width, 0);
  }
  for(i=0; i<42; i++) {
    buttons[i]->setFixedSize(column_width, row_height);
    buttons[i]->move(start_x+(i%7)*column_width,
		     (i/7+1)*row_height+1);
  }
  headingSep->setFixedWidth(7*column_width);
  headingSep->move(start_x, row_height);
  weeknumSep->setFixedHeight(6*row_height);

  setUpdatesEnabled(TRUE);
  repaint();
}

void KDateNavigator::updateDates()
{
  int index;

  // If month begins on Monday and Monday is first day of week,
  // month should begin on second line. Sunday doesn't have this problem.
  int nextLine = ((m_fstDayOfWk == 1) && (weekStartsMonday == 1)) ? 7 : 0;

  //find the first day of the week of the current month.
  QDate dayone(m_MthYr.year(), m_MthYr.month(), 1);

  // set the date of each of the day buttons which make up the part of
  // the calendar prior to the first day of this month (they will be
  // grayed out).
  for(int i = 0; i < 42; i++) {
    index = i + (weekStartsMonday ? 1 : 0) - m_fstDayOfWk - nextLine;
    buttons[i]->setDate(dayone.addDays(index));
  }
}

void KDateNavigator::updateView()
{
  register int i;
  const QString months[] = { i18n("January"), i18n("February"), i18n("March"),
                             i18n("April"),   i18n("May"),      i18n("June"),
                             i18n("July"),    i18n("August"),   i18n("September"),
                             i18n("October"), i18n("November"), i18n("December")
                           };

  setUpdatesEnabled(FALSE);

  // compute the label at the top of the navigator
  QString dtstr = months[m_MthYr.month()-1];
  QString yrstr;
  yrstr.setNum(m_MthYr.year());
  dtstr += " " + yrstr;
  dateLabel->setText(dtstr);
  
  for(i = 0; i < 42; i++) {
    updateButton(i);
  }

  // set the week numbers.
  for(i = 0; i < 6; i++) {
    QString weeknum;
    // remember, according to ISO 8601, the first week of the year is the
    // first week that contains a thursday.  Thus we must subtract off 4,
    // not just 1.
    int dayOfYear = buttons[(i + 1) * 7 - 4]->date().dayOfYear();
    if (dayOfYear % 7 != 0)
      weeknum.setNum(dayOfYear / 7 + 1);
    else
      weeknum.setNum(dayOfYear / 7);
    weeknos[i+1]->setText(weeknum);
  }

  setUpdatesEnabled(TRUE);
  repaint();

}

void KDateNavigator::updateConfig()
{
  char sDay[2] = "*";
  const QString monHeaders[] = { i18n("Monday"),    i18n("Tuesday"),
                                 i18n("Wednesday"), i18n("Thursday"),
                                 i18n("Friday"),    i18n("Saturday"),
                                 i18n("Sunday")
                               };
  const QString sunHeaders[] = { i18n("Sunday"),    i18n("Monday"),
                                 i18n("Tuesday"),   i18n("Wednesday"),
                                 i18n("Thursday"),  i18n("Friday"),
                                 i18n("Saturday")
                               };

  KConfig config(locate("config", "korganizerrc")); 

  config.setGroup("Views");
  showDailyRecurrences = config.readBoolEntry("Show Daily Recurrences", FALSE);

  config.setGroup("Colors");
  
  QColor hiliteColor(config.readColorEntry("List Color", &kapp->winStyleHighlightColor()));
  QPalette heading_Palette(palette());

  QColorGroup my_Group = QColorGroup(palette().active().base(),
                  palette().active().base(),
                  palette().active().base(),
                  palette().active().base(),
                  palette().active().base(),
                  hiliteColor,
                  palette().active().base());
  
// Temporarily disabled.
//  heading_Palette.setActive(my_Group);
//  heading_Palette.setInactive(my_Group);
 
  config.setGroup("Time & Date");
  weekStartsMonday = config.readBoolEntry("Week Starts Monday", FALSE);
  curHeaders = (weekStartsMonday ? monHeaders : sunHeaders);
  for(int i=0; i<7; i++) {
    // take the first letter of the day name to be the abbreviation
    sDay[0] = curHeaders[i][0];
    headings[i]->setText(sDay);
    headings[i]->setPalette(heading_Palette);
  }
  updateDates();

  // call static config update function for all the date buttons,
  // and then update them.
  KDateButton::updateConfig();
  for (int i = 0; i < 42; i++)
    updateButton(i);

  updateView();
}

inline void KDateNavigator::updateButton(int i)
{
  int index;
  int extraDays;
  // If month begins on Monday and Monday is first day of week,
  // month should begin on second line. Sunday doesn't have this problem.
  //  int nextLine = ((m_fstDayOfWk == 1) && (weekStartsMonday == 1)) ? 7 : 0;
  //  int daysInMonth = m_MthYr.daysInMonth();

  // check right away that a valid calendar is available.
  ASSERT(calendar != 0);

  //  index = i + (weekStartsMonday ? 1 : 0) - m_fstDayOfWk - nextLine;
  /*if (buttons[i]->date().month() == m_MthYr.month())
    buttons[i]->setItalics(FALSE);
  else
  buttons[i]->setItalics(TRUE);*/

  // check calendar for events on this day, so that we can
  // color it properly. Some care is taken to only change the
  // coloring if necessary, to reduce flicker.
  // must be processed at the beginning cause a day with some events
  // shall render in bold, regardless the other hilite-action
  if(calendar->numEvents(buttons[i]->date())) {
    if (!showDailyRecurrences &&
	(calendar->numEvents(buttons[i]->date()) == 1) && 
	 (calendar->getEventsForDate(buttons[i]->date()).first()->doesRecur() 
	  == KOEvent::rDaily)) {
      if(buttons[i]->hiliteStyle() != KDateButton::NoHilite) {
        buttons[i]->setHiliteStyle(KDateButton::NoHilite);
      }
    } else {
      if(buttons[i]->hiliteStyle() != KDateButton::EventHilite) {
        buttons[i]->setHiliteStyle(KDateButton::EventHilite);
      }
    }
  } else {
    if(buttons[i]->hiliteStyle() != KDateButton::NoHilite) {
      buttons[i]->setHiliteStyle(KDateButton::NoHilite);
    }
  }
  
  // check to see if this button is currently selected, because then
  // it's coloring is simply the selected coloring.
  for (QDate *tmpDate = selectedDates.first(); tmpDate;
       tmpDate = selectedDates.next()) {

    // find out if the selected date is in the previous or the next
    // month, and compute an offset accordingly
    // get the day of the week on the first day
    QDate dayone(m_MthYr.year(), m_MthYr.month(), 1);
    if (*tmpDate > dayone.addDays(dayone.daysInMonth()-1)) {
      extraDays = dayone.daysInMonth();
    } else if (*tmpDate < dayone) {
      extraDays = 0 - tmpDate->daysInMonth();
    } else {
      extraDays = 0;
    }

    // compute the "number" of the date in the table. Check that 
    // it is in the valid range.
    index = dayToIndex((tmpDate->day() + extraDays));
    ASSERT(index >= 0);
    ASSERT(index < 42);

    // if the current button's index is the same as that of one of the
    // selected dates, hilite this button.
    if (index == i)
      buttons[i]->setHiliteStyle(KDateButton::SelectHilite);

  }

  if (!weekStartsMonday && (float(i)/7 == float(i/7)) ||
      weekStartsMonday && (float(i-6)/7 == float((i-6)/7))) {
    if (buttons[i]->hiliteStyle() == KDateButton::SelectHilite) {
      buttons[i]->setHiliteStyle(KDateButton::HolidaySelectHilite);
    } else { // it's a holiday, but not selected
      buttons[i]->setHiliteStyle(KDateButton::HolidayHilite);
    }
  }

  // FIXME: today overwrites holiday for now
  // FIXME: the colorchanges makes it flicker
  // if the current button is a holiday
  if (!calendar->getHolidayForDate(buttons[i]->date()).isEmpty()) {
    if (buttons[i]->hiliteStyle() == KDateButton::SelectHilite) {
      buttons[i]->setHiliteStyle(KDateButton::HolidaySelectHilite);
    } else { // it's a holiday, but not selected
      buttons[i]->setHiliteStyle(KDateButton::HolidayHilite);
    }
  }
  
  // if today is in the currently displayed month, hilight today. 
  if ((buttons[i]->date().year() == QDate::currentDate().year()) &&
      (buttons[i]->date().month() == QDate::currentDate().month()) &&
      (buttons[i]->date().day() == QDate::currentDate().day())) {
    if (buttons[i]->hiliteStyle() == KDateButton::SelectHilite) {
      buttons[i]->setHiliteStyle(KDateButton::TodaySelectHilite);
    } else { // it's Today, but not selected
      buttons[i]->setHiliteStyle(KDateButton::TodayHilite);
    }
    //return;
  }   
}

void KDateNavigator::setShowWeekNums(bool enabled)
{
  m_bShowWeekNums = enabled;
  for(int i=0; i<7; i++) {
    if(enabled)
      weeknos[i]->show();
    else
      weeknos[i]->hide();
  }
  resize(size());
}

const QDateList KDateNavigator::getSelected()
{
  QDate *tmpDate;

  for (tmpDate = selectedDates.first(); tmpDate;
       tmpDate = selectedDates.next()) {
    if (!tmpDate->isValid()) {
      debug("Null or invalid date selected!!!");
      selectedDates.clear();
      selectedDates.append(new QDate(QDate::currentDate()));
      emit datesSelected(selectedDates);
      break;
    }
  }
  return selectedDates;
}


void KDateNavigator::goNextMonth()
{
  int yr, mth, day;

  // calculate yr, mth and day for the next month, wrapping if
  // necessary.
  yr  = m_MthYr.month() < 12 ? m_MthYr.year() : m_MthYr.year()+1;
  mth = m_MthYr.month() < 12 ? m_MthYr.month()+1 : 1;
  day = m_MthYr.day();

  // make sure that we move to a valid day in the next month.
  for(;;) {
    if(QDate::isValid(yr, mth, day))
      break;
    day--;
  }

  // set our record of the month, year and day that this datetbl is
  // displaying.
  m_MthYr.setYMD(yr, mth, day);
  QDate dayone(m_MthYr.year(), m_MthYr.month(), 1);
  m_fstDayOfWk = dayone.dayOfWeek();

  updateDates();
  fixupSelectedDates(yr, mth);
  updateView();
}

void KDateNavigator::goPrevMonth()
{
  int yr, mth, day;

  // calculate yr, mth and day for the next month, wrapping if
  // necessary.
  yr  = m_MthYr.month() > 1 ? m_MthYr.year() : m_MthYr.year()-1;
  mth = m_MthYr.month() > 1 ? m_MthYr.month()-1 : 12;
  day = m_MthYr.day();

  // make sure that we move to a valid day in the next month.
  for(;;) {
    if(QDate::isValid(yr, mth, day))
      break;
    day--;
  }

  // set our record of the month and year that this datetbl is
  // displaying.
  m_MthYr.setYMD(yr, mth, day);
  QDate dayone(m_MthYr.year(), m_MthYr.month(), 1);
  m_fstDayOfWk = dayone.dayOfWeek();

  updateDates();
  fixupSelectedDates(yr, mth);
  updateView();
}

void KDateNavigator::goNextYear()
{
  int yr, mth, day;

  // calculate yr, mth and day for the next month, wrapping if
  // necessary.
  yr  = m_MthYr.year()+1;
  mth = m_MthYr.month();
  day = m_MthYr.day();

  // make sure that we move to a valid day in the next month.
  for(;;) {
    if(QDate::isValid(yr, mth, day))
      break;
    day--;
  }

  // set our record of the month, year and day that this datetbl is
  // displaying.
  m_MthYr.setYMD(yr, mth, day);
  QDate dayone(m_MthYr.year(), m_MthYr.month(), 1);
  m_fstDayOfWk = dayone.dayOfWeek();

  updateDates();
  fixupSelectedDates(yr, mth);
  updateView();
}

void KDateNavigator::goPrevYear()
{
  int yr, mth, day;

  // calculate yr, mth and day for the next month, wrapping if
  // necessary.
  yr  = m_MthYr.year()-1;
  mth = m_MthYr.month();
  day = m_MthYr.day();

  // make sure that we move to a valid day in the next month.
  for(;;) {
    if(QDate::isValid(yr, mth, day))
      break;
    day--;
  }

  // set our record of the month, year and day that this datetbl is
  // displaying.
  m_MthYr.setYMD(yr, mth, day);
  QDate dayone(m_MthYr.year(), m_MthYr.month(), 1);
  m_fstDayOfWk = dayone.dayOfWeek();

  updateDates();
  fixupSelectedDates(yr, mth);
  updateView();
}


// when prev/next year/month are pressed, this fixes the selected
// date list and emits datesSelected signal.
//void KDateNavigator::fixupSelectedDates(int yr, int mth)
void KDateNavigator::fixupSelectedDates(int, int)
{
  selectedDates.clear();
  for (int i = 0; i < 42; i++) {
    if (buttons[i]->isSelected()) {
      if (! buttons[i]->date().isValid())
        debug("KDateNavigator::fixupSelectedDates(): invalid date stored on buttons[%i]; ignoring", i);
      else
        selectedDates.inSort(new QDate(buttons[i]->date()));
    }
  }

  emit datesSelected(selectedDates);
}

void KDateNavigator::selectDates(const QDateList dateList)
{
  if (dateList.count() > 0) {
    selectedDates.clear();
    selectedDates = dateList;
  
    // check to see if these dates are valid.
    QListIterator<QDate> it(dateList);
    for (; it.current(); ++it) {
      if (!it.current()->isValid()) {
        selectedDates.clear();
        selectedDates.append(new QDate(QDate::currentDate()));
        debug("KDateNavigator::selectDates(const QDateList): an invalid date was passed as a parameter!");
        emit datesSelected(selectedDates);
      }
    }
  
    // set our record of the month and year that this datetbl is
    // displaying.
    m_MthYr = *selectedDates.first();

    // set our record of the first day of the week of the current
    // month. This needs to be done before calling dayToIndex, since it
    // relies on this information being up to date.
    QDate dayone(m_MthYr.year(), m_MthYr.month(), 1);
    m_fstDayOfWk = dayone.dayOfWeek();

    updateDates();
    updateView();
  }
}

void KDateNavigator::selectDates(QDate qd)
{
  if (! qd.isValid()) {
    debug("KDateNavigator::selectDates(QDate): an invalid date was passed as a parameter!");
    qd = QDate::currentDate();
  }

  selectedDates.clear();
  selectedDates.append(new QDate(qd));
  m_MthYr = qd;
  
  QDate dayone(m_MthYr.year(), m_MthYr.month(), 1);
  m_fstDayOfWk = dayone.dayOfWeek();

  updateDates();
  updateView();
}

void KDateNavigator::addSelection(QDate selDate, int index, bool ctrlPressed)
{
  QDate *tmpDate;
  bool found = FALSE;
  int extraDays;

  if (! selDate.isValid()) {
    debug("KDateNavigator::addSelection(): invalid date passed as a parameter!");
    return;
  }

  // if control wasn't held down, remove all selected dates.  Update those
  // dates appearances.
  if (!ctrlPressed) {
    // make a deep copy.
    QDateList tmpDates;
    tmpDates.setAutoDelete(TRUE);
    tmpDates = selectedDates;

    selectedDates.clear();
    for (tmpDate = tmpDates.first(); tmpDate;
          tmpDate = tmpDates.next()) {
      
      // find out if the selected date is in the previous or the next
      // month, and compute an offset accordingly
      // get the day of the week on the first day
      QDate dayone(m_MthYr.year(), m_MthYr.month(), 1);
      if (*tmpDate > dayone.addDays(dayone.daysInMonth()-1)) {
        extraDays = dayone.daysInMonth();
      } else if (*tmpDate < dayone) {
        extraDays = 0 - tmpDate->daysInMonth();
      } else {
        extraDays = 0;
      }
      updateButton(dayToIndex(tmpDate->day() + extraDays));
    }
    // clear it out, we're done.
    tmpDates.clear();
  }

  // now, if there are any dates left (i.e. ctrl was held down), check
  // to see if they clicked an already selected date. 
  // If so, we need to unselect it.
  // if not, we need to add it to the list of selected dates.
  for (tmpDate = selectedDates.first(); tmpDate;
       tmpDate = selectedDates.next()) {
    if (*tmpDate == selDate) {
      found = TRUE;
      break;
    }
  }
  if (found && selectedDates.count() > 1)
    selectedDates.remove();
  else
    selectedDates.inSort(new QDate(selDate));

  // update the appearance of the clicked date
  updateButton(index);

  emit datesSelected(selectedDates);
}

inline int KDateNavigator::dayNum(int row, int col)
{
  return 7 * (row - 1) + (col + 1) - m_fstDayOfWk;
}

inline int KDateNavigator::dayToIndex(int dayNum)
{
  int row, col;

  row = (dayNum+m_fstDayOfWk-1-(weekStartsMonday ? 1 : 0)) / 7;
  if (weekStartsMonday && (m_fstDayOfWk == 1))
    row++;
  col = (dayNum+m_fstDayOfWk-1-(weekStartsMonday ? 1 : 0)) % 7;
  return row * 7 + col;
}
