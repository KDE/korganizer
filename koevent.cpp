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
#include "koevent.moc"

int KOEvent::eventCount = 0;

KOEvent::KOEvent() 
{
  // initialize an event object

  KOEvent::eventCount++;
  id = KOEvent::eventCount;

  isTodo = false;

  status = TENTATIVE;
  secrecy = PRIVATE;
//  categories = 0;
//  attachments = 0;
//  resources = 0;

  floats = TRUE; // whether or not the event has a time attached.
  mHasDueDate = false;
  mHasStartDate = false;

  priority = 1;
  transparency = 0;
  
  pilotId = 0;
  syncStatus = 1;
}

KOEvent::~KOEvent()
{
  KOEvent::eventCount--;
}

void KOEvent::setDtStart(const QDateTime &dtStart)
{  
  int diffsecs = mDtStart.secsTo(dtStart);

  if (mReadOnly) return;
  if (getAlarmRepeatCount())
    setAlarmTime(getAlarmTime().addSecs(diffsecs));

  mDtStart = dtStart;

  setRecurStart(mDtStart);
  setAlarmStart(mDtStart);

  emit eventUpdated(this);
}


const QDateTime &KOEvent::getDtStart() const
{
  return dtStart();
}

QString KOEvent::getDtStartTimeStr() const
{
  return KGlobal::locale()->formatTime(dtStart().time());
}

QString KOEvent::getDtStartDateStr(bool shortfmt) const
{
  return KGlobal::locale()->formatDate(dtStart().date(),shortfmt);
}

QString KOEvent::getDtStartStr() const
{
  return KGlobal::locale()->formatDateTime(dtStart());
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

/*
void KOEvent::setDtDue(const QString &dtDueStr)
{
  QDateTime tmpDt(strToDateTime(dtDueStr));
  int diffsecs = KOEvent::dtDue.secsTo(tmpDt);

  if (mReadOnly) return;
  if (alarmRepeatCount)
    alarmTime = alarmTime.addSecs(diffsecs);

  KOEvent::dtDue = tmpDt;
  emit eventUpdated(this);
}
*/

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


void KOEvent::setDtEnd(const QDateTime &dtEnd)
{  
  if (mReadOnly) return;
  KOEvent::dtEnd = dtEnd;
  emit eventUpdated(this);
}

/*
void KOEvent::setDtEnd(const QString &dtEndStr)
{
  if (mReadOnly) return;
  KOEvent::dtEnd = strToDateTime(dtEndStr);
  emit eventUpdated(this);
}
*/

const QDateTime &KOEvent::getDtEnd() const
{
  return dtEnd;
}

QString KOEvent::getDtEndTimeStr() const
{
  return KGlobal::locale()->formatTime(dtEnd.time());
}

QString KOEvent::getDtEndDateStr(bool shortfmt) const
{
  return KGlobal::locale()->formatDate(dtEnd.date(),shortfmt);
}

QString KOEvent::getDtEndStr() const
{
  return KGlobal::locale()->formatDateTime(dtEnd);
}


bool KOEvent::doesFloat() const
{
  return floats;
}

void KOEvent::setFloats(bool f)
{
  if (mReadOnly) return;
  floats = f;
  emit eventUpdated(this);
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

void KOEvent::setSecrecy(const QString &secStr)
{
  if (mReadOnly) return;
  if (secStr == "PUBLIC")
    secrecy = PUBLIC;
  else if (secStr == "PRIVATE")
    secrecy = PRIVATE;
  else if (secStr == "CONFIDENTIAL")
    secrecy = CONFIDENTIAL;
  else
    kdDebug() << "Unknown secrecy value specified!" << endl;

  emit eventUpdated(this);
}

void KOEvent::setSecrecy(const char *secStr)
{
  if (mReadOnly) return;
  QString sec = secStr;
  setSecrecy(sec.toInt());
}

void KOEvent::setSecrecy(int sec)
{
  if (mReadOnly) return;
  secrecy = sec;
  emit eventUpdated(this);
}

int KOEvent::getSecrecy() const
{
  return secrecy;
}

QString KOEvent::getSecrecyStr() const
{
  switch (secrecy) {
  case PUBLIC:
    return QString("PUBLIC");
    break;
  case PRIVATE:
    return QString("PRIVATE");
    break;
  case CONFIDENTIAL:
    return QString("CONFIDENTIAL");
    break;
  }
  // should never reach here...
  return QString("");
}

void KOEvent::setAttachments(const QStringList &attachments)
{
  if (mReadOnly) return;
  KOEvent::attachments = attachments;
  emit eventUpdated(this);
}

const QStringList &KOEvent::getAttachments() const
{
  return attachments;
}

void KOEvent::setResources(const QStringList &resources)
{
  if (mReadOnly) return;
  KOEvent::resources = resources;
  emit eventUpdated(this);
}

const QStringList &KOEvent::getResources() const
{
  return resources;
}

void KOEvent::setPriority(int priority)
{
  if (mReadOnly) return;
  KOEvent::priority = priority;
  emit eventUpdated(this);
}

int KOEvent::getPriority() const
{
  return priority;
}

void KOEvent::setTransparency(int transparency)
{
  if (mReadOnly) return;
  KOEvent::transparency = transparency;
  emit eventUpdated(this);
}

int KOEvent::getTransparency() const
{
  return transparency;
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

bool KOEvent::isMultiDay() const
{
  bool multi = !(dtStart().date() == dtEnd.date());
  return multi;
}

void KOEvent::setPilotId(int id)
{
  if (mReadOnly) return;
  pilotId = id;
  //emit eventUpdated(this);
}

int KOEvent::getPilotId() const
{
  return pilotId;
}

void KOEvent::setSyncStatus(int stat)
{
  if (mReadOnly) return;
  syncStatus = stat;
  //  emit eventUpdated(this);
}

int KOEvent::getSyncStatus() const
{
  return syncStatus;
}
