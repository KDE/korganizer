// 	$Id$	
// EVENT CLASS

#ifndef _KDPEVENT_H
#define _KDPEVENT_H

#include <qobject.h>
#include <qdatetm.h>
#include <qstring.h>
#include <qstrlist.h>
#include <qbitarry.h>

#include "qdatelist.h"

class CalObject;
class KOEvent;

class Attendee
{
  friend KOEvent;

public:
  enum { NEEDS_ACTION = 0, ACCEPTED = 1, SENT = 2, TENTATIVE = 3,
	 CONFIRMED = 4, DECLINED = 5, COMPLETED = 6, DELEGATED = 7 };
  // used to tell whether we have need to mail this person or not.
  bool flag;
  Attendee(const char *n, const char *e = 0L,
	   bool _rsvp=FALSE, int s = NEEDS_ACTION, int r = 0);
  Attendee(const Attendee &);
  virtual ~Attendee();
  void setName(const char *n) { name = n; }
  void setName(const QString &n) { name = n; }
  const QString &getName() const { return name; }
  void setEmail(const char *e) { email = e; }
  void setEmail(const QString e) { email = e; }
  const QString &getEmail() const { return email; }
  void setRole(int r) { role = r; }
  int getRole() const { return role; }
  QString getRoleStr() const;
  void setStatus(int s) { status = s; }
  void setStatus(const char *s);
  int getStatus() const { return status; }
  QString getStatusStr() const;
  void setRSVP(bool r) { rsvp = r; }
  void setRSVP(const char *r);
  bool RSVP() const { return rsvp; }

private:
  bool rsvp;
  int role, status;
  QString name, email;

};

/** This is a class which contains all the information necessary about a single
 * event, or an event that occurs multiple times through recurrence 
 * information. Methods provide access to the data, which is as fully
 * encapsulated as possible.
 *
 * @short a class which provides an abstract view of an event or appointment.
 * @author Preston Brown
 * @version $Revision$
 */
class KOEvent : public QObject {
  Q_OBJECT

  friend CalObject;
public:
  /** number of events created */
  static int eventCount;

  /** enumeration for describing how an event recurs, if at all. */
  enum { rNone = 0, rDaily = 0x0001, rWeekly = 0x0002, rMonthlyPos = 0x0003,
	 rMonthlyDay = 0x0004, rYearlyMonth = 0x0005, rYearlyDay = 0x0006 };
  /** enumeration for describing an event's status. */
  enum { NEEDS_ACTION = 0, ACCEPTED = 1, SENT = 2, TENTATIVE = 3,
	 CONFIRMED = 4, DECLINED = 5, COMPLETED = 6, DELEGATED = 7 };
  /** enumeration for describing an event's secrecy. */
  enum { PUBLIC = 0, PRIVATE = 1, CONFIDENTIAL = 2 };
  /** enumeration for printing style */
  enum { ASCII, POSTSCRIPT };
  /** structure for RecursMonthlyPos */
  struct rMonthPos {
    bool negative;
    short rPos;
    QBitArray rDays;
  };

  /** constructs a new event with variables initialized to "sane" values. */
  KOEvent();
  ~KOEvent();

  /** Recreate event. The event is made a new unique event, but already stored
  event information is preserved. Sets uniquie id, creation date, last
  modification date and revision number. */
  void recreate();

  /** sets the event to be read only or not */
  void setReadOnly(bool readonly) { ro = readonly; };
  /** returns the event's read only status */
  bool isReadOnly() { return ro; };

  /** sets the organizer for the event */
  void setOrganizer(const QString &o);
  const QString &getOrganizer() const;

  /** attendee stuff */
  void addAttendee(Attendee *a);
  void removeAttendee(Attendee *a);
  void removeAttendee(const char *n);
  void clearAttendees();
  Attendee *getAttendee(const char *n) const;
  const QList<Attendee> &getAttendeeList() const { return attendeeList; };
  int attendeeCount() const { return attendeeList.count(); };

