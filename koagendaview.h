// $Id$
//
// KOAgendaView is the agenda-like view used to display events in an one or
// multi-day view.

#ifndef KOAGENDAVIEW_H 
#define KOAGENDAVIEW_H

//#include <qlabel.h>
//include <qframe.h>
//#include <qdatetm.h>
//#include <qlistbox.h>
//#include <qlayout.h>
//#include <qintdict.h>

//#include <kapp.h>

#include "kobaseview.h"

#include <qscrollview.h>
#include <qdatetime.h>
#include <qlayout.h>

class KOAgenda;
class KOAgendaItem;
class QHBox;
class QFrame;
class KConfig;

class TimeLabels : public QScrollView {
  public:
    TimeLabels(int rows,QWidget *parent=0,const char *name=0,WFlags f=0);
    
    void setCellHeight(int height);
    
  protected:
    void drawContents(QPainter *p,int cx, int cy, int cw, int ch);
    
  private:
    int mRows;
    int mCellHeight;
};

class KOAgendaView: public KOBaseView {
    Q_OBJECT
  public:
    KOAgendaView(CalObject *cal,QWidget *parent = 0,const char *name = 0);
    virtual ~KOAgendaView();

    /** Returns maximum number of days supported by the koagendaview */
    virtual int maxDatesHint();

    /** returns the currently selected events */
    virtual QList<KOEvent> getSelected();

    /** Agenda view types. DAY is a one day view, WORKWEEK is a 5 day view of a
    week, excluding the weekend, WEEK is a 7 day view of a complete week and
    LIST is a view of an arbitrary number of days */
    enum { DAY, WORKWEEK, WEEK, LIST };

    /** Return type of current agenda view */
    int currentView() { return mViewType; }

    /** Remove all events from view */
    void clearView();

   virtual void printPreview(CalPrinter *calPrinter,
                             const QDate &, const QDate &);
    
  public slots:
    virtual void updateView();
    virtual void updateConfig();
    virtual void selectDates(const QDateList);
    virtual void selectEvents(QList<KOEvent> eventList);

    void changeEventDisplay(KOEvent *, int);

    void slotViewChange(int newView);
    void slotViewChange();

    void slotNextDates();
    void slotPrevDates();

    void newEvent(int gx,int gy);
    void newEventAllDay(int gx, int gy);

  signals:
    void editEventSignal(KOEvent *);  // From KOBaseView
    void deleteEventSignal(KOEvent *);  // From KOBaseView
    void datesSelected(const QDateList);  // From KOBaseView
    void newEventSignal();  // From KOBaseView
    void newEventSignal(QDate);
    void newEventSignal(QDateTime);
    void newEventSignal(QDateTime, QDateTime);  // From KOBaseView

  protected:
    /** Fill agenda beginning with date startDate */
    void fillAgenda(const QDate &startDate);
    
    /** Fill agenda using the current set value for the start date */
    void fillAgenda();

    /** Create labels for the selected dates. */
    void createDayLabels();

    /** Set type of agenda view. See also the definitions above. */
    void setView( int ViewType );

  protected slots:

    /** Move TimeLabels, so that its positions correspond to the agenda. */
    void adjustTimeLabels();
    
    /** Update event belonging to agenda item */
    void updateEventDates(KOAgendaItem *item);

  private:
    // view widgets
    QFrame *mDayLabels;
    QHBox *mDayLabelsFrame;
    QBoxLayout *mLayoutDayLabels;
    QFrame *mAllDayFrame;
    KOAgenda *mAllDayAgenda;
    KOAgenda *mAgenda;
    TimeLabels *mTimeLabels;

    QDate mStartDate;  // Date of first event displayed
    QDateList mSelectedDates;  // List of dates to be displayed
    int mViewType;

    KConfig *mConfig;
    bool mWeekStartsMonday;
    int mStartHour;
};

#endif  // KOAGENDAVIEW_H
