/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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
#include "kalarmdclient.h"
#include "kalarmdclient.moc"

#include "kalarmd/alarmdaemoniface_stub.h"

#include <kstandarddirs.h>
#include <kprocess.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>

#include <qstring.h>
#include <qfile.h>

KalarmdClient::KalarmdClient() :
  mAlarmDaemonIface("kalarmd","ad")
{
}

KalarmdClient::~KalarmdClient()
{
}

void KalarmdClient::startDaemon()
{
  if( !kapp->dcopClient()->isApplicationRegistered( "kalarmd" ) ) {
    // Start alarmdaemon. It is a KUniqueApplication, that means it is
    // automatically made sure that there is only one instance of the alarm daemon
    // running.
    QString execStr = locate( "exe", "kalarmd" );
    system( QFile::encodeName( execStr ) );
  }

  if( kapp->dcopClient()->isApplicationRegistered( "korgac" ) ) {
    // Alarm daemon already registered
    return;
  }

  KProcess *proc = new KProcess;
  *proc << "korgac";
  *proc << "--miniicon" << "korganizer";
  connect( proc, SIGNAL( processExited( KProcess* ) ),
           SLOT( startCompleted( KProcess* ) ) );
  if( !proc->start() ) delete proc;
}

void KalarmdClient::startCompleted( KProcess* process )
{
  delete process;

  // Register this application with the alarm daemon
  AlarmDaemonIface_stub stub( "kalarmd", "ad" );
  stub.registerApp( "korgac", "KOrganizer", "ac", 3, true );
  if( !stub.ok() ) {
    kdDebug() << "KalarmdClient::startCompleted(): dcop send failed" << endl;
  }
}

bool KalarmdClient::setCalendars( const QStringList & )
{
  return false;
}

bool KalarmdClient::addCalendar( const QString &url )
{
  mAlarmDaemonIface.addCal( "korgac", url );
  return mAlarmDaemonIface.ok();
}

bool KalarmdClient::removeCalendar( const QString &url )
{
  mAlarmDaemonIface.removeCal( url );
  return mAlarmDaemonIface.ok();
}

bool KalarmdClient::reloadCalendar( const QString &url )
{
  mAlarmDaemonIface.reloadCal( "korgac", url );
  return mAlarmDaemonIface.ok();
}

