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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kdebug.h>

#include <qfile.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qregexp.h>

#include <kglobal.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kstandarddirs.h>

#include <libkcal/event.h>
#include <libkcal/freebusy.h>
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
#include "docprefs.h"
#include "kogroupware.h"
#include "freebusymanager.h"
#include "docprefs.h"

ScheduleItemOut::ScheduleItemOut(QListView *parent,IncidenceBase *ev,
                                 Scheduler::Method method,
                                 const QString &recipients)
  : QListViewItem(parent)
{
  mIncidence = ev;
  mMethod = method;
  mRecipients = recipients;

//  kdDebug(5850) << "ScheduleItemOut: setting the summary" << endl;
  //Set the summary
  if(ev->type() != "FreeBusy") {
    Incidence *incidence = static_cast<Incidence *>(ev);
    setText(0,incidence->summary());
  } else {
    setText(0,i18n("Free Busy Object"));
  }

//  kdDebug(5850) << "ScheduleItemOut: checking if the object is an event" << endl;
  //If the object is an event
  if(ev->type()=="Event") {
    Event *event = static_cast<Event *>(ev);

    setText(1,event->dtStartDateStr());
    if (event->doesFloat()) {  //If the event floats set the start and end times to no time
      setText(2,i18n("no time"));
      setText(4,i18n("no time"));
    } else {  //If it does not float
      setText(2,event->dtStartTimeStr());
      if (event->hasDuration()) {
        setText(4,event->dtEndTimeStr());
      } else {
        setText(4,i18n("no time"));
      }
    }
    if (event->hasEndDate()) {
      setText(3,event->dtEndDateStr());
    }
    else {
       setText(3,i18n("no time"));
    }
  }

  //If the object is an Todo
  if(ev->type()=="Todo") {
    Todo *event = static_cast<Todo *>(ev);
    if (event->hasStartDate()) {
      setText(1,event->dtStartDateStr());
      if (!event->doesFloat()) {
        setText(2,event->dtStartTimeStr());
      }
    }
    if (event->hasDueDate()) {
      setText(3,event->dtDueDateStr());
      if (!event->doesFloat()) {
        setText(4,event->dtDueTimeStr());
      }
    }
  }

//  kdDebug(5850) << "ScheduleItemOut: checking if the object is a FreeBusy object" << endl;
  //If the object is a freebusy object
  if(ev->type() == "FreeBusy") {
    FreeBusy *freebusy = static_cast<FreeBusy *>(ev);

    setText(1,freebusy->dtStartDateStr());
    setText(2,freebusy->dtStartTimeStr());
    //Must try and get this to the users local settings
    setText(3,KGlobal::locale()->formatDate( freebusy->dtEnd().date() ) );
    setText(4,KGlobal::locale()->formatTime( freebusy->dtEnd().time() ) );
  }

//  kdDebug(5850) << "ScheduleItemOut: Setting the method" << endl;
  //Set the Method
  setText(5,Scheduler::translatedMethodName(mMethod));
}

OutgoingDialog::OutgoingDialog(Calendar *calendar,QWidget* parent,
                               const char* name,bool modal,
                               WFlags fl)
    : OutgoingDialog_base(parent,name,modal,fl)
{
  mCalendar = calendar;

  mFormat = new ICalFormat;

  if (KOPrefs::instance()->mIMIPScheduler == KOPrefs::IMIPDummy ) {
    mScheduler = new DummyScheduler(mCalendar);
  } else {
#ifndef KORG_NOMAIL
    mScheduler = new MailScheduler(mCalendar);
#else
    mScheduler = new DummyScheduler(mCalendar);
#endif
  }
  mScheduler->setFreeBusyCache( KOGroupware::instance()->freeBusyManager() );

  mMessageListView->setColumnAlignment(1,AlignHCenter);
  mMessageListView->setColumnAlignment(2,AlignHCenter);
  mMessageListView->setColumnAlignment(3,AlignHCenter);
  mMessageListView->setColumnAlignment(4,AlignHCenter);
  QObject::connect(mMessageListView,SIGNAL(doubleClicked(QListViewItem *)),
                   this,SLOT(showEvent(QListViewItem *)));
  mDocPrefs = new DocPrefs("groupschedule");
  loadMessages();
}

OutgoingDialog::~OutgoingDialog()
{
  delete mScheduler;
  delete mDocPrefs;
  delete mFormat;
}

