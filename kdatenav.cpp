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

#include <qstring.h>
#include <qtooltip.h>
#include <qkeycode.h>
#include <qpushbutton.h>
#include <qlayout.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "koprefs.h"
#ifndef KORG_NOPLUGINS
#include "kocore.h"
#endif

#include "kdatenav.h"
#include "kdatenav.moc"

KDateNavigator::KDateNavigator(QWidget *parent,Calendar *calendar,
                               bool show_week_nums,const char *name,
                               QDate startDate)
  : QFrame(parent, name)
{
  mCalendar = calendar;

  setFrameStyle(QFrame::NoFrame);

  QGridLayout *topLayout = new QGridLayout(this,8,8);

  if (! startDate.isValid()) {
    kdDebug() << "KDateNavigator::KDateNavigator(): an invalid date was passed as a parameter!" << endl;
    startDate = QDate::currentDate();
  }

  mSelectedDates.append(startDate);
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
  ctrlLayout->addSpacing(2);
  ctrlLayout->addWidget(dateLabel);
  ctrlLayout->addSpacing(2);
  ctrlLayout->addStretch(1);
  ctrlLayout->addWidget(nextMonth,3);
  ctrlLayout->addWidget(nextYear,3);

  connect(prevYear, SIGNAL(clicked()), this, SLOT(goPrevYear()));
  connect(prevMonth, SIGNAL(clicked()), this, SLOT(goPrevMonth()));
  connect(nextMonth, SIGNAL(clicked()), this, SLOT(goNextMonth()));
  connect(nextYear, SIGNAL(clicked()), this, SLOT(goNextYear()));

  // get the day of the week on the first day
  QDate dayone(m_MthYr.year(), m_MthYr.month(), 1);
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

  //QDate startday = dayone.addDays((KGlobal::locale()->weekStartsMonday() ? 1 : 0) - m_fstDayOfWk - nextLine);
  daymatrix = new KODayMatrix(this, mCalendar, dayone, "KDateNavigator::DayMatrix");
  daymatrix->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  daymatrix->setLineWidth(1);

  //handle dragged selections
  connect(daymatrix, SIGNAL(selected(const DateList)),
          this, SLOT(addSelection(const DateList)));

  connect(daymatrix, SIGNAL(eventDropped(Event *)),
          this, SIGNAL(eventDropped(Event *)));

  topLayout->addMultiCellWidget(daymatrix,2,7,1,7);

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

  // update the matrix dates
  index = (KGlobal::locale()->weekStartsMonday() ? 1 : 0) - m_fstDayOfWk - nextLine;
  daymatrix->updateView(dayone.addDays(index));
//each updateDates is followed by an updateView -> repaint is issued there !
//  daymatrix->repaint();
}

void KDateNavigator::shiftEvent(const QDate& olddate, const QDate& newdate)
{
  //TODO ET forward to specific method of daymatrix the in/decreases events[]
  //and repaints itself
  kdDebug() << "KDateNavigator::shiftEvent" << endl;
  daymatrix->updateView();
  daymatrix->repaint();
}


void KDateNavigator::updateView()
{
  setUpdatesEnabled(FALSE);

  // compute the label at the top of the navigator
  QString dtstr = KGlobal::locale()->monthName(m_MthYr.month()) + " " +
                  QString::number(m_MthYr.year());
  dateLabel->setText(dtstr);

  int i;

//  kdDebug() << "updateView() -> daymatrix->updateView()" << endl;
  daymatrix->updateView();

  // set the week numbers.
  for(i = 0; i < 6; i++) {
    QString weeknum;
    // remember, according to ISO 8601, the first week of the year is the
    // first week that contains a thursday.  Thus we must subtract off 4,
    // not just 1.

    //ET int dayOfYear = buttons[(i + 1) * 7 - 4]->date().dayOfYear();
    int dayOfYear = daymatrix->getDate((i+1)*7-4).dayOfYear();
    if (dayOfYear % 7 != 0)
      weeknum.setNum(dayOfYear / 7 + 1);
    else
      weeknum.setNum(dayOfYear / 7);
    weeknos[i]->setText(weeknum);
  }

  setUpdatesEnabled(TRUE);
//  kdDebug() << "updateView() -> repaint()" << endl;
  repaint();
  daymatrix->repaint();
}

