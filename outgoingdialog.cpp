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

#include <qfile.h>
#include <qdir.h>

#include <kglobal.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kstandarddirs.h>

#include <libkcal/event.h>
//#include <libkcal/imipscheduler.h>
#include <libkcal/dummyscheduler.h>
#include <libkcal/icalformat.h>
#include <libkcal/calendar.h>

#ifndef KORG_NOMAIL
#include "mailscheduler.h"
#endif

#include "koprefs.h"
#include "outgoingdialog.h"
#include "koeventviewerdialog.h"

ScheduleItemOut::ScheduleItemOut(QListView *parent,Event *ev,
                                 Scheduler::Method method,
                                 const QString &recipients)
  : QListViewItem(parent)
{
  mEvent = ev;
  mMethod = method;
  mRecipients = recipients;
  setText(0,ev->summary());
  setText(1,ev->dtStartDateStr());
  if (ev->doesFloat()) {
    setText(2,i18n("no time"));
    setText(4,i18n("no time"));
  }
  else {
    setText(2,ev->dtStartTimeStr());
    if (ev->hasDuration()) {
      setText(4,ev->dtEndTimeStr());
    }
    else {
      setText(4,i18n("no time"));
    }
  }
  if (ev->hasEndDate()) {
    setText(3,ev->dtEndDateStr());
  }
  else {
    setText(3,"");
  }
  setText(5,Scheduler::methodName(mMethod));
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
#ifndef KORG_NOMAIL
    kdDebug() << "--- Mailer" << endl;
    mScheduler = new MailScheduler(mCalendar);
#else
    mScheduler = new DummyScheduler(mCalendar);
#endif
  }
  mMessageListView->setColumnAlignment(1,AlignHCenter);
  mMessageListView->setColumnAlignment(2,AlignHCenter);
  mMessageListView->setColumnAlignment(3,AlignHCenter);
  mMessageListView->setColumnAlignment(4,AlignHCenter);
  QObject::connect(mMessageListView,SIGNAL(doubleClicked(QListViewItem *)),
                   this,SLOT(showEvent(QListViewItem *)));
  loadMessages();
}

OutgoingDialog::~OutgoingDialog()
{
}

bool OutgoingDialog::addMessage(Event *incidence,Scheduler::Method method)
{
  if (method == Scheduler::Publish) return false;

  if (KOPrefs::instance()->mIMIPSend == KOPrefs::IMIPOutbox) {
    new ScheduleItemOut(mMessageListView,incidence,method);
    saveMessage(incidence,method);
    emit numMessagesChanged(mMessageListView->childCount());
  }
  else {
    mScheduler->performTransaction(incidence,method);
  }
  return true;
}

bool OutgoingDialog::addMessage(Event *incidence,Scheduler::Method method,
                                const QString &recipients)
{
  if (method != Scheduler::Publish) return false;
  if (KOPrefs::instance()->mIMIPSend == KOPrefs::IMIPOutbox) {
    new ScheduleItemOut(mMessageListView,incidence,method,recipients);
    saveMessage(incidence,method,recipients);
    emit numMessagesChanged(mMessageListView->childCount());
  }
  else {
    mScheduler->publish(incidence,recipients);
  }
  return true;
}

void OutgoingDialog::send()
{
  kdDebug() << "OutgoingDialog::send" << endl;
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
    if (success) {
      deleteMessage(oldItem->event());
      delete (oldItem->event());
      delete oldItem;
    }
  }

  emit numMessagesChanged(mMessageListView->childCount());
}

void OutgoingDialog::deleteItem()
{
  ScheduleItemOut *item = (ScheduleItemOut *)(mMessageListView->selectedItem());
  if(!item)
      return;
  deleteMessage(item->event());
  delete(item->event());
  mMessageListView->takeItem(item);
  emit numMessagesChanged(mMessageListView->childCount());
}

void OutgoingDialog::showEvent(QListViewItem *qitem)
{
  ScheduleItemOut *item = (ScheduleItemOut *)qitem;
  Event *event = item->event();
  QString sendText;
  if (event) {
    KOEventViewerDialog *eventViewer = new KOEventViewerDialog(this);
    eventViewer->setEvent(event);
    sendText = "<hr><h4>"+i18n("Event will be sent to:")+"</h4>";
    switch (item->method()) {
      case Scheduler::Publish: {
        sendText += item->recipients();
        break; }
      case Scheduler::Request: {
        sendText += i18n("All attendees");
        break; }
      case Scheduler::Refresh: {
        sendText += i18n("All attendees");
        break; }
      case Scheduler::Cancel: {
        sendText += i18n("All attendees");
        break; }
      case Scheduler::Add: {
        sendText += i18n("All attendees");
        break; }
      case Scheduler::Reply: {
        sendText += i18n("The organizer %1").arg(item->event()->organizer());
        break; }
      case Scheduler::Counter: {
        sendText += i18n("The organizer %1").arg(item->event()->organizer());
        break; }
      case Scheduler::Declinecounter: {
        sendText += i18n("All attendees");
        break; }
      case Scheduler::NoMethod: {
        sendText += "";
        break; }
      default:
        sendText = "";
    }
    eventViewer->addText(sendText);
    eventViewer->show();
  }
}

