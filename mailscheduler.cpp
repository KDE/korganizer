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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qdir.h>
#include <qfile.h>
#include <qregexp.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <libkcal/calendar.h>
#include <libkcal/event.h>
#include <libkcal/icalformat.h>

#include "komailclient.h"
#include "incidencechanger.h"

#include "mailscheduler.h"


using namespace KCal;

MailScheduler::MailScheduler( Calendar *calendar )
  : IMIPScheduler( calendar )
{
}

MailScheduler::~MailScheduler()
{
}

bool MailScheduler::publish( IncidenceBase *incidence,
                             const QString &recipients )
{
  QString messageText = mFormat->createScheduleMessage( incidence,
                                                        Scheduler::Publish );
  KOMailClient mailer;
  return mailer.mailTo( incidence, recipients, messageText );
}

bool MailScheduler::performTransaction( IncidenceBase *incidence,
                                        Method method,
                                        const QString &recipients )
{
  QString messageText = mFormat->createScheduleMessage( incidence, method );

  KOMailClient mailer;
  return mailer.mailTo( incidence, recipients, messageText );
}

bool MailScheduler::performTransaction( IncidenceBase *incidence,
                                        Method method )
{
  QString messageText = mFormat->createScheduleMessage( incidence, method );

  KOMailClient mailer;
  bool status;
  if ( method == Request ||
       method == Cancel ||
       method == Add ||
       method == Declinecounter ) {
    status = mailer.mailAttendees( incidence, messageText );
  } else {
    QString subject;
    Incidence *inc = dynamic_cast<Incidence*>( incidence );
    if ( inc && method == Counter )
      subject = i18n( "Counter proposal: %1" ).arg( inc->summary() );
    status = mailer.mailOrganizer( incidence, messageText, subject );
  }
  return status;
}

QPtrList<ScheduleMessage> MailScheduler::retrieveTransactions()
{
  QString incomingDirName = locateLocal( "data", "korganizer/income" );
  kdDebug(5850) << "MailScheduler::retrieveTransactions: dir: "
                << incomingDirName << endl;

  QPtrList<ScheduleMessage> messageList;

  QDir incomingDir( incomingDirName );
  QStringList incoming = incomingDir.entryList( QDir::Files );
  QStringList::ConstIterator it;
  for( it = incoming.begin(); it != incoming.end(); ++it ) {
    kdDebug(5850) << "-- File: " << (*it) << endl;

    QFile f( incomingDirName + "/" + (*it) );
    bool inserted = false;
    QMap<IncidenceBase*, QString>::Iterator iter;
    for ( iter = mEventMap.begin(); iter != mEventMap.end(); ++iter ) {
      if ( iter.data() == incomingDirName + "/" + (*it) )
        inserted = true;
    }
    if ( !inserted ) {
      if ( !f.open( IO_ReadOnly ) ) {
        kdDebug(5850)
          << "MailScheduler::retrieveTransactions(): Can't open file'"
          << (*it) << "'" << endl;
      } else {
        QTextStream t( &f );
        t.setEncoding( QTextStream::Latin1 );
        QString messageString = t.read();
        messageString.replace( QRegExp( "\n[ \t]"), "" );
        messageString = QString::fromUtf8( messageString.latin1() );
        ScheduleMessage *mess = mFormat->parseScheduleMessage( mCalendar,
                                                               messageString );
        if ( mess) {
          kdDebug(5850)
            << "MailScheduler::retrieveTransactions: got message '"
            << (*it) << "'" << endl;
          messageList.append( mess );
          mEventMap[ mess->event() ] = incomingDirName + "/" + (*it);
        } else {
          QString errorMessage;
          if ( mFormat->exception() ) {
            errorMessage = mFormat->exception()->message();
          }
          kdDebug(5850)
            << "MailScheduler::retrieveTransactions() Error parsing message: "
            << errorMessage << endl;
        }
        f.close();
      }
    }
  }
  return messageList;
}

bool MailScheduler::deleteTransaction( IncidenceBase *incidence )
{
  bool status;
  QFile f( mEventMap[incidence] );
  mEventMap.remove( incidence );
  if ( !f.exists() ) {
    status = false;
  } else {
    status = f.remove();
  }
  return status;
}

QString MailScheduler::freeBusyDir()
{
  return locateLocal( "data", "korganizer/freebusy" );
}

bool MailScheduler::acceptCounterProposal( Incidence *incidence )
{
  if ( !incidence )
    return false;

  Incidence *exInc = mCalendar->incidence( incidence->uid() );
  if ( !exInc )
    exInc = mCalendar->incidenceFromSchedulingID( incidence->uid() );
  incidence->setRevision( incidence->revision() + 1 );
  if ( exInc ) {
    incidence->setRevision( QMAX( incidence->revision(), exInc->revision() + 1 ) );
    // some stuff we don't want to change, just to be safe
    incidence->setSchedulingID( exInc->schedulingID() );
    incidence->setUid( exInc->uid() );

    mCalendar->beginChange( exInc );
    IncidenceChanger::assignIncidence( exInc, incidence );
    exInc->updated();
    mCalendar->endChange( exInc );
  } else {
    mCalendar->addIncidence( incidence );
  }
  return true;
}
