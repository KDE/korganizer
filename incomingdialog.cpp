// $Id$

#include <qlistview.h>

#include <kdebug.h>

#include "koevent.h"
#include "dummyscheduler.h"
#include "calobject.h"

#include "incomingdialog.h"

ScheduleItemIn::ScheduleItemIn(QListView *parent,KOEvent *ev,
                               Scheduler::Method method,icalclass status)
  : QListViewItem(parent)
{
  mEvent = ev;
  mMethod = method;
  mStatus = status;
  
  setText(0,ev->getSummary());
  setText(1,Scheduler::methodName(mMethod));
  setText(2,Scheduler::statusName(status));
}


/* 
 *  Constructs a IncomingDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
IncomingDialog::IncomingDialog(CalObject *calendar,QWidget* parent,
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
  QList <ScheduleMessage> messages = mScheduler->retrieveTransactions();

  ScheduleMessage *message;
  for(message = messages.first();message;message = messages.next()) {
    KOEvent *event = message->event();
    Scheduler::Method method = (Scheduler::Method)message->method();
    icalclass status = message->status();
  
    kdDebug() << "IncomingDialog::retrieve(): summary: " << event->getSummary()
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
