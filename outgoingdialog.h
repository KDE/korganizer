#ifndef OUTGOINGDIALOG_H
#define OUTGOINGDIALOG_H
// $Id$

#include <qlistview.h>

#include <libkcal/scheduler.h>

#include "outgoingdialog_base.h"

using namespace KCal;

class ScheduleItemOut : public QListViewItem
{
  public:
    ScheduleItemOut(QListView *parent,Event *ev,Scheduler::Method method,
                 const QString &recipients=QString::null);
    virtual ~ScheduleItemOut() {}

    Event *event() { return mEvent; }
    Scheduler::Method method() { return mMethod; }
    QString recipients() { return mRecipients; }

  private:
    Event *mEvent;
    Scheduler::Method mMethod;
    QString mRecipients;
};

class OutgoingDialog : public OutgoingDialog_base
{ 
    Q_OBJECT
  public:
    OutgoingDialog(Calendar *,QWidget* parent=0,const char* name=0,
                   bool modal=false,WFlags fl=0);
    ~OutgoingDialog();

    bool addMessage(Event *,Scheduler::Method);
    bool addMessage(Event *,Scheduler::Method,const QString &recipients);

  signals:
    void numMessagesChanged(int);

  protected slots:
    void send();
  
  private:
    Calendar *mCalendar;
    Scheduler *mScheduler;
};

#endif // OUTGOINGDIALOG_H
