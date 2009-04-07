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
#include "calendarview.h"
#include "freebusymanager.h"
#include "koprefs.h"
#include "koincidenceeditor.h"
#include "mailscheduler.h"

#include <KCal/CalendarResources>
#include <KCal/IncidenceFormatter>
#include <KPIMUtils/Email>

#include <KDirWatch>
#include <KMessageBox>
#include <KStandardDirs>

#include <QDir>
#include <QFile>
#include <QTimer>

FreeBusyManager *KOGroupware::mFreeBusyManager = 0;

KOGroupware *KOGroupware::mInstance = 0;

KOGroupware *KOGroupware::create( CalendarView *view,
                                  KCal::CalendarResources *calendar )
{
  if ( !mInstance ) {
    mInstance = new KOGroupware( view, calendar );
  }
  return mInstance;
}

KOGroupware *KOGroupware::instance()
{
  // Doesn't create, that is the task of create()
  Q_ASSERT( mInstance );
  return mInstance;
}

KOGroupware::KOGroupware( CalendarView *view, KCal::CalendarResources *cal )
  : QObject( 0 ), mView( view ), mCalendar( cal )
{
  setObjectName( "kmgroupware_instance" );
  // Set up the dir watch of the three incoming dirs
  KDirWatch *watcher = KDirWatch::self();
  watcher->addDir( KStandardDirs::locateLocal( "data", "korganizer/income.accepted/" ) );
  watcher->addDir( KStandardDirs::locateLocal( "data", "korganizer/income.tentative/" ) );
  watcher->addDir( KStandardDirs::locateLocal( "data", "korganizer/income.counter/" ) );
  watcher->addDir( KStandardDirs::locateLocal( "data", "korganizer/income.cancel/" ) );
  watcher->addDir( KStandardDirs::locateLocal( "data", "korganizer/income.reply/" ) );
  watcher->addDir( KStandardDirs::locateLocal( "data", "korganizer/income.delegated/" ) );
  connect( watcher, SIGNAL(dirty(const QString&)),
           this, SLOT(incomingDirChanged(const QString&)) );
  // Now set the ball rolling
  QTimer::singleShot( 0, this, SLOT(initialCheckForChanges()) );
}

void KOGroupware::initialCheckForChanges()
{
  incomingDirChanged( KStandardDirs::locateLocal( "data", "korganizer/income.accepted/" ) );
  incomingDirChanged( KStandardDirs::locateLocal( "data", "korganizer/income.tentative/" ) );
  incomingDirChanged( KStandardDirs::locateLocal( "data", "korganizer/income.counter/" ) );
  incomingDirChanged( KStandardDirs::locateLocal( "data", "korganizer/income.cancel/" ) );
  incomingDirChanged( KStandardDirs::locateLocal( "data", "korganizer/income.reply/" ) );
  incomingDirChanged( KStandardDirs::locateLocal( "data", "korganizer/income.delegated/" ) );

  if ( !mFreeBusyManager ) {
    mFreeBusyManager = new FreeBusyManager( this );
    mFreeBusyManager->setObjectName( "freebusymanager" );
    mFreeBusyManager->setCalendar( mCalendar );
    connect( mCalendar, SIGNAL(calendarChanged()),
             mFreeBusyManager, SLOT(slotPerhapsUploadFB()) );
    connect( mView, SIGNAL(newIncidenceChanger(IncidenceChangerBase*)),
             this, SLOT(slotViewNewIncidenceChanger(IncidenceChangerBase*)) );
    slotViewNewIncidenceChanger( mView->incidenceChanger() );
  }
}

void KOGroupware::slotViewNewIncidenceChanger( IncidenceChangerBase *changer )
{
  // Call slot perhapsUploadFB if an incidence was added, changed or removed
  connect( changer, SIGNAL(incidenceAdded(Incidence*)),
           mFreeBusyManager, SLOT(slotPerhapsUploadFB()) );
  connect( changer, SIGNAL(incidenceChanged(Incidence*,Incidence*,int)),
           mFreeBusyManager, SLOT(slotPerhapsUploadFB()) );
  connect( changer, SIGNAL(incidenceChanged(Incidence*,Incidence*)),
           mFreeBusyManager, SLOT(slotPerhapsUploadFB()) ) ;
  connect( changer, SIGNAL(incidenceDeleted(Incidence*)),
           mFreeBusyManager, SLOT(slotPerhapsUploadFB()) );
}

FreeBusyManager *KOGroupware::freeBusyManager()
{
  return mFreeBusyManager;
}

