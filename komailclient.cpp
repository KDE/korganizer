/*
    This file is part of KOrganizer.
    Copyright (c) 1998 Barry D Benowitz
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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

// $Id$

#include <unistd.h>
#include <stdio.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kapplication.h>
#include <dcopclient.h>

#include <libkcal/event.h>

#include "version.h"
#include "koprefs.h"

#include "komailclient.h"

KOMailClient::KOMailClient()
{
}

KOMailClient::~KOMailClient()
{
}

bool KOMailClient::mailAttendees(Incidence *incidence,const QString &attachment)
{
  QPtrList<Attendee> attendees = incidence->attendees();
  if (attendees.count() == 0) return false;

  QString to;
  for(uint i=0; i<attendees.count();++i) {
    to += attendees.at(i)->email();
    if (i != attendees.count()-1) to += ", ";
  }

  QString from = KOPrefs::instance()->email();

  QString subject = incidence->summary();

  QString body = createBody(incidence);

  bool bcc = KOPrefs::instance()->mBcc;

  return send(from,to,subject,body,bcc,attachment);
}

bool KOMailClient::mailOrganizer(Incidence *incidence,const QString &attachment)
{
  QString to = incidence->organizer();

  QString from = KOPrefs::instance()->email();

  QString subject = incidence->summary();

  QString body = createBody(incidence);

  bool bcc = KOPrefs::instance()->mBcc;

  return send(from,to,subject,body,bcc,attachment);
}

bool KOMailClient::mailTo(Incidence *incidence,const QString &recipients,
                          const QString &attachment)
{
  QString from = KOPrefs::instance()->email();
  QString subject = incidence->summary();
  QString body = createBody(incidence);
  bool bcc = KOPrefs::instance()->mBcc;
  kdDebug () << "KOMailClient::mailTo " << recipients << endl;
  return send(from,recipients,subject,body,bcc,attachment);
}

bool KOMailClient::send(const QString &from,const QString &to,
                        const QString &subject,const QString &body,bool bcc,
                        const QString &attachment)
{
  kdDebug() << "KOMailClient::sendMail():\nFrom: " << from << "\nTo: " << to
            << "\nSubject: " << subject << "\nBody: \n" << body
            << "\nAttachment:\n" << attachment << endl;

  if (KOPrefs::instance()->mMailClient == KOPrefs::MailClientSendmail) {
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
  } else {
    if (!kapp->dcopClient()->isApplicationRegistered("kmail")) {
			if (KApplication::startServiceByDesktopName("kmail")) {
        KMessageBox::error(0,i18n("No running instance of KMail found."));
        return false;
			}
    }

    if (attachment.isEmpty()) {
      if (!kMailOpenComposer(to,"",from,subject,body,0,KURL())) return false;
    } else {
      if (!kMailOpenComposer(to,"",from,subject,body,0,"cal.ics","7bit",
                             attachment.utf8(),"text","calendar","method","publish",
                             "attachment")) return false;
    }
  }
  return true;
}

int KOMailClient::kMailOpenComposer(const QString& arg0,const QString& arg1,
  const QString& arg2,const QString& arg3,const QString& arg4,int arg5,
  const KURL& arg6)
{
  int result = 0;

  QByteArray data, replyData;
  QCString replyType;
  QDataStream arg( data, IO_WriteOnly );
  arg << arg0;
  arg << arg1;
  arg << arg2;
  arg << arg3;
  arg << arg4;
  arg << arg5;
  arg << arg6;
  if (kapp->dcopClient()->call("kmail","KMailIface","openComposer(QString,QString,QString,QString,QString,int,KURL)", data, replyType, replyData ) ) {
    if ( replyType == "int" ) {
      QDataStream _reply_stream( replyData, IO_ReadOnly );
      _reply_stream >> result;
    } else {
      kdDebug() << "kMailOpenComposer() call failed." << endl;
    }
  } else {
    kdDebug() << "kMailOpenComposer() call failed." << endl;
  }
  return result;
}

int KOMailClient::kMailOpenComposer( const QString& arg0, const QString& arg1,
                                     const QString& arg2, const QString& arg3,
                                     const QString& arg4, int arg5, const QString& arg6,
                                     const QCString& arg7, const QCString& arg8,
                                     const QCString& arg9, const QCString& arg10,
                                     const QCString& arg11, const QString& arg12,
                                     const QCString& arg13 )
{
    int result = 0;

    QByteArray data, replyData;
    QCString replyType;
    QDataStream arg( data, IO_WriteOnly );
    arg << arg0;
    arg << arg1;
    arg << arg2;
    arg << arg3;
    arg << arg4;
    arg << arg5;
    arg << arg6;
    arg << arg7;
    arg << arg8;
    arg << arg9;
    arg << arg10;
    arg << arg11;
    arg << arg12;
    arg << arg13;
    if ( kapp->dcopClient()->call("kmail","KMailIface","openComposer(QString,QString,QString,QString,QString,int,QString,QCString,QCString,QCString,QCString,QCString,QString,QCString)", data, replyType, replyData ) ) {
	if ( replyType == "int" ) {
	    QDataStream _reply_stream( replyData, IO_ReadOnly );
	    _reply_stream >> result;
	} else {
            kdDebug() << "kMailOpenComposer() call failed." << endl;
	}
    } else { 
        kdDebug() << "kMailOpenComposer() call failed." << endl;
    }
    return result;
}


QString KOMailClient::createBody(Incidence *incidence)
{
  QString CR = ("\n");

  QString body;

  Event *selectedEvent = dynamic_cast<Event *>(incidence);
  if (selectedEvent) {
    QString recurrence[]= {"None","Daily","Weekly","Monthly Same Day",
                           "Monthly Same Position","Yearly","Yearly"};
  
    if (selectedEvent->organizer() != "") {
      body += i18n("Organizer: %1").arg(selectedEvent->organizer());
      body += CR;
    }

    body += i18n("Summary: %1").arg(selectedEvent->summary());
    if (!selectedEvent->doesFloat()) {
      body += CR;
      body += i18n("Start Date: %1").arg(selectedEvent->dtStartDateStr());
      body += CR;
      body += i18n("Start Time: %1").arg(selectedEvent->dtStartTimeStr());
      body += CR;
      if (selectedEvent->recurrence()->doesRecur()) {
        body += i18n("Recurs: %1")
                 .arg(recurrence[selectedEvent->recurrence()->frequency()]);
        body += CR;
        if (selectedEvent->recurrence()->duration() > 0 ) {
          body += i18n ("Repeats %1 times")
                   .arg(QString::number(selectedEvent->recurrence()->duration()));
          body += CR;
        } else {
          if (selectedEvent->recurrence()->duration() != -1) {
            body += i18n("End Date : %1")
                     .arg(selectedEvent->recurrence()->endDateStr());
            body += CR;
          } else {
            body += i18n("Repeats forever");
            body += CR;
          }
        }
      }
      body += i18n("End Time : %1").arg(selectedEvent->dtEndTimeStr());
      body += CR;
      QString details = incidence->description();
      if (!details.isEmpty()) {
        body += i18n("Details:");
	body += CR;
	body += details;
      }
    }
  } else {
    body = incidence->summary();
    body += CR;
    body += incidence->description();
  }

  return body;
}
