// Calendar class for KOrganizer
// (c) 1998 Preston Brown
// 	$Id$

#include "config.h"

#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <stdlib.h>

#include <qdatetm.h>
#include <qstring.h>
#include <qlist.h>
#include <stdlib.h>
#include <qmsgbox.h>
#include <qregexp.h>
#include <qclipbrd.h>
#include <qdialog.h>
#include <qmsgbox.h>
#include <qfile.h>

#include <kapp.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "vcaldrag.h"
#include "qdatelist.h"
#include "koprefs.h"

#include "calobject.h"
#include "calobject.moc"

CalObject::CalObject() : QObject(), recursCursor(recursList)
{
  struct passwd *pwent;
  uid_t userId;
  QString tmpStr;

  // initialization
  topWidget = 0L;
  oldestDate = 0L;
  newestDate = 0L;
  cursor = 0L;
  dialogsOn = TRUE;

  // initialize random numbers.  This is a hack, and not
  // even that good of one at that.
//  srandom(time(0L));

  // user information...
  userId = getuid();
  pwent = getpwuid(userId);
  tmpStr = KOPrefs::instance()->mName;
  if (tmpStr.isEmpty()) {
    
    if (strlen(pwent->pw_gecos) > 0)
      setOwner(pwent->pw_gecos);
    else
      setOwner(pwent->pw_name);
      KOPrefs::instance()->mName = getOwner();
  } else {
    setOwner(tmpStr);
  }

  tmpStr = KOPrefs::instance()->mEmail;
  if (tmpStr.isEmpty()) {
    tmpStr = pwent->pw_name;
    tmpStr += "@";

#ifdef HAVE_GETHOSTNAME
    char cbuf[80];
    if (gethostname(cbuf, 79)) {
      // error getting hostname
      tmpStr += "localhost";
    } else {
      hostent he;
      if (gethostbyname(cbuf)) {
	  he = *gethostbyname(cbuf);
	  tmpStr += he.h_name;
      } else {
	// error getting hostname
	tmpStr += "localhost";
      }
    }
#else
    tmpStr += "localhost";
#endif
    setEmail(tmpStr);
    KOPrefs::instance()->mEmail = tmpStr;
  } else {
    setEmail(tmpStr);
  }

  readHolidayFileName();

  tmpStr = KOPrefs::instance()->mTimeZone;
  int dstSetting = KOPrefs::instance()->mDaylightSavings;
  extern long int timezone;
  struct tm *now;
  time_t curtime;
  curtime = time(0);
  now = localtime(&curtime);
  int hourOff = - ((timezone / 60) / 60);
  if (now->tm_isdst)
    hourOff += 1;
  QString tzStr;
  tzStr.sprintf("%.2d%.2d",
		hourOff, 
		abs((timezone / 60) % 60));

  // if no time zone was in the config file, write what we just discovered.
  if (tmpStr.isEmpty()) {
    KOPrefs::instance()->mTimeZone = tzStr;
  }
  
  // if daylight savings has changed since last load time, we need
  // to rewrite these settings to the config file.
  if ((now->tm_isdst && !dstSetting) ||
      (!now->tm_isdst && dstSetting)) {
    KOPrefs::instance()->mTimeZone = tzStr;
    KOPrefs::instance()->mDaylightSavings = now->tm_isdst;
  }
  
  setTimeZone(tzStr.ascii());

  KOPrefs::instance()->writeConfig();

  recursList.setAutoDelete(TRUE);
  // solves the leak?
  todoList.setAutoDelete(TRUE);

  calDict = new QIntDict<QList<KOEvent> > (BIGPRIME);
  calDict->setAutoDelete(TRUE);
}

CalObject::~CalObject() 
{
  close();
  delete calDict;
  if (cursor)
    delete cursor;
  if (newestDate)
    delete newestDate;
  if (oldestDate)
    delete oldestDate;
}

bool CalObject::load(const QString &fileName)
{
  // do we want to silently accept this, or make some noise?  Dunno...
  // it is a semantical thing vs. a practical thing.
  if (fileName.isEmpty()) return false;

  VObject *vcal = 0L;
  const char *fn = fileName.latin1();

  // this is not necessarily only 1 vcal.  Could be many vcals, or include
  // a vcard...
  vcal = Parse_MIME_FromFileName(fn);

  if (!vcal) {
    loadError(fileName);
    return FALSE;
  }

  // any other top-level calendar stuff should be added/initialized here

  // put all vobjects into their proper places
  populate(vcal);

  // clean up from vcal API stuff
  cleanVObjects(vcal);
  cleanStrTbl();

  // set cursors to beginning of list.
  first(); 

  return TRUE;
}

void CalObject::close()
{
  QIntDictIterator<QList<KOEvent> > qdi(*calDict);
  QList<KOEvent> *tmpList;
  QList<KOEvent> multiDayEvents;
  KOEvent *anEvent;
  while (qdi.current()) {
    tmpList = qdi.current();
    ++qdi;
    tmpList->setAutoDelete(TRUE);
    for (anEvent = tmpList->first(); anEvent; anEvent = tmpList->next()) {
      // if the event spans multiple days, we need to put a pointer to it
      // in a list for later deletion, and only remove it's reference.  
      // Otherwise, since the list contains the only pointer to it, we
      // really want to blow it away.
      if (anEvent->isMultiDay()) {
	if (multiDayEvents.findRef(anEvent) == -1)
	  multiDayEvents.append(anEvent);
	tmpList->take();
      } else {
	tmpList->remove();
      }
    }
  }
  multiDayEvents.setAutoDelete(TRUE);
  multiDayEvents.clear();
  calDict->clear();
  recursList.clear();
  todoList.clear();
  
  // reset oldest/newest date markers
  delete oldestDate; oldestDate = 0L;
  delete newestDate; newestDate = 0L;
  if (cursor) {
    delete cursor;
    cursor = 0L;
  } 
}

bool CalObject::save(const QString &fileName)
{
  QString tmpStr;
  VObject *vcal, *vo;
  const char *fn = fileName.latin1();

  qDebug("CalObject::save(): %s",fn);

  vcal = newVObject(VCCalProp);

  //  addPropValue(vcal,VCLocationProp, "0.0");
  addPropValue(vcal,VCProdIdProp, _PRODUCT_ID);
  tmpStr = getTimeZoneStr();
  addPropValue(vcal,VCTimeZoneProp, tmpStr.ascii());
  addPropValue(vcal,VCVersionProp, _VCAL_VERSION);

  // TODO STUFF
  QListIterator<KOEvent> qlt(todoList);
  for (; qlt.current(); ++qlt) {
    vo = eventToVTodo(qlt.current());
    addVObjectProp(vcal, vo);
  }


  // EVENT STUFF
  QIntDictIterator<QList<KOEvent> > dictIt(*calDict);

  while (dictIt.current()) {
    QListIterator<KOEvent> listIt(*dictIt.current());
    while (listIt.current()) {
      // if the event is multi-day, we only want to save the
      // first instance that is in the dictionary
      if (listIt.current()->isMultiDay()) {
	QList<KOEvent> *tmpList = calDict->find(makeKey(listIt.current()->getDtStart().date()));
	if (dictIt.current() == tmpList) {
	  vo = eventToVEvent(listIt.current());
	  addVObjectProp(vcal, vo);
	}
      } else {
	vo = eventToVEvent(listIt.current());
	addVObjectProp(vcal, vo);
      }
      ++listIt;
    }
    ++dictIt;
  }

  // put in events that recurs
  QListIterator<KOEvent> qli(recursList);
  for (; qli.current(); ++qli) {
    vo = eventToVEvent(qli.current());
    addVObjectProp(vcal, vo);
  }

  writeVObjectToFile((char *) fn, vcal);
  cleanVObjects(vcal);
  cleanStrTbl();

  if (QFile::exists(fn)) {
    qDebug("No error");
    return true;
  } else  {
    qDebug("Error");
    return false; // error
  }
}

VCalDrag *CalObject::createDrag(KOEvent *selectedEv, QWidget *owner)
{
  VObject *vcal, *vevent;
  QString tmpStr;
  
  vcal = newVObject(VCCalProp);
  
  addPropValue(vcal,VCProdIdProp, _PRODUCT_ID);
  tmpStr = getTimeZoneStr();
  addPropValue(vcal,VCTimeZoneProp, tmpStr.latin1());
  addPropValue(vcal,VCVersionProp, _VCAL_VERSION);
  
  vevent = eventToVEvent(selectedEv);
  
  addVObjectProp(vcal, vevent);

  VCalDrag *vcd = new VCalDrag(vcal, owner);
  // free memory associated with vCalendar stuff
  cleanVObject(vcal);  
  vcd->setPixmap(UserIcon("newevent"));

  return vcd;
}

VCalDrag *CalObject::createDragTodo(KOEvent *selectedEv, QWidget *owner)
{
  VObject *vcal, *vevent;
  QString tmpStr;
  
  vcal = newVObject(VCCalProp);
  
  addPropValue(vcal,VCProdIdProp, _PRODUCT_ID);
  tmpStr = getTimeZoneStr();
  addPropValue(vcal,VCTimeZoneProp, tmpStr.latin1());
  addPropValue(vcal,VCVersionProp, _VCAL_VERSION);
  
  vevent = eventToVTodo(selectedEv);
  
  addVObjectProp(vcal, vevent);

  VCalDrag *vcd = new VCalDrag(vcal, owner);
  // free memory associated with vCalendar stuff
  cleanVObject(vcal);  
  vcd->setPixmap(UserIcon("newevent"));

  return vcd;
}

KOEvent *CalObject::createDrop(QDropEvent *de)
{
  VObject *vcal;
  KOEvent *event = 0;

  if (VCalDrag::decode(de, &vcal)) {
    VObjectIterator i;
    VObject *curvo;
    initPropIterator(&i, vcal);
    
    // we only take the first object.
    do  {
      curvo = nextVObject(&i);
    } while (strcmp(vObjectName(curvo), VCEventProp) &&
             strcmp(vObjectName(curvo), VCTodoProp));

    if (strcmp(vObjectName(curvo), VCTodoProp) == 0) {
      qDebug("CalObject::createDrop(): Got todo instead of event.");
    } else if (strcmp(vObjectName(curvo), VCEventProp) == 0) {
      event = VEventToEvent(curvo);
    } else {
      qDebug("CalObject::createDropTodo(): Unknown event type in drop.");
    }
    // get rid of temporary VObject
    deleteVObject(vcal);
  }
  
  return event;
}

KOEvent *CalObject::createDropTodo(QDropEvent *de)
{
  VObject *vcal;
  KOEvent *event = 0;

  if (VCalDrag::decode(de, &vcal)) {
    VObjectIterator i;
    VObject *curvo;
    initPropIterator(&i, vcal);
    
    // we only take the first object.
    do  {
      curvo = nextVObject(&i);
    } while (strcmp(vObjectName(curvo), VCEventProp) &&
             strcmp(vObjectName(curvo), VCTodoProp));

    if (strcmp(vObjectName(curvo), VCEventProp) == 0) {
      qDebug("CalObject::createDropTodo(): Got event instead of todo.");
    } else if (strcmp(vObjectName(curvo), VCTodoProp) == 0) {
      event = VTodoToEvent(curvo);
    } else {
      qDebug("CalObject::createDropTodo(): Unknown event type in drop.");
    }
    // get rid of temporary VObject
    deleteVObject(vcal);
  }
  
  return event;
}

void CalObject::cutEvent(KOEvent *selectedEv)
{
  if (copyEvent(selectedEv))
    deleteEvent(selectedEv);
}

bool CalObject::copyEvent(KOEvent *selectedEv)
{
  QClipboard *cb = QApplication::clipboard();
  VObject *vcal, *vevent;
  QString tmpStr;
  char *buf;

  vcal = newVObject(VCCalProp);

  //  addPropValue(vcal,VCLocationProp, "0.0");
  addPropValue(vcal,VCProdIdProp, _PRODUCT_ID);
  tmpStr = getTimeZoneStr();
  addPropValue(vcal,VCTimeZoneProp, tmpStr.ascii());
  addPropValue(vcal,VCVersionProp, _VCAL_VERSION);

  vevent = eventToVEvent(selectedEv);

  addVObjectProp(vcal, vevent);

  buf = writeMemVObject(0, 0, vcal);
  
  // free memory associated with vCalendar stuff
  cleanVObject(vcal);
  
  // paste to clipboard, then clear temp. buffer
  cb->setText(buf);
  delete buf;
  buf = 0L;

  return TRUE;
}

