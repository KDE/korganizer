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
#include <qvbox.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kdatepik.h>
#include <kiconloader.h>

#include "qdatelist.h"
#include "misc.h"

#include "eventwin.moc"

EventWin::EventWin(CalObject *cal)
  : KTMainWindow("EventWin")
{
  addFlag = TRUE;
  calendar = cal;
  initToolBar();
//  m_frame = new QVBox(this);
  tabWidget = new QTabWidget(this);
//  tabCtl = new KTabCtl(m_frame);
  CHECK_PTR(tabWidget);
  currEvent = 0L;
  Type = TYPE_UNKNOWN; // derived classes MUST set this correctly
  Modified = false;
}



void EventWin::initConstructor()
{
}

void EventWin::newEvent( QDateTime from, QDateTime to, bool allDay )
{
  currEvent = 0L;
  fillInDefaults( from, to, allDay );
  unsetModified();
}

EventWin::~EventWin()
{
  // delete NON-CHILDREN widgets
//  delete catDlg;
//  delete m_frame;
}

/*************************************************************************/
/********************** PROTECTED METHODS ********************************/
/*************************************************************************/

void EventWin::initToolBar()
{
  QPixmap pixmap;

  toolBar = new KToolBar(this);
  addToolBar(toolBar);

  toolBar->enableMoving(false);

  saveExit = new KPTButton( this );
  saveExit->setText(i18n("Save and Close"));
  saveExit->setPixmap(BarIcon("filefloppy"));
  connect( saveExit, SIGNAL(clicked()), SLOT(saveAndClose()) );
  toolBar->insertWidget(0, saveExit->sizeHint().width(), saveExit);

  pixmap = BarIcon("exit");
  toolBar->insertButton(pixmap, 0,
			SIGNAL(clicked()), this,
			SLOT(cancel()), TRUE,
			i18n("Cancel"));

  toolBar->insertSeparator();
//  QFrame *sepFrame = new QFrame(toolBar);
//  sepFrame->setFrameStyle(QFrame::VLine|QFrame::Raised);
//  toolBar->insertWidget(0, 10, sepFrame);

  pixmap = BarIcon("up");
  toolBar->insertButton(pixmap, 0,
			SIGNAL(clicked()), this, 
			SLOT(prevEvent()), TRUE, 
			i18n("Previous Event"));

  pixmap = BarIcon("down");
  toolBar->insertButton(pixmap, 0,
			SIGNAL(clicked()), this,
			SLOT(nextEvent()), TRUE, 
			i18n("Next Event"));
  
  toolBar->insertSeparator();
//  QFrame *sepFrame = new QFrame(toolBar);
//  sepFrame->setFrameStyle(QFrame::VLine|QFrame::Raised);
//  toolBar->insertWidget(0, 10, sepFrame);

  pixmap = BarIcon("delete");
  toolBar->insertButton(pixmap, 50,
			SIGNAL(clicked()), this,
			SLOT(deleteEvent()), TRUE,
			i18n("Delete Event"));
	btnDelete = (KToolBarButton*)toolBar->getWidget(50);
}

void EventWin::fillInFields(QDate qd)
{
}

void EventWin::fillInDefaults(QDateTime from, 
				  QDateTime to, bool allDay)
{
}

// Replacing KTopWidget by KTMainWindow
//void EventWin::closeEvent(QCloseEvent *)
bool EventWin::queryClose()
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

KOEvent* EventWin::makeEvent()
{
  KOEvent *anEvent;

  if (addFlag) {
    anEvent = new KOEvent;
  } else {
    anEvent = currEvent;
  }

  // turn off signal emission when all these changes are going on to
  // avoid a signal storm
  anEvent->blockSignals(TRUE);

  /******************************* GENERAL WIN ***************************/
  // set summary
  anEvent->setSummary(General->summaryEdit->text());

  // set description
  anEvent->setDescription(General->descriptionEdit->text());

  // set categories
  anEvent->setCategories(QString(General->categoriesLabel->text()));

  // set secrecy value
  anEvent->setSecrecy(General->privateButton->isChecked() ? 1 : 0);
  /******************************* DETAILS WIN ***************************/
  // set attendee list

  // this might not be such a good idea because of the modified flag?
  anEvent->clearAttendees();
  unsigned int i;
  for (i = 0; i < Details->mAttendeeList.count(); i++)
    anEvent->addAttendee(new Attendee(*(Details->mAttendeeList.at(i)->
                                      attendee())));

  unsetModified();

  return anEvent;
}

QDate *EventWin::dateFromText(const char *text)
{
  QString tmpStr = text;
  tmpStr.remove(0,4);
  QString name = tmpStr.left(3);

  int y, m, d;

  name = name.upper();

  y = tmpStr.right(4).toInt();
  d = tmpStr.mid(4,2).toInt();
  if (name == "JAN")
    m = 1;
  else if (name == "FEB")
    m = 2;
  else if (name == "MAR")
    m = 3;
  else if (name == "APR")
    m = 4;
  else if (name == "MAY")
    m = 5;
  else if (name == "JUN")
    m = 6;
  else if (name == "JUL")
    m = 7;
  else if (name == "AUG")
    m = 8;
  else if (name == "SEP")
    m = 9;
  else if (name == "OCT")
    m = 10;
  else if (name == "NOV")
    m = 11;
  else if (name == "DEC")
    m = 12;
  else
    // should never get here!
    m = 0;  
  
  return new QDate(y,m,d);
}

void EventWin::cancel()
{
  close();
}

void EventWin::updateCatEdit(QString _str)
{
  General->categoriesLabel->setText(_str.data());
  Details->categoriesLabel->setText(_str.data());
}

bool EventWin::getModified()
{ return Modified; }

void EventWin::setModified()
{
// we're leaving this funtionality turned off for awhile (freeze coming)
/*  if (! Modified) {
    Modified = true;
    setTitle();
  }
*/
}

void EventWin::unsetModified()
{
  if (Modified) {
    Modified = false;
    setTitle();
  }
}

void EventWin::setTitle()
{
  QString  title;

	if (addFlag)
    title = i18n("New");
  else
    title = i18n("Edit");

  switch (Type) {
  case TYPE_TODO :
    title += " ";
    title += i18n("Todo");
    break;
  case TYPE_APPOINTMENT :
    title += " ";
    title += i18n("Appointment");
    break;
  default :
    title += " [";
    title += "unknown event type";
    title += "]";
    break;
  }

  if (currEvent) {
    if (currEvent->isReadOnly()) {
      title += " (";
      title += i18n("readonly");
      title += ")";
    } else if (Modified) {
      title += " (";
      title += i18n("modified");
      title += ")";
    }
	} else {
    if (Modified) {
      title += " (";
      title += i18n("modified");
      title += ")";
    }
	}

  title += " - ";
  title += i18n("KOrganizer");

  setCaption(title);
}

/**
     These probably make sense only for the events
 */

/*****************************************************************************/

void EventWin::saveAndClose()
{
//  if (getModified()) // for later
  makeEvent();
  close();
}
