/*
  This file is part of KOrganizer.

  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "incidencechanger.h"
#include "kogroupware.h"
#include "koprefs.h"
#include "mailscheduler.h"
#include "calendarbase.h"

#include <akonadi/kcal/utils.h>

#include <KCal/AssignmentVisitor>
#include <KCal/CalendarResources>
#include <KCal/DndFactory>
#include <KCal/FreeBusy>
#include <KCal/Incidence>

#include <KDebug>
#include <KLocale>
#include <KMessageBox>

using namespace KCal;
using namespace Akonadi;

bool IncidenceChanger::beginChange( const Item &incidence )
{
  if ( !Akonadi::hasIncidence( incidence ) ) {
    return false;
  }
  kDebug() << "for incidence \"" << Akonadi::incidence( incidence )->summary() << "\"";
  return mCalendar->beginChangeFORAKONADI( incidence );
}

bool IncidenceChanger::sendGroupwareMessage( const Item &aitem,
                                             KCal::iTIPMethod method, bool deleting )
{
#ifdef AKONADI_PORT_DISABLED // reenable once this works again (sendIcalMessage, in particular)
  const Incidence::Ptr incidence = Akonadi::incidence( aitem );
  if ( !incidence )
    return false;
  if ( KOPrefs::instance()->thatIsMe( incidence->organizer().email() ) &&
       incidence->attendeeCount() > 0 &&
       !KOPrefs::instance()->mUseGroupwareCommunication ) {
    emit schedule( method, aitem );
    return true;
  } else if ( KOPrefs::instance()->mUseGroupwareCommunication ) {
    // FIXME: Find a widget to use as parent, instead of 0
    return KOGroupware::instance()->sendICalMessage( 0, method, incidence.get(), deleting );
  }
#endif // AKONADI_PORT_DISABLED
  return true;
}

void IncidenceChanger::cancelAttendees( const Item &aitem )
{
  const Incidence::Ptr incidence = Akonadi::incidence( aitem );
  Q_ASSERT( incidence );
  if ( KOPrefs::instance()->mUseGroupwareCommunication ) {
    if ( KMessageBox::questionYesNo(
           0,
           i18n( "Some attendees were removed from the incidence. "
                 "Shall cancel messages be sent to these attendees?" ),
           i18n( "Attendees Removed" ), KGuiItem( i18n( "Send Messages" ) ),
           KGuiItem( i18n( "Do Not Send" ) ) ) == KMessageBox::Yes ) {
      // don't use KOGroupware::sendICalMessage here, because that asks just
      // a very general question "Other people are involved, send message to
      // them?", which isn't helpful at all in this situation. Afterwards, it
      // would only call the MailScheduler::performTransaction, so do this
      // manually.
      // FIXME: Groupware schedulling should be factored out to it's own class
      //        anyway
#ifdef AKONADI_PORT_DISABLED
      MailScheduler scheduler( mCalendar );
      scheduler.performTransaction( incidence, iTIPCancel );
#endif // AKONADI_PORT_DISABLED
    }
  }
}

bool IncidenceChanger::endChange( const Item &incidence )
{
  if ( !Akonadi::hasIncidence( incidence ) )
    return false;

  // FIXME: if that's a groupware incidence, and I'm not the organizer,
  // send out a mail to the organizer with a counterproposal instead
  // of actually changing the incidence. Then no locking is needed.
  // FIXME: if that's a groupware incidence, and the incidence was
  // never locked, we can't unlock it with endChange().

  kDebug() << "\"" << Akonadi::incidence( incidence )->summary() << "\"";
  return mCalendar->endChangeFORAKONADI( incidence );
}

bool IncidenceChanger::deleteIncidence( const Item &aitem )
{
  const Incidence::Ptr incidence = Akonadi::incidence( aitem );
  if ( !incidence ) {
    return true;
  }

  kDebug() << "\"" << incidence->summary() << "\"";
  bool doDelete = sendGroupwareMessage( aitem, KCal::iTIPCancel, true );
  if( doDelete ) {
    // @TODO: let Calendar::deleteIncidence do the locking...
    Incidence::Ptr tmp( incidence->clone() );
    emit incidenceToBeDeleted( aitem );
    doDelete = mCalendar->deleteIncidenceFORAKONADI( aitem );
    if ( !KOPrefs::instance()->thatIsMe( tmp->organizer().email() ) ) {
      const QStringList myEmails = KOPrefs::instance()->allEmails();
      bool notifyOrganizer = false;
      for ( QStringList::ConstIterator it = myEmails.begin(); it != myEmails.end(); ++it ) {
        QString email = *it;
        Attendee *me = tmp->attendeeByMail( email );
        if ( me ) {
          if ( me->status() == KCal::Attendee::Accepted ||
               me->status() == KCal::Attendee::Delegated ) {
            notifyOrganizer = true;
          }
          Attendee *newMe = new Attendee( *me );
          newMe->setStatus( KCal::Attendee::Declined );
          tmp->clearAttendees();
          tmp->addAttendee( newMe );
          break;
        }
      }

      if ( !KOGroupware::instance()->doNotNotify() && notifyOrganizer ) {
#ifdef AKONADI_PORT_DISABLED
        MailScheduler scheduler( mCalendar );
        scheduler.performTransaction( tmp, KCal::iTIPReply );
#endif
      }
      //reset the doNotNotify flag
      KOGroupware::instance()->setDoNotNotify( false );
    }
    emit incidenceDeleted( aitem );
  }
  return doDelete;
}

bool IncidenceChanger::cutIncidence( const Item& aitem )
{
  const Incidence::Ptr incidence = Akonadi::incidence( aitem );

  if ( !incidence ) {
    return true;
  }

  kDebug() << "\"" << incidence->summary() << "\"";
  bool doDelete = sendGroupwareMessage( aitem, KCal::iTIPCancel );
  if( doDelete ) {
#ifdef AKONADI_PORT_DISABLED
    // @TODO: the factory needs to do the locking!
    DndFactory factory( mCalendar );
    emit incidenceToBeDeleted( incidence );
    factory.cutIncidence( incidence );
    emit incidenceDeleted( incidence );
#endif
  }
  return doDelete;
}

class IncidenceChanger::ComparisonVisitor : public IncidenceBase::Visitor
{
  public:
    ComparisonVisitor() {}
    bool act( IncidenceBase *incidence, IncidenceBase *inc2 )
    {
      mIncidence2 = inc2;
      if ( incidence ) {
        return incidence->accept( *this );
      } else {
        return inc2 == 0;
      }
    }

  protected:
    bool visit( Event *event )
    {
      Event *ev2 = dynamic_cast<Event*>( mIncidence2 );
      if ( event && ev2 ) {
        return *event == *ev2;
      } else {
        // either both 0, or return false;
        return ev2 == event;
      }
    }
    bool visit( Todo *todo )
    {
      Todo *to2 = dynamic_cast<Todo*>( mIncidence2 );
      if ( todo && to2 ) {
        return *todo == *to2;
      } else {
        // either both 0, or return false;
        return todo == to2;
      }
    }
    bool visit( Journal *journal )
    {
      Journal *j2 = dynamic_cast<Journal*>( mIncidence2 );
      if ( journal && j2 ) {
        return *journal == *j2;
      } else {
        // either both 0, or return false;
        return journal == j2;
      }
    }
    bool visit( FreeBusy *fb )
    {
      FreeBusy *fb2 = dynamic_cast<FreeBusy*>( mIncidence2 );
      if ( fb && fb2 ) {
        return *fb == *fb2;
      } else {
        // either both 0, or return false;
        return fb2 == fb;
      }
    }

  protected:
    IncidenceBase *mIncidence2;
};

bool IncidenceChanger::incidencesEqual( Incidence *inc1, Incidence *inc2 )
{
  ComparisonVisitor v;
  return ( v.act( inc1, inc2 ) );
}

bool IncidenceChanger::assignIncidence( Incidence *inc1, Incidence *inc2 )
{
  if ( !inc1 || !inc2 ) {
    return false;
  }
  // PENDING(AKONADI_PORT) review
  AssignmentVisitor v;
  return v.assign( inc1, inc2 );
}

bool IncidenceChanger::myAttendeeStatusChanged( const Incidence* newInc, const Incidence* oldInc )
{
  Attendee *oldMe = oldInc->attendeeByMails( KOPrefs::instance()->allEmails() );
  Attendee *newMe = newInc->attendeeByMails( KOPrefs::instance()->allEmails() );
  if ( oldMe && newMe && ( oldMe->status() != newMe->status() ) ) {
    return true;
  }

  return false;
}

bool IncidenceChanger::changeIncidence( const Incidence::Ptr &oldinc, const Item &newItem,
                                        int action )
{
  const Incidence::Ptr newinc = Akonadi::incidence( newItem );

  kDebug() << "for incidence \"" << newinc->summary() << "\""
           << "( old one was \"" << oldinc->summary() << "\")";

  if ( incidencesEqual( newinc.get(), oldinc.get() ) ) {
    // Don't do anything
    kDebug() << "Incidence not changed";
  } else {
    kDebug() << "Changing incidence";
    bool statusChanged = myAttendeeStatusChanged( oldinc.get(), newinc.get() );
    int revision = newinc->revision();
    newinc->setRevision( revision + 1 );
    // FIXME: Use a generic method for this! Ideally, have an interface class
    //        for group cheduling. Each implementation could then just do what
    //        it wants with the event. If no groupware is used,use the null
    //        pattern...
    bool success = true;
    if ( KOPrefs::instance()->mUseGroupwareCommunication ) {
      success = KOGroupware::instance()->sendICalMessage( 0,
                                                          KCal::iTIPRequest,
                                                          newinc.get(), false, statusChanged );
    }

    if ( success ) {
      // Accept the event changes
      if ( action < 0 ) {
        emit incidenceChanged( oldinc, newItem );
      } else {
        emit incidenceChanged( oldinc, newItem, action );
      }
    } else {
      kDebug() << "Changing incidence failed. Reverting changes.";
      assignIncidence( newinc.get(), oldinc.get() );
      return false;
    }
  }
  return true;
}

bool IncidenceChanger::addIncidence( const Incidence::Ptr &incidence, QWidget *parent )
{
  kDebug() << "\"" << incidence->summary() << "\"";
  CalendarResources *stdcal = dynamic_cast<CalendarResources*>( mCalendar );
  if( stdcal && !stdcal->hasCalendarResources() ) {
    KMessageBox::sorry( parent, i18n( "No calendars found. We can not add event." ) );
    return false;
  }

  // FIXME: This is a nasty hack, since we need to set a parent for the
  //        resource selection dialog. However, we don't have any UI methods
  //        in the calendar, only in the CalendarResources::DestinationPolicy
  //        So we need to type-cast it and extract it from the CalendarResources
  QWidget *tmpparent = 0;
  if ( stdcal ) {
    tmpparent = stdcal->dialogParentWidget();
    stdcal->setDialogParentWidget( parent );
  }
  bool success = mCalendar->addIncidenceFORAKONADI( incidence );
  if ( stdcal ) {
    // Reset the parent widget, otherwise we'll end up with pointers to deleted
    // widgets sooner or later
    stdcal->setDialogParentWidget( tmpparent );
  }
  if ( !success ) {
    // We can have a failure if the user pressed [cancel] in the resource
    // selectdialog, so check the exception.
    ErrorFormat *e = stdcal ? stdcal->exception() : 0;
    if ( !e || ( e && ( e->errorCode() != KCal::ErrorFormat::UserCancel ) ) ) {
      KMessageBox::sorry( parent,
                          i18n( "Unable to save %1 \"%2\".",
                                i18n( incidence->type() ),
                                incidence->summary() ) );
    }
    return false;
  }

  if ( KOPrefs::instance()->mUseGroupwareCommunication ) {
    if ( !KOGroupware::instance()->sendICalMessage( parent,
                                                    KCal::iTIPRequest,
                                                    incidence.get() ) ) {
      kError() << "sendIcalMessage failed.";
    }
  }
#ifdef AKONADI_PORT_DISABLED // don't notify here, wait for akonadi notification
  emit incidenceAdded( item );
#endif // AKONADI_PORT_DISABLED
  return true;
}

#include "incidencechanger.moc"
