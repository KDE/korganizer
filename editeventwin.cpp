/*
    $Id$

     Requires the Qt and KDE widget libraries, available at no cost at
     http://www.troll.no and http://www.kde.org respectively

     Copyright (C) 1997, 1998 Preston Brown
     preston.brown@yale.edu

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

     -*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-

     This file implements a class for displaying a dialog box for
     adding or editing appointments/events.
*/

#include <stdio.h>

#include <qtooltip.h>
#include <qframe.h>
#include <qpixmap.h>

#include <kdatepik.h>
#include <kiconloader.h>
#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>
#include <ktoolbarbutton.h>

#include "qdatelist.h"
#include "misc.h"
#include "editeventwin.moc"

EditEventWin::EditEventWin(CalObject *cal)
  : EventWin(cal)
{
  initConstructor();

  KConfig *config = kapp->config();
  config->setGroup("Time & Date");
  QString confStr(config->readEntry("Default Start Time", "12:00"));
  int pos = confStr.find(':');
  if (pos >= 0)
    confStr.truncate(pos);
  int fmt = confStr.toUInt();
  fillInDefaults(QDateTime(QDate::currentDate(), QTime(fmt,0,0)),
		 QDateTime(QDate::currentDate(), QTime(fmt+1,0,0)), FALSE);
  prevNextUsed = FALSE;
  Type = TYPE_APPOINTMENT;
}

void EditEventWin::editEvent( KOEvent *event, QDate qd )
{
  // check to see if save & close should be enabled or not
  saveExit->setEnabled(! event->isReadOnly());
  btnDelete->setEnabled(true);

  currEvent = event;
  addFlag = FALSE;
  fillInFields(qd);

  unsetModified();
  setTitle();
}

void EditEventWin::newEvent( QDateTime from, QDateTime to, bool allDay )
{
  saveExit->setEnabled(true);
  btnDelete->setEnabled(false);

  currEvent = 0L;
  addFlag = true;
  fillInDefaults( from, to, allDay );

  unsetModified();
  setTitle();
}

void EditEventWin::initConstructor()
{
    /*
  initToolBar();
  m_frame = new QFrame(this);
  tabCtl = new KTabCtl(m_frame);
  CHECK_PTR(tabCtl);
      */
  QString tmpStr;

  General = new EventWinGeneral(tabWidget, "General");
  CHECK_PTR(General);
  connect(General, SIGNAL(modifiedEvent()),
    this, SLOT(setModified()));

  Details = new EventWinDetails(tabWidget, "Details", false);
  CHECK_PTR(Details);
  connect(Details, SIGNAL(modifiedEvent()),
    this, SLOT(setModified()));

  Recurrence = new EventWinRecurrence(tabWidget, "Recurrence", false);
  CHECK_PTR(Recurrence);
  connect(Recurrence, SIGNAL(modifiedEvent()),
    this, SLOT(setModified()));

//  setFrameBorderWidth(0);

  tabWidget->addTab(General, i18n("General"));  
  tabWidget->addTab(Details, i18n("Details"));
  tabWidget->addTab(Recurrence, i18n("Recurrence"));

  setView(tabWidget);
  
  // QTabWidget::minimumSizeHint() seems not to work. This is a workaroud
  tabWidget->setMinimumSize(tabWidget->sizeHint());
  
  // how do you resize the view to fit everything by default??
//  resize(605, 455);

  // this is temporary until we can have the thing resize properly.
//  setFixedWidth(605);
//  setFixedHeight(455);

  // signal/slot stuff

  // category views on General and Details tab are synchronized
  catDlg = new CategoryDialog();
  connect(General->categoriesButton, SIGNAL(clicked()), 
	  catDlg, SLOT(show()));
  connect(Details->categoriesButton, SIGNAL(clicked()),
	  catDlg, SLOT(show()));
  connect(catDlg, SIGNAL(okClicked(QString)), 
	  this, SLOT(updateCatEdit(QString)));
  // time widgets on General and Recurrence tab are synchronized
  connect(General->startTimeEdit, SIGNAL(timeChanged(QTime, int)),
	  this, SLOT(startTimeChanged(QTime, int)));
  connect(General->endTimeEdit, SIGNAL(timeChanged(QTime, int)),
	  this, SLOT(endTimeChanged(QTime, int)));
  connect(Recurrence->startTimeEdit, SIGNAL(timeChanged(QTime, int)),
	  this, SLOT(startTimeChanged(QTime, int)));
  connect(Recurrence->endTimeEdit, SIGNAL(timeChanged(QTime, int)),
	  this, SLOT(endTimeChanged(QTime, int)));

  // date widgets on General and Recurrence tab are synchronized
  connect(General->startDateEdit, SIGNAL(dateChanged(QDate)),
	  this, SLOT(startDateChanged(QDate)));
  connect(Recurrence->startDateEdit, SIGNAL(dateChanged(QDate)),
	  this, SLOT(startDateChanged(QDate)));
  connect(General->endDateEdit, SIGNAL(dateChanged(QDate)),
	  this, SLOT(endDateChanged(QDate)));

  // attach the enable/disabling of recurrence tab stuff
  connect(General->recursButton, SIGNAL(toggled(bool)),
	  this, SLOT(recurStuffEnable(bool)));
  connect(General->noTimeButton, SIGNAL(toggled(bool)),
	  Recurrence, SLOT(timeStuffDisable(bool)));

}

