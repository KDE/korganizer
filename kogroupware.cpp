/*
  This file is part of the Groupware/KOrganizer integration.

  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.trolltech.com and http://www.kde.org respectively

  Copyright (c) 2002 Klarälvdalens Datakonsult AB

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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston,
  MA  02111-1307, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kogroupware.h"
#include "koprefs.h"
#include "calendarview.h"
#include "mailscheduler.h"
#include "kogroupwareincomingdialog.h"
#include "koviewmanager.h"

#include <libkcal/incidencebase.h>
#include <libkcal/attendee.h>
#include <libkcal/freebusy.h>
#include <libkcal/journal.h>

#include <kdebug.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kapplication.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <dcopref.h>

#include <qfile.h>
#include <qregexp.h>

#include <mimelib/enum.h>

KOGroupware* KOGroupware::mInstance = 0;

KOGroupware* KOGroupware::create( CalendarView* view, KCal::Calendar* calendar )
{
  if( !mInstance )
    mInstance = new KOGroupware( view, calendar );
  return mInstance;
}

KOGroupware* KOGroupware::instance()
{
  // Doesn't create, that is the task of create()
  Q_ASSERT( mInstance );
  return mInstance;
}


KOGroupware::KOGroupware( CalendarView* view, KCal::Calendar* calendar )
  : QObject( 0, "kmgroupware_instance" )/*, mKMail( 0 )*/
{
  mView = view;
  mCalendar = calendar;

//   kdDebug(5850) << "KOGroupware::KOGroupware(), connecting " << kmailTarget->name() << ", className="<< kmailTarget->className() << endl;
}


/*!
  This method processes incoming event requests. The requests can
  already be tentatively or unconditionally accepted, in which case
  all this method does is registering them in the calendar. If the
  event request is a query, a dialog will be shown that lets the
  user select whether to accept, tentatively accept or decline the
  invitation.

  \param state whether the request is preaccepted, tentatively
  preaccepted or not yet decided on. The state Declined should not be
  used for incoming state values, it is only used for internal
  handling.
  \param vCalIn a string containing the vCal representation of the
  event request
  \param vCalInOK is true after the call if vCalIn was well-formed
  \param vCalOut a string containing the vCal representation of the
  answer. This string is suitable for sending back to the event
  originator. If vCalInOK is false on return, the value of this
  parameter is undefined.
  \param vCalOutOK is true if the user has made a decision, or if the
  event was preaccepted. If this parameter is false, the user has
  declined to decide on whether to accept or decline the event. The
  value of vCalOut is undefined in this case.
*/

