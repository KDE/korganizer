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
#ifndef OUTGOINGDIALOG_H
#define OUTGOINGDIALOG_H

#include <qlistview.h>
#include <qmap.h>
#include <qstring.h>

#include <libkcal/scheduler.h>

#include "docprefs.h"
#include "outgoingdialog_base.h"

using namespace KCal;

class ScheduleItemOut : public QListViewItem
{
  public:
    ScheduleItemOut(QListView *parent,IncidenceBase *ev,
        Scheduler::Method method, const QString &recipients=QString::null);
    virtual ~ScheduleItemOut() {}

    IncidenceBase *event() { return mIncidence; }
    Scheduler::Method method() { return mMethod; }
    QString recipients() { return mRecipients; }

  private:
    IncidenceBase *mIncidence;
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

    bool addMessage(IncidenceBase *,Scheduler::Method);
    bool addMessage(IncidenceBase *,Scheduler::Method,const QString &recipients);
    void setDocumentId( const QString &id );

  public slots:
    void loadMessages();

  signals:
    void numMessagesChanged(int);

  protected slots:
    void send();
    void deleteItem();
    void showEvent(QListViewItem *);

  private:
    bool saveMessage(IncidenceBase *,Scheduler::Method,const QString &recipients=0);
    bool deleteMessage(IncidenceBase *);

    Calendar *mCalendar;
    ICalFormat *mFormat;
    Scheduler *mScheduler;
    QMap<IncidenceBase*, QString> mMessageMap;
    DocPrefs *mDocPrefs;
};

#endif // OUTGOINGDIALOG_H
