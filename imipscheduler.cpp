// $Id$
//
// IMIPScheduler - iMIP implementation of iTIP methods
//

#include "koevent.h"
#include "icalformat.h"

#include "imipscheduler.h"

IMIPScheduler::IMIPScheduler(CalObject *calendar)
  : Scheduler(calendar)
{
}

IMIPScheduler::~IMIPScheduler()
{
}

bool IMIPScheduler::publish (KOEvent *incidence,const QString &recipients)
{
  return false;
}

bool IMIPScheduler::performTransaction(KOEvent *incidence,Method method)
{
  mFormat->createScheduleMessage(incidence,method);

  return false;
}

QList<ScheduleMessage> IMIPScheduler::retrieveTransactions()
{
  QList<ScheduleMessage> messageList;

  return messageList;
}