EditEventWin::~EditEventWin()
{
  // delete NON-CHILDREN widgets
  delete catDlg;
//  delete m_frame;
}

/*************************************************************************/
/********************** PROTECTED METHODS ********************************/
/*************************************************************************/

void EditEventWin::fillInFields(QDate qd)
{
  blockSignals(true); // be nice now...

  QString tmpStr;
  QDateTime tmpDT;
  int i;

  /******************** GENERAL *******************************/
  General->summaryEdit->setText(currEvent->getSummary().data());
  General->descriptionEdit->setText(currEvent->getDescription().data());

  // organizer information
  General->ownerLabel->setText(tmpStr.sprintf((const char*)i18n("Owner: %s"),
					      currEvent->getOrganizer().data()).data());

  // the rest is for the events only
  General->noTimeButton->setChecked(currEvent->doesFloat());
  General->timeStuffDisable(currEvent->doesFloat());
  General->alarmStuffDisable(currEvent->doesFloat());

  currStartDateTime = currEvent->getDtStart();
  currEndDateTime = currEvent->getDtEnd();
  setDuration();

  General->startDateEdit->setDate(currStartDateTime.date());
  General->startTimeEdit->setTime(currStartDateTime.time());
  
  General->endDateEdit->setDate(currEndDateTime.date());
  General->endTimeEdit->setTime(currEndDateTime.time());

  General->recursButton->setChecked(currEvent->doesRecur());

  General->privateButton->setChecked((currEvent->getSecrecy() > 0) 
				     ? TRUE : FALSE);

  // set up alarm stuff
  General->alarmButton->setChecked(currEvent->getAlarmRepeatCount());
  if (General->alarmButton->isChecked()) {
    General->alarmStuffEnable(TRUE);
    tmpDT = currEvent->getAlarmTime();
    i = tmpDT.secsTo(currStartDateTime);
    i = i / 60; // make minutes
    if (i % 60 == 0) { // divides evenly into hours?
      i = i / 60;
      General->alarmIncrCombo->setCurrentItem(1);
    }
    if (i % 24 == 0) { // divides evenly into days?
      i = i / 24;
      General->alarmIncrCombo->setCurrentItem(2);
    }

    tmpStr.sprintf("%d",i);
    General->alarmTimeEdit->setText(tmpStr.data());

    if (!currEvent->getProgramAlarmFile().isEmpty()) {
      General->alarmProgram = currEvent->getProgramAlarmFile();
      General->alarmProgramButton->setOn(TRUE);
      QString dispStr = "Running \"";
      dispStr += General->alarmProgram.data();
      dispStr += "\"";
      QToolTip::add(General->alarmProgramButton, dispStr.data());
    }
    if (!currEvent->getAudioAlarmFile().isEmpty()) {
      General->alarmSound = currEvent->getAudioAlarmFile();
      General->alarmSoundButton->setOn(TRUE);
      QString dispStr = "Playing \"";
      dispStr += General->alarmSound.data();
      dispStr += "\"";
      QToolTip::add(General->alarmSoundButton, dispStr.data());
    }
  } else
    General->alarmStuffEnable(FALSE);
    
    

  if (currEvent->getTransparency() > 0)
    General->freeTimeCombo->setCurrentItem(1);
  // else it is implicitly 0 (i.e. busy)

  // categories
  catDlg->setSelected(currEvent->getCategories());
  General->categoriesLabel->setText(currEvent->getCategoriesStr());

  /******************** DETAILS *******************************/
  // attendee information
  // first remove whatever might be here
  Details->mAttendeeList.clear();
  QList<Attendee> tmpAList = currEvent->getAttendeeList();
  Attendee *a;
  for (a = tmpAList.first(); a; a = tmpAList.next())
    Details->insertAttendee(new Attendee (*a));

  //  Details->attachListBox->insertItem(i18n("Not implemented yet."));
  
  // set the status combobox
  Details->statusCombo->setCurrentItem(currEvent->getStatus());

  Details->categoriesLabel->setText( currEvent->getCategoriesStr() );  

  /******************** RECURRENCE *******************************/
  QBitArray rDays;
  QList<KOEvent::rMonthPos> rmp;
  QList<int> rmd;
  bool ok;

  Recurrence->startTimeEdit->setTime(General->startTimeEdit->getTime(ok));
  Recurrence->endTimeEdit->setTime(General->endTimeEdit->getTime(ok));
  Recurrence->timeStuffDisable(currEvent->doesFloat());

  recurStuffEnable(currEvent->doesRecur());
  General->recursButton->setChecked(currEvent->doesRecur());

  // unset everything
  Recurrence->unsetAllCheckboxes();
  switch (currEvent->doesRecur()) {
  case KOEvent::rNone:
    break;
  case KOEvent::rDaily:
    Recurrence->dailyButton->setChecked(TRUE);
    tmpStr.sprintf("%d",currEvent->getRecursFrequency());
    Recurrence->nDaysEntry->setText(tmpStr.data());
    break;
  case KOEvent::rWeekly:
    Recurrence->weeklyButton->setChecked(TRUE);
    tmpStr.sprintf("%d", currEvent->getRecursFrequency());
    Recurrence->nWeeksEntry->setText(tmpStr.data());
    
    rDays = currEvent->getRecursDays();
    Recurrence->setCheckedDays(rDays);
    break;
  case KOEvent::rMonthlyPos:
    // we only handle one possibility in the list right now,
    // so I have hardcoded calls with first().  If we make the GUI
    // more extended, this can be changed.
    Recurrence->monthlyButton->setChecked(TRUE);
    Recurrence->onNthTypeOfDay->setChecked(TRUE);
    rmp = currEvent->getRecursMonthPositions();
    if (rmp.first()->negative)
      i = 5 - rmp.first()->rPos - 1;
    else
      i = rmp.first()->rPos - 1;
    Recurrence->nthNumberEntry->setCurrentItem(i);
    i = 0;
    while (!rmp.first()->rDays.testBit(i))
      ++i;
    Recurrence->nthTypeOfDayEntry->setCurrentItem(i);
    tmpStr.sprintf("%d",currEvent->getRecursFrequency());
    Recurrence->nMonthsEntry->setText(tmpStr.data());
    break;
  case KOEvent::rMonthlyDay:
    Recurrence->monthlyButton->setChecked(TRUE);
    Recurrence->onNthDay->setChecked(TRUE);
    rmd = currEvent->getRecursMonthDays();
    i = *rmd.first() - 1;
    Recurrence->nthDayEntry->setCurrentItem(i);
    tmpStr.sprintf("%d",currEvent->getRecursFrequency());
    Recurrence->nMonthsEntry->setText(tmpStr.data());
    break;
  case KOEvent::rYearlyMonth:
    Recurrence->yearlyButton->setChecked(TRUE);
    Recurrence->yearMonthButton->setChecked(TRUE);
    rmd = currEvent->getRecursYearNums();
    Recurrence->yearMonthComboBox->setCurrentItem(*rmd.first() - 1);
    tmpStr.sprintf("%d",currEvent->getRecursFrequency());
    Recurrence->nYearsEntry->setText(tmpStr.data());
    break;
  case KOEvent::rYearlyDay:
    Recurrence->yearlyButton->setChecked(TRUE);
    Recurrence->yearDayButton->setChecked(TRUE);
    tmpStr.sprintf("%d",currEvent->getRecursFrequency());
    Recurrence->nYearsEntry->setText(tmpStr.data());
    break;
  default:
    break;
  }

  if (currEvent->doesRecur()) {
    // get range information
    if (currEvent->getRecursDuration() == -1)
      Recurrence->noEndDateButton->setChecked(TRUE);
    else if (currEvent->getRecursDuration() == 0) {
      Recurrence->endDateButton->setChecked(TRUE);
      Recurrence->endDateEdit->setDate(currEvent->getRecursEndDate());
    } else {
      Recurrence->endDurationButton->setChecked(TRUE);
      tmpStr.sprintf("%d",currEvent->getRecursDuration());
      Recurrence->endDurationEdit->setText(tmpStr.data());
    }
  } else {
    // the event doesn't recur, but we should provide some logical
    // defaults in case they go and make it recur.
    Recurrence->noEndDateButton->setChecked(TRUE);
    Recurrence->weeklyButton->setChecked(TRUE);
    Recurrence->nDaysEntry->setText("1");
    Recurrence->nWeeksEntry->setText("1");
    Recurrence->CheckDay(currStartDateTime.date().dayOfWeek());
    Recurrence->onNthDay->setChecked(TRUE);
    Recurrence->nthDayEntry->setCurrentItem(currStartDateTime.date().day()-1);
    Recurrence->nMonthsEntry->setText("1");
    Recurrence->yearDayButton->setChecked(TRUE);
    tmpStr.sprintf("%i",currStartDateTime.date().dayOfYear());
    Recurrence->nYearsEntry->setText("1");
  }

  QDateList exDates(FALSE);
  exDates = currEvent->getExDates();
  QDate *curDate;
  Recurrence->exceptionDateEdit->setDate(qd);
  for (curDate = exDates.first(); curDate;
       curDate = exDates.next())
    Recurrence->exceptionList->insertItem(curDate->toString().data());

  blockSignals(false);
}

