// $Id$
//
// DummyScheduler - iMIP implementation of iTIP methods
//

#include <qfile.h>
#include <qtextstream.h>

#include <kdebug.h>

#include "koevent.h"
#include "icalformat.h"

#include "dummyscheduler.h"

DummyScheduler::DummyScheduler(CalObject *calendar)
  : Scheduler(calendar)
{
}

DummyScheduler::~DummyScheduler()
{
}

bool DummyScheduler::publish (KOEvent *incidence,const QString &recipients)
{
  QString messageText = mFormat->createScheduleMessage(incidence,
                                                       Scheduler::Publish);

  return saveMessage(messageText);
}

bool DummyScheduler::performTransaction(KOEvent *incidence,Method method)
{
  QString messageText = mFormat->createScheduleMessage(incidence,method);

  return saveMessage(messageText);
}

bool DummyScheduler::saveMessage(const QString &message)
{
  QFile f("dummyscheduler.store");
  if (f.open(IO_WriteOnly | IO_Append)) {
    QTextStream t(&f);
    t << message << endl;
    f.close();
    return true;
  } else {
    return false;
  }
}

QList<ScheduleMessage> DummyScheduler::retrieveTransactions()
{
  QList<ScheduleMessage> messageList;

  QFile f("dummyscheduler.store");
  if (!f.open(IO_ReadOnly)) {
    kdDebug() << "DummyScheduler::retrieveTransactions(): Can't open file"
              << endl;
  } else {
    QTextStream t(&f);
    QString messageString;
    QString messageLine = t.readLine();
    while (!messageLine.isNull()) {
//      kdDebug() << "++++++++" << messageLine << endl;
      messageString += messageLine + "\n";
      if (messageLine.find("END:VCALENDAR") >= 0) {
        kdDebug() << "---------------" << messageString << endl;
        ScheduleMessage *message = mFormat->parseScheduleMessage(messageString);
        kdDebug() << "--Parsed" << endl;
        if (message) {
          messageList.append(message);
        } else {
          QString errorMessage;
          if (mFormat->exception()) {
            errorMessage = mFormat->exception()->message();
          }
          kdDebug() << "DummyScheduler::retrieveTransactions() Error parsing "
                       "message: " << errorMessage << endl;
        }
        messageString="";
      }
      messageLine = t.readLine();
    }
    f.close();
  }

  return messageList;
}