KOEvent *CalObject::pasteEvent(const QDate *newDate, 
				const QTime *newTime, VObject *vc)
{
  VObject *vcal, *curVO, *curVOProp;
  VObjectIterator i;
  int daysOffset;

  KOEvent *anEvent = 0L;

  if (!vc) {
    QClipboard *cb = QApplication::clipboard();
    int bufsize;
    const char *buf;
    buf = cb->text();
    bufsize = strlen(buf);
    
    if (strncmp("BEGIN:VCALENDAR",buf,
		strlen("BEGIN:VCALENDAR"))) {
      if (dialogsOn)
	QMessageBox::critical(topWidget, i18n("KOrganizer Error"),
			      i18n("An error has occurred parsing the "
				   "contents of the clipboard.\nYou can "
				   "only paste a valid vCalendar into "
				   "KOrganizer.\n"));
      return 0;
    }
    
    vcal = Parse_MIME(buf, bufsize);
    
    if (vcal == 0)
      if ((curVO = isAPropertyOf(vcal, VCCalProp)) == 0) {
	if (dialogsOn)
	  QMessageBox::critical(topWidget, i18n("KOrganizer Error"),
				i18n("An error has occurred parsing the "
				     "contents of the clipboard.\nYou can "
				     "only paste a valid vCalendar into "
				     "KOrganizer.\n"));
	return 0;
      }
  } else {
    vcal = vc;
  }

  initPropIterator(&i, vcal);
  
  // we only take the first object.
  do  {
    curVO = nextVObject(&i);
  } while (strcmp(vObjectName(curVO), VCEventProp));
  
  // now, check to see that the object is BOTH an event, and if so,
  // that it has a starting date
  if (strcmp(vObjectName(curVO), VCEventProp) == 0) {
    if ((curVOProp = isAPropertyOf(curVO, VCDTstartProp)) ||
	(curVOProp = isAPropertyOf(curVO, VCDTendProp))) {
      
      // we found an event with a start time, put it in the dict
      anEvent = VEventToEvent(curVO);
      // if we pasted an event that was the result of a copy in our
      // own calendar, now we have duplicate UID strings.  Need to generate
      // a new one for this new event.
      int hashTime = QTime::currentTime().hour() + 
	QTime::currentTime().minute() + QTime::currentTime().second() +
	QTime::currentTime().msec();
      QString uidStr;
      uidStr.sprintf("KOrganizer - %li.%d",KApplication::random(),hashTime);
      if (getEvent(anEvent->getVUID()))
	anEvent->setVUID(uidStr.ascii());

      daysOffset = anEvent->getDtEnd().date().dayOfYear() - 
	anEvent->getDtStart().date().dayOfYear();
      
      if (newTime)
	anEvent->setDtStart(QDateTime(*newDate, *newTime));
      else
	anEvent->setDtStart(QDateTime(*newDate, anEvent->getDtStart().time()));
      
      anEvent->setDtEnd(QDateTime(newDate->addDays(daysOffset),
				  anEvent->getDtEnd().time()));
      addEvent(anEvent);
    } else {
      debug("found a VEvent with no DTSTART/DTEND! Skipping");
    }
  } else if (strcmp(vObjectName(curVO), VCTodoProp) == 0) {
    anEvent = VTodoToEvent(curVO);
    addTodo(anEvent);
  } else {
    debug("unknown event type in paste!!!");
  }
  // get rid of temporary VObject
  deleteVObject(vcal);
  updateCursors(anEvent);
  return anEvent;
}

const QString &CalObject::getOwner() const
{
  return ownerString;
}

void CalObject::setOwner(const QString &os)
{
  int i;
  ownerString = os.ascii(); // to detach it
  i = ownerString.find(',');
  if (i != -1)
    ownerString = ownerString.left(i);
}

void CalObject::setTimeZone(const char *tz)
{
  bool neg = FALSE;
  int hours, minutes;
  QString tmpStr(tz);

  if (tmpStr.left(1) == "-")
    neg = TRUE;
  if (tmpStr.left(1) == "-" || tmpStr.left(1) == "+")
    tmpStr.remove(0, 1);
  hours = tmpStr.left(2).toInt();
  if (tmpStr.length() > 2) 
    minutes = tmpStr.right(2).toInt();
  else
    minutes = 0;
  timeZone = (60*hours+minutes);
  if (neg)
    timeZone = -timeZone;
}

QString CalObject::getTimeZoneStr() const 
{
  QString tmpStr;
  int hours = abs(timeZone / 60);
  int minutes = abs(timeZone % 60);
  bool neg = timeZone < 0;

  tmpStr.sprintf("%c%.2d%.2d",
		 (neg ? '-' : '+'),
		 hours, minutes);
  return tmpStr;
}

// don't ever call this unless a kapp exists!
void CalObject::updateConfig()
{
//  qDebug("CalObject::updateConfig()");

  bool updateFlag = FALSE;

  ownerString = KOPrefs::instance()->mName;

  // update events to new organizer (email address) 
  // if it has changed...
  QString configEmail = KOPrefs::instance()->mEmail;

  if (emailString != configEmail) {
    QString oldEmail = emailString;
    //    oldEmail.detach();
    emailString = configEmail;
    KOEvent *firstEvent, *currEvent;
    bool atFirst = TRUE;

    firstEvent = last();
    // gotta skip over the first one, which is same as first. 
    // I know, bad coding.
    for (currEvent = prev(); currEvent; currEvent = prev()) {
//      debug("in calobject::updateConfig(), currEvent summary: %s",currEvent->getSummary().ascii());
      if ((currEvent == firstEvent) && !atFirst) {
	break;
      }
      if (currEvent->getOrganizer() == oldEmail) {
	currEvent->setReadOnly(FALSE);
	currEvent->setOrganizer(emailString);
        updateFlag = TRUE;
      }
      atFirst = FALSE;
    }
  }

  readHolidayFileName();

  setTimeZone(KOPrefs::instance()->mTimeZone.latin1());

  if (updateFlag)
    emit calUpdated((KOEvent *) 0L);
}


void CalObject::readHolidayFileName()
{
  QString holidays(KOPrefs::instance()->mHoliday);
  if (holidays == "(none)") mHolidayfile = "";
  holidays = holidays.prepend("holiday_");
  mHolidayfile = locate("appdata",holidays);

//  qDebug("holifile: %s",mHolidayfile.latin1());
}

void CalObject::addEvent(KOEvent *anEvent)
{
  anEvent->isTodo = FALSE;
  insertEvent(anEvent);
  // set event's read/write status  
  if (anEvent->getOrganizer() != getEmail())
    anEvent->setReadOnly(TRUE);
  connect(anEvent, SIGNAL(eventUpdated(KOEvent *)), this,
	  SLOT(updateEvent(KOEvent *)));
  emit calUpdated(anEvent);
}

void CalObject::deleteEvent(const QDate &date, int eventId)
{
  QList<KOEvent> *tmpList;
  KOEvent *anEvent;
  int extraDays, dayOffset;
  QDate startDate, tmpDate;

  tmpList = calDict->find(makeKey(date));
  // if tmpList exists, the event is in the normal dictionary; 
  // it doesn't recur.
  if (tmpList) {
    for (anEvent = tmpList->first(); anEvent;
	 anEvent = tmpList->next()) {
      if (anEvent->getEventId() == eventId) {
	if (!anEvent->isMultiDay()) {
	  tmpList->setAutoDelete(FALSE);
	  tmpList->remove();
	  goto FINISH;
	} else {
	  //debug("deleting multi-day event");
	  // event covers multiple days.
	  startDate = anEvent->getDtStart().date();
	  extraDays = startDate.daysTo(anEvent->getDtEnd().date());
	  for (dayOffset = 0; dayOffset <= extraDays; dayOffset++) {
	    tmpDate = startDate.addDays(dayOffset);
	    tmpList = calDict->find(makeKey(tmpDate));
	    if (tmpList) {
	      for (anEvent = tmpList->first(); anEvent;
		   anEvent = tmpList->next()) {
		if (anEvent->getEventId() == eventId)
		  tmpList->remove();
	      }
	    }
	  }
	  // now we need to free the memory taken up by the event...
	  delete anEvent;
	  goto FINISH;
	}
      }
    }
  }
  for (anEvent = recursList.first(); anEvent;
       anEvent = recursList.next()) {
    if (anEvent->getEventId() == eventId) {
      recursList.remove();
    }
  }


 FINISH:
  // update oldest / newest dates if necessary
  // basically, first we check to see if this was the oldest
  // date in the calendar.  If it is, then we keep adding 1 to
  // the oldest date until we come up with a location in the 
  // QDate dictionary which has some entries.  Now, this might
  // be the oldest date, but we want to check the recurrence list
  // to make sure it has nothing older.  We start looping through
  // it, and each time we find something older, we adjust the oldest
  // date and start the loop again.  If we go through all the entries,
  // we are assured to have the new oldest date.
  //
  // the newest date is analogous, but sort of opposite.
  if (date == (*oldestDate)) {
    for (; !calDict->find(makeKey((*oldestDate))) &&
	   (*oldestDate != *newestDate); 
	 (*oldestDate) = oldestDate->addDays(1));
    recursList.first();
    while ((anEvent = recursList.current())) {
      if (anEvent->getDtStart().date() < (*oldestDate)) {
	(*oldestDate) = anEvent->getDtStart().date();
	recursList.first();
      }
      anEvent = recursList.next();
    }
  }

  if (date == (*newestDate)) {
    for (; !calDict->find(makeKey((*newestDate))) &&
	   (*newestDate != *oldestDate);
	 (*newestDate) = newestDate->addDays(-1));
    recursList.first();
    while ((anEvent = recursList.current())) {
      if (anEvent->getDtStart().date() > (*newestDate)) {
	(*newestDate) = anEvent->getDtStart().date();
	recursList.first();
      }
      anEvent = recursList.next();
    }
  }	  
}

// probably not really efficient, but...it works for now.
void CalObject::deleteEvent(KOEvent *anEvent)
{
  qDebug("CalObject::deleteEvent");
  
  int id = anEvent->getEventId();
  QDate startDate(anEvent->getDtStart().date());

  deleteEvent(startDate, id);
  emit calUpdated(anEvent);
}

KOEvent *CalObject::getEvent(const QDate &date, int eventId)
{
  QList<KOEvent> tmpList(getEventsForDate(date));
  KOEvent *anEvent = 0;

  for (anEvent = tmpList.first(); anEvent; anEvent = tmpList.next()) {
    if (anEvent->getEventId() == eventId) {
      updateCursors(anEvent);
      return anEvent;
    }
  }
  return (KOEvent *) 0L;
}

KOEvent *CalObject::getEvent(int eventId)
{
  QList<KOEvent>* eventList;
  QIntDictIterator<QList<KOEvent> > dictIt(*calDict);
  KOEvent *anEvent;

  while (dictIt.current()) {
    eventList = dictIt.current();
    for (anEvent = eventList->first(); anEvent;
	 anEvent = eventList->next()) {
      if (anEvent->getEventId() == eventId) {
	updateCursors(anEvent);
	return anEvent;
      }
    }
    ++dictIt;
  }
  for (anEvent = recursList.first(); anEvent;
       anEvent = recursList.next()) {
    if (anEvent->getEventId() == eventId) {
      updateCursors(anEvent);
      return anEvent;
    }
  }
  // catch-all.
  return (KOEvent *) 0L;
}

