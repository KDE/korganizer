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
#ifndef INCOMINGDIALOG_H
#define INCOMINGDIALOG_H

#include <qlistview.h>

#include <libkcal/incidence.h>
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
  This class provides the initialization of a ScheduleItemIn for calendar
  components using the Incidence::Visitor.
*/
class ScheduleItemVisitor : public IncidenceBase::Visitor
{
  public:
    ScheduleItemVisitor(ScheduleItemIn *);
    ~ScheduleItemVisitor();

    bool visit( Event * );
    bool visit( Todo * );
    bool visit( Journal * );
    bool visit( FreeBusy * );

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
  public slots:
    void retrieve();
    
  protected slots:
    void acceptAllMessages();
    void acceptMessage();
    void rejectMessage();
    void showEvent(QListViewItem *);
    void updateActions();

  protected:
    bool acceptMessage(ScheduleItemIn *item);
    bool incomeRefresh(ScheduleItemIn *item);
    bool incomeCounter(ScheduleItemIn *item);
    bool incomeDeclineCounter(ScheduleItemIn *item);
    bool incomeAdd(ScheduleItemIn *item);
    bool incomeRequest(ScheduleItemIn *item);
    bool incomeDefault(ScheduleItemIn *item);
    bool automaticAction(ScheduleItemIn *item);

  private:
    bool checkAttendeesInAddressbook(IncidenceBase *inc);
    bool checkOrganizerInAddressbook( const QString &organizer );
    Calendar *mCalendar;
    Scheduler *mScheduler;
    OutgoingDialog *mOutgoing;
};

#endif // INCOMINGDIALOG_H