bool KOGroupware::incomingEventRequest( const QString& request,
                                        const QCString& receiver,
                                        const QString& vCalIn )
{
  EventState state;
  if( request == "accept" )
    state = Accepted;
  else if( request == "accept conditionally" )
    state = ConditionallyAccepted;
  else if( request == "decline" )
    state = Declined;
  else if( request == "check" )
    state = Request;
  else
    return false;

  // Parse the event request into a ScheduleMessage; this needs to
  // be done in any case.
  KCal::ScheduleMessage *message = mFormat.parseScheduleMessage( mCalendar,
                                                                 vCalIn );
  if( message ) {
    kdDebug(5850) << "KOGroupware::incomingEventRequest: got message '"
                  << vCalIn << "'" << endl;
  } else {
    QString errorMessage;
    if( mFormat.exception() ) {
      errorMessage = mFormat.exception()->message();
    }
    kdDebug(5850) << "KOGroupware::incomingEventRequest() Error parsing "
                  << "message: " << errorMessage << endl;
    // If the message was broken, there's nothing we can do.
    return false;
  }

  KCal::Incidence* event = dynamic_cast<KCal::Incidence*>( message->event() );
  Q_ASSERT( event );
  if( !event ) { // something bad happened, just to be safe
    kdDebug(5850) << "KOGroupware::incomingEventRequest(): Not an event???\n";
    return false;
  }

  // Now check if the event needs to be accepted or if this is
  // already done.
  if( state == Request ) {
    // Need to accept, present it to the user
    KOGroupwareIncomingDialog dlg( event );
    int ret = dlg.exec();
    if( ret == QDialog::Rejected ) {
      // User declined to make a choice, we can't send a vCal back
      kdDebug(5850) << "KOGroupware::incomingEventRequest(): User canceled\n";
      return false;
    }

    if( dlg.isDeclined() )
      state = Declined;
    else if( dlg.isConditionallyAccepted() )
      state = ConditionallyAccepted;
    else if( dlg.isAccepted() )
      state = Accepted;
    else
      kdDebug(5850) << "KOGroupware::incomingEventRequest(): unknown "
					<< "event request state" << endl;
  }

  // If the event has an alarm, make sure it doesn't have a negative time.
  // This is yet another OL workaround
#if 0
  // PENDING(bo): Disabled for now, until I figure out how the old offset
  // matches the two new offsets
  Alarm::List alarms = event->alarms();
  Alarm::List::ConstIterator it;
  for ( it = alarms.begin(); it != alarms.end(); ++it) {
    if ( (*it)->hasTime() ) {
      QDateTime t = (*it)->time();
      int offset = event->dtStart().secsTo( t );
      if( offset > 0 )
       // PENDING(Bo): Not implemented yet
       kdDebug(5850) << "Warning: Alarm fires after the event\n";
    } else {
      int offset = (*it)->offset().asSeconds();
      if( offset > 0 ) {
       // This number should be negative so the alarm fires before the event
       Duration d( -offset );
       (*it)->setOffset( d );
      }
    }
  }
#endif

  // Enter the event into the calendar. We just create a
  // Scheduler, because all the code we need is already there. We
  // take an MailScheduler, because we need a concrete one, but we
  // really only want code from Scheduler.
  // PENDING(kalle) Handle tentative acceptance differently.
  KCal::MailScheduler scheduler( mCalendar );
  if( state == Accepted || state == ConditionallyAccepted ) {
    scheduler.acceptTransaction( event,
                                 (KCal::Scheduler::Method)message->method(),
                                 message->status() );
    mView->updateView();
  }

  KCal::Attendee::List attendees = event->attendees();
  KCal::Attendee::List::ConstIterator it;
  KCal::Attendee* myself = 0;
  // Find myself, there will always be all attendees listed, even if
  // only I need to answer it.
  for ( it = attendees.begin(); it != attendees.end(); ++it ) {
    if( (*it)->email().utf8() == receiver ) {
      // We are the current one, and even the receiver, note
      // this and quit searching.
      myself = (*it);
      break;
    }

    if( (*it)->email() == KOPrefs::instance()->email() ) {
      // If we are the current one, note that. Still continue to
      // search in case we find the receiver himself.
      myself = (*it);
    }
  }

  Q_ASSERT( myself );

  KCal::Attendee* newMyself = 0;
  if( myself ) {
    switch( state ) {
    case Accepted:
      myself->setStatus( KCal::Attendee::Accepted );
      break;
    case ConditionallyAccepted:
      myself->setStatus( KCal::Attendee::Tentative );
      break;
    case Declined:
      myself->setStatus( KCal::Attendee::Declined );
      break;
    default:
      ;
    };

    // No more request response
    myself->setRSVP(false);

    event->updated();

    newMyself = new KCal::Attendee( myself->name(),
                                    receiver.isEmpty() ?
                                    myself->email() :
                                    receiver,
                                    myself->RSVP(),
                                    myself->status(),
                                    myself->role(),
                                    myself->uid() );
  }

  event->updated();

  // Send back the answer; construct it on the base of state. We
  // make a clone of the event since we need to manipulate it here.
  // NOTE: This contains a workaround around a libkcal bug: REPLY
  // vCals may not have more than one ATTENDEE (as libical correctly
  // specifies), but libkcal always writes out all the ATTENDEEs,
  // thus producing invalid vCals. We make a clone of the vEvent
  // here and remove all attendees except ourselves.
  Incidence* newIncidence = event->clone();
  Event* newEvent = static_cast<KCal::Event*>( newIncidence );

#if 0
  // OL compatibility thing. To be ported.
  // The problem is that OL is braindead when it comes to receiving
  // events that mention alarms. So strip them before sending to OL
  // people
  bool stripAlarms = false;
  emit getStripAlarmsForSending( stripAlarms );
  if( stripAlarms )
    // Strip alarms from the send
    newEvent->clearAlarms();
#endif

  newEvent->clearAttendees();
  if( newMyself )
    newEvent->addAttendee( newMyself );

  // Create the outgoing vCal
  QString messageText = mFormat.createScheduleMessage( newEvent,
                                                       KCal::Scheduler::Reply );
  scheduler.performTransaction( newEvent, KCal::Scheduler::Reply );

  // Fix broken OL appointments
  if( vCalIn.contains( "PRODID:-//Microsoft" ) ) {
    // OL doesn't send the organizer as an attendee as it should
    Attendee* organizer = new KCal::Attendee( i18n("Organizer"),
                                             event->organizer(), false,
                                             KCal::Attendee::Accepted );
    event->addAttendee( organizer );
  }

  kdDebug(5850) << "Done" << endl;
  return true;
}


