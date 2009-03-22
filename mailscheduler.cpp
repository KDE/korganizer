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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "mailscheduler.h"
#include "komailclient.h"
#include "incidencechanger.h"

#include <kcal/calendar.h>
#include <kcal/event.h>
#include <kcal/icalformat.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <QDir>
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QList>

MailScheduler::MailScheduler( Calendar *calendar )
  : Scheduler( calendar )
{
}

MailScheduler::~MailScheduler()
{
}

bool MailScheduler::publish( IncidenceBase *incidence,
                             const QString &recipients )
{
  QString messageText = mFormat->createScheduleMessage( incidence,
                                                        iTIPPublish );
  KOMailClient mailer;
  return mailer.mailTo( incidence, recipients, messageText );
}

bool MailScheduler::performTransaction( IncidenceBase *incidence,
                                        iTIPMethod method,
                                        const QString &recipients )
{
  QString messageText = mFormat->createScheduleMessage( incidence, method );

  KOMailClient mailer;
  return mailer.mailTo( incidence, recipients, messageText );
}

bool MailScheduler::performTransaction( IncidenceBase *incidence,
                                        iTIPMethod method )
{
  QString messageText = mFormat->createScheduleMessage( incidence, method );

  KOMailClient mailer;
  bool status;
  if ( method == iTIPRequest ||
       method == iTIPCancel ||
       method == iTIPAdd ||
       method == iTIPDeclineCounter ) {
    status = mailer.mailAttendees( incidence, messageText );
  } else {
    QString subject;
    Incidence *inc = dynamic_cast<Incidence*>( incidence );
    if ( inc && method == iTIPCounter ) {
      subject = i18n( "Counter proposal: %1", inc->summary() );
    }
    status = mailer.mailOrganizer( incidence, messageText, subject );
  }
  return status;
}

QList<ScheduleMessage*> MailScheduler::retrieveTransactions()
{
  QString incomingDirName = KStandardDirs::locateLocal( "data", "korganizer/income" );
  kDebug() << "dir:" << incomingDirName;

  QList<ScheduleMessage*> messageList;

  QDir incomingDir( incomingDirName );
  QStringList incoming = incomingDir.entryList( QDir::Files );
  QStringList::ConstIterator it;
  for ( it = incoming.constBegin(); it != incoming.constEnd(); ++it ) {
    kDebug() << "-- File:" << (*it);

    QFile f( incomingDirName + '/' + (*it) );
    bool inserted = false;
    QMap<IncidenceBase *, QString>::Iterator iter;
    for ( iter = mEventMap.begin(); iter != mEventMap.end(); ++iter ) {
      if ( iter.value() == incomingDirName + '/' + (*it) ) {
        inserted = true;
      }
    }
    if ( !inserted ) {
      if ( !f.open( QIODevice::ReadOnly ) ) {
        kDebug() << "Can't open file'" << (*it) << "'";
      } else {
        QTextStream t( &f );
        t.setCodec( "ISO 8859-1" );
        QString messageString = t.readAll();
        messageString.remove( QRegExp( "\n[ \t]" ) );
        messageString = QString::fromUtf8( messageString.toLatin1() );
        ScheduleMessage *mess = mFormat->parseScheduleMessage( mCalendar,
                                                               messageString );
        if ( mess ) {
          kDebug() << "got message '" << (*it) << "'";
          messageList.append( mess );
          mEventMap[ mess->event() ] = incomingDirName + '/' + (*it);
        } else {
          QString errorMessage;
          if ( mFormat->exception() ) {
            errorMessage = mFormat->exception()->message();
          }
          kDebug() << "Error parsing message:" << errorMessage;
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
  return KStandardDirs::locateLocal( "data", "korganizer/freebusy" );
}

bool MailScheduler::acceptCounterProposal( Incidence *incidence )
{
  if ( !incidence ) {
    return false;
  }

  Incidence *exInc = mCalendar->incidence( incidence->uid() );
  if ( !exInc ) {
    exInc = mCalendar->incidenceFromSchedulingID( incidence->uid() );
  }

  incidence->setRevision( incidence->revision() + 1 );
  if ( exInc ) {
    incidence->setRevision( qMax( incidence->revision(), exInc->revision() + 1 ) );
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
