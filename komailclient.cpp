/*
  This file is part of KOrganizer.

  Copyright (c) 1998 Barry D Benowitz <b.benowitz@telesciences.com>
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2009 Allen Winter <winter@kde.org>

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
#include "kmailinterface.h" //generated
#include "version.h"

#include <KCal/Attendee>
#include <KCal/Incidence>
#include <KCal/IncidenceBase>
#include <KCal/IncidenceFormatter>

#include <KPIMUtils/Email>

#include <KPIMIdentities/Identity>

#include <KApplication>
#include <KDebug>
#include <KLocale>
#include <KMessageBox>
#include <KShell>
#include <KStandardDirs>
#include <KSystemTimeZone>
#include <KToolInvocation>

#if 0

#include <unistd.h>
#include <stdio.h>
#endif

KOMailClient::KOMailClient()
{
}

KOMailClient::~KOMailClient()
{
}

bool KOMailClient::mailAttendees( IncidenceBase *incidence,
                                  const Identity &identity,
                                  bool bccMe, const QString &attachment,
                                  bool useSendmail )
{
  Attendee::List attendees = incidence->attendees();
  if ( attendees.count() == 0 ) {
    return false;
  }

  const QString from = incidence->organizer().fullName();
  const QString organizerEmail = incidence->organizer().email();

  QStringList toList;
  QStringList ccList;
  for ( int i=0; i<attendees.count(); ++i ) {
    Attendee *a = attendees.at(i);

    const QString email = a->email();
    if ( email.isEmpty() ) {
      continue;
    }

    // In case we (as one of our identities) are the organizer we are sending
    // this mail. We could also have added ourselves as an attendee, in which
    // case we don't want to send ourselves a notification mail.
    if ( organizerEmail == email ) {
      continue;
    }

    // Build a nice address for this attendee including the CN.
    QString tname, temail;
    const QString username = KPIMUtils::quoteNameIfNecessary( a->name() );
    // ignore the return value from extractEmailAddressAndName() because
    // it will always be false since tusername does not contain "@domain".
    KPIMUtils::extractEmailAddressAndName( username, temail, tname );
    tname += " <" + email + '>';

    // Optional Participants and Non-Participants are copied on the email
    if ( a->role() == Attendee::OptParticipant ||
         a->role() == Attendee::NonParticipant ) {
      ccList << tname;
    } else {
      toList << tname;
    }
  }
  if( toList.count() == 0 && ccList.count() == 0 ) {
    // Not really to be called a groupware meeting, eh
    return false;
  }
  QString to;
  if ( toList.count() > 0 ) {
    to = toList.join( ", " );
  }
  QString cc;
  if ( ccList.count() > 0 ) {
    cc = ccList.join( ", " );
  }

  QString subject;
  if ( incidence->type() != "FreeBusy" ) {
    Incidence *inc = static_cast<Incidence *>( incidence );
    subject = inc->summary();
  } else {
    subject = "Free Busy Object";
  }

  QString body = IncidenceFormatter::mailBodyStr( incidence, KSystemTimeZones::local() );

  return send( identity, from, to, cc, subject, body, bccMe, attachment, useSendmail );
}

bool KOMailClient::mailOrganizer( IncidenceBase *incidence,
                                  const Identity &identity,
                                  const QString &from, bool bccMe,
                                  const QString &attachment,
                                  const QString &sub, bool useSendmail )
{
  QString to = incidence->organizer().fullName();

  QString subject = sub;
  if ( incidence->type() != "FreeBusy" ) {
    Incidence *inc = static_cast<Incidence *>( incidence );
    if ( subject.isEmpty() ) {
      subject = inc->summary();
    }
  } else {
    subject = "Free Busy Message";
  }

  QString body = IncidenceFormatter::mailBodyStr( incidence, KSystemTimeZones::local() );

  return send( identity, from, to, QString(), subject, body, bccMe, attachment, useSendmail );
}

bool KOMailClient::mailTo( IncidenceBase *incidence, const Identity &identity,
                           const QString &from, bool bccMe,
                           const QString &recipients, const QString &attachment,
                           bool useSendmail )
{
  QString subject;

  if ( incidence->type() != "FreeBusy" ) {
    Incidence *inc = static_cast<Incidence *>( incidence );
    subject = inc->summary();
  } else {
    subject = "Free Busy Message";
  }
  QString body = IncidenceFormatter::mailBodyStr( incidence, KSystemTimeZones::local() );

  return send( identity, from, recipients, QString(), subject, body, bccMe,
               attachment, useSendmail );
}

bool KOMailClient::send( const Identity &identity,
                         const QString &from, const QString &_to,
                         const QString &cc, const QString &subject,
                         const QString &body, bool bccMe,
                         const QString &attachment, bool useSendmail )
{
  // We must have a recipients list for most MUAs. Thus, if the 'to' list
  // is empty simply use the 'from' address as the recipient.
  QString to = _to;
  if ( to.isEmpty() ) {
    to = from;
  }
  kDebug() << "\nFrom:" << from
           << "\nTo:" << to
           << "\nCC:" << cc
           << "\nSubject:" << subject << "\nBody: \n" << body
           << "\nAttachment:\n" << attachment;

  if ( useSendmail ) {
    bool needHeaders = true;

    QString command = KStandardDirs::findExe(
      QString::fromLatin1( "sendmail" ), QString::fromLatin1( "/sbin:/usr/sbin:/usr/lib" ) );

    if ( !command.isEmpty() ) {
      command += QString::fromLatin1( " -oi -t" );
    } else {
      command = KStandardDirs::findExe( QString::fromLatin1( "mail" ) );
      if ( command.isEmpty() ) {
        return false; // give up
      }

      command.append( QString::fromLatin1( " -s " ) );
      command.append( KShell::quoteArg( subject ) );

      if ( bccMe ) {
        command.append( QString::fromLatin1( " -b " ) );
        command.append( KShell::quoteArg( from ) );
      }

      if ( !cc.isEmpty() ) {
        command.append( " -c " );
        command.append( KShell::quoteArg( cc ) );
      }

      command.append( " " );
      command.append( KShell::quoteArg( to ) );

      needHeaders = false;
    }

    FILE *fd = popen( command.toLocal8Bit(), "w" );
    if ( !fd ) {
      kError() << "Unable to open a pipe to" << command;
      return false;
    }

    QString textComplete;
    if ( needHeaders ) {
      textComplete += QString::fromLatin1( "From: " ) + from + '\n';
      textComplete += QString::fromLatin1( "To: " ) + to + '\n';
      if ( !cc.isEmpty() ) {
        textComplete += QString::fromLatin1( "Cc: " ) + cc + '\n';
      }
      if ( bccMe ) {
        textComplete += QString::fromLatin1( "Bcc: " ) + from + '\n';
      }
      textComplete += QString::fromLatin1( "Subject: " ) + subject + '\n';
      textComplete += QString::fromLatin1( "X-Mailer: KOrganizer" ) + korgVersion + '\n';
    }
    textComplete += '\n'; // end of headers
    textComplete += body;
    textComplete += '\n';
    textComplete += attachment;

    fwrite( textComplete.toLocal8Bit(), textComplete.length(), 1, fd );

    pclose( fd );
  } else {
    if ( !QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.kmail" ) ) {
      if ( KToolInvocation::startServiceByDesktopName( "kmail" ) ) {
        KMessageBox::error( 0, i18n( "No running instance of KMail found." ) );
        return false;
      }
    }
    org::kde::kmail::kmail kmail( "org.kde.kmail", "/KMail", QDBusConnection::sessionBus() );
    kapp->updateRemoteUserTimestamp( "org.kde.kmail" );
    if ( attachment.isEmpty() ) {
      return kmail.openComposer( to, cc, bccMe ? from : QString(), subject, body, false ).isValid();
    } else {
      QString meth;
      int idx = attachment.indexOf( "METHOD" );
      if ( idx >= 0 ) {
        idx = attachment.indexOf( ':', idx ) + 1;
        const int newline = attachment.indexOf( '\n', idx );
        meth = attachment.mid( idx, newline - idx - 1 );
        meth = meth.toLower().trimmed();
      } else {
        meth = "publish";
      }
      return kmail.openComposer(
        to, cc, bccMe ? from : QString(), subject, body, false, "cal.ics", "7bit",
        attachment.toUtf8(), "text", "calendar", "method", meth, "attachment",
        "utf-8", identity.uoid() ).isValid();
    }
  }
  return true;
}

