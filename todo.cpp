// $Id$

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "todo.h"

Todo::Todo()
{
//  mStatus = TENTATIVE;

  mHasDueDate = false;
  mHasStartDate = false;
  
  mIsCompleted = false;
  mHasCompletedDate = false;
}

Todo::~Todo()
{
}

void Todo::setDtDue(const QDateTime &dtDue)
{  
  int diffsecs = mDtDue.secsTo(dtDue);

  if (mReadOnly) return;
  if (alarm()->alarmRepeatCount())
    alarm()->setAlarmTime(alarm()->alarmTime().addSecs(diffsecs));

  mDtDue = dtDue;
  emit eventUpdated(this);
}

const QDateTime &Todo::dtDue() const
{
  return mDtDue;
}

QString Todo::dtDueTimeStr() const
{
  return KGlobal::locale()->formatTime(mDtDue.time());
}

QString Todo::dtDueDateStr(bool shortfmt) const
{
  return KGlobal::locale()->formatDate(mDtDue.date(),shortfmt);
}

QString Todo::dtDueStr() const
{
  return KGlobal::locale()->formatDateTime(mDtDue);
}




bool Todo::hasDueDate() const
{
  return mHasDueDate;
}

void Todo::setHasDueDate(bool f)
{
  if (mReadOnly) return;
  mHasDueDate = f;
  emit eventUpdated(this);
}


bool Todo::hasStartDate() const
{
  return mHasStartDate;
}

void Todo::setHasStartDate(bool f)
{
  if (mReadOnly) return;
  mHasStartDate = f;
  emit eventUpdated(this);
}

#if 0
void Todo::setStatus(const QString &statStr)
{
  if (mReadOnly) return;
  QString ss(statStr.upper());

  if (ss == "X-ACTION")
    mStatus = NEEDS_ACTION;
  else if (ss == "NEEDS ACTION")
    mStatus = NEEDS_ACTION;
  else if (ss == "ACCEPTED")
    mStatus = ACCEPTED;
  else if (ss == "SENT")
    mStatus = SENT;
  else if (ss == "TENTATIVE")
    mStatus = TENTATIVE;
  else if (ss == "CONFIRMED")
    mStatus = CONFIRMED;
  else if (ss == "DECLINED")
    mStatus = DECLINED;
  else if (ss == "COMPLETED")
    mStatus = COMPLETED;
  else if (ss == "DELEGATED")
    mStatus = DELEGATED;
  else
    kdDebug() << "error setting status, unknown status!" << endl;

  emit eventUpdated(this);
}

void Todo::setStatus(int status)
{
  if (mReadOnly) return;
  mStatus = status;
  emit eventUpdated(this);
}

int Todo::status() const
{
  return mStatus;
}

QString Todo::statusStr() const
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
#endif

bool Todo::isCompleted() const
{
  return mIsCompleted;
}

void Todo::setCompleted(bool completed)
{
  mIsCompleted = completed;
}

QDateTime Todo::completed() const
{
  return mCompleted;
}

QString Todo::completedStr() const
{
  return KGlobal::locale()->formatDateTime(mCompleted);
}

void Todo::setCompleted(const QDateTime &completed)
{
  mCompleted = completed;
  setCompleted(true);
  mHasCompletedDate = true;
}

bool Todo::hasCompletedDate() const
{
  return mHasCompletedDate;
}
