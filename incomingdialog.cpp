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

#include <qlistview.h>
#include <qfile.h>
#include <qdir.h>
#include <qmap.h>
#include <qpushbutton.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include <libkcal/incidence.h>
#include <libkcal/event.h>
#include <libkcal/calendar.h>
#include <libkcal/freebusy.h>
#include <libkcal/attendee.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcecalendar.h>

#ifndef KORG_NOMAIL
#include "mailscheduler.h"
#else
#include <libkcal/dummyscheduler.h>
#endif

#ifndef KORG_NOKABC
 #include <kabc/stdaddressbook.h>
#endif

#include "incomingdialog.h"
#include "koeventviewerdialog.h"
#include "kocounterdialog.h"
#include "koprefs.h"
#include "kogroupware.h"
#include "freebusymanager.h"

ScheduleItemIn::ScheduleItemIn(QListView *parent,IncidenceBase *ev,
                               Scheduler::Method method,ScheduleMessage::Status status)
  : QListViewItem(parent)
{
  mIncidence = ev;
  mMethod = method;
  mStatus = status;
  setText(6,Scheduler::translatedMethodName(mMethod)+" ");
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
    mItem->setText(2,i18n("no time "));
    mItem->setText(4,i18n("no time "));
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
  mItem->setText(5,e->organizer().fullName() );

  return true;
}

bool ScheduleItemVisitor::visit(Todo *e)
{
  mItem->setText(0,e->summary());
  if (e->hasStartDate()) {
    mItem->setText(1,e->dtStartDateStr());
    if (!e->doesFloat()) {
      mItem->setText(2,e->dtStartTimeStr());
    }
  }
  if (e->hasDueDate()) {
    mItem->setText(1,e->dtDueDateStr());
    if (!e->doesFloat()) {
      mItem->setText(2,e->dtDueTimeStr());
    }
  }
  mItem->setText(5,e->organizer().fullName() );

  return true;
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
 *  true to construct a modal dialog.
 */
IncomingDialog::IncomingDialog( Calendar *calendar, OutgoingDialog * outgoing,
                                QWidget *parent, const char *name, bool modal,
                                WFlags fl )
  : IncomingDialog_base( parent, name, modal, fl )
{
  mCalendar = calendar;
  mOutgoing = outgoing;
#ifndef KORG_NOMAIL
  mScheduler = new MailScheduler( mCalendar );
#else
  mScheduler = new DummyScheduler( mCalendar );
#endif
  mScheduler->setFreeBusyCache( KOGroupware::instance()->freeBusyManager() );
  mMessageListView->setColumnAlignment( 1, AlignHCenter );
  mMessageListView->setColumnAlignment( 2, AlignHCenter );
  mMessageListView->setColumnAlignment( 3, AlignHCenter );
  mMessageListView->setColumnAlignment( 4, AlignHCenter );
  connect( mMessageListView, SIGNAL( doubleClicked( QListViewItem * ) ),
           SLOT( showEvent( QListViewItem * ) ) );
  connect( mMessageListView, SIGNAL( selectionChanged() ),
	   SLOT( updateActions() ) );
  retrieve();
}

/*
 *  Destroys the object and frees any allocated resources
 */
IncomingDialog::~IncomingDialog()
{
  delete mScheduler;
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
    IncidenceBase *inc = message->event();
    Scheduler::Method method = (Scheduler::Method)message->method();
    ScheduleMessage::Status status = message->status();

    ScheduleItemIn *item = new ScheduleItemIn(mMessageListView,inc,method,status);
    if(inc->type()!="FreeBusy") {
      Incidence *incidence = static_cast<Incidence *>(inc);
      ScheduleItemVisitor v(item);
      if (!incidence->accept(v)) delete item;
    } else {
      FreeBusy *fb = static_cast<FreeBusy *>(item->event());
      item->setText(0, "FreeBusy");
      item->setText(1, KGlobal::locale()->formatDate( fb->dtStart().date() ) );
      item->setText(2, KGlobal::locale()->formatTime( fb->dtStart().time() ) );
      item->setText(3, KGlobal::locale()->formatDate( fb->dtEnd().date() ) );
      item->setText(4, KGlobal::locale()->formatTime( fb->dtEnd().time() ) );
      item->setText(5, fb->organizer().fullName());
    }
    automaticAction(item);
  }
  emit numMessagesChanged(mMessageListView->childCount());

  updateActions();
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
    case Scheduler::Add:
        return incomeAdd(item);
        break;
    case Scheduler::Request:
        return incomeRequest(item);
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
  IncidenceBase *incidence = ((ScheduleItemIn *)item)->event();
  if( incidence && incidence->type() == "Event" ) {
    Event *event = static_cast<Event *>(incidence);
    KOEventViewerDialog *eventViewer = new KOEventViewerDialog(this);
    eventViewer->setEvent(event);
    eventViewer->show();
  }
}

