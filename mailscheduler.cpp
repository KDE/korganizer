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

//  KOMailClient mailer;

  return false;  
}

bool MailScheduler::performTransaction(Event *incidence,Method method)
{
  QString messageText = mFormat->createScheduleMessage(incidence,method);

  KOMailClient mailer;
  return mailer.mailAttendees(incidence,messageText);
}

QPtrList<ScheduleMessage> MailScheduler::retrieveTransactions()
{
  QString incomingDirName = locateLocal("appdata","income");
  kdDebug() << "MailScheduler::retrievTransactions: dir: " << incomingDirName
            << endl;

  QPtrList<ScheduleMessage> messageList;

  QDir incomingDir(incomingDirName);
  QStringList incoming = incomingDir.entryList(QDir::Files);
  QStringList::ConstIterator it;
  for(it = incoming.begin(); it != incoming.end(); ++it) {
    kdDebug() << "-- File: " << (*it) << endl;
    QFile f(incomingDirName + "/" + (*it));
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

  return messageList;
}