  /** for setting the event's starting date/time with a QDateTime. */
  void setDtStart(const QDateTime &dtStart);
  /** for setting the event's starting date/time with a symbolic string.
   * the string should be in the format YYYYMMDDTHHMMSSZ
   * where: Y = year, M = months, D = day, T is a placeholder,
   * H = hour, M = minutes, S = seconds, and Z is another placeholder.
   */
  void setDtStart(const QString &dtStartStr);
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
  /** for setting the event's due date/time with a symbolic string.
   * the string should be in the format YYYYMMDDTHHMMSSZ
   * where: Y = year, M = months, D = day, T is a placeholder,
   * H = hour, M = minutes, S = seconds, and Z is another placeholder.
   */
  void setDtDue(const QString &dtDueStr);
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
  /** for setting an event's endating date/time with a string.
   * @see setDtStart
   */
  void setDtEnd(const QString &dtEndStr);
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

  /** sets the event's lengthy description. */
  void setDescription(const QString &description);
  /** sets the event's lengthy description. */
  void setDescription(const char *);
  /** returns a reference to the event's description. */
  const QString &getDescription() const;

  /** sets the event's short summary. */
  void setSummary(const QString &summary);
  /** sets the event's short summary. */
  void setSummary(const char *);
  /** returns a reference to the event's summary. */
  const QString &getSummary() const;

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

  /** set event's applicable categories */
  void setCategories(const QStrList &categories);
  /** set event's categories based on a comma delimited string */
  void setCategories(const QString &catStr);
  /** return categories in a list */
  const QStrList &getCategories() const;
  /** return categories as a comma separated string */
  QString getCategoriesStr();
  /** get color associated with first category */
  const QColor &getColor();

  /** set the list of attachments/associated files for this event */
  void setAttachments(const QStrList &attachments);
  /** return list of associated files */
  const QStrList &getAttachments() const;

  /** set resources used, such as Office, Car, etc. */
  void setResources(const QStrList &resources);
  /** return list of current resources */
  const QStrList &getResources() const;

  /** set the event to have this file as the noise for the alarm. */
  void setAudioAlarmFile(const QString &audioAlarmFile);
  void setAudioAlarmFile(const char *);
  /** return the name of the audio file for the alarm */
  const QString &getAudioAlarmFile() const;

  /** set this program to run when an alarm is triggered */
  void setProgramAlarmFile(const QString &programAlarmFile);
  void setProgramAlarmFile(const char *);
  /** return the name of the program to run when an alarm is triggered */
  const QString &getProgramAlarmFile() const;

  /** send mail to this address when an alarm goes off */
  void setMailAlarmAddress(const QString &mailAlarmAddress);
  void setMailAlarmAddress(const char *);
  /** return the address to send mail to when an alarm goes off */
  const QString &getMailAlarmAddress() const;

  /** set the text to display when an alarm goes off */
  void setAlarmText(const QString &alarmText);
  void setAlarmText(const char *);
  /** return the text string that displays when an alarm goes off */
  const QString &getAlarmText() const;

  /** set the time to trigger an alarm */
  void setAlarmTime(const QDateTime &alarmTime);
  void setAlarmTime(const QString &alarmTimeStr);
  /** return the date/time when an alarm goes off */
  const QDateTime &getAlarmTime() const;

  /** set the interval between snoozes for the alarm */
  void setAlarmSnoozeTime(int alarmSnoozeTime);
  /** get how long the alarm snooze interval is */
  int getAlarmSnoozeTime() const;
  /** set how many times an alarm is to repeat itself (w/snoozes) */
  void setAlarmRepeatCount(int alarmRepeatCount);
  /** get how many times an alarm repeats */
  int getAlarmRepeatCount() const;

  /** toggles the value of alarm to be either on or off.
      set's the alarm time to be x minutes before dtStart time. */
  void toggleAlarm();
    
  /** set the event's priority, 0 is undefined, 1 highest (decreasing order) */
  void setPriority(int priority);
  /** get the event's priority */
  int getPriority() const;
  /** set the event's time transparency level. */
  void setTransparency(int transparency);
  /** get the event's time transparency level. */
  int getTransparency() const;

  /** point at some other event to which the event relates. This function should
   *  only be used when constructing a calendar before the related KOEvent
   *  exists. */
  void setRelatedToVUID(const char *);
  /** what event does this one relate to? This function should
   *  only be used when constructing a calendar before the related KOEvent
   *  exists. */
  const QString &getRelatedToVUID() const;
  /** point at some other event to which the event relates */
  void setRelatedTo(KOEvent *relatedTo);
  /** what event does this one relate to? */
  KOEvent *getRelatedTo() const;
  /** All events that are related to this event */
  const QList<KOEvent> &getRelations() const;
  /** Add an event which is related to this event */
  void addRelation(KOEvent *);
  /** Remove event that is related to this event */
  void removeRelation(KOEvent *);

