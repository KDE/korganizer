/*   $Id$

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

#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>
#include <kdatepik.h>
#include <kiconloader.h>
#include <ktoolbarbutton.h>

#include "qdatelist.h"
#include "misc.h"
#include "todowingeneral.h"

#include "todoeventwin.moc"

TodoEventWin::TodoEventWin(CalObject *cal)
  : EventWin(cal)
{
  mRelatedEvent = 0;
  initConstructor();
  // This should be redundant. No time to try if it really is right now.
  fillInDefaults(QDateTime::currentDateTime(),0,true);
  // anyhow, it is never called like this
  prevNextUsed = FALSE;
  Type = TYPE_TODO;
}

void TodoEventWin::editEvent( KOEvent *event, QDate qd )
{
  // check to see if save & close should be enabled or not
  saveExit->setEnabled(! event->isReadOnly());
  btnDelete->setEnabled(true);

  currEvent = event;
  addFlag = false;
  fillInFields();

  unsetModified();
  setTitle();
}

void TodoEventWin::newEvent( QDateTime due, KOEvent *relatedEvent,
                             bool allDay )
{
  saveExit->setEnabled(true);
  btnDelete->setEnabled(false);

  currEvent = 0L;
  addFlag = true;
  fillInDefaults( due, relatedEvent, allDay );

  unsetModified();
  setTitle();
}

void TodoEventWin::initConstructor()
{
/*
  initToolBar();
  m_frame = new QFrame(this);
  tabCtl = new KTabCtl(m_frame);
  CHECK_PTR(tabCtl);
  */
  QString tmpStr;

//  CHECK_PTR(tabWidget);
  General = new TodoWinGeneral(tabWidget, "General");
  CHECK_PTR(General);
  connect(General, SIGNAL(modifiedEvent()),
    this, SLOT(setModified()));

  Details = new EventWinDetails(tabWidget, "Details", true);
  CHECK_PTR(Details);
//  Details->lower();
//  Details->hide();
  connect(Details, SIGNAL(modifiedEvent()),
    this, SLOT(setModified()));

//  setFrameBorderWidth(0);

  tabWidget->addTab(General, i18n("General"));  
  tabWidget->addTab(Details, i18n("Details"));
//  tabCtl->addTab(General, i18n("General"));  
//  tabCtl->addTab(Details, i18n("Details"));

  setView(tabWidget);
  
  // QTabWidget::minimumSizeHint() seems not to work. This is a workaroud
  tabWidget->setMinimumSize(tabWidget->sizeHint());
   
//  setView(m_frame);
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

  unsetModified();
}

TodoEventWin::~TodoEventWin()
{
  // delete NON-CHILDREN widgets
  delete catDlg;
//  delete m_frame;
}

/*************************************************************************/
/********************** PROTECTED METHODS ********************************/
/*************************************************************************/

void TodoEventWin::fillInFields()
{
  blockSignals(true);

  /******************** GENERAL *******************************/
  General->summaryEdit->setText(currEvent->getSummary());
  General->descriptionEdit->setText(currEvent->getDescription());
  // organizer information
  General->ownerLabel->setText(i18n("Owner: %1").arg(currEvent->getOrganizer()));

  if (currEvent->hasDueDate()) {
    General->startDateEdit->setDate(currEvent->getDtDue().date());
    General->startTimeEdit->setTime(currEvent->getDtDue().time());
    General->noDueButton->setChecked(false);
  } else {
    General->startDateEdit->setDate(QDate::currentDate());
    General->startTimeEdit->setTime(QTime::currentTime());
    General->noDueButton->setChecked(true);
  } 

  General->noTimeButton->setChecked(currEvent->doesFloat());

  if (currEvent->getStatusStr() == "NEEDS ACTION")
    General->completedButton->setChecked(FALSE);
  else
    General->completedButton->setChecked(TRUE);

  General->priorityCombo->setCurrentItem(currEvent->getPriority()-1);


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

  blockSignals(false);
}

