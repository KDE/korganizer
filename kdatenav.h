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
*/
#ifndef _KDATENAV_H
#define _KDATENAV_H
// $Id$

#include <qframe.h>
#include <qdatetime.h>
#include <qlabel.h>

#include <libkcal/qdatelist.h>
#include <libkcal/calendar.h>

#include "kdpdatebutton.h"

class QPushButton;

class KDateNavigator: public QFrame {
   Q_OBJECT
 public:
   KDateNavigator(QWidget *parent=0, Calendar *calendar=0,
		  bool show_week_numbers=FALSE,
		  const char *name=0,
		  QDate date=QDate::currentDate());
   ~KDateNavigator();

   const QDateList getSelected();

   void gotoYMD(int yr, int mth, int day);
   
 public slots:
   void selectDates(const QDateList);
   void selectDates(QDate);
   void addSelection(QDate, int, bool);
   void setShowWeekNums(bool enabled);
   void updateView();
   void updateConfig();
   
 signals:
   void datesSelected(const QDateList);
   void eventDropped(Event *);
   void weekClicked(QDate);

 protected slots:
   void goNextMonth();
   void goPrevMonth();
   void goNextYear();
   void goPrevYear();
   void updateButton(int);

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
   QFrame *viewFrame;
   QFrame *headingSep;
   QFrame *weeknumSep;
   QLabel *headings[7];
   QLabel *weeknos[7];
   KDateButton *buttons[42];

   QDateList selectedDates;
   QDate m_MthYr;
   int m_fstDayOfWk;
   bool m_bShowWeekNums;

   int dayNum(int row, int col);
   int dayToIndex(int dayNum);
   void fixupSelectedDates(int yr, int mth);

   Calendar *mCalendar;

   const QString *curHeaders;

   // Disabling copy constructor and assignment operator 
   KDateNavigator(const KDateNavigator & );
   KDateNavigator &operator=(const KDateNavigator &);
};

#endif
