/* 	$Id$	 */

#ifndef _CALOBJECT_H
#define _CALOBJECT_H

#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qlist.h>

#include "koevent.h"
#include "calformat.h"

#define _TIME_ZONE "-0500" /* hardcoded, overridden in config file. */

class KConfig;
class VCalDrag;
class ICalFormat;

// TODO: This class should be renamed to Calendar
/**
  * This is the main "calendar" object class for KOrganizer.  It holds
  * information like all appointments/events, user information, etc. etc.
  * one calendar is associated with each CalendarView (@see calendarview.h).
  * This is an abstract base class defining the interface to a calendar. It is
  * implemented by subclasses like @see CalendarLocal, which use different
  * methods to store and access the data.
  *
  * @short class providing an interface to a calendar
  * @author Preston Brown
  * @version $Revision$
  */
class CalObject : public QObject {
    Q_OBJECT
  public:
    /** constructs a new calendar, with variables initialized to sane values. */
    CalObject();
    virtual ~CalObject();

    /** Return the iCalendar format class the calendar object uses. */
    ICalFormat *iCalFormat();

    /** Set the widget the calendar belongs to */
    void setTopwidget(QWidget *topWidget);

    /** loads a calendar on disk into the current calendar.
     * Returns TRUE if successful, else returns FALSE.
     * @param fileName the name of the calendar on disk.
     */
    virtual bool load(const QString &fileName) = 0;
    /** writes out the calendar to disk in the format specified by the format
     * parameter. If the format is 0, vCalendar is used. Returns true if
     * successful and false on error.
     * @param fileName the name of the file
     */
    virtual bool save(const QString &fileName,CalFormat *format=0) = 0;
    /** clears out the current calendar, freeing all used memory etc. etc. */
    virtual void close() = 0;
  
    /** create an object to be used with the Xdnd Drag And Drop protocol. */
    VCalDrag *createDrag(KOEvent *selectedEv, QWidget *owner);
    /** create an object to be used with the Xdnd Drag And Drop protocol. */
    VCalDrag *createDragTodo(KOEvent *selectedEv, QWidget *owner);
    /** Create Todo object from drop event */
    KOEvent *createDropTodo(QDropEvent *de);
    /** Create Event object from drop event */
    KOEvent *createDrop(QDropEvent *de);
  
    /** cut event to clipboard */
    void cutEvent(KOEvent *);
    /** copy event to clipboard */
    bool copyEvent(KOEvent *);
    /** pastes the event and returns a pointer to the new event pasted. */
    KOEvent *pasteEvent(const QDate *, const QTime *newTime = 0L);
  
    /** set the owner of the calendar.  Should be owner's full name. */
    const QString &getOwner() const;
    /** return the owner of the calendar's full name. */
    void setOwner(const QString &os);
    /** set the email address of the calendar owner. */
    const QString &getEmail();
    /** return the email address of the calendar owner. */
    void setEmail(const QString &);
  
    /** set time zone from a timezone string (e.g. -2:00) */
    void setTimeZone(const QString & tz);
    /** set time zone froma aminutes value (e.g. -60) */
    void setTimeZone(int tz);
    /** Return time zone as offest in minutes */
    int getTimeZone() const;
    /* compute an ISO 8601 format string from the time zone. */
    QString getTimeZoneStr() const;
  
    /** adds a KOEvent to this calendar object.
     * @param anEvent a pointer to the event to add
     */
    virtual void addEvent(KOEvent *anEvent) = 0;
    /** deletes an event from this calendar. We could just use
     * a unique ID to search for the event, but using the date too is faster.
     * @param date the date upon which the event occurs.  
     * @param eventId the unique ID attached to the event
     */
    virtual void deleteEvent(const QDate &date, int eventId) = 0;
    /** Delete event from calendar */
    virtual void deleteEvent(KOEvent *) = 0;
    /** retrieves an event from the calendar, based on a date and an evenId.
     * faster than specifying an eventId alone. 
     */
    virtual KOEvent *getEvent(const QDate &date, int eventId) = 0;
    /** retrieves an event from the calendar on the basis of ID alone. */
    virtual KOEvent *getEvent(int eventId) = 0;
    /** retrieves an event on the basis of the unique string ID. */
    virtual KOEvent *getEvent(const QString &UniqueStr) = 0;
    /** builds and then returns a list of all events that match for the
     * date specified. useful for dayView, etc. etc. */
    virtual QList<KOEvent> getEventsForDate(const QDate &date, bool sorted = FALSE) = 0;
    /** Get events, which occur on the given date */
    virtual QList<KOEvent> getEventsForDate(const QDateTime &qdt) = 0;
    /** Get events in a range of dates. If inclusive is set to true, only events
     * are returned, which are completely included in the range. */
    virtual QList<KOEvent> getEvents(const QDate &start,const QDate &end,
                             bool inclusive=false) = 0;
    /** Return all events in calendar */
    virtual QList<KOEvent> getAllEvents() = 0;
  
    /*
     * returns a QString with the text of the holiday (if any) that falls
     * on the specified date.
     */
    QString getHolidayForDate(const QDate &qd);
    
    /** returns the number of events that are present on the specified date. */
    virtual int numEvents(const QDate &qd) = 0;
  
    /** add a todo to the todolist. */
    virtual void addTodo(KOEvent *todo) = 0;
    /** remove a todo from the todolist. */
    virtual void deleteTodo(KOEvent *) = 0;
    virtual const QList<KOEvent> &getTodoList() const = 0;
    /** searches todolist for an event with this id, returns pointer or null. */
    virtual KOEvent *getTodo(int id) = 0;
    /** searches todolist for an event with this unique string identifier,
      returns a pointer or null. */
    virtual KOEvent *getTodo(const QString &UniqueStr) = 0;
    /** Returns list of todos due on the specified date */
    virtual QList<KOEvent> getTodosForDate(const QDate & date) = 0;
  
    /* traversal methods */
    virtual KOEvent *first() = 0;
    virtual KOEvent *last() = 0;
    virtual KOEvent *next() = 0;
    virtual KOEvent *prev() = 0;
    virtual KOEvent *current() = 0;
  
    /** update internal position information */
    virtual void updateCursors(KOEvent *dEvent) = 0;

    /** Enable/Disable dialogs shown by calendar class */  
    void showDialogs(bool d);
  
  signals:
    /** emitted at regular intervals to indicate that the events in the
      list have triggered an alarm. */
    void alarmSignal(QList<KOEvent> &);
    /** emitted whenever an event in the calendar changes.  Emits a pointer
      to the changed event. */
    void calUpdated(KOEvent *);
  
  public slots:
    /** checks to see if any alarms are pending, and if so, returns a list
     * of those events that have alarms. */
    virtual void checkAlarms() = 0;
    /** Update configuration values from KOPrefs */
    void updateConfig();
   
  protected:
    /** Read name of holidayfile from config object */
    void readHolidayFileName();
  
    CalFormat *mFormat;
    ICalFormat *mICalFormat;
  
  private:
    QString mHolidayfile;  // name of file defining holidays
    QWidget *mTopWidget;   // topWidget this calendar belongs to
    QString mOwner;        // who the calendar belongs to
    QString mOwnerEmail;   // email address of the owner
    int mTimeZone;         // timezone OFFSET from GMT (MINUTES)
    bool mDialogsOn;       // display various GUI dialogs?
};
  
#endif
