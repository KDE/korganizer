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
#include <libkcal/incidenceformatter.h>

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
  const QString organizerEmail= incidence->organizer()->email();
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

  QString body = IncidenceFormatter::mailBodyString(incidence);

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

  QString body = IncidenceFormatter::mailBodyString(incidence);

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
  QString body = IncidenceFormatter::mailBodyString(incidence);
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


