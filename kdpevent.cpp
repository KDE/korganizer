// 	$Id$	

#include <stdlib.h>
#include <stdio.h>

#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>

#include "kdpevent.h"
#include "kdpevent.moc"

int KDPEvent::eventCount = 0;

bool KDPEvent::weekStartsMonday = FALSE;

KDPEvent::KDPEvent() 
  : QObject()
{
  KConfig *config;
  // initialize an event object

  KDPEvent::eventCount++;
  id = KDPEvent::eventCount;

  dateCreated = QDateTime::currentDateTime();
  int hashTime = dateCreated.time().hour() + 
    dateCreated.time().minute() + dateCreated.time().second() +
    dateCreated.time().msec();
  vUID.sprintf("KOrganizer - %li.%d",random(),hashTime);

  revisionNum = 0;
  relatedTo = 0;
  lastModified = QDateTime::currentDateTime();
  
  // KConfig now handles the cfg dir
//  QString cfgPath = KApplication::localconfigdir();
//  cfgPath += "/korganizerrc";
  
//  config = new KConfig(cfgPath.data());
  config = new KConfig("korganizerrc");
  config->setGroup("Personal Settings");
  organizer = config->readEntry("user_email");
  if (organizer.isEmpty())
    organizer = "x-none";
  delete config;
  
  description = "";
  summary = "";
  status = TENTATIVE;
  secrecy = PRIVATE;
  categories = 0;
  attachments = 0;
  resources = 0;

  audioAlarmFile = "";
  programAlarmFile = "";
  mailAlarmAddress = "";
  alarmText = "";

  floats = TRUE; // whether or not the event has a time attached.

  alarmSnoozeTime = 5;
  alarmRepeatCount = 0; // alarm disabled
  
  priority = 0;
  transparency = 0;
  
  recurs = rNone; // by default, it's not a recurring event.
  //  rDays.resize(7); // can't initialize in the header
  rMonthDays.setAutoDelete(TRUE);
  rMonthPositions.setAutoDelete(TRUE);
  rYearNums.setAutoDelete(TRUE);
  exDates.setAutoDelete(TRUE);
  categories.setAutoDelete(TRUE);
  attachments.setAutoDelete(TRUE);
  attendeeList.setAutoDelete(TRUE);
  resources.setAutoDelete(TRUE);

  pilotId = 0;
  syncStatus = 1;

  ro = FALSE;
}

KDPEvent::~KDPEvent()
{
  KDPEvent::eventCount--;
}

inline void KDPEvent::setOrganizer(const char *o)
{
  // we don't check for readonly here, because it is
  // possible that by setting the organizer we are changing
  // the event's readonly status...
  organizer = o;
  if (organizer.left(7).upper() == "MAILTO:")
    organizer = organizer.remove(0,7);
  emit eventUpdated(this);  
}

inline void KDPEvent::setOrganizer(const QString &o)
{
  setOrganizer(o.data());
}

inline const QString &KDPEvent::getOrganizer() const
{
  return organizer;
}

inline void KDPEvent::addAttendee(Attendee *a)
{
  if (ro) return;
  if (a->name.left(7).upper() == "MAILTO:")
    a->name = a->name.remove(0,7);

  attendeeList.append(a);
  emit eventUpdated(this);
}

inline void KDPEvent::removeAttendee(Attendee *a)
{
  if (ro) return;
  attendeeList.removeRef(a);
  emit eventUpdated(this);
}

inline void KDPEvent::removeAttendee(const char *n)
{
  Attendee *a;

  if (ro) return;
  for (a = attendeeList.first(); a; a = attendeeList.next())
    if (a->getName() == n) {
      attendeeList.remove();
      break;
    }
}
    
inline void KDPEvent::clearAttendees()
{
  if (ro) return;
  attendeeList.clear();
}

Attendee *KDPEvent::getAttendee(const char *n) const
{
  QListIterator<Attendee> qli(attendeeList);

  qli.toFirst();
  while (qli) {
    if (qli.current()->getName() == n)
      return qli.current();
    ++qli;
  }
  return 0L;
}

inline void KDPEvent::setDtStart(const QDateTime &dtStart)
{  
  int diffsecs = KDPEvent::dtStart.secsTo(dtStart);

  if (ro) return;
  if (alarmRepeatCount)
    alarmTime = alarmTime.addSecs(diffsecs);

  KDPEvent::dtStart = dtStart;
  emit eventUpdated(this);
}

