// Calendar class for KOrganizer
// (c) 1998 Preston Brown
// 	$Id$

#include "config.h"

#include <qdatetime.h>
#include <qstring.h>
#include <qlist.h>
#include <qregexp.h>
#include <qclipboard.h>
#include <qdialog.h>
#include <qfile.h>

#include <kapp.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kiconloader.h>
#include <klocale.h>

extern "C" {
#include <icalparser.h>
#include <icalrestriction.h>
}


#include "qdatelist.h"
#include "calobject.h"

#include "icalformat.h"

#define _ICAL_VERSION "2.0"

ICalFormat::ICalFormat(CalObject *cal) :
  CalFormat(cal)
{
}

ICalFormat::~ICalFormat()
{
}

bool ICalFormat::load(const QString &fileName)
{
  kdDebug() << "ICalFormat::load()" << endl;

  icalfileset *fs = icalfileset_new(writeText(fileName));

  if (!fs) {
    loadError(fileName);
    return false;
  }

  // Get first VCALENDAR component.
  // TODO: Handle more than one VCALENDAR or non-VCALENDAR top components
  icalcomponent *calendar;
  calendar = icalfileset_get_first_component(fs);

  while(calendar) {
    if (icalcomponent_isa(calendar) == ICAL_VCALENDAR_COMPONENT) break;
    calendar = icalfileset_get_next_component(fs);
  }

  if (!calendar) {
    kdDebug("ICalFormat::load(): No VCALENDAR component found");
    icalfileset_free(fs);
    return false;
  }

  // put all vobjects into their proper places
  populate(calendar);

  icalfileset_free(fs);

  return true;

#if 0
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
  mCalendar->first(); 

  return TRUE;
#endif
}


bool ICalFormat::save(const QString &fileName)
{
  kdDebug() << "ICalFormat::save()" << endl;

  // Remove file. icalfileset does not seem to do this.
  QFile::remove(fileName);

  // Create iCalendar file
  icalfileset *fs = icalfileset_new(writeText(fileName));

  icalcomponent *calendar = createCalendarComponent();
  icalfileset_add_component(fs,calendar);
  
  icalcomponent *component;

  // TODO STUFF
  QList<KOEvent> todoList = mCalendar->getTodoList();
  QListIterator<KOEvent> qlt(todoList);
  for (; qlt.current(); ++qlt) {
    component = writeTodo(qlt.current());
    icalcomponent_add_component(calendar,component);
  }  

  // EVENT STUFF
  QList<KOEvent> events = mCalendar->getAllEvents();
  KOEvent *ev;
  for(ev=events.first();ev;ev=events.next()) {
    component = writeEvent(ev);
    icalcomponent_add_component(calendar,component);
  }

  // Write out iCalendar file
  icalfileset_mark(fs);
  icalfileset_commit(fs);
  icalfileset_free(fs);

  return true;
}

// Disabled until iCalendar drag and drop is implemented
VCalDrag *ICalFormat::createDrag(KOEvent *selectedEv, QWidget *owner)
{
  return 0;
#if 0
  VObject *vcal, *vevent;
  QString tmpStr;
  
  vcal = newVObject(VCCalProp);
  
  addPropValue(vcal,VCProdIdProp, _PRODUCT_ID);
  tmpStr = mCalendar->getTimeZoneStr();
  addPropValue(vcal,VCTimeZoneProp, tmpStr.latin1());
  addPropValue(vcal,VCVersionProp, _VCAL_VERSION);
  
  vevent = eventToVEvent(selectedEv);
  
  addVObjectProp(vcal, vevent);

  VCalDrag *vcd = new VCalDrag(vcal, owner);
  // free memory associated with vCalendar stuff
  cleanVObject(vcal);  
  vcd->setPixmap(BarIcon("appointment"));

  return vcd;
#endif
}

VCalDrag *ICalFormat::createDragTodo(KOEvent *selectedEv, QWidget *owner)
{
  return 0;
#if 0
  VObject *vcal, *vevent;
  QString tmpStr;
  
  vcal = newVObject(VCCalProp);
  
  addPropValue(vcal,VCProdIdProp, _PRODUCT_ID);
  tmpStr = mCalendar->getTimeZoneStr();
  addPropValue(vcal,VCTimeZoneProp, tmpStr.latin1());
  addPropValue(vcal,VCVersionProp, _VCAL_VERSION);
  
  vevent = eventToVTodo(selectedEv);
  
  addVObjectProp(vcal, vevent);

  VCalDrag *vcd = new VCalDrag(vcal, owner);
  // free memory associated with vCalendar stuff
  cleanVObject(vcal);  
  vcd->setPixmap(BarIcon("todo"));

  return vcd;
#endif
}

