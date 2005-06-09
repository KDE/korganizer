/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "alarmclient.h"

#include <kprocess.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>

AlarmClient::AlarmClient( QObject *o, const char *name )
  : QObject( o, name )
{
  kdDebug() << "AlarmClient::AlarmClient()" << endl;
}

void AlarmClient::startDaemon()
{
  if ( kapp->dcopClient()->isApplicationRegistered( "korgac" ) ) {
    // Alarm daemon already runs
    return;
  }

  KProcess *proc = new KProcess;
  *proc << "korgac";
  *proc << "--miniicon" <<  "korganizer";
  connect( proc, SIGNAL( processExited( KProcess * ) ),
           SLOT( startCompleted( KProcess * ) ) );
  if ( !proc->start() ) delete proc;
}

void AlarmClient::startCompleted( KProcess *process )
{
  delete process;
}

void AlarmClient::stopDaemon()
{
  kapp->dcopClient()->send( "korgac", "ac", "quit()", QByteArray() );
}

#include "alarmclient.moc"
