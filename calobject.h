/* 	$Id$	 */

#ifndef _CALOBJECT_H
#define _CALOBJECT_H

#include <qobject.h>
#include <qstring.h>
#include <qdatetm.h>
#include <qintdict.h>
#include <qdict.h>
#include <qlist.h>
#include <qstrlist.h>
#include <qregexp.h>

#include "vcc.h"
#include "vobject.h"
#include "koevent.h"

#define _PRODUCT_ID "-//K Desktop Environment//NONSGML KOrganizer//EN"
#define _TIME_ZONE "-0500" /* hardcoded, overridden in config file. */
#define _VCAL_VERSION "1.0"
#define BIGPRIME 1031 /* should allow for at least 4 appointments 365 days/yr
			 to be almost instantly fast. */

class KConfig;
class VCalDrag;

/**
  * This is the main "calendar" object class for KOrganizer.  It holds
  * information like all appointments/events, user information, etc. etc.
  * one calendar is associated with each TopWidget (@see topwidget.h).
  *
  * @short class providing in interface to a calendar
  * @author Preston Brown
  * @version $Revision$
  */
class CalObject : public QObject {
  Q_OBJECT

public:
  /** constructs a new calendar, with variables initialized to sane values. */
  CalObject();
  virtual ~CalObject();

  void setTopwidget(QWidget *_topWidget) { topWidget = _topWidget; }

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
  /** clears out the current calendar, freeing all used memory etc. etc. */
  void close();

  /** create an object to be used with the Xdnd Drag And Drop protocol. */
  VCalDrag *createDrag(KOEvent *selectedEv, QWidget *owner);
  /** create an object to be used with the Xdnd Drag And Drop protocol. */
  VCalDrag *createDragTodo(KOEvent *selectedEv, QWidget *owner);
  /** Create Todo object from drop event */
  KOEvent *createDropTodo(QDropEvent *de);
  /** Create Event object from drop event */
  KOEvent *createDrop(QDropEvent *de);

  /** cut, copy, and paste operations follow. */
  void cutEvent(KOEvent *);
  bool copyEvent(KOEvent *);
  /** pastes the event and returns a pointer to the new event pasted. */
  KOEvent *pasteEvent(const QDate *, const QTime *newTime = 0L,
		       VObject *vc=0L);

  /** set the owner of the calendar.  Should be owner's full name. */
  const QString &getOwner() const;
  /** return the owner of the calendar's full name. */
  void setOwner(const QString &os);
  /** set the email address of the calendar owner. */
  inline const QString &getEmail() { return emailString; };
  /** return the email address of the calendar owner. */
  inline void setEmail(const QString &es) { emailString = es; };
  /** adds a KOEvent to this calendar object.
   * @param anEvent a pointer to the event to add
   */

  /** methods to get/set the local time zone */
  void setTimeZone(const QString & tz);
  inline void setTimeZone(int tz) { timeZone = tz; };
  int getTimeZone() const { return timeZone; };
  /* compute an ISO 8601 format string from the time zone. */
  QString getTimeZoneStr() const;

  void addEvent(KOEvent *anEvent);
  /** deletes an event from this calendar. We could just use
   * a unique ID to search for the event, but using the date too is faster.
   * @param date the date upon which the event occurs.  
   * @param eventId the unique ID attached to the event
   */
  void deleteEvent(const QDate &date, int eventId);
  void deleteEvent(KOEvent *);
  /** retrieves an event from the calendar, based on a date and an evenId.
   * faster than specifying an eventId alone. 
   */
  KOEvent *getEvent(const QDate &date, int eventId);
  /** retrieves an event from the calendar on the basis of ID alone. */
  KOEvent *getEvent(int eventId);
  /** retrieves an event on the basis of the unique string ID. */
  KOEvent *getEvent(const QString &UniqueStr);
  /** builds and then returns a list of all events that match for the
   * date specified. useful for dayView, etc. etc. */
  QList<KOEvent> getEventsForDate(const QDate &date, bool sorted = FALSE);
  QList<KOEvent> getEventsForDate(const QDateTime &qdt);
  /** Get events in a range of dates. If inclusive is set to true, only events
   * are returned, which are completely included in the range. */
  QList<KOEvent> getEvents(const QDate &start,const QDate &end,
                           bool inclusive=false);

