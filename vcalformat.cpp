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

#include "vcc.h"
#include "vobject.h"

#include "vcaldrag.h"
#include "qdatelist.h"

#include "calobject.h"

#include "vcalformat.h"

VCalFormat::VCalFormat(CalObject *cal) :
  CalFormat(cal)
{
}

VCalFormat::~VCalFormat()
{
}

bool VCalFormat::load(const QString &fileName)
{
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
}


bool VCalFormat::save(const QString &fileName)
{
  QString tmpStr;
  VObject *vcal, *vo;
  const char *fn = fileName.latin1();

  kdDebug() << "VCalFormat::save(): " << fn << endl;

  vcal = newVObject(VCCalProp);

  //  addPropValue(vcal,VCLocationProp, "0.0");
  addPropValue(vcal,VCProdIdProp, _PRODUCT_ID);
  tmpStr = mCalendar->getTimeZoneStr();
  addPropValue(vcal,VCTimeZoneProp, tmpStr.ascii());
  addPropValue(vcal,VCVersionProp, _VCAL_VERSION);

  // TODO STUFF
  QList<KOEvent> todoList = mCalendar->getTodoList();
  QListIterator<KOEvent> qlt(todoList);
  for (; qlt.current(); ++qlt) {
    vo = eventToVTodo(qlt.current());
    addVObjectProp(vcal, vo);
  }

  // EVENT STUFF
  QList<KOEvent> events = mCalendar->getAllEvents();
  KOEvent *ev;
  for(ev=events.first();ev;ev=events.next()) {
    vo = eventToVEvent(ev);
    addVObjectProp(vcal, vo);
  }

#if 0  
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
#endif

  writeVObjectToFile((char *) fn, vcal);
  cleanVObjects(vcal);
  cleanStrTbl();

  if (QFile::exists(fn)) {
    kdDebug() << "No error" << endl;
    return true;
  } else  {
    kdDebug() << "Error" << endl;
    return false; // error
  }
}


VCalDrag *VCalFormat::createDrag(KOEvent *selectedEv, QWidget *owner)
{
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
}

VCalDrag *VCalFormat::createDragTodo(KOEvent *selectedEv, QWidget *owner)
{
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
}

KOEvent *VCalFormat::createDrop(QDropEvent *de)
{
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
      kdDebug() << "VCalFormat::createDrop(): Got todo instead of event." << endl;
    } else if (strcmp(vObjectName(curvo), VCEventProp) == 0) {
      event = VEventToEvent(curvo);
    } else {
      kdDebug() << "VCalFormat::createDropTodo(): Unknown event type in drop." << endl;
    }
    // get rid of temporary VObject
    deleteVObject(vcal);
  }
  
  return event;
}

KOEvent *VCalFormat::createDropTodo(QDropEvent *de)
{
  kdDebug() << "VCalFormat::createDropTodo()" << endl;

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
      kdDebug() << "VCalFormat::createDropTodo(): Got event instead of todo." << endl;
    } else if (strcmp(vObjectName(curvo), VCTodoProp) == 0) {
      event = VTodoToEvent(curvo);
    } else {
      kdDebug() << "VCalFormat::createDropTodo(): Unknown event type in drop." << endl;
    }
    // get rid of temporary VObject
    deleteVObject(vcal);
  }
  
  return event;
}

bool VCalFormat::copyEvent(KOEvent *selectedEv)
{
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
}

KOEvent *VCalFormat::pasteEvent(const QDate *newDate, 
				const QTime *newTime)
{
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
      int hashTime = QTime::currentTime().hour() + 
	QTime::currentTime().minute() + QTime::currentTime().second() +
	QTime::currentTime().msec();
      QString uidStr;
      uidStr.sprintf("KOrganizer-%li.%d",KApplication::random(),hashTime);
      if (mCalendar->getEvent(anEvent->getVUID()))
	anEvent->setVUID(uidStr.ascii());

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
  mCalendar->updateCursors(anEvent);
  return anEvent;
}


