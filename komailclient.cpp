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
#include "version.h"

#include <KCal/Attendee>
#include <KCal/Incidence>
#include <KCal/IncidenceBase>
#include <KCal/IncidenceFormatter>

#include <KPIMUtils/Email>

#include <KPIMIdentities/Identity>

#include <kmime/kmime_content.h>
#include <kmime/kmime_message.h>

#include <mailtransport/transport.h>
#include <mailtransport/transporttype.h>
#include <mailtransport/transportmanager.h>
#include <mailtransport/smtpjob.h>
#include <mailtransport/sendmailjob.h>
#include <mailtransport/messagequeuejob.h>
#include <mailtransport/sentbehaviourattribute.h>

#include <Akonadi/Item>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/CollectionCreateJob>
#include <Akonadi/AgentManager>
#include <Akonadi/AgentInstance>
#include <akonadi/kmime/specialcollections.h>
#include <akonadi/kmime/specialcollectionsrequestjob.h>

#include <KApplication>
#include <KDebug>
#include <KLocale>
#include <KMessageBox>
#include <KShell>
#include <KStandardDirs>
#include <KSystemTimeZone>
#include <KToolInvocation>
#include <KProtocolManager>

KOMailClient::KOMailClient() : QObject()
{
}

KOMailClient::~KOMailClient()
{
}

bool KOMailClient::mailAttendees( IncidenceBase *incidence,
                                  const Identity &identity,
                                  bool bccMe, const QString &attachment,
                                  const QString &mailTransport )
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

  return send( identity, from, to, cc, subject, body, false,
               bccMe, attachment, mailTransport );
}

bool KOMailClient::mailOrganizer( IncidenceBase *incidence,
                                  const Identity &identity,
                                  const QString &from, bool bccMe,
                                  const QString &attachment,
                                  const QString &sub, const QString &mailTransport )
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

  return send( identity, from, to, QString(), subject, body, false,
               bccMe, attachment, mailTransport );
}

bool KOMailClient::mailTo( IncidenceBase *incidence, const Identity &identity,
                           const QString &from, bool bccMe,
                           const QString &recipients, const QString &attachment,
                           const QString &mailTransport )
{
  QString subject;

  if ( incidence->type() != "FreeBusy" ) {
    Incidence *inc = static_cast<Incidence *>( incidence );
    subject = inc->summary();
  } else {
    subject = "Free Busy Message";
  }
  QString body = IncidenceFormatter::mailBodyStr( incidence, KSystemTimeZones::local() );

  return send( identity, from, recipients, QString(), subject, body, false,
               bccMe, attachment, mailTransport );
}

