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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "datechecker.h"

#include <QTimer>

DateChecker::DateChecker( QObject *parent ) : QObject( parent ), mUpdateTimer( 0 )
{
  enableRollover( FollowMonth );
}

DateChecker::~DateChecker()
{
}

void DateChecker::enableRollover( RolloverType r )
{
  switch( r ) {
    case None:
      if ( mUpdateTimer ) {
        mUpdateTimer->stop();
        delete mUpdateTimer;
        mUpdateTimer = 0;
      }
      break;
    case FollowDay:
    case FollowMonth:
      if ( !mUpdateTimer ) {
        mUpdateTimer = new QTimer( this );
        connect(mUpdateTimer, &QTimer::timeout, this, &DateChecker::possiblyPastMidnight);
      }
      mUpdateTimer->setSingleShot( true );
      mUpdateTimer->start( 0 );
      mLastDayChecked = QDate::currentDate();
  }
  mUpdateRollover = r;
}

void DateChecker::passedMidnight()
{
  QDate today = QDate::currentDate();

  if ( today.month() != mLastDayChecked.month() ) {
     if ( mUpdateRollover == FollowMonth ) {
       emit monthPassed( today );
     }
  }
  emit dayPassed( today );
}

void DateChecker::possiblyPastMidnight()
{
  if ( mLastDayChecked != QDate::currentDate() ) {
    passedMidnight();
    mLastDayChecked = QDate::currentDate();
  }
  // Set the timer to go off 1 second after midnight
  // or after 8 minutes, whichever comes first.
  if ( mUpdateTimer ) {
    QTime now = QTime::currentTime();
    QTime midnight = QTime( 23, 59, 59 );
    int msecsWait = qMin( 480000, now.msecsTo( midnight ) + 2000 );

    mUpdateTimer->stop();
    mUpdateTimer->start( msecsWait );
  }
}

