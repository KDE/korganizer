#ifndef MAILSCHEDULER_H
#define MAILSCHEDULER_H
// $Id$
//
// Mail implementation of iTIP methods
//

#include <qlist.h>

#include <imipscheduler.h>

namespace KCal {

/*
  This class implements the iTIP interface using the email interface specified
  as Mail.
*/
class MailScheduler : public IMIPScheduler {
  public:
    MailScheduler(Calendar *);
    virtual ~MailScheduler();
    
    bool publish (Event *incidence,const QString &recipients);
    bool performTransaction(Event *incidence,Method method);
    QList<ScheduleMessage> retrieveTransactions();
};

}

#endif
