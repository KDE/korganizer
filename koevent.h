// $Id$
// EVENT CLASS

#ifndef _KOEVENT_H
#define _KOEVENT_H

#include <qobject.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qstrlist.h>
#include <qbitarray.h>

#include "qdatelist.h"
#include "event.h"

class CalObject;
class KOEvent;
class VCalFormat;

/** This is a class which contains all the information necessary about a single
 * event, or an event that occurs multiple times through recurrence
 * information. Methods provide access to the data, which is as fully
 * encapsulated as possible.
 *
 * @short a class which provides an abstract view of an event or appointment.
 * @author Preston Brown
 * @version $Revision$
 */
class KOEvent : public Event {
//  friend class VCalFormat;
public:
  /** number of events created */
  static int eventCount;

  /** constructs a new event with variables initialized to "sane" values. */
  KOEvent();
  ~KOEvent();

  /** for setting the todo's due date/time with a QDateTime. */
  void setDtDue(const QDateTime &dtDue);
  /** returns an event's Due date/time as a QDateTime. */
  const QDateTime &getDtDue() const;
  /** returns an event's due time as a string formatted according to the
   users locale settings */
  QString getDtDueTimeStr() const;
  /** returns an event's due date as a string formatted according to the
   users locale settings */
  QString getDtDueDateStr(bool shortfmt=true) const;
  /** returns an event's due date and time as a string formatted according
   to the users locale settings */
  QString getDtDueStr() const;

  /** returns TRUE or FALSE depending on whether the todo has a due date */
  bool hasDueDate() const;
  /** sets the event's hasDueDate value. */
  void setHasDueDate(bool f);

  /** returns TRUE or FALSE depending on whether the todo has a start date */
  bool hasStartDate() const;
  /** sets the event's hasStartDate value. */
  void setHasStartDate(bool f);

  // -> Todo
  /** sets the event's status to the string specified.  The string
   * must be a recognized value for the status field, i.e. a string
   * equivalent of the possible status enumerations previously described. */
  void setStatus(const QString &statStr);
  /** sets the event's status to the value specified.  See the enumeration
   * above for possible values. */
  void setStatus(int);
  /** return the event's status. */
  int getStatus() const;
  /** return the event's status in string format. */
  QString getStatusStr() const;

  /** set the internal identifier for the event */
  void setEventId(int id);
  /** return the internal identifier for the event */
  int getEventId() const;

  void setTodoStatus(bool stat) { isTodo = stat; emit eventUpdated(this); };
  bool getTodoStatus() const { return isTodo; };

protected:

  // data variables
  int id;                              // globally unique ID for this event

  QDateTime dtDue;                     // due date of todo

  bool mHasDueDate;                    // if todo has associated due date
  bool mHasStartDate;                  // if todo has associated start date

  int  status;                         // confirmed/delegated/tentative/etc

  bool isTodo;                         // true if this is a "todo"
};

#endif