KOEvent *ICalFormat::createDrop(QDropEvent *de)
{
  return 0;
#if 0
  VObject *vcal;
  KOEvent *event = 0;

  if (VCalDrag::decode(de, &vcal)) {
    de->accept();
    VObjectIterator i;
    VObject *curvo;
    initPropIterator(&i, vcal);
    
    // we only take the first object.
    do  {
      curvo = nextVObject(&i);
    } while (strcmp(vObjectName(curvo), VCEventProp) &&
             strcmp(vObjectName(curvo), VCTodoProp));

    if (strcmp(vObjectName(curvo), VCTodoProp) == 0) {
      kdDebug() << "ICalFormat::createDrop(): Got todo instead of event." << endl;
    } else if (strcmp(vObjectName(curvo), VCEventProp) == 0) {
      event = VEventToEvent(curvo);
    } else {
      kdDebug() << "ICalFormat::createDropTodo(): Unknown event type in drop." << endl;
    }
    // get rid of temporary VObject
    deleteVObject(vcal);
  }
  
  return event;
#endif
}

KOEvent *ICalFormat::createDropTodo(QDropEvent *de)
{
  return 0;
#if 0
  VObject *vcal;
  KOEvent *event = 0;

  if (VCalDrag::decode(de, &vcal)) {
    de->accept();
    VObjectIterator i;
    VObject *curvo;
    initPropIterator(&i, vcal);
    
    // we only take the first object.
    do  {
      curvo = nextVObject(&i);
    } while (strcmp(vObjectName(curvo), VCEventProp) &&
             strcmp(vObjectName(curvo), VCTodoProp));

    if (strcmp(vObjectName(curvo), VCEventProp) == 0) {
      kdDebug() << "ICalFormat::createDropTodo(): Got event instead of todo." << endl;
    } else if (strcmp(vObjectName(curvo), VCTodoProp) == 0) {
      event = VTodoToEvent(curvo);
    } else {
      kdDebug() << "ICalFormat::createDropTodo(): Unknown event type in drop." << endl;
    }
    // get rid of temporary VObject
    deleteVObject(vcal);
  }
  
  return event;
#endif
}

bool ICalFormat::copyEvent(KOEvent *selectedEv)
{
  return false;
#if 0
  QClipboard *cb = QApplication::clipboard();
  VObject *vcal, *vevent;
  QString tmpStr;

  vcal = newVObject(VCCalProp);

  //  addPropValue(vcal,VCLocationProp, "0.0");
  addPropValue(vcal,VCProdIdProp, _PRODUCT_ID);
  tmpStr = mCalendar->getTimeZoneStr();
  addPropValue(vcal,VCTimeZoneProp, tmpStr.ascii());
  addPropValue(vcal,VCVersionProp, _VCAL_VERSION);

  vevent = eventToVEvent(selectedEv);

  addVObjectProp(vcal, vevent);

  // paste to clipboard
  cb->setData(new VCalDrag(vcal));
  
  // free memory associated with vCalendar stuff
  cleanVObject(vcal);
  
  return TRUE;
#endif
}

KOEvent *ICalFormat::pasteEvent(const QDate *newDate, 
				const QTime *newTime)
{
  return 0;
#if 0
  VObject *vcal, *curVO, *curVOProp;
  VObjectIterator i;
  int daysOffset;

  KOEvent *anEvent = 0L;

  QClipboard *cb = QApplication::clipboard();
  int bufsize;
  const char * buf;
  buf = cb->text().ascii();
  bufsize = strlen(buf);

  if (!VCalDrag::decode(cb->data(),&vcal)) {
    if (mEnableDialogs) {
      KMessageBox::sorry(mTopWidget, 
                            i18n("An error has occurred parsing the "
                                 "contents of the clipboard.\nYou can "
                                 "only paste a valid vCalendar into "
                                 "KOrganizer.\n"));
      return 0;
    }
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
      QString uidStr = createUniqueId();
      if (mCalendar->getEvent(anEvent->VUID()))
	anEvent->setVUID(uidStr);

      daysOffset = anEvent->getDtEnd().date().dayOfYear() - 
	anEvent->getDtStart().date().dayOfYear();
      
      if (newTime)
	anEvent->setDtStart(QDateTime(*newDate, *newTime));
      else
	anEvent->setDtStart(QDateTime(*newDate, anEvent->getDtStart().time()));
      
      anEvent->setDtEnd(QDateTime(newDate->addDays(daysOffset),
				  anEvent->getDtEnd().time()));
      mCalendar->addEvent(anEvent);
    } else {
      kdDebug() << "found a VEvent with no DTSTART/DTEND! Skipping" << endl;
    }
  } else if (strcmp(vObjectName(curVO), VCTodoProp) == 0) {
    anEvent = VTodoToEvent(curVO);
    mCalendar->addTodo(anEvent);
  } else {
    kdDebug() << "unknown event type in paste!!!" << endl;
  }
  // get rid of temporary VObject
  deleteVObject(vcal);
  return anEvent;
#endif
}

