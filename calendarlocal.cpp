// Calendar class for KOrganizer
// (c) 1998 Preston Brown
// 	$Id$

#include "config.h"

#include <qdatetime.h>
#include <qstring.h>
#include <qlist.h>
#include <stdlib.h>
#include <qregexp.h>
#include <qclipboard.h>
#include <qdialog.h>
#include <qfile.h>

#include <kapp.h>
#include <kdebug.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "vcaldrag.h"
#include "qdatelist.h"
#include "koprefs.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "koexceptions.h"
#include "incidence.h"

#include "calendarlocal.h"
#include "calendarlocal.moc"

CalendarLocal::CalendarLocal()
  : CalObject()
{
  mOldestDate = 0L;
  mNewestDate = 0L;

  mRecursList.setAutoDelete(TRUE);
  // solves the leak?
  mTodoList.setAutoDelete(TRUE);

  mCalDict = new QIntDict<QList<KOEvent> > (BIGPRIME);
  mCalDict->setAutoDelete(TRUE);
}

CalendarLocal::~CalendarLocal() 
{
  close();
  delete mCalDict;
  delete mNewestDate;
  delete mOldestDate;
}

bool CalendarLocal::load(const QString &fileName)
{
  kdDebug() << "CalendarLocal::load(): '" << fileName << "'" << endl;

  // do we want to silently accept this, or make some noise?  Dunno...
  // it is a semantical thing vs. a practical thing.
  if (fileName.isEmpty()) return false;

  // If cal format can be identified by extension use appropriate format. If
  // not, try to load with iCalendar. It will detect, if it is actually a
  // vCalendar file.
  QString extension = fileName.right(4);
  if (extension == ".ics") {
    return mICalFormat->load(fileName);
  } else if (extension == ".vcs") {
    return mFormat->load(fileName);
  } else {
    mICalFormat->clearException();
    if (!mICalFormat->load(fileName)) {
      // iCalendar loading failed
      if (mICalFormat->exception()) {
        if (mICalFormat->exception()->errorCode() == 
            KOErrorFormat::CalVersion1) {
          return mFormat->load(fileName);
        } else {
          return false;
        }
      } else {
        return false;
      }
    } else {
      // successfully loaded iCalendar
      return true;
    }
  }
}

bool CalendarLocal::save(const QString &fileName,CalFormat *format)
{
  if (format) {
    return format->save(fileName);
  } else {
    return mFormat->save(fileName);
  }
}

void CalendarLocal::close()
{
  QIntDictIterator<QList<KOEvent> > qdi(*mCalDict);
  QList<KOEvent> *tmpList;

  // Delete non-recurring events
  qdi.toFirst();
  while (qdi.current()) {
    tmpList = qdi.current();
    QDate keyDate = keyToDate(qdi.currentKey());
    KOEvent *ev;
    for(ev = tmpList->first();ev;ev = tmpList->next()) {
//      kdDebug() << "-----FIRST.  " << ev->getSummary() << endl;
//      kdDebug() << "---------MUL: " << (ev->isMultiDay() ? "Ja" : "Nein") << endl;
      bool del = false;
      if (ev->isMultiDay()) {
        if (ev->getDtStart().date() == keyDate) {
          del = true;
        }
      } else {
        del = true;
      }
      if (del) {
//        kdDebug() << "-----DEL  " << ev->getSummary() << endl;
        delete ev;
      }
    }
    ++qdi;
  }

  mCalDict->clear();
  mRecursList.clear();
  mTodoList.clear();
  
  // reset oldest/newest date markers
  delete mOldestDate;
  mOldestDate = 0L;
  delete mNewestDate;
  mNewestDate = 0L;
}


void CalendarLocal::addEvent(KOEvent *anEvent)
{
// OBSOLETE: anEvent->setTodoStatus(false);
  insertEvent(anEvent);
  // set event's read/write status  
  if (anEvent->getOrganizer() != getEmail())
    anEvent->setReadOnly(TRUE);
  connect(anEvent, SIGNAL(eventUpdated(Incidence *)), this,
	  SLOT(updateEvent(Incidence *)));
  emit calUpdated(anEvent);
}

