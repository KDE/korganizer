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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOAGENDAVIEW_H
#define KOAGENDAVIEW_H
// $Id$
//
// KOAgendaView is the agenda-like view used to display events in an one or
// multi-day view.

#include <qscrollview.h>
#include <qdatetime.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qmemarray.h>

#include "koeventview.h"

class KOAgenda;
class KOAgendaItem;
class QHBox;
class QFrame;
class KConfig;

class TimeLabels : public QScrollView {
    Q_OBJECT
  public:
    TimeLabels(int rows,QWidget *parent=0,const char *name=0,WFlags f=0);

    void setCellHeight(int height);

    /** Calculates the minimum width */
    virtual int minimumWidth() const;

    /** updates widget's internal state */
    void updateConfig();

    /**  */
    void setAgenda(KOAgenda* agenda);

    /**  */
    virtual void paintEvent(QPaintEvent* e);

  public slots:
    /** update time label positions */
    void positionChanged();

  protected:
    void drawContents(QPainter *p,int cx, int cy, int cw, int ch);

  private:
    int mRows;
    int mCellHeight;

    /**  */
    KOAgenda* mAgenda;
};

class EventIndicator : public QFrame {
    Q_OBJECT
  public:
    enum Location { Top, Bottom };
    EventIndicator(Location loc=Top,QWidget *parent=0,const char *name=0);
    virtual ~EventIndicator();

    void changeColumns(int columns);

    void enableColumn(int column, bool enable);

  protected:
    void drawContents(QPainter *);

  private:
    int mColumns;
    QHBox *mTopBox;
    QBoxLayout *mTopLayout;
    Location mLocation;
    QPixmap mPixmap;
    QMemArray<bool> mEnabled;
};

class KOAgendaView : public KOEventView {
    Q_OBJECT
  public:
    KOAgendaView(Calendar *cal,QWidget *parent = 0,const char *name = 0);
    virtual ~KOAgendaView();

    /** Returns maximum number of days supported by the koagendaview */
    virtual int maxDatesHint();

    /** Returns number of currently shown dates. */
    virtual int currentDateCount();

    /** returns the currently selected events */
    virtual QPtrList<Incidence> selectedIncidences();

    /** Agenda view types. DAY is a one day view, WORKWEEK is a 5 day view of a
    week, excluding the weekend, WEEK is a 7 day view of a complete week and
    LIST is a view of an arbitrary number of days */
    enum { DAY, WORKWEEK, WEEK, LIST };

    /** Set type of agenda view. See also the definitions above. */
    void setView( int ViewType );
    /** Return type of current agenda view */
    int currentView() { return mViewType; }

    /** Remove all events from view */
    void clearView();

    virtual void printPreview(CalPrinter *calPrinter,
                              const QDate &, const QDate &);

  public slots:
    virtual void updateView();
    virtual void updateConfig();
    virtual void showDates(const QDate &start, const QDate &end);
    virtual void showEvents(QPtrList<Event> eventList);

    void changeEventDisplay(Event *, int);

    void slotViewChange(int newView);
    void slotViewChange();

    void slotNextDates();
    void slotPrevDates();

    void newEvent(int gx,int gy);
    void newEventAllDay(int gx, int gy);

    void startDrag(Event *);

    void readSettings();
    void readSettings(KConfig *);
    void writeSettings(KConfig *);

    void setContentsPos(int y);

    void setExpandedButton( bool expanded );

  signals:
    void editEventSignal(Event *);  // From KOBaseView
    void showEventSignal(Event *);
    void deleteEventSignal(Event *);  // From KOBaseView
    void datesSelected(const DateList &);  // From KOBaseView
    void newEventSignal();  // From KOBaseView
    void newEventSignal(QDate);
    void newEventSignal(QDateTime);
    void newEventSignal(QDateTime, QDateTime);  // From KOBaseView

    void toggleExpand();

  protected:
    /** Fill agenda beginning with date startDate */
    void fillAgenda(const QDate &startDate);

    /** Fill agenda using the current set value for the start date */
    void fillAgenda();

    /** Create labels for the selected dates. */
    void createDayLabels();

    /**
      Set the masks on the agenda widgets indicating, which days are holidays.
    */
    void setHolidayMasks();

    /** Displays the next set of dates by going forward, (mul = +1),
	or backwards (mul = -1).
    */
    void shiftDates(int multiplier);

  protected slots:

    /** Move TimeLabels, so that its positions correspond to the agenda. */
    //void adjustTimeLabels();

    /** Update event belonging to agenda item */
    void updateEventDates(KOAgendaItem *item);

    void showAgendaPopup(Event *event);
    void showAllDayAgendaPopup(Event *event);

    void updateEventIndicatorTop(int newY);
    void updateEventIndicatorBottom(int newY);

  private:
    // view widgets
    QFrame *mDayLabels;
    QHBox *mDayLabelsFrame;
    QBoxLayout *mLayoutDayLabels;
    QFrame *mAllDayFrame;
    KOAgenda *mAllDayAgenda;
    KOAgenda *mAgenda;
    TimeLabels *mTimeLabels;
    QWidget *mDummyAllDayLeft;
    QSplitter *mSplitterAgenda;
    QPushButton *mExpandButton;

    DateList mSelectedDates;  // List of dates to be displayed
    int mViewType;

    bool mWeekStartsMonday;
    int mStartHour;

    KOEventPopupMenu *mAgendaPopup;
    KOEventPopupMenu *mAllDayAgendaPopup;

    EventIndicator *mEventIndicatorTop;
    EventIndicator *mEventIndicatorBottom;

    QMemArray<int> mMinY;
    QMemArray<int> mMaxY;

    QMemArray<bool> mHolidayMask;
    
    QPixmap mExpandedPixmap;
    QPixmap mNotExpandedPixmap;
};

#endif  // KOAGENDAVIEW_H