void EditEventWin::fillInDefaults(QDateTime from, 
				  QDateTime to, bool allDay)
{
  QString tmpStr;

  blockSignals(true);

  currStartDateTime = from;
  currEndDateTime = to;
  setDuration();

  // default owner is calendar owner.
  General->ownerLabel->setText(tmpStr.sprintf((const char*)i18n("Owner: %s"),
					      calendar->getEmail().data()).data());

  /********************************* GENERAL *******************************/

  General->noTimeButton->setChecked(allDay);
  General->timeStuffDisable(allDay);
  General->alarmStuffDisable(allDay);
  General->startDateEdit->setDate(from.date());
  General->endDateEdit->setDate(to.date());
  General->startTimeEdit->setTime(from.time());
  General->endTimeEdit->setTime(to.time());

  recurStuffEnable(FALSE);
  General->recursButton->setChecked(FALSE);
  KConfig *config(kapp->config());
  config->setGroup("Time & Date");
  QString alarmText(config->readEntry("Default Alarm Time", "15"));
  int pos = alarmText.find(' ');
  if (pos >= 0)
    alarmText.truncate(pos);
  General->alarmTimeEdit->setText(alarmText.data());
  General->alarmStuffEnable(FALSE);
  /******************************** DETAILS ********************************/
  Details->attendeeRSVPButton->setChecked(TRUE);

  /******************************* RECURRENCE ******************************/
  Recurrence->timeStuffDisable(allDay);
  Recurrence->noEndDateButton->setChecked(TRUE);
  Recurrence->weeklyButton->setChecked(TRUE);
  Recurrence->nDaysEntry->setText("1");
  Recurrence->nWeeksEntry->setText("1");
  // unset everything
  Recurrence->unsetAllCheckboxes();
  // now get the right one for the start date
  Recurrence->CheckDay(from.date().dayOfWeek());
  Recurrence->onNthDay->setChecked(TRUE);
  Recurrence->nthDayEntry->setCurrentItem(from.date().day()-1);
  Recurrence->nMonthsEntry->setText("1");
  Recurrence->yearDayButton->setChecked(TRUE);
  tmpStr.sprintf("%i",from.date().dayOfYear());
  Recurrence->nYearsEntry->setText("1");

  blockSignals(false);
}