bool OutgoingDialog::addMessage(IncidenceBase *incidence,Scheduler::Method method)
{
  kdDebug(5850) << "Outgoing::addMessage" << "Method:" << method << endl;
  if (method == Scheduler::Publish) return false;
  if( mDocPrefs ) {
    if (method != Scheduler::Cancel) {
      mDocPrefs->writeEntry( incidence->uid()+"-scheduled", true );
    } else {
      if (!mDocPrefs->readBoolEntry(incidence->uid()+"-scheduled") )
        return true;
    }
  }

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

bool OutgoingDialog::addMessage(IncidenceBase *incidence,Scheduler::Method method,
                                const QString &recipients)
{
  //if (method != Scheduler::Publish) return false;
  if( mDocPrefs ) {
    if (method != Scheduler::Cancel) {
      mDocPrefs->writeEntry( incidence->uid()+"-scheduled", true );
    } else {
      if (!mDocPrefs->readBoolEntry(incidence->uid()+"-scheduled") )
        return true;
    }
  }
  if (KOPrefs::instance()->mIMIPSend == KOPrefs::IMIPOutbox) {
    new ScheduleItemOut(mMessageListView,incidence,method,recipients);
    saveMessage(incidence,method,recipients);
    emit numMessagesChanged(mMessageListView->childCount());
  }
  else {
    mScheduler->performTransaction(incidence,method,recipients);
  }
  return true;
}

void OutgoingDialog::send()
{
  kdDebug(5850) << "OutgoingDialog::send" << endl;
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
  Event *event = 0;
  Todo *todo = 0;
  if ( item->event()->type()=="Event" ) {
    event = static_cast<Event *>(item->event());
  }
  if ( item->event()->type()=="Todo" ) {
    todo = static_cast<Todo *>(item->event());
  }
  QString sendText;
  if (event || todo) {
    KOEventViewerDialog *eventViewer = new KOEventViewerDialog(this);
    if (event) eventViewer->setEvent(event);
    if (todo) eventViewer->setTodo(todo);
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
    case Scheduler::Reply:
    case Scheduler::Counter: {
      sendText += i18n("The organizer %1").arg( item->event()->organizer().fullName() );
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

bool OutgoingDialog::saveMessage(IncidenceBase *incidence,Scheduler::Method method,
          const QString &recipients)
{
  KTempFile ktfile(locateLocal("data","korganizer/outgoing/"),"ics");
  QString messageText = mFormat->createScheduleMessage(incidence,method);
  QTextStream *qts = ktfile.textStream();
  qts->setEncoding( QTextStream::UnicodeUTF8 );
  *qts << messageText;
  *qts << "METHOD-BEGIN:" << endl << method << endl << ":METHOD-END" << endl;
  *qts << "RECIPIENTS-BEGIN:" << endl << recipients << endl << ":RECIPIENTS-END" << endl;
  mMessageMap[incidence]=ktfile.name();

  return true;
}

bool OutgoingDialog::deleteMessage(IncidenceBase *incidence)
{
  QFile f( mMessageMap[incidence] );
  mMessageMap.remove(incidence);
  if ( !f.exists() ) return false;
  else
    return f.remove();
}

void OutgoingDialog::loadMessages()
{
  Scheduler::Method method;
  QString recipients;

  QString outgoingDirName = locateLocal("data","korganizer/outgoing");
  QDir outgoingDir(outgoingDirName);
  QStringList outgoing = outgoingDir.entryList(QDir::Files);
  QStringList::ConstIterator it;
  for(it = outgoing.begin(); it != outgoing.end(); ++it) {
    kdDebug(5850) << "-- File: " << (*it) << endl;
    QFile f(outgoingDirName + "/" + (*it));
    bool inserted = false;
    QMap<IncidenceBase*, QString>::Iterator iter;
    for ( iter = mMessageMap.begin(); iter != mMessageMap.end(); ++iter ) {
      if (iter.data() == outgoingDirName + "/" + (*it)) inserted = true;
    }
    if (!inserted) {
    if (!f.open(IO_ReadOnly)) {
      kdDebug(5850) << "OutgoingDialog::loadMessage(): Can't open file'"
                << (*it) << "'" << endl;
    } else {
      QTextStream t(&f);
      t.setEncoding( QTextStream::Latin1 );
      QString messageString = t.read();
      messageString.replace( QRegExp("\n[ \t]"), "");
      messageString = QString::fromUtf8( messageString.latin1() );
      ScheduleMessage *message = mFormat->parseScheduleMessage(mCalendar,
                                                               messageString);
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
      kdDebug(5850) << "Outgoing::loadMessage(): Recipients: " << recipients << endl;

      if (message) {
        bool inserted = false;
        QMap<IncidenceBase*, QString>::Iterator iter;
        for ( iter = mMessageMap.begin(); iter != mMessageMap.end(); ++iter ) {
          if (iter.data() == outgoingDirName + "/" + (*it)) inserted = true;
        }
        if (!inserted) {
          kdDebug(5850) << "OutgoingDialog::loadMessage(): got message '"
                    << (*it) << "'" << endl;
          IncidenceBase *inc = message->event();
          new ScheduleItemOut(mMessageListView,inc,method,recipients);
          mMessageMap[message->event()]=outgoingDirName + "/" + (*it);
        }
      } else {
        QString errorMessage;
        if (mFormat->exception()) {
          errorMessage = mFormat->exception()->message();
        }
        kdDebug(5850) << "OutgoingDialog::loadMessage(): Error parsing "
                     "message: " << errorMessage << endl;
      }
      f.close();
    }
    }
  }
  emit numMessagesChanged(mMessageListView->childCount());
}

void OutgoingDialog::setDocumentId( const QString &id )
{
  mDocPrefs->setDoc( id );
}

#include "outgoingdialog.moc"
