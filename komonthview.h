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

#ifndef _KOMONTHVIEW_H
#define _KOMONTHVIEW_H

#include <qlabel.h>
#include <qframe.h>
#include <qdatetime.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qintdict.h>
#include <qpushbutton.h>
#include <qvaluelist.h>

#include <libkcal/calendar.h>
#include <libkcal/event.h>

#include "koeventview.h"
#include "ksellabel.h" 

class KONavButton: public QPushButton
{
  public:
    KONavButton( QPixmap pixmap, QWidget *parent, const char *name=0 ) :
      QPushButton(parent,name)
    {
      // We should probably check, if the pixmap is valid.
      setPixmap(pixmap);
    }; 
    
    QSizePolicy sizePolicy() const
    {
      return QSizePolicy(QSizePolicy::Minimum,QSizePolicy::MinimumExpanding);
    }
};


class EventListBoxItem: public QListBoxItem
{
 public:
  EventListBoxItem(const QString & s);
  void setText(const QString & s)
    { QListBoxItem::setText(s); }
  void setRecur(bool on) 
    { recur = on; }
  void setAlarm(bool on)
    { alarm = on; }
  const QPalette &palette () const { return myPalette; };
  void setPalette(const QPalette &p) { myPalette = p; };

 protected:
  virtual void paint(QPainter *);
  virtual int height(const QListBox *) const;
  virtual int width(const QListBox *) const;
 private:
  bool    recur;
  bool    alarm;
  QPixmap alarmPxmp;
  QPixmap recurPxmp;
  QPalette myPalette;
};

class KNoScrollListBox: public QListBox {
  Q_OBJECT
 public:
  KNoScrollListBox(QWidget *parent=0, const char *name=0);
  ~KNoScrollListBox() {}

 signals:
  void shiftDown();
  void shiftUp();
  void rightClick();

 protected slots:
  void keyPressEvent(QKeyEvent *);
  void keyReleaseEvent(QKeyEvent *);
  void mousePressEvent(QMouseEvent *);
};

class KSummaries: public KNoScrollListBox {
  Q_OBJECT
 public:
  KSummaries(QWidget    *parent, 
	     Calendar  *calendar, 
	     QDate       qd       = QDate::currentDate(),
	     int         index    = 0,
	     const char *name     = 0);
  ~KSummaries() {}
  QDate getDate() { return(myDate); }
  void setDate(QDate);
  void calUpdated();
  Event *getSelected();
  
  QSize minimumSizeHint() const;

 signals:
  void daySelected(int index);
  void editEventSignal(Event *);

 protected slots:
  void itemHighlighted(int);
  void itemSelected(int);

 private:
   QDate               myDate;
   int                 idx, itemIndex;
   Calendar          *myCal;
   QIntDict<Event> *currIdxs; 
};

class KOMonthView: public KOEventView {
   Q_OBJECT
 public:
   KOMonthView(Calendar *cal,
		QWidget    *parent   = 0, 
		const char *name     = 0,
		QDate       qd       = QDate::currentDate());
   ~KOMonthView();

   enum { EVENTADDED, EVENTEDITED, EVENTDELETED };

   /** Returns maximum number of days supported by the komonthview */
   virtual int maxDatesHint();

   /** Returns number of currently shown dates. */
   virtual int currentDateCount();

   /** returns the currently selected events */
   virtual QPtrList<Incidence> selectedIncidences();

   virtual void printPreview(CalPrinter *calPrinter,
                             const QDate &, const QDate &);

 public slots:
   virtual void updateView();
   virtual void updateConfig();
   virtual void showDates(const QDate &start, const QDate &end);
   virtual void showEvents(QPtrList<Event> eventList);

   void changeEventDisplay(Event *, int);

 signals:
   void newEventSignal();  // From KOBaseView
   void newEventSignal(QDate);
   void newEventSignal(QDateTime, QDateTime);  // From KOBaseView
   void editEventSignal(Event *);  // From KOBaseView
   void deleteEventSignal(Event *);  // From KOBaseView
   void datesSelected(const DateList);  // From KOBaseView

 protected slots:
   void resizeEvent(QResizeEvent *);
   void goBackYear();
   void goForwardYear();
   void goBackMonth();
   void goForwardMonth();
   void goBackWeek();
   void goForwardWeek();
   void daySelected(int index);
   void newEventSlot(int index);
   void doRightClickMenu();
//   void newEventSelected() { emit newEventSignal(daySummaries[*selDateIdxs.first()]->getDate()); };
   void processSelectionChange();

 protected:
   void viewChanged();
   
 private:
   // date range label.
   QLabel           *dispLabel;
   // day display vars
   QLabel           *dayNames[7];
   KSelLabel        *dayHeaders[42];
   KSummaries       *daySummaries[42];
   bool              shortdaynames;
   bool              weekStartsMonday;

   // display control vars
   KOEventPopupMenu *rightClickMenu;

   // state data.
   QDate            myDate;
   Calendar        *myCal;
   QValueList<int>  selDateIdxs;
   QPalette         holidayPalette;
};

#endif
 