//void EditEventWin::closeEvent(QCloseEvent *)
bool EditEventWin::queryClose()
{
  // we clean up after ourselves...
//  Commented out because it does nothing useful.
//  emit closed(this);
  return true;
}

/*************************************************************************/
/********************** PROTECTED SLOTS **********************************/
/*************************************************************************/

// used to make a new event / update an old event

KOEvent* EditEventWin::makeEvent()
{
  KOEvent *anEvent;
  QDate tmpDate;
  QTime tmpTime;
  QDateTime tmpDT;

  // temp. until something better happens.
  QString tmpStr;
  bool ok;
  uint i;
  int j;

  anEvent = EventWin::makeEvent();

  if (General->noTimeButton->isChecked()) {
    anEvent->setFloats(TRUE);
    // need to change this.
    tmpDate = General->startDateEdit->getDate();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    anEvent->setDtStart(tmpDT);

    tmpDate = General->endDateEdit->getDate();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    anEvent->setDtEnd(tmpDT);
  } else {
    anEvent->setFloats(FALSE);
    
    // set date/time end
    tmpDate = General->endDateEdit->getDate();
    tmpTime = General->endTimeEdit->getTime(ok);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    anEvent->setDtEnd(tmpDT);

    // set date/time start
    tmpDate = General->startDateEdit->getDate();
    tmpTime = General->startTimeEdit->getTime(ok);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    anEvent->setDtStart(tmpDT);
    
  } // check for float


  // alarm stuff
  if (General->alarmButton->isChecked()) {
    anEvent->setAlarmRepeatCount(1);
    tmpStr = General->alarmTimeEdit->text();
    j = tmpStr.toInt() * -60;
    if (General->alarmIncrCombo->currentItem() == 1)
      j = j * 60;
    else if (General->alarmIncrCombo->currentItem() == 2)
      j = j * (60 * 24);

    tmpDT = anEvent->getDtStart();
    tmpDT = tmpDT.addSecs(j);
    anEvent->setAlarmTime(tmpDT);
    if (!General->alarmProgram.isEmpty() && 
	General->alarmProgramButton->isOn())
      anEvent->setProgramAlarmFile(General->alarmProgram);
    else
      anEvent->setProgramAlarmFile("");
    if (!General->alarmSound.isEmpty() &&
	General->alarmSoundButton->isOn())
      anEvent->setAudioAlarmFile(General->alarmSound);
    else
      anEvent->setAudioAlarmFile("");
  } else {
    anEvent->setAlarmRepeatCount(0);
    anEvent->setProgramAlarmFile("");
    anEvent->setAudioAlarmFile("");
  }
  
  // note, that if on the details tab the "Transparency" option is implemented,
  // we will have to change this to suit.
  anEvent->setTransparency(General->freeTimeCombo->currentItem());

  /******************************* RECURRENCE WIN ************************/
  // get recurrence information
  // need a check to see if recurrence is enabled...
  if (General->recursButton->isChecked()) {

    int rDuration;
    QDate rEndDate;
    
    // clear out any old settings;
    anEvent->unsetRecurs();

    // first get range information.  It is common to all types
    // of recurring events.
    if (Recurrence->noEndDateButton->isChecked()) {
      rDuration = -1;
    } else if (Recurrence->endDurationButton->isChecked()) {
      tmpStr = Recurrence->endDurationEdit->text();
      rDuration = tmpStr.toInt();
    } else {
      rDuration = 0;
      rEndDate = Recurrence->endDateEdit->getDate();
    }
    
    // check for daily recurrence
    if (Recurrence->dailyButton->isChecked()) {
      int rFreq;
      
      tmpStr = Recurrence->nDaysEntry->text();
      rFreq = tmpStr.toInt();
      if (rDuration != 0)
	anEvent->setRecursDaily(rFreq, rDuration);
      else
	anEvent->setRecursDaily(rFreq, rEndDate);
      // check for weekly recurrence
    } else if (Recurrence->weeklyButton->isChecked()) {
      int rFreq;
      QBitArray rDays(7);
      
      tmpStr = Recurrence->nWeeksEntry->text();
      rFreq = tmpStr.toInt();

      Recurrence->getCheckedDays(rDays);
      
      if (rDuration != 0)
	anEvent->setRecursWeekly(rFreq, rDays, rDuration);
      else
	anEvent->setRecursWeekly(rFreq, rDays, rEndDate);
    } else if (Recurrence->monthlyButton->isChecked()) {
      if (Recurrence->onNthTypeOfDay->isChecked()) {
	// it's by position
	int rFreq, rPos;
	QBitArray rDays(7);
	
	tmpStr = Recurrence->nMonthsEntry->text();
	rFreq = tmpStr.toInt();
	rDays.fill(FALSE);
	rPos = Recurrence->nthNumberEntry->currentItem() + 1;
	rDays.setBit(Recurrence->nthTypeOfDayEntry->currentItem());
	if (rDuration != 0)
	  anEvent->setRecursMonthly(KOEvent::rMonthlyPos, rFreq, rDuration);
	else
	  anEvent->setRecursMonthly(KOEvent::rMonthlyPos, rFreq, rEndDate);
	anEvent->addRecursMonthlyPos(rPos, rDays);
      } else {
	// it's by day
	int rFreq;
	short rDay;
	
	tmpStr = Recurrence->nMonthsEntry->text();
	rFreq = tmpStr.toInt();
	
	rDay = Recurrence->nthDayEntry->currentItem() + 1;
	
	if (rDuration != 0)
	  anEvent->setRecursMonthly(KOEvent::rMonthlyDay, rFreq, rDuration);
	else
	  anEvent->setRecursMonthly(KOEvent::rMonthlyDay, rFreq, rEndDate);
	anEvent->addRecursMonthlyDay(rDay);
      }
    } else if (Recurrence->yearlyButton->isChecked()) {
      if (Recurrence->yearMonthButton->isChecked()) {
	int rFreq, rMonth;

	tmpStr = Recurrence->nYearsEntry->text();
	rFreq = tmpStr.toInt();
	rMonth = Recurrence->yearMonthComboBox->currentItem() + 1;
	if (rDuration != 0)
	  anEvent->setRecursYearly(KOEvent::rYearlyMonth, rFreq, rDuration);
	else
	  anEvent->setRecursYearly(KOEvent::rYearlyMonth, rFreq, rEndDate);
	anEvent->addRecursYearlyNum(rMonth);
      } else {
	// it's by day
	int rFreq;
	int rDay;

	tmpStr = Recurrence->nYearsEntry->text();
	rFreq = tmpStr.toInt();
	
	//tmpStr = Recurrence->yearDayLineEdit->text();
	rDay = anEvent->getDtStart().date().dayOfYear();

	if (rDuration != 0)
	  anEvent->setRecursYearly(KOEvent::rYearlyDay, rFreq, rDuration);
	else
	  anEvent->setRecursYearly(KOEvent::rYearlyDay, rFreq, rEndDate);
	anEvent->addRecursYearlyNum(rDay);
      }
    } // yearly
  } else
    anEvent->unsetRecurs();

  QDateList exDates;
  for (i = 0; i < Recurrence->exceptionList->count(); i++)
    exDates.inSort(dateFromText(Recurrence->exceptionList->text(i)));

  anEvent->setExDates(exDates);
  exDates.clear();
  // we should remove this.
  anEvent->setStatus(Details->statusCombo->currentItem());

  /******************************* ADD THE EVENT ***************************/

  // makeEvent_final:
  
  // allow signals again so other stuff sees this update
  anEvent->blockSignals(FALSE);

  if (addFlag) {
    calendar->addEvent(anEvent);
    emit eventChanged(anEvent, EVENTADDED);
    currEvent = anEvent;
    addFlag = FALSE;
  } else {
    anEvent->setRevisionNum(anEvent->getRevisionNum()+1);
    emit eventChanged(anEvent, EVENTEDITED);
  }
  return anEvent;
}