  /** set the internal identifier for the event */
  void setEventId(int id);
  /** return the internal identifier for the event */
  int getEventId() const;
  
  /** set the unique text string for the event */
  void setVUID(const char *);
  /** get the unique text string for the event */
  const QString &getVUID() const;

  /** set the number of revisions this event has seen */
  void setRevisionNum(int rev);
  /** return the number of revisions this event has seen */
  int getRevisionNum() const;

  /** set the time the event was last modified */
  void setLastModified(const QDateTime &lm);
  /** return the time the event was last modified */
  const QDateTime &getLastModified() const;

  /** returns the event's recurrence status.  See the enumeration at the top
   * of this file for possible values. */
  ushort doesRecur() const;
  /** returns TRUE if the date specified is one on which the event will
   * recur. */
  bool recursOn(const QDate &qd) const;
  /** turn off recurrence for this event. */
  void unsetRecurs();
  /** set an event to recur daily.
   * @var _rFreq the frequency to recur, i.e. 2 is every other day
   * @var _rDuration the duration for which to recur, i.e. 10 times
   */
  void setRecursDaily(int _rFreq, int _rDuration);
  /** set an event to recur daily.
   * @var _rFreq the frequency to recur, i.e. 2 is every other day
   * @var _rEndDate the ending date for which to stop recurring
   */
  void setRecursDaily(int _rFreq, const QDate &_rEndDate);
  /** set an event to recur weekly.
   * @var _rFreq the frequency to recur, i.e every other week etc.
   * @var _rDays a 7 bit array indicating which days on which to recur.
   * @var _rDuration the duration for which to recur
   */
  int getRecursFrequency() const;
  int getRecursDuration() const;
  /**
   * return the date on which recurrences end.  Only set currently
   * if a duration is NOT set.  We should compute it from the duration
   * if the duration, and not a specific end date is set, but this is
   * functionality is not complete at the moment. 
   */
  const QDate &getRecursEndDate() const;
  /** Returns a string representing the recurrence end date in the format
   according to the users lcoale settings. */
  QString getRecursEndDateStr(bool shortfmt=true) const;
  const QBitArray &getRecursDays() const;
  struct rMonthPos;
  const QList<rMonthPos> &getRecursMonthPositions() const;
  const QList<int> &getRecursMonthDays() const;

  void setRecursWeekly(int _rFreq, const QBitArray &_rDays, int _rDuration);
  /** set an event to recur weekly.
   * @var _rFreq the frequency to recur, i.e every other week etc.
   * @var _rDays a 7 bit array indicating which days on which to recur.
   * @var _rEndDate the date on which to stop recurring.
   */
  void setRecursWeekly(int _rFreq, const QBitArray &_rDays, const QDate &_rEndDate);

  /** set an event to recur monthly.
   * @var type rMonthlyPos or rMonthlyDay
   * @var _rFreq the frequency to recur, i.e. every third month etc.
   * @var _rDuration the number of times to recur, i.e. 13
   */
  void setRecursMonthly(short type, int _rFreq, int _rDuration);
  /** same as above, but with ending date not number of recurrences */
  void setRecursMonthly(short type, int _rFreq, const QDate &_rEndDate);
  /** add a position the the recursMonthlyPos recurrence rule, if it is
   * set.
   * @var _rPos the position in the month for the recurrence, with valid
   * values being 1-5 (5 weeks max in a month).
   * @var _rDays the days for the position to recur on.
   * Example: _rPos = 2, and bits 1 and 3 are set in _rDays.
   * the rule is to repeat every 2nd week on Monday and Wednesday.
   */
  void addRecursMonthlyPos(short _rPos, const QBitArray &_rDays);

  /** add a position the the recursMonthlyDay list. */
  void addRecursMonthlyDay(short _rDay);

  void setRecursYearly(int type, int _rFreq, int _rDuration);
  void setRecursYearly(int type, int _rFreq, const QDate &_rEndDate);
  void addRecursYearlyNum(short _rNum);
  const QList<int> &getRecursYearNums() const;

  /** returns the list of dates which are exceptions to the recurrence rule */
  const QDateList &getExDates() const;
  /** sets the list of dates which are exceptions to the recurrence rule */
  void setExDates(const QDateList &_exDates);
  void setExDates(const char *dates);
  void addExDate(const QDate &date);

