/* $Id$ */

#ifndef _KDATENAV_H
#define _KDATENAV_H

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