  /*
   * returns a QString with the text of the holiday (if any) that falls
   * on the specified date.
   */
  QString getHolidayForDate(const QDate &qd);
  
  /** returns the number of events that are present on the specified date. */
  int numEvents(const QDate &qd);

  /** add a todo to the todolist. */
  void addTodo(KOEvent *todo);
  /** remove a todo from the todolist. */
  void deleteTodo(KOEvent *);
  const QList<KOEvent> &getTodoList() const;
  /** searches todolist for an event with this id, returns pointer or null. */
  KOEvent *getTodo(int id);
  /** searches todolist for an event with this unique string identifier,
    returns a pointer or null. */
  KOEvent *getTodo(const QString &UniqueStr);
  /** Returns list of todos due on the specified date */
  QList<KOEvent> getTodosForDate(const QDate & date);

  /** traversal methods */
  KOEvent *first();
  KOEvent *last();
  KOEvent *next();
  KOEvent *prev();
  KOEvent *current();

  /** translates a VObject of the TODO type into a KOEvent */
  KOEvent *VTodoToEvent(VObject *vtodo);
  /** translates a VObject into a KOEvent and returns a pointer to it. */
  KOEvent *VEventToEvent(VObject *vevent);
  /** translate a KOEvent into a VTodo-type VObject and return pointer */
  VObject *eventToVTodo(const KOEvent *anEvent);
  /** translate a KOEvent into a VObject and returns a pointer to it. */
  VObject* eventToVEvent(const KOEvent *anEvent);
  QList<KOEvent> search(const QRegExp &) const;

  void showDialogs(bool d) { dialogsOn = d; };

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
  void checkAlarms();
  void updateConfig();
 
protected slots:
  /** this method should be called whenever a KOEvent is modified directly
   * via it's pointer.  It makes sure that the calObject is internally
   * consistent. */
  void updateEvent(KOEvent *anEvent);

protected:
  /* methods */

  /** inserts an event into its "proper place" in the calendar. */
  void insertEvent(const KOEvent *anEvent);

  /** on the basis of a QDateTime, forms a hash key for the dictionary. */
  long int makeKey(const QDateTime &dt);
  /** on the basis of a QDate, forms a hash key for the dictionary */
  long int makeKey(const QDate &d);
  /** Return the date for which the specified key was made. */
  QDate keyToDate(long int key);

  /** update internal position information */
  void updateCursors(KOEvent *dEvent);

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
  void loadError(const QString &fileName);
  /** shows an error dialog box. */
  void parseError(const char *prop);
  /** Read name of holidayfile from config object */
  void readHolidayFileName();

  /* variables */
  QWidget *topWidget;                     // topWidget this calendar belongs to
  QString ownerString;                    // who the calendar belongs to
  QString emailString;                    // email address of the owner
  int timeZone;                           // timezone OFFSET from GMT (MINUTES)
  QIntDict<QList<KOEvent> > *calDict;    // dictionary of lists of events.
  QList<KOEvent> recursList;             // list of repeating events.

  QList<KOEvent> todoList;               // list of "todo" items.

  QDate *oldestDate;
  QDate *newestDate;
  QDate cursorDate;                       // last date we were looking at.
  QListIterator<KOEvent> *cursor;        // for linear traversal methods
  QListIterator<KOEvent> recursCursor;   // for linear traversal methods

  bool dialogsOn;                         // display various GUI dialogs?

private:
  QList<KOEvent> mEventsRelate;           // events with relations
  QList<KOEvent> mTodosRelate;             // todos with relations

  QString mHolidayfile;  // name of file defining holidays
};

#ifdef __cplusplus
extern "C" {
#endif
char *parse_holidays(const char *, int year, short force);
struct holiday {
  char            *string;        /* name of holiday, 0=not a holiday */
  unsigned short  dup;            /* reference count */
};   
extern struct holiday holiday[366];
#if __cplusplus
};
#endif

#endif
