#ifndef INCOMINGDIALOG_H
#define INCOMINGDIALOG_H
// $Id§

#include <qlistview.h>

#include "scheduler.h"

#include "incomingdialog_base.h"

class Incidence;
class CalObject;

class ScheduleItemIn : public QListViewItem
{
  public:
    ScheduleItemIn(QListView *parent,Incidence *ev,Scheduler::Method method,
                   icalclass status);
    virtual ~ScheduleItemIn() {}

    Incidence *event() { return mEvent; }
    Scheduler::Method method() { return mMethod; }
    icalclass status() { return mStatus; }

  private:
    Incidence *mEvent;
    Scheduler::Method mMethod;
    icalclass mStatus;
};


class IncomingDialog : public IncomingDialog_base
{ 
    Q_OBJECT
  public:
    IncomingDialog(CalObject *calendar,QWidget* parent=0,const char* name=0,
                   bool modal=false,WFlags fl=0);
    ~IncomingDialog();

  signals:
    void calendarUpdated();
    void numMessagesChanged(int);

  protected slots:
    void retrieve();
    void acceptAllMessages();
    void acceptMessage();
    void rejectMessage();

  protected:
    bool acceptMessage(ScheduleItemIn *item);

  private:
    CalObject *mCalendar;
    Scheduler *mScheduler;
};

#endif // INCOMINGDIALOG_H
