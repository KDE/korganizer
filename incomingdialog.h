#ifndef INCOMINGDIALOG_H
#define INCOMINGDIALOG_H
// $Id§

#include <qlistview.h>

#include "scheduler.h"

#include "incomingdialog_base.h"

class KOEvent;
class CalObject;

class ScheduleItemIn : public QListViewItem
{
  public:
    ScheduleItemIn(QListView *parent,KOEvent *ev,Scheduler::Method method,
                   icalclass status);
    virtual ~ScheduleItemIn() {}

    KOEvent *event() { return mEvent; }
    Scheduler::Method method() { return mMethod; }
    icalclass status() { return mStatus; }

  private:
    KOEvent *mEvent;
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