KOEvent *CalObject::getEvent(const QString &UniqueStr)
{
  QList<KOEvent> *eventList;
  QIntDictIterator<QList<KOEvent> > dictIt(*calDict);
  KOEvent *anEvent;

  while (dictIt.current()) {
    eventList = dictIt.current();
    for (anEvent = eventList->first(); anEvent;
	 anEvent = eventList->next()) {
      if (anEvent->getVUID() == UniqueStr) {
	updateCursors(anEvent);
	return anEvent;
      }
    }
    ++dictIt;
  }
  for (anEvent = recursList.first(); anEvent;
       anEvent = recursList.next()) {
    if (anEvent->getVUID() == UniqueStr) {
      updateCursors(anEvent);
      return anEvent;
    }
  }
  // catch-all.
  return (KOEvent *) 0L;
}

void CalObject::addTodo(KOEvent *todo)
{
  todo->isTodo = TRUE;
  todoList.append(todo);
  connect(todo, SIGNAL(eventUpdated(KOEvent *)), this,
	  SLOT(updateEvent(KOEvent *)));
  emit calUpdated(todo);
}

void CalObject::deleteTodo(KOEvent *todo)
{
  todoList.findRef(todo);
  todoList.remove();
  emit calUpdated(todo);
}


const QList<KOEvent> &CalObject::getTodoList() const
{
  return todoList;
}

KOEvent *CalObject::getTodo(int id)
{
  KOEvent *aTodo;
  for (aTodo = todoList.first(); aTodo;
       aTodo = todoList.next())
    if (id == aTodo->getEventId())
      return aTodo;
  // item not found
  return (KOEvent *) 0L;
}

KOEvent *CalObject::getTodo(const QString &UniqueStr)
{
  KOEvent *aTodo;
  for (aTodo = todoList.first(); aTodo;
       aTodo = todoList.next())
    if (aTodo->getVUID() == UniqueStr)
      return aTodo;
  // not found
  return (KOEvent *) 0L;
}

QList<KOEvent> CalObject::getTodosForDate(const QDate & date)
{
  QList<KOEvent> todos;

  KOEvent *aTodo;
  for (aTodo = todoList.first();aTodo;aTodo = todoList.next()) {
    if (aTodo->hasDueDate() && aTodo->getDtDue().date() == date) {
      todos.append(aTodo);
    }
  }
  
  return todos;
}

KOEvent *CalObject::first()
{
  if (!oldestDate || !newestDate)
    return (KOEvent *) 0L;
  
  QList<KOEvent> *tmpList;
  
  if (calDict->isEmpty() && recursList.isEmpty())
    return (KOEvent *) 0L;
  
  
  if (cursor) {
    delete cursor;
    cursor = 0L;
  }
  
  recursCursor.toFirst();
  if ((tmpList = calDict->find(makeKey((*oldestDate))))) {
    cursor = new QListIterator<KOEvent> (*tmpList);
    cursorDate = cursor->current()->getDtStart().date();
    return cursor->current();
  } else {
    recursCursor.toFirst();
    while (recursCursor.current() &&
	   recursCursor.current()->getDtStart().date() != (*oldestDate))
      ++recursCursor;
    cursorDate = recursCursor.current()->getDtStart().date();
    return recursCursor.current();
  }
}

KOEvent *CalObject::last()
{
  if (!oldestDate || !newestDate)
    return (KOEvent *) 0L;

  QList<KOEvent> *tmpList;
  
  if (calDict->isEmpty() && recursList.isEmpty())
    return (KOEvent *) 0L;
  
  if (cursor) {
    delete cursor;
    cursor = 0L;
  }
  
  recursCursor.toLast();
  if ((tmpList = calDict->find(makeKey((*newestDate))))) {
    cursor = new QListIterator<KOEvent> (*tmpList);
    cursor->toLast();
    cursorDate = cursor->current()->getDtStart().date();
    return cursor->current();
  } else {
    while (recursCursor.current() &&
	   recursCursor.current()->getDtStart().date() != (*newestDate))
      --recursCursor;
    cursorDate = recursCursor.current()->getDtStart().date();
    return recursCursor.current();
  }
}

KOEvent *CalObject::next()
{
  if (!oldestDate || !newestDate)
    return (KOEvent *) 0L;

  QList<KOEvent> *tmpList;
  int maxIterations = oldestDate->daysTo(*newestDate);
  int itCount = 0;
  
  if (calDict->isEmpty() && recursList.isEmpty())
    return (KOEvent *) 0L;
  
  // if itCount is greater than maxIterations (i.e. going around in 
  // a full circle) we have a bug. :)
  while (itCount <= maxIterations) {
    ++itCount;
  RESET: if (cursor) {
    if (!cursor->current()) {
      // the cursor should ALWAYS be on a valid element here,
      // but if something has been deleted from the list, it may
      // no longer be on one. be dumb; reset to beginning of list.
      cursor->toFirst();
      if (!cursor->current()) {
	// shit! we are traversing a cursor on a deleted list.
	delete cursor;
	cursor = 0L;
	goto RESET;
      }
    }
    KOEvent *anEvent = cursor->current();
    ++(*cursor);
    if (!cursor->current()) {
      // we have run out of events on this day.  delete cursor!
      delete cursor;
      cursor = 0L;
    }
    return anEvent;
  } else {
    // no cursor, we must check if anything in recursList
    while (recursCursor.current() &&
	   recursCursor.current()->getDtStart().date() != cursorDate)
      ++recursCursor;
    if (recursCursor.current()) {
      // we found one. :)
      KOEvent *anEvent = recursCursor.current();
      // increment, so we are set for the next time.
      // there are more dates in the list we were last looking at
      ++recursCursor;
      // check to see that we haven't run off the end.  If so, reset.
      //if (!recursCursor.current())
      //	recursCursor.toFirst();
      return anEvent;
    } else {
      // we ran out of events in the recurrence cursor.  
      // Increment cursorDate, make a new calDict cursor if dates available.
      // reset recursCursor to beginning of list.
      cursorDate = cursorDate.addDays(1);
      // we may have circled the globe.
      if (cursorDate == (newestDate->addDays(1)))
	cursorDate = (*oldestDate);
      
      if ((tmpList = calDict->find(makeKey(cursorDate)))) {
	cursor = new QListIterator<KOEvent> (*tmpList);
	cursor->toFirst();
      }
      recursCursor.toFirst();
    }
  }
  } // infinite while loop.
  debug("ieee! bug in calobject::next()");
  return (KOEvent *) 0L;
}

KOEvent *CalObject::prev()
{
  if (!oldestDate || !newestDate)
    return (KOEvent *) 0L;

  QList<KOEvent> *tmpList;
  int maxIterations = oldestDate->daysTo(*newestDate);
  int itCount = 0;

  if (calDict->isEmpty() && recursList.isEmpty())
    return (KOEvent *) 0L;
  
  // if itCount is greater than maxIterations (i.e. going around in 
  // a full circle) we have a bug. :)
  while (itCount <= maxIterations) {
    ++itCount;
  RESET: if (cursor) {
    if (!cursor->current()) {
      // the cursor should ALWAYS be on a valid element here,
      // but if something has been deleted from the list, it may
      // no longer be on one. be dumb; reset to end of list.
      cursor->toLast();
      if (!cursor->current()) {
	// shit! we are traversing a cursor on a deleted list.
	delete cursor;
	cursor = 0L;
	goto RESET;
      }
    }
    KOEvent *anEvent = cursor->current();
    --(*cursor);
    if (!cursor->current()) {
      // we have run out of events on this day.  delete cursor!
      delete cursor;
      cursor = 0L;
    }
    return anEvent;
  } else {
    // no cursor, we must check if anything in recursList
    while (recursCursor.current() &&
	   recursCursor.current()->getDtStart().date() != cursorDate)
      --recursCursor;
    if (recursCursor.current()) {
      // we found one. :)
      KOEvent *anEvent = recursCursor.current();
      // increment, so we are set for the next time.
      // there are more dates in the list we were last looking at
      --recursCursor;
      // check to see that we haven't run off the end.  If so, reset.
      //if (!recursCursor.current())
      //	recursCursor.toLast();
      return anEvent;
    } else {
      // we ran out of events in the recurrence cursor.  
      // Decrement cursorDate, make a new calDict cursor if dates available.
      // reset recursCursor to end of list.
      cursorDate = cursorDate.addDays(-1);
      // we may have circled the globe.
      if (cursorDate == (oldestDate->addDays(-1)))
	cursorDate = (*newestDate);
      
      if ((tmpList = calDict->find(makeKey(cursorDate)))) {
	cursor = new QListIterator<KOEvent> (*tmpList);
	cursor->toLast();
      }
      recursCursor.toLast();
    }
  }
  } // while loop
  debug("ieee! bug in calobject::prev()");
  return (KOEvent *) 0L;
}

KOEvent *CalObject::current()
{
  if (!oldestDate || !newestDate)
    return (KOEvent *) 0L;

  if (calDict->isEmpty() && recursList.isEmpty())
    return (KOEvent *) 0L;

  RESET: if (cursor) {
    if (cursor->current()) {
      // cursor should always be current, but weird things can
      // happen if it pointed to an event that got deleted...
      cursor->toFirst();
      if (!cursor->current()) {
	// shit! we are on a deleted list.
	delete cursor;
	cursor = 0L;
	goto RESET;
      }
    }
    return cursor->current();
  } else {
    // we must be in the recursList;
    return recursList.current();
  }
}

VObject *CalObject::eventToVTodo(const KOEvent *anEvent)
{
  VObject *vtodo;
  QString tmpStr;

  vtodo = newVObject(VCTodoProp);

  // due date
  if (anEvent->hasDueDate()) {
    tmpStr = qDateTimeToISO(anEvent->getDtDue(),
                            !anEvent->doesFloat());
    addPropValue(vtodo, VCDueProp, tmpStr.ascii());
  }

  // creation date
  tmpStr = qDateTimeToISO(anEvent->dateCreated);
  addPropValue(vtodo, VCDCreatedProp, tmpStr.ascii());

  // unique id
  addPropValue(vtodo, VCUniqueStringProp, 
	       anEvent->getVUID().ascii());

  // revision
  tmpStr.sprintf("%i", anEvent->getRevisionNum());
  addPropValue(vtodo, VCSequenceProp, tmpStr.ascii());

  // last modification date
  tmpStr = qDateTimeToISO(anEvent->getLastModified());
  addPropValue(vtodo, VCLastModifiedProp, tmpStr.ascii());  

  // organizer stuff
  tmpStr.sprintf("MAILTO:%s",anEvent->getOrganizer().ascii());
  addPropValue(vtodo, ICOrganizerProp,
	       tmpStr.ascii());

  // attendees
  if (anEvent->attendeeCount() != 0) {
    QList<Attendee> al = anEvent->getAttendeeList();
    QListIterator<Attendee> ai(al);
    Attendee *curAttendee;
    
    for (; ai.current(); ++ai) {
      curAttendee = ai.current();
      if (!curAttendee->getEmail().isEmpty() && 
	  !curAttendee->getName().isEmpty())
	tmpStr.sprintf("MAILTO:%s <%s>",curAttendee->getName().ascii(),
		       curAttendee->getEmail().ascii());
      else if (curAttendee->getName().isEmpty())
	tmpStr.sprintf("MAILTO: %s",curAttendee->getEmail().ascii());
      else if (curAttendee->getEmail().isEmpty())
	tmpStr.sprintf("MAILTO: %s",curAttendee->getName().ascii());
      else if (curAttendee->getName().isEmpty() && 
	       curAttendee->getEmail().isEmpty())
	debug("warning! this koevent has an attendee w/o name or email!");
      VObject *aProp = addPropValue(vtodo, VCAttendeeProp, tmpStr.ascii());
      addPropValue(aProp, VCRSVPProp, curAttendee->RSVP() ? "TRUE" : "FALSE");;
      addPropValue(aProp, VCStatusProp, curAttendee->getStatusStr().ascii());
    }
  }

  // description BL:
  if (!anEvent->getDescription().isEmpty()) {
    VObject *d = addPropValue(vtodo, VCDescriptionProp,
			      (const char *) anEvent->getDescription());
    if (strchr((const char *) anEvent->getDescription(), '\n'))
      addProp(d, VCQuotedPrintableProp);
  }

  // summary
  if (!anEvent->getSummary().isEmpty())
    addPropValue(vtodo, VCSummaryProp, anEvent->getSummary().ascii());

  // status
  addPropValue(vtodo, VCStatusProp, anEvent->getStatusStr().ascii());

  // priority
  tmpStr.sprintf("%i",anEvent->getPriority());
  addPropValue(vtodo, VCPriorityProp, tmpStr.ascii());

  // related event
  if (anEvent->getRelatedTo()) {
    addPropValue(vtodo, VCRelatedToProp,
	         anEvent->getRelatedTo()->getVUID().ascii());
  }

  // pilot sync stuff
  tmpStr.sprintf("%i",anEvent->getPilotId());
  addPropValue(vtodo, KPilotIdProp, tmpStr.ascii());
  tmpStr.sprintf("%i",anEvent->getSyncStatus());
  addPropValue(vtodo, KPilotStatusProp, tmpStr.ascii());

  return vtodo;
}

