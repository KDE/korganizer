// A barebones mail client designed to grow as we need it
// Copyright (c) 1998 Barry D Benowitz
// $Id$

#include <unistd.h>
#include <stdio.h>

#include <klocale.h>
#include <kstddirs.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include "version.h"
#include "event.h"
#include "koprefs.h"

#include "komailclient.h"
#include "komailclient.moc"


MailMsgString::MailMsgString()
{
}

MailMsgString::~MailMsgString()
{
}

void MailMsgString::setAddressee(Attendee *newAddressee)
{
  mAddressee = newAddressee->getEmail();
  if (!newAddressee->getName().isEmpty()) {
    mAddressee.prepend(" <");
    mAddressee.append(">");
    mAddressee.prepend(newAddressee->getName());
  }
}

void MailMsgString::buildTextMsg(Event * selectedEvent)
{
  QString CR = ("\n");
  QString recurrence[]= {"None","Daily","Weekly","Monthly Same Day",
                         "Monthly Same Position","Yearly","Yearly"};
  
  if (selectedEvent->getOrganizer() != "") {
    mBody += i18n("Organizer: %1").arg(selectedEvent->getOrganizer());
    mBody += CR;
  }

  mBody += i18n("Summary: %1").arg(selectedEvent->getSummary());
  if (!selectedEvent->doesFloat()) {
    mBody += CR;
    mBody += i18n("Start Date: %1").arg(selectedEvent->getDtStartDateStr());
    mBody += CR;
    mBody += i18n("Start Time: %1").arg(selectedEvent->getDtStartTimeStr());
    mBody += CR;
    if (selectedEvent->doesRecur()) {
      mBody += i18n("Recurs: %1")
               .arg(recurrence[selectedEvent->getRecursFrequency()]);
      mBody += CR;
      if (selectedEvent->getRecursDuration() > 0 ) {
        mBody += i18n ("Repeats %1 times")
                 .arg(QString::number(selectedEvent->getRecursDuration()));
        mBody += CR;
      } else {
        if (selectedEvent->getRecursDuration() != -1) {
          mBody += i18n("End Date : %1")
                   .arg(selectedEvent->getRecursEndDateStr());
          mBody += CR;
        } else {
          mBody += i18n("Repeats forever");
          mBody += CR;
        }
      }
    }
    mBody += i18n("End Time : %1").arg(selectedEvent->getDtEndTimeStr());
    mBody += CR;
  } 
}


KOMailClient::KOMailClient()
{   
}

KOMailClient::~KOMailClient()
{
}

void KOMailClient::emailEvent(Event *selectedEvent)
{
  MailMsgString msg;
  msg.buildTextMsg(selectedEvent);

  bool sent = false;

  // Generate List of Addressees
  QList<Attendee> participants = selectedEvent->getAttendeeList();
  Attendee *a;
  for (a = participants.first();a;a=participants.next()) {
    if (a->getStatus() == Attendee::NEEDS_ACTION) {
      msg.setAddressee(a);
      if (!sendMail(KOPrefs::instance()->mEmail,msg.addressee(),
                    selectedEvent->getSummary(),msg.body(),
                    KOPrefs::instance()->mBcc)) {
        KMessageBox::error(0,i18n("Mail delivery to %1 failed.")
                             .arg(msg.addressee()));
      } else {
        sent = true;
        a->setStatus(Attendee::SENT);
      }
    }
  }

  if (!sent) return;  // no recips were in NEEDS_ACTION status - bail out

#if 0
  // update the status on the event object
  if (selectedEvent->getStatus() == selectedEvent->NEEDS_ACTION)
    selectedEvent->setStatus(selectedEvent->SENT);
#endif
  
  return;
}


bool KOMailClient::sendMail(const QString &from,const QString &to,
                            const QString &subject,const QString &body,
                            bool bcc)
{
  kdDebug() << "KOMailClient::sendMail():\nFrom: " << from << "\nTo: " << to << "\nSubject: " << subject << "\nBody: \n" << body << endl;

  bool needHeaders = true;

  QString command = KStandardDirs::findExe(QString::fromLatin1("sendmail"),
      QString::fromLatin1("/sbin:/usr/sbin:/usr/lib"));
  if (!command.isNull()) command += QString::fromLatin1(" -oi -t");
  else {
    command = KStandardDirs::findExe(QString::fromLatin1("mail"));
    if (command.isNull()) return false; // give up
    
    command.append(QString::fromLatin1(" -s \x22"));
    command.append(subject);
    command.append(QString::fromLatin1("\x22"));

    if (bcc) {
      command.append(QString::fromLatin1(" -b "));
      command.append(from);
    }

    command.append(" ");
    command.append(to);

    needHeaders = false;
  }

  FILE * fd = popen(command.local8Bit(),"w");
  if (!fd)
  {
    kdError() << "Unable to open a pipe to " << command << endl;
    return false;
  }

  QString textComplete;
  if (needHeaders)
  {
    textComplete += QString::fromLatin1("From: ") + from + '\n';
    textComplete += QString::fromLatin1("To: ") + to + '\n';
    if (bcc) textComplete += QString::fromLatin1("Bcc: ") + from + '\n';
    textComplete += QString::fromLatin1("Subject: ") + subject + '\n';
    textComplete += QString::fromLatin1("X-Mailer: KOrganizer") + korgVersion +
                    '\n';
  }
  textComplete += '\n'; // end of headers
  textComplete += body;

  fwrite(textComplete.local8Bit(),textComplete.length(),1,fd);

  pclose(fd);

  return true;
}