inline void KDPEvent::setDtStart(const QString &dtStartStr)
{
  QDateTime tmpDt(strToDateTime(dtStartStr));
  int diffsecs = KDPEvent::dtStart.secsTo(tmpDt);

  if (ro) return;
  if (alarmRepeatCount)
    alarmTime = alarmTime.addSecs(diffsecs);

  KDPEvent::dtStart = tmpDt;
  emit eventUpdated(this);
}

inline const QDateTime &KDPEvent::getDtStart() const
{
  return dtStart;
}

inline QString KDPEvent::getDtStartTimeStr() const
{
  QString timeStr;

  timeStr.sprintf("%02d:%02d",dtStart.time().hour(), 
		  dtStart.time().minute());
  return timeStr;
		  
}

inline QString KDPEvent::getDtStartDateStr() const
{
  QString dateStr;
 
  dateStr.sprintf("%.2d %3s %4d",dtStart.date().day(),
		  (const char*)dtStart.date().monthName(dtStart.date().month()),
		  dtStart.date().year());
  return dateStr;
}

inline void KDPEvent::setDtEnd(const QDateTime &dtEnd)
{  
  if (ro) return;
  KDPEvent::dtEnd = dtEnd;
  emit eventUpdated(this);
}

inline void KDPEvent::setDtEnd(const QString &dtEndStr)
{
  if (ro) return;
  KDPEvent::dtEnd = strToDateTime(dtEndStr);
  emit eventUpdated(this);
}

inline const QDateTime &KDPEvent::getDtEnd() const
{
  return dtEnd;
}

inline QString KDPEvent::getDtEndTimeStr() const
{
  QString timeStr;

  timeStr.sprintf("%02d:%02d",dtEnd.time().hour(), 
		  dtEnd.time().minute());
  return timeStr;
		  
}

inline QString KDPEvent::getDtEndDateStr() const
{
  QString dateStr;

  dateStr.sprintf("%.2d %3s %4d",dtEnd.date().day(),
		  (const char*)dtEnd.date().monthName(dtEnd.date().month()),
		  dtEnd.date().year());
  return dateStr;
}

inline bool KDPEvent::doesFloat() const
{
  return floats;
}

inline void KDPEvent::setFloats(bool f)
{
  if (ro) return;
  floats = f;
  emit eventUpdated(this);
}

inline void KDPEvent::setDescription(const QString &description)
{
  if (ro) return;
  KDPEvent::description = description;
  emit eventUpdated(this);
}

inline void KDPEvent::setDescription(const char *description)
{
  if (ro) return;
  KDPEvent::description = description;
  emit eventUpdated(this);
}

inline const QString &KDPEvent::getDescription() const
{
  return description;
}

inline void KDPEvent::setSummary(const QString &summary)
{
  if (ro) return;
  KDPEvent::summary = summary.data(); // so it gets detached
  emit eventUpdated(this);
}

inline void KDPEvent::setSummary(const char *summary)
{
  if (ro) return;
  KDPEvent::summary = summary;
  emit eventUpdated(this);
}

inline const QString &KDPEvent::getSummary() const
{
  return summary;
}

void KDPEvent::setStatus(const QString &statStr)
{
  if (ro) return;
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
    debug("error setting status, unknown status!");

  emit eventUpdated(this);
}

inline void KDPEvent::setStatus(int status)
{
  if (ro) return;
  KDPEvent::status = status;
  emit eventUpdated(this);
}

inline int KDPEvent::getStatus() const
{
  return status;
}

QString KDPEvent::getStatusStr() const
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

void KDPEvent::setSecrecy(const QString &secStr)
{
  if (ro) return;
  if (secStr == "PUBLIC")
    secrecy = PUBLIC;
  else if (secStr == "PRIVATE")
    secrecy = PRIVATE;
  else if (secStr == "CONFIDENTIAL")
    secrecy = CONFIDENTIAL;
  else
    debug("Unknown secrecy value specified!");

  emit eventUpdated(this);
}

void KDPEvent::setSecrecy(const char *secStr)
{
  if (ro) return;
  QString sec = secStr;
  setSecrecy(sec.toInt());
}

void KDPEvent::setSecrecy(int sec)
{
  if (ro) return;
  secrecy = sec;
  emit eventUpdated(this);
}

