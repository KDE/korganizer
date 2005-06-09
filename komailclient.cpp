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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <unistd.h>
#include <stdio.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <kprocess.h>

#include <libkcal/event.h>
#include <libkcal/todo.h>

#include <libkdepim/email.h>

#include "version.h"
#include "koprefs.h"

#include "komailclient.h"

KOMailClient::KOMailClient()
{
}

KOMailClient::~KOMailClient()
{
}

bool KOMailClient::mailAttendees(IncidenceBase *incidence,const QString &attachment)
{
  Attendee::List attendees = incidence->attendees();
  if (attendees.count() == 0) return false;

  const QString from = incidence->organizer().fullName();
  const QString organizerEmail = incidence->organizer().email();
  QStringList toList;
  for(uint i=0; i<attendees.count();++i) {
    const QString email = (*attendees.at(i))->email();
    // In case we (as one of our identities) are the organizer we are sending this
    // mail. We could also have added ourselves as an attendee, in which case we 
    // don't want to send ourselves a notification mail.
    if( organizerEmail !=  email )
      toList << email;
  }
  if( toList.count() == 0 )
    // Not really to be called a groupware meeting, eh
    return false;
  QString to = toList.join( ", " );

  QString subject;
  if(incidence->type()!="FreeBusy") {
    Incidence *inc = static_cast<Incidence *>(incidence);
    subject = inc->summary();
  } else {
    subject = "Free Busy Object";
  }

  QString body = createBody(incidence);

  bool bcc = KOPrefs::instance()->mBcc;

  return send(from,to,subject,body,bcc,attachment);
}

bool KOMailClient::mailOrganizer(IncidenceBase *incidence,const QString &attachment)
{
  QString to = incidence->organizer().fullName();

  QString from = KOPrefs::instance()->email();

  QString subject;
  if(incidence->type()!="FreeBusy") {
    Incidence *inc = static_cast<Incidence *>(incidence);
    subject = inc->summary();
  } else {
    subject = "Free Busy Message";
  }

  QString body = createBody(incidence);

  bool bcc = KOPrefs::instance()->mBcc;

  return send(from,to,subject,body,bcc,attachment);
}

bool KOMailClient::mailTo(IncidenceBase *incidence,const QString &recipients,
                          const QString &attachment)
{
  QString from = KOPrefs::instance()->email();
  QString subject;
  if(incidence->type()!="FreeBusy") {
    Incidence *inc = static_cast<Incidence *>(incidence);
    subject = inc->summary();
  } else {
    subject = "Free Busy Message";
  }
  QString body = createBody(incidence);
  bool bcc = KOPrefs::instance()->mBcc;
  kdDebug () << "KOMailClient::mailTo " << recipients << endl;
  return send(from,recipients,subject,body,bcc,attachment);
}

bool KOMailClient::send(const QString &from,const QString &to,
                        const QString &subject,const QString &body,bool bcc,
                        const QString &attachment)
{
  kdDebug(5850) << "KOMailClient::sendMail():\nFrom: " << from << "\nTo: " << to
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

      command.append(QString::fromLatin1(" -s "));
      command.append(KProcess::quote(subject));

      if (bcc) {
        command.append(QString::fromLatin1(" -b "));
        command.append(KProcess::quote(from));
      }

      command.append(" ");
      command.append(KProcess::quote(to));

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
      textComplete += QString::fromLatin1("X-Mailer: KOrganizer") + korgVersion + '\n';
    }
    textComplete += '\n'; // end of headers
    textComplete += body;
    textComplete += '\n';
    textComplete += attachment;

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
      if (!kMailOpenComposer(to,"",bcc ? from : "",subject,body,0,KURL())) return false;
    } else {
      QString meth;
      int idx = attachment.find("METHOD");
      if (idx>=0) {
        idx = attachment.find(':',idx)+1;
        meth = attachment.mid(idx,attachment.find('\n',idx)-idx);
        meth = meth.lower();
      } else {
        meth = "publish";
      }
      if (!kMailOpenComposer(to,"",bcc ? from : "",subject,body,0,"cal.ics","7bit",
                             attachment.utf8(),"text","calendar","method",meth,
                             "attachment","utf-8")) return false;
    }
  }
  return true;
}

int KOMailClient::kMailOpenComposer(const QString& arg0,const QString& arg1,
  const QString& arg2,const QString& arg3,const QString& arg4,int arg5,
  const KURL& arg6)
{
  //kdDebug(5850) << "KOMailClient::kMailOpenComposer( "
  //  << arg0 << " , " << arg1 << arg2 << " , " << arg3
  //  << arg4 << " , " << arg5 << " , " << arg6 << " )" << endl;
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
#if KDE_IS_VERSION( 3, 2, 90 )
  kapp->updateRemoteUserTimestamp( "kmail" );
#endif
  if (kapp->dcopClient()->call("kmail","KMailIface","openComposer(QString,QString,QString,QString,QString,int,KURL)", data, replyType, replyData ) ) {
    if ( replyType == "int" ) {
      QDataStream _reply_stream( replyData, IO_ReadOnly );
      _reply_stream >> result;
    } else {
      kdDebug(5850) << "kMailOpenComposer() call failed." << endl;
    }
  } else {
    kdDebug(5850) << "kMailOpenComposer() call failed." << endl;
  }
  return result;
}

