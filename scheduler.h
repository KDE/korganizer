// $Id$
#ifndef SCHEDULER_H
#define SCHEDULER_H

// iTIP transactions base class

#include <qstring.h>
#include <qlist.h>

extern "C" {
  #include <ical.h>
  #include <icalss.h>
/*
  #include "icalrestriction.h"
  #include "icalclassify.h"
*/
}

class KOEvent;
class CalObject;
class ICalFormat;

/**
  This class provides an encapsulation of a scheduling message. It asscisates an
  event with a method and status information. This class is used by the
  Scheduler class.

  @short A Scheduling message
  @author Cornelius Schumacher
  @version $Revision$
*/
class ScheduleMessage {
  public:
    /**
      Create a scheduling message with method as defined in Scheduler::Method
      and a status.
    */
    ScheduleMessage(KOEvent *,int method,icalclass status);
    ~ScheduleMessage() {};
    
    /** Return event associated with this message. */
    KOEvent *event() { return mEvent; }
    /** Return iTIP method associated with this message */
    int method() { return mMethod; }
    /** Return status of this message */
    icalclass status() { return mStatus; }
    /** Return error message if there is any */
    QString error() { return mError; }

  private:
    KOEvent *mEvent;
    int mMethod;
    icalclass mStatus;
    QString mError;
};

/**
  This class provides an encapsulation of iTIP transactions. It is an abstract
  base class for inheritance by implementations of the iTIP scheme like iMIP or
  iRIP.
  
  @short iTIP interface
  @author Cornelius Schumacher
  @version $Revision$
*/
class Scheduler {
  public:
    enum Method { Publish,Request,Refresh,Cancel,Add,Reply,Counter,
                  Declinecounter,NoMethod };
  
    /** Create scheduler for calendar specified as argument */
    Scheduler(CalObject *calendar);
    virtual ~Scheduler();
    
    /** iTIP publish action */
    virtual bool publish (KOEvent *incidence,const QString &recipients) = 0;
    /** Perform iTIP transaction on incidence. The method is specified as the
    method argumanet and can be any valid iTIP method. */
    virtual bool performTransaction(KOEvent *incidence,Method method) = 0;
    /** Retrieve incoming iTIP transactions */
    virtual QList<ScheduleMessage> retrieveTransactions() = 0;

    /**
      Accept transaction. The incidence argument specifies the iCal compoennt
      on which the transaction acts. The status is the result of processing a
      iTIP message with the current calendar and specifies the action to be
      taken for this incidence.
    */
    bool acceptTransaction(KOEvent *,icalclass status);

    /** Return a human-readable name for a iTIP method. */
    static QString methodName(Method);
    /** Return a human-readable name for an ical message status. */
    static QString statusName(icalclass status);

  protected:
    CalObject *mCalendar;
    ICalFormat *mFormat;
};


#endif  // SCHEDULER_H