inline int KDPEvent::getSecrecy() const
{
  return secrecy;
}

QString KDPEvent::getSecrecyStr() const
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

inline void KDPEvent::setCategories(const QStrList &categories)
{
  if (ro) return;
  KDPEvent::categories = categories;
  emit eventUpdated(this);
}

void KDPEvent::setCategories(const QString &catStr)
{
  if (ro) return;
  QStrList tmpList;
  int index1 = 0;
  int index2 = 0;

  while ((index2 = catStr.find(',', index1)) != -1) {
    tmpList.append(catStr.mid(index1, index2-index1).data());
    index1 = index2 + 1;
  }
  // get last category
  tmpList.append(catStr.mid(index1, (catStr.length()-index1)));
  categories = tmpList;
  emit eventUpdated(this);
}

inline const QStrList &KDPEvent::getCategories() const
{
  return categories;
}

QString KDPEvent::getCategoriesStr()
{
  QString temp; 
  QString cat;
  bool first = TRUE;
  for (cat = categories.first(); cat; cat = categories.next()) {
      if (!first) {
	  temp += ",";
      } else {
	  first = FALSE;
      }
      temp += cat;
  }
  return temp;
}

inline void KDPEvent::setAttachments(const QStrList &attachments)
{
  if (ro) return;
  KDPEvent::attachments = attachments;
  emit eventUpdated(this);
}

inline const QStrList &KDPEvent::getAttachments() const
{
  return attachments;
}

inline void KDPEvent::setResources(const QStrList &resources)
{
  if (ro) return;
  KDPEvent::resources = resources;
  emit eventUpdated(this);
}

inline const QStrList &KDPEvent::getResources() const
{
  return resources;
}

inline void KDPEvent::setAudioAlarmFile(const QString &audioAlarmFile)
{
  if (ro) return;
  KDPEvent::audioAlarmFile = audioAlarmFile;
  emit eventUpdated(this);
}

inline void KDPEvent::setAudioAlarmFile(const char *audioAlarmFile)
{
  if (ro) return;
  KDPEvent::audioAlarmFile = audioAlarmFile;
  emit eventUpdated(this);
}

inline const QString &KDPEvent::getAudioAlarmFile() const
{
  return audioAlarmFile;
}

inline void KDPEvent::setProgramAlarmFile(const QString &programAlarmFile)
{
  if (ro) return;
  KDPEvent::programAlarmFile = programAlarmFile;
  emit eventUpdated(this);
}

inline void KDPEvent::setProgramAlarmFile(const char *programAlarmFile)
{
  if (ro) return;
  KDPEvent::programAlarmFile = programAlarmFile;
  emit eventUpdated(this);
}

inline const QString &KDPEvent::getProgramAlarmFile() const
{
  return programAlarmFile;
}

inline void KDPEvent::setMailAlarmAddress(const QString &mailAlarmAddress)
{
  if (ro) return;
  KDPEvent::mailAlarmAddress = mailAlarmAddress;
  emit eventUpdated(this);
}

inline void KDPEvent::setMailAlarmAddress(const char *mailAlarmAddress)
{
  if (ro) return;
  KDPEvent::mailAlarmAddress = mailAlarmAddress;
  emit eventUpdated(this);
}

inline const QString &KDPEvent::getMailAlarmAddress() const
{
  return mailAlarmAddress;
}

inline void KDPEvent::setAlarmText(const QString &alarmText)
{
  if (ro) return;
  KDPEvent::alarmText = alarmText.data(); // so it gets detached
  emit eventUpdated(this);
}

inline void KDPEvent::setAlarmText(const char *alarmText)
{
  if (ro) return;
  KDPEvent::alarmText = alarmText;
  emit eventUpdated(this);
}

inline const QString &KDPEvent::getAlarmText() const
{
  return alarmText;
}

inline void KDPEvent::setAlarmTime(const QDateTime &alarmTime)
{
  if (ro) return;
  KDPEvent::alarmTime = alarmTime;
  emit eventUpdated(this);
}

inline void KDPEvent::setAlarmTime(const QString &alarmTimeStr)
{
  if (ro) return;
  KDPEvent::alarmTime = strToDateTime(alarmTimeStr);
  emit eventUpdated(this);
}

inline const QDateTime &KDPEvent::getAlarmTime() const
{
  return alarmTime;
}

