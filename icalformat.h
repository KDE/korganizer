// $Id$

#ifndef _ICALFORMAT_H
#define _ICALFORMAT_H

#include <qstring.h>

#include "scheduler.h"

#include "calformat.h"

extern "C" {
  #include <ical.h>
  #include <icalss.h>
}

/**
  This class implements the iCalendar format. It provides methods for
  loading/saving/converting iCalendar format data into the internal KOrganizer
  representation as CalObject and Events.

  @short iCalendar format implementation
  @author Cornelius Schumacher
  @version $Revision$
*/

class ICalFormat : public CalFormat {
  public:
    /** Create new iCal format for calendar object */
    ICalFormat(CalObject *);
    virtual ~ICalFormat();

    /**
      loads a calendar on disk in iCalendar format  into current calendar.
      Returns TRUE if successful, else returns FALSE. Provides more error
      information by exception().
      @param fileName the name of the calendar on disk.
    */
    bool load(const QString &fileName);
    /** writes out the calendar to disk in iCalendar format. Returns true if
     * successful and false on error.
     * @param fileName the name of the file
     */
    bool save(const QString &fileName);
  
    /** create an object to be used with the Xdnd Drag And Drop protocol. */
    VCalDrag *createDrag(Event *selectedEv, QWidget *owner);
    /** create an object to be used with the Xdnd Drag And Drop protocol. */
    VCalDrag *createDragTodo(Todo *selectedEv, QWidget *owner);
    /** Create Todo object from drop event */
    Todo *createDropTodo(QDropEvent *de);
    /** Create Event object from drop event */
    Event *createDrop(QDropEvent *de);
  
    /** cut, copy, and paste operations follow. */
    bool copyEvent(Event *);
    /** pastes the event and returns a pointer to the new event pasted. */
    Event *pasteEvent(const QDate *, const QTime *newTime = 0L);
    
    QString createScheduleMessage(Event *,Scheduler::Method);
    ScheduleMessage *parseScheduleMessage(const QString &);
    
  protected:
    void populate(icalfileset *fs);

    icalcomponent *writeTodo(Todo *todo);
    icalcomponent *writeEvent(Event *event);
    void writeIncidence(icalcomponent *parent,Incidence *incidence);
    icalproperty *writeRecurrenceRule(Event *event);

    QString extractErrorProperty(icalcomponent *);    
    Todo *readTodo(icalcomponent *vtodo);
    Event *readEvent(icalcomponent *vevent);
    Attendee *readAttendee(icalproperty *attendee);
    void readIncidence(icalcomponent *parent,Incidence *incidence);
    void readRecurrenceRule(icalproperty *rrule,Incidence *event);

    icaltimetype writeICalDate(const QDate &);
    icaltimetype writeICalDateTime(const QDateTime &);
    QDate readICalDate(icaltimetype);
    QDateTime readICalDateTime(icaltimetype);
    char *writeText(const QString &);
    icalcomponent *createCalendarComponent();
    icalcomponent *createScheduleComponent(Incidence *,Scheduler::Method);

    /** shows an error dialog box. */
    void parseError(const char *prop);
  
  private:
    QList<Event> mEventsRelate;           // events with relations
    QList<Todo> mTodosRelate;             // todos with relations
};

#endif