void KDateNavigator::updateConfig()
{
  int day;
  for(int i=0; i<7; i++) {
    // take the first letter of the day name to be the abbreviation
    if (KGlobal::locale()->weekStartsMonday()) {
      day = i+1;
    } else {
      if (i==0) day = 7;
      else day = i;
    }
    headings[i]->setText(KGlobal::locale()->weekDayName(day).left(1));
  }
  kdDebug() << "updateConfig() -> updateDates()" << endl;
  updateDates();

  kdDebug() << "updateConfig() -> updateView()" << endl;
  updateView();
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

DateList KDateNavigator::selectedDates()
{
  return mSelectedDates;
}

void KDateNavigator::gotoYMD(int yr, int mth, int day)
{
  // make sure that we move to a valid day and month.
  if( day <= 0 ) day = 1;
  if( mth <= 0 ) mth = 1;

  for(;;) {
  	if(QDate::isValid(yr, mth, day))
  		break;
  	if( day > 1 ) day--;
	else if( mth > 1 ) mth--;
	else yr=1900;
  }

  // set our record of the month, year and day that this datetbl is
  // displaying.
  m_MthYr.setYMD(yr, mth, day);
  QDate dayone(m_MthYr.year(), m_MthYr.month(), 1);
  m_fstDayOfWk = dayone.dayOfWeek();

  kdDebug() << "KDateNavigator::gotoYMD -> updateDates()" << endl;
  updateDates();

  mSelectedDates.clear();
  daymatrix->addSelectedDaysTo(mSelectedDates);
  emit datesSelected(mSelectedDates);

  kdDebug() << "KDateNavigator::gotoYMD -> updateView()" << endl;
  updateView();
  kdDebug() << "KDateNavigator::gotoYMD -> end" << endl;
}


void KDateNavigator::goNextMonth()
{
  int yr, mth, day;

  // calculate yr, mth and day for the next month, wrapping if
  // necessary.
  yr  = m_MthYr.month() < 12 ? m_MthYr.year() : m_MthYr.year()+1;
  mth = m_MthYr.month() < 12 ? m_MthYr.month()+1 : 1;
  day = m_MthYr.day();

  gotoYMD(yr,mth,day);
}

void KDateNavigator::goPrevMonth()
{
  int yr, mth, day;

  // calculate yr, mth and day for the next month, wrapping if
  // necessary.
  yr  = m_MthYr.month() > 1 ? m_MthYr.year() : m_MthYr.year()-1;
  mth = m_MthYr.month() > 1 ? m_MthYr.month()-1 : 12;
  day = m_MthYr.day();

  gotoYMD(yr,mth,day);
}

void KDateNavigator::goNextYear()
{
  gotoYMD(m_MthYr.year()+1, m_MthYr.month(), m_MthYr.day());
}

void KDateNavigator::goPrevYear()
{
  gotoYMD(m_MthYr.year()-1, m_MthYr.month(), m_MthYr.day());
}


void KDateNavigator::selectDates(const DateList& dateList)
{
  if (dateList.count() > 0) {
    mSelectedDates = dateList;

    // set our record of the month and year that this datetbl is
    // displaying.
    m_MthYr = mSelectedDates.first();

    // set our record of the first day of the week of the current
    // month. This needs to be done before calling dayToIndex, since it
    // relies on this information being up to date.
    QDate dayone(m_MthYr.year(), m_MthYr.month(), 1);
    m_fstDayOfWk = dayone.dayOfWeek();

    daymatrix->setSelectedDaysFrom(*(dateList.begin()), *(--dateList.end()));
    updateDates();
    updateView();
  }
}

void KDateNavigator::selectDates(QDate qd)
{
  if (! qd.isValid()) {
    kdDebug() << "KDateNavigator::selectDates(QDate): an invalid date was passed as a parameter!" << endl;
    qd = QDate::currentDate();
  }

  mSelectedDates.clear();
  mSelectedDates.append(qd);
  m_MthYr = qd;

  QDate dayone(m_MthYr.year(), m_MthYr.month(), 1);
  m_fstDayOfWk = dayone.dayOfWeek();

  kdDebug() << "qd:" << qd.day() << endl;
  daymatrix->setSelectedDaysFrom(qd, qd);
  updateDates();
  updateView();
}

void KDateNavigator::addSelection(const DateList newsel)
{
  mSelectedDates = newsel;
  emit datesSelected(newsel);
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
        QDate weekstart = daymatrix->getDate(i*7);
        emit weekClicked(weekstart);
        break;
      }
    }
    return true;
  } else {
    return false;
  }
}