inline void KDPEvent::setAlarmSnoozeTime(int alarmSnoozeTime)
{
  if (ro) return;
  KDPEvent::alarmSnoozeTime = alarmSnoozeTime;
  emit eventUpdated(this);
}

inline int KDPEvent::getAlarmSnoozeTime() const
{
  return alarmSnoozeTime;
}

inline void KDPEvent::setAlarmRepeatCount(int alarmRepeatCount)
{
  if (ro) return;
  KDPEvent::alarmRepeatCount = alarmRepeatCount;
  emit eventUpdated(this);
}

inline int KDPEvent::getAlarmRepeatCount() const
{
  return alarmRepeatCount;
}

inline void KDPEvent::toggleAlarm()
{
  if (ro) return;
  if (alarmRepeatCount) {
    alarmRepeatCount = 0;
  } else {
    alarmRepeatCount = 1;
    KConfig *config(kapp->getConfig());
    config->setGroup("Time & Date");
    QString alarmStr(config->readEntry("Default Alarm Time", "15"));
    int pos = alarmStr.find(' ');
    if (pos >= 0)
      alarmStr.truncate(pos);
    alarmTime = dtStart.addSecs(-60 * alarmStr.toUInt());
  }
  emit eventUpdated(this);
}

inline void KDPEvent::setPriority(int priority)
{
  if (ro) return;
  KDPEvent::priority = priority;
  emit eventUpdated(this);
}

inline int KDPEvent::getPriority() const
{
  return priority;
}

inline void KDPEvent::setTransparency(int transparency)
{
  if (ro) return;
  KDPEvent::transparency = transparency;
  emit eventUpdated(this);
}

inline int KDPEvent::getTransparency() const
{
  return transparency;
}

inline void KDPEvent::setRelatedTo(int relatedTo)
{
  if (ro) return;
  KDPEvent::relatedTo = relatedTo;
  emit eventUpdated(this);
}

inline int KDPEvent::getRelatedTo() const
{
  return relatedTo;
}

inline void KDPEvent::setEventId(int id)
{
  KDPEvent::id = id;
  emit eventUpdated(this);
}

inline int KDPEvent::getEventId() const
{
  return id;
}

inline void KDPEvent::setVUID(const char *vUID)
{
  KDPEvent::vUID = vUID;
  emit eventUpdated(this);
}

inline const QString &KDPEvent::getVUID() const
{
  return vUID;
}

inline void KDPEvent::setRevisionNum(int rev)
{
  if (ro) return;
  revisionNum = rev;
  emit eventUpdated(this);
}

inline int KDPEvent::getRevisionNum() const
{
  return revisionNum;
}

inline void KDPEvent::setLastModified(const QDateTime &lm)
{
  // DON'T! emit eventUpdated because we call this from
  // CalObject::updateEvent().
  lastModified = lm;
}

inline const QDateTime &KDPEvent::getLastModified() const
{
  return lastModified;
}

inline ushort KDPEvent::doesRecur() const
{
  return recurs;
}

bool KDPEvent::recursOn(const QDate &qd) const
{
  // first off, check to see if the flag is even set
  if (recurs == rNone)
    return FALSE;

  // check if this date is on the exception list.
  if (recurs != rNone) {
    if (isException(qd))
      return FALSE;
  }

  // it recurs, let's check to see if this date is valid
  switch(recurs) {
  case rDaily:
    return recursDaily(qd);
    break;
  case rWeekly:
    return recursWeekly(qd);
    break;
  case rMonthlyPos:
    return recursMonthlyByPos(qd);
    break;
  case rMonthlyDay:
    return recursMonthlyByDay(qd);
    break;
  case rYearlyMonth:
    return recursYearlyByMonth(qd);
    break;
  case rYearlyDay:
    return recursYearlyByDay(qd);
    break;
  default:
    // catch-all.  Should never get here.
    debug("Control should never reach here in recursOn()!");
    return FALSE;
    break;
  } // case
}

bool KDPEvent::recursDaily(const QDate &qd) const
{
  QDate dStart = dtStart.date();
  int i;

  if ((qd >= dStart) && 
      (((qd <= dStart.addDays((rDuration-1+exDates.count())*rFreq)) && (rDuration > 0)) ||
       (rDuration == -1) ||
       ((rDuration == 0) && (qd <= rEndDate)))) {
    i = dStart.daysTo(qd);
    // here's the real check...
    if ((i % rFreq) == 0) {
      return TRUE;
    }
    else // frequency didn't match
      return FALSE;
  } 
  // the date queried fell outside the range of the event
  return FALSE;  
}

