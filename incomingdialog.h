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
#ifndef INCOMINGDIALOG_H
#define INCOMINGDIALOG_H
// $Id$

#include <qlistview.h>

#include <libkcal/calendar.h>
#include <libkcal/scheduler.h>

#include "incomingdialog_base.h"
#include "outgoingdialog.h"

using namespace KCal;

class ScheduleItemIn : public QListViewItem
{
  public:
    ScheduleItemIn(QListView *parent,IncidenceBase *ev,Scheduler::Method method,
                   ScheduleMessage::Status status);
    virtual ~ScheduleItemIn() {}

    IncidenceBase *event() { return mIncidence; }
    Scheduler::Method method() { return mMethod; }
    ScheduleMessage::Status status() { return mStatus; }

  private:
    IncidenceBase *mIncidence;
    Scheduler::Method mMethod;
    ScheduleMessage::Status mStatus;
};


/**
  This class provides the initialisation of a ScheduleItemIn for calendar
  components using the Incidence::Visitor.
*/
class ScheduleItemVisitor : public Incidence::Visitor
{
  public:
    ScheduleItemVisitor(ScheduleItemIn *);
    ~ScheduleItemVisitor();

    bool visit(Event *);
    bool visit(Todo *);
    bool visit(Journal *);

  private:
    ScheduleItemIn *mItem;
};



class IncomingDialog : public IncomingDialog_base
{
    Q_OBJECT
  public:
    IncomingDialog(Calendar *calendar,OutgoingDialog *outgoing,
            QWidget* parent=0,const char* name=0,bool modal=false,WFlags fl=0);
    ~IncomingDialog();
    
    void setOutgoingDialog(OutgoingDialog *outgoing);

  signals:
    void calendarUpdated();
    void numMessagesChanged(int);

  protected slots:
    void retrieve();
    void acceptAllMessages();
    void acceptMessage();
    void rejectMessage();
    void showEvent(QListViewItem *);

  protected:
    bool acceptMessage(ScheduleItemIn *item);
    bool incomeRefresh(ScheduleItemIn *item);
    bool incomeCounter(ScheduleItemIn *item);
    bool incomeDeclineCounter(ScheduleItemIn *item);
    bool incomeAdd(ScheduleItemIn *item);
    bool incomeDefault(ScheduleItemIn *item);

  private:
    Calendar *mCalendar;
    Scheduler *mScheduler;
    OutgoingDialog *mOutgoing;
};

#endif // INCOMINGDIALOG_H
