/*
  This file is part of the Groupware/KOrganizer integration.

  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.trolltech.com and http://www.kde.org respectively

  Copyright (c) 2002-2004 Klarälvdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>

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

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#include "kogroupware.h"

#include "freebusymanager.h"
#include "calendarview.h"
#include "mailscheduler.h"
#include "kogroupwareincomingdialog.h"
#include "koviewmanager.h"
#include "kocore.h"

#include <libemailfunctions/email.h>

#include <libkcal/incidencebase.h>
#include <libkcal/attendee.h>
#include <libkcal/freebusy.h>
#include <libkcal/journal.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>
#include <libkcal/incidenceformatter.h>

#include <kdebug.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kapplication.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <kstandarddirs.h>
#include <kdirwatch.h>

#include <qfile.h>
#include <qregexp.h>

#include <mimelib/enum.h>

#include <stdlib.h>
#include <qdir.h>
#include "koprefs.h"

FreeBusyManager *KOGroupware::mFreeBusyManager = 0;

KOGroupware *KOGroupware::mInstance = 0;

KOGroupware *KOGroupware::create( CalendarView *view,
                                  KCal::Calendar *calendar )
{
  if( !mInstance )
    mInstance = new KOGroupware( view, calendar );
  return mInstance;
}

KOGroupware *KOGroupware::instance()
{
  // Doesn't create, that is the task of create()
  Q_ASSERT( mInstance );
  return mInstance;
}


KOGroupware::KOGroupware( CalendarView* view, KCal::Calendar* calendar )
  : QObject( 0, "kmgroupware_instance" )
{
  mView = view;
  mCalendar = calendar;

  // Set up the dir watch of the three incoming dirs
  KDirWatch* watcher = KDirWatch::self();
  watcher->addDir( locateLocal( "data", "korganizer/income.accepted/" ) );
  watcher->addDir( locateLocal( "data", "korganizer/income.cancel/" ) );
  watcher->addDir( locateLocal( "data", "korganizer/income.reply/" ) );
  connect( watcher, SIGNAL( dirty( const QString& ) ),
           this, SLOT( incomingDirChanged( const QString& ) ) );
  // Now set the ball rolling
  incomingDirChanged( locateLocal( "data", "korganizer/income.accepted/" ) );
  incomingDirChanged( locateLocal( "data", "korganizer/income.cancel/" ) );
  incomingDirChanged( locateLocal( "data", "korganizer/income.reply/" ) );
}

FreeBusyManager *KOGroupware::freeBusyManager()
{
  if ( !mFreeBusyManager ) {
    mFreeBusyManager = new FreeBusyManager( this, "freebusymanager" );
    mFreeBusyManager->setCalendar( mCalendar );
    connect( mCalendar, SIGNAL( calendarChanged() ),
             mFreeBusyManager, SLOT( slotPerhapsUploadFB() ) );
  }

  return mFreeBusyManager;
}

void KOGroupware::incomingDirChanged( const QString& path )
{
  const QString incomingDirName = locateLocal( "data","korganizer/" )
                                  + "income.";
  if ( !path.startsWith( incomingDirName ) ) {
    kdDebug(5850) << "incomingDirChanged: Wrong dir " << path << endl;
    return;
  }
  QString action = path.mid( incomingDirName.length() );
  while ( action.length() > 0 && action[ action.length()-1 ] == '/' )
    // Strip slashes at the end
    action.truncate( action.length()-1 );

  // Handle accepted invitations
  QDir dir( path );
  QStringList files = dir.entryList( QDir::Files );
  if ( files.count() == 0 )
    // No more files here
    return;

  // Read the file and remove it
  QFile f( path + "/" + files[0] );
  if (!f.open(IO_ReadOnly)) {
    kdError(5850) << "Can't open file '" << files[0] << "'" << endl;
    return;
  }
  QTextStream t(&f);
  t.setEncoding( QTextStream::UnicodeUTF8 );
  QString receiver = KPIM::getEmailAddr( t.readLine() );
  QString iCal = t.read();

  ScheduleMessage *message = mFormat.parseScheduleMessage( mCalendar, iCal );
  if ( !message ) {
    QString errorMessage;
    if (mFormat.exception())
      errorMessage = "\nError message: " + mFormat.exception()->message();
    kdDebug(5850) << "MailScheduler::retrieveTransactions() Error parsing"
                  << errorMessage << endl;
    f.close();
    return;
  } else
    f.remove();

  KCal::Scheduler::Method method =
    static_cast<KCal::Scheduler::Method>( message->method() );
  KCal::ScheduleMessage::Status status = message->status();
  KCal::Incidence* incidence =
    dynamic_cast<KCal::Incidence*>( message->event() );
  KCal::MailScheduler scheduler( mCalendar );
  if ( action.startsWith( "accepted" ) ) {
    // Find myself and set to answered and accepted
    KCal::Attendee::List attendees = incidence->attendees();
    KCal::Attendee::List::ConstIterator it;
    for ( it = attendees.begin(); it != attendees.end(); ++it ) {
      if( (*it)->email() == receiver ) {
        (*it)->setStatus( KCal::Attendee::Accepted );
        (*it)->setRSVP(false);
        break;
      }
    }
    scheduler.acceptTransaction( incidence, method, status );
  } else if ( action.startsWith( "cancel" ) || action.startsWith( "reply" ) )
    scheduler.acceptTransaction( incidence, method, status );
  else
    kdError(5850) << "Unknown incoming action " << action << endl;
  mView->updateView();
}

class KOInvitationFormatterHelper : public InvitationFormatterHelper
{
  public:
    virtual QString generateLinkURL( const QString &id ) { return "kmail:groupware_request_" + id; }
};

QString KOGroupware::formatICal( const QString& iCal )
{
  KCal::CalendarLocal cl( mCalendar->timeZoneId() );
  KOInvitationFormatterHelper helper;
  return IncidenceFormatter::formatICalInvitation( iCal, &cl, &helper );
}

QString KOGroupware::formatTNEF( const QByteArray& tnef )
{
  KCal::CalendarLocal cl( mCalendar->timeZoneId() );
  KOInvitationFormatterHelper helper;
  return IncidenceFormatter::formatTNEFInvitation( tnef, &cl, &helper );
}

QString KOGroupware::msTNEFToVPart( const QByteArray& tnef )
{
  return IncidenceFormatter::msTNEFToVPart( tnef );
}


bool KOGroupware::incomingEventRequest( const QString& request,
                                        const QString& receiver,
                                        const QString& vCalIn )
{
  // This was the code to accept a choice from KMail. Needs porting
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
    if( (*it)->email() == receiver ) {
      // We are the current one, and even the receiver, note
      // this and quit searching.
      myself = (*it);
      break;
    }

    if ( KOPrefs::instance()->thatIsMe( (*it)->email() ) ) {
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
  QString messageText =
    mFormat.createScheduleMessage( newEvent, KCal::Scheduler::Reply );
  scheduler.performTransaction( newEvent, KCal::Scheduler::Reply );

  // Fix broken OL appointments
  if( vCalIn.contains( "PRODID:-//Microsoft" ) ) {
    // OL doesn't send the organizer as an attendee as it should
    Attendee* organizer = new KCal::Attendee( event->organizer().name(),
                                             event->organizer().email(), false,
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
void KOGroupware::incomingResourceRequest( const QValueList<QPair<QDateTime,
                                           QDateTime> >& busy,
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
  QValueList<QPair<QDateTime, QDateTime> >::ConstIterator it;
  for( it = busy.begin(); it != busy.end(); ++it ) {
    if( (*it).second <= start || (*it).first >= end )
      // Busy period ends before try period or starts after try period
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
  KCal::Attendee::List::ConstIterator it2;
  for( it2 = attendees.begin(); it2 != attendees.end(); ++it2 ) {
    if( (*it2)->email().utf8() == resource ) {
      resourceAtt = *it2;
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
  QString messageText =
    mFormat.createScheduleMessage( event, KCal::Scheduler::Reply );
  // kdDebug(5850) << "Sending vCal back to KMail: " << messageText << endl;
  vCalOut = messageText;
  vCalOutOK = true;
  return;
}


/*!
  This method is called when the user has invited people and is now
  receiving the answers to the invitation.
*/
bool KOGroupware::incidenceAnswer( const QString& vCal )
{
  // Parse the event reply
  KCal::ScheduleMessage *message = mFormat.parseScheduleMessage( mCalendar,
                                                                 vCal );
  if( !message ) {
    // a parse error of some sort
    KMessageBox::
      error( mView,
             i18n( "<b>There was a problem parsing the iCal data:</b><br>%1" )
             .arg( mFormat.exception()->message() ) );
    return false;
  }

  KCal::IncidenceBase* incidence = message->event();

  // Enter the answer into the calendar.
  QString uid = incidence->uid();
  KCal::MailScheduler scheduler( mCalendar );
  if( !scheduler.acceptTransaction( incidence,
                                    (KCal::Scheduler::Method)message->method(),
                                    message->status() ) ) {
    KMessageBox::error( mView, i18n("Scheduling failed") );
    return false;
  }

  mView->updateView();
  return true;
}