bool KDPEvent::recursWeekly(const QDate &qd) const
{
  QDate dStart = dtStart.date();
  int i;

  
  i = ((rDuration-1+exDates.count())*7) + (7 - dStart.dayOfWeek());
  if ((qd >= dStart) &&
      (((qd <= dStart.addDays(i*rFreq)) && (rDuration > 0)) ||
       (rDuration == -1) ||
       ((rDuration == 0) && (qd <= rEndDate)))) {
    // do frequency check.
    i = dStart.daysTo(qd)/7;
    if ((i % rFreq) == 0) {
      // check if the bits set match today.
      i = qd.dayOfWeek()-1;
      if (rDays.testBit((uint) i))
	return TRUE;
      else 
	return FALSE;
    } else // frequency didn't match
      return FALSE;
  }
  // the date queried fell outside the range of the event
  return FALSE;
}

bool KDPEvent::recursMonthlyByDay(const QDate &qd) const
{
  QDate dStart = dtStart.date();
  int monthsAhead = 0;
  int i = 0;
  QListIterator<int> qlid(rMonthDays);
  // calculate how many months ahead this date is from the original
  // event's date
  
  // calculate year's months first
  monthsAhead = (qd.year() - dStart.year()) * 12;
  
  // calculate month offset within the year.
  i = qd.month() - dStart.month(); // may be positive or negative
  
  monthsAhead += i; // add the month offset in
  
  // check to see if the date is in the proper range
  if ((qd >= dStart) &&
      (((monthsAhead <= (rDuration-1+(signed)exDates.count())*rFreq) && (rDuration > 0)) || 
       (rDuration == -1) ||
       ((rDuration == 0) && (qd <= rEndDate)))) {
    // do frequency check
    if ((monthsAhead % rFreq) == 0) {
      i = qd.day();
      for (; qlid.current(); ++qlid) {
	if (*qlid.current() < 0) {
	  if (i == (qd.daysInMonth()-*qlid.current()+1))
	    return TRUE;
	} else { 
	  if (i == *qlid.current())
	    return TRUE;
	}
      } // for loop
      // no dates matched, return false
      return FALSE;
    } else // frequency didn't match
      return FALSE;
  } 
  // outsize proper date range
  return FALSE;
}

bool KDPEvent::recursMonthlyByPos(const QDate &qd) const
{
  QDate dStart = dtStart.date();
  int monthsAhead = 0;
  int i = 0;
  QListIterator<rMonthPos> qlip(rMonthPositions);

  // calculate how many months ahead this date is from the original
  // event's date
  
  // calculate year's months first
  monthsAhead = (qd.year() - dStart.year()) * 12;
  
  // calculate month offset within the year.
  i = qd.month() - dStart.month(); // may be positive or negative
  
  monthsAhead += i; // add the month offset in
  
  // check to see if the date is in the proper range
  if ((qd >= dStart) &&
      (((monthsAhead <= (rDuration-1+(signed)exDates.count())*rFreq) && (rDuration > 0)) || 
       (rDuration == -1) ||
       ((rDuration == 0) && (qd <= rEndDate)))) {
    // do frequency check
    if ((monthsAhead % rFreq) == 0) {
      i = weekOfMonth(qd);
      // check to see if this day of the week isn't found in the first
      // week of the month.
      QDate first(qd.year(), qd.month(), 1);
      if (qd.dayOfWeek() < first.dayOfWeek())
	--i;

      // now loop through the list of modifiers, and check them
      // all against the day of the month
      for (; qlip.current(); ++qlip) {
	if (qlip.current()->negative) {
	  i = 5 - i; // convert to negative offset format
	}
	// check position offset
	if (i == qlip.current()->rPos) {
	  // check day(s)
	  if (qlip.current()->rDays.testBit((uint) qd.dayOfWeek()-1))
	    return TRUE;
	} // if position test
      } // for loop 
      // no dates matched as true, must be false.
      return FALSE;
    } else // frequency didn't match
      return FALSE;
  }
  // the date queried fell outside the range of the event
  return FALSE;
}

