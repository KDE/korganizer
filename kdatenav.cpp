// 	$Id$	

#include <qstring.h>
#include <qtooltip.h>
#include <qkeycode.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kstddirs.h>

#include "koprefs.h"

#include "kdatenav.h"
#include "kdatenav.moc"

KDateNavigator::KDateNavigator(QWidget *parent, 
			       CalObject *calendar, 
			       bool show_week_nums,
			       const char *name,
			       QDate startDate)
  : QFrame(parent, name)
{
  mCalendar = calendar;
  
  setFrameStyle(QFrame::NoFrame);

  QGridLayout *topLayout = new QGridLayout(this,8,8);

  if (! startDate.isValid()) {
    qDebug("KDateNavigator::KDateNavigator(): an invalid date was passed as a parameter!");
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

  topLayout->addMultiCellWidget(ctrlFrame,0,0,0,7);
  
  QFont tfont = font();
  tfont.setPointSize(10);
  tfont.setBold(FALSE);

  // Create backward navigation buttons
  prevYear = new QPushButton(ctrlFrame);
  prevYear->setPixmap(SmallIcon("2leftarrow"));
  QToolTip::add(prevYear, i18n("Previous Year"));

  prevMonth = new QPushButton(ctrlFrame);
  prevMonth->setPixmap(SmallIcon("1leftarrow"));
  QToolTip::add(prevMonth, i18n("Previous Month"));

  // Create forward navigation buttons
  nextMonth = new QPushButton(ctrlFrame);
  nextMonth->setPixmap(SmallIcon("1rightarrow"));
  QToolTip::add(nextMonth, i18n("Next Month"));

  nextYear = new QPushButton(ctrlFrame);
  nextYear->setPixmap(SmallIcon("2rightarrow"));
  QToolTip::add(nextYear, i18n("Next Year"));

  // Create month name label
  dateLabel = new QLabel(ctrlFrame);
  dateLabel->setFont(tfont);
  dateLabel->setAlignment(AlignCenter);

  // Set minimum width to width of widest month name label
  int i;
  int maxwidth = 0;
  QFontMetrics fm = dateLabel->fontMetrics();
  for(i=1;i<=12;++i) {
    int width = fm.width(KGlobal::locale()->monthName(i) + " 2000");
    if (width > maxwidth) maxwidth = width;
  }
  dateLabel->setMinimumWidth(maxwidth);

  // set up control frame layout
  QBoxLayout *ctrlLayout = new QHBoxLayout(ctrlFrame,1);
  ctrlLayout->addWidget(prevYear,3);
  ctrlLayout->addWidget(prevMonth,3);
  ctrlLayout->addStretch(1);
  ctrlLayout->addSpacing(4);
  ctrlLayout->addWidget(dateLabel);
  ctrlLayout->addSpacing(4);
  ctrlLayout->addStretch(1);
  ctrlLayout->addWidget(nextMonth,3);
  ctrlLayout->addWidget(nextYear,3);

  connect(prevYear, SIGNAL(clicked()), this, SLOT(goPrevYear()));
  connect(prevMonth, SIGNAL(clicked()), this, SLOT(goPrevMonth()));
  connect(nextMonth, SIGNAL(clicked()), this, SLOT(goNextMonth()));
  connect(nextYear, SIGNAL(clicked()), this, SLOT(goNextYear()));

  viewFrame = new QFrame(this, "KDateNavigator::ViewFrame");
  viewFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  viewFrame->setLineWidth(1);
 
  topLayout->addMultiCellWidget(viewFrame,2,7,1,7);

  // get the day of the week on the first day
  QDate dayone(m_MthYr.year(), 
	       m_MthYr.month(), 1);
  m_fstDayOfWk = dayone.dayOfWeek();

  // Set up the heading fields.
  for(i=0; i<7; i++) {
    headings[i] = new QLabel("",this);
    headings[i]->setFont(QFont("Arial", 10, QFont::Bold));
    headings[i]->setAlignment(AlignCenter);
    
    topLayout->addWidget(headings[i],1,i+1);
  }

  // Create the weeknumber labels
  for(i=0; i<6; i++) {
    weeknos[i] = new QLabel(this);
    weeknos[i]->setAlignment(AlignCenter);
    weeknos[i]->setFont(QFont("Arial", 10));
    if(!show_week_nums) {
      weeknos[i]->hide();
    }
    weeknos[i]->installEventFilter(this);
    
    topLayout->addWidget(weeknos[i],i+2,0);
  }

  // If month begins on Monday and Monday is first day of week,
  // month should begin on second line. Sunday doesn't have this problem.
  int nextLine = ((m_fstDayOfWk == 1) && (KGlobal::locale()->weekStartsMonday() == 1)) ? 7 : 0;
  int index = 0;
  //  int daysInMonth = m_MthYr.daysInMonth();

  QGridLayout *dateLayout = new QGridLayout(viewFrame,6,7,1,0);

  int row,col;
  for(i = 0; i < 42; i++) {
    index = i + (KGlobal::locale()->weekStartsMonday() ? 1 : 0) - m_fstDayOfWk - nextLine;
    buttons[i] = new KDateButton(dayone.addDays(index),i,mCalendar,viewFrame);
    row = i/7;
    col = i-row*7;
    dateLayout->addWidget(buttons[i],row,col);
    connect(buttons[i], SIGNAL(selected(QDate, int, bool)),
            SLOT(addSelection(QDate, int, bool)));
    connect(buttons[i], SIGNAL(updateMe(int)),
	    SLOT(updateButton(int)));
    connect(buttons[i],SIGNAL(eventDropped(KOEvent *)),
            SIGNAL(eventDropped(KOEvent *)));
  }

  // read settings from configuration file.
  updateConfig();
}


KDateNavigator::~KDateNavigator()
{
}


void KDateNavigator::updateDates()
{
  int index;

  // If month begins on Monday and Monday is first day of week,
  // month should begin on second line. Sunday doesn't have this problem.
  int nextLine = ((m_fstDayOfWk == 1) && (KGlobal::locale()->weekStartsMonday() == 1)) ? 7 : 0;

  //find the first day of the week of the current month.
  QDate dayone(m_MthYr.year(), m_MthYr.month(), 1);

  // set the date of each of the day buttons. Buttons not belonging to the month
  // currently displayed are shown in italics.
  for(int i = 0; i < 42; i++) {
    index = i + (KGlobal::locale()->weekStartsMonday() ? 1 : 0) - m_fstDayOfWk - nextLine;
    QDate buttonDate = dayone.addDays(index);
    buttons[i]->setDate(buttonDate);
    if (buttonDate.month() != dayone.month()) buttons[i]->setItalic(true);
    else buttons[i]->setItalic(false);
  }
}

void KDateNavigator::updateView()
{
  setUpdatesEnabled(FALSE);

  // compute the label at the top of the navigator
  QString dtstr = KGlobal::locale()->monthName(m_MthYr.month()) + " " +
                  QString::number(m_MthYr.year());
  dateLabel->setText(dtstr);
  
  int i;
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
    weeknos[i]->setText(weeknum);
  }

  setUpdatesEnabled(TRUE);
  repaint();
}

