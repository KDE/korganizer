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

#include <libkdepim/email.h>
#include <ktnef/ktnefparser.h>
#include <ktnef/ktnefmessage.h>
#include <ktnef/ktnefdefs.h>

#include <libkcal/incidencebase.h>
#include <libkcal/attendee.h>
#include <libkcal/freebusy.h>
#include <libkcal/journal.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/icalformat.h>

#include <kabc/phonenumber.h>
#include <kabc/vcardconverter.h>

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
#include <qbuffer.h>
#include <qregexp.h>

#include <mimelib/enum.h>

#include <stdlib.h>
#include <time.h>
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
  watcher->addDir( locateLocal( "data", "korganizer/income.tentative/" ) );
  watcher->addDir( locateLocal( "data", "korganizer/income.cancel/" ) );
  watcher->addDir( locateLocal( "data", "korganizer/income.reply/" ) );
  connect( watcher, SIGNAL( dirty( const QString& ) ),
           this, SLOT( incomingDirChanged( const QString& ) ) );
  // Now set the ball rolling
  incomingDirChanged( locateLocal( "data", "korganizer/income.accepted/" ) );
  incomingDirChanged( locateLocal( "data", "korganizer/income.tentative/" ) );
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
  } else if ( action.startsWith( "tentative" ) ) {
    // Find myself and set to answered and tentative
    KCal::Attendee::List attendees = incidence->attendees();
    KCal::Attendee::List::ConstIterator it;
    for ( it = attendees.begin(); it != attendees.end(); ++it ) {
      if( (*it)->email() == receiver ) {
        (*it)->setStatus( KCal::Attendee::Tentative );
        (*it)->setRSVP(false);
        break;
      }
    }
    scheduler.acceptTransaction( incidence, method, status );
  } else if ( action.startsWith( "cancel" ) )
    // TODO: Could this be done like the others?
    mCalendar->deleteIncidence( incidence );
  else if ( action.startsWith( "reply" ) )
    scheduler.acceptTransaction( incidence, method, status );
  else
    kdError(5850) << "Unknown incoming action " << action << endl;
  mView->updateView();
}

static void vPartMicroParser( const QString& str, QString& s )
{
  QString line;
  uint len = str.length();

  for( uint i=0; i<len; ++i) {
    if( str[i] == '\r' || str[i] == '\n' ) {
      if( str[i] == '\r' )
        ++i;
      if( i+1 < len && str[i+1] == ' ' ) {
        // Found a continuation line, skip it's leading blanc
        ++i;
      } else {
        // Found a logical line end, process the line
        if( line.startsWith( s ) ) {
          s = line.mid( s.length() + 1 );
          return;
        }
        line = "";
      }
    } else {
      line += str[i];
    }
  }
  s.truncate(0);
}

static void string2HTML( QString& str ) {
  str.replace( QChar( '&' ), "&amp;" );
  str.replace( QChar( '<' ), "&lt;" );
  str.replace( QChar( '>' ), "&gt;" );
  str.replace( QChar( '\"' ), "&quot;" );
  str.replace( "\\n", "<br>" );
  str.replace( "\\,", "," );
}

