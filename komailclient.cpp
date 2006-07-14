/*
    This file is part of KOrganizer.
    Copyright (c) 1998 Barry D Benowitz <b.benowitz@telesciences.com>
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

#include <unistd.h>
#include <stdio.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kapplication.h>
#include <kprocess.h>

#include <kcal/event.h>
#include <kcal/todo.h>
#include <kcal/incidenceformatter.h>

#include "version.h"
#include "koprefs.h"

#include "komailclient.h"
//Added by qt3to4:
#include <QByteArray>
#include <ktoolinvocation.h>
#include <QtDBus>

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
  for(int i=0; i<attendees.count();++i) {
    const QString email = attendees.at(i)->email();
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
  kDebug () << "KOMailClient::mailTo " << recipients << endl;
  return send(from,recipients,subject,body,bcc,attachment);
}

bool KOMailClient::send(const QString &from,const QString &to,
                        const QString &subject,const QString &body,bool bcc,
                        const QString &attachment)
{
  kDebug(5850) << "KOMailClient::sendMail():\nFrom: " << from << "\nTo: " << to
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

    FILE * fd = popen(command.toLocal8Bit(),"w");
    if (!fd)
    {
      kError() << "Unable to open a pipe to " << command << endl;
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

    fwrite(textComplete.toLocal8Bit(),textComplete.length(),1,fd);

    pclose(fd);
  } else {
    if (!QDBus::sessionBus().interface()->isServiceRegistered("kmail")) {
      if (KToolInvocation::startServiceByDesktopName("kmail")) {
        KMessageBox::error(0,i18n("No running instance of KMail found."));
        return false;
      }
    }

    if (attachment.isEmpty()) {
      if (!kMailOpenComposer(to,"",bcc ? from : "",subject,body,0,KUrl())) return false;
    } else {
      QString meth;
      int idx = attachment.indexOf( "METHOD" );
      if (idx>=0) {
        idx = attachment.indexOf( ':', idx )+1;
        meth = attachment.mid( idx, attachment.indexOf( '\n', idx ) - idx );
        meth = meth.toLower();
      } else {
        meth = "publish";
      }
      if (!kMailOpenComposer(to,"",bcc ? from : "",subject,body,0,"cal.ics","7bit",
                             attachment.toUtf8(),"text","calendar","method",meth,
                             "attachment","utf-8")) return false;
    }
  }
  return true;
}

int KOMailClient::kMailOpenComposer(const QString& arg0,const QString& arg1,
  const QString& arg2,const QString& arg3,const QString& arg4,int arg5,
  const KUrl& arg6)
{
  //kDebug(5850) << "KOMailClient::kMailOpenComposer( "
  //  << arg0 << " , " << arg1 << arg2 << " , " << arg3
  //  << arg4 << " , " << arg5 << " , " << arg6 << " )" << endl;
  int result = 0;
  kapp->updateRemoteUserTimestamp( "kmail" );

  QDBusInterface kmail("org.kde.kmail", "/KMail", "org.kde.kmail.KMail");
  QDBusReply<int> reply = kmail.call("openComposer", arg0, arg1, arg2, arg3, arg4, arg5, arg6.url());
  if (reply.isValid() ) {
      result=reply;
  }
  else
  {
    kDebug(5850) << "kMailOpenComposer() call failed." << endl;
  }
  return result;
}

int KOMailClient::kMailOpenComposer( const QString& arg0, const QString& arg1,
                                     const QString& arg2, const QString& arg3,
                                     const QString& arg4, int arg5, const QString& arg6,
                                     const QByteArray& arg7, const QByteArray& arg8,
                                     const QByteArray& arg9, const QByteArray& arg10,
                                     const QByteArray& arg11, const QString& arg12,
                                     const QByteArray& arg13, const QByteArray& arg14 )
{
    //kDebug(5850) << "KOMailClient::kMailOpenComposer( "
    //    << arg0 << " , " << arg1 << arg2 << " , " << arg3
    //   << arg4 << " , " << arg5 << " , " << arg6
    //    << arg7 << " , " << arg8 << " , " << arg9
    //    << arg10<< " , " << arg11<< " , " << arg12
    //    << arg13<< " , " << arg14<< " )" << endl;

    int result = 0;

    kapp->updateRemoteUserTimestamp("kmail");
    QDBusInterface kmail("org.kde.kmail", "/KMail", "org.kde.kmail.KMail");
    QList<QVariant> argList;
    argList << arg0;
    argList << arg1;
    argList << arg2;
    argList << arg3;
    argList << arg4;
    argList << arg5;
    argList << arg6;
    argList << arg7;
    argList << arg8;
    argList << arg9;
    argList << arg10;
    argList << arg11;
    argList << arg12;
    argList << arg13;
    argList << arg14;

    QDBusReply<int> reply = kmail.callWithArgumentList(QDBus::Block,"openComposer",argList);

    if (reply.isValid()) {
            result=reply;
    } else {
      kDebug(5850) << "kMailOpenComposer() call failed." << endl;
    }
    return result;
}