void KOGroupware::incomingDirChanged( const QString &path )
{
  const QString incomingDirName = KStandardDirs::locateLocal( "data","korganizer/" ) + "income.";
  if ( !path.startsWith( incomingDirName ) ) {
    kDebug() << "Wrong dir" << path;
    return;
  }
  QString action = path.mid( incomingDirName.length() );
  while ( action.length() > 0 && action[ action.length()-1 ] == '/' ) {
    // Strip slashes at the end
    action.truncate( action.length() - 1 );
  }

  // Handle accepted invitations
  QDir dir( path );
  const QStringList files = dir.entryList( QDir::Files );
  if ( files.isEmpty() ) {
    // No more files here
    return;
  }
  // Read the file and remove it
  QFile f( path + '/' + files[0] );
  if ( !f.open( QIODevice::ReadOnly ) ) {
    kError() << "Can't open file '" << files[0] << "'";
    return;
  }
  QTextStream t( &f );
  t.setCodec( "UTF-8" );
  QString receiver = KPIMUtils::firstEmailAddress( t.readLine() );
  QString iCal = t.readAll();

  f.remove();

  ScheduleMessage *message = mFormat.parseScheduleMessage( mCalendar, iCal );
  if ( !message ) {
    QString errorMessage;
    if ( mFormat.exception() ) {
      errorMessage = i18n( "Error message: %1", mFormat.exception()->message() );
    }
    kDebug() << "Error parsing" << errorMessage;
    KMessageBox::detailedError( mView,
                                i18n( "Error while processing an invitation or update." ),
                                errorMessage );
    return;
  }

  KCal::iTIPMethod method = static_cast<KCal::iTIPMethod>( message->method() );
  KCal::ScheduleMessage::Status status = message->status();
  KCal::Incidence *incidence = dynamic_cast<KCal::Incidence*>( message->event() );
  if( !incidence ) {
    delete message;
    return;
  }
  MailScheduler scheduler( mCalendar );
  if ( action.startsWith( QLatin1String( "accepted" ) ) ||
       action.startsWith( QLatin1String( "tentative" ) ) ||
       action.startsWith( QLatin1String( "delegated" ) ) ||
       action.startsWith( QLatin1String( "counter" ) ) ) {
    // Find myself and set my status. This can't be done in the scheduler,
    // since this does not know the choice I made in the KMail bpf
    KCal::Attendee::List attendees = incidence->attendees();
    KCal::Attendee::List::ConstIterator it;
    for ( it = attendees.constBegin(); it != attendees.constEnd(); ++it ) {
      if ( (*it)->email() == receiver ) {
        if ( action.startsWith( QLatin1String( "accepted" ) ) ) {
          (*it)->setStatus( KCal::Attendee::Accepted );
        } else if ( action.startsWith( QLatin1String( "tentative" ) ) ) {
          (*it)->setStatus( KCal::Attendee::Tentative );
        } else if ( KOPrefs::instance()->outlookCompatCounterProposals() &&
                    action.startsWith( QLatin1String( "counter" ) ) ) {
          (*it)->setStatus( KCal::Attendee::Tentative );
        } else if ( action.startsWith( QLatin1String( "delegated" ) ) ) {
          (*it)->setStatus( KCal::Attendee::Delegated );
        }
        break;
      }
    }
    if ( KOPrefs::instance()->outlookCompatCounterProposals() ||
         !action.startsWith( QLatin1String( "counter" ) ) ) {
      scheduler.acceptTransaction( incidence, method, status );
    }
  } else if ( action.startsWith( QLatin1String( "cancel" ) ) ) {
    // Delete the old incidence, if one is present
    scheduler.acceptTransaction( incidence, KCal::iTIPCancel, status );
  } else if ( action.startsWith( QLatin1String( "reply" ) ) ) {
    if ( method != iTIPCounter ) {
      scheduler.acceptTransaction( incidence, method, status );
    } else {
      // accept counter proposal
      scheduler.acceptCounterProposal( incidence );
      // send update to all attendees
      sendICalMessage( mView, iTIPRequest, incidence );
    }
  } else {
    kError() << "Unknown incoming action" << action;
  }

  if ( action.startsWith( QLatin1String( "counter" ) ) ) {
    mView->editIncidence( incidence, true );
    KOIncidenceEditor *tmp = mView->editorDialog( incidence );
    tmp->selectInvitationCounterProposal( true );
  }
  mView->updateView();
  delete message;
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
bool KOGroupware::sendICalMessage( QWidget *parent,
                                   KCal::iTIPMethod method,
                                   Incidence *incidence, bool isDeleting,
                                   bool statusChanged )
{
  // If there are no attendees, don't bother
  if ( incidence->attendees().isEmpty() ) {
    return true;
  }

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
    if ( incidence->attendees().count() > 1 ||
         incidence->attendees().first()->email() != incidence->organizer().email() ) {
      QString type;
      if ( incidence->type() == "Event" ) {
        type = i18nc( "incidence type is event", "event" );
      } else if ( incidence->type() == "Todo" ) {
        type = i18nc( "incidence type is to-do/task", "task" );
      } else if ( incidence->type() == "Journal" ) {
        type = i18nc( "incidence type is journal", "journal entry" );
      } else {
        type = incidence->type();
      }
      QString txt = i18n( "This %1 includes other people. "
                          "Should email be sent out to the attendees?",
                          type );
      rc = KMessageBox::questionYesNoCancel(
             parent, txt, i18n( "Group Scheduling Email" ),
             KGuiItem( i18n( "Send Email" ) ), KGuiItem( i18n( "Do Not Send" ) ) );
    } else {
      return true;
    }
  } else if ( incidence->type() == "Todo" ) {
    if ( method == iTIPRequest ) {
      // This is an update to be sent to the organizer
      method = iTIPReply;
    }
    // Ask if the user wants to tell the organizer about the current status
    QString txt = i18n( "Do you want to send a status update to the "
                        "organizer of this task?" );
    rc = KMessageBox::questionYesNo(
           parent, txt, QString(),
           KGuiItem( i18n( "Send Update" ) ), KGuiItem( i18n( "Do Not Send" ) ) );
  } else if ( incidence->type() == "Event" ) {
    QString txt;
    if ( statusChanged && method == iTIPRequest ) {
      txt = i18n( "Your status as an attendee of this event "
                  "changed. Do you want to send a status update to the "
                  "organizer of this event?" );
      method = iTIPReply;
      rc = KMessageBox::questionYesNo(
             parent, txt, QString(),
             KGuiItem( i18n( "Send Update" ) ), KGuiItem( i18n( "Do Not Send" ) ) );
    } else {
      if ( isDeleting ) {
        const QStringList myEmails = KOPrefs::instance()->allEmails();
        bool askConfirmation = false;
        for ( QStringList::ConstIterator it = myEmails.begin(); it != myEmails.end(); ++it ) {
          QString email = *it;
          Attendee *me = incidence->attendeeByMail(email);
          if ( me &&
               ( me->status() == KCal::Attendee::Accepted ||
                 me->status() == KCal::Attendee::Delegated ) ) {
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
                    "with the organizer's calendar. Do you really want "
                    "to edit it?" );
      }
      rc = KMessageBox::warningYesNo( parent, txt );
      return rc == KMessageBox::Yes;
    }
  } else {
    kWarning() << "Groupware messages for Journals are not implemented yet!";
    return true;
  }

  if ( rc == KMessageBox::Yes ) {
    // We will be sending out a message here. Now make sure there is
    // some summary
    if ( incidence->summary().isEmpty() ) {
      incidence->setSummary( i18n( "<placeholder>No summary given</placeholder>" ) );
    }
    // Send the mail
    MailScheduler scheduler( mCalendar );
    scheduler.performTransaction( incidence, method );
    return true;
  } else if ( rc == KMessageBox::No ) {
    return true;
  } else {
    return false;
  }
}

void KOGroupware::sendCounterProposal( KCal::Calendar *calendar,
                                       KCal::Event *oldEvent,
                                       KCal::Event *newEvent ) const
{
  if ( !oldEvent || !newEvent || *oldEvent == *newEvent ||
       !KOPrefs::instance()->mUseGroupwareCommunication ) {
    return;
  }
  if ( KOPrefs::instance()->outlookCompatCounterProposals() ) {
    Incidence *tmp = oldEvent->clone();
    tmp->setSummary( i18n( "Counter proposal: %1", newEvent->summary() ) );
    tmp->setDescription( newEvent->description() );
    tmp->addComment( i18n( "Proposed new meeting time: %1 - %2",
                           IncidenceFormatter::dateToString( newEvent->dtStart() ),
                           IncidenceFormatter::dateToString( newEvent->dtEnd() ) ) );
    MailScheduler scheduler( calendar );
    scheduler.performTransaction( tmp, KCal::iTIPReply );
    delete tmp;
  } else {
    MailScheduler scheduler( calendar );
    scheduler.performTransaction( newEvent, iTIPCounter );
  }
}

#include "kogroupware.moc"