void TodoEventWin::fillInDefaults(QDateTime due, KOEvent *relatedEvent,
                                  bool allDay)
{
  QString tmpStr;

  blockSignals(true);

  // default owner is calendar owner.
  General->ownerLabel->setText(tmpStr.sprintf((const char*)i18n("Owner: %s"),
					      calendar->getEmail().data()).data());

  /********************************* GENERAL *******************************/

  General->noTimeButton->setChecked(allDay);
  General->noDueButton->setChecked(true);
  ((TodoWinGeneral *)General)->dueStuffDisable(true);
//  General->alarmStuffDisable(allDay);

  General->startDateEdit->setDate(due.date());
  General->startTimeEdit->setTime(due.time());

  mRelatedEvent = relatedEvent;
  
/*
  KConfig *config(kapp->config());
  config->setGroup("Time & Date");
  QString alarmText(config->readEntry("Default Alarm Time", "15"));
  int pos = alarmText.find(' ');
  if (pos >= 0)
    alarmText.truncate(pos);
  General->alarmTimeEdit->setText(alarmText.data());
  General->alarmStuffEnable(FALSE);
*/

  /******************************** DETAILS ********************************/
  Details->attendeeRSVPButton->setChecked(TRUE);

  // default owner is calendar owner.
/*
  qDebug("Getting email");
  qDebug("e-Mail: %s",(calendar->getEmail()).latin1());
  qDebug("ownerLabelsetText");
  General->ownerLabel->setText("Huhu");
*/  
// This line seems to cause a crash. Have a look at it later.  
//  General->ownerLabel->setText(i18n("Owner: %1").arg(calendar->getEmail()));

//  General->ownerLabel->setText(tmpStr.sprintf(i18n("Owner: %s"),
//					      calendar->getEmail().data()).data());

  blockSignals(false);
}

//void TodoEventWin::closeEvent(QCloseEvent *)
bool TodoEventWin::queryClose()
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

KOEvent* TodoEventWin::makeEvent()
{
  KOEvent *anEvent;
  anEvent = EventWin::makeEvent();

  anEvent->setHasDueDate(!General->noDueButton->isChecked());

//  qDebug("TodoEventWin::makeEvent(): hasDueDate %s", anEvent->hasDueDate() ?
//                                                     "true" : "false" );

  QDate tmpDate;
  QTime tmpTime;
  QDateTime tmpDT;
  bool ok;
  if (General->noTimeButton->isChecked()) {
    anEvent->setFloats(true);

    // need to change this.
    tmpDate = General->startDateEdit->getDate();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    anEvent->setDtDue(tmpDT);
  } else {
    anEvent->setFloats(false);
    
    // set date/time start
    tmpDate = General->startDateEdit->getDate();
    tmpTime = General->startTimeEdit->getTime(ok);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    anEvent->setDtDue(tmpDT);
  } // check for float
  
  anEvent->setPriority(General->priorityCombo->currentItem()+1);

  if (General->completedButton->isChecked()) {
      anEvent->setStatus(QString("COMPLETED"));
  } else {
      anEvent->setStatus(QString("NEEDS ACTION"));
  }

  // set related event, i.e. parent to-do in this case.
  if (mRelatedEvent) {
    anEvent->setRelatedTo (mRelatedEvent);
  }

  if (addFlag) {
    calendar->addTodo(anEvent);
    emit eventChanged(anEvent, EVENTADDED);
    currEvent = anEvent;
    addFlag = FALSE;
  } else {
    anEvent->setRevisionNum(anEvent->getRevisionNum()+1);
    emit eventChanged(anEvent, EVENTEDITED);
  }
  return anEvent;
}

void TodoEventWin::updateCatEdit(QString _str)
{
  General->categoriesLabel->setText(_str);
  Details->categoriesLabel->setText(_str);

  setModified();
}

void TodoEventWin::setDuration()
{
  setModified();
}

/*****************************************************************************/
void TodoEventWin::prevEvent()
{
}

void TodoEventWin::nextEvent()
{
}

void TodoEventWin::deleteEvent()
{
  if (!addFlag) {
      calendar->deleteTodo(currEvent);
  }
  emit eventChanged(currEvent, EVENTDELETED);
  close();
}

