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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$
//
// MailScheduler - Mail implementation of iTIP methods
//

#include <qdir.h>
#include <qfile.h>

#include <kstddirs.h>
#include <kdebug.h>

#include <libkcal/event.h>
#include <libkcal/icalformat.h>

#include "komailclient.h"

#include "mailscheduler.h"

using namespace KCal;

MailScheduler::MailScheduler(Calendar *calendar)
  : IMIPScheduler(calendar)
{
}

MailScheduler::~MailScheduler()
{
}

bool MailScheduler::publish (Event *incidence,const QString &recipients)
{
  QString messageText = mFormat->createScheduleMessage(incidence,
                                                       Scheduler::Publish);
  KOMailClient mailer;
//  kdDebug () << "MailScheduler::publish to " << recipients << endl;
  return mailer.mailTo(incidence,recipients,messageText);
}

bool MailScheduler::performTransaction(Event *incidence,Method method)
{
  QString messageText = mFormat->createScheduleMessage(incidence,method);

  KOMailClient mailer;
//  kdDebug () << "MailScheduler::performTransaction"  << endl;
  return mailer.mailAttendees(incidence,messageText);
}

QPtrList<ScheduleMessage> MailScheduler::retrieveTransactions()
{
  QString incomingDirName = locateLocal("appdata","income");
  kdDebug() << "MailScheduler::retrieveTransactions: dir: " << incomingDirName
            << endl;

  QPtrList<ScheduleMessage> messageList;

  QDir incomingDir(incomingDirName);
  QStringList incoming = incomingDir.entryList(QDir::Files);
  QStringList::ConstIterator it;
  for(it = incoming.begin(); it != incoming.end(); ++it) {
    kdDebug() << "-- File: " << (*it) << endl;

    QFile f(incomingDirName + "/" + (*it));
    bool inserted = false;
    QMap<Incidence*, QString>::Iterator iter;
    for ( iter = mEventMap.begin(); iter != mEventMap.end(); ++iter ) {
      if (iter.data() == incomingDirName + "/" + (*it)) inserted = true;
    }
    if (!inserted) {
    if (!f.open(IO_ReadOnly)) {
      kdDebug() << "MailScheduler::retrieveTransactions(): Can't open file'"
                << (*it) << "'" << endl;
    } else {
      QTextStream t(&f);
      QString messageString = t.read();
      ScheduleMessage *message = mFormat->parseScheduleMessage(messageString);
      if (message) {
        kdDebug() << "MailScheduler::retrieveTransactions: got message '"
                  << (*it) << "'" << endl;
        messageList.append(message);
        mEventMap[message->event()]=incomingDirName + "/" + (*it);
      } else {
        QString errorMessage;
        if (mFormat->exception()) {
          errorMessage = mFormat->exception()->message();
        }
        kdDebug() << "MailScheduler::retrieveTransactions() Error parsing "
                     "message: " << errorMessage << endl;
      }
      f.close();
    }
    }
  }
  return messageList;
}

bool MailScheduler::deleteTransaction(Incidence *incidence)
{
  QFile f( mEventMap[incidence] );
  mEventMap.remove(incidence);
  if ( !f.exists() ) return false;
  else
    return f.remove();
}