void EditEventWin::updateCatEdit(QString _str)
{
  General->categoriesLabel->setText(_str.data());
  Details->categoriesLabel->setText(_str.data());

  setModified();
}

void EditEventWin::recurStuffEnable(bool enable)
{
  if (enable) {
    //General->endLabel->hide();
    General->startDateEdit->hide();
    General->endDateEdit->hide();
  } else {
    //General->endLabel->show();
    General->startDateEdit->show();
    General->endDateEdit->show();
  }
  tabWidget->setTabEnabled(Recurrence, enable);

  setModified();
}

void EditEventWin::startTimeChanged(QTime newtime, int wrapval)
{
  int secsep;

  secsep = currStartDateTime.secsTo(currEndDateTime);
  
  currStartDateTime = currStartDateTime.addDays(wrapval);
  currStartDateTime.setTime(newtime);

  currEndDateTime = currStartDateTime.addSecs(secsep);
  
  //  debug("startTimeChanged, start date: %s, end date: %s\n",
  //	currStartDateTime.toString().data(),
  //	currEndDateTime.toString().data());

  Recurrence->startDateEdit->setDate(currStartDateTime.date());

  Recurrence->startTimeEdit->setTime(currStartDateTime.time());
  Recurrence->endTimeEdit->setTime(currEndDateTime.time());

  Recurrence->exceptionDateEdit->setDate(currStartDateTime.date());

  General->startDateEdit->setDate(currStartDateTime.date());
  General->endDateEdit->setDate(currEndDateTime.date());

  General->startTimeEdit->setTime(currStartDateTime.time());
  General->endTimeEdit->setTime(currEndDateTime.time());

  setDuration();

  setModified();
}

