/*
    This file is part of KOrganizer.
    Copyright (c) 2002,2003 Cornelius Schumacher <schumacher@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "simplealarmclient.h"

#include <kprocess.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include <qfile.h>
#include <qtextstream.h>

SimpleAlarmClient::SimpleAlarmClient()
  : mProcess( 0 )
{
  mCalendarsFile = locateLocal( "data", "simplealarmdaemon/calendars" );

  kdDebug() << "SimpleAlarmClient(): file: " << mCalendarsFile << endl;
}

SimpleAlarmClient::~SimpleAlarmClient()
{
  delete mProcess;
}

void SimpleAlarmClient::startDaemon()
{
  kdDebug() << "SimpleAlarmClient::startDaemon()" << endl;

  if ( !mProcess ) {
    mProcess = new KProcess;
    *mProcess << "simplealarmdaemon";
    if ( !mProcess->start() ) {
      kdDebug() << "Failed to start process." << endl;
    }
  }
}

bool SimpleAlarmClient::setCalendars( const QStringList &calendars )
{
  kdDebug() << "SimpleAlarmClient::setCalendars()" << endl;

  QFile f( mCalendarsFile );
  if ( !f.open( IO_WriteOnly ) ) {
    kdDebug() << "Unable to open file '" << mCalendarsFile << "'" << endl;
    return false;
  }
  QTextStream ts( &f );
  QStringList::ConstIterator it;
  for ( it = calendars.begin(); it != calendars.end(); ++it ) {
    kdDebug() << "CAL: " << *it << endl;
    ts << *it << "\n";
  }
  f.close();
  
  return true;
}

bool SimpleAlarmClient::addCalendar( const QString &calendar )
{
  QFile f( mCalendarsFile );
  if ( !f.open( IO_WriteOnly | IO_Append ) ) return false;
  QTextStream ts( &f );
  ts << calendar << "\n";
  f.close();

  return true;
}

bool SimpleAlarmClient::removeCalendar( const QString &calendar )
{
  QStringList calendars;

  QFile f( mCalendarsFile );
  if ( !f.open( IO_ReadOnly ) ) return false;
  QTextStream ts( &f );
  bool found = false;
  QString line;
  while ( !( line = ts.readLine() ).isNull() ) {
    if ( line != calendar ) calendars.append( line );
    else found = true;
  }

  if ( found ) return setCalendars( calendars );
  else return true;
}

bool SimpleAlarmClient::reloadCalendar( const QString & )
{
  return true;
}
