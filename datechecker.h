/*
    This file is part of KOrganizer.

    Copyright (c) 2002 Adriaan de Groot <groot@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef DATECHECKER_H
#define DATECHECKER_H

#include <qobject.h>
#include <qdatetime.h>

class QTimer;

class KCalendarSystem;

class NavigatorBar;

class DateChecker: public QObject
{
    Q_OBJECT
  public:
    DateChecker( QObject *parent = 0, const char *name = 0 );
    ~DateChecker();

    /**
      The DateChecker automatically checks for
      the passage of midnight. If rollover type is
      set to None, no signals are emitted and no
      processing is done. With rollover set to
      FollowDay, the day highlighter changes at
      midnight and dayPassed() is emitted.
      With FollowMonth, it has the same effect
      as FollowDay but also adjusts the month that is
      visible and emits monthPassed() when the month changes.
    */
    enum RolloverType { None, FollowDay, FollowMonth };
    void enableRollover( RolloverType );

  signals:
    // Signals emitted at midnight carrying the new date.
    void dayPassed( const QDate & );
    void monthPassed( const QDate & );

  protected slots:
     /**
       Called regularly to see if we need to update the view
       wrt. the today box and the month box. Only important
       if you leave KOrganizer idle for long periods of time.
     
       Until we have a reliable way of setting QTimers to go
       off at a particular wall-clock time, we need this,
       which calls passedMidnight() at the right moments.
     */
     void possiblyPastMidnight();

     /**
       Handles updating the view when midnight has come by due to idle time.
     */
     void passedMidnight();

  private:
    QTimer *mUpdateTimer;
    QDate mLastDayChecked;
    RolloverType mUpdateRollover;
};

#endif