icalcomponent *ICalFormat::createScheduleComponent(KOEvent *incidence,
                                                   Scheduler::Method method)
{
  icalcomponent *message = createCalendarComponent();

  icalproperty_method icalmethod = ICAL_METHOD_NONE;
  
  switch (method) {
    case Scheduler::Publish:
      icalmethod = ICAL_METHOD_PUBLISH;
      break;
    case Scheduler::Request:
      icalmethod = ICAL_METHOD_REQUEST;
      break;
    case Scheduler::Refresh:
      icalmethod = ICAL_METHOD_REFRESH;
      break;
    case Scheduler::Cancel:
      icalmethod = ICAL_METHOD_CANCEL;
      break;
    case Scheduler::Add:
      icalmethod = ICAL_METHOD_ADD;
      break;
    case Scheduler::Reply:
      icalmethod = ICAL_METHOD_REPLY;
      break;
    case Scheduler::Counter:
      icalmethod = ICAL_METHOD_COUNTER;
      break;
    case Scheduler::Declinecounter:
      icalmethod = ICAL_METHOD_DECLINECOUNTER;
      break;
    default:
      kdDebug() << "ICalFormat::createScheduleMessage(): Unknow method" << endl;
      return message;
  }

  icalcomponent_add_property(message,icalproperty_new_method(icalmethod));

  if (incidence->getTodoStatus()) {
    icalcomponent_add_component(message,writeTodo(incidence));
  } else {
    icalcomponent_add_component(message,writeEvent(incidence));
  }

  return message;
}

QString ICalFormat::createScheduleMessage(KOEvent *incidence,
                                          Scheduler::Method method)
{
  icalcomponent *message = createScheduleComponent(incidence,method);

  QString messageText = icalcomponent_as_ical_string(message);

#if 0
  kdDebug() << "ICalFormat::createScheduleMessage: message START\n"
            << messageText
            << "ICalFormat::createScheduleMessage: message END" << endl;
#endif

  return messageText;
}

ScheduleMessage *ICalFormat::parseScheduleMessage(const QString &messageText)
{
  clearException();

  icalcomponent *message;
  message = icalparser_parse_string(writeText(messageText));
  
  if (!message) return 0;
  
  icalproperty *m = icalcomponent_get_first_property(message,
                                                     ICAL_METHOD_PROPERTY);
                                                          
  if (!m) return 0;
  
  icalcomponent *c;
  
  KOEvent *incidence = 0;
  c = icalcomponent_get_first_component(message,ICAL_VEVENT_COMPONENT);
  if (c) {
    incidence = readEvent(c);
  } else {
    c = icalcomponent_get_first_component(message,ICAL_VTODO_COMPONENT);
    if (c) {
      incidence = readTodo(c);
    }
  }

  if (!incidence) return 0;

  kdDebug() << "ICalFormat::parseScheduleMessage() getting method..." << endl;

  icalproperty_method icalmethod = icalproperty_get_method(m);
  Scheduler::Method method;
  
  switch (icalmethod) {
    case ICAL_METHOD_PUBLISH:
      method = Scheduler::Publish;
      break;
    case ICAL_METHOD_REQUEST:
      method = Scheduler::Request;
      break;
    case ICAL_METHOD_REFRESH:
      method = Scheduler::Refresh;
      break;
    case ICAL_METHOD_CANCEL:
      method = Scheduler::Cancel;
      break;
    case ICAL_METHOD_ADD:
      method = Scheduler::Add;
      break;
    case ICAL_METHOD_REPLY:
      method = Scheduler::Reply;
      break;
    case ICAL_METHOD_COUNTER:
      method = Scheduler::Counter;
      break;
    case ICAL_METHOD_DECLINECOUNTER:
      method = Scheduler::Declinecounter;
      break;
    default:
      method = Scheduler::NoMethod;
      kdDebug() << "ICalFormat::parseScheduleMessage(): Unknow method" << endl;
      break;
  }

  kdDebug() << "ICalFormat::parseScheduleMessage() restriction..." << endl;

  if (!icalrestriction_check(message)) {
    setException(new KOErrorFormat(KOErrorFormat::Restriction,
                                   Scheduler::methodName(method) + ": " +
                                   extractErrorProperty(c)));
    return 0;
  }
  
  icalcomponent *calendarComponent = createCalendarComponent();

  KOEvent *existingIncidence = mCalendar->getEvent(incidence->VUID());
  if (existingIncidence) {
    if (existingIncidence->getTodoStatus()) {
      icalcomponent_add_component(calendarComponent,
                                  writeEvent(existingIncidence));
    } else {
      icalcomponent_add_component(calendarComponent,
                                  writeTodo(existingIncidence));
    }
  } else {
    calendarComponent = 0;
  }

  kdDebug() << "ICalFormat::parseScheduleMessage() classify..." << endl;

  icalclass result = icalclassify(message,calendarComponent,(char *)"");
  
  kdDebug() << "ICalFormat::parseScheduleMessage() returning..." << endl;

  return new ScheduleMessage(incidence,method,result);
}

icalcomponent *ICalFormat::writeTodo(KOEvent *todo)
{
  QString tmpStr;
  QStringList tmpStrList;

  icalcomponent *vtodo = icalcomponent_new(ICAL_VTODO_COMPONENT);

  writeIncidence(vtodo,todo);

  // due date
  if (todo->hasDueDate()) {
    icaltimetype due;
    if (todo->doesFloat()) {
      due = writeICalDate(todo->getDtDue().date());
    } else {
      due = writeICalDateTime(todo->getDtDue());
    }
    icalcomponent_add_property(vtodo,icalproperty_new_due(due));
  }
  
  return vtodo;
}