void EditEventWin::endTimeChanged(QTime newtime, int wrapval)
{
  QDateTime newdt(currEndDateTime.addDays(wrapval).date(), newtime);

  if(newdt < currStartDateTime) {
    // oops, can't let that happen.
    newdt = currStartDateTime;
  }
  currEndDateTime = newdt;
  
  General->endDateEdit->setDate(currEndDateTime.date());
  General->endTimeEdit->setTime(currEndDateTime.time());

  Recurrence->endTimeEdit->setTime(currEndDateTime.time());  

  setDuration();

  setModified();
}

void EditEventWin::startDateChanged(QDate newdate)
{
  int daysep;
  daysep = currStartDateTime.daysTo(currEndDateTime);
  
  currStartDateTime.setDate(newdate);
  currEndDateTime.setDate(currStartDateTime.date().addDays(daysep));

  Recurrence->startDateEdit->setDate(currStartDateTime.date());

  General->startDateEdit->setDate(currStartDateTime.date());
  General->endDateEdit->setDate(currEndDateTime.date());

  if (newdate > Recurrence->endDateEdit->getDate())
    Recurrence->endDateEdit->setDate(newdate);

  setDuration();

  setModified();
}

void EditEventWin::endDateChanged(QDate newdate)
{
  QDateTime newdt(newdate, currEndDateTime.time());

  if(newdt < currStartDateTime) {
    // oops, we can't let that happen.
    newdt = currStartDateTime;
    General->endDateEdit->setDate(newdt.date());
    General->endTimeEdit->setTime(newdt.time());

    Recurrence->endTimeEdit->setTime(newdt.time());
  }
  currEndDateTime = newdt;

  setDuration();

  setModified();
}