void KDateNavigator::updateConfig()
{
  int day;
  for(int i=0; i<7; i++) {
    // take the first letter of the day name to be the abbreviation
    if (KGlobal::locale()->weekStartsMonday()) {
      day = i+1;
    } else {
      if (i==8) day = 1;
      else day = i;
    }
    headings[i]->setText(KGlobal::locale()->weekDayName(day).left(1));
  }
  updateDates();

  for (int i = 0; i < 42; i++) {
    updateButton(i);
    buttons[i]->updateConfig();
  }

  updateView();
}

void KDateNavigator::updateButton(int i)
{
  int index;
  int extraDays;
  // If month begins on Monday and Monday is first day of week,
  // month should begin on second line. Sunday doesn't have this problem.
  //  int nextLine = ((m_fstDayOfWk == 1) && (KGlobal::locale()->weekStartsMonday() == 1)) ? 7 : 0;
  //  int daysInMonth = m_MthYr.daysInMonth();

  // check right away that a valid calendar is available.
  ASSERT(mCalendar != 0);

  //  index = i + (KGlobal::locale()->weekStartsMonday() ? 1 : 0) - m_fstDayOfWk - nextLine;
  /*if (buttons[i]->date().month() == m_MthYr.month())
    buttons[i]->setItalic(FALSE);
  else
  buttons[i]->setItalic(TRUE);*/

  // check calendar for events on this day
  bool hasEvents = false;
  QList<KOEvent> events = mCalendar->getEventsForDate(buttons[i]->date());
  KOEvent *event;
  for(event=events.first();event;event=events.next()) {
    ushort recurType = event->doesRecur();
    if ((recurType == KOEvent::rNone) ||
        (recurType == KOEvent::rDaily && KOPrefs::instance()->mDailyRecur) ||
        (recurType == KOEvent::rWeekly && KOPrefs::instance()->mWeeklyRecur)) {
      hasEvents = true;
      break;
    }
  }
  buttons[i]->setEvent(hasEvents);


  // check to see if this button is currently selected
  bool selected = false;
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
    if (index == i) {
      selected = true;
    }
  }
  buttons[i]->setSelected(selected);

  // Calculate holidays. Sunday is also treated as holiday.
  if (!KGlobal::locale()->weekStartsMonday() && (float(i)/7 == float(i/7)) ||
      KGlobal::locale()->weekStartsMonday() && (float(i-6)/7 == float((i-6)/7)) ||
      !mCalendar->getHolidayForDate(buttons[i]->date()).isEmpty()) {
    buttons[i]->setHoliday();
  } else {
    buttons[i]->setHoliday(false);
  }
  
  // if today is in the currently displayed month, hilight today. 
  if ((buttons[i]->date().year() == QDate::currentDate().year()) &&
      (buttons[i]->date().month() == QDate::currentDate().month()) &&
      (buttons[i]->date().day() == QDate::currentDate().day())) {
    buttons[i]->setToday();
  } else {
    buttons[i]->setToday(false);
  }
}