icalcomponent *ICalFormat::writeEvent(KOEvent *event)
{
  QString tmpStr;
  QStringList tmpStrList;

  icalcomponent *vevent = icalcomponent_new(ICAL_VEVENT_COMPONENT);

  writeIncidence(vevent,event);

  // start and end time
  icaltimetype start,end;
  if (event->doesFloat()) {
    start = writeICalDate(event->getDtStart().date());
    end = writeICalDate(event->getDtEnd().date());
  } else {
    start = writeICalDateTime(event->getDtStart());
    end = writeICalDateTime(event->getDtEnd());
  }
  icalcomponent_add_property(vevent,icalproperty_new_dtstart(start));
  icalcomponent_add_property(vevent,icalproperty_new_dtend(end));

// TODO: recurrence
#if 0
  // recurrence rule stuff
  if (anEvent->doesRecur()) {
    icalcomponent_add_property(vevent,writeRecurrencRule(event));
  }
#endif

// TODO: exdates
#if 0
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
#endif

// TODO: find correspondence to secrecy in iCal
  // secrecy
//  addPropValue(vevent, VCClassProp, anEvent->getSecrecyStr().ascii());


// TODO: attachements, resources, alarm
#if 0
  // attachments
  tmpStrList = anEvent->attachments();
  for ( QStringList::Iterator it = tmpStrList.begin();
        it != tmpStrList.end();
        ++it )
    addPropValue(vevent, VCAttachProp, (*it).ascii());
  
  // resources
  tmpStrList = anEvent->resources();
  tmpStr = tmpStrList.join(";");
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
#endif

// TODO: transparency
  // transparency
//  tmpStr.sprintf("%i",anEvent->getTransparency());
//  addPropValue(vevent, VCTranspProp, tmpStr.ascii());
  
  return vevent;
}

void ICalFormat::writeIncidence(icalcomponent *parent,KOEvent *incidence)
{
  // creation date
  icalcomponent_add_property(parent,icalproperty_new_created(
      writeICalDateTime(incidence->created())));

  // unique id
  icalcomponent_add_property(parent,icalproperty_new_uid(
      writeText(incidence->VUID())));

  // revision
  icalcomponent_add_property(parent,icalproperty_new_sequence(
      incidence->revision()));

  // last modification date
  icalcomponent_add_property(parent,icalproperty_new_lastmodified(
      writeICalDateTime(incidence->getLastModified())));

  icalcomponent_add_property(parent,icalproperty_new_dtstamp(
      writeICalDateTime(QDateTime::currentDateTime())));

  // organizer stuff
  icalcomponent_add_property(parent,icalproperty_new_organizer(
      writeText("MAILTO:" + incidence->getOrganizer())));

  // attendees
  if (incidence->attendeeCount() != 0) {
    QList<Attendee> al = incidence->getAttendeeList();
    QListIterator<Attendee> ai(al);
    Attendee *curAttendee;
    
    QString attendee;
    for (; ai.current(); ++ai) {
      curAttendee = ai.current();
      if (!curAttendee->getEmail().isEmpty() &&
          !curAttendee->getName().isEmpty()) {
        attendee = curAttendee->getName() + " <" + curAttendee->getEmail() +
                   ">";
      } else if (curAttendee->getName().isEmpty()) {
        attendee = curAttendee->getEmail();
      } else if (curAttendee->getEmail().isEmpty()) {
        attendee = curAttendee->getName();
      } else if (curAttendee->getName().isEmpty() && 
	         curAttendee->getEmail().isEmpty()) {
        attendee = "";
	kdDebug() << "warning! this koevent has an attendee w/o name or email!"
                  << endl;
      } else {
        attendee = "";
      }
      
      icalproperty *p = icalproperty_new_attendee(
          writeText("MAILTO:" + attendee));
      icalproperty_add_parameter(p,icalparameter_new_rsvp(
          curAttendee->RSVP()));
// TODO: attendee status
//      addPropValue(aProp, VCStatusProp, curAttendee->getStatusStr().ascii());

      icalcomponent_add_property(parent,p);
    }
  }

  // description
  if (!incidence->getDescription().isEmpty()) {
    icalcomponent_add_property(parent,icalproperty_new_description(
        writeText(incidence->getDescription())));
// TODO:
//    if (incidence->getDescription().find('\n') != -1)
//      addProp(d, VCQuotedPrintableProp);
  }

  // summary
  if (!incidence->getSummary().isEmpty()) {
    icalcomponent_add_property(parent,icalproperty_new_summary(
        writeText(incidence->getSummary())));
  }

// TODO:
  // status
//  addPropValue(parent, VCStatusProp, incidence->getStatusStr().ascii());

  // priority
  icalcomponent_add_property(parent,icalproperty_new_priority(
      incidence->getPriority()));

  // categories
  QStringList categories = incidence->getCategories();
  QStringList::Iterator it;
  for(it = categories.begin(); it != categories.end(); ++it ) {
    icalcomponent_add_property(parent,icalproperty_new_categories(
        writeText(*it)));
  }
// TODO: Ensure correct concatenation of categories properties.

/*
  // categories
  tmpStrList = incidence->getCategories();
  tmpStr = "";
  QString catStr;
  for ( QStringList::Iterator it = tmpStrList.begin();
        it != tmpStrList.end();
        ++it ) {
    catStr = *it;
    if (catStr[0] == ' ')
      tmpStr += catStr.mid(1);
    else
      tmpStr += catStr;
    // this must be a ';' character as the vCalendar specification requires!
    // vcc.y has been hacked to translate the ';' to a ',' when the vcal is
    // read in.
    tmpStr += ";";
  }
  if (!tmpStr.isEmpty()) {
    tmpStr.truncate(tmpStr.length()-1);
    icalcomponent_add_property(parent,icalproperty_new_categories(
        writeText(incidence->getCategories().join(";"))));
  }
*/

  // related event
  if (incidence->getRelatedTo()) {
    icalcomponent_add_property(parent,icalproperty_new_relatedto(
        writeText(incidence->getRelatedTo()->VUID())));
  }

// TODO:
  // pilot sync stuff
#if 0
  tmpStr.sprintf("%i",incidence->getPilotId());
  addPropValue(parent, KPilotIdProp, tmpStr.ascii());
  tmpStr.sprintf("%i",incidence->getSyncStatus());
  addPropValue(parent, KPilotStatusProp, tmpStr.ascii());
#endif

}