/*!
  This method processes resource requests. KMail has at this point
  already decided whether the request should be accepted or declined.
*/
void KOGroupware::incomingResourceRequest( const QValueList<QPair<QDateTime, QDateTime> >& busy,
                                           const QCString& resource,
                                           const QString& vCalIn,
                                           bool& vCalInOK,
                                           QString& vCalOut,
                                           bool& vCalOutOK,
                                           bool& isFree,
                                           QDateTime& start,
                                           QDateTime& end )
{
  // Parse the event request into a ScheduleMessage; this needs to
  // be done in any case.
  KCal::ScheduleMessage *message = mFormat.parseScheduleMessage( mCalendar,
								 vCalIn );
  if( message )
    vCalInOK = true;
  else {
    QString errorMessage;
    if( mFormat.exception() ) {
      errorMessage = mFormat.exception()->message();
    }
    kdDebug(5850) << "KOGroupware::incomingResourceRequest() Error parsing "
      "message: " << errorMessage << endl;
    vCalInOK = false;
    // If the message was broken, there's nothing we can do.
    return;
  }

  KCal::Event* event = dynamic_cast<KCal::Event*>( message->event() );
  Q_ASSERT( event );
  if( !event ) {
    // Something has gone badly wrong
    vCalInOK = false;
    return;
  }

  // Now find out whether the resource is free at the requested
  // time, take the opportunity to assign the reference parameters.
  start = event->dtStart();
  end = event->dtEnd();
  isFree = true;
  for( QValueList<QPair<QDateTime, QDateTime> >::ConstIterator it = busy.begin();
       it != busy.end(); ++it ) {
    if( (*it).second <= start || // busy period ends before try period
	(*it).first >= end )  // busy period starts after try period
      continue;
    else {
      isFree = false;
      break; // no need to search further
    }
  }

  // Send back the answer; construct it on the base of state
  KCal::Attendee::List attendees = event->attendees();
  KCal::Attendee* resourceAtt = 0;

  // Find the resource addresse, there will always be all attendees
  // listed, even if only one needs to answer it.
  KCal::Attendee::List::ConstIterator it;
  for( it = attendees.begin(); it != attendees.end(); ++it ) {
    if( (*it)->email().utf8() == resource ) {
      resourceAtt = *it;
      break;
    }
  }
  Q_ASSERT( resourceAtt );
  if( resourceAtt ) {
    if( isFree )
      resourceAtt->setStatus( KCal::Attendee::Accepted );
    else
      resourceAtt->setStatus( KCal::Attendee::Declined );
  } else {
    vCalOutOK = false;
    return;
  }

  // Create the outgoing vCal
  QString messageText = mFormat.createScheduleMessage( event,
						       KCal::Scheduler::Reply );
  // kdDebug(5850) << "Sending vCal back to KMail: " << messageText << endl;
  vCalOut = messageText;
  vCalOutOK = true;
  return;
}


/*!
  This method is called when the user has invited people and is now
  receiving the answers to the invitation. Returns the updated vCal
  representing the whole incidence in the vCalOut parameter.

  If returning false because of something going wrong, set vCalOut
  to "false" to signal this to KMail.
*/

