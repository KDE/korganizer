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
#ifndef KDATENAVIGATOR_H
#define KDATENAVIGATOR_H

#include <qframe.h>
#include <qdatetime.h>
#include <qlabel.h>

#include <libkcal/calendar.h>

#include "kodaymatrix.h"

class QPushButton;

class KCalendarSystem;

class KDateNavigator: public QFrame
{
   Q_OBJECT
 public:
   KDateNavigator( QWidget *parent = 0, Calendar *calendar = 0,
                   bool show_week_numbers = false, const char *name = 0,
                   QDate date = QDate::currentDate(),
                   KCalendarSystem *calSys = 0 );
   ~KDateNavigator();

   DateList selectedDates();

   void gotoYMD(int yr, int mth, int day);

 public slots:
   void selectDates(const DateList &);
   void selectDates(QDate);
   void addSelection(const DateList);
   void setShowWeekNums(bool enabled);
   void updateView();
   void updateConfig();
   void shiftEvent(const QDate& , const QDate&);

 signals:
   void datesSelected(const DateList &);
   void eventDropped(Event *);
   void weekClicked(QDate);

 protected slots:

//+   void goNextWeek();
//+   void goPrevWeek();

   void goNextMonth();
   void goPrevMonth();
   void goNextYear();
   void goPrevYear();

 protected:
   void updateDates();

   bool eventFilter (QObject *,QEvent *);

 private:
   QFrame   *ctrlFrame;
   QPushButton *prevYear;
   QPushButton *prevMonth;
   QPushButton *nextMonth;
   QPushButton *nextYear;
   QLabel *dateLabel;
   QFrame *headingSep;
   QFrame *weeknumSep;
   QLabel *headings[7];
   QLabel *weeknos[7];
   KODayMatrix *daymatrix;

   DateList mSelectedDates;
   QDate m_MthYr;
   int m_fstDayOfWk;
   bool m_bShowWeekNums;

   int dayNum(int row, int col);
   int dayToIndex(int dayNum);

   Calendar *mCalendar;
   KCalendarSystem *mCalendarSystem; 

   const QString *curHeaders;

   // Disabling copy constructor and assignment operator
   KDateNavigator(const KDateNavigator & );
   KDateNavigator &operator=(const KDateNavigator &);
};

#endif
