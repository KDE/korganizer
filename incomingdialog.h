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

using namespace KCal;

class ScheduleItemIn : public QListViewItem
{
  public:
    ScheduleItemIn(QListView *parent,Incidence *ev,Scheduler::Method method,
                   ScheduleMessage::Status status);
    virtual ~ScheduleItemIn() {}

    Incidence *event() { return mEvent; }
    Scheduler::Method method() { return mMethod; }
    ScheduleMessage::Status status() { return mStatus; }

  private:
    Incidence *mEvent;
    Scheduler::Method mMethod;
    ScheduleMessage::Status mStatus;
};


class IncomingDialog : public IncomingDialog_base
{ 
    Q_OBJECT
  public:
    IncomingDialog(Calendar *calendar,QWidget* parent=0,const char* name=0,
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
    Calendar *mCalendar;
    Scheduler *mScheduler;
};

#endif // INCOMINGDIALOG_H
