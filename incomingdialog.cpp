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
#include "kocounterdialog.h"


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
IncomingDialog::IncomingDialog(Calendar *calendar,OutgoingDialog *outgoing,
                QWidget* parent,const char* name,bool modal,WFlags fl) :
  IncomingDialog_base(parent,name,modal,fl)
{
  mCalendar = calendar;
  mOutgoing = outgoing;
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

void IncomingDialog::setOutgoingDialog(OutgoingDialog *outgoing)
{
  mOutgoing = outgoing;
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
  switch (item->method()) {
    case Scheduler::Refresh:
        return incomeRefresh(item);
        break;
    case Scheduler::Counter:
        return incomeCounter(item);
        break;
    case Scheduler::Declinecounter:
        return incomeDeclineCounter(item);
        break;
    default:
        return incomeDefault(item);
  }
  return false;
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

bool IncomingDialog::incomeRefresh(ScheduleItemIn *item)
{
  Event *ev = mCalendar->getEvent(item->event()->VUID());
  if (ev) {
    //user interaction before??
    Event *event = new Event(*ev);
    mOutgoing->addMessage(event,Scheduler::Request);
    delete(event);
    delete item;
    emit numMessagesChanged(mMessageListView->childCount());
    return true;
  }
  return false;
}

bool IncomingDialog::incomeCounter(ScheduleItemIn *item)
{
  Event *counterevent = dynamic_cast<Event *>(((ScheduleItemIn *)item)->event());

  Event *even = mCalendar->getEvent(item->event()->VUID());

  KOCounterDialog *eventViewer = new KOCounterDialog(this);
  //eventViewer->addText(i18n("You received a counterevent<p>"));
  //eventViewer->addText(i18n("<hr>"));
  eventViewer->addText(i18n("<b>Counterevent:</b><p>"));
  eventViewer->addEvent(counterevent);
  eventViewer->addText(i18n("<hr>"));
  eventViewer->addText(i18n("<b>Original event:</b><p>"));
  if (even) eventViewer->addEvent(even);
    else eventViewer->addText(i18n("A corresponding event is missing in your canlendar!"));
  eventViewer->addText(i18n("<hr>"));
  eventViewer->addText(i18n("If this counterevent is a good proprosal for your event, press 'Accept'. And all Attendees will get the new version of this event"));
  eventViewer->show();

  eventViewer->exec();
  if (eventViewer->result()) {
    kdDebug() << "IncomingDialog::Counter:Accept" << endl;
    int revision;
    if (even) {
      revision = even->revision();
      mCalendar->deleteEvent(even);
    }
    mCalendar->addIncidence(item->event());
    even = mCalendar->getEvent(item->event()->VUID());
    if (even) {
      if (revision < even->revision())
        even->setRevision(even->revision()+1);
      else
        even->setRevision(revision+1);
      Event *ev = new Event(*even);
      mOutgoing->addMessage(ev,Scheduler::Request);
      delete(ev);
    }
    mScheduler->deleteTransaction(item->event());
    delete item;
    emit numMessagesChanged(mMessageListView->childCount());
    return true;
  }
  else {
    kdDebug() << "IncomingDialog::Counter:Decline" << endl;
    //the counter-sender's email is missing...
    //mOutgoing->addMessage(counterevent,Scheduler::Declinecounter);
    //delete item;
    //emit numMessagesChanged(mMessageListView->childCount());
    mScheduler->deleteTransaction(item->event());
    delete item;
    emit numMessagesChanged(mMessageListView->childCount());
    return true;
  }
  //mScheduler->deleteTransaction(item->event());
  delete item;
  emit numMessagesChanged(mMessageListView->childCount());
  return false;
}

bool IncomingDialog::incomeDeclineCounter(ScheduleItemIn *item)
{
  Event *even = mCalendar->getEvent(item->event()->VUID());
  if (even) {
    mOutgoing->addMessage(even,Scheduler::Refresh);
    mScheduler->deleteTransaction(item->event());
    delete item;
    emit numMessagesChanged(mMessageListView->childCount());
    return true;
  }
  mScheduler->deleteTransaction(item->event());
  delete item;
  emit numMessagesChanged(mMessageListView->childCount());
  return false;
}

bool IncomingDialog::incomeDefault(ScheduleItemIn *item)
{
  if (mScheduler->acceptTransaction(item->event(),item->method(),item->status())) {
    delete item;
    emit numMessagesChanged(mMessageListView->childCount());
    return true;
  }
  else {
    kdDebug() << "IncomingDialog::acceptMessage(): Error!" << endl;
    return false;
  }
  return false;
}

#include "incomingdialog.moc"