bool KDPEvent::recursYearlyByMonth(const QDate &qd) const 
{
  QDate dStart = dtStart.date();
  int yearsAhead = 0;
  int  i = 0;
  QListIterator<int> qlin(rYearNums);

  // calculate how many years ahead this date is from the original
  // event's date
  
  yearsAhead = (qd.year() - dStart.year());
  
  // check to see if the date is in the proper range
  if ((qd >= dStart) &&
      (((yearsAhead <= (rDuration-1+(signed)exDates.count())*rFreq) && (rDuration > 0)) || 
       (rDuration == -1) ||
       ((rDuration == 0) && (qd <= rEndDate)))) {
    // do frequency check
    if ((yearsAhead % rFreq) == 0) {
      i = qd.month();
      for (; qlin.current(); ++qlin) {
	if (i == *qlin.current())
	  if (qd.day() == dStart.day())
	    return TRUE;
      }
      // no dates matched, return false
      return FALSE;
    } else
      // frequency didn't match
      return FALSE;
  } // outside proper date range
  return FALSE;
}

bool KDPEvent::recursYearlyByDay(const QDate &qd) const
{
  QDate dStart = dtStart.date();
  int yearsAhead = 0;
  int i = 0;
  QListIterator<int> qlin(rYearNums);

  // calculate how many years ahead this date is from the original
  // event's date
  
  yearsAhead = (qd.year() - dStart.year());
  
  // check to see if date is in the proper range
  if ((qd >= dStart) &&
      (((yearsAhead <= (rDuration-1+(signed)exDates.count())*rFreq) && (rDuration > 0)) ||
       (rDuration == -1) ||
       ((rDuration == 0) && (qd <= rEndDate)))) {
    // do frequency check
    if ((yearsAhead % rFreq) == 0) {
      i = qd.dayOfYear();
      // correct for leapYears
      if (!QDate::leapYear(dStart.year()) &&
	  QDate::leapYear(qd.year()) &&
	  qd > QDate(qd.year(), 2, 28))
	--i;
      if (QDate::leapYear(dStart.year()) &&
	  !QDate::leapYear(qd.year()) &&
	  qd > QDate(qd.year(), 2, 28))
	++i;

      for (; qlin.current(); ++qlin) {
	if (i == *qlin.current())
	  return TRUE;
      }
      // no dates matched, return false
      return FALSE;
    } else 
      // frequency didn't match
      return FALSE;
  } // outside allowable date range
  return FALSE;
}

void KDPEvent::unsetRecurs()
{
  if (ro) return;
  recurs = rNone;
  rMonthPositions.clear();
  rMonthDays.clear();
  rYearNums.clear();
}

void KDPEvent::setRecursDaily(int _rFreq, int _rDuration)
{
  if (ro) return;
  recurs = rDaily;

  rFreq = _rFreq;
  rDuration = _rDuration;
  rMonthPositions.clear();
  rMonthDays.clear();
  rYearNums.clear();
  emit eventUpdated(this);
}

void KDPEvent::setRecursDaily(int _rFreq, const QDate &_rEndDate)
{
  if (ro) return;
  recurs = rDaily;

  rFreq = _rFreq;
  rEndDate = _rEndDate;
  rDuration = 0; // set to 0 because there is an end date
  rMonthPositions.clear();
  rMonthDays.clear();
  rYearNums.clear();
  emit eventUpdated(this);
}

inline int KDPEvent::getRecursFrequency() const
{
  return rFreq;
}

inline int KDPEvent::getRecursDuration() const
{
  return rDuration;
}

inline const QDate &KDPEvent::getRecursEndDate() const
{
  return rEndDate;
}

inline QString KDPEvent::getRecursEndDateStr() const
{
  QString dateStr;

  dateStr.sprintf("%.2d %3s %4d",rEndDate.day(),
		  (const char*)rEndDate.monthName(rEndDate.month()),
		  rEndDate.year());
  return dateStr;
}

inline const QBitArray &KDPEvent::getRecursDays() const
{
  return rDays;
}

inline const QList<KDPEvent::rMonthPos> &KDPEvent::getRecursMonthPositions() const
{
  return rMonthPositions;
}

inline const QList<int> &KDPEvent::getRecursMonthDays() const
{
  return rMonthDays;
}

void KDPEvent::setRecursWeekly(int _rFreq, const QBitArray &_rDays, 
			       int _rDuration)
{
  if (ro) return;
  recurs = rWeekly;

  rFreq = _rFreq;
  rDays = _rDays;
  rDuration = _rDuration;
  rMonthPositions.clear();
  rMonthDays.clear();
  emit eventUpdated(this);
}