bool OutgoingDialog::saveMessage(Incidence *incidence,Scheduler::Method method,
          const QString &recipients)
{
  ICalFormat *mFormat = mCalendar->iCalFormat();
  KTempFile ktfile(locateLocal("data","korganizer/outgoing/"),"ics");
  QString messageText = mFormat->createScheduleMessage(incidence,method);
  QTextStream *qts = ktfile.textStream();
  *qts << messageText;
  *qts << "METHOD-BEGIN:" << endl << method << endl << ":METHOD-END" << endl;
  *qts << "RECIPIENTS-BEGIN:" << endl << recipients << endl << ":RECIPIENTS-END" << endl;
  mMessageMap[incidence]=ktfile.name();
}

bool OutgoingDialog::deleteMessage(Incidence *incidence)
{
  QFile f( mMessageMap[incidence] );
  mMessageMap.remove(incidence);
  if ( !f.exists() ) return false;
  else
    return f.remove();
}

void OutgoingDialog::loadMessages()
{
  ICalFormat *mFormat = mCalendar->iCalFormat();
  Scheduler::Method method;
  QString recipients;

  QString outgoingDirName = locateLocal("appdata","outgoing");
  QDir outgoingDir(outgoingDirName);
  QStringList outgoing = outgoingDir.entryList(QDir::Files);
  QStringList::ConstIterator it;
  for(it = outgoing.begin(); it != outgoing.end(); ++it) {
    kdDebug() << "-- File: " << (*it) << endl;
    QFile f(outgoingDirName + "/" + (*it));
    bool inserted = false;
    QMap<Incidence*, QString>::Iterator iter;
    for ( iter = mMessageMap.begin(); iter != mMessageMap.end(); ++iter ) {
      if (iter.data() == outgoingDirName + "/" + (*it)) inserted = true;
    }
    if (!inserted) {
    if (!f.open(IO_ReadOnly)) {
      kdDebug() << "OutgoingDialog::loadMessage(): Can't open file'"
                << (*it) << "'" << endl;
    } else {
      QTextStream t(&f);
      QString messageString = t.read();
      ScheduleMessage *message = mFormat->parseScheduleMessage(messageString);
      int begin_pos = messageString.find("METHOD-BEGIN:");
      begin_pos = messageString.find('\n',begin_pos)+1;
      QString meth = messageString.mid(begin_pos,1);
      switch (meth.toInt()) {
        case 0:method=Scheduler::Publish; break;
        case 1:method=Scheduler::Request; break;
        case 2:method=Scheduler::Refresh; break;
        case 3:method=Scheduler::Cancel; break;
        case 4:method=Scheduler::Add; break;
        case 5:method=Scheduler::Reply; break;
        case 6:method=Scheduler::Counter; break;
        case 7:method=Scheduler::Declinecounter; break;
        default :method=Scheduler::NoMethod; break;
      }
      begin_pos = messageString.find("RECIPIENTS-BEGIN:");
      begin_pos = messageString.find('\n',begin_pos)+1;
      int end_pos = messageString.find(":RECIPIENTS-END",begin_pos)-1;
      recipients = messageString.mid(begin_pos, end_pos-begin_pos);
      kdDebug() << "Outgoing::loadMessage(): Recipients: " << recipients << endl;

      if (message) {
        kdDebug() << "OutgoingDialog::loadMessage(): got message '"
                  << (*it) << "'" << endl;
        Event *ev=0;
        if (message->event()->type()="Event") {
          ev = static_cast<Event *>(message->event());
        }
        new ScheduleItemOut(mMessageListView,ev,method,recipients);
        emit numMessagesChanged(mMessageListView->childCount());

        mMessageMap[message->event()]=outgoingDirName + "/" + (*it);
      } else {
        QString errorMessage;
        if (mFormat->exception()) {
          errorMessage = mFormat->exception()->message();
        }
        kdDebug() << "OutgoingDialog::loadMessage(): Error parsing "
                     "message: " << errorMessage << endl;
      }
      f.close();
    }
    }
  }
}

#include "outgoingdialog.moc"