bool KOGroupware::cancelIncidence( const QString& iCal )
{
  // Parse the event reply
  KCal::ScheduleMessage *message = mFormat.parseScheduleMessage( mCalendar,
                                                                 iCal );
  if( !message ) {
    // a parse error of some sort
    KMessageBox::
      error( mView,
             i18n( "<b>There was a problem parsing the iCal data:</b><br>%1" )
             .arg( mFormat.exception()->message() ) );
    return false;
  }

  KCal::IncidenceBase* incidence = message->event();

  // Enter the answer into the calendar.
  Incidence *inc = mCalendar->incidence( incidence->uid() );
  if( !inc )
    return false;

  mCalendar->deleteIncidence( inc );
  return true;
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
  bool isOrganizer = KOPrefs::instance()->thatIsMe( incidence->organizer().email() );

  int rc = 0;
  if( isOrganizer ) {
    // Figure out if there are other people involved in this incidence
    bool otherPeople = false;
    Attendee::List attendees = incidence->attendees();
    Attendee::List::ConstIterator it;
    for ( it = attendees.begin(); it != attendees.end(); ++it ) {
      // Don't send email to ourselves
      if ( !KOPrefs::instance()->thatIsMe( (*it)->email() ) ) {
        otherPeople = true;
        break;
      }
    }
    if( !otherPeople )
      // You never send something out if no others are involved
      return true;

    QString txt = i18n( "This %1 includes other people. "
                        "Should email be sent out to the attendees?" )
      .arg( i18n( incidence->type() ) );
    rc = KMessageBox::questionYesNoCancel( parent, txt,
                                           i18n("Group scheduling email") );
  // @TODO: use a visitor here
  } else if( incidence->type() == "Todo" ) {
    if( method == Scheduler::Request )
      // This is an update to be sent to the organizer
      method = Scheduler::Reply;

    // Ask if the user wants to tell the organizer about the current status
    QString txt = i18n( "Do you want to send a status update to the "
                        "organizer of this task?");
    rc = KMessageBox::questionYesNo( parent, txt );
  } else if( incidence->type() == "Event" ) {
    // When you're not the organizer of an event, an update mail can
    // never be sent out
    // Pending(Bo): So how will an attendee cancel his participation?
    QString txt;
    if( isDeleting )
      txt = i18n( "You are not the organizer of this event. "
                  "Deleting it will bring your calendar out of sync "
                  "with the organizers calendar. Do you really want "
                  "to delete it?" );
    else
      txt = i18n( "You are not the organizer of this event. "
                  "Editing it will bring your calendar out of sync "
                  "with the organizers calendar. Do you really want "
                  "to edit it?" );
    rc = KMessageBox::questionYesNo( parent, txt );
    return ( rc == KMessageBox::Yes );
  } else {
    kdWarning(5850) << "Some unimplemented thing happened" << endl;
    return true;
  }

  if( rc == KMessageBox::Yes ) {
    // We will be sending out a message here. Now make sure there is
    // some summary
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