icalproperty *writeRecurrenceRule(KOEvent *event)
{
#if 0
    // some more variables
    QList<KOEvent::rMonthPos> tmpPositions;
    QList<int> tmpDays;
    int *tmpDay;
    KOEvent::rMonthPos *tmpPos;
    QString tmpStr2;

    switch(anEvent->doesRecur()) {
    case KOEvent::rDaily:
      tmpStr.sprintf("D%i ",anEvent->rFreq);
//      if (anEvent->rDuration > 0)
//	tmpStr += "#";
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
      kdDebug() << "ERROR, it should never get here in eventToVEvent!" << endl;
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
#endif
}

KOEvent *ICalFormat::readTodo(icalcomponent *vtodo)
{
  KOEvent *todo = new KOEvent;
  todo->setTodoStatus(true);

  readIncidence(vtodo,todo);

  icalproperty *p = icalcomponent_get_first_property(vtodo,ICAL_ANY_PROPERTY);

  const char *text;
  int intvalue;
  icaltimetype icaltime;

  QStringList categories;

  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_DUE_PROPERTY:  // due date
        icaltime = icalproperty_get_due(p);
        if (icaltime.is_date) {
          todo->setDtDue(QDateTime(readICalDate(icaltime),QTime(0,0,0)));
          todo->setFloats(true);
        } else {
          todo->setDtDue(readICalDateTime(icaltime));
          todo->setFloats(false);
        }
        todo->setHasDueDate(true);
        break;

      case ICAL_RELATEDTO_PROPERTY:  // releated todo (parent)
        text = icalproperty_get_relatedto(p);
        todo->setRelatedToVUID(text);
        mTodosRelate.append(todo);
        break;

      default:
//        kdDebug() << "ICALFormat::readTodo(): Unknown property: " << kind
//                  << endl;
        break;
    }

    p = icalcomponent_get_next_property(vtodo,ICAL_ANY_PROPERTY);
  }

  return todo;
}