VObject* CalObject::eventToVEvent(const KOEvent *anEvent)
{
  VObject *vevent;
  QString tmpStr;
  QStrList tmpStrList;
  
  vevent = newVObject(VCEventProp);

  // start and end time
  tmpStr = qDateTimeToISO(anEvent->getDtStart(),
			  !anEvent->doesFloat());
  addPropValue(vevent, VCDTstartProp, tmpStr.ascii());

  // events that have time associated but take up no time should
  // not have both DTSTART and DTEND.
  if (anEvent->getDtStart() != anEvent->getDtEnd()) {
    tmpStr = qDateTimeToISO(anEvent->getDtEnd(),
			    !anEvent->doesFloat());
    addPropValue(vevent, VCDTendProp, tmpStr.ascii());
  }

  // creation date
  tmpStr = qDateTimeToISO(anEvent->dateCreated);
  addPropValue(vevent, VCDCreatedProp, tmpStr.ascii());

  // unique id
  addPropValue(vevent, VCUniqueStringProp,
	       anEvent->getVUID().ascii());

  // revision
  tmpStr.sprintf("%i", anEvent->getRevisionNum());
  addPropValue(vevent, VCSequenceProp, tmpStr.ascii());

  // last modification date
  tmpStr = qDateTimeToISO(anEvent->getLastModified());
  addPropValue(vevent, VCLastModifiedProp, tmpStr.ascii());

  // attendee and organizer stuff
  tmpStr.sprintf("MAILTO:%s",anEvent->getOrganizer().ascii());
  addPropValue(vevent, ICOrganizerProp,
	       tmpStr.ascii());
  if (anEvent->attendeeCount() != 0) {
    QList<Attendee> al = anEvent->getAttendeeList();
    QListIterator<Attendee> ai(al);
    Attendee *curAttendee;
    
    for (; ai.current(); ++ai) {
      curAttendee = ai.current();
      if (!curAttendee->getEmail().isEmpty() && 
	  !curAttendee->getName().isEmpty())
	tmpStr.sprintf("MAILTO:%s <%s>",curAttendee->getName().ascii(),
		       curAttendee->getEmail().ascii());
      else if (curAttendee->getName().isEmpty())
	tmpStr.sprintf("MAILTO: %s",curAttendee->getEmail().ascii());
      else if (curAttendee->getEmail().isEmpty())
	tmpStr.sprintf("MAILTO: %s",curAttendee->getName().ascii());
      else if (curAttendee->getName().isEmpty() && 
	       curAttendee->getEmail().isEmpty())
	debug("warning! this koevent has an attendee w/o name or email!");
      VObject *aProp = addPropValue(vevent, VCAttendeeProp, tmpStr.ascii());
      addPropValue(aProp, VCRSVPProp, curAttendee->RSVP() ? "TRUE" : "FALSE");;
      addPropValue(aProp, VCStatusProp, curAttendee->getStatusStr().ascii());
    }
  }


  // recurrence rule stuff
  if (anEvent->doesRecur()) {
    // some more variables
    QList<KOEvent::rMonthPos> tmpPositions;
    QList<int> tmpDays;
    int *tmpDay;
    KOEvent::rMonthPos *tmpPos;
    QString tmpStr2;

    switch(anEvent->doesRecur()) {
    case KOEvent::rDaily:
      tmpStr.sprintf("D%i ",anEvent->rFreq);
      if (anEvent->rDuration > 0)
	tmpStr += "#";
      break;
    case KOEvent::rWeekly:
      tmpStr.sprintf("W%i ",anEvent->rFreq);
      for (int i = 0; i < 7; i++) {
	if (anEvent->rDays.testBit(i))
	  tmpStr += dayFromNum(i);
      }
      break;
    case KOEvent::rMonthlyPos:
      tmpStr.sprintf("MP%i ", anEvent->rFreq);
      // write out all rMonthPos's
      tmpPositions = anEvent->rMonthPositions;
      for (tmpPos = tmpPositions.first();
	   tmpPos;
	   tmpPos = tmpPositions.next()) {
	
	tmpStr2.sprintf("%i", tmpPos->rPos);
	if (tmpPos->negative)
	  tmpStr2 += "- ";
	else
	  tmpStr2 += "+ ";
	tmpStr += tmpStr2;
	for (int i = 0; i < 7; i++) {
	  if (tmpPos->rDays.testBit(i))
	    tmpStr += dayFromNum(i);
	}
      } // loop for all rMonthPos's
      break;
    case KOEvent::rMonthlyDay:
      tmpStr.sprintf("MD%i ", anEvent->rFreq);
      // write out all rMonthDays;
      tmpDays = anEvent->rMonthDays;
      for (tmpDay = tmpDays.first();
	   tmpDay;
	   tmpDay = tmpDays.next()) {
	tmpStr2.sprintf("%i ", *tmpDay);
	tmpStr += tmpStr2;
      }
      break;
    case KOEvent::rYearlyMonth:
      tmpStr.sprintf("YM%i ", anEvent->rFreq);
      // write out all the rYearNums;
      tmpDays = anEvent->rYearNums;
      for (tmpDay = tmpDays.first();
	   tmpDay;
	   tmpDay = tmpDays.next()) {
	tmpStr2.sprintf("%i ", *tmpDay);
	tmpStr += tmpStr2;
      }
      break;
    case KOEvent::rYearlyDay:
      tmpStr.sprintf("YD%i ", anEvent->rFreq);
      // write out all the rYearNums;
      tmpDays = anEvent->rYearNums;
      for (tmpDay = tmpDays.first();
	   tmpDay;
	   tmpDay = tmpDays.next()) {
	tmpStr2.sprintf("%i ", *tmpDay);
	tmpStr += tmpStr2;
      }
      break;
    default:
      debug("ERROR, it should never get here in eventToVEvent!");
      break;
    } // switch

    if (anEvent->rDuration > 0) {
      tmpStr2.sprintf("#%i",anEvent->rDuration);
      tmpStr += tmpStr2;
    } else if (anEvent->rDuration == -1) {
      tmpStr += "#0"; // defined as repeat forever
    } else {
      tmpStr += qDateTimeToISO(anEvent->rEndDate, FALSE);
    }
    addPropValue(vevent,VCRRuleProp, tmpStr.ascii());

  } // event repeats


  // exceptions to recurrence
  QDateList dateList(FALSE);
  dateList = anEvent->getExDates();
  QDate *tmpDate;
  QString tmpStr2 = "";

  for (tmpDate = dateList.first(); tmpDate; tmpDate = dateList.next()) {
    tmpStr = qDateToISO(*tmpDate) + ";";
    tmpStr2 += tmpStr;
  }
  if (!tmpStr2.isEmpty()) {
    tmpStr2.truncate(tmpStr2.length()-1);
    addPropValue(vevent, VCExDateProp, tmpStr2.ascii());
  }

  // description
  if (!anEvent->getDescription().isEmpty()) {
    VObject *d = addPropValue(vevent, VCDescriptionProp,
			      (const char *) anEvent->getDescription());
    if (strchr((const char *) anEvent->getDescription(), '\n'))
      addProp(d, VCQuotedPrintableProp);
  }

  // summary
  if (!anEvent->getSummary().isEmpty())
    addPropValue(vevent, VCSummaryProp, anEvent->getSummary().ascii());

  // status
  addPropValue(vevent, VCStatusProp, anEvent->getStatusStr().ascii());
  
  // secrecy
  addPropValue(vevent, VCClassProp, anEvent->getSecrecyStr().ascii());

  // categories
  tmpStrList = anEvent->getCategories();
  tmpStr = "";
  char *catStr;
  for (catStr = tmpStrList.first(); catStr; 
       catStr = tmpStrList.next()) {
    if (catStr[0] == ' ')
      tmpStr += (catStr+1);
    else
      tmpStr += catStr;
    // this must be a ';' character as the vCalendar specification requires!
    // vcc.y has been hacked to translate the ';' to a ',' when the vcal is
    // read in.
    tmpStr += ";";
  }
  if (!tmpStr.isEmpty()) {
    tmpStr.truncate(tmpStr.length()-1);
    addPropValue(vevent, VCCategoriesProp, tmpStr.ascii());
  }

  // attachments
  tmpStrList = anEvent->getAttachments();
  char *attachStr;
  for (attachStr = tmpStrList.first(); attachStr;
       attachStr = tmpStrList.next())
    addPropValue(vevent, VCAttachProp, attachStr);
  
  // resources
  tmpStrList = anEvent->getResources();
  tmpStr = "";
  char *resStr;
  for (resStr = tmpStrList.first(); resStr;
       resStr = tmpStrList.next()) {
    tmpStr += resStr;
    if (tmpStrList.next())
      tmpStr += ";";
  }
  if (!tmpStr.isEmpty())
    addPropValue(vevent, VCResourcesProp, tmpStr.ascii());
  
  // alarm stuff
  if (anEvent->getAlarmRepeatCount()) {
    VObject *a = addProp(vevent, VCDAlarmProp);
    tmpStr = qDateTimeToISO(anEvent->getAlarmTime());
    addPropValue(a, VCRunTimeProp, tmpStr.ascii());
    addPropValue(a, VCRepeatCountProp, "1");
    addPropValue(a, VCDisplayStringProp, "beep!");
    if (!anEvent->getAudioAlarmFile().isEmpty()) {
      a = addProp(vevent, VCAAlarmProp);
      addPropValue(a, VCRunTimeProp, tmpStr.ascii());
      addPropValue(a, VCRepeatCountProp, "1");
      addPropValue(a, VCAudioContentProp, anEvent->getAudioAlarmFile().ascii());
    }
    if (!anEvent->getProgramAlarmFile().isEmpty()) {
      a = addProp(vevent, VCPAlarmProp);
      addPropValue(a, VCRunTimeProp, tmpStr.ascii());
      addPropValue(a, VCRepeatCountProp, "1");
      addPropValue(a, VCProcedureNameProp, anEvent->getProgramAlarmFile().ascii());
    }
  }

  // priority	    
  tmpStr.sprintf("%i",anEvent->getPriority());
  addPropValue(vevent, VCPriorityProp, tmpStr.ascii());

  // transparency
  tmpStr.sprintf("%i",anEvent->getTransparency());
  addPropValue(vevent, VCTranspProp, tmpStr.ascii());
  
  // related event
  if (anEvent->getRelatedTo()) {
    addPropValue(vevent, VCRelatedToProp,
	         anEvent->getRelatedTo()->getVUID().ascii());
  }

  // pilot sync stuff
  tmpStr.sprintf("%i",anEvent->getPilotId());
  addPropValue(vevent, KPilotIdProp, tmpStr.ascii());
  tmpStr.sprintf("%i",anEvent->getSyncStatus());
  addPropValue(vevent, KPilotStatusProp, tmpStr.ascii());

  return vevent;
}

