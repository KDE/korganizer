// $Id$

#ifndef _ICALFORMAT_H
#define _ICALFORMAT_H

#include <qstring.h>

#include "scheduler.h"

#include "calformat.h"

extern "C" {
  #include <ical.h>
  #include <icalss.h>
/*
  #include "icalfileset.h"
  #include "icalclassify.h"
*/
}

/**
  This class implements the iCalendar format. It provides methods for
  loading/saving/converting iCalendar format data into the internal KOrganizer
  representation as CalObject and KOEvents.

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
    VCalDrag *createDrag(KOEvent *selectedEv, QWidget *owner);
    /** create an object to be used with the Xdnd Drag And Drop protocol. */
    VCalDrag *createDragTodo(KOEvent *selectedEv, QWidget *owner);
    /** Create Todo object from drop event */
    KOEvent *createDropTodo(QDropEvent *de);
    /** Create Event object from drop event */
    KOEvent *createDrop(QDropEvent *de);
  
    /** cut, copy, and paste operations follow. */
    bool copyEvent(KOEvent *);
    /** pastes the event and returns a pointer to the new event pasted. */
    KOEvent *pasteEvent(const QDate *, const QTime *newTime = 0L);
    
    QString createScheduleMessage(KOEvent *,Scheduler::Method);
    ScheduleMessage *parseScheduleMessage(const QString &);
    
  protected:
    void populate(icalfileset *fs);

    icalcomponent *writeTodo(KOEvent *todo);
    icalcomponent *writeEvent(KOEvent *event);
    void writeIncidence(icalcomponent *parent,KOEvent *incidence);
    icalproperty *writeRecurrenceRule(KOEvent *event);

    QString extractErrorProperty(icalcomponent *);    
    KOEvent *readTodo(icalcomponent *vtodo);
    KOEvent *readEvent(icalcomponent *vevent);
    Attendee *readAttendee(icalproperty *attendee);
    void readIncidence(icalcomponent *parent,KOEvent *incidence);
    void readRecurrenceRule(icalproperty *rrule,KOEvent *event);

    icaltimetype writeICalDate(const QDate &);
    icaltimetype writeICalDateTime(const QDateTime &);
    QDate readICalDate(icaltimetype);
    QDateTime readICalDateTime(icaltimetype);
    char *writeText(const QString &);
    icalcomponent *createCalendarComponent();
    icalcomponent *createScheduleComponent(KOEvent *,Scheduler::Method);

    /** shows an error dialog box. */
    void parseError(const char *prop);
  
  private:
    QList<KOEvent> mEventsRelate;           // events with relations
    QList<KOEvent> mTodosRelate;             // todos with relations
};

#endif