KOEvent *ICalFormat::readEvent(icalcomponent *vevent)
{
  KOEvent *event = new KOEvent;
  event->setFloats(false);

  readIncidence(vevent,event);  

  icalproperty *p = icalcomponent_get_first_property(vevent,ICAL_ANY_PROPERTY);

  const char *text;
  int intvalue;
  icaltimetype icaltime;

  QStringList categories;

  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_DTSTART_PROPERTY:  // start date and time
        icaltime = icalproperty_get_dtstart(p);
        if (icaltime.is_date) {
          event->setDtStart(QDateTime(readICalDate(icaltime),QTime(0,0,0)));
          event->setFloats(true);
        } else {
          event->setDtStart(readICalDateTime(icaltime));
        }
        break;

      case ICAL_DTEND_PROPERTY:  // start date and time
        icaltime = icalproperty_get_dtend(p);
        if (icaltime.is_date) {
          event->setDtEnd(QDateTime(readICalDate(icaltime),QTime(0,0,0)));
          event->setFloats(true);
        } else {
          event->setDtEnd(readICalDateTime(icaltime));
        }
        break;

// TODO:  
  // at this point, there should be at least a start or end time.
  // fix up for events that take up no time but have a time associated
#if 0
  if (!(vo = isAPropertyOf(vevent, VCDTstartProp)))
    anEvent->setDtStart(anEvent->getDtEnd());
  if (!(vo = isAPropertyOf(vevent, VCDTendProp)))
    anEvent->setDtEnd(anEvent->getDtStart());
#endif
  
      case ICAL_RRULE_PROPERTY:
        readRecurrenceRule(p,event);
        break;

// TODO: exdates
#if 0
  // recurrence exceptions
  if ((vo = isAPropertyOf(vevent, VCExDateProp)) != 0) {
    anEvent->setExDates(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  }
#endif

#if 0
  // secrecy
  if ((vo = isAPropertyOf(vevent, VCClassProp)) != 0) {
    anEvent->setSecrecy(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  }
  else
    anEvent->setSecrecy("PUBLIC");

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
    QString resource;
    while ((index2 = resources.find(';', index1)) != -1) {
      resource = resources.mid(index1, (index2 - index1));
      tmpStrList.append(resource);
      index1 = index2;
    }
    anEvent->setResources(tmpStrList);
  }
#endif

// TODO: read alarms
#if 0
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
#endif

// TODO: transparency
#if 0
  // transparency
  if ((vo = isAPropertyOf(vevent, VCTranspProp)) != 0) {
    anEvent->setTransparency(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
#endif

      case ICAL_RELATEDTO_PROPERTY:  // releated event (parent)
        text = icalproperty_get_relatedto(p);
        event->setRelatedToVUID(text);
        mEventsRelate.append(event);
        break;

      default:
//        kdDebug() << "ICALFormat::readEvent(): Unknown property: " << kind
//                  << endl;
        break;
    }

    p = icalcomponent_get_next_property(vevent,ICAL_ANY_PROPERTY);
  }

  // some stupid vCal exporters ignore the standard and use Description
  // instead of Summary for the default field.  Correct for this.
  if (event->getSummary().isEmpty() && 
      !(event->getDescription().isEmpty())) {
    QString tmpStr = event->getDescription().simplifyWhiteSpace();
    event->setDescription("");
    event->setSummary(tmpStr);
  }

  return event;
}

Attendee *ICalFormat::readAttendee(icalproperty *attendee)
{
  Attendee *a;

  QString text = icalproperty_get_attendee(attendee);
  
  text = text.simplifyWhiteSpace();
  int emailPos1, emailPos2;
  if ((emailPos1 = text.find('<')) > 0) {
    // both email address and name
    emailPos2 = text.find('>');
    a = new Attendee(text.left(emailPos1 - 1),
                     text.mid(emailPos1 + 1, 
                     emailPos2 - (emailPos1 + 1)));
  } else if (text.find('@') > 0) {
    // just an email address
    a = new Attendee(0, text);
  } else {
    // just a name
    a = new Attendee(text);
  }

  icalparameter *p = icalproperty_get_first_parameter(attendee,
                                                      ICAL_RSVP_PARAMETER);
  if (p) {
    a->setRSVP(icalparameter_get_rsvp(p));
  }

// TODO: attendee status
#if 0  
      // is there a status property?
      if ((vp = isAPropertyOf(vo, VCStatusProp)) != 0)
	a->setStatus(vObjectStringZValue(vp));
#endif

  return a;
}

void ICalFormat::readIncidence(icalcomponent *parent,KOEvent *incidence)
{
  icalproperty *p = icalcomponent_get_first_property(parent,ICAL_ANY_PROPERTY);

  const char *text;
  int intvalue;
  icaltimetype icaltime;

  QStringList categories;

  while (p) {
    icalproperty_kind kind = icalproperty_isa(p);
    switch (kind) {

      case ICAL_CREATED_PROPERTY:
        icaltime = icalproperty_get_created(p);
        incidence->setCreated(readICalDateTime(icaltime));
        break;

      case ICAL_UID_PROPERTY:  // unique id
        text = icalproperty_get_uid(p);
        incidence->setVUID(text);
        break;

      case ICAL_SEQUENCE_PROPERTY:  // sequence
        intvalue = icalproperty_get_sequence(p);
        incidence->setRevision(intvalue);
        break;

      case ICAL_LASTMODIFIED_PROPERTY:  // last modification date
        icaltime = icalproperty_get_lastmodified(p);
        incidence->setLastModified(readICalDateTime(icaltime));
        break;

      case ICAL_ORGANIZER_PROPERTY:  // organizer
        text = icalproperty_get_organizer(p);
        incidence->setOrganizer(text);
        break;

      case ICAL_ATTENDEE_PROPERTY:  // attendee
        incidence->addAttendee(readAttendee(p));
        break;

      case ICAL_DESCRIPTION_PROPERTY:  // description
        text = icalproperty_get_description(p);
        incidence->setDescription(text);
        break;
 
      case ICAL_SUMMARY_PROPERTY:  // summary
        text = icalproperty_get_summary(p);
        incidence->setSummary(text);
        break;

#if 0  
  // status
  if ((vo = isAPropertyOf(vincidence, VCStatusProp)) != 0) {
    incidence->setStatus(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
  }
  else
    incidence->setStatus("NEEDS ACTION");
#endif
  
      case ICAL_PRIORITY_PROPERTY:  // priority
        intvalue = icalproperty_get_priority(p);
        incidence->setPriority(intvalue);
        break;

      case ICAL_CATEGORIES_PROPERTY:  // categories
        text = icalproperty_get_categories(p);
        categories.append(text);
        break;

// TODO:
#if 0
  /* PILOT SYNC STUFF */
  if ((vo = isAPropertyOf(vincidence, KPilotIdProp))) {
    incidence->setPilotId(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    incidence->setPilotId(0);

  if ((vo = isAPropertyOf(vincidence, KPilotStatusProp))) {
    incidence->setSyncStatus(atoi(s = fakeCString(vObjectUStringZValue(vo))));
    deleteStr(s);
  }
  else
    incidence->setSyncStatus(KOEvent::SYNCMOD);
#endif

      default:
//        kdDebug() << "ICALFormat::readIncidence(): Unknown property: " << kind
//                  << endl;
        break;
    }

    p = icalcomponent_get_next_property(parent,ICAL_ANY_PROPERTY);
  }

  // add categories
  incidence->setCategories(categories);
}

void ICalFormat::readRecurrenceRule(icalproperty *rrule,KOEvent *event)
{
#if 0
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
	QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length()-index))).date();
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
	QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length()-index))).date();
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
						    index))).date();
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
	QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length()-index))).date();
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
	QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length()-index))).date();
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
	QDate rEndDate = (ISOToQDateTime(tmpStr.mid(index, tmpStr.length()-index))).date();
	anEvent->setRecursYearly(KOEvent::rYearlyDay, rFreq, rEndDate);
      } else {
	int rDuration = tmpStr.mid(index, tmpStr.length()-index).toInt();
	if (rDuration == 0)
	  anEvent->setRecursYearly(KOEvent::rYearlyDay, rFreq, -1);
	else
	  anEvent->setRecursYearly(KOEvent::rYearlyDay, rFreq, rDuration);
      }
    } else {
      kdDebug() << "we don't understand this type of recurrence!" << endl;
    } // if
  } // repeats
