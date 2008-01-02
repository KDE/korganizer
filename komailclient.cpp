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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "komailclient.h"
#include "koprefs.h"
#include "version.h"
#include <kmailinterface.h>

#include <kcal/event.h>
#include <kcal/todo.h>
#include <kcal/incidenceformatter.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kurl.h>
#include <kapplication.h>
#include <ktoolinvocation.h>
#include <kshell.h>

#include <QtDBus/QtDBus>
#include <QByteArray>

#include <unistd.h>
#include <stdio.h>

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
  kDebug (5850) <<"KOMailClient::mailTo" << recipients;
  return send(from,recipients,subject,body,bcc,attachment);
}

bool KOMailClient::send(const QString &from,const QString &to,
                        const QString &subject,const QString &body,bool bcc,
                        const QString &attachment)
{
  kDebug(5850) <<"KOMailClient::sendMail():\nFrom:" << from <<"\nTo:" << to
               << "\nSubject:" << subject << "\nBody: \n" << body
               << "\nAttachment:\n" << attachment;

  if (KOPrefs::instance()->mMailClient == KOPrefs::MailClientSendmail) {
    bool needHeaders = true;

    QString command = KStandardDirs::findExe(QString::fromLatin1("sendmail"),
        QString::fromLatin1("/sbin:/usr/sbin:/usr/lib"));
    if (!command.isNull()) command += QString::fromLatin1(" -oi -t");
    else {
      command = KStandardDirs::findExe(QString::fromLatin1("mail"));
      if (command.isNull()) return false; // give up

      command.append(QString::fromLatin1(" -s "));
      command.append(KShell::quoteArg(subject));

      if (bcc) {
        command.append(QString::fromLatin1(" -b "));
        command.append(KShell::quoteArg(from));
      }

      command.append(" ");
      command.append(KShell::quoteArg(to));

      needHeaders = false;
    }

    FILE * fd = popen(command.toLocal8Bit(),"w");
    if (!fd)
    {
      kError() <<"Unable to open a pipe to" << command;
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
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kmail")) {
      if (KToolInvocation::startServiceByDesktopName("kmail")) {
        KMessageBox::error(0,i18n("No running instance of KMail found."));
        return false;
      }
    }
    org::kde::kmail::kmail kmail("org.kde.kmail", "/KMail", QDBusConnection::sessionBus());
    kapp->updateRemoteUserTimestamp("org.kde.kmail");
    if (attachment.isEmpty()) {
      return kmail.openComposer(to,"",bcc ? from : "",subject,body,false).isValid();
    } else {
      QString meth;
      int idx = attachment.indexOf( "METHOD" );
      if (idx>=0) {
        idx = attachment.indexOf( ':', idx )+1;
        const int newline = attachment.indexOf('\n',idx);
        meth = attachment.mid(idx, newline - idx - 1);
        meth = meth.toLower().trimmed();
      } else {
        meth = "publish";
      }
      return kmail.openComposer
          (to,"",bcc ? from : "",subject,body,false,"cal.ics","7bit",
           attachment.toUtf8(),"text","calendar","method",meth,"attachment",
           "utf-8").isValid();
    }
  }
  return true;
}