int KOMailClient::kMailOpenComposer( const QString& arg0, const QString& arg1,
                                     const QString& arg2, const QString& arg3,
                                     const QString& arg4, int arg5, const QString& arg6,
                                     const QCString& arg7, const QCString& arg8,
                                     const QCString& arg9, const QCString& arg10,
                                     const QCString& arg11, const QString& arg12,
                                     const QCString& arg13, const QCString& arg14 )
{
    //kdDebug(5850) << "KOMailClient::kMailOpenComposer( "
    //    << arg0 << " , " << arg1 << arg2 << " , " << arg3
    //   << arg4 << " , " << arg5 << " , " << arg6
    //    << arg7 << " , " << arg8 << " , " << arg9
    //    << arg10<< " , " << arg11<< " , " << arg12
    //    << arg13<< " , " << arg14<< " )" << endl;

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
    arg << arg14;
#if KDE_IS_VERSION( 3, 2, 90 )
    kapp->updateRemoteUserTimestamp("kmail");
#endif
    if ( kapp->dcopClient()->call("kmail","KMailIface",
          "openComposer(QString,QString,QString,QString,QString,int,QString,QCString,QCString,QCString,QCString,QCString,QString,QCString,QCString)", data, replyType, replyData ) ) {
        if ( replyType == "int" ) {
            QDataStream _reply_stream( replyData, IO_ReadOnly );
            _reply_stream >> result;
        } else {
            kdDebug(5850) << "kMailOpenComposer() call failed." << endl;
        }
    } else {
        kdDebug(5850) << "kMailOpenComposer() call failed." << endl;
    }
    return result;
}


QString KOMailClient::createBody(IncidenceBase *incidence)
{
  QString CR = ("\n");

  QString body;

  // mailbody for Event
  if (incidence->type()=="Event") {
    Event *selectedEvent = static_cast<Event *>(incidence);
    QString recurrence[]= {i18n("no recurrence", "None"),
      i18n("Minutely"), i18n("Hourly"), i18n("Daily"),
      i18n("Weekly"), i18n("Monthly Same Day"), i18n("Monthly Same Position"),
      i18n("Yearly"), i18n("Yearly"), i18n("Yearly")};

    if (!selectedEvent->organizer().isEmpty()) {
      body += i18n("Organizer: %1").arg(selectedEvent->organizer().fullName());
      body += CR;
    }
    body += i18n("Summary: %1").arg(selectedEvent->summary());
    body += CR;
    if (!selectedEvent->location().isEmpty()) {
      body += i18n("Location: %1").arg(selectedEvent->location());
      body += CR;
    }
    body += i18n("Start Date: %1").arg(selectedEvent->dtStartDateStr());
    body += CR;
    if (!selectedEvent->doesFloat()) {
      body += i18n("Start Time: %1").arg(selectedEvent->dtStartTimeStr());
      body += CR;
    }
    if ( selectedEvent->dtStart()!=selectedEvent->dtEnd() ) {
      body += i18n("End Date: %1").arg(selectedEvent->dtEndDateStr());
      body += CR;
    }
    if (!selectedEvent->doesFloat()) {
      body += i18n("End Time: %1").arg(selectedEvent->dtEndTimeStr());
      body += CR;
    }
    if (selectedEvent->doesRecur()) {
      body += i18n("Recurs: %1")
               .arg(recurrence[selectedEvent->recurrence()->doesRecur()]);
      body += CR;
/* TODO: frequency
      body += i18n("Frequency: %1")
               .arg(recurrence[selectedEvent->recurrence()->frequency()]);
      body += CR;
*/
      if (selectedEvent->recurrence()->duration() > 0 ) {
        body += i18n ("Repeats %1 times")
                 .arg(QString::number(selectedEvent->recurrence()->duration()));
        body += CR;
      } else {
        if (selectedEvent->recurrence()->duration() != -1) {
//          body += i18n("Repeat until: %1")
          body += i18n("End Date: %1")
                   .arg(selectedEvent->recurrence()->endDateStr());
          body += CR;
        } else {
          body += i18n("Repeats forever");
          body += CR;
        }
      }
    }
    QString details = selectedEvent->description();
    if (!details.isEmpty()) {
      body += i18n("Details:");
      body += CR;
      body += details;
      body += CR;
    }
  }

  // mailbody for Todo
  if (incidence->type()=="Todo") {
    Todo *selectedEvent = static_cast<Todo *>(incidence);
    if (!selectedEvent->organizer().isEmpty()) {
      body += i18n("Organizer: %1").arg(selectedEvent->organizer().fullName());
      body += CR;
    }
    body += i18n("Summary: %1").arg(selectedEvent->summary());
    body += CR;
    if (!selectedEvent->location().isEmpty()) {
      body += i18n("Location: %1").arg(selectedEvent->location());
      body += CR;
    }
    if (selectedEvent->hasStartDate()) {
      body += i18n("Start Date: %1").arg(selectedEvent->dtStartDateStr());
      body += CR;
      if (!selectedEvent->doesFloat()) {
        body += i18n("Start Time: %1").arg(selectedEvent->dtStartTimeStr());
        body += CR;
      }
    }
    if (selectedEvent->hasDueDate()) {
      body += i18n("Due Date: %1").arg(selectedEvent->dtDueDateStr());
      body += CR;
      if (!selectedEvent->doesFloat()) {
        body += i18n("Due Time: %1").arg(selectedEvent->dtDueTimeStr());
        body += CR;
      }
    }
    QString details = selectedEvent->description();
    if (!details.isEmpty()) {
      body += i18n("Details:");
      body += CR;
      body += details;
      body += CR;
    }
  }

  // mailbody for FreeBusy
  if(incidence->type()=="FreeBusy") {
    body = i18n("This is a Free Busy Object");
  }

  // mailbody for Journal
  if(incidence->type()=="Journal") {
    Incidence *inc = static_cast<Incidence *>(incidence);
    body = inc->summary();
    body += CR;
    body += inc->description();
    body += CR;
  }

  return body;
}
