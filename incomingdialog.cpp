// $Id$

#include <qlistview.h>

#include <kdebug.h>

#include <libkcal/incidence.h>
#include <libkcal/dummyscheduler.h>
#include <libkcal/calendar.h>

#include "incomingdialog.h"

ScheduleItemIn::ScheduleItemIn(QListView *parent,Incidence *ev,
                               Scheduler::Method method,ScheduleMessage::Status status)
  : QListViewItem(parent)
{
  mEvent = ev;
  mMethod = method;
  mStatus = status;
  
  setText(0,ev->summary());
  setText(1,Scheduler::methodName(mMethod));
  setText(2,ScheduleMessage::statusName(status));
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
  mScheduler = new DummyScheduler(mCalendar);
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
    new ScheduleItemIn(mMessageListView,event,method,status);
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
  if (mScheduler->acceptTransaction(item->event(),item->status())) {
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
    delete item;
    emit numMessagesChanged(mMessageListView->childCount());
  }
}
#include "incomingdialog.moc"
