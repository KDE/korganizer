/*
    This file is part of the KOrganizer interfaces.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef ALARMCLIENT_H
#define ALARMCLIENT_H

#include <qstring.h>
#include <qstringlist.h>

/**
  This class provides the abstract interface for communicating with the alarm
  daemon. It has to be implemented by subclasses for specific daemons. The
  default implementation does nothing.
*/
class AlarmClient
{
  public:
    /**
      Start alarm daemon.
    */
    virtual void startDaemon() = 0;

    /**
      Set the list of monitored calendars. Deletes previous settings.
    */
    virtual bool setCalendars( const QStringList & ) = 0;

    /**
      Add calendar for monitoring by alarm daemon.
    */
    virtual bool addCalendar( const QString & ) = 0;
    
    /**
      Remove calendar from monitoring by alarm daemon.
    */
    virtual bool removeCalendar( const QString & ) = 0;

    /**
      Reload calendar at URL.
    */
    virtual bool reloadCalendar( const QString & ) = 0;    
};

#endif