static QString meetingDetails( Incidence* incidence, Event* event )
{
  // Meeting details are formatted into an HTML table

  QString html;

  QString sSummary = i18n( "Summary unspecified" );
  if ( incidence ) {
    if ( ! incidence->summary().isEmpty() ) {
      sSummary = incidence->summary();
      string2HTML( sSummary );
    }
  }

  QString sLocation = i18n( "Location unspecified" );
  if ( incidence ) {
    if ( ! incidence->location().isEmpty() ) {
      sLocation = incidence->location();
      string2HTML( sLocation );
    }
  }

  QString dir = ( QApplication::reverseLayout() ? "rtl" : "ltr" );
  html = QString("<div dir=\"%1\">\n").arg(dir);

  html += "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n";

  // Meeting Summary Row
  html += "<tr>";
  html += "<td>" + i18n( "What:" ) + "</td>";
  html += "<td>" + sSummary + "</td>";
  html += "</tr>\n";

  // Meeting Location Row
  html += "<tr>";
  html += "<td>" + i18n( "Where:" ) + "</td>";
  html += "<td>" + sLocation + "</td>";
  html += "</tr>\n";

  // Meeting Start Time Row
  html += "<tr>";
  html += "<td>" + i18n( "Start Time:" ) + "</td>";
  html += "<td>";
  if ( ! event->doesFloat() ) {
    html +=  i18n("%1: Start Date, %2: Start Time", "%1 %2")
             .arg( event->dtStartDateStr(), event->dtStartTimeStr() );
  } else {
    html += i18n("%1: Start Date", "%1 (time unspecified)")
            .arg( event->dtStartDateStr() );
  }
  html += "</td>";
  html += "</tr>\n";

  // Meeting End Time Row
  html += "<tr>";
  html += "<td>" + i18n( "End Time:" ) + "</td>";
  html += "<td>";
  if ( event->hasEndDate() ) {
    if ( ! event->doesFloat() ) {
      html +=  i18n("%1: End Date, %2: End Time", "%1 %2")
               .arg( event->dtEndDateStr(), event->dtEndTimeStr() );
    } else {
      html += i18n("%1: End Date", "%1 (time unspecified)")
              .arg( event->dtEndDateStr() );
    }
  } else {
    html += i18n( "Unspecified" );
  }
  html += "</td>";
  html += "</tr>\n";

  // Meeting Duration Row
  if ( !event->doesFloat() && event->hasEndDate() ) {
    html += "<tr>";
    QTime sDuration, t;
    int secs = event->dtStart().secsTo( event->dtEnd() );
    t = sDuration.addSecs( secs );
    html += "<td>" + i18n( "Duration:" ) + "</td>";
    html += "<td>";
    if ( t.hour() > 0 ) {
      html += i18n( "1 hour ", "%n hours ", t.hour() );
    }
    if ( t.minute() > 0 ) {
      html += i18n( "1 minute ", "%n minutes ",  t.minute() );
    }
    html += "</td>";
    html += "</tr>\n";
  }

  html += "</table>\n";
  html += "</div>\n";

  return html;
}

static QString taskDetails( Incidence* incidence )
{
  // Task details are formatted into an HTML table

  QString html;

  QString sSummary = i18n( "Summary unspecified" );
  QString sDescr = i18n( "Description unspecified" );
  if ( incidence ) {
    if ( ! incidence->summary().isEmpty() ) {
      sSummary = incidence->summary();
    }
    if ( ! incidence->description().isEmpty() ) {
      sDescr = incidence->description();
    }
  }
  html = "<table border=\"0\" cellpadding=\"1\" cellspacing=\"1\">\n";

  // Task Summary Row
  html += "<tr>";
  html += "<td>" + i18n( "Summary:" ) + "</td>";
  html += "<td>" + sSummary + "</td>";
  html += "</tr>\n";

  // Task Description Row
  html += "<tr>";
  html += "<td>" + i18n( "Description:" ) + "</td>";
  html += "<td>" + sDescr + "</td>";
  html += "</tr>\n";

  html += "</table>\n";

  return html;
}