#endif
}

icaltimetype ICalFormat::writeICalDate(const QDate &date)
{
  icaltimetype t;
  
  t.year = date.year();
  t.month = date.month();
  t.day = date.day();

  t.is_date = 1;
  
  t.is_utc = 0;

  return t;
}

icaltimetype ICalFormat::writeICalDateTime(const QDateTime &datetime)
{
  icaltimetype t;
  
  t.year = datetime.date().year();
  t.month = datetime.date().month();
  t.day = datetime.date().day();

  t.hour = datetime.time().hour();
  t.minute = datetime.time().minute();
  t.second = datetime.time().second();

  t.is_date = 0;

  t.is_utc = 0;
  
  return t;
}

QDateTime ICalFormat::readICalDateTime(icaltimetype t)
{
/*
  kdDebug() << "ICalFormat::readICalDateTime()" << endl;
  kdDebug() << "--- Y: " << t.year << " M: " << t.month << " D: " << t.day
            << endl;
  kdDebug() << "--- H: " << t.hour << " M: " << t.minute << " S: " << t.second
            << endl;
  kdDebug() << "--- isDate: " << t.is_date << endl;
*/

  return QDateTime(QDate(t.year,t.month,t.day),
                   QTime(t.hour,t.minute,t.second));
}

QDate ICalFormat::readICalDate(icaltimetype t)
{
  return QDate(t.year,t.month,t.day);
}

char *ICalFormat::writeText(const QString &text)
{
  return const_cast<char *>(text.latin1());
}

icalcomponent *ICalFormat::createCalendarComponent()
{
  icalcomponent *calendar;

  // Root component
  calendar = icalcomponent_new(ICAL_VCALENDAR_COMPONENT);

  icalproperty *p;
  
  // Product Identifier
  p = icalproperty_new_prodid(const_cast<char *>(_PRODUCT_ID));
  icalcomponent_add_property(calendar,p);
  
  // TODO: Add time zone
  
  // iCalendar version (2.0)
  p = icalproperty_new_version(const_cast<char *>(_ICAL_VERSION));
  icalcomponent_add_property(calendar,p);
  
  return calendar;
}



