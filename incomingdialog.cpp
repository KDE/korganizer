/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

// $Id$

#include <qlistview.h>
#include <qfile.h>
#include <qmap.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include <libkcal/incidence.h>
#include <libkcal/event.h>
#include <libkcal/calendar.h>

#ifndef KORG_NOMAIL
#include "mailscheduler.h"
#else
#include <libkcal/dummyscheduler.h>
#endif

#include "incomingdialog.h"
#include "koeventviewerdialog.h"


ScheduleItemIn::ScheduleItemIn(QListView *parent,Incidence *ev,
                               Scheduler::Method method,ScheduleMessage::Status status)
  : QListViewItem(parent)
{
  mEvent = ev;
  mMethod = method;
  mStatus = status;
  setText(6,Scheduler::methodName(mMethod)+" ");
  setText(7,ScheduleMessage::statusName(status));
}


/* Visitor */
ScheduleItemVisitor::ScheduleItemVisitor(ScheduleItemIn *item)
{
  mItem = item;
}

ScheduleItemVisitor::~ScheduleItemVisitor()
{
}

bool ScheduleItemVisitor::visit(Event *e)
{
  mItem->setText(0,e->summary());
  mItem->setText(1,e->dtStartDateStr());
  if (e->doesFloat()) {
    mItem->setText(2,"no time ");
    mItem->setText(4,"no time ");
  }
  else {
    mItem->setText(2,e->dtStartTimeStr());
    mItem->setText(4,e->dtEndTimeStr());
  }
  if (e->hasEndDate()) {
    mItem->setText(3,e->dtEndDateStr());
  }
  else {
    mItem->setText(3,"");
  }
  mItem->setText(5,e->organizer()+" ");

  return true;
}

bool ScheduleItemVisitor::visit(Todo *)
{
  return false;
}

bool ScheduleItemVisitor::visit(Journal *)
{
  return false;
}


/*
 *  Constructs a IncomingDialog which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
IncomingDialog::IncomingDialog(Calendar *calendar,QWidget* parent,
                               const char* name,bool modal,WFlags fl) :
  IncomingDialog_base(parent,name,modal,fl)
{
  mCalendar = calendar;
#ifndef KORG_NOMAIL
  mScheduler = new MailScheduler(mCalendar);
  retrieve();
#else
  mScheduler = new DummyScheduler(mCalendar);
#endif
  mMessageListView->setColumnAlignment(1,AlignHCenter);
  mMessageListView->setColumnAlignment(2,AlignHCenter);
  mMessageListView->setColumnAlignment(3,AlignHCenter);
  mMessageListView->setColumnAlignment(4,AlignHCenter);
  QObject::connect(mMessageListView,SIGNAL(doubleClicked(QListViewItem *)),
                   this,SLOT(showEvent(QListViewItem *)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
IncomingDialog::~IncomingDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

void IncomingDialog::retrieve()
{
  QPtrList <ScheduleMessage> messages = mScheduler->retrieveTransactions();

  ScheduleMessage *message;
  for(message = messages.first();message;message = messages.next()) {
    Incidence *event = message->event();
    Scheduler::Method method = (Scheduler::Method)message->method();
    ScheduleMessage::Status status = message->status();

    kdDebug() << "IncomingDialog::retrieve(): summary: " << event->summary()
              << "  method: " << Scheduler::methodName(method) << endl;
    ScheduleItemIn *item = new ScheduleItemIn(mMessageListView,event,method,status);
    ScheduleItemVisitor v(item);
    if (!event->accept(v)) delete item;

  }
  emit numMessagesChanged(mMessageListView->childCount());
}

void IncomingDialog::acceptAllMessages()
{
  bool success = false;

  ScheduleItemIn *item = (ScheduleItemIn *)mMessageListView->firstChild();
  while(item) {
    ScheduleItemIn *nextitem = (ScheduleItemIn *)(item->nextSibling());
    if (acceptMessage(item)) success = true;
    item = nextitem;
  }

  if (success) emit calendarUpdated();
}

void IncomingDialog::acceptMessage()
{
  ScheduleItemIn *item = (ScheduleItemIn *)mMessageListView->selectedItem();
  if (item) {
    if (acceptMessage(item)) emit calendarUpdated();
  }
}

bool IncomingDialog::acceptMessage(ScheduleItemIn *item)
{
  if (mScheduler->acceptTransaction(item->event(),item->method(),item->status())) {
    delete item;
    emit numMessagesChanged(mMessageListView->childCount());
    return true;
  } else {
    kdDebug() << "IncomingDialog::acceptMessage(): Error!" << endl;
    return false;
  }
}

void IncomingDialog::rejectMessage()
{
  ScheduleItemIn *item = (ScheduleItemIn *)mMessageListView->selectedItem();
  if (item) {
    mScheduler->deleteTransaction(item->event());
    delete item;
    emit numMessagesChanged(mMessageListView->childCount());
  }
}

void IncomingDialog::showEvent(QListViewItem *item)
{
  Incidence *incidence = ((ScheduleItemIn *)item)->event();
  if( incidence && incidence->type() == "Event" ) {
    Event *event = static_cast<Event *>(incidence);
    KOEventViewerDialog *eventViewer = new KOEventViewerDialog(this);
    eventViewer->setEvent(event);
    eventViewer->show();
  }
}

#include "incomingdialog.moc"