QString KOGroupware::formatICal( const QString& iCal )
{
  KCal::CalendarLocal cl( mCalendar->timeZoneId() );
  ICalFormat format;
  format.fromString( &cl, iCal );

  // Make a shallow copy of the event and task lists
  if( cl.events().count() == 0 && cl.todos().count() == 0 ) {
    kdDebug(5850) << "No iCal in this one\n";
    return QString();
  }

  // Parse the first event out of the vcal
  // TODO: Is it legal to have several events/todos per mail part?
  Incidence* incidence = 0;
  Event* event = 0;
  Todo* todo = 0;
  if( cl.events().count() > 0 )
    incidence = event = cl.events().first();
  else
    incidence = todo = cl.todos().first();

  // TODO: Actually the scheduler needs to do this:
  QString sMethod; // = incidence->method();
  // TODO: This is a temporary workaround to get the method
  sMethod = "METHOD";
  vPartMicroParser( iCal, sMethod );
  sMethod = sMethod.lower();

  // First make the text of the message
  QString html;
  if( sMethod == "request" ) {
    if( event ) {
      html = i18n( "<h2>You have been invited to this meeting</h2>" );
      html += meetingDetails( incidence, event );
    } else {
      html = i18n( "<h2>You have been assigned this task</h2>" );
      html += taskDetails( incidence );
    }
  } else if( sMethod == "reply" ) {
    Attendee::List attendees = incidence->attendees();
    if( attendees.count() == 0 ) {
      kdDebug(5850) << "No attendees in the iCal reply!\n";
      return QString();
    }
    if( attendees.count() != 1 )
      kdDebug(5850) << "Warning: attendeecount in the reply should be 1 "
                    << "but is " << attendees.count() << endl;
    Attendee* attendee = *attendees.begin();

    switch( attendee->status() ) {
    case Attendee::Accepted:
      if( event ) {
        html = i18n( "<h2>Sender accepts this meeting invitation</h2>" );
        html += meetingDetails( incidence, event );
      } else {
        html = i18n( "<h2>Sender accepts this task</h2>" );
        html += taskDetails( incidence );
      }
      break;

    case Attendee::Tentative:
      if( event ) {
        html = i18n( "<h2>Sender tentatively accepts this "
                     "meeting invitation</h2>" );
        html += meetingDetails( incidence, event );
      } else {
        html = i18n( "<h2>Sender tentatively accepts this task</h2>" );
        html += taskDetails( incidence );
      }
      break;

    case Attendee::Declined:
      if( event ) {
        html = i18n( "<h2>Sender declines this meeting invitation</h2>" );
        html += meetingDetails( incidence, event );
      } else {
        html = i18n( "<h2>Sender declines this task</h2>" );
        html += taskDetails( incidence );
      }
      break;

    default:
      if( event ) {
        html = i18n( "<h2>Unknown response to this meeting invitation</h2>" );
        html += meetingDetails( incidence, event );
      } else {
        html = i18n( "<h2>Unknown response to this task</h2>" );
        html += taskDetails( incidence );
      }
    }
  } else if( sMethod == "cancel" ) {
    if( event ) {
      html = i18n( "<h2>This meeting has been canceled</h2>" );
      html += meetingDetails( incidence, event );
    } else {
      html = i18n( "<h2>This task was canceled</h2>" );
      html += taskDetails( incidence );
    }
  }

  // Add the groupware URLs
  html += "<br>&nbsp;<br>&nbsp;<br>";
  html += "<table border=\"0\" cellspacing=\"0\"><tr><td>&nbsp;</td><td>";
  if( sMethod == "request" || sMethod == "update" ) {
    // Accept
    html += "<a href=\"kmail:groupware_request_accept\"><b>";
    html += i18n( "[Accept]" );
    html += "</b></a></td><td> &nbsp; </td><td>";
    // Accept conditionally
    html += "<a href=\"kmail:groupware_request_accept conditionally\"><b>";
    html += i18n( "Accept conditionally", "[Accept cond.]" );
    html += "</b></a></td><td> &nbsp; </td><td>";
    // Decline
    html += "<a href=\"kmail:groupware_request_decline\"><b>";
    html += i18n( "[Decline]" );
    if( event ) {
      // Check my calendar...
      html += "</b></a></td><td> &nbsp; </td><td>";
      html += "<a href=\"kmail:groupware_request_check\"><b>";
      html += i18n("[Check my calendar...]" );
    }
  } else if( sMethod == "reply" ) {
    // Enter this into my calendar
    html += "<a href=\"kmail:groupware_reply#%1\"><b>";
    if( event )
      html += i18n( "[Enter this into my calendar]" );
    else
      html += i18n( "[Enter this into my task list]" );
  } else if( sMethod == "cancel" ) {
    // Cancel event from my calendar
    html += "<a href=\"kmail:groupware_cancel\"><b>";
    html += i18n( "[Remove this from my calendar]" );
  }
  html += "</b></a></td></tr></table>";

  QString sDescr = incidence->description();
  if( ( sMethod == "request" || sMethod == "cancel" ) && !sDescr.isEmpty() ) {
    string2HTML( sDescr );
    html += "<br>&nbsp;<br>&nbsp;<br><u>" + i18n("Description:")
      + "</u><br><table border=\"0\"><tr><td>&nbsp;</td><td>";
    html += sDescr + "</td></tr></table>";
  }
  html += "&nbsp;<br>&nbsp;<br><u>" + i18n( "Original message:" ) + "</u>";

  return html;
}

