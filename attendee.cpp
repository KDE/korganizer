// 	$Id$	

#include <kdebug.h>

#include "attendee.h"

/******************************* ATTENDEE CLASS *****************************/
// most methods have been inlined, see koevent.h for more information.
Attendee::Attendee(const QString & n, const QString & e, bool _rsvp, int s, int r)
{
  mFlag = TRUE;
  mRSVP = _rsvp;
  mName = n;
  mEmail = e;
  mStatus = s;
  mRole = r;
}

#if 0
Attendee::Attendee(const Attendee &a)
{
  flag = a.flag;
  rsvp = a.rsvp;
  name = a.name;
  email = a.email;
  status = a.status;
  role = a.role;
}
#endif

Attendee::~Attendee()
{
}

void Attendee::setStatus(const char *s)
{
  QString statStr = s;
  statStr = statStr.upper();

  if (statStr == "X-ACTION")
    mStatus = NEEDS_ACTION;
  else if (statStr == "NEEDS ACTION")
    mStatus = NEEDS_ACTION;
  else if (statStr== "ACCEPTED")
    mStatus = ACCEPTED;
  else if (statStr== "SENT")
    mStatus = SENT;
  else if (statStr== "TENTATIVE")
  
  mStatus = TENTATIVE;
  else if (statStr== "CONFIRMED")
    mStatus = CONFIRMED;
  else if (statStr== "DECLINED")
    mStatus = DECLINED;
  else if (statStr== "COMPLETED")
    mStatus = COMPLETED;
  else if (statStr== "DELEGATED")
    mStatus = DELEGATED;
  else {
    kdDebug() << "error setting attendee mStatus, unknown mStatus!" << endl;
    mStatus = NEEDS_ACTION;
  }
}

QString Attendee::statusStr() const
{
  switch(mStatus) {
  case NEEDS_ACTION:
    return QString("NEEDS ACTION");
    break;
  case ACCEPTED:
    return QString("ACCEPTED");
    break;
  case SENT:
    return QString("SENT");
    break;
  case TENTATIVE:
    return QString("TENTATIVE");
    break;
  case CONFIRMED:
    return QString("CONFIRMED");
    break;
  case DECLINED:
    return QString("DECLINED");
    break;
  case COMPLETED:
    return QString("COMPLETED");
    break;
  case DELEGATED:
    return QString("DELEGATED");
    break;
  }
  return QString("");
}

QString Attendee::roleStr() const
{
  switch(mRole) {
  case 0:
    return QString("Attendee");
    break;
  case 1:
    return QString("Organizer");
    break;
  case 2:
    return QString("Owner");
    break;
  case 3:
    return QString("Delegate");
    break;
  default:
    return QString("Attendee");
    break;
  }
  
}

void Attendee::setRSVP(const char *r)
{
  QString s;
  s = r;
  s = s.upper();
  if (s == "TRUE")
    mRSVP = TRUE;
  else
    mRSVP = FALSE;
}