bool KOGroupware::incidenceAnswer( const QCString& /*sender*/,
				   const QString& vCalIn,
				   QString& vCalOut )
{
  vCalOut = "";

  // Parse the event request into a ScheduleMessage; this needs to
  // be done in any case.
  KCal::ScheduleMessage *message = mFormat.parseScheduleMessage( mCalendar, vCalIn );
  if( !message ) {
    // a parse error of some sort
    KMessageBox::error( mView, i18n("<b>There was a problem parsing the iCal data:</b><br>%1")
			.arg(mFormat.exception()->message()) );
    vCalOut = "false";
    return false;
  }

  KCal::IncidenceBase* incidence = message->event();

  // Enter the answer into the calendar. We just create a
  // Scheduler, because all the code we need is already there. We
  // take a MailScheduler, because we need a concrete one, but we
  // really only want code from Scheduler.
  QString uid = incidence->uid();
  KCal::MailScheduler scheduler( mCalendar );
  // TODO: Make this work
  if( !scheduler.acceptTransaction( incidence,
				    (KCal::Scheduler::Method)message->method(),
				    message->status()/*, sender*/ ) ) {
    KMessageBox::error( mView, i18n("Scheduling failed") );
    vCalOut = "false";
    return false;
  }

  mView->updateView();

  // Find the calendar entry that corresponds to this uid.
  if( Event* event = mCalendar->event( uid ) ) {
    vCalOut = mFormat.createScheduleMessage( event, KCal::Scheduler::Reply );
  } else if( Todo* todo = mCalendar->todo( uid ) )
    vCalOut = mFormat.createScheduleMessage( todo, KCal::Scheduler::Reply );
  else if( Journal* journal = mCalendar->journal( uid ) )
    vCalOut = mFormat.createScheduleMessage( journal, KCal::Scheduler::Reply );

  return true;
}

QString KOGroupware::getFreeBusyString()
{
  QDateTime start = QDateTime::currentDateTime();
  QDateTime end = start.addDays( KOPrefs::instance()->mPublishFreeBusyDays );

  FreeBusy freebusy( mCalendar, start, end );
  freebusy.setOrganizer( KOPrefs::instance()->email() );

//   kdDebug(5850) << "KOGroupware::publishFreeBusy(): startDate: "
//                 << KGlobal::locale()->formatDateTime( start ) << " End Date: "
//                 << KGlobal::locale()->formatDateTime( end ) << endl;

  return mFormat.createScheduleMessage( &freebusy, Scheduler::Publish );
}

#if 0
/*!
  This method is called when the user has selected to publish its
  free/busy list.
*/

void KOGroupware::publishFreeBusy()
{
  if( !KOPrefs::instance()->mPublishFreeBusy ) {
    KMessageBox::sorry( 0, i18n( "<qt>Publishing free/busy lists has been disabled. If you are sure that you want to publish your free/busy list, go to <em>Settings/Configure KOrganizer.../Groupware</em> and turn on publishing free/busy lists.</qt>" ) );
    return;
  }

  QString messageText = getFreeBusyString();

//   kdDebug(5850) << "KOGroupware::publishFreeBusy(): message = " << messageText
//                 << endl;

  // We need to massage the list a bit so that Outlook understands
  // it.
  messageText = messageText.replace( QRegExp( "ORGANIZER\\s*:MAILTO:" ), "ORGANIZER:" );

//   kdDebug(5850) << "KOGroupware::publishFreeBusy(): message after massaging = " << messageText
//                 << endl;

  QString emailHost = KOPrefs::instance()->email().mid( KOPrefs::instance()->email().find( '@' ) + 1 );

  // Create a local temp file and save the message to it
  KTempFile tempFile;
  tempFile.setAutoDelete( true );
  QTextStream* textStream = tempFile.textStream();
  if( textStream ) {
    *textStream << messageText;
    tempFile.close();

    // Put target string together
    KURL targetURL;
    if( KOPrefs::instance()->mPublishKolab ) {
      // we use Kolab
      QString server;
      if( KOPrefs::instance()->mPublishKolabServer == "%SERVER%" ||
          KOPrefs::instance()->mPublishKolabServer.isEmpty() )
        server = emailHost;
      else
        server = KOPrefs::instance()->mPublishKolabServer;

      targetURL.setProtocol( "webdavs" );
      targetURL.setHost( server );
      targetURL.setPath( "/freebusy/" + KOPrefs::instance()->mPublishUserName + ".vfb" );
      targetURL.setUser( KOPrefs::instance()->mPublishUserName );
      targetURL.setPass( KOPrefs::instance()->mPublishPassword );
    } else {
      // we use something else
      targetURL = KOPrefs::instance()->mPublishAnyURL.replace( "%SERVER%", emailHost );
      targetURL.setUser( KOPrefs::instance()->mPublishUserName );
      targetURL.setPass( KOPrefs::instance()->mPublishPassword );
    }

    if( !KIO::NetAccess::upload( tempFile.name(), targetURL ) ) {
      KMessageBox::sorry( 0,
                          i18n( "<qt>The software could not upload your free/busy list to the URL %1. There might be a problem with the access rights, or you specified an incorrect URL. The system said: <em>%2</em>.<br>Please check the URL or contact your system administrator.</qt>" ).arg( targetURL.url() ).arg( KIO::NetAccess::lastErrorString() ) );
    }
  }
}
#endif

