// $Id$

#ifndef HTMLEXPORT_H
#define HTMLEXPORT_H

#include <qstring.h>
#include <qdatetime.h>

class QFile;
class QTextStream;

class CalObject;
class Incidence;
class Todo;
class Event;

/**
  This class provides the functions to export a calendar as a HTML page.
*/
class HtmlExport {
  public:
    /** Create new HTML exporter for calendar */
    HtmlExport(CalObject *calendar) :
        mCalendar(calendar),
        mMonthViewEnabled(true),mEventsEnabled(false),mTodosEnabled(true),
        mCategoriesTodoEnabled(false),mAttendeesTodoEnabled(false),
        mCategoriesEventEnabled(false),mAttendeesEventEnabled(false),
        mDueDateEnabled(false) {}
    virtual ~HtmlExport() {};

    /**
      writes out the calendar in HTML format.
    */
    bool save(const QString &fileName);

    /**
      writes out calendar to file. The QFile has to already be opened for writing.
    */
    bool save(QFile *);

    void setMonthViewEnabled(bool enable=true) { mMonthViewEnabled = enable; }
    bool monthViewEnabled() { return mMonthViewEnabled; }

    void setEventsEnabled(bool enable=true) { mEventsEnabled = enable; }
    bool eventsEnabled() { return mEventsEnabled; }

    void setTodosEnabled(bool enable=true) { mTodosEnabled = enable; }
    bool todosEnabled() { return mTodosEnabled; }
  
    void setCategoriesTodoEnabled(bool enable=true) { mCategoriesTodoEnabled = enable; }
    bool categoriesTodoEnabled() { return mCategoriesTodoEnabled; }
  
    void setAttendeesTodoEnabled(bool enable=true) { mAttendeesTodoEnabled = enable; }
    bool attendeesTodoEnabled() { return mAttendeesTodoEnabled; }
  
    void setCategoriesEventEnabled(bool enable=true) { mCategoriesEventEnabled = enable; }
    bool categoriesEventEnabled() { return mCategoriesEventEnabled; }
  
    void setAttendeesEventEnabled(bool enable=true) { mAttendeesEventEnabled = enable; }
    bool attendeesEventEnabled() { return mAttendeesEventEnabled; }
  
    void setDueDateEnabled(bool enable=true) { mDueDateEnabled = enable; }
    bool dueDateEnabled() { return mDueDateEnabled; }
  
    void setDateRange(const QDate &from,const QDate &to) { mFromDate = from, mToDate = to; }
    QDate fromDate() { return mFromDate; }
    QDate toDate() { return mToDate; }
  
  protected:

    void createHtmlMonthView (QTextStream *ts);  
    void createHtmlEventList (QTextStream *ts);
    void createHtmlTodoList (QTextStream *ts);

    void createHtmlTodo (QTextStream *ts,Todo *todo);
    void createHtmlEvent (QTextStream *ts,Event *event,QDate date, bool withDescription = true);

    void formatHtmlCategories (QTextStream *ts,Incidence *event);
    void formatHtmlAttendees (QTextStream *ts,Incidence *event);

  private:
    CalObject *mCalendar;

    bool mMonthViewEnabled;
    bool mEventsEnabled;
    bool mTodosEnabled;
    bool mCategoriesTodoEnabled;
    bool mAttendeesTodoEnabled;
    bool mCategoriesEventEnabled;
    bool mAttendeesEventEnabled;
    bool mDueDateEnabled;
    
    QDate mFromDate;
    QDate mToDate;
};

#endif
