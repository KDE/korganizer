// $Id$	

#include <stdlib.h>
#include <stdio.h>

#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstddirs.h>

#include "koprefs.h"
#include "calformat.h"

#include "koevent.h"

int KOEvent::eventCount = 0;

KOEvent::KOEvent() 
{
  // initialize an event object

  KOEvent::eventCount++;
  id = KOEvent::eventCount;

  isTodo = false;

  status = TENTATIVE;

  mHasDueDate = false;
  mHasStartDate = false;
}

KOEvent::~KOEvent()
{
  KOEvent::eventCount--;
}


void KOEvent::setEventId(int id)
{
  KOEvent::id = id;
  emit eventUpdated(this);
}

int KOEvent::getEventId() const
{
  return id;
}


void KOEvent::setDtDue(const QDateTime &dtDue)
{  
  int diffsecs = KOEvent::dtDue.secsTo(dtDue);

  if (mReadOnly) return;
  if (alarmRepeatCount())
    setAlarmTime(alarmTime().addSecs(diffsecs));

  KOEvent::dtDue = dtDue;
  emit eventUpdated(this);
}

const QDateTime &KOEvent::getDtDue() const
{
  return dtDue;
}

QString KOEvent::getDtDueTimeStr() const
{
  return KGlobal::locale()->formatTime(dtDue.time());
}

QString KOEvent::getDtDueDateStr(bool shortfmt) const
{
  return KGlobal::locale()->formatDate(dtDue.date(),shortfmt);
}

QString KOEvent::getDtDueStr() const
{
  return KGlobal::locale()->formatDateTime(dtDue);
}




bool KOEvent::hasDueDate() const
{
  return mHasDueDate;
}

void KOEvent::setHasDueDate(bool f)
{
  if (mReadOnly) return;
  mHasDueDate = f;
  emit eventUpdated(this);
}


bool KOEvent::hasStartDate() const
{
  return mHasStartDate;
}

void KOEvent::setHasStartDate(bool f)
{
  if (mReadOnly) return;
  mHasStartDate = f;
  emit eventUpdated(this);
}


void KOEvent::setStatus(const QString &statStr)
{
  if (mReadOnly) return;
  QString ss(statStr.upper());

  if (ss == "X-ACTION")
    status = NEEDS_ACTION;
  else if (ss == "NEEDS ACTION")
    status = NEEDS_ACTION;
  else if (ss == "ACCEPTED")
    status = ACCEPTED;
  else if (ss == "SENT")
    status = SENT;
  else if (ss == "TENTATIVE")
    status = TENTATIVE;
  else if (ss == "CONFIRMED")
    status = CONFIRMED;
  else if (ss == "DECLINED")
    status = DECLINED;
  else if (ss == "COMPLETED")
    status = COMPLETED;
  else if (ss == "DELEGATED")
    status = DELEGATED;
  else
    kdDebug() << "error setting status, unknown status!" << endl;

  emit eventUpdated(this);
}

void KOEvent::setStatus(int status)
{
  if (mReadOnly) return;
  KOEvent::status = status;
  emit eventUpdated(this);
}

int KOEvent::getStatus() const
{
  return status;
}

QString KOEvent::getStatusStr() const
{
  switch(status) {
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
