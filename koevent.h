// $Id$
// EVENT CLASS

#ifndef _KOEVENT_H
#define _KOEVENT_H

#include <qobject.h>
#include <qdatetm.h>
#include <qstring.h>
#include <qstrlist.h>
#include <qbitarry.h>

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

  /** for setting the event's starting date/time with a QDateTime. */
  void setDtStart(const QDateTime &dtStart);
  /** returns an event's starting date/time as a QDateTime. */
  const QDateTime &getDtStart() const;
  /** returns an event's starting time as a string formatted according to the
   users locale settings */
  QString getDtStartTimeStr() const;
  /** returns an event's starting date as a string formatted according to the
   users locale settings */
  QString getDtStartDateStr(bool shortfmt=true) const;
  /** returns an event's starting date and time as a string formatted according
   to the users locale settings */
  QString getDtStartStr() const;

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

  /** for setting an event's ending date/time with a QDateTime. */
  void setDtEnd(const QDateTime &dtEnd);
  /** returns an event's ending date/time as a QDateTime. */
  const QDateTime &getDtEnd() const;
  /** returns an event's end time as a string formatted according to the
   users locale settings */
  QString getDtEndTimeStr() const;
  /** returns an event's end date as a string formatted according to the
   users locale settings */
  QString getDtEndDateStr(bool shortfmt=true) const;
  /** returns an event's end date and time as a string formatted according
   to the users locale settings */
  QString getDtEndStr() const;

  /** returns TRUE or FALSE depending on whether the event "floats,"
   * or doesn't have a time attached to it, only a date. */
  bool doesFloat() const;
  /** sets the event's float value. */
  void setFloats(bool f);

  /** returns TRUE or FALSE depending on whether the todo has a due date */
  bool hasDueDate() const;
  /** sets the event's hasDueDate value. */
  void setHasDueDate(bool f);

  /** returns TRUE or FALSE depending on whether the todo has a start date */
  bool hasStartDate() const;
  /** sets the event's hasStartDate value. */
  void setHasStartDate(bool f);

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

  /** sets the event's secrecy to the string specified.  The string
   * must be one of PUBLIC, PRIVATE, or CONFIDENTIAL. */
  void setSecrecy(const QString &secrecy);
  /** sets the event's secrecy to the string specified.  The string
   * must be one of PUBLIC, PRIVATE, or CONFIDENTIAL. */
  void setSecrecy(const char *);
  /** sets the event's status the value specified.  See the enumeration
   * above for possible values. */
  void setSecrecy(int);
  /** return the event's secrecy. */
  int getSecrecy() const;
  /** return the event's secrecy in string format. */
  QString getSecrecyStr() const;

  /** set the list of attachments/associated files for this event */
  void setAttachments(const QStringList &attachments);
  /** return list of associated files */
  const QStringList &getAttachments() const;

  /** set resources used, such as Office, Car, etc. */
  void setResources(const QStringList &resources);
  /** return list of current resources */
  const QStringList &getResources() const;

  /** set the event's priority, 0 is undefined, 1 highest (decreasing order) */
  void setPriority(int priority);
  /** get the event's priority */
  int getPriority() const;
  /** set the event's time transparency level. */
  void setTransparency(int transparency);
  /** get the event's time transparency level. */
  int getTransparency() const;

  /** set the internal identifier for the event */
  void setEventId(int id);
  /** return the internal identifier for the event */
  int getEventId() const;

  /** pilot syncronization routines */
  enum { SYNCNONE = 0, SYNCMOD = 1, SYNCDEL = 3 };
  void setPilotId(int id);
  int getPilotId() const;

  void setSyncStatus(int stat);
  int getSyncStatus() const;

  void setTodoStatus(bool stat) { isTodo = stat; emit eventUpdated(this); };
  bool getTodoStatus() const { return isTodo; };

  bool isMultiDay() const;

protected:

  // data variables
  int id;                              // globally unique ID for this event

  QDateTime dtEnd;                     // end time for event.  Events with
                                       // only end time or start time
                                       // take up "no space".

  bool floats;			       // floating means date without time

  QDateTime dtDue;                     // due date of todo

  bool mHasDueDate;                    // if todo has associated due date
  bool mHasStartDate;                  // if todo has associated start date

  int  status;                         // confirmed/delegated/tentative/etc
  int  secrecy;                        // public/private/confidential
  QStringList attachments;                // attached files, sounds, anything!
  QStringList resources;                  // a list of resources needed for event

  int priority;                        // 1 = highest, 2 = less, etc.
  int transparency;                    // how transparent the event is to
                                       // free time requests. 0 = always block,
                                       // 1 = never block.
  bool isTodo;                         // true if this is a "todo"

  // PILOT SYNCHRONIZATION STUFF
  int pilotId;                         // unique id for pilot sync
  int syncStatus;                      // status (for sync)
};

#endif
