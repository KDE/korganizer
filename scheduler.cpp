// $Id$

#include <klocale.h>

#include "event.h"
#include "icalformat.h"
#include "calobject.h"

#include "scheduler.h"

ScheduleMessage::ScheduleMessage(Incidence *event,int method,icalclass status)
{
  mEvent = event;
  mMethod = method;
  mStatus = status;
}


Scheduler::Scheduler(CalObject *calendar)
{
  mCalendar = calendar;
  mFormat = mCalendar->iCalFormat();
}

Scheduler::~Scheduler()
{
}

bool Scheduler::acceptTransaction(Incidence *incidence,icalclass status)
{
  switch (status) {
    case ICAL_PUBLISH_NEW_CLASS:
      if (!mCalendar->getEvent(incidence->VUID())) {
        mCalendar->addIncidence(incidence);
      }
      return true;
    case ICAL_OBSOLETE_CLASS:
      return true;
    case ICAL_REQUEST_NEW_CLASS:
      mCalendar->addIncidence(incidence);
      return true;
    default:
      return false;
  }
}

QString Scheduler::methodName(Method method)
{
  switch (method) {
    case Publish:
      return i18n("Publish");
    case Request:
      return i18n("Request");
    case Refresh:
      return i18n("Refresh");
    case Cancel:
      return i18n("Cancel");
    case Add:
      return i18n("Add");
    case Reply:
      return i18n("Reply");
    case Counter:
      return i18n("Counter");
    case Declinecounter:
      return i18n("Decline Counter");
    default:
      return i18n("Unknown");
  }
}

QString Scheduler::statusName(icalclass status)
{
  switch (status) {
    case ICAL_PUBLISH_NEW_CLASS:
      return i18n("Publish");
    case ICAL_OBSOLETE_CLASS:
      return i18n("Obsolete");
    case ICAL_REQUEST_NEW_CLASS:
      return i18n("New Request");
    case ICAL_REQUEST_UPDATE_CLASS:
      return i18n("Updated Request");
    default:
      return i18n("Unknown Status: %1").arg(QString::number(status));
  }
}
