// $Id$
//
// MailScheduler - Mail implementation of iTIP methods
//

#include "event.h"
#include "icalformat.h"

#include "komailclient.h"

#include "mailscheduler.h"

using namespace KCal;

MailScheduler::MailScheduler(Calendar *calendar)
  : IMIPScheduler(calendar)
{
}

MailScheduler::~MailScheduler()
{
}

bool MailScheduler::publish (Event *incidence,const QString &recipients)
{
  QString messageText = mFormat->createScheduleMessage(incidence,
                                                       Scheduler::Publish);

//  KOMailClient mailer;

  return false;  
}

bool MailScheduler::performTransaction(Event *incidence,Method method)
{
  QString messageText = mFormat->createScheduleMessage(incidence,method);

  KOMailClient mailer;
  return mailer.mailAttendees(incidence,messageText);
}

QList<ScheduleMessage> MailScheduler::retrieveTransactions()
{
  QList<ScheduleMessage> messageList;

  return messageList;
}
