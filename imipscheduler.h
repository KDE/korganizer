#ifndef IMIPSCHEDULER_H
#define IMIPSCHEDULER_H
// $Id$
//
// iMIP implementation of iTIP methods
//

#include <qlist.h>

#include "scheduler.h"

/*
  This class implements the iTIP interface using the email interface specified
  as iMIP.
*/
class IMIPScheduler : public Scheduler {
  public:
    IMIPScheduler(CalObject *);
    virtual ~IMIPScheduler();
    
    bool publish (KOEvent *incidence,const QString &recipients);
    bool performTransaction(KOEvent *incidence,Method method);
    QList<ScheduleMessage> retrieveTransactions();
};

#endif  // IMIPSCHEDULER_H