void KDPEvent::setRecursWeekly(int _rFreq, const QBitArray &_rDays, 
			       const QDate &_rEndDate)
{
  if (ro) return;
  recurs = rWeekly;

  rFreq = _rFreq;
  rDays = _rDays;
  rEndDate = _rEndDate;
  rDuration = 0; // set to 0 because there is an end date
  rMonthPositions.clear();
  rMonthDays.clear();
  rYearNums.clear();
  emit eventUpdated(this);
}

void KDPEvent::setRecursMonthly(short type, int _rFreq, int _rDuration)
{
  if (ro) return;
  recurs = type;

  rFreq = _rFreq;
  rDuration = _rDuration;
  rYearNums.clear();
  emit eventUpdated(this);
}

void KDPEvent::setRecursMonthly(short type, int _rFreq, 
				const QDate &_rEndDate)
{
  if (ro) return;
  recurs = type;

  rFreq = _rFreq;
  rEndDate = _rEndDate;
  rDuration = 0; // set to 0 because there is an end date
  rYearNums.clear();
  emit eventUpdated(this);
}

void KDPEvent::addRecursMonthlyPos(short _rPos, const QBitArray &_rDays)
{
  if (ro) return;
  rMonthPos *tmpPos = new rMonthPos;
  tmpPos->negative = FALSE;
  if (_rPos < 0) {
    _rPos = 0 - _rPos; // take abs()
    tmpPos->negative = TRUE;
  }
  tmpPos->rPos = _rPos;
  tmpPos->rDays = _rDays;
  rMonthPositions.append(tmpPos);
  emit eventUpdated(this);
}

void KDPEvent::addRecursMonthlyDay(short _rDay)
{
  if (ro) return;
  int *tmpDay = new int;
  *tmpDay = _rDay;
  rMonthDays.append(tmpDay);
  emit eventUpdated(this);
}

void KDPEvent::setRecursYearly(int type, int _rFreq, int _rDuration)
{
  if (ro) return;
  recurs = type;
  
  rFreq = _rFreq;
  rDuration = _rDuration;
  rMonthPositions.clear();
  rMonthDays.clear();
  emit eventUpdated(this);
}

void KDPEvent::setRecursYearly(int type, int _rFreq, const QDate &_rEndDate)
{
  if (ro) return;
  recurs = type;

  rFreq = _rFreq;
  rEndDate = _rEndDate;
  rDuration = 0;
  rMonthPositions.clear();
  rMonthDays.clear();
  emit eventUpdated(this);
}

inline const QList<int> &KDPEvent::getRecursYearNums() const
{
  return rYearNums;
}

void KDPEvent::addRecursYearlyNum(short _rNum)
{

  if (ro) return;
  int *tmpNum = new int;
  *tmpNum = _rNum;
  rYearNums.append(tmpNum);
  emit eventUpdated(this);
}

void KDPEvent::setExDates(const QDateList &_exDates)
{
  if (ro) return;
  exDates.clear();
  exDates = _exDates;
  emit eventUpdated(this);
}

void KDPEvent::setExDates(const char *dates)
{
  if (ro) return;
  exDates.clear();
  QString tmpStr = dates;
  int index = 0;
  int index2 = 0;

  while ((index2 = tmpStr.find(',', index)) != -1) {
    QDate *tmpDate = new QDate;
    *tmpDate = strToDate(tmpStr.mid(index, (index2-index)));
    
    exDates.append(tmpDate);
    index = index2 + 1;
  }
  QDate *tmpDate = new QDate;
  *tmpDate = strToDate(tmpStr.mid(index, (tmpStr.length()-index)));
  exDates.inSort(tmpDate);
  emit eventUpdated(this);
}

void KDPEvent::addExDate(const QDate &date)
{
  if (ro) return;
  QDate *addDate = new QDate(date);
  exDates.inSort(addDate);
  emit eventUpdated(this);
}

inline const QDateList &KDPEvent::getExDates() const
{
  return exDates;
}

bool KDPEvent::isException(const QDate &qd) const
{
  QDateList tmpList(FALSE); // we want a shallow copy

  tmpList = exDates;

  QDate *datePtr;
  for (datePtr = tmpList.first(); datePtr;
       datePtr = tmpList.next()) {
    if (qd == *datePtr) {
      return TRUE;
    }
  }
  return FALSE;
}