// take a raw vcalendar (i.e. from a file on disk, clipboard, etc. etc.
// and break it down from it's tree-like format into the dictionary format
// that is used internally in the ICalFormat.
void ICalFormat::populate(icalcomponent *calendar)
{
  // this function will populate the caldict dictionary and other event 
  // lists. It turns vevents into KOEvents and then inserts them.

// TODO: check for METHOD
#if 0
  if ((curVO = isAPropertyOf(vcal, ICMethodProp)) != 0) {
    char *methodType = 0;
    methodType = fakeCString(vObjectUStringZValue(curVO));
    if (mEnableDialogs)
      KMessageBox::information(mTopWidget, 
			       i18n("This calendar is an iTIP transaction of type \"%1\".")
			       .arg(methodType),
                               i18n("KOrganizer: iTIP Transaction"));
    delete methodType;
  }
#endif

// TODO: check for unknown PRODID
#if 0
  // warn the user that we might have trouble reading non-known calendar.
  if ((curVO = isAPropertyOf(vcal, VCProdIdProp)) != 0) {
    char *s = fakeCString(vObjectUStringZValue(curVO));
    if (strcmp(_PRODUCT_ID, s) != 0)
      if (mEnableDialogs)
	KMessageBox::information(mTopWidget, 
			     i18n("This vCalendar file was not created by KOrganizer\n"
				     "or any other product we support. Loading anyway..."),
                             i18n("KOrganizer: Unknown vCalendar Vendor"));
    deleteStr(s);
  }
#endif

// TODO: check for calendar format version
#if 0  
  // warn the user we might have trouble reading this unknown version.
  if ((curVO = isAPropertyOf(vcal, VCVersionProp)) != 0) {
    char *s = fakeCString(vObjectUStringZValue(curVO));
    if (strcmp(_VCAL_VERSION, s) != 0)
      if (mEnableDialogs)
	KMessageBox::sorry(mTopWidget, 
			     i18n("This vCalendar file has version %1.\n"
			          "We only support %2.")
                             .arg(s).arg(_VCAL_VERSION),
                             i18n("KOrganizer: Unknown vCalendar Version"));
    deleteStr(s);
  }
#endif

// TODO: set time zone
#if 0  
  // set the time zone
  if ((curVO = isAPropertyOf(vcal, VCTimeZoneProp)) != 0) {
    char *s = fakeCString(vObjectUStringZValue(curVO));
    mCalendar->setTimeZone(s);
    deleteStr(s);
  }
#endif

  // Store all events with a relatedTo property in a list for post-processing
  mEventsRelate.clear();
  mTodosRelate.clear();
  // TODO: make sure that only actually added ecvens go to this lists.

  icalcomponent *c;

  // Iterate through all todos
  c = icalcomponent_get_first_component(calendar,ICAL_VTODO_COMPONENT);
  while (c) {
    kdDebug() << "----Todo found" << endl;
    KOEvent *todo = readTodo(c);
    if (!mCalendar->getTodo(todo->VUID())) mCalendar->addTodo(todo);
    c = icalcomponent_get_next_component(calendar,ICAL_VTODO_COMPONENT);
  }
  
  // Iterate through all events
  c = icalcomponent_get_first_component(calendar,ICAL_VEVENT_COMPONENT);
  while (c) {
    kdDebug() << "----Event found" << endl;  
    KOEvent *event = readEvent(c);
    if (!mCalendar->getEvent(event->VUID())) mCalendar->addEvent(event);
    c = icalcomponent_get_next_component(calendar,ICAL_VEVENT_COMPONENT);
  }
  
#if 0
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
	  kdDebug() << "skipping pilot-deleted event" << endl;
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

	if (mCalendar->getEvent(tmpStr)) {
	  goto SKIP;
	}
	if (mCalendar->getTodo(tmpStr)) {
	  goto SKIP;
	}
      }

      if ((!(curVOProp = isAPropertyOf(curVO, VCDTstartProp))) &&
	  (!(curVOProp = isAPropertyOf(curVO, VCDTendProp)))) {
	kdDebug() << "found a VEvent with no DTSTART and no DTEND! Skipping..." << endl;
	goto SKIP;
      }

      anEvent = VEventToEvent(curVO);
      // we now use addEvent instead of insertEvent so that the
      // signal/slot get connected.
      if (anEvent)
	mCalendar->addEvent(anEvent);
      else {
	// some sort of error must have occurred while in translation.
	goto SKIP;
      }
    } else if (strcmp(vObjectName(curVO), VCTodoProp) == 0) {
      anEvent = VTodoToEvent(curVO);
      mCalendar->addTodo(anEvent);
    } else if ((strcmp(vObjectName(curVO), VCVersionProp) == 0) ||
	       (strcmp(vObjectName(curVO), VCProdIdProp) == 0) ||
	       (strcmp(vObjectName(curVO), VCTimeZoneProp) == 0)) {
      // do nothing, we know these properties and we want to skip them.
      // we have either already processed them or are ignoring them.
      ;
    } else {
      kdDebug() << "Ignoring unknown vObject \"" << vObjectName(curVO) << "\"" << endl;
    }
  SKIP:
    ;
  } // while
#endif
  
  // Post-Process list of events with relations, put KOEvent objects in relation
  KOEvent *ev;
  for ( ev=mEventsRelate.first(); ev != 0; ev=mEventsRelate.next() ) {
    ev->setRelatedTo(mCalendar->getEvent(ev->getRelatedToVUID()));
  }
  for ( ev=mTodosRelate.first(); ev != 0; ev=mTodosRelate.next() ) {
    ev->setRelatedTo(mCalendar->getTodo(ev->getRelatedToVUID()));
  }
}

QString ICalFormat::extractErrorProperty(icalcomponent *c)
{
//  kdDebug() << "ICalFormat:extractErrorProperty: "
//            << icalcomponent_as_ical_string(c) << endl;

  QString errorMessage;

  icalproperty *error;
  error = icalcomponent_get_first_property(c,ICAL_XLICERROR_PROPERTY);
  while(error) {
    errorMessage += icalproperty_get_xlicerror(error);
    errorMessage += "\n";
    error = icalcomponent_get_next_property(c,ICAL_XLICERROR_PROPERTY);
  }

//  kdDebug() << "ICalFormat:extractErrorProperty: " << errorMessage << endl;
  
  return errorMessage;
}

void ICalFormat::parseError(const char *prop) 
{
  if (mEnableDialogs)
    KMessageBox::sorry(mTopWidget,
                       i18n("An error has occurred parsing this file.\n"
                            "It is missing property \"%1\".\n"
                            "Please verify that the file is in vCalendar "
                            "format\n"
                            "and try again, or load another file.\n")
                            .arg(prop),
                       i18n("KOrganizer: Error Parsing Calendar"));
}
