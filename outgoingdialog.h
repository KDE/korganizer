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