inline void KDPEvent::setPilotId(int id)
{
  if (ro) return;
  pilotId = id;
  //emit eventUpdated(this);
}

inline int KDPEvent::getPilotId() const
{
  return pilotId;
}

inline void KDPEvent::setSyncStatus(int stat)
{
  if (ro) return;
  syncStatus = stat;
  //  emit eventUpdated(this);
}

inline int KDPEvent::getSyncStatus() const
{
  return syncStatus;
}

void KDPEvent::print(int style) const
{
  switch(style) {
  case ASCII:
    if (doesFloat())
      debug("\t\t: %s", getSummary().data());
    else
      debug("\t%02d:%02d-%02d:%02d: %s",
	     dtStart.time().hour(), dtStart.time().minute(),
	     dtEnd.time().hour(), dtEnd.time().minute(),
	     summary.data());
    break;
  case POSTSCRIPT:
    break;
  }
}

/***************************** PROTECTED FUNCTIONS ***************************/

QDateTime KDPEvent::strToDateTime(const QString &dateStr)
{
  // string should be in the format yyyymmddThhmmss
  
  int year, month, day, hour, minute, second;
  QDate tmpDate;
  QTime tmpTime;
  
  year = dateStr.left(4).toInt();
  month = dateStr.mid(4, 2).toInt();
  day = dateStr.mid(6, 2).toInt();
  
  hour = dateStr.mid(9, 2).toInt();
  minute = dateStr.mid(11, 2).toInt();
  second = dateStr.right(2).toInt();
  
  tmpDate.setYMD(year, month, day);
  tmpTime.setHMS(hour, minute, second);

  return QDateTime(tmpDate, tmpTime);
}

QDate KDPEvent::strToDate(const QString &dateStr)
{

  int year, month, day;

  year = dateStr.left(4).toInt();
  month = dateStr.mid(4,2).toInt();
  day = dateStr.mid(6,2).toInt();
  return(QDate(year, month, day));
}

// this should return the week of the month for the date
int KDPEvent::weekOfMonth(const QDate &qd) const
{
  QDate firstDate(qd.year(), qd.month(), 1);
  // I don't really know what formulas I'm using here.  :)
  int firstWeekNum(1 +(firstDate.dayOfYear() - firstDate.dayOfWeek() + 6)/7);
  int thisWeekNum(1 +(qd.dayOfYear() - qd.dayOfWeek() + 6)/7);
  return (thisWeekNum - firstWeekNum + 1);
}

void KDPEvent::updateConfig() 
{
  KConfig *config(kapp->getConfig());
  config->setGroup("Time & Date");
  
  weekStartsMonday = config->readBoolEntry("Week Starts Monday", FALSE);
}

/******************************* ATTENDEE CLASS *****************************/
// most methods have been inlined, see kdpevent.h for more information.
Attendee::Attendee(const char *n, const char *e, bool _rsvp, int s, int r)
{
  flag = TRUE;
  rsvp = _rsvp;
  name = n;
  email = e;
  status = s;
  role = r;
}

Attendee::Attendee(const Attendee &a)
{
  flag = a.flag;
  rsvp = a.rsvp;
  name = a.name.copy();
  email = a.email.copy();
  status = a.status;
  role = a.role;
}

Attendee::~Attendee()
{
}

void Attendee::setStatus(const char *s)
{
  QString statStr = s;
  statStr = statStr.upper();

  if (statStr == "X-ACTION")
    status = NEEDS_ACTION;
  else if (statStr == "NEEDS ACTION")
    status = NEEDS_ACTION;
  else if (statStr== "ACCEPTED")
    status = ACCEPTED;
  else if (statStr== "SENT")
    status = SENT;
  else if (statStr== "TENTATIVE")
  
  status = TENTATIVE;
  else if (statStr== "CONFIRMED")
    status = CONFIRMED;
  else if (statStr== "DECLINED")
    status = DECLINED;
  else if (statStr== "COMPLETED")
    status = COMPLETED;
  else if (statStr== "DELEGATED")
    status = DELEGATED;
  else {
    debug("error setting attendee status, unknown status!");
    status = NEEDS_ACTION;
  }
}

QString Attendee::getStatusStr() const
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

QString Attendee::getRoleStr() const
{
  switch(role) {
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
    rsvp = TRUE;
  else
    rsvp = FALSE;
}
