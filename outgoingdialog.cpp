// $Id$

#include "event.h"
//#include "imipscheduler.h"
#include "dummyscheduler.h"

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

/* 
 *  Constructs a OutgoingDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
OutgoingDialog::OutgoingDialog(CalObject *calendar,QWidget* parent,
                               const char* name,bool modal,
                               WFlags fl)
    : OutgoingDialog_base(parent,name,modal,fl)
{
  mCalendar = calendar;
  
  mScheduler = new DummyScheduler(mCalendar);
}

/*  
 *  Destroys the object and frees any allocated resources
 */
OutgoingDialog::~OutgoingDialog()
{
    // no need to delete child widgets, Qt does it all for us
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