  /** returns true if there is an exception for this date in the recurrence
     rule set, or false otherwise. */
  bool isException(const QDate &qd) const;

  /** pilot syncronization routines */
  enum { SYNCNONE = 0, SYNCMOD = 1, SYNCDEL = 3 };
  void setPilotId(int id);
  int getPilotId() const;
  
  void setSyncStatus(int stat);
  int getSyncStatus() const;

  void setTodoStatus(bool stat) { isTodo = stat; priority = 1; emit eventUpdated(this); };
  bool getTodoStatus() const { return isTodo; };

  bool isMultiDay() const {  return !(dtStart.date() == dtEnd.date()); };

  void print(int) const;

signals:
  void eventUpdated(KOEvent *);
  
protected:
  bool recursDaily(const QDate &) const;
  bool recursWeekly(const QDate &) const;
  bool recursMonthlyByDay(const QDate &) const;
  bool recursMonthlyByPos(const QDate &) const;
  bool recursYearlyByMonth(const QDate &) const;
  bool recursYearlyByDay(const QDate &) const;

  QDateTime strToDateTime(const QString &dateStr);
  QDate strToDate(const QString &dateStr);
  int weekOfMonth(const QDate &qd) const;

  // data variables
  bool ro;                             // is this event able to be changed?

  QDateTime dateCreated;               // date that the event was first created
  int id;                              // globally unique ID for this event
  QString vUID;                        // vCalendar UID

  int revisionNum;                     // how many times it has been modified.
                                       // Note that as per
				       // the iCalendar spec, we only bump 
                                       // this when DTSTART,
				       // DTEND, RDATE, RRULE, EXDATE
                                       // EXRULE are changed.

  QDateTime lastModified;              // last time this entry was touched

  QString organizer;                   // who owns / organizes this event
  QList<Attendee> attendeeList;        // list of attendees for event

  QDateTime dtStart;                   // start time for event
  QDateTime dtEnd;                     // end time for event.  Events with
                                       // only end time or start time
                                       // take up "no space".

  bool floats;			       // floating means date without time				      

  QDateTime dtDue;                     // due date of todo
  
  bool mHasDueDate;                    // if todo has associated due date

  QString description;                 // a detailed description of the event
  QString summary;                     // summary of event
  int  status;                         // confirmed/delegated/tentative/etc
  int  secrecy;                        // public/private/confidential
  QStrList categories;                 // business/personal/vacation/etc
  QStrList attachments;                // attached files, sounds, anything!
  QStrList resources;                  // a list of resources needed for event

  QString audioAlarmFile;              // url/filename of sound to play
  QString programAlarmFile;            // filename of program to run
  QString mailAlarmAddress;            // who to mail for reminder
  QString alarmText;                   // text to display/mail for alarm

  QDateTime alarmTime;                 // time at which to display the alarm
  int alarmSnoozeTime;                 // number of minutes after alarm to
                                       // snooze before ringing again
  int alarmRepeatCount;                // number of times for alarm to repeat

  int priority;                        // 1 = highest, 2 = less, etc.
  int transparency;                    // how transparent the event is to
                                       // free time requests. 0 = always block,
                                       // 1 = never block.
  KOEvent *relatedTo;                  // related event;
  QString relatedToVUID;               // UID of related event;
  QList<KOEvent> relations;            // List of events that are related to
                                       // this event.

  // stuff below here is for recurring events
  // this is a SUBSET of vCalendar and should be expanded...
  short recurs;                        // should be one of the enums.

  QBitArray rDays;                     // array of days during week it recurs
  
  QList<rMonthPos> rMonthPositions;    // list of positions during a month
                                       // on which an event recurs
					    
  QList<int> rMonthDays;               // list of days during a month on
                                       // which the event recurs

  QList<int> rYearNums;                // either months/days to recur on
                                       // for rYearly

  int rFreq;                           // frequency of period

  // one of the following must be specified
  int rDuration;                       // num times to Recur, -1 = infin.
  QDate rEndDate;                      // date on which to end Recurring
  
  QDateList exDates;                   // exceptions to recurrence rules

  bool isTodo;                         // true if this is a "todo"

  // PILOT SYNCHRONIZATION STUFF
  int pilotId;                         // unique id for pilot sync
  int syncStatus;                      // status (for sync)

  bool newRevision;                    // true if revisionNum should be incremented.
};

#endif