bool KOMailClient::send( const Identity &identity,
                         const QString &from, const QString &_to,
                         const QString &cc, const QString &subject,
                         const QString &body, bool hidden, bool bccMe,
                         const QString &attachment, const QString &mailTransport )
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

  QTime timer;
  timer.start();
           
  MailTransport::Transport *transport = MailTransport::TransportManager::self()->transportByName( mailTransport );
  if( ! transport ) {
    transport = MailTransport::TransportManager::self()->transportByName(
                  MailTransport::TransportManager::self()->defaultTransportName() );
  }
  if( ! transport ) {
    kWarning() << "Error fetching transport";
    return false;
  }

  const int transportId = transport->id();

  // First we need to check if the default outbox is known already. If not then we need to fetch it
  // using a SpecialCollectionsRequestJob to have the outbox ready.
  if( ! Akonadi::SpecialCollections::self()->hasDefaultCollection( Akonadi::SpecialCollections::Outbox ) ) {
    Akonadi::SpecialCollectionsRequestJob *rjob = new Akonadi::SpecialCollectionsRequestJob( this );
    rjob->requestDefaultCollection( Akonadi::SpecialCollections::Outbox );
    if( ! rjob->exec()) {
      kWarning() << "Error requesting outbox folder:" << rjob->errorText();
      return false;
    }
  }

  // Outbox should be ready now.
  Q_ASSERT( Akonadi::SpecialCollections::self()->hasDefaultCollection( Akonadi::SpecialCollections::Outbox ) );
  Akonadi::Collection outbox = Akonadi::SpecialCollections::self()->defaultCollection( Akonadi::SpecialCollections::Outbox );
  Q_ASSERT( outbox.isValid() );

  // Now build the message we like to send. The message KMime::Message::Ptr instance
  // will be the root message that has 2 additional message. The body itself and
  // the attached cal.ics calendar file.
  KMime::Message::Ptr message = KMime::Message::Ptr( new KMime::Message );
  // We need to set following 4 lines by hand else KMime::Content::addContent
  // will create a new Content instance for us to attach the main message
  // what we don't need cause we already have the main message instance where
  // 2 additional messages are attached.
  KMime::Headers::ContentType *ct = message->contentType();
  ct->setMimeType( "multipart/mixed" );
  ct->setBoundary( KMime::multiPartBoundary() );
  ct->setCategory( KMime::Headers::CCcontainer );
  message->contentTransferEncoding()->clear();  // 7Bit, decoded.

  // Set the headers
  message->userAgent()->fromUnicodeString( KProtocolManager::userAgentForApplication( "KOrganizer", KDEPIM_VERSION ), "utf-8" );
  message->from()->fromUnicodeString( from, "utf-8" );
  message->to()->fromUnicodeString( to, "utf-8" );
  message->cc()->fromUnicodeString( cc, "utf-8" );
  if( bccMe )
    message->bcc()->fromUnicodeString( from, "utf-8" ); //from==me, right?
  message->date()->setDateTime( KDateTime::currentLocalDateTime() );
  message->subject()->fromUnicodeString( subject, "utf-8" );

  // Set the first multipart, the body message.
  KMime::Content *bodyMessage = new KMime::Content;
  bodyMessage->contentType()->setMimeType( "text/plain" );
  bodyMessage->setBody( body.toUtf8() );

  // Set the sedcond multipart, the attachment.
  KMime::Content *attachMessage = new KMime::Content;
  KMime::Headers::ContentDisposition *attachDisposition = new KMime::Headers::ContentDisposition( attachMessage );
  attachDisposition->setFilename( "cal.ics" );
  attachDisposition->setDisposition( KMime::Headers::CDattachment );
  attachMessage->contentType()->setMimeType( "text/plain" );
  attachMessage->setHeader( attachDisposition );
  attachMessage->setBody( attachment.toUtf8() );

  // Job done, attach the both multiparts and assemble the message.
  message->addContent( bodyMessage );
  message->addContent( attachMessage );
  message->assemble();

  /*
  MailTransport::TransportJob *tjob = MailTransport::TransportManager::self()->createTransportJob( transportId );
  Q_ASSERT( tjob );
  Q_ASSERT( tjob->transport() );
  tjob->setSender( from );
  tjob->setTo( KPIMUtils::splitAddressList( to ) );
  tjob->setCc( KPIMUtils::splitAddressList( cc ) );
  if( bccMe )
    tjob->setBcc( KPIMUtils::splitAddressList(from) ); //from==me, right?
  if( ! tjob->exec() ) {
    kWarning() << "Error executing the transport job:" << tjob->errorText();
    return false;
  }
  Akonadi::Item item( transport->id() );
  item.setMimeType( KMime::Message::mimeType() );
  item.setPayload( message );
  Akonadi::ItemCreateJob *cjob = new Akonadi::ItemCreateJob( item, outbox, this );
  if( ! cjob->exec() ) {
    kWarning() << "Error creating message in outbox:" << cjob->errorText();
    return false;
  }
  item = cjob->item();
  Q_ASSERT( item.isValid() );
  Q_ASSERT( item.hasPayload<KMime::Message::Ptr>() );
  Q_ASSERT( MailTransport::TransportManager::self()->transportById(transportId, false) );
  */

  // Put the newly created item inh the MessageQueueJob.
  MailTransport::MessageQueueJob *qjob = new MailTransport::MessageQueueJob( this );
  qjob->setTransportId( transportId );
  qjob->setSentBehaviour( MailTransport::SentBehaviourAttribute::MoveToDefaultSentCollection );
  qjob->setMoveToCollection( -1 );
  qjob->setFrom( from );
  qjob->setTo( KPIMUtils::splitAddressList( to ) );
  qjob->setCc( KPIMUtils::splitAddressList( cc ) );
  if( bccMe )
    qjob->setBcc( KPIMUtils::splitAddressList( from ) );
  qjob->setMessage( message );
  if( ! qjob->exec() ) {
    kWarning() << "Error queuing message in outbox:" << qjob->errorText();
    return false;
  }

  // Everything done now.
  kDebug() << "Send mail finished. Time elapsed in ms:" << timer.elapsed();
  return true;
}