void KDateNavigator::setShowWeekNums(bool enabled)
{
  m_bShowWeekNums = enabled;
  for(int i=0; i<6; i++) {
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
      qDebug("Null or invalid date selected!!!");
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
        qDebug("KDateNavigator::fixupSelectedDates(): invalid date stored on buttons[%i]; ignoring", i);
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
        qDebug("KDateNavigator::selectDates(const QDateList): an invalid date was passed as a parameter!");
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
    qDebug("KDateNavigator::selectDates(QDate): an invalid date was passed as a parameter!");
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
    qDebug("KDateNavigator::addSelection(): invalid date passed as a parameter!");
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

int KDateNavigator::dayNum(int row, int col)
{
  return 7 * (row - 1) + (col + 1) - m_fstDayOfWk;
}

int KDateNavigator::dayToIndex(int dayNum)
{
  int row, col;

  row = (dayNum+m_fstDayOfWk-1-(KGlobal::locale()->weekStartsMonday() ? 1 : 0)) / 7;
  if (KGlobal::locale()->weekStartsMonday() && (m_fstDayOfWk == 1))
    row++;
  col = (dayNum+m_fstDayOfWk-1-(KGlobal::locale()->weekStartsMonday() ? 1 : 0)) % 7;
  return row * 7 + col;
}

bool KDateNavigator::eventFilter (QObject *o,QEvent *e)
{
  if (e->type() == QEvent::MouseButtonPress) {
    int i;
    for(i=0;i<6;++i) {
      if (o == weeknos[i]) {
        QDate weekstart = buttons[i*7]->date();
        emit weekClicked(weekstart);
        break;
      }
    }
    return true;
  } else {
    return false;
  }
}
