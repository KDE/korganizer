#ifndef OUTGOINGDIALOG_H
#define OUTGOINGDIALOG_H
// $Id$

#include <qlistview.h>

#include "scheduler.h"

#include "outgoingdialog_base.h"

class ScheduleItemOut : public QListViewItem
{
  public:
    ScheduleItemOut(QListView *parent,KOEvent *ev,Scheduler::Method method,
                 const QString &recipients=QString::null);
    virtual ~ScheduleItemOut() {}

    KOEvent *event() { return mEvent; }
    Scheduler::Method method() { return mMethod; }
    QString recipients() { return mRecipients; }

  private:
    KOEvent *mEvent;
    Scheduler::Method mMethod;
    QString mRecipients;
};

class OutgoingDialog : public OutgoingDialog_base
{ 
    Q_OBJECT
  public:
    OutgoingDialog(CalObject *,QWidget* parent=0,const char* name=0,
                   bool modal=false,WFlags fl=0);
    ~OutgoingDialog();

    bool addMessage(KOEvent *,Scheduler::Method);
    bool addMessage(KOEvent *,Scheduler::Method,const QString &recipients);

  signals:
    void numMessagesChanged(int);

  protected slots:
    void send();
  
  private:
    CalObject *mCalendar;
    Scheduler *mScheduler;
};

#endif // OUTGOINGDIALOG_H
