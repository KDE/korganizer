#ifndef EVENT_H
#define EVENT_H
// $Id$
//
// Event component, representing a VEVENT object
//

#include "incidence.h"

class Event : public Incidence
{
  public:
    Event();
    ~Event();
    
    bool accept(IncidenceVisitor &v) { return v.visit(this); }

    /** for setting an event's ending date/time with a QDateTime. */
    void setDtEnd(const QDateTime &dtEnd);
    /** returns an event's ending date/time as a QDateTime. */
    const QDateTime &dtEnd() const;
    const QDateTime &getDtEnd() const { return dtEnd(); }
    /** returns an event's end time as a string formatted according to the
     users locale settings */
    QString dtEndTimeStr() const;
    QString getDtEndTimeStr() const { return dtEndTimeStr(); }
    /** returns an event's end date as a string formatted according to the
     users locale settings */
    QString dtEndDateStr(bool shortfmt=true) const;
    QString getDtEndDateStr(bool shortfmt=true) const { return dtEndDateStr(shortfmt); }
    /** returns an event's end date and time as a string formatted according
     to the users locale settings */
    QString dtEndStr() const;
    QString getDtEndStr() const { return dtEndStr(); }

    bool isMultiDay() const;

    /** set the event's time transparency level. */
    void setTransparency(int transparency);
    /** get the event's time transparency level. */
    int transparency() const;
    int getTransparency() const { return transparency(); }

  private:
    QDateTime mDtEnd;
    int mTransparency;
};

#endif