VObject *VCalFormat::eventToVTodo(const KOEvent *anEvent)
{
  VObject *vtodo;
  QString tmpStr;
  QStringList tmpStrList;

  vtodo = newVObject(VCTodoProp);

  // due date
  if (anEvent->hasDueDate()) {
    tmpStr = qDateTimeToISO(anEvent->getDtDue(),
                            !anEvent->doesFloat());
    addPropValue(vtodo, VCDueProp, tmpStr.ascii());
  }

  // start date
  if (anEvent->hasStartDate()) {
    tmpStr = qDateTimeToISO(anEvent->getDtStart(),
	                    !anEvent->doesFloat());
    addPropValue(vtodo, VCDTstartProp, tmpStr.ascii());
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
	kdDebug() << "warning! this koevent has an attendee w/o name or email!" << endl;
      VObject *aProp = addPropValue(vtodo, VCAttendeeProp, tmpStr.ascii());
      addPropValue(aProp, VCRSVPProp, curAttendee->RSVP() ? "TRUE" : "FALSE");;
      addPropValue(aProp, VCStatusProp, curAttendee->getStatusStr().ascii());
    }
  }

  // description BL:
  if (!anEvent->getDescription().isEmpty()) {
    VObject *d = addPropValue(vtodo, VCDescriptionProp,
			      anEvent->getDescription().ascii());
    if (anEvent->getDescription().find('\n') != -1)
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

  // categories
  tmpStrList = anEvent->getCategories();
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
    addPropValue(vtodo, VCCategoriesProp, tmpStr.ascii());
  }

  // pilot sync stuff
  tmpStr.sprintf("%i",anEvent->getPilotId());
  addPropValue(vtodo, KPilotIdProp, tmpStr.ascii());
  tmpStr.sprintf("%i",anEvent->getSyncStatus());
  addPropValue(vtodo, KPilotStatusProp, tmpStr.ascii());

  return vtodo;
}

VObject* VCalFormat::eventToVEvent(const KOEvent *anEvent)
{
  VObject *vevent;
  QString tmpStr;
  QStringList tmpStrList;
  
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
	kdDebug() << "warning! this koevent has an attendee w/o name or email!" << endl;
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
			      anEvent->getDescription().ascii());
    if (anEvent->getDescription().find('\n') != -1)
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
    addPropValue(vevent, VCCategoriesProp, tmpStr.ascii());
  }

  // attachments
  tmpStrList = anEvent->getAttachments();
  for ( QStringList::Iterator it = tmpStrList.begin();
        it != tmpStrList.end();
        ++it )
    addPropValue(vevent, VCAttachProp, (*it).ascii());
  
  // resources
  tmpStrList = anEvent->getResources();
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