FBDownloadJob::FBDownloadJob( const QString& email, const KURL& url, KOGroupware* kogroupware, const char* name )
  : QObject( kogroupware, name ), mKogroupware(kogroupware), mEmail( email )
{
  KIO::Job* job = KIO::get( url, false, false );
  connect( job, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( slotResult( KIO::Job* ) ) );
  connect( job, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
           this, SLOT( slotData( KIO::Job*, const QByteArray& ) ) );
}

FBDownloadJob::~FBDownloadJob()
{
  // empty for now
}

void FBDownloadJob::slotData( KIO::Job*, const QByteArray& data)
{
  QByteArray tmp = data;
  tmp.resize( tmp.size() + 1 );
  tmp[tmp.size()-1] = 0;
  mFBData += tmp;
}

void FBDownloadJob::slotResult( KIO::Job* job )
{
  if( job->error() ) {
    kdDebug(5850) << "FBDownloadJob::slotResult() job error :-(" << endl;
  }

  FreeBusy* fb = mKogroupware->parseFreeBusy( mFBData );
  emit fbDownloaded( mEmail, fb );
  delete this;
}

bool KOGroupware::downloadFreeBusyData( const QString& email, QObject* receiver, const char* member )
{
  // Don't do anything with free/busy if the user does not want it.
  if( !KOPrefs::instance()->mRetrieveFreeBusy )
    return false;

  // Sanity check: Don't download if it's not a correct email
  // address (this also avoids downloading for "(empty email)").
  int emailpos = email.find( '@' );
  if( emailpos == -1 )
    return false;

  // Cut off everything left of the @ sign to get the user name.
  QString emailName = email.left( emailpos );
  QString emailHost = email.mid( emailpos + 1 );

  // Put download string together
  KURL sourceURL;
  if( KOPrefs::instance()->mRetrieveKolab ) {
    // we use Kolab
    QString server;
    if( KOPrefs::instance()->mRetrieveKolabServer == "%SERVER%" ||
        KOPrefs::instance()->mRetrieveKolabServer.isEmpty() )
      server = emailHost;
    else
      server = KOPrefs::instance()->mRetrieveKolabServer;

    sourceURL.setProtocol( "webdavs" );
    sourceURL.setHost( server );
    sourceURL.setPass( KOPrefs::instance()->mRetrievePassword );
    sourceURL.setUser( KOPrefs::instance()->mRetrieveUserName );
    sourceURL.setPath( QString::fromLatin1( "/freebusy/" ) + emailName +
                       QString::fromLatin1( ".vfb" ) );
  } else {
    // we use something else
    QString anyurl = KOPrefs::instance()->mRetrieveAnyURL;
    if( anyurl.contains( "%SERVER%" ) )
      anyurl.replace( "%SERVER%", emailHost );
    sourceURL = anyurl;
  }

  FBDownloadJob* job = new FBDownloadJob( email, sourceURL, this, "fb_download_job" );
  connect( job, SIGNAL( fbDownloaded( const QString&, FreeBusy*) ),
           receiver, member );

  return true;
}

