/*
    This file is part of KOrganizer.
    Copyright (c) 2001,2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <qstring.h>
#include <qtooltip.h>
#include <qkeycode.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "koglobals.h"
#include "koprefs.h"
#ifndef KORG_NOPLUGINS
#include "kocore.h"
#endif

#include <kcalendarsystem.h>

#include "kdatenavigator.h"
#include "kdatenavigator.moc"

KDateNavigator::KDateNavigator( QWidget *parent, Calendar *calendar,
                                bool show_week_nums, const char *name,
                                QDate startDate )
  : QFrame(parent, name),
    updateTimer(0L)
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

  bool isRTL = KOGlobals::self()->reverseLayout();

  // Create backward navigation buttons
  prevYear = new QPushButton(ctrlFrame);
  prevYear->setPixmap(SmallIcon(isRTL ? "2rightarrow" : "2leftarrow"));
  QToolTip::add(prevYear, i18n("Previous Year"));

  prevMonth = new QPushButton(ctrlFrame);
  prevMonth->setPixmap(SmallIcon(isRTL ? "1rightarrow" : "1leftarrow"));
  QToolTip::add(prevMonth, i18n("Previous Month"));

  // Create forward navigation buttons
  nextMonth = new QPushButton(ctrlFrame);
  nextMonth->setPixmap(SmallIcon(isRTL ? "1leftarrow" : "1rightarrow"));
  QToolTip::add(nextMonth, i18n("Next Month"));

  nextYear = new QPushButton(ctrlFrame);
  nextYear->setPixmap(SmallIcon(isRTL ? "2leftarrow" : "2rightarrow"));
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
    int width = fm.width( KOGlobals::self()->calendarSystem()->monthName(i) + " 2000" );
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

  connect( prevYear, SIGNAL( clicked() ), SIGNAL( goPrevYear() ) );
  connect( prevMonth, SIGNAL( clicked() ), SIGNAL( goPrevMonth() ) );
  connect( nextMonth, SIGNAL( clicked() ), SIGNAL( goNextMonth() ) );
  connect( nextYear, SIGNAL( clicked() ), SIGNAL( goNextYear() ) );

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

  daymatrix = new KODayMatrix( this, mCalendar, dayone,
                               "KDateNavigator::DayMatrix");
  daymatrix->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  daymatrix->setLineWidth(1);

  connect( daymatrix, SIGNAL( selected( const KCal::DateList & ) ),
           SIGNAL( datesSelected( const KCal::DateList & ) ) );

  connect( daymatrix, SIGNAL( eventDropped( Event * ) ),
           SIGNAL( eventDropped( Event * ) ) );

  topLayout->addMultiCellWidget(daymatrix,2,7,1,7);

  // read settings from configuration file.
  updateConfig();
  enableRollover(FollowMonth);
}

void KDateNavigator::enableRollover(RolloverType r)
{
  switch(r)
  {
  case None :
    if (updateTimer)
    {
      updateTimer->stop();
      delete updateTimer;
      updateTimer=0L;
    }
    break;
  case FollowDay :
  case FollowMonth :
    if (!updateTimer)
    {
      updateTimer = new QTimer(this);
      QObject::connect(updateTimer,SIGNAL(timeout()),
        this,SLOT(possiblyPastMidnight()));
    }
    updateTimer->start(0,true);
    lastDayChecked = QDate::currentDate();
  }
  updateRollover=r;
}


KDateNavigator::~KDateNavigator()
{
}


void KDateNavigator::passedMidnight()
{
    QDate today = QDate::currentDate();
    bool emitMonth = false;

    if (today.month() != lastDayChecked.month())
    {
       if (updateRollover==FollowMonth &&
           daymatrix->isEndOfMonth()) {
         goNextMonth();
	 emitMonth=true;
       }
    }
    daymatrix->recalculateToday();
    daymatrix->repaint();
    emit dayPassed(today);
    if (emitMonth) { emit monthPassed(today); }
}

/* slot */ void KDateNavigator::possiblyPastMidnight()
{
  if (lastDayChecked!=QDate::currentDate())
  {
    passedMidnight();
    lastDayChecked=QDate::currentDate();
  }
  // Set the timer to go off 1 second after midnight
  // or after 8 minutes, whichever comes first.
  if (updateTimer)
  {
    QTime now = QTime::currentTime();
    QTime midnight = QTime(23,59,59);
    int msecsWait = QMIN(480000,now.msecsTo(midnight)+2000);

    // qDebug(QString("Waiting %1 msec from %2 to %3.").arg(msecsWait)
    //	.arg(now.toString()).arg(midnight.toString()));

    updateTimer->stop();
    updateTimer->start(msecsWait,true);
  }
}

void KDateNavigator::updateDates()
{
  // Find the first day of the week of the current month.
  //int d1 = KOGlobals::self()->calendarSystem()->day( m_MthYr );
  QDate dayone( m_MthYr.year(), m_MthYr.month(), m_MthYr.day() );
  int d2 = KOGlobals::self()->calendarSystem()->day( dayone );
  //int di = d1 - d2 + 1;
  dayone = dayone.addDays( -d2 + 1 );

  int m_fstDayOfWkCalsys = KOGlobals::self()->calendarSystem()->dayOfWeek( dayone );

  // If month begins on Monday and Monday is first day of week,
  // month should begin on second line. Sunday doesn't have this problem.
  int nextLine = ( ( m_fstDayOfWkCalsys == 1) &&
                   ( KGlobal::locale()->weekStartsMonday() == 1 ) ) ? 7 : 0;

  // update the matrix dates
  int index = (KGlobal::locale()->weekStartsMonday() ? 1 : 0) - m_fstDayOfWkCalsys - nextLine;


  daymatrix->updateView(dayone.addDays(index));
//each updateDates is followed by an updateView -> repaint is issued there !
//  daymatrix->repaint();
}

void KDateNavigator::updateDayMatrix()
{
  daymatrix->updateView();
  daymatrix->repaint();
}


void KDateNavigator::updateView()
{
  setUpdatesEnabled( false );

  // compute the label at the top of the navigator
  QDate cT( m_MthYr.year(), m_MthYr.month(), m_MthYr.day() );
  QString dtstr = KOGlobals::self()->calendarSystem()->monthName( cT ) + " " +
                  QString::number(KOGlobals::self()->calendarSystem()->year( cT ) );
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
    int dayOfYear = KOGlobals::self()->calendarSystem()->dayOfYear((daymatrix->getDate((i+1)*7-4)));

    if (dayOfYear % 7 != 0)
      weeknum.setNum(dayOfYear / 7 + 1);
    else
      weeknum.setNum(dayOfYear / 7);
    weeknos[i]->setText(weeknum);
  }

  setUpdatesEnabled( true );
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
    headings[i]->setText( KOGlobals::self()->calendarSystem()->weekDayName(day, true) );
  }
  updateDates();
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

    updateDates();

    daymatrix->setSelectedDaysFrom(*(dateList.begin()), *(--dateList.end()));

    updateView();
  }
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

void KDateNavigator::wheelEvent (QWheelEvent *e)
{
  if(e->delta()>0) emit goPrevious();
  else emit goNext();

  e->accept();
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
