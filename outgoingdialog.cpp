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

#include <kdebug.h>

#include <libkcal/event.h>
//#include <libkcal/imipscheduler.h>
#include <libkcal/dummyscheduler.h>

#include "mailscheduler.h"

#include "koprefs.h"
#include "outgoingdialog.h"

ScheduleItemOut::ScheduleItemOut(QListView *parent,Event *ev,
                                 Scheduler::Method method,
                                 const QString &recipients)
  : QListViewItem(parent)
{
  mEvent = ev;
  mMethod = method;
  mRecipients = recipients;
  
  setText(0,ev->summary());
  setText(1,Scheduler::methodName(mMethod));
  if (mMethod == Scheduler::Publish) {
    if (!recipients.isEmpty())
    setText(2,mRecipients);
  }
}

OutgoingDialog::OutgoingDialog(Calendar *calendar,QWidget* parent,
                               const char* name,bool modal,
                               WFlags fl)
    : OutgoingDialog_base(parent,name,modal,fl)
{
  mCalendar = calendar;
  
  if (KOPrefs::instance()->mIMIPScheduler == KOPrefs::IMIPDummy ) {
    kdDebug() << "--- Dummy" << endl;
    mScheduler = new DummyScheduler(mCalendar);
  } else {
    kdDebug() << "--- Mailer" << endl;
    mScheduler = new MailScheduler(mCalendar);
  }
}

OutgoingDialog::~OutgoingDialog()
{
}

bool OutgoingDialog::addMessage(Event *incidence,Scheduler::Method method)
{
  if (method == Scheduler::Publish) return false;

  new ScheduleItemOut(mMessageListView,incidence,method);

  emit numMessagesChanged(mMessageListView->childCount());

  return true;
}

bool OutgoingDialog::addMessage(Event *incidence,Scheduler::Method method,
                                const QString &recipients)
{
  if (method != Scheduler::Publish) return false;
  
  new ScheduleItemOut(mMessageListView,incidence,method,recipients);

  emit numMessagesChanged(mMessageListView->childCount());

  return true;
}

void OutgoingDialog::send()
{
  ScheduleItemOut *item = (ScheduleItemOut *)(mMessageListView->firstChild());
  while(item) {
    bool success;
    if (item->method() == Scheduler::Publish) {
      success = mScheduler->publish(item->event(),item->recipients());
    } else {
      success = mScheduler->performTransaction(item->event(),item->method());
    }
    ScheduleItemOut *oldItem = item;
    item = (ScheduleItemOut *)(item->nextSibling());
    if (success) delete oldItem;
  }

  emit numMessagesChanged(mMessageListView->childCount());
}

#include "outgoingdialog.moc"
