// $Id$

#ifndef _KORECURRENCE_H
#define _KORECURRENCE_H

#include <qstring.h>
#include <qbitarray.h>
#include <qlist.h>

#include "qdatelist.h"

class Incidence;

class KORecurrence {
  public:

    /** enumeration for describing how an event recurs, if at all. */
    enum { rNone = 0, rDaily = 0x0001, rWeekly = 0x0002, rMonthlyPos = 0x0003,
	   rMonthlyDay = 0x0004, rYearlyMonth = 0x0005, rYearlyDay = 0x0006 };

    /** structure for RecursMonthlyPos */
    struct rMonthPos {
      bool negative;
      short rPos;
      QBitArray rDays;
    };

    /** constructs a new event with variables initialized to "sane" values. */
    KORecurrence(Incidence *parent);
    ~KORecurrence();

    void setRecurStart(QDateTime start) { mRecurStart = start; }
    void setRecurReadOnly(bool readOnly ) { mRecurReadOnly = readOnly; }
    void setRecurExDatesCount(int count ) { mRecurExDatesCount = count; }

    /** returns the event's recurrence status.  See the enumeration at the top
     * of this file for possible values. */
    ushort doesRecur() const;
    /** returns TRUE if the date specified is one on which the event will
     * recur. */
    bool recursOnPure(const QDate &qd) const;
    /** turn off recurrence for this event. */
    void unsetRecurs();
    /** set an event to recur daily.
     * @var _rFreq the frequency to recur, i.e. 2 is every other day
     * @var _rDuration the duration for which to recur, i.e. 10 times
     */
    void setDaily(int _rFreq, int _rDuration);
    /** set an event to recur daily.
     * @var _rFreq the frequency to recur, i.e. 2 is every other day
     * @var _rEndDate the ending date for which to stop recurring
     */
    void setDaily(int _rFreq, const QDate &_rEndDate);

    int frequency() const;
    int duration() const;
    /**
     * return the date on which recurrences end.  Only set currently
     * if a duration is NOT set.  We should compute it from the duration
     * if the duration, and not a specific end date is set, but this is
     * functionality is not complete at the moment.
     */
    const QDate &endDate() const;
    /** Returns a string representing the recurrence end date in the format
     according to the users lcoale settings. */
    QString endDateStr(bool shortfmt=true) const;
    const QBitArray &days() const;
    struct rMonthPos;
    const QList<rMonthPos> &monthPositions() const;
    const QList<int> &monthDays() const;
  
    /** set an event to recur weekly.
     * @var _rFreq the frequency to recur, i.e every other week etc.
     * @var _rDays a 7 bit array indicating which days on which to recur.
     * @var _rDuration the duration for which to recur
     */
    void setWeekly(int _rFreq, const QBitArray &_rDays, int _rDuration);
    /** set an event to recur weekly.
     * @var _rFreq the frequency to recur, i.e every other week etc.
     * @var _rDays a 7 bit array indicating which days on which to recur.
     * @var _rEndDate the date on which to stop recurring.
     */
    void setWeekly(int _rFreq, const QBitArray &_rDays, const QDate &_rEndDate);
  
    /** set an event to recur monthly.
     * @var type rMonthlyPos or rMonthlyDay
     * @var _rFreq the frequency to recur, i.e. every third month etc.
     * @var _rDuration the number of times to recur, i.e. 13
     */
    void setMonthly(short type, int _rFreq, int _rDuration);
    /** same as above, but with ending date not number of recurrences */
    void setMonthly(short type, int _rFreq, const QDate &_rEndDate);
    /** add a position the the recursMonthlyPos recurrence rule, if it is
     * set.
     * @var _rPos the position in the month for the recurrence, with valid
     * values being 1-5 (5 weeks max in a month).
     * @var _rDays the days for the position to recur on.
     * Example: _rPos = 2, and bits 1 and 3 are set in _rDays.
     * the rule is to repeat every 2nd week on Monday and Wednesday.
     */
    void addMonthlyPos(short _rPos, const QBitArray &_rDays);
  
    /** add a position the the recursMonthlyDay list. */
    void addMonthlyDay(short _rDay);
  
    void setYearly(int type, int _rFreq, int _rDuration);
    void setYearly(int type, int _rFreq, const QDate &_rEndDate);
    void addYearlyNum(short _rNum);
    const QList<int> &yearNums() const;
  
  protected:
    bool recursDaily(const QDate &) const;
    bool recursWeekly(const QDate &) const;
    bool recursMonthlyByDay(const QDate &) const;
    bool recursMonthlyByPos(const QDate &) const;
    bool recursYearlyByMonth(const QDate &) const;
    bool recursYearlyByDay(const QDate &) const;
  
    int weekOfMonth(const QDate &qd) const;
  
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
  
  private:
    QDateTime mRecurStart;
    bool mRecurReadOnly;
    int mRecurExDatesCount;
  
    Incidence *mParent;
};
  
#endif