KOEvent *VCalFormat::VTodoToEvent(VObject *vtodo)
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
    anEvent->setOrganizer(mCalendar->getEmail());
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
	a = new Attendee(tmpStr.left(emailPos1 - 1),
			 tmpStr.mid(emailPos1 + 1, 
				    emailPos2 - (emailPos1 + 1)));
      } else if (tmpStr.find('@') > 0) {
	// just an email address
	a = new Attendee(0, tmpStr);
      } else {
	// just a name
	a = new Attendee(tmpStr);
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

  // start time
  if ((vo = isAPropertyOf(vtodo, VCDTstartProp)) != 0) {
    anEvent->setDtStart(ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))));
    //    kdDebug() << "s is " << //	  s << ", ISO is " << ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))).toString() << endl;
    deleteStr(s);
    anEvent->setHasStartDate(true);
  } else {
    anEvent->setHasStartDate(false);
  }  

  // related todo  
  if ((vo = isAPropertyOf(vtodo, VCRelatedToProp)) != 0) {
    anEvent->setRelatedToVUID(s = fakeCString(vObjectUStringZValue(vo)));
    deleteStr(s);
    mTodosRelate.append(anEvent);
  }

  // categories
  QStringList tmpStrList;
  int index1 = 0;
  int index2 = 0;
  if ((vo = isAPropertyOf(vtodo, VCCategoriesProp)) != 0) {
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

KOEvent* VCalFormat::VEventToEvent(VObject *vevent)
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
    anEvent->setOrganizer(mCalendar->getEmail());
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
	a = new Attendee(tmpStr.left(emailPos1 - 1),
			 tmpStr.mid(emailPos1 + 1, 
				    emailPos2 - (emailPos1 + 1)));
      } else if (tmpStr.find('@') > 0) {
	// just an email address
	a = new Attendee(0, tmpStr);
      } else {
	// just a name
	a = new Attendee(tmpStr);
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
    //    kdDebug() << "s is " << //	  s << ", ISO is " << ISOToQDateTime(s = fakeCString(vObjectUStringZValue(vo))).toString() << endl;
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
    anEvent->setSummary(tmpStr);
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
  QStringList tmpStrList;
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
    QString resource;
    while ((index2 = resources.find(';', index1)) != -1) {
      resource = resources.mid(index1, (index2 - index1));
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


QString VCalFormat::qDateToISO(const QDate &qd)
{
  QString tmpStr;

  ASSERT(qd.isValid());

  tmpStr.sprintf("%.2d%.2d%.2d",
		 qd.year(), qd.month(), qd.day());
  return tmpStr;
 
}

QString VCalFormat::qDateTimeToISO(const QDateTime &qdt, bool zulu)
{
  QString tmpStr;

  ASSERT(qdt.date().isValid());
  ASSERT(qdt.time().isValid());
  if (zulu) {
    QDateTime tmpDT(qdt);
    tmpDT = tmpDT.addSecs(60*(-mCalendar->getTimeZone())); // correct to GMT.
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

QDateTime VCalFormat::ISOToQDateTime(const QString & dtStr)
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
  if (dtStr[dtStr.length()-1] == 'Z')
    tmpDT = tmpDT.addSecs(60*mCalendar->getTimeZone());
  return tmpDT;
}

// take a raw vcalendar (i.e. from a file on disk, clipboard, etc. etc.
// and break it down from it's tree-like format into the dictionary format
// that is used internally in the VCalFormat.
void VCalFormat::populate(VObject *vcal)
{
  // this function will populate the caldict dictionary and other event 
  // lists. It turns vevents into KOEvents and then inserts them.

  VObjectIterator i;
  VObject *curVO, *curVOProp;
  KOEvent *anEvent;

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
  
  // set the time zone
  if ((curVO = isAPropertyOf(vcal, VCTimeZoneProp)) != 0) {
    char *s = fakeCString(vObjectUStringZValue(curVO));
    mCalendar->setTimeZone(s);
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
// Disable message boxes, because it hinders merging of calendars and does not
// give a sensible advice anyway.
#if 0
	  if (mEnableDialogs)
	    KMessageBox::error(mTopWidget,
				 i18n("Aborting merge of an event already in "
				      "calendar.\n"
				      "UID is %1\n"
				      "Please check your vCalendar file for "
				      "duplicate UIDs\n"
				      "and change them MANUALLY to be unique "
				      "if you find any.\n").arg(tmpStr),
				 i18n("KOrganizer: Possible Duplicate Event"));
#endif
	  goto SKIP;
	}
	if (mCalendar->getTodo(tmpStr)) {
#if 0
	  if (mEnableDialogs)
	    KMessageBox::error(mTopWidget,
				 i18n("Aborting merge of an event already in "
				      "calendar.\n"
				      "UID is %1\n"
				      "Please check your vCalendar file for "
				      "duplicate UIDs\n"
				      "and change them MANUALLY to be unique "
				      "if you find any.\n").arg(tmpStr),
				 i18n("KOrganizer: Possible Duplicate Event"));
	  
#endif
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
  
  // Post-Process list of events with relations, put KOEvent objects in relation
  KOEvent *ev;
  for ( ev=mEventsRelate.first(); ev != 0; ev=mEventsRelate.next() ) {
    ev->setRelatedTo(mCalendar->getEvent(ev->getRelatedToVUID()));
  }
  for ( ev=mTodosRelate.first(); ev != 0; ev=mTodosRelate.next() ) {
    ev->setRelatedTo(mCalendar->getTodo(ev->getRelatedToVUID()));
  }
}

const char *VCalFormat::dayFromNum(int day)
{
  const char *days[7] = { "MO ", "TU ", "WE ", "TH ", "FR ", "SA ", "SU " };

  return days[day];
}

int VCalFormat::numFromDay(const QString &day)
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

void VCalFormat::parseError(const char *prop) 
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