void EditEventWin::setDuration()
{
  QString tmpStr, catStr;
  int hourdiff, minutediff;

  if (General->noTimeButton->isChecked()) {
    tmpStr.sprintf((const char*)i18n("Duration: all day"));
  } else {
    hourdiff = currStartDateTime.date().daysTo(currEndDateTime.date()) * 24;
    hourdiff += currEndDateTime.time().hour() - 
      currStartDateTime.time().hour();
    minutediff = currEndDateTime.time().minute() -
      currStartDateTime.time().minute();
    // If minutediff is negative, "borrow" 60 minutes from hourdiff
    if (minutediff < 0 && hourdiff > 0) {
      hourdiff -= 1;
      minutediff += 60;
    }
    if (hourdiff || minutediff){
      tmpStr.sprintf((const char*)i18n("Duration: "));
      if (hourdiff){
        if (hourdiff > 1)
          catStr.sprintf((const char*)i18n("%i hours"), hourdiff);
        else if (hourdiff == 1)
            catStr.sprintf((const char*)i18n("%i hour"), hourdiff);
        tmpStr.append(catStr);
      }
      if (hourdiff && minutediff){
        catStr.sprintf((const char*)i18n(", "));
        tmpStr += catStr;
      }
      if (minutediff){
        if (minutediff > 1)
          catStr.sprintf((const char*)i18n("%i minutes"), minutediff);
        else if (minutediff == 1)
          catStr.sprintf((const char*)i18n("%i minute"), minutediff);
        tmpStr += catStr;
      }
    } else
       tmpStr = "";

/*
    if (hourdiff && minutediff)
      tmpStr.sprintf((const char*)i18n("Duration: %i hour(s), %i minute(s)"), hourdiff, minutediff);
    else if (hourdiff)
      tmpStr.sprintf((const char*)i18n("Duration: %i hour(s)"), hourdiff);
    else if (minutediff)
      tmpStr.sprintf((const char*)i18n("Duration: %i minute(s)"), minutediff);
    else
      tmpStr = "";
*/
  }
  Recurrence->durationLabel->setText(tmpStr.data());
  Recurrence->durationLabel->adjustSize();

  setModified();
}

/*****************************************************************************/
void EditEventWin::prevEvent()
{
  if (!prevNextUsed) {
    calendar->prev();
    prevNextUsed = TRUE;
  }
  KOEvent *anEvent(calendar->prev());
  if (anEvent) // it might be NULL.
    editEvent(anEvent, anEvent->getDtStart().date());
}

void EditEventWin::nextEvent()
{
  if (!prevNextUsed) {
    calendar->next();
    prevNextUsed = TRUE;
  }
  KOEvent *anEvent(calendar->next());
  if (anEvent) // it might be NULL.
    editEvent(anEvent, anEvent->getDtStart().date());
}

void EditEventWin::deleteEvent()
{
  if (!addFlag) {
      calendar->deleteEvent(currEvent->getDtStart().date(),
			    currEvent->getEventId());
  }
  emit eventChanged(currEvent, EVENTDELETED);
  close();
}
