/*
    KOrganizer Alarm Daemon Client.

    This file is part of KOrganizer.

    Copyright (c) 2002,2003 Cornelius Schumacher

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
#ifndef KOALARMCLIENT_H
#define KOALARMCLIENT_H

#include "alarmclientiface.h"

#include <qtimer.h>

class AlarmDialog;
class AlarmDockWindow;

namespace KCal {
class CalendarResources;
}

class KOAlarmClient : public QObject, virtual public AlarmClientIface
{
    Q_OBJECT
  public:
    KOAlarmClient( QObject *parent = 0, const char *name = 0 );
    ~KOAlarmClient();

    // DCOP interface
    void quit();
    void forceAlarmCheck();
    void dumpDebug();
    QStringList dumpAlarms();

  public slots:
    /**
      Schedule the alarm dialog for redisplay after a specified number of
      minutes.
    */
    void suspend( int minutes );

  protected slots:
    void showAlarmDialog();
    void checkAlarms();    

  private:
    AlarmDockWindow *mDocker;  // the panel icon
    AlarmDialog *mAlarmDialog;
    QTimer mSuspendTimer;

    KCal::CalendarResources *mCalendar;

    QTimer mCheckTimer;
};

#endif
