// $Id$

#ifndef _VCALFORMAT_H
#define _VCALFORMAT_H

#include "calformat.h"

#define _VCAL_VERSION "1.0"


/**
  This class implements the vCalendar format. It provides methods for
  loading/saving/converting vCalendar format data into the internal KOrganizer
  representation as CalObject and Events.

  @short vCalendar format implementation
  @author Preston Brown
  @version $Revision$
*/
/**
  * This is the main "calendar" object class for KOrganizer.  It holds
  * information like all appointments/events, user information, etc. etc.
  * one calendar is associated with each TopWidget (@see topwidget.h).
  *
  * @short class providing in interface to a calendar
  * @author Preston Brown
  * @version $Revision$
  */
class VCalFormat : public CalFormat {
  public:
    /** constructs a new calendar, with variables initialized to sane values. */
    VCalFormat(CalObject *);
    virtual ~VCalFormat();

   /** loads a calendar on disk in vCalendar format into the current calendar.
     * any information already present is lost. Returns TRUE if successful,
     * else returns FALSE.
     * @param fileName the name of the calendar on disk.
     */
    bool load(const QString &fileName);
    /** writes out the calendar to disk in vCalendar format. Returns true if
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
    
  protected:
    /** translates a VObject of the TODO type into a Event */
    Todo *VTodoToEvent(VObject *vtodo);
    /** translates a VObject into a Event and returns a pointer to it. */
    Event *VEventToEvent(VObject *vevent);
    /** translate a Event into a VTodo-type VObject and return pointer */
    VObject *eventToVTodo(const Todo *anEvent);
    /** translate a Event into a VObject and returns a pointer to it. */
    VObject* eventToVEvent(const Event *anEvent);

    /** takes a QDate and returns a string in the format YYYYMMDDTHHMMSS */
    QString qDateToISO(const QDate &);
    /** takes a QDateTime and returns a string in format YYYYMMDDTHHMMSS */
    QString qDateTimeToISO(const QDateTime &, bool zulu=TRUE);
    /** takes a string in the format YYYYMMDDTHHMMSS and returns a 
     * valid QDateTime. */
    QDateTime ISOToQDateTime(const QString & dtStr);
    /** takes a vCalendar tree of VObjects, and puts all of them that have
     * the "event" property into the dictionary, todos in the todo-list, etc. */
    void populate(VObject *vcal);
  
    /** takes a number 0 - 6 and returns the two letter string of that day,
      * i.e. MO, TU, WE, etc. */
    const char *dayFromNum(int day);
    /** the reverse of the above function. */
    int numFromDay(const QString &day);
  
    /** shows an error dialog box. */
    void parseError(const char *prop);
  
  private:
    QList<Event> mEventsRelate;           // events with relations
    QList<Todo> mTodosRelate;             // todos with relations
};

#endif