QString KOGroupware::formatTNEF( const QByteArray& tnef )
{
  QString vPart = msTNEFToVPart( tnef );
  QString iCal = formatICal( vPart );
  if( !iCal.isEmpty() )
    return iCal;
  return vPart;
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
  Event* event = mCalendar->event( incidence->uid() );
  if( !event )
    return false;

  mCalendar->deleteEvent( event );
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
                                   Incidence* incidence, bool isDeleting,
                                   bool statusChanged )
{
  bool isOrganizer = KOPrefs::instance()->thatIsMe( incidence->organizer() );

  int rc = 0;
  // Figure out if there are other people involved in this incidence
  bool otherPeople = false;
  Attendee::List attendees = incidence->attendees();
  Attendee *me = 0;
  Attendee::List::ConstIterator it;
  for ( it = attendees.begin(); it != attendees.end(); ++it ) {
    // Don't send email to ourselves
    if ( !KOPrefs::instance()->thatIsMe( (*it)->email() ) ) {
      otherPeople = true;
    } else {
      me = (*it);
    }
  }

  if( !otherPeople && isOrganizer )
    return true;

  if ( isOrganizer ) {
    QString type;
    if( incidence->type() == "Event") type = i18n("event");
    else if( incidence->type() == "Todo" ) type = i18n("task");
    else if( incidence->type() == "Journal" ) type = i18n("journal entry");
    else type = incidence->type();
    QString txt = i18n( "This %1 includes other people. "
                        "Should email be sent out to the attendees?" )
      .arg( type );
    rc = KMessageBox::questionYesNoCancel( parent, txt,
                                           i18n("Group scheduling email") );
  } else if( incidence->type() == "Todo" ) {
    if( method == Scheduler::Request )
      // This is an update to be sent to the organizer
      method = Scheduler::Reply;

    // Ask if the user wants to tell the organizer about the current status
    QString txt = i18n( "Do you want to send a status update to the "
                        "organizer of this task?");
    rc = KMessageBox::questionYesNo( parent, txt );
  } else if( incidence->type() == "Event" ) {
    QString txt;
    if ( statusChanged && method == Scheduler::Request ) {
      txt = i18n( "Your status as an attendee of this event "
          "changed. Do you want to send a status update to the "
          "organizer of this event?" );
      method = Scheduler::Reply;
      rc = KMessageBox::questionYesNo( parent, txt );
    } else {
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
    }
  } else {
    kdWarning(5850) << "Groupware messages for Journals are not implemented yet!" << endl;
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


//-----------------------------------------------------------------------------

static QString stringProp( KTNEFMessage* tnefMsg, const Q_UINT32& key,
                           const QString& fallback = QString::null)
{
  return tnefMsg->findProp( key < 0x10000 ? key & 0xFFFF : key >> 16,
                            fallback );
}

static QString sNamedProp( KTNEFMessage* tnefMsg, const QString& name,
                           const QString& fallback = QString::null )
{
  return tnefMsg->findNamedProp( name, fallback );
}

struct save_tz { char* old_tz; char* tz_env_str; };

/* temporarily go to a different timezone */
static struct save_tz set_tz( const char* _tc )
{
  const char *tc = _tc?_tc:"UTC";

  struct save_tz rv;

  rv.old_tz = 0;
  rv.tz_env_str = 0;

  //kdDebug(5006) << "set_tz(), timezone before = " << timezone << endl;

  char* tz_env = 0;
  if( getenv( "TZ" ) ) {
    tz_env = strdup( getenv( "TZ" ) );
    rv.old_tz = tz_env;
  }
  char* tmp_env = (char*)malloc( strlen( tc ) + 4 );
  strcpy( tmp_env, "TZ=" );
  strcpy( tmp_env+3, tc );
  putenv( tmp_env );

  rv.tz_env_str = tmp_env;

  /* tmp_env is not free'ed -- it is part of the environment */

  tzset();
  //kdDebug(5006) << "set_tz(), timezone after = " << timezone << endl;

  return rv;
}

/* restore previous timezone */
static void unset_tz( struct save_tz old_tz )
{
  if( old_tz.old_tz ) {
    char* tmp_env = (char*)malloc( strlen( old_tz.old_tz ) + 4 );
    strcpy( tmp_env, "TZ=" );
    strcpy( tmp_env+3, old_tz.old_tz );
    putenv( tmp_env );
    /* tmp_env is not free'ed -- it is part of the environment */
    free( old_tz.old_tz );
  } else {
    /* clear TZ from env */
    putenv( strdup("TZ") );
  }
  tzset();

  /* is this OK? */
  if( old_tz.tz_env_str ) free( old_tz.tz_env_str );
}

static QDateTime utc2Local( const QDateTime& utcdt )
{
  struct tm tmL;

  save_tz tmp_tz = set_tz("UTC");
  time_t utc = utcdt.toTime_t();
  unset_tz( tmp_tz );

  localtime_r( &utc, &tmL );
  return QDateTime( QDate( tmL.tm_year+1900, tmL.tm_mon+1, tmL.tm_mday ),
                    QTime( tmL.tm_hour, tmL.tm_min, tmL.tm_sec ) );
}

static QDateTime pureISOToLocalQDateTime( const QString& dtStr,
                                          bool bDateOnly = false )
{
  QDate tmpDate;
  QTime tmpTime;
  int year, month, day, hour, minute, second;

  if( bDateOnly ) {
    year = dtStr.left( 4 ).toInt();
    month = dtStr.mid( 4, 2 ).toInt();
    day = dtStr.mid( 6, 2 ).toInt();
    hour = 0;
    minute = 0;
    second = 0;
  } else {
    year = dtStr.left( 4 ).toInt();
    month = dtStr.mid( 4, 2 ).toInt();
    day = dtStr.mid( 6, 2 ).toInt();
    hour = dtStr.mid( 9, 2 ).toInt();
    minute = dtStr.mid( 11, 2 ).toInt();
    second = dtStr.mid( 13, 2 ).toInt();
  }
  tmpDate.setYMD( year, month, day );
  tmpTime.setHMS( hour, minute, second );

  if( tmpDate.isValid() && tmpTime.isValid() ) {
    QDateTime dT = QDateTime( tmpDate, tmpTime );

    if( !bDateOnly ) {
      // correct for GMT ( == Zulu time == UTC )
      if (dtStr.at(dtStr.length()-1) == 'Z') {
        //dT = dT.addSecs( 60 * KRFCDate::localUTCOffset() );
        //localUTCOffset( dT ) );
        dT = utc2Local( dT );
      }
    }
    return dT;
  } else
    return QDateTime();
}

QString KOGroupware::msTNEFToVPart( const QByteArray& tnef )
{
  bool bOk = false;

  KTNEFParser parser;
  QBuffer buf( tnef );
  CalendarLocal cal;
  KABC::Addressee addressee;
  KABC::VCardConverter cardConv;
  ICalFormat calFormat;
  Event* event = new Event();

  if( parser.openDevice( &buf ) ) {
    KTNEFMessage* tnefMsg = parser.message();
    //QMap<int,KTNEFProperty*> props = parser.message()->properties();

    // Everything depends from property PR_MESSAGE_CLASS
    // (this is added by KTNEFParser):
    QString msgClass = tnefMsg->findProp( 0x001A, QString::null, true )
      .upper();
    if( !msgClass.isEmpty() ) {
      // Match the old class names that might be used by Outlook for
      // compatibility with Microsoft Mail for Windows for Workgroups 3.1.
      bool bCompatClassAppointment = false;
      bool bCompatMethodRequest = false;
      bool bCompatMethodCancled = false;
      bool bCompatMethodAccepted = false;
      bool bCompatMethodAcceptedCond = false;
      bool bCompatMethodDeclined = false;
      if( msgClass.startsWith( "IPM.MICROSOFT SCHEDULE." ) ) {
        bCompatClassAppointment = true;
        if( msgClass.endsWith( ".MTGREQ" ) )
          bCompatMethodRequest = true;
        if( msgClass.endsWith( ".MTGCNCL" ) )
          bCompatMethodCancled = true;
        if( msgClass.endsWith( ".MTGRESPP" ) )
          bCompatMethodAccepted = true;
        if( msgClass.endsWith( ".MTGRESPA" ) )
          bCompatMethodAcceptedCond = true;
        if( msgClass.endsWith( ".MTGRESPN" ) )
          bCompatMethodDeclined = true;
      }
      bool bCompatClassNote = ( msgClass == "IPM.MICROSOFT MAIL.NOTE" );

      if( bCompatClassAppointment || "IPM.APPOINTMENT" == msgClass ) {
        // Compose a vCal
        bool bIsReply = false;
        QString prodID = "-//Microsoft Corporation//Outlook ";
        prodID += tnefMsg->findNamedProp( "0x8554", "9.0" );
        prodID += "MIMEDIR/EN\n";
        prodID += "VERSION:2.0\n";
        calFormat.setApplication( "Outlook", prodID );

        Scheduler::Method method;
        if( bCompatMethodRequest )
          method = Scheduler::Request;
        else if( bCompatMethodCancled )
          method = Scheduler::Cancel;
        else if( bCompatMethodAccepted || bCompatMethodAcceptedCond ||
                 bCompatMethodDeclined ) {
          method = Scheduler::Reply;
          bIsReply = true;
        } else {
          // pending(khz): verify whether "0x0c17" is the right tag ???
          //
          // at the moment we think there are REQUESTS and UPDATES
          //
          // but WHAT ABOUT REPLIES ???
          //
          //

          if( tnefMsg->findProp(0x0c17) == "1" )
            bIsReply = true;
          method = Scheduler::Request;
        }

        /// ###  FIXME Need to get this attribute written
        ScheduleMessage schedMsg(event, method, ScheduleMessage::Unknown );

        QString sSenderSearchKeyEmail( tnefMsg->findProp( 0x0C1D ) );

        if( !sSenderSearchKeyEmail.isEmpty() ) {
          int colon = sSenderSearchKeyEmail.find( ':' );
          // May be e.g. "SMTP:KHZ@KDE.ORG"
          if( sSenderSearchKeyEmail.find( ':' ) == -1 )
            sSenderSearchKeyEmail.remove( 0, colon+1 );
        }

        QString s( tnefMsg->findProp( 0x0e04 ) );
        QStringList attendees = QStringList::split( ';', s );
        if( attendees.count() ) {
          for( QStringList::Iterator it = attendees.begin();
               it != attendees.end(); ++it ) {
            // Skip all entries that have no '@' since these are
            // no mail addresses
            if( (*it).find('@') == -1 ) {
              s = (*it).stripWhiteSpace();

              Attendee *attendee = new Attendee( s, s, true );
              if( bIsReply ) {
                if( bCompatMethodAccepted )
                  attendee->setStatus( Attendee::Accepted );
                if( bCompatMethodDeclined )
                  attendee->setStatus( Attendee::Declined );
                if( bCompatMethodAcceptedCond )
                  attendee->setStatus(Attendee::Tentative);
              } else {
                attendee->setStatus( Attendee::NeedsAction );
                attendee->setRole( Attendee::ReqParticipant );
              }
              event->addAttendee(attendee);
            }
          }
        } else {
          // Oops, no attendees?
          // This must be old style, let us use the PR_SENDER_SEARCH_KEY.
          s = sSenderSearchKeyEmail;
          if( !s.isEmpty() ) {
            Attendee *attendee = new Attendee( QString::null, QString::null,
                                               true );
            if( bIsReply ) {
              if( bCompatMethodAccepted )
                attendee->setStatus( Attendee::Accepted );
              if( bCompatMethodAcceptedCond )
                attendee->setStatus( Attendee::Declined );
              if( bCompatMethodDeclined )
                attendee->setStatus( Attendee::Tentative );
            } else {
              attendee->setStatus(Attendee::NeedsAction);
              attendee->setRole(Attendee::ReqParticipant);
            }
            event->addAttendee(attendee);
          }
        }
        s = tnefMsg->findProp( 0x0c1f ); // look for organizer property
        if( s.isEmpty() && !bIsReply )
          s = sSenderSearchKeyEmail;
        if( !s.isEmpty() )
          event->setOrganizer( s );

        s = tnefMsg->findProp( 0x8516 ).replace( QChar( '-' ), QString::null )
          .replace( QChar( ':' ), QString::null );
        event->setDtStart( QDateTime::fromString( s ) ); // ## Format??

        s = tnefMsg->findProp( 0x8517 ).replace( QChar( '-' ), QString::null )
          .replace( QChar( ':' ), QString::null );
        event->setDtEnd( QDateTime::fromString( s ) );

        s = tnefMsg->findProp( 0x8208 );
        event->setLocation( s );

        // is it OK to set this to OPAQUE always ??
        //vPart += "TRANSP:OPAQUE\n"; ###FIXME, portme!
        //vPart += "SEQUENCE:0\n";

        // is "0x0023" OK  -  or should we look for "0x0003" ??
        s = tnefMsg->findProp( 0x0023 );
        event->setUid( s );

        // PENDING(khz): is this value in local timezone? Must it be
        // adjusted? Most likely this is a bug in the server or in
        // Outlook - we ignore it for now.
        s = tnefMsg->findProp( 0x8202 ).replace( QChar( '-' ), QString::null )
          .replace( QChar( ':' ), QString::null );
        // ### libkcal always uses currentDateTime()
        // event->setDtStamp(QDateTime::fromString(s));

        s = tnefMsg->findNamedProp( "Keywords" );
        event->setCategories( s );

        s = tnefMsg->findProp( 0x1000 );
        event->setDescription( s );

        s = tnefMsg->findProp( 0x0070 );
        event->setSummary( s );

        s = tnefMsg->findProp( 0x0026 );
        event->setPriority( s.toInt() );

        // is reminder flag set ?
        if(!tnefMsg->findProp(0x8503).isEmpty()) {
          Alarm *alarm = new Alarm(event);
          QDateTime highNoonTime =
            pureISOToLocalQDateTime( tnefMsg->findProp( 0x8502 )
                                     .replace( QChar( '-' ), "" )
                                     .replace( QChar( ':' ), "" ) );
          QDateTime wakeMeUpTime =
            pureISOToLocalQDateTime( tnefMsg->findProp( 0x8560, "" )
                                     .replace( QChar( '-' ), "" )
                                     .replace( QChar( ':' ), "" ) );
          alarm->setTime(wakeMeUpTime);

          if( highNoonTime.isValid() && wakeMeUpTime.isValid() )
            alarm->setStartOffset( Duration( highNoonTime, wakeMeUpTime ) );
          else
            // default: wake them up 15 minutes before the appointment
            alarm->setStartOffset( Duration( 15*60 ) );
          alarm->setDisplayAlarm( i18n( "Reminder" ) );

          // Sorry: the different action types are not known (yet)
          //        so we always set 'DISPLAY' (no sounds, no images...)
          event->addAlarm( alarm );
        }
        cal.addEvent( event );
        bOk = true;
        // we finished composing a vCal
      } else if( bCompatClassNote || "IPM.CONTACT" == msgClass ) {
        addressee.setUid( stringProp( tnefMsg, attMSGID ) );
        addressee.setFormattedName( stringProp( tnefMsg, MAPI_TAG_PR_DISPLAY_NAME ) );
        addressee.insertEmail( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_EMAIL1EMAILADDRESS ), true );
        addressee.insertEmail( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_EMAIL2EMAILADDRESS ), false );
        addressee.insertEmail( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_EMAIL3EMAILADDRESS ), false );
        addressee.insertCustom( "KADDRESSBOOK", "X-IMAddress", sNamedProp( tnefMsg, MAPI_TAG_CONTACT_IMADDRESS ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-SpousesName", stringProp( tnefMsg, MAPI_TAG_PR_SPOUSE_NAME ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-ManagersName", stringProp( tnefMsg, MAPI_TAG_PR_MANAGER_NAME ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-AssistantsName", stringProp( tnefMsg, MAPI_TAG_PR_ASSISTANT ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-Department", stringProp( tnefMsg, MAPI_TAG_PR_DEPARTMENT_NAME ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-Office", stringProp( tnefMsg, MAPI_TAG_PR_OFFICE_LOCATION ) );
        addressee.insertCustom( "KADDRESSBOOK", "X-Profession", stringProp( tnefMsg, MAPI_TAG_PR_PROFESSION ) );

        QString s = tnefMsg->findProp( MAPI_TAG_PR_WEDDING_ANNIVERSARY )
          .replace( QChar( '-' ), QString::null )
          .replace( QChar( ':' ), QString::null );
        if( !s.isEmpty() )
          addressee.insertCustom( "KADDRESSBOOK", "X-Anniversary", s );

        addressee.setUrl( KURL( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_WEBPAGE )  ) );

        // collect parts of Name entry
        addressee.setFamilyName( stringProp( tnefMsg, MAPI_TAG_PR_SURNAME ) );
        addressee.setGivenName( stringProp( tnefMsg, MAPI_TAG_PR_GIVEN_NAME ) );
        addressee.setAdditionalName( stringProp( tnefMsg, MAPI_TAG_PR_MIDDLE_NAME ) );
        addressee.setPrefix( stringProp( tnefMsg, MAPI_TAG_PR_DISPLAY_NAME_PREFIX ) );
        addressee.setSuffix( stringProp( tnefMsg, MAPI_TAG_PR_GENERATION ) );

        addressee.setNickName( stringProp( tnefMsg, MAPI_TAG_PR_NICKNAME ) );
        addressee.setRole( stringProp( tnefMsg, MAPI_TAG_PR_TITLE ) );
        addressee.setOrganization( stringProp( tnefMsg, MAPI_TAG_PR_COMPANY_NAME ) );
        /*
        the MAPI property ID of this (multiline) )field is unknown:
        vPart += stringProp(tnefMsg, "\n","NOTE", ... , "" );
        */

        KABC::Address adr;
        adr.setPostOfficeBox( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_PO_BOX ) );
        adr.setStreet( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_STREET ) );
        adr.setLocality( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_CITY ) );
        adr.setRegion( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_STATE_OR_PROVINCE ) );
        adr.setPostalCode( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_POSTAL_CODE ) );
        adr.setCountry( stringProp( tnefMsg, MAPI_TAG_PR_HOME_ADDRESS_COUNTRY ) );
        adr.setType(KABC::Address::Home);
        addressee.insertAddress(adr);

        adr.setPostOfficeBox( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSPOBOX ) );
        adr.setStreet( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSSTREET ) );
        adr.setLocality( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSCITY ) );
        adr.setRegion( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSSTATE ) );
        adr.setPostalCode( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSPOSTALCODE ) );
        adr.setCountry( sNamedProp( tnefMsg, MAPI_TAG_CONTACT_BUSINESSADDRESSCOUNTRY ) );
        adr.setType( KABC::Address::Work );
        addressee.insertAddress( adr );

        adr.setPostOfficeBox( stringProp( tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_PO_BOX ) );
        adr.setStreet( stringProp(tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_STREET ) );
        adr.setLocality( stringProp(tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_CITY ) );
        adr.setRegion( stringProp(tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_STATE_OR_PROVINCE ) );
        adr.setPostalCode( stringProp(tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_POSTAL_CODE ) );
        adr.setCountry( stringProp(tnefMsg, MAPI_TAG_PR_OTHER_ADDRESS_COUNTRY ) );
        adr.setType( KABC::Address::Dom );
        addressee.insertAddress(adr);

        // problem: the 'other' address was stored by KOrganizer in
        //          a line looking like the following one:
        // vPart += "\nADR;TYPE=dom;TYPE=intl;TYPE=parcel;TYPE=postal;TYPE=work;TYPE=home:other_pobox;;other_str1\nother_str2;other_loc;other_region;other_pocode;other_country

        QString nr;
        nr = stringProp( tnefMsg, MAPI_TAG_PR_HOME_TELEPHONE_NUMBER );
        addressee.insertPhoneNumber( KABC::PhoneNumber( nr, KABC::PhoneNumber::Home ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_BUSINESS_TELEPHONE_NUMBER );
        addressee.insertPhoneNumber( KABC::PhoneNumber( nr, KABC::PhoneNumber::Work ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_MOBILE_TELEPHONE_NUMBER );
        addressee.insertPhoneNumber( KABC::PhoneNumber( nr, KABC::PhoneNumber::Cell ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_HOME_FAX_NUMBER );
        addressee.insertPhoneNumber( KABC::PhoneNumber( nr, KABC::PhoneNumber::Fax | KABC::PhoneNumber::Home ) );
        nr = stringProp( tnefMsg, MAPI_TAG_PR_BUSINESS_FAX_NUMBER );
        addressee.insertPhoneNumber( KABC::PhoneNumber( nr, KABC::PhoneNumber::Fax | KABC::PhoneNumber::Work ) );

        s = tnefMsg->findProp( MAPI_TAG_PR_BIRTHDAY )
          .replace( QChar( '-' ), QString::null )
          .replace( QChar( ':' ), QString::null );
        if( !s.isEmpty() )
          addressee.setBirthday( QDateTime::fromString( s ) );

        bOk = ( !addressee.isEmpty() );
      } else if( "IPM.NOTE" == msgClass ) {

      } // else if ... and so on ...
    }
  }

  // Compose return string
  QString iCal = calFormat.toString( &cal );
  if( !iCal.isEmpty() )
    // This was an iCal
    return iCal;

  // Not an iCal - try a vCard
  KABC::VCardConverter converter;
  return converter.createVCard( addressee );
}


#include "kogroupware.moc"