// probably not really efficient, but...it works for now.
void CalendarLocal::deleteEvent(KOEvent *event)
{
  kdDebug() << "CalendarLocal::deleteEvent" << endl;
  
  QDate date(event->getDtStart().date());

  QList<KOEvent> *tmpList;
  KOEvent *anEvent;
  int extraDays, dayOffset;
  QDate startDate, tmpDate;

  tmpList = mCalDict->find(makeKey(date));
  // if tmpList exists, the event is in the normal dictionary; 
  // it doesn't recur.
  if (tmpList) {
    for (anEvent = tmpList->first(); anEvent;
	 anEvent = tmpList->next()) {
      if (anEvent == event) {
	if (!anEvent->isMultiDay()) {
	  tmpList->setAutoDelete(FALSE);
	  tmpList->remove();
	  goto FINISH;
	} else {
	  //kdDebug() << "deleting multi-day event" << endl;
	  // event covers multiple days.
	  startDate = anEvent->getDtStart().date();
	  extraDays = startDate.daysTo(anEvent->getDtEnd().date());
	  for (dayOffset = 0; dayOffset <= extraDays; dayOffset++) {
	    tmpDate = startDate.addDays(dayOffset);
	    tmpList = mCalDict->find(makeKey(tmpDate));
	    if (tmpList) {
	      for (anEvent = tmpList->first(); anEvent;
		   anEvent = tmpList->next()) {
		if (anEvent == event)
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
  for (anEvent = mRecursList.first(); anEvent;
       anEvent = mRecursList.next()) {
    if (anEvent == event) {
      mRecursList.remove();
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
  if (date == (*mOldestDate)) {
    for (; !mCalDict->find(makeKey((*mOldestDate))) &&
	   (*mOldestDate != *mNewestDate); 
	 (*mOldestDate) = mOldestDate->addDays(1));
    mRecursList.first();
    while ((anEvent = mRecursList.current())) {
      if (anEvent->getDtStart().date() < (*mOldestDate)) {
	(*mOldestDate) = anEvent->getDtStart().date();
	mRecursList.first();
      }
      anEvent = mRecursList.next();
    }
  }

  if (date == (*mNewestDate)) {
    for (; !mCalDict->find(makeKey((*mNewestDate))) &&
	   (*mNewestDate != *mOldestDate);
	 (*mNewestDate) = mNewestDate->addDays(-1));
    mRecursList.first();
    while ((anEvent = mRecursList.current())) {
      if (anEvent->getDtStart().date() > (*mNewestDate)) {
	(*mNewestDate) = anEvent->getDtStart().date();
	mRecursList.first();
      }
      anEvent = mRecursList.next();
    }
  }


  emit calUpdated(anEvent);
}


KOEvent *CalendarLocal::getEvent(const QString &UniqueStr)
{
  QList<KOEvent> *eventList;
  QIntDictIterator<QList<KOEvent> > dictIt(*mCalDict);
  KOEvent *anEvent;

  while (dictIt.current()) {
    eventList = dictIt.current();
    for (anEvent = eventList->first(); anEvent;
	 anEvent = eventList->next()) {
      if (anEvent->getVUID() == UniqueStr) {
	return anEvent;
      }
    }
    ++dictIt;
  }
  for (anEvent = mRecursList.first(); anEvent;
       anEvent = mRecursList.next()) {
    if (anEvent->getVUID() == UniqueStr) {
      return anEvent;
    }
  }
  // catch-all.
  return (KOEvent *) 0L;
}

void CalendarLocal::addTodo(Todo *todo)
{
// OBSOLETE:  todo->setTodoStatus(true);
  mTodoList.append(todo);
  connect(todo, SIGNAL(eventUpdated(Incidence *)), this,
	  SLOT(updateEvent(Incidence *)));
  emit calUpdated(todo);
}

void CalendarLocal::deleteTodo(Todo *todo)
{
  mTodoList.findRef(todo);
  mTodoList.remove();
  emit calUpdated(todo);
}


const QList<Todo> &CalendarLocal::getTodoList() const
{
  return mTodoList;
}

Todo *CalendarLocal::getTodo(const QString &UniqueStr)
{
  Todo *aTodo;
  for (aTodo = mTodoList.first(); aTodo;
       aTodo = mTodoList.next())
    if (aTodo->getVUID() == UniqueStr)
      return aTodo;
  // not found
  return 0;
}

QList<Todo> CalendarLocal::getTodosForDate(const QDate & date)
{
  QList<Todo> todos;

  Todo *aTodo;
  for (aTodo = mTodoList.first();aTodo;aTodo = mTodoList.next()) {
    if (aTodo->hasDueDate() && aTodo->dtDue().date() == date) {
      todos.append(aTodo);
    }
  }
  
  return todos;
}

int CalendarLocal::numEvents(const QDate &qd)
{
  QList<KOEvent> *tmpList;
  KOEvent *anEvent;
  int count = 0;
  int extraDays, i;

  // first get the simple case from the dictionary.
  tmpList = mCalDict->find(makeKey(qd));
  if (tmpList)
    count += tmpList->count();

  // next, check for repeating events.  Even those that span multiple days...
  for (anEvent = mRecursList.first(); anEvent; anEvent = mRecursList.next()) {
    if (anEvent->isMultiDay()) {
      extraDays = anEvent->getDtStart().date().daysTo(anEvent->getDtEnd().date());
      //kdDebug() << "multi day event w/" << extraDays << " days" << endl;
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

void CalendarLocal::checkAlarms()
{
  QList<KOEvent> alarmEvents;
  QIntDictIterator<QList<KOEvent> > dictIt(*mCalDict);
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
  for (anEvent = mRecursList.first(); anEvent;
       anEvent = mRecursList.next()) {
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
void CalendarLocal::updateEvent(Incidence *incidence)
{
  incidence->setSyncStatus(KOEvent::SYNCMOD);
  incidence->setLastModified(QDateTime::currentDateTime());
  // we should probably update the revision number here,
  // or internally in the KOEvent itself when certain things change.
  // need to verify with ical documentation.

  // handle sending the event to those attendees that need it.
  // mostly broken right now.
  if (incidence->attendeeCount()) {
    QList<Attendee> al;
    Attendee *a;
    
    al = incidence->getAttendeeList();
    for (a = al.first(); a; a = al.next()) {
      if ((a->flag()) && (a->RSVP())) {
	//kdDebug() << "send appointment to " << a->getName() << endl;
	a->setFlag(false);
      }
    }
  }

  KOEvent *anEvent = dynamic_cast<KOEvent *>(incidence);
  if (!anEvent) {
//    kdDebug() << "CalendarLocal::updateEvent(): Warning! Passed non-KOEvent" << endl;
  } else {
    // we don't need to do anything to Todo events.
 
    QIntDictIterator<QList<KOEvent> > qdi(*mCalDict);
    QList<KOEvent> *tmpList;

    // the first thing we do is REMOVE all occurances of the event from 
    // both the dictionary and the recurrence list.  Then we reinsert it.
    // We don't bother about optimizations right now.
    qdi.toFirst();
    while ((tmpList = qdi.current()) != 0L) {
      ++qdi;
      tmpList->removeRef(anEvent);
    }
    // take any instances of it out of the recurrence list
    if (mRecursList.findRef(anEvent) != -1)
      mRecursList.take();
    
    // ok the event is now GONE.  we want to re-insert it.
    insertEvent(anEvent);
  }

  emit calUpdated(anEvent);
}

// this function will take a VEvent and insert it into the event
// dictionary for the CalendarLocal.  If there is no list of events for that
// particular location in the dictionary, a new one will be created.
void CalendarLocal::insertEvent(const KOEvent *anEvent)
{
  long tmpKey;
  QString tmpDateStr;
  QList<KOEvent> *eventList;
  int extraDays, dayCount;

  // initialize if they haven't been allocated yet;
  if (!mOldestDate) {
    mOldestDate = new QDate();
    (*mOldestDate) = anEvent->getDtStart().date();
  } 
  if (!mNewestDate) {
    mNewestDate = new QDate();
    (*mNewestDate) = anEvent->getDtStart().date();
  }
    
  // update oldest and newest dates if necessary.
  if (anEvent->getDtStart().date() < (*mOldestDate))
    (*mOldestDate) = anEvent->getDtStart().date();
  if (anEvent->getDtStart().date() > (*mNewestDate))
    (*mNewestDate) = anEvent->getDtStart().date();

  if (anEvent->doesRecur()) {
    mRecursList.append(anEvent);
  } else {
    // set up the key
    extraDays = anEvent->getDtStart().date().daysTo(anEvent->getDtEnd().date());
    for (dayCount = 0; dayCount <= extraDays; dayCount++) {
      tmpKey = makeKey(anEvent->getDtStart().addDays(dayCount));
      // insert the item into the proper list in the dictionary
      if ((eventList = mCalDict->find(tmpKey)) != 0) {
	eventList->append(anEvent);
      } else {
	// no items under that date yet
	eventList = new QList<KOEvent>;
	eventList->append(anEvent);
	mCalDict->insert(tmpKey, eventList);
      }
    }
  }
}

// make a long dict key out of a QDateTime
long int CalendarLocal::makeKey(const QDateTime &dt)
{
  QDate tmpD;
  QString tmpStr;

  tmpD = dt.date();
  tmpStr.sprintf("%d%.2d%.2d",tmpD.year(), tmpD.month(), tmpD.day());
//  kdDebug() << "CalendarLocal::makeKey(): " << tmpStr << endl;
  return tmpStr.toLong();
}

// make a long dict key out of a QDate
long int CalendarLocal::makeKey(const QDate &d)
{
  QString tmpStr;

  tmpStr.sprintf("%d%.2d%.2d",d.year(), d.month(), d.day());
  return tmpStr.toLong();
}

QDate CalendarLocal::keyToDate(long int key)
{  
  QString dateStr = QString::number(key);
//  kdDebug() << "CalendarLocal::keyToDate(): " << dateStr << endl;
  QDate date(dateStr.mid(0,4).toInt(),dateStr.mid(4,2).toInt(),
             dateStr.mid(6,2).toInt());
             
//  kdDebug() << "  QDate: " << date.toString() << endl;

  return date;
}


// taking a QDate, this function will look for an eventlist in the dict
// with that date attached -
// BL: an the returned list should be deleted!!!
QList<KOEvent> CalendarLocal::eventsForDate(const QDate &qd, bool sorted)
{
  QList<KOEvent> eventList;
  QList<KOEvent> *tmpList;
  KOEvent *anEvent;
  tmpList = mCalDict->find(makeKey(qd));
  if (tmpList) {
    for (anEvent = tmpList->first(); anEvent;
	 anEvent = tmpList->next())
      eventList.append(anEvent);
  }
  int extraDays, i;
  for (anEvent = mRecursList.first(); anEvent; anEvent = mRecursList.next()) {
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
    return eventList;
  }
  //  kdDebug() << "Sorting getEvents for date\n" << endl;
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
  return eventListSorted;
}


QList<KOEvent> CalendarLocal::events(const QDate &start,const QDate &end,
                                    bool inclusive)
{
  QIntDictIterator<QList<KOEvent> > qdi(*mCalDict);
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
  for(ev = mRecursList.first();ev;ev = mRecursList.next()) {
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
}

QList<KOEvent> CalendarLocal::getAllEvents()
{
  return events(*mOldestDate,*mNewestDate);
}


// taking a QDateTime, this function will look for an eventlist in the dict
// with that date attached.
// this list is dynamically allocated and SHOULD BE DELETED when done with!
QList<KOEvent> CalendarLocal::eventsForDate(const QDateTime &qdt)
{
  return eventsForDate(qdt.date());
}