bool IncomingDialog::incomeRefresh(ScheduleItemIn *item)
{
  Event *ev = mCalendar->event(item->event()->uid());
  if (ev) {
    //user interaction before??
    Attendee::List attList = ev->attendees();
    Attendee::List::ConstIterator it;
    for( it = attList.begin(); it != attList.end(); ++it ) {
      // TODO: Why do we clone the event here?
      Event *event = new Event( *ev );
      // TODO_RK: I don't understand why we use the mOutgoing here?
      mOutgoing->addMessage( event, Scheduler::Request, (*it)->email() );
      delete event;
    }
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

bool IncomingDialog::incomeCounter(ScheduleItemIn *item)
{
  IncidenceBase *incidence = ((ScheduleItemIn *)item)->event();
  // currently only events supportet - attetion at insertion below!
  if ( incidence->type() != "Event" ) return false;

  Event *counterEvent = static_cast<Event *>( incidence );

  Event *even = mCalendar->event(counterEvent->uid());

  KOCounterDialog *eventViewer = new KOCounterDialog(this);
  eventViewer->addText(i18n("counter proposal event","<b>Counter-event:</b><p>"));
  eventViewer->addEvent(counterEvent);
  eventViewer->addText("<hr>");
  eventViewer->addText(i18n("<b>Original event:</b><p>"));
  if (even) eventViewer->addEvent(even);
  else eventViewer->addText(i18n("A corresponding event is missing in your calendar."));
  eventViewer->addText("<hr>");
  eventViewer->addText(i18n("If this counter-event is a good proposal for your event, press 'Accept'. All Attendees will then get the new version of this event"));
  eventViewer->show();

  eventViewer->exec();
  if (eventViewer->result()) {
    kdDebug(5850) << "IncomingDialog::Counter:Accept" << endl;
    int revision = 0;
    if (even) {
      revision = even->revision();
      mCalendar->deleteEvent(even);
    }
    mCalendar->addIncidence(counterEvent);

    even = mCalendar->event(item->event()->uid());
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
  } else {
    kdDebug(5850) << "IncomingDialog::Counter:Decline" << endl;
    //the counter-sender's email is missing...
    //now every attendee gets an declinecounter :-(
    mOutgoing->addMessage(counterEvent,Scheduler::Declinecounter);
    delete item;
    emit numMessagesChanged(mMessageListView->childCount());
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
  Event *even = mCalendar->event(item->event()->uid());
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

bool IncomingDialog::incomeAdd(ScheduleItemIn *item)
{
  IncidenceBase *incidence = ((ScheduleItemIn *)item)->event();
  if (incidence->type() == "Event" ) {
    Event *refr = static_cast<Event *>( incidence );
    mOutgoing->addMessage(refr,Scheduler::Refresh);
    mScheduler->deleteTransaction( incidence );
    delete item;
    emit numMessagesChanged(mMessageListView->childCount());
    return true;
  }
  else {
    kdDebug(5850) << "IncomingDialog::incomeAdd - only Events are supportet yet" << endl;
    mScheduler->deleteTransaction( incidence );
    delete item;
    emit numMessagesChanged(mMessageListView->childCount());
    return false;
  }
}

bool IncomingDialog::incomeDefault(ScheduleItemIn *item)
{
  if (mScheduler->acceptTransaction(item->event(),item->method(),item->status())) {
    delete item;
    emit numMessagesChanged(mMessageListView->childCount());
    return true;
  }
  else {
    KMessageBox::error(this,i18n("Unable to accept the IMIP-message. It may be a problem with the email addresses."));
    kdDebug(5850) << "IncomingDialog::acceptMessage(): Error!" << endl;
    return false;
  }
  return false;
}

bool IncomingDialog::incomeRequest(ScheduleItemIn *item)
{
  if (item->event()->type()=="FreeBusy") {
    //handel freebusy request
    IncidenceBase *inc = item->event();
    QDateTime start = inc->dtStart();
    QDateTime end = start.addDays(inc->duration()/86400);

    FreeBusy *freebusy = new FreeBusy(mCalendar, start, end);
    freebusy->setOrganizer( inc->organizer() );
    Attendee *att = new Attendee(KOPrefs::instance()->fullName(),
                               KOPrefs::instance()->email());
    freebusy->addAttendee(att);

    kdDebug(5850) << "calendarview: schedule_publish_freebusy: startDate: "
      << KGlobal::locale()->formatDateTime( start ) << " End Date: "
      << KGlobal::locale()->formatDateTime( end ) << endl;

    if (mOutgoing->addMessage(freebusy,Scheduler::Reply)) {
      delete item;
      emit numMessagesChanged(mMessageListView->childCount());
      delete(freebusy);
      return true;
    }
    return false;
  } else {
    return incomeDefault(item);
  }
  return false;
}

bool IncomingDialog::automaticAction(ScheduleItemIn *item)
{
  bool autoAction = false;
  IncidenceBase *inc = item->event();
  Scheduler::Method method = item->method();

  if( inc->type()=="FreeBusy" ) {
    if ( method==Scheduler::Request ) {
      if ( KOPrefs::instance()->mIMIPAutoFreeBusy==KOPrefs::addressbookAuto ) {
        // reply freebusy information
        if ( checkOrganizerInAddressbook( inc->organizer().email() ) ) {
          incomeRequest(item);
        }
      } else return false;
    } else {

      if ( method==Scheduler::Reply ) {
        if ( KOPrefs::instance()->mIMIPAutoFreeBusy==KOPrefs::addressbookAuto ) {
          // insert freebusy information
          //if ( checkAttendeesInAddressbook(inc) )

        } else return false;
      } else {
        if ( method==Scheduler::Publish) {
          if ( KOPrefs::instance()->mIMIPAutoFreeBusy==KOPrefs::addressbookAuto ) {
            // insert freebusy information
            //if ( checkOrganizerInAddressbook(inc->organizer().email()) )

          }
        } else return false;
      }
    }
  }

  if ( inc->type()=="Event" ) {
    if ( method==Scheduler::Request || method==Scheduler::Publish ) {
      if ( KOPrefs::instance()->mIMIPAutoInsertRequest==KOPrefs::addressbookAuto ) {
        // insert event
        if ( checkOrganizerInAddressbook(inc->organizer().email()) )
          autoAction = acceptMessage(item);
      } else return false;
    } else {

      if ( method==Scheduler::Reply ) {
        if ( KOPrefs::instance()->mIMIPAutoInsertReply==KOPrefs::addressbookAuto ) {
          //  update event information
          if ( checkAttendeesInAddressbook(inc) )
            autoAction = acceptMessage(item);
        } else return false;
      } else {

        if ( method==Scheduler::Refresh ) {
          if ( KOPrefs::instance()->mIMIPAutoRefresh==KOPrefs::addressbookAuto ) {
            // send refresh-information
            if ( checkAttendeesInAddressbook(inc) )
              autoAction = acceptMessage(item);
            else return false;
          } else return false;
        } else return false;
      }
    }
  }
  return autoAction;
}

bool IncomingDialog::checkOrganizerInAddressbook(QString organizer)
{
  bool inBook = false;
#ifndef KORG_NOKABC
  KABC::AddressBook *add_book = KABC::StdAddressBook::self();
  KABC::Addressee::List addressList;
  addressList = add_book->findByEmail(organizer);
  if ( addressList.size()>0 ) inBook = true;
#endif
  return inBook;
}

bool IncomingDialog::checkAttendeesInAddressbook(IncidenceBase *inc)
{
  bool inBook = false;
#ifndef KORG_NOKABC
  KABC::AddressBook *add_book = KABC::StdAddressBook::self();
  KABC::Addressee::List addressList;
  Attendee::List attendees = inc->attendees();
  Attendee::List::ConstIterator it;
  for( it = attendees.begin(); it != attendees.end(); ++it ) {
    addressList = add_book->findByEmail( (*it)->email() );
    if ( addressList.size() > 0 ) inBook = true;
  }
#endif
  return inBook;
}

void IncomingDialog::updateActions()
{
  unsigned int count = 0;
  unsigned int total = 0;
  ScheduleItemIn *item = (ScheduleItemIn *)mMessageListView->firstChild();
  while(item) {
    ScheduleItemIn *nextitem = (ScheduleItemIn *)(item->nextSibling());
    total++;
    if (mMessageListView->isSelected(item)) count++;
    item = nextitem;
  }

  mAcceptAllButton->setEnabled(total>0);
  mAcceptButton->setEnabled(count>0);
  mRejectButton->setEnabled(count>0);
}

#include "incomingdialog.moc"

