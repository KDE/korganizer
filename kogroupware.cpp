/*
  This file is part of the Groupware/KOrganizer integration.

  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.trolltech.com and http://www.kde.org respectively

  Copyright (c) 2002-2004 KlarÃ¤lvdalens Datakonsult AB
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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA  02110-1301, USA.

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
#include "koprefs.h"
#include "koincidenceeditor.h"
#include <libemailfunctions/email.h>
#include <libkcal/attendee.h>
#include <libkcal/journal.h>
#include <libkcal/incidenceformatter.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdirwatch.h>
#include <qfile.h>
#include <qregexp.h>
#include <qdir.h>
#include <qtimer.h>

FreeBusyManager *KOGroupware::mFreeBusyManager = 0;

KOGroupware *KOGroupware::mInstance = 0;

KOGroupware *KOGroupware::create( CalendarView *view,
                                  KCal::CalendarResources *calendar )
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


 KOGroupware::KOGroupware( CalendarView* view, KCal::CalendarResources* cal )
   : QObject( 0, "kmgroupware_instance" ), mView( view ), mCalendar( cal )
{
  // Set up the dir watch of the three incoming dirs
  KDirWatch* watcher = KDirWatch::self();
  watcher->addDir( locateLocal( "data", "korganizer/income.accepted/" ) );
  watcher->addDir( locateLocal( "data", "korganizer/income.tentative/" ) );
  watcher->addDir( locateLocal( "data", "korganizer/income.counter/" ) );
  watcher->addDir( locateLocal( "data", "korganizer/income.cancel/" ) );
  watcher->addDir( locateLocal( "data", "korganizer/income.reply/" ) );
  watcher->addDir( locateLocal( "data", "korganizer/income.delegated/" ) );
  connect( watcher, SIGNAL( dirty( const QString& ) ),
           this, SLOT( incomingDirChanged( const QString& ) ) );
  // Now set the ball rolling
  QTimer::singleShot( 0, this, SLOT(initialCheckForChanges()) );
}

void KOGroupware::initialCheckForChanges()
{
  incomingDirChanged( locateLocal( "data", "korganizer/income.accepted/" ) );
  incomingDirChanged( locateLocal( "data", "korganizer/income.tentative/" ) );
  incomingDirChanged( locateLocal( "data", "korganizer/income.counter/" ) );
  incomingDirChanged( locateLocal( "data", "korganizer/income.cancel/" ) );
  incomingDirChanged( locateLocal( "data", "korganizer/income.reply/" ) );
  incomingDirChanged( locateLocal( "data", "korganizer/income.delegated/" ) );
}

void KOGroupware::slotViewNewIncidenceChanger( IncidenceChangerBase* changer )
{
    // Call slot perhapsUploadFB if an incidence was added, changed or removed
    connect( changer, SIGNAL( incidenceAdded( Incidence* ) ),
             mFreeBusyManager, SLOT( slotPerhapsUploadFB() ) );
    connect( changer, SIGNAL( incidenceChanged( Incidence*, Incidence*, int ) ),
             mFreeBusyManager, SLOT( slotPerhapsUploadFB() ) );
    connect( changer, SIGNAL( incidenceChanged( Incidence*, Incidence* ) ),
             mFreeBusyManager, SLOT( slotPerhapsUploadFB() ) ) ;
    connect( changer, SIGNAL( incidenceDeleted( Incidence * ) ),
             mFreeBusyManager, SLOT( slotPerhapsUploadFB() ) );
}

FreeBusyManager *KOGroupware::freeBusyManager()
{
  if ( !mFreeBusyManager ) {
    mFreeBusyManager = new FreeBusyManager( this, "freebusymanager" );
    mFreeBusyManager->setCalendar( mCalendar );
    connect( mCalendar, SIGNAL( calendarChanged() ),
             mFreeBusyManager, SLOT( slotPerhapsUploadFB() ) );
    connect( mView, SIGNAL( newIncidenceChanger( IncidenceChangerBase* ) ),
             this, SLOT( slotViewNewIncidenceChanger( IncidenceChangerBase* ) ) );
    slotViewNewIncidenceChanger( mView->incidenceChanger() );
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
  const QStringList files = dir.entryList( QDir::Files );
  if ( files.isEmpty() )
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
  QString receiver = KPIM::getFirstEmailAddress( t.readLine() );
  QString iCal = t.read();

  f.remove();

  ScheduleMessage *message = mFormat.parseScheduleMessage( mCalendar, iCal );
  if ( !message ) {
    QString errorMessage;
    if (mFormat.exception())
      errorMessage = i18n( "Error message: %1" ).arg( mFormat.exception()->message() );
    kdDebug(5850) << "MailScheduler::retrieveTransactions() Error parsing "
                  << errorMessage << endl;
    KMessageBox::detailedError( mView,
        i18n("Error while processing an invitation or update."),
        errorMessage );
    return;
  }

  KCal::Scheduler::Method method =
    static_cast<KCal::Scheduler::Method>( message->method() );
  KCal::ScheduleMessage::Status status = message->status();
  KCal::Incidence* incidence =
    dynamic_cast<KCal::Incidence*>( message->event() );
  if(!incidence) {
    delete message;
    return;
  }
  KCal::MailScheduler scheduler( mCalendar );
  if ( action.startsWith( "accepted" ) || action.startsWith( "tentative" )
       || action.startsWith( "delegated" ) || action.startsWith( "counter" ) ) {
    // Find myself and set my status. This can't be done in the scheduler,
    // since this does not know the choice I made in the KMail bpf
    KCal::Attendee::List attendees = incidence->attendees();
    KCal::Attendee::List::ConstIterator it;
    for ( it = attendees.begin(); it != attendees.end(); ++it ) {
      if( (*it)->email() == receiver ) {
        if ( action.startsWith( "accepted" ) )
          (*it)->setStatus( KCal::Attendee::Accepted );
        else if ( action.startsWith( "tentative" ) )
          (*it)->setStatus( KCal::Attendee::Tentative );
        else if ( KOPrefs::instance()->outlookCompatCounterProposals() && action.startsWith( "counter" ) )
          (*it)->setStatus( KCal::Attendee::Tentative );
        else if ( action.startsWith( "delegated" ) )
          (*it)->setStatus( KCal::Attendee::Delegated );
        break;
      }
    }
    if ( KOPrefs::instance()->outlookCompatCounterProposals() || !action.startsWith( "counter" ) )
      scheduler.acceptTransaction( incidence, method, status );
  } else if ( action.startsWith( "cancel" ) )
    // Delete the old incidence, if one is present
    scheduler.acceptTransaction( incidence, KCal::Scheduler::Cancel, status );
  else if ( action.startsWith( "reply" ) ) {
    if ( method != Scheduler::Counter ) {
      scheduler.acceptTransaction( incidence, method, status );
    } else {
      // accept counter proposal
      scheduler.acceptCounterProposal( incidence );
      // send update to all attendees
      sendICalMessage( mView, Scheduler::Request, incidence );
    }
  } else
    kdError(5850) << "Unknown incoming action " << action << endl;

  if ( action.startsWith( "counter" ) ) {
    mView->editIncidence( incidence, true );
    KOIncidenceEditor *tmp = mView->editorDialog( incidence );
    tmp->selectInvitationCounterProposal( true );
  }
  mView->updateView();
}

class KOInvitationFormatterHelper : public InvitationFormatterHelper
{
  public:
    virtual QString generateLinkURL( const QString &id ) { return "kmail:groupware_request_" + id; }
};

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
  // If there are no attendees, don't bother
  if( incidence->attendees().isEmpty() )
    return true;

  bool isOrganizer = KOPrefs::instance()->thatIsMe( incidence->organizer().email() );
  int rc = 0;
  /*
   * There are two scenarios:
   * o "we" are the organizer, where "we" means any of the identities or mail
   *   addresses known to Kontact/PIM. If there are attendees, we need to mail
   *   them all, even if one or more of them are also "us". Otherwise there
   *   would be no way to invite a resource or our boss, other identities we
   *   also manage.
   * o "we: are not the organizer, which means we changed the completion status
   *   of a todo, or we changed our attendee status from, say, tentative to
   *   accepted. In both cases we only mail the organizer. All other changes
   *   bring us out of sync with the organizer, so we won't mail, if the user
   *   insists on applying them.
   */

  if ( isOrganizer ) {
    /* We are the organizer. If there is more than one attendee, or if there is
     * only one, and it's not the same as the organizer, ask the user to send
     * mail. */
    if ( incidence->attendees().count() > 1
        || incidence->attendees().first()->email() != incidence->organizer().email() ) {
      QString type;
      if( incidence->type() == "Event") type = i18n("event");
      else if( incidence->type() == "Todo" ) type = i18n("task");
      else if( incidence->type() == "Journal" ) type = i18n("journal entry");
      else type = incidence->type();
      QString txt = i18n( "This %1 includes other people. "
          "Should email be sent out to the attendees?" )
        .arg( type );
      rc = KMessageBox::questionYesNoCancel( parent, txt,
          i18n("Group Scheduling Email"), i18n("Send Email"), i18n("Do Not Send") );
    } else {
      return true;
    }
  } else if( incidence->type() == "Todo" ) {
    if( method == Scheduler::Request )
      // This is an update to be sent to the organizer
      method = Scheduler::Reply;

    // Ask if the user wants to tell the organizer about the current status
    QString txt = i18n( "Do you want to send a status update to the "
                        "organizer of this task?");
    rc = KMessageBox::questionYesNo( parent, txt, QString::null, i18n("Send Update"), i18n("Do Not Send") );
  } else if( incidence->type() == "Event" ) {
    QString txt;
    if ( statusChanged && method == Scheduler::Request ) {
      txt = i18n( "Your status as an attendee of this event "
          "changed. Do you want to send a status update to the "
          "organizer of this event?" );
      method = Scheduler::Reply;
      rc = KMessageBox::questionYesNo( parent, txt, QString::null, i18n("Send Update"), i18n("Do Not Send") );
    } else {
      if( isDeleting ) {
        const QStringList myEmails = KOPrefs::instance()->allEmails();
        bool askConfirmation = false;
        for ( QStringList::ConstIterator it = myEmails.begin(); it != myEmails.end(); ++it ) {
          QString email = *it;
          Attendee *me = incidence->attendeeByMail(email);
          if (me && (me->status()==KCal::Attendee::Accepted || me->status()==KCal::Attendee::Delegated)) {
            askConfirmation = true;
            break;
          }
        }

        if ( !askConfirmation ) {
          return true;
        }

        txt = i18n( "You are not the organizer of this event, "
            "but you were supposed to attend. Do you really want "
            "to delete it and notify the organizer?" );
      } else {
        txt = i18n( "You are not the organizer of this event. "
            "Editing it will bring your calendar out of sync "
            "with the organizers calendar. Do you really want "
            "to edit it?" );
      }
      rc = KMessageBox::warningYesNo( parent, txt );
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

void KOGroupware::sendCounterProposal(KCal::Calendar *calendar, KCal::Event * oldEvent, KCal::Event * newEvent) const
{
  if ( !oldEvent || !newEvent || *oldEvent == *newEvent || !KOPrefs::instance()->mUseGroupwareCommunication )
    return;
  if ( KOPrefs::instance()->outlookCompatCounterProposals() ) {
    Incidence* tmp = oldEvent->clone();
    tmp->setSummary( i18n("Counter proposal: %1").arg( newEvent->summary() ) );
    tmp->setDescription( newEvent->description() );
    tmp->addComment( i18n("Proposed new meeting time: %1 - %2").arg( newEvent->dtStartStr(), newEvent->dtEndStr() ) );
    KCal::MailScheduler scheduler( calendar );
    scheduler.performTransaction( tmp, Scheduler::Reply );
    delete tmp;
  } else {
    KCal::MailScheduler scheduler( calendar );
    scheduler.performTransaction( newEvent, Scheduler::Counter );
  }
}

#include "kogroupware.moc"
