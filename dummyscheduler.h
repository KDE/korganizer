#ifndef DUMMYSCHEDULER_H
#define DUMMYSCHEDULER_H
// $Id$
//
// Dummy implementation of iTIP methods
//

#include "scheduler.h"

/**
  This class implements the iTIP interface as a primitive local version for
  testing. It uses a file dummyscheduler.store as inbox/outbox.
*/
class DummyScheduler : public Scheduler {
  public:
    DummyScheduler(CalObject *);
    virtual ~DummyScheduler();
    
    bool publish (KOEvent *incidence,const QString &recipients);
    bool performTransaction(KOEvent *incidence,Method method);
    QList<ScheduleMessage> retrieveTransactions();

  protected:
    bool saveMessage(const QString &);
};

#endif  // DUMMYSCHEDULER_H