KOEvent *CalObject::VTodoToEvent(VObject *vtodo)
{
  KOEvent *anEvent;
  VObject *vo;
  VObjectIterator voi;
  char *s;

  anEvent = new KOEvent;
  anEvent->setTodoStatus(TRUE);

  // creation date
  if ((vo = isAPropertyOf(vtodo, VCDCreatedProp)) != 0) {
      anEvent->dateCreated = ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo)));
      deleteStr(s);
  }

  // unique id
  vo = isAPropertyOf(vtodo, VCUniqueStringProp);
  // while the UID property is preferred, it is not required.  We'll use the
  // default KOEvent UID if none is given.
  if (vo) {
    anEvent->setVUID(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  }

  // last modification date
  if ((vo = isAPropertyOf(vtodo, VCLastModifiedProp)) != 0) {
    anEvent->setLastModified(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    anEvent->setLastModified(QDateTime(QDate::currentDate(),
				       QTime::currentTime()));

  // organizer
  // if our extension property for the event's ORGANIZER exists, add it.
  if ((vo = isAPropertyOf(vtodo, ICOrganizerProp)) != 0) {
    anEvent->setOrganizer(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  } else {
    anEvent->setOrganizer(getEmail());
  }

  // attendees.
  initPropIterator(&voi, vtodo);
  while (moreIteration(&voi)) {
    vo = nextVObject(&voi);
    if (strcmp(vObjectName(vo), VCAttendeeProp) == 0) {
      Attendee *a;
      VObject *vp;
      s = fakeCString(vObjectUStringZValue(vo));
      QString tmpStr = s;
      deleteStr(s);
      tmpStr = tmpStr.simplifyWhiteSpace();
      int emailPos1, emailPos2;
      if ((emailPos1 = tmpStr.find('<')) > 0) {
	// both email address and name
	emailPos2 = tmpStr.find('>');
	a = new Attendee(tmpStr.left(emailPos1 - 1).ascii(),
			 tmpStr.mid(emailPos1 + 1, 
				    emailPos2 - (emailPos1 + 1)).ascii());
      } else if (tmpStr.find('@') > 0) {
	// just an email address
	a = new Attendee(0, tmpStr.ascii());
      } else {
	// just a name
	a = new Attendee(tmpStr.ascii());
      }

      // is there an RSVP property?
      if ((vp = isAPropertyOf(vo, VCRSVPProp)) != 0)
	a->setRSVP(vObjectStringZValue(vp));
      // is there a status property?
      if ((vp = isAPropertyOf(vo, VCStatusProp)) != 0)
	a->setStatus(vObjectStringZValue(vp));
      // add the attendee
      anEvent->addAttendee(a);
    }
  }

  // BL: description for todo
  if ((vo = isAPropertyOf(vtodo, VCDescriptionProp)) != 0) {
    anEvent->setDescription(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  }
  
  // summary
  if ((vo = isAPropertyOf(vtodo, VCSummaryProp))) {
    anEvent->setSummary(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  }
  
  // status
  if ((vo = isAPropertyOf(vtodo, VCStatusProp)) != 0) {
    anEvent->setStatus(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  }
  else
    anEvent->setStatus("NEEDS ACTION");
  
  // priority
  if ((vo = isAPropertyOf(vtodo, VCPriorityProp))) {
    anEvent->setPriority(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }

  // due date
  if ((vo = isAPropertyOf(vtodo, VCDueProp)) != 0) {
    anEvent->setDtDue(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
    anEvent->setHasDueDate(true);
  } else {
    anEvent->setHasDueDate(false);
  }

  // related todo  
  if ((vo = isAPropertyOf(vtodo, VCRelatedToProp)) != 0) {
    anEvent->setRelatedToVUID(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
    mTodosRelate.append(anEvent);
  }

  /* PILOT SYNC STUFF */
  if ((vo = isAPropertyOf(vtodo, KPilotIdProp))) {
    anEvent->setPilotId(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    anEvent->setPilotId(0);

  if ((vo = isAPropertyOf(vtodo, KPilotStatusProp))) {
    anEvent->setSyncStatus(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    anEvent->setSyncStatus(KOEvent::SYNCMOD);

  return anEvent;
}

KOEvent* CalObject::VEventToEvent(VObject *vevent)
{
  KOEvent *anEvent;
  VObject *vo;
  VObjectIterator voi;
  char *s;

  anEvent = new KOEvent;
  
  // creation date
  if ((vo = isAPropertyOf(vevent, VCDCreatedProp)) != 0) {
      anEvent->dateCreated = ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo)));
      deleteStr(s);
  }

  // unique id
  vo = isAPropertyOf(vevent, VCUniqueStringProp);
  if (!vo) {
    parseError(VCUniqueStringProp);
    return 0;
  }
  anEvent->setVUID(s = fakeCString(vObjectUStringZValue(vo)));
  deleteStr(s);

  // revision
  // again NSCAL doesn't give us much to work with, so we improvise...
  if ((vo = isAPropertyOf(vevent, VCSequenceProp)) != 0) {
    anEvent->setRevisionNum(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    anEvent->setRevisionNum(0);

  // last modification date
  if ((vo = isAPropertyOf(vevent, VCLastModifiedProp)) != 0) {
    anEvent->setLastModified(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    anEvent->setLastModified(QDateTime(QDate::currentDate(),
				       QTime::currentTime()));

  // organizer
  // if our extension property for the event's ORGANIZER exists, add it.
  if ((vo = isAPropertyOf(vevent, ICOrganizerProp)) != 0) {
    anEvent->setOrganizer(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  } else {
    anEvent->setOrganizer(getEmail());
  }

  // deal with attendees.
  initPropIterator(&voi, vevent);
  while (moreIteration(&voi)) {
    vo = nextVObject(&voi);
    if (strcmp(vObjectName(vo), VCAttendeeProp) == 0) {
      Attendee *a;
      VObject *vp;
      s = fakeCString(vObjectUStringZValue(vo));
      QString tmpStr = s;
      deleteStr(s);
      tmpStr = tmpStr.simplifyWhiteSpace();
      int emailPos1, emailPos2;
      if ((emailPos1 = tmpStr.find('<')) > 0) {
	// both email address and name
	emailPos2 = tmpStr.find('>');
	a = new Attendee(tmpStr.left(emailPos1 - 1).ascii(),
			 tmpStr.mid(emailPos1 + 1, 
				    emailPos2 - (emailPos1 + 1)).ascii());
      } else if (tmpStr.find('@') > 0) {
	// just an email address
	a = new Attendee(0, tmpStr.ascii());
      } else {
	// just a name
	a = new Attendee(tmpStr.ascii());
      }

      // is there an RSVP property?
      if ((vp = isAPropertyOf(vo, VCRSVPProp)) != 0)
	a->setRSVP(vObjectStringZValue(vp));
      // is there a status property?
      if ((vp = isAPropertyOf(vo, VCStatusProp)) != 0)
	a->setStatus(vObjectStringZValue(vp));
      // add the attendee
      anEvent->addAttendee(a);
    }
  }
 
  // This isn't strictly true.  An event that doesn't have a start time
  // or an end time doesn't "float", it has an anchor in time but it doesn't
  // "take up" any time.
  /*if ((isAPropertyOf(vevent, VCDTstartProp) == 0) ||
      (isAPropertyOf(vevent, VCDTendProp) == 0)) {
    anEvent->setFloats(TRUE);
    } else {
    }*/

  anEvent->setFloats(FALSE);
  
  // start time
  if ((vo = isAPropertyOf(vevent, VCDTstartProp)) != 0) {
    anEvent->setDtStart(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))));
    //    debug("s is %s, ISO is %s",
    //	  s, ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))).toString().ascii());
    deleteStr(s);
    if (anEvent->getDtStart().time().isNull())
      anEvent->setFloats(TRUE);
  }
  
  // stop time
  if ((vo = isAPropertyOf(vevent, VCDTendProp)) != 0) {
    anEvent->setDtEnd(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))));
      deleteStr(s);
      if (anEvent->getDtEnd().time().isNull())
	anEvent->setFloats(TRUE);
  }
  
  // at this point, there should be at least a start or end time.
  // fix up for events that take up no time but have a time associated
  if (!(vo = isAPropertyOf(vevent, VCDTstartProp)))
    anEvent->setDtStart(anEvent->getDtEnd());
  if (!(vo = isAPropertyOf(vevent, VCDTendProp)))
    anEvent->setDtEnd(anEvent->getDtStart());
  
  ///////////////////////////////////////////////////////////////////////////

  // repeat stuff
  if ((vo = isAPropertyOf(vevent, VCRRuleProp)) != 0) {
    QString tmpStr = (s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
    tmpStr.simplifyWhiteSpace();
    tmpStr = tmpStr.upper();

    /********************************* DAILY ******************************/
    if (tmpStr.left(1) == "D") {
      int index = tmpStr.find(' ');
      int rFreq = tmpStr.mid(1, (index-1)).toInt();
      index = tmpStr.findRev(' ') + 1; // advance to last field
      if (tmpStr.mid(index,1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
	QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length()-index).ascii())).date();
	anEvent->setRecursDaily(rFreq, rEndDate);
      } else {
	int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	if (rDuration == 0) // VEvents set this to 0 forever, we use -1
	  anEvent->setRecursDaily(rFreq, -1);
	else
	  anEvent->setRecursDaily(rFreq, rDuration);
      }
    } 
    /********************************* WEEKLY ******************************/
    else if (tmpStr.left(1) == "W") {
      int index = tmpStr.find(' ');
      int last = tmpStr.findRev(' ') + 1;
      int rFreq = tmpStr.mid(1, (index-1)).toInt();
      index += 1; // advance to beginning of stuff after freq
      QBitArray qba(7);
      QString dayStr;
      if( index == last ) {
	// e.g. W1 #0
	qba.setBit(anEvent->getDtStart().date().dayOfWeek() - 1);
      }
      else {
	// e.g. W1 SU #0
	while (index < last) {
	  dayStr = tmpStr.mid(index, 3);
	  int dayNum = numFromDay(dayStr);
	  qba.setBit(dayNum);
	  index += 3; // advance to next day, or possibly "#"
	}
      }
      index = last; if (tmpStr.mid(index,1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
	QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length()-index).ascii())).date();
	anEvent->setRecursWeekly(rFreq, qba, rEndDate);
      } else {
	int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	if (rDuration == 0)
	  anEvent->setRecursWeekly(rFreq, qba, -1);
	else
	  anEvent->setRecursWeekly(rFreq, qba, rDuration);
      }
    } 
    /**************************** MONTHLY-BY-POS ***************************/
    else if (tmpStr.left(2) == "MP") {
      int index = tmpStr.find(' ');
      int last = tmpStr.findRev(' ') + 1;
      int rFreq = tmpStr.mid(2, (index-1)).toInt();
      index += 1; // advance to beginning of stuff after freq
      QBitArray qba(7);
      short tmpPos;
      if( index == last ) {
	// e.g. MP1 #0
	tmpPos = anEvent->getDtStart().date().day()/7 + 1;
	if( tmpPos == 5 )
	  tmpPos = -1;
	qba.setBit(anEvent->getDtStart().date().dayOfWeek() - 1);
	anEvent->addRecursMonthlyPos(tmpPos, qba);
      }
      else {
	// e.g. MP1 1+ SU #0
	while (index < last) {
	  tmpPos = tmpStr.mid(index,1).toShort();
	  index += 1;
	  if (tmpStr.mid(index,1) == "-")
	    // convert tmpPos to negative
	    tmpPos = 0 - tmpPos;
	  index += 2; // advance to day(s)
	  while (numFromDay(tmpStr.mid(index,3)) >= 0) {
	    int dayNum = numFromDay(tmpStr.mid(index,3));
	    qba.setBit(dayNum);
	    index += 3; // advance to next day, or possibly pos or "#"
	  }
	  anEvent->addRecursMonthlyPos(tmpPos, qba);
	  qba.detach();
	  qba.fill(FALSE); // clear out
	} // while != "#"
      }
      index = last; if (tmpStr.mid(index,1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
	QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length() - 
						    index).ascii())).date();
	anEvent->setRecursMonthly(KOEvent::rMonthlyPos, rFreq, rEndDate);
      } else {
	int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	if (rDuration == 0)
	  anEvent->setRecursMonthly(KOEvent::rMonthlyPos, rFreq, -1);
	else
	  anEvent->setRecursMonthly(KOEvent::rMonthlyPos, rFreq, rDuration);
      }
    }

    /**************************** MONTHLY-BY-DAY ***************************/
    else if (tmpStr.left(2) == "MD") {
      int index = tmpStr.find(' ');
      int last = tmpStr.findRev(' ') + 1;
      int rFreq = tmpStr.mid(2, (index-1)).toInt();
      index += 1;
      short tmpDay;
      if( index == last ) {
	// e.g. MD1 #0
	tmpDay = anEvent->getDtStart().date().day();
	anEvent->addRecursMonthlyDay(tmpDay);
      }
      else {
	// e.g. MD1 3 #0
	while (index < last) {
	  int index2 = tmpStr.find(' ', index); 
	  tmpDay = tmpStr.mid(index, (index2-index)).toShort();
	  index = index2-1;
	  if (tmpStr.mid(index, 1) == "-")
	    tmpDay = 0 - tmpDay;
	  index += 2; // advance the index;
	  anEvent->addRecursMonthlyDay(tmpDay);
	} // while != #
      }
      index = last; if (tmpStr.mid(index,1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
	QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length()-index).ascii())).date();
	anEvent->setRecursMonthly(KOEvent::rMonthlyDay, rFreq, rEndDate);
      } else {
	int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	if (rDuration == 0)
	  anEvent->setRecursMonthly(KOEvent::rMonthlyDay, rFreq, -1);
	else
	  anEvent->setRecursMonthly(KOEvent::rMonthlyDay, rFreq, rDuration);
      }
    }

    /*********************** YEARLY-BY-MONTH *******************************/
    else if (tmpStr.left(2) == "YM") {
      int index = tmpStr.find(' ');
      int last = tmpStr.findRev(' ') + 1;
      int rFreq = tmpStr.mid(2, (index-1)).toInt();
      index += 1;
      short tmpMonth;
      if( index == last ) {
	// e.g. YM1 #0
	tmpMonth = anEvent->getDtStart().date().month();
	anEvent->addRecursYearlyNum(tmpMonth);
      }
      else {
	// e.g. YM1 3 #0
	while (index < last) {
	  int index2 = tmpStr.find(' ', index);
	  tmpMonth = tmpStr.mid(index, (index2-index)).toShort();
	  index = index2+1;
	  anEvent->addRecursYearlyNum(tmpMonth);
	} // while != #
      }
      index = last; if (tmpStr.mid(index,1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
	QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length()-index).ascii())).date();
	anEvent->setRecursYearly(KOEvent::rYearlyMonth, rFreq, rEndDate);
      } else {
	int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	if (rDuration == 0)
	  anEvent->setRecursYearly(KOEvent::rYearlyMonth, rFreq, -1);
	else
	  anEvent->setRecursYearly(KOEvent::rYearlyMonth, rFreq, rDuration);
      }
    }

    /*********************** YEARLY-BY-DAY *********************************/
    else if (tmpStr.left(2) == "YD") {
      int index = tmpStr.find(' ');
      int last = tmpStr.findRev(' ') + 1;
      int rFreq = tmpStr.mid(2, (index-1)).toInt();
      index += 1;
      short tmpDay;
      if( index == last ) {
	// e.g. YD1 #0
	tmpDay = anEvent->getDtStart().date().dayOfYear();
	anEvent->addRecursYearlyNum(tmpDay);
      }
      else {
	// e.g. YD1 123 #0
	while (index < last) {
	  int index2 = tmpStr.find(' ', index);
	  tmpDay = tmpStr.mid(index, (index2-index)).toShort();
	  index = index2+1;
	  anEvent->addRecursYearlyNum(tmpDay);
	} // while != #
      }
      index = last; if (tmpStr.mid(index,1) == "#") index++;
      if (tmpStr.find('T', index) != -1) {
	QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length()-index).ascii())).date();
	anEvent->setRecursYearly(KOEvent::rYearlyDay, rFreq, rEndDate);
      } else {
	int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	if (rDuration == 0)
	  anEvent->setRecursYearly(KOEvent::rYearlyDay, rFreq, -1);
	else
	  anEvent->setRecursYearly(KOEvent::rYearlyDay, rFreq, rDuration);
      }
    } else {
      debug("we don't understand this type of recurrence!");
    } // if
  } // repeats


  // recurrence exceptions
  if ((vo = isAPropertyOf(vevent, VCExDateProp)) != 0) {
    anEvent->setExDates(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  }

  // summary
  if ((vo = isAPropertyOf(vevent, VCSummaryProp))) {
    anEvent->setSummary(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  }

  // description
  if ((vo = isAPropertyOf(vevent, VCDescriptionProp)) != 0) {
    if (!anEvent->getDescription().isEmpty()) {
      anEvent->setDescription(anEvent->getDescription() + "\n" +
			      s = fakeCString(vObjectUStringZValue(vo)));
    } else {
      anEvent->setDescription(s = fakeCString(vObjectUStringZValue(vo)));
    }
    deleteStr(s);
  }

  // some stupid vCal exporters ignore the standard and use Description
  // instead of Summary for the default field.  Correct for this.
  if (anEvent->getSummary().isEmpty() && 
      !(anEvent->getDescription().isEmpty())) {
    QString tmpStr = anEvent->getDescription().simplifyWhiteSpace();
    anEvent->setDescription("");
    anEvent->setSummary(tmpStr.ascii());
  }  

  // status
  if ((vo = isAPropertyOf(vevent, VCStatusProp)) != 0) {
    QString tmpStr(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
    anEvent->setStatus(tmpStr);
  }
  else
    anEvent->setStatus("NEEDS ACTION");

  // secrecy
  if ((vo = isAPropertyOf(vevent, VCClassProp)) != 0) {
    anEvent->setSecrecy(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  }
  else
    anEvent->setSecrecy("PUBLIC");

  // categories
  QStrList tmpStrList;
  int index1 = 0;
  int index2 = 0;
  if ((vo = isAPropertyOf(vevent, VCCategoriesProp)) != 0) {
    QString categories = (s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
    //const char* category;
    QString category;
    while ((index2 = categories.find(',', index1)) != -1) {
	//category = (const char *) categories.mid(index1, (index2 - index1));
      category = categories.mid(index1, (index2 - index1));	
      tmpStrList.append(category);
      index1 = index2+1;
    }
    // get last category
    category = categories.mid(index1, (categories.length()-index1));
    tmpStrList.append(category);
    anEvent->setCategories(tmpStrList);
  }

  // attachments
  tmpStrList.clear();
  initPropIterator(&voi, vevent);
  while (moreIteration(&voi)) {
    vo = nextVObject(&voi);
    if (strcmp(vObjectName(vo), VCAttachProp) == 0) {
      tmpStrList.append(s = fakeCString(vObjectUStringZValue(vo)));
      deleteStr(s);
    }
  }
  anEvent->setAttachments(tmpStrList);

  // resources
  if ((vo = isAPropertyOf(vevent, VCResourcesProp)) != 0) {
    QString resources = (s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
    tmpStrList.clear();
    index1 = 0;
    index2 = 0;
    const char *resource;
    while ((index2 = resources.find(';', index1)) != -1) {
      resource = (const char *) resources.mid(index1, (index2 - index1));
      tmpStrList.append(resource);
      index1 = index2;
    }
    anEvent->setResources(tmpStrList);
  }

  /* alarm stuff */
  if ((vo = isAPropertyOf(vevent, VCDAlarmProp))) {
    VObject *a;
    if ((a = isAPropertyOf(vo, VCRunTimeProp))) {
      anEvent->setAlarmTime(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(a))));
      deleteStr(s);
    }
    anEvent->setAlarmRepeatCount(1);
    if ((vo = isAPropertyOf(vevent, VCPAlarmProp))) {
      if ((a = isAPropertyOf(vo, VCProcedureNameProp))) {
	anEvent->setProgramAlarmFile(s = fakeCString(vObjectUStringZValue(a)));
	deleteStr(s);
      }
    }
    if ((vo = isAPropertyOf(vevent, VCAAlarmProp))) {
      if ((a = isAPropertyOf(vo, VCAudioContentProp))) {
	anEvent->setAudioAlarmFile(s = fakeCString(vObjectUStringZValue(a)));
	deleteStr(s);
      }
    }
  }

  // priority
  if ((vo = isAPropertyOf(vevent, VCPriorityProp))) {
    anEvent->setPriority(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  
  // transparency
  if ((vo = isAPropertyOf(vevent, VCTranspProp)) != 0) {
    anEvent->setTransparency(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }

  // related event
  if ((vo = isAPropertyOf(vevent, VCRelatedToProp)) != 0) {
    anEvent->setRelatedToVUID(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
    mEventsRelate.append(anEvent);
  }

  /* PILOT SYNC STUFF */
  if ((vo = isAPropertyOf(vevent, KPilotIdProp))) {
    anEvent->setPilotId(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    anEvent->setPilotId(0);

  if ((vo = isAPropertyOf(vevent, KPilotStatusProp))) {
    anEvent->setSyncStatus(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    anEvent->setSyncStatus(KOEvent::SYNCMOD);

  return anEvent;
}

QList<KOEvent> CalObject::search(const QRegExp &searchExp) const
{
  QIntDictIterator<QList<KOEvent> > qdi(*calDict);
  const char *testStr;
  QList<KOEvent> matchList, *tmpList, tmpList2;
  KOEvent *matchEvent;

  qdi.toFirst();
  while ((tmpList = qdi.current()) != 0L) {
    ++qdi;
    for (matchEvent = tmpList->first(); matchEvent;
	 matchEvent = tmpList->next()) {
      testStr = matchEvent->getSummary();
      if ((searchExp.match(testStr) != -1) && (matchList.findRef(matchEvent) == -1))
	matchList.append(matchEvent);
      // do other match tests here...
    }
  }

  tmpList2 = recursList;
  tmpList2.setAutoDelete(FALSE); // just to make sure
  for (matchEvent = tmpList2.first(); matchEvent;
       matchEvent = tmpList2.next()) {
    testStr = matchEvent->getSummary();
    if ((searchExp.match(testStr) != -1) && 
	(matchList.findRef(matchEvent) == -1)) 
      matchList.append(matchEvent);
    // do other match tests here...
  }

  // now, we have to sort it based on getDtStart()
  QList<KOEvent> matchListSorted;
  for (matchEvent = matchList.first(); matchEvent; 
       matchEvent = matchList.next()) {
    if (!matchListSorted.isEmpty() &&
        matchEvent->getDtStart() < matchListSorted.at(0)->getDtStart()) {
      matchListSorted.insert(0,matchEvent);
      goto nextToInsert;
    }
    for (int i = 0; (uint) i+1 < matchListSorted.count(); i++) {
      if (matchEvent->getDtStart() > matchListSorted.at(i)->getDtStart() &&
          matchEvent->getDtStart() <= matchListSorted.at(i+1)->getDtStart()) {
        matchListSorted.insert(i+1,matchEvent);
        goto nextToInsert;
      }
    }
    matchListSorted.append(matchEvent);
  nextToInsert:
    continue;
  }

  return matchListSorted;
}

int CalObject::numEvents(const QDate &qd)
{
  QList<KOEvent> *tmpList;
  KOEvent *anEvent;
  int count = 0;
  int extraDays, i;

  // first get the simple case from the dictionary.
  tmpList = calDict->find(makeKey(qd));
  if (tmpList)
    count += tmpList->count();

  // next, check for repeating events.  Even those that span multiple days...
  for (anEvent = recursList.first(); anEvent; anEvent = recursList.next()) {
    if (anEvent->isMultiDay()) {
      extraDays = anEvent->getDtStart().date().daysTo(anEvent->getDtEnd().date());
      //debug("multi day event w/%d days", extraDays);
      for (i = 0; i <= extraDays; i++) {
	if (anEvent->recursOn(qd.addDays(i))) {
	  ++count;
	  break;
	}
      }
    } else {
      if (anEvent->recursOn(qd))
	++count;
    }
  }
  return count;
}

void CalObject::checkAlarms()
{
  QList<KOEvent> alarmEvents;
  QIntDictIterator<QList<KOEvent> > dictIt(*calDict);
  QList<KOEvent> *tmpList;
  KOEvent *anEvent;
  QDateTime tmpDT;

  // this function has to look at every event in the whole database
  // and find if any have an alarm pending.

  while (dictIt.current()) {
    tmpList = dictIt.current();
    for (anEvent = tmpList->first(); anEvent;
	 anEvent = tmpList->next()) {
      tmpDT = anEvent->getAlarmTime();
      if (tmpDT.date() == QDate::currentDate()) {
	if ((tmpDT.time().hour() == QTime::currentTime().hour()) &&
	    (tmpDT.time().minute() == QTime::currentTime().minute()))
	  alarmEvents.append(anEvent);
      }
    }
    ++dictIt;
  }
  for (anEvent = recursList.first(); anEvent;
       anEvent = recursList.next()) {
    tmpDT = anEvent->getAlarmTime();
    if(anEvent->recursOn(QDate::currentDate())) {
      if ((tmpDT.time().hour() == QTime::currentTime().hour()) &&
	  (tmpDT.time().minute() == QTime::currentTime().minute()))
	alarmEvents.append(anEvent);
    }
  }

  if (!alarmEvents.isEmpty())
    emit alarmSignal(alarmEvents);
}

/****************************** PROTECTED METHODS ****************************/
// after changes are made to an event, this should be called.
void CalObject::updateEvent(KOEvent *anEvent)
{
  QIntDictIterator<QList<KOEvent> > qdi(*calDict);
  QList<KOEvent> *tmpList;

  anEvent->setSyncStatus(KOEvent::SYNCMOD);
  anEvent->setLastModified(QDateTime::currentDateTime());
  // we should probably update the revision number here,
  // or internally in the KOEvent itself when certain things change.
  // need to verify with ical documentation.

  // handle sending the event to those attendees that need it.
  // mostly broken right now.
  if (anEvent->attendeeCount()) {
    QList<Attendee> al;
    Attendee *a;
    
    al = anEvent->getAttendeeList();
    for (a = al.first(); a; a = al.next()) {
      if ((a->flag) && (a->RSVP())) {
	//debug("send appointment to %s",a->getName().ascii());
	a->flag = FALSE;
      }
    }
  }
      
  // we don't need to do anything to Todo events.
  if (anEvent->getTodoStatus() != TRUE) {
    // the first thing we do is REMOVE all occurances of the event from 
    // both the dictionary and the recurrence list.  Then we reinsert it.
    // We don't bother about optimizations right now.
    qdi.toFirst();
    while ((tmpList = qdi.current()) != 0L) {
      ++qdi;
      tmpList->removeRef(anEvent);
    }
    // take any instances of it out of the recurrence list
    if (recursList.findRef(anEvent) != -1)
      recursList.take();
    
    // ok the event is now GONE.  we want to re-insert it.
    insertEvent(anEvent);
  }
  emit calUpdated(anEvent);
  return;  
}

// this function will take a VEvent and insert it into the event
// dictionary for the CalObject.  If there is no list of events for that
// particular location in the dictionary, a new one will be created.
void CalObject::insertEvent(const KOEvent *anEvent)
{
  long tmpKey;
  QString tmpDateStr;
  QList<KOEvent> *eventList;
  int extraDays, dayCount;

  // initialize if they haven't been allocated yet;
  if (!oldestDate) {
    oldestDate = new QDate();
    (*oldestDate) = anEvent->getDtStart().date();
  } 
  if (!newestDate) {
    newestDate = new QDate();
    (*newestDate) = anEvent->getDtStart().date();
  }
    
  // update oldest and newest dates if necessary.
  if (anEvent->getDtStart().date() < (*oldestDate))
    (*oldestDate) = anEvent->getDtStart().date();
  if (anEvent->getDtStart().date() > (*newestDate))
    (*newestDate) = anEvent->getDtStart().date();

  if (anEvent->doesRecur()) {
    recursList.append(anEvent);
  } else {
    // set up the key
    extraDays = anEvent->getDtStart().date().daysTo(anEvent->getDtEnd().date());
    for (dayCount = 0; dayCount <= extraDays; dayCount++) {
      tmpKey = makeKey(anEvent->getDtStart().addDays(dayCount));
      // insert the item into the proper list in the dictionary
      if ((eventList = calDict->find(tmpKey)) != 0) {
	eventList->append(anEvent);
      } else {
	// no items under that date yet
	eventList = new QList<KOEvent>;
	eventList->append(anEvent);
	calDict->insert(tmpKey, eventList);
      }
    }
  }
}

// make a long dict key out of a QDateTime
long int CalObject::makeKey(const QDateTime &dt)
{
  QDate tmpD;
  QString tmpStr;

  tmpD = dt.date();
  tmpStr.sprintf("%d%.2d%.2d",tmpD.year(), tmpD.month(), tmpD.day());
//  qDebug("CalObject::makeKey(): %s",tmpStr.latin1());
  return tmpStr.toLong();
}

// make a long dict key out of a QDate
long int CalObject::makeKey(const QDate &d)
{
  QString tmpStr;

  tmpStr.sprintf("%d%.2d%.2d",d.year(), d.month(), d.day());
  return tmpStr.toLong();
}

QDate CalObject::keyToDate(long int key)
{  
  QString dateStr = QString::number(key);
//  qDebug("CalObject::keyToDate(): %s",dateStr.latin1());
  QDate date(dateStr.mid(0,4).toInt(),dateStr.mid(4,2).toInt(),
             dateStr.mid(6,2).toInt());
             
//  qDebug("  QDate: %s",date.toString().latin1());

  return date;
}


// taking a QDate, this function will look for an eventlist in the dict
// with that date attached -
// BL: an the returned list should be deleted!!!
QList<KOEvent> CalObject::getEventsForDate(const QDate &qd, bool sorted)
{
  QList<KOEvent> eventList;
  QList<KOEvent> *tmpList;
  KOEvent *anEvent;
  tmpList = calDict->find(makeKey(qd));
  if (tmpList) {
    for (anEvent = tmpList->first(); anEvent;
	 anEvent = tmpList->next())
      eventList.append(anEvent);
  }
  int extraDays, i;
  for (anEvent = recursList.first(); anEvent; anEvent = recursList.next()) {
    if (anEvent->isMultiDay()) {
      extraDays = anEvent->getDtStart().date().daysTo(anEvent->getDtEnd().date());
      for (i = 0; i <= extraDays; i++) {
	if (anEvent->recursOn(qd.addDays(i))) {
	  eventList.append(anEvent);
	  break;
	}
      }
    } else {
      if (anEvent->recursOn(qd))
	eventList.append(anEvent);
    }
  }
  if (!sorted) {
    updateCursors(eventList.first());
    return eventList;
  }
  //  debug("Sorting getEvents for date\n");
  // now, we have to sort it based on getDtStart.time()
  QList<KOEvent> eventListSorted;
  for (anEvent = eventList.first(); anEvent; anEvent = eventList.next()) {
    if (!eventListSorted.isEmpty() &&
	anEvent->getDtStart().time() < eventListSorted.at(0)->getDtStart().time()) {
      eventListSorted.insert(0,anEvent);
      goto nextToInsert;
    }
    for (i = 0; (uint) i+1 < eventListSorted.count(); i++) {
      if (anEvent->getDtStart().time() > eventListSorted.at(i)->getDtStart().time() &&
	  anEvent->getDtStart().time() <= eventListSorted.at(i+1)->getDtStart().time()) {
	eventListSorted.insert(i+1,anEvent);
	goto nextToInsert;
      }
    }
    eventListSorted.append(anEvent);
  nextToInsert:
    continue;
  }
  updateCursors(eventListSorted.first());
  return eventListSorted;
}


QList<KOEvent> CalObject::getEvents(const QDate &start,const QDate &end,
                                    bool inclusive)
{
  QIntDictIterator<QList<KOEvent> > qdi(*calDict);
  QList<KOEvent> matchList, *tmpList, tmpList2;
  KOEvent *ev = 0;

  qdi.toFirst();

  // Get non-recurring events
  while (qdi.current()) {
    QDate keyDate = keyToDate(qdi.currentKey());
    if (keyDate >= start && keyDate <= end) {
      tmpList = qdi.current();
      for(ev = tmpList->first();ev;ev = tmpList->next()) {
        bool found = false;
        if (ev->isMultiDay()) {  // multi day event
          QDate mStart = ev->getDtStart().date();
          QDate mEnd = ev->getDtEnd().date();

          // Check multi-day events only on one date of its duration, the first
          // date which lies in the specified range.
          if ((mStart >= start && mStart == keyDate) ||
              (mStart < start && start == keyDate)) {
            if (inclusive) {
              if (mStart >= start && mEnd <= end) {
                // Event is completely included in range
                found = true;
              }
            } else {
              // Multi-day event has a day in the range
              found = true;
            }
          }
        } else {  // single day event
          found = true;
        }
        if (found) matchList.append(ev);
      }
    }
    ++qdi;
  }

  // Get recurring events
  for(ev = recursList.first();ev;ev = recursList.next()) {
    QDate rStart = ev->getDtStart().date();
    bool found = false;
    if (inclusive) {
      if (rStart >= start && rStart <= end) {
        // Start date of event is in range. Now check for end date.
        // if duration is negative, event recurs forever, so do not include it.
        if (ev->getRecursDuration() == 0) {  // End date set
          QDate rEnd = ev->getRecursEndDate();
          if (rEnd >= start && rEnd <= end) {  // End date within range
            found = true;
          }
        } else if (ev->getRecursDuration() > 0) {  // Duration set
          // TODO: Calculate end date from duration. Should be done in KOEvent
          // For now exclude all events with a duration.
        }
      }
    } else {
      if (rStart <= end) {  // Start date not after range
        if (rStart >= start) {  // Start date within range
          found = true;
        } else if (ev->getRecursDuration() == -1) {  // Recurs forever
          found = true;
        } else if (ev->getRecursDuration() == 0) {  // End date set
          QDate rEnd = ev->getRecursEndDate();
          if (rEnd >= start && rEnd <= end) {  // End date within range
            found = true;
          }
        } else {  // Duration set
          // TODO: Calculate end date from duration. Should be done in KOEvent
          // For now include all events with a duration.
          found = true;
        }
      }
    }

    if (found) matchList.append(ev);
  }

  return matchList;

#if 0
  tmpList2 = recursList;
  tmpList2.setAutoDelete(FALSE); // just to make sure
  for (matchEvent = tmpList2.first(); matchEvent;
       matchEvent = tmpList2.next()) {
    testStr = matchEvent->getSummary();
    if ((searchExp.match(testStr) != -1) && 
	(matchList.findRef(matchEvent) == -1)) 
      matchList.append(matchEvent);
    // do other match tests here...
  }

#endif

#if 0
  QIntDictIterator<QList<KOEvent>> it(calDict );

  while ( it.current() ) {
    
    printf( "%d -> %s\n", it.currentKey(), it.current() );
    ++it;
  }



  QList<KOEvent> eventList;
  QList<KOEvent> *tmpList;
  KOEvent *anEvent;
  tmpList = calDict->find(makeKey(qd));
  if (tmpList) {
    for (anEvent = tmpList->first(); anEvent;
	 anEvent = tmpList->next())
      eventList.append(anEvent);
  }
  int extraDays, i;
  for (anEvent = recursList.first(); anEvent; anEvent = recursList.next()) {
    if (anEvent->isMultiDay()) {
      extraDays = anEvent->getDtStart().date().daysTo(anEvent->getDtEnd().date());
      for (i = 0; i <= extraDays; i++) {
	if (anEvent->recursOn(qd.addDays(i))) {
	  eventList.append(anEvent);
	  break;
	}
      }
    } else {
      if (anEvent->recursOn(qd))
	eventList.append(anEvent);
    }
  }

  updateCursors(eventList.first());
  return eventList;
#endif
}

// taking a QDateTime, this function will look for an eventlist in the dict
// with that date attached.
// this list is dynamically allocated and SHOULD BE DELETED when done with!
QList<KOEvent> CalObject::getEventsForDate(const QDateTime &qdt)
{
  return getEventsForDate(qdt.date());
}

QString CalObject::getHolidayForDate(const QDate &qd)
{
  static int lastYear = 0;

//  qDebug("CalObject::getHolidayForDate(): Holiday: %s",holidays.latin1());

  if (mHolidayfile.isEmpty()) return (QString(""));

  if ((lastYear == 0) || (qd.year() != lastYear)) {
      lastYear = qd.year() - 1900; // silly parse_year takes 2 digit year...
    parse_holidays(mHolidayfile.latin1(), lastYear, 0);
  }

  if (holiday[qd.dayOfYear()-1].string) {
    QString holidayname = QString(holiday[qd.dayOfYear()-1].string);
//    qDebug("Holi name: %s",holidayname.latin1());
    return(holidayname);
  } else {
//    qDebug("No holiday");
    return(QString(""));
  }
}

void CalObject::updateCursors(KOEvent *dEvent)
{
  if (!dEvent)
    return;
  
  QDate newDate(dEvent->getDtStart().date());
  cursorDate = newDate;

  if (calDict->isEmpty() && recursList.isEmpty())
    return;
  
  if (cursor && cursor->current() && 
      (newDate == cursor->current()->getDtStart().date()))
    return;
  
  if (cursor) {
    delete cursor;
    cursor = 0L;
  }
  QList<KOEvent> *tmpList;
  // we have to check tmpList->count(), because sometimes there are
  // empty lists in the dictionary (they had events once, but they
  // have all been deleted from that date)
  if ((tmpList = calDict->find(makeKey(newDate))) && 
      tmpList->count()) {
    cursor = new QListIterator<KOEvent> (*tmpList);
    cursor->toFirst();
    while (cursor->current() && 
	   (cursor->current() != dEvent)) 
      ++(*cursor);
    if (cursor->current()) {
      return;
    }
  }

  // the little shit is in the recurrence list, or nonexistent.
  if (!recursCursor.current())
    // there's nothing in the recurrence list...
    return;
  if (recursCursor.current()->recursOn(newDate))
    // we are already there...
    return;
  // try to find something in the recurrence list that matches new date.
  recursCursor.toFirst();
  while (recursCursor.current() && 
	 recursCursor.current() != dEvent)
    ++recursCursor;
  if (!recursCursor.current())
    // reset to beginning, no events exist on the new date.
    recursCursor.toFirst();
  return;
}

QString CalObject::qDateToISO(const QDate &qd)
{
  QString tmpStr;

  ASSERT(qd.isValid());

  tmpStr.sprintf("%.2d%.2d%.2d",
		 qd.year(), qd.month(), qd.day());
  return tmpStr;
 
}

QString CalObject::qDateTimeToISO(const QDateTime &qdt, bool zulu)
{
  QString tmpStr;

  ASSERT(qdt.date().isValid());
  ASSERT(qdt.time().isValid());
  if (zulu) {
    QDateTime tmpDT(qdt);
    tmpDT = tmpDT.addSecs(60*(-timeZone)); // correct to GMT.
    tmpStr.sprintf("%.2d%.2d%.2dT%.2d%.2d%.2dZ",
		   tmpDT.date().year(), tmpDT.date().month(), 
		   tmpDT.date().day(), tmpDT.time().hour(), 
		   tmpDT.time().minute(), tmpDT.time().second());
  } else {
    tmpStr.sprintf("%.2d%.2d%.2dT%.2d%.2d%.2d",
		   qdt.date().year(), qdt.date().month(), 
		   qdt.date().day(), qdt.time().hour(), 
		   qdt.time().minute(), qdt.time().second());
  }
  return tmpStr;
}

QDateTime CalObject::ISOToQDateTime(const char *dtStr)
{
  QDate tmpDate;
  QTime tmpTime;
  QString tmpStr;
  int year, month, day, hour, minute, second;

  tmpStr = dtStr;
  year = tmpStr.left(4).toInt();
  month = tmpStr.mid(4,2).toInt();
  day = tmpStr.mid(6,2).toInt();
  hour = tmpStr.mid(9,2).toInt();
  minute = tmpStr.mid(11,2).toInt();
  second = tmpStr.mid(13,2).toInt();
  tmpDate.setYMD(year, month, day);
  tmpTime.setHMS(hour, minute, second);
  
  ASSERT(tmpDate.isValid());
  ASSERT(tmpTime.isValid());
  QDateTime tmpDT(tmpDate, tmpTime);
  // correct for GMT if string is in Zulu format
  if (dtStr[strlen(dtStr)-1] == 'Z')
    tmpDT = tmpDT.addSecs(60*timeZone);
  return tmpDT;
}

// take a raw vcalendar (i.e. from a file on disk, clipboard, etc. etc.
// and break it down from it's tree-like format into the dictionary format
// that is used internally in the CalObject.
void CalObject::populate(VObject *vcal)
{
  // this function will populate the caldict dictionary and other event 
  // lists. It turns vevents into KOEvents and then inserts them.

  VObjectIterator i;
  VObject *curVO, *curVOProp;
  KOEvent *anEvent;

  if ((curVO = isAPropertyOf(vcal, ICMethodProp)) != 0) {
    char *methodType = 0;
    methodType = fakeCString(vObjectUStringZValue(curVO));
    if (dialogsOn)
      QMessageBox::information(topWidget, i18n("KOrganizer: iTIP Transaction"),
			       i18n("This calendar is an iTIP transaction of type \"%1\".")
			       .arg(methodType));
    delete methodType;
  }

  // warn the user that we might have trouble reading non-known calendar.
  if ((curVO = isAPropertyOf(vcal, VCProdIdProp)) != 0) {
    char *s = fakeCString(vObjectUStringZValue(curVO));
    if (strcmp(_PRODUCT_ID, s) != 0)
      if (dialogsOn)
	QMessageBox::warning(topWidget, i18n("KOrganizer: Unknown vCalendar Vendor"),
			     i18n("This vCalendar file was not created by KOrganizer\n"
				     "or any other product we support. Loading anyway..."));
    deleteStr(s);
  }
  
  // warn the user we might have trouble reading this unknown version.
  if ((curVO = isAPropertyOf(vcal, VCVersionProp)) != 0) {
    char *s = fakeCString(vObjectUStringZValue(curVO));
    if (strcmp(_VCAL_VERSION, s) != 0)
      if (dialogsOn)
	QMessageBox::warning(topWidget, i18n("KOrganizer: Unknown vCalendar Version"),
			     i18n("This vCalendar file has version %1.\n"
			          "We only support %2.")
         .arg(s).arg(_VCAL_VERSION));
    deleteStr(s);
  }
  
  // set the time zone
  if ((curVO = isAPropertyOf(vcal, VCTimeZoneProp)) != 0) {
    char *s = fakeCString(vObjectUStringZValue(curVO));
    setTimeZone(s);
    deleteStr(s);
  }


  // Store all events with a relatedTo property in a list for post-processing
  mEventsRelate.clear();
  mTodosRelate.clear();

  initPropIterator(&i, vcal);

  // go through all the vobjects in the vcal
  while (moreIteration(&i)) {
    curVO = nextVObject(&i);

    /************************************************************************/

    // now, check to see that the object is an event or todo.
    if (strcmp(vObjectName(curVO), VCEventProp) == 0) {

      if ((curVOProp = isAPropertyOf(curVO, KPilotStatusProp)) != 0) {
	char *s;
	s = fakeCString(vObjectUStringZValue(curVOProp));
	// check to see if event was deleted by the kpilot conduit
	if (atoi(s) == KOEvent::SYNCDEL) {
	  deleteStr(s);
	  debug("skipping pilot-deleted event");
	  goto SKIP;
	}
	deleteStr(s);
      }

      // this code checks to see if we are trying to read in an event
      // that we already find to be in the calendar.  If we find this
      // to be the case, we skip the event.
      if ((curVOProp = isAPropertyOf(curVO, VCUniqueStringProp)) != 0) {
	char *s = fakeCString(vObjectUStringZValue(curVOProp));
	QString tmpStr(s);
	deleteStr(s);

	if (getEvent(tmpStr)) {
	  if (dialogsOn)
	    QMessageBox::warning(topWidget,
				 i18n("KOrganizer: Possible Duplicate Event"),
				 i18n("Aborting merge of an event already in "
				      "calendar.\n"
				      "UID is %1\n"
				      "Please check your vCalendar file for "
				      "duplicate UIDs\n"
				      "and change them MANUALLY to be unique "
				      "if you find any.\n").arg(tmpStr));
	  goto SKIP;
	}
	if (getTodo(tmpStr)) {
	  if (dialogsOn)
	    QMessageBox::warning(topWidget,
				 i18n("KOrganizer: Possible Duplicate Event"),
				 i18n("Aborting merge of an event already in "
				      "calendar.\n"
				      "UID is %1\n"
				      "Please check your vCalendar file for "
				      "duplicate UIDs\n"
				      "and change them MANUALLY to be unique "
				      "if you find any.\n").arg(tmpStr));
	  
	  goto SKIP;
	}
      }

      if ((!(curVOProp = isAPropertyOf(curVO, VCDTstartProp))) &&
	  (!(curVOProp = isAPropertyOf(curVO, VCDTendProp)))) {
	debug("found a VEvent with no DTSTART and no DTEND! Skipping...");
	goto SKIP;
      }

      anEvent = VEventToEvent(curVO);
      // we now use addEvent instead of insertEvent so that the
      // signal/slot get connected.
      if (anEvent)
	addEvent(anEvent);
      else {
	// some sort of error must have occurred while in translation.
	goto SKIP;
      }
    } else if (strcmp(vObjectName(curVO), VCTodoProp) == 0) {
      anEvent = VTodoToEvent(curVO);
      addTodo(anEvent);
    } else if ((strcmp(vObjectName(curVO), VCVersionProp) == 0) ||
	       (strcmp(vObjectName(curVO), VCProdIdProp) == 0) ||
	       (strcmp(vObjectName(curVO), VCTimeZoneProp) == 0)) {
      // do nothing, we know these properties and we want to skip them.
      // we have either already processed them or are ignoring them.
      ;
    } else {
      debug("Ignoring unknown vObject \"%s\"", vObjectName(curVO));
    }
  SKIP:
    ;
  } // while
  
  // Post-Process list of events with relations, put KOEvent objects in relation
  KOEvent *ev;
  for ( ev=mEventsRelate.first(); ev != 0; ev=mEventsRelate.next() ) {
    ev->setRelatedTo(getEvent(ev->getRelatedToVUID()));
  }
  for ( ev=mTodosRelate.first(); ev != 0; ev=mTodosRelate.next() ) {
    ev->setRelatedTo(getTodo(ev->getRelatedToVUID()));
  }
}

const char *CalObject::dayFromNum(int day)
{
  const char *days[7] = { "MO ", "TU ", "WE ", "TH ", "FR ", "SA ", "SU " };

  return days[day];
}

int CalObject::numFromDay(const QString &day)
{
  if (day == "MO ") return 0;
  if (day == "TU ") return 1;
  if (day == "WE ") return 2;
  if (day == "TH ") return 3;
  if (day == "FR ") return 4;
  if (day == "SA ") return 5;
  if (day == "SU ") return 6;

  return -1; // something bad happened. :)
}

void CalObject::loadError(const QString &fileName) 
{
  if (dialogsOn)
    QMessageBox::critical(topWidget,
			  i18n("KOrganizer: Error Loading Calendar"),
			  i18n("An error has occurred loading the file:\n"
			       "%1.\n"
			       "Please verify that the file is in vCalendar "
			       "format,\n"
			       "that it exists, and it is readeable, then "
			       "try again or load another file.\n")
			  .arg(fileName));
}

void CalObject::parseError(const char *prop) 
{
  if (dialogsOn)
    QMessageBox::critical(topWidget,
			  i18n("KOrganizer: Error Parsing Calendar"),
			  i18n("An error has occurred parsing this file.\n"
			       "It is missing property \"%1\".\n"
			       "Please verify that the file is in vCalendar "
			       "format\n"
			       "and try again, or load another file.\n")
			  .arg(prop));
}