KCal::FreeBusy* KOGroupware::parseFreeBusy( const QCString& data )
{
  KCal::FreeBusy* fb = 0;
  QString freeBusyVCal = QString::fromUtf8(data);
  KCal::ScheduleMessage *message = mFormat.parseScheduleMessage( mCalendar,
                                                                 freeBusyVCal );
  if( message ) {
    KCal::IncidenceBase* event = message->event();
    Q_ASSERT( event );

    if( event ) {
      // Enter the answer into the calendar. We just create a
      // Scheduler, because all the code we need is
      // already there. We take a MailScheduler, because
      // we need a concrete one, but we really only want
      // code from Scheduler.
      KCal::MailScheduler scheduler( mCalendar );
      scheduler.acceptTransaction( event,
                                   (KCal::Scheduler::Method)message->method(),
                                   message->status() );
      fb = dynamic_cast<KCal::FreeBusy*>( event );
      Q_ASSERT( fb );
    }
  }
  return fb;
}


/* This function sends mails if necessary, and makes sure the user really
 * want to change his calendar.
 *
 * Return true means accept the changes
 * Return false means revert the changes
 */
bool KOGroupware::sendICalMessage( QWidget* parent,
				   KCal::Scheduler::Method method,
				   Incidence* incidence, bool isDeleting )
{
  bool isOrganizer = KOPrefs::instance()->email() == incidence->organizer();

  int rc = 0;
  if( isOrganizer ) {
    // Figure out if there are other people involved in this incidence
    bool otherPeople = false;
    Attendee::List attendees = incidence->attendees();
    Attendee::List::ConstIterator it;
    for ( it = attendees.begin(); it != attendees.end(); ++it ) {
      // Don't send email to ourselves
      if( (*it)->email() != KOPrefs::instance()->email() ) {
        otherPeople = true;
        break;
      }
    }
    if( !otherPeople )
      // You never send something out if no others are involved
      return true;

    QString type;
    if( incidence->type() == "Event") type = i18n("event");
    else if( incidence->type() == "Todo" ) type = i18n("task");
    else if( incidence->type() == "Journal" ) type = i18n("journal entry");
    else type = incidence->type();
    QString txt = i18n("This %1 includes other people. "
		       "Should email be sent out to the attendees?").arg(type);
    rc = KMessageBox::questionYesNoCancel( parent, txt, i18n("Group scheduling email") );
  } else if( incidence->type() == "Todo" ) {
    if( method == Scheduler::Request )
      // This is an update to be sent to the organizer
      method = Scheduler::Reply;

    // Ask if the user wants to tell the organizer about the current status
    QString txt = i18n("Do you want to send a status update to the organizer of this task?");
    rc = KMessageBox::questionYesNo( parent, txt );
  } else if( incidence->type() == "Event" ) {
    // When you're not the organizer of an event, an update mail can never be sent out
    // Pending(Bo): So how will an attendee cancel his participation?
    QString txt;
    if( isDeleting )
      txt = i18n("You are not the organizer of this event. "
                 "Deleting it will bring your calendar out of sync "
                 "with the organizers calendar. Do you really want to delete it?");
    else
      txt = i18n("You are not the organizer of this event. "
                 "Editing it will bring your calendar out of sync "
                 "with the organizers calendar. Do you really want to edit it?");
    rc = KMessageBox::questionYesNo( parent, txt );
    return ( rc == KMessageBox::Yes );
  } else {
    qFatal( "Some unimplemented thing happened" );
  }

  if( rc == KMessageBox::Yes ) {
    // We will be sending out a message here. Now make sure there is some summary
    if( incidence->summary().isEmpty() )
      incidence->setSummary( i18n("<No summary given>") );

    // Send the mail
    KCal::MailScheduler scheduler( mCalendar );
    scheduler.performTransaction( incidence, method );

    return true;
  } else if( rc == KMessageBox::No )
    return true;
  else
    return false;
}

#include "kogroupware.moc"
