/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "incidencechanger.h"
#include "koglobals.h"
#include "koprefs.h"
#include "kogroupware.h"
#include "mailscheduler.h"

#include <libkcal/freebusy.h>
#include <libkcal/dndfactory.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>


bool IncidenceChanger::beginChange( Incidence *incidence,
                                    ResourceCalendar *res, const QString &subRes )
{
  if ( !incidence ) {
    return false;
  }

  kdDebug(5850) << "IncidenceChanger::beginChange for incidence \""
                << incidence->summary() << "\"" << endl;

  CalendarResources *calRes = dynamic_cast<CalendarResources*>( mCalendar );
  if ( !calRes ) {
    return false;
  }

  return calRes->beginChange( incidence, res, subRes );
}

bool IncidenceChanger::sendGroupwareMessage( Incidence *incidence,
                                             KCal::Scheduler::Method method,
                                             KOGlobals::HowChanged action,
                                             QWidget *parent )
{
  if ( KOPrefs::instance()->thatIsMe( incidence->organizer().email() ) && incidence->attendeeCount()>0
      && !KOPrefs::instance()->mUseGroupwareCommunication ) {
    emit schedule( method, incidence );
    return true;
  } else if( KOPrefs::instance()->mUseGroupwareCommunication ) {
    return
      KOGroupware::instance()->sendICalMessage( parent, method, incidence, action, false );
  }
  return true;
}

void IncidenceChanger::cancelAttendees( Incidence *incidence )
{
  if ( KOPrefs::instance()->mUseGroupwareCommunication ) {
    if ( KMessageBox::questionYesNo( 0, i18n("Some attendees were removed "
       "from the incidence. Shall cancel messages be sent to these attendees?"),
       i18n( "Attendees Removed" ), i18n("Send Messages"), i18n("Do Not Send") ) == KMessageBox::Yes ) {
      // don't use KOGroupware::sendICalMessage here, because that asks just
      // a very general question "Other people are involved, send message to
      // them?", which isn't helpful at all in this situation. Afterwards, it
      // would only call the MailScheduler::performTransaction, so do this
      // manually.
      // FIXME: Groupware scheduling should be factored out to it's own class
      //        anyway
      KCal::MailScheduler scheduler( mCalendar );
      scheduler.performTransaction( incidence, Scheduler::Cancel );
    }
  }
}

bool IncidenceChanger::endChange( Incidence *incidence,
                                  ResourceCalendar *res, const QString &subRes )
{
  // FIXME: if that's a groupware incidence, and I'm not the organizer,
  // send out a mail to the organizer with a counterproposal instead
  // of actually changing the incidence. Then no locking is needed.
  // FIXME: if that's a groupware incidence, and the incidence was
  // never locked, we can't unlock it with endChange().

  if ( !incidence ) {
    return false;
  }

  kdDebug(5850) << "IncidenceChanger::endChange for incidence \""
                << incidence->summary() << "\"" << incidence->dtStart() << endl;

  CalendarResources *calRes = dynamic_cast<CalendarResources*>( mCalendar );
  if ( !calRes ) {
    kdDebug() << "CalRes is null!" << endl;
    return false;
  }

  return calRes->endChange( incidence, res, subRes );
}

bool IncidenceChanger::deleteIncidence( Incidence *incidence, QWidget *parent )
{
  if ( !incidence ) return true;
kdDebug(5850)<<"IncidenceChanger::deleteIncidence for incidence \""<<incidence->summary()<<"\""<<endl;
  bool doDelete = sendGroupwareMessage( incidence, KCal::Scheduler::Cancel,
                                        KOGlobals::INCIDENCEDELETED, parent );
  if( doDelete ) {
    // @TODO: let Calendar::deleteIncidence do the locking...
    Incidence* tmp = incidence->clone();
    emit incidenceToBeDeleted( incidence );
    doDelete = mCalendar->deleteIncidence( incidence );
    if ( !KOPrefs::instance()->thatIsMe( tmp->organizer().email() ) ) {
      const QStringList myEmails = KOPrefs::instance()->allEmails();
      bool notifyOrganizer = false;
      for ( QStringList::ConstIterator it = myEmails.begin(); it != myEmails.end(); ++it ) {
        QString email = *it;
        Attendee *me = tmp->attendeeByMail(email);
        if ( me ) {
          if ( me->status() == KCal::Attendee::Accepted || me->status() == KCal::Attendee::Delegated )
            notifyOrganizer = true;
          Attendee *newMe = new Attendee( *me );
          newMe->setStatus( KCal::Attendee::Declined );
          tmp->clearAttendees();
          tmp->addAttendee( newMe );
          break;
        }
      }

      if ( !KOGroupware::instance()->doNotNotify() && notifyOrganizer ) {
          KCal::MailScheduler scheduler( mCalendar );
          scheduler.performTransaction( tmp, Scheduler::Reply );
      }
      //reset the doNotNotify flag
      KOGroupware::instance()->setDoNotNotify( false );
    }
    emit incidenceDeleted( incidence );
  }
  return doDelete;
}

bool IncidenceChanger::cutIncidences( const Incidence::List &incidences,
                                      QWidget *parent )
{
  Incidence::List::ConstIterator it;
  bool doDelete = true;
  Incidence::List incsToCut;
  for ( it = incidences.constBegin(); it != incidences.constEnd(); ++it ) {
    if ( *it ) {
      doDelete = sendGroupwareMessage( *it, KCal::Scheduler::Cancel,
                                       KOGlobals::INCIDENCEDELETED, parent );
      if ( doDelete ) {
        emit incidenceToBeDeleted( *it );
        incsToCut.append( *it );
      }
    }
  }

  DndFactory factory( mCalendar );

  if ( factory.cutIncidences( incsToCut ) ) {
    for ( it = incsToCut.constBegin(); it != incsToCut.constEnd(); ++it ) {
      emit incidenceDeleted( *it );
    }
    return !incsToCut.isEmpty();
  } else {
    return false;
  }
}

bool IncidenceChanger::cutIncidence( Incidence *incidence, QWidget *parent )
{
  Incidence::List incidences;
  incidences.append( incidence );
  return cutIncidences( incidences, parent );
}

class IncidenceChanger::ComparisonVisitor : public IncidenceBase::Visitor
{
  public:
    ComparisonVisitor() {}
    bool act( IncidenceBase *incidence, IncidenceBase *inc2 )
    {
      mIncidence2 = inc2;
      if ( incidence )
        return incidence->accept( *this );
      else
        return (inc2 == 0);
    }
  protected:
    bool visit( Event *event )
    {
      Event *ev2 = dynamic_cast<Event*>(mIncidence2);
      if ( event && ev2 ) {
        return *event == *ev2;
      } else {
        // either both 0, or return false;
        return ( ev2 == event );
      }
    }
    bool visit( Todo *todo )
    {
      Todo *to2 = dynamic_cast<Todo*>( mIncidence2 );
      if ( todo && to2 ) {
        return *todo == *to2;
      } else {
        // either both 0, or return false;
        return ( todo == to2 );
      }
    }
    bool visit( Journal *journal )
    {
      Journal *j2 = dynamic_cast<Journal*>( mIncidence2 );
      if ( journal && j2 ) {
        return *journal == *j2;
      } else {
        // either both 0, or return false;
        return ( journal == j2 );
      }
    }
    bool visit( FreeBusy *fb )
    {
      FreeBusy *fb2 = dynamic_cast<FreeBusy*>( mIncidence2 );
      if ( fb && fb2 ) {
        return *fb == *fb2;
      } else {
        // either both 0, or return false;
        return ( fb2 == fb );
      }
    }

  protected:
    IncidenceBase *mIncidence2;
};

class IncidenceChanger::AssignmentVisitor : public IncidenceBase::Visitor
{
  public:
    AssignmentVisitor() {}
    bool act( IncidenceBase *incidence, IncidenceBase *inc2 )
    {
      mIncidence2 = inc2;
      if ( incidence )
        return incidence->accept( *this );
      else
        return false;
    }
  protected:
    bool visit( Event *event )
    {
      Event *ev2 = dynamic_cast<Event*>( mIncidence2 );
      if ( event && ev2 ) {
        *event = *ev2;
        return true;
      } else {
        return false;
      }
    }
    bool visit( Todo *todo )
    {
      Todo *to2 = dynamic_cast<Todo*>( mIncidence2 );
      if ( todo && to2 ) {
        *todo = *to2;
        return true;
      } else {
        return false;
      }
    }
    bool visit( Journal *journal )
    {
      Journal *j2 = dynamic_cast<Journal*>(mIncidence2);
      if ( journal && j2 ) {
        *journal = *j2;
        return true;
      } else {
        return false;
      }
    }
    bool visit( FreeBusy *fb )
    {
      FreeBusy *fb2 = dynamic_cast<FreeBusy*>( mIncidence2 );
      if ( fb && fb2 ) {
        *fb = *fb2;
        return true;
      } else {
        return false;
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
  AssignmentVisitor v;
  return v.act( inc1, inc2 );
}

bool IncidenceChanger::myAttendeeStatusChanged( Incidence *oldInc, Incidence *newInc )
{
  Attendee *oldMe = oldInc->attendeeByMails( KOPrefs::instance()->allEmails() );
  Attendee *newMe = newInc->attendeeByMails( KOPrefs::instance()->allEmails() );
  if ( oldMe && newMe && ( oldMe->status() != newMe->status() ) )
    return true;

  return false;
}

bool IncidenceChanger::changeIncidence( Incidence *oldinc, Incidence *newinc,
                                        KOGlobals::WhatChanged action,
                                        QWidget *parent,
                                        bool useLastDialogAnswer )
{
kdDebug(5850)<<"IncidenceChanger::changeIncidence for incidence \""<<newinc->summary()<<"\" ( old one was \""<<oldinc->summary()<<"\")"<<endl;
  if ( incidencesEqual( newinc, oldinc ) ) {
    // Don't do anything
    kdDebug(5850) << "Incidence not changed\n";
  } else {
    kdDebug(5850) << "Incidence changed\n";
    bool attendeeStatusChanged = myAttendeeStatusChanged( oldinc, newinc );
    int revision = newinc->revision();
    newinc->setRevision( revision + 1 );
    // FIXME: Use a generic method for this! Ideally, have an interface class
    //        for group cheduling. Each implementation could then just do what
    //        it wants with the event. If no groupware is used,use the null
    //        pattern...
    bool success = true;
    if ( KOPrefs::instance()->mUseGroupwareCommunication ) {
      success = KOGroupware::instance()->sendICalMessage(
        parent,
        KCal::Scheduler::Request,
        newinc, KOGlobals::INCIDENCEEDITED, attendeeStatusChanged,
        useLastDialogAnswer );
    }

    if ( success ) {
      // Accept the event changes
      emit incidenceChanged( oldinc, newinc, action );
    } else {
      // revert changes
      assignIncidence( newinc, oldinc );
      return false;
    }
  }
  return true;
}

bool IncidenceChanger::addIncidence( Incidence *incidence,
                                     ResourceCalendar *res, const QString &subRes,
                                     QWidget *parent, bool useLastDialogAnswer )
{
  CalendarResources *stdcal = dynamic_cast<CalendarResources *>( mCalendar );
  if ( stdcal && !stdcal->hasCalendarResources() ) {
    KMessageBox::sorry(
      parent,
      i18n( "No calendars found, unable to save %1 \"%2\"." ).
      arg( i18n( incidence->type() ) ).
      arg( incidence->summary() ) );
    kdDebug(5850) << "IncidenceChanger: No calendars found" << endl;
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

  // If a ResourceCalendar isn't provided, then try to compute one
  // along with any subResource from the incidence.
  ResourceCalendar *pRes = res;
  QString pSubRes = subRes;
  QString pResName;
  if ( !pRes ) {
    if ( stdcal ) {
      pRes = stdcal->resource( incidence );
      if ( pRes ) {
        pResName = pRes->resourceName();
        if ( pRes->canHaveSubresources() ) {
          pSubRes = pRes->subresourceIdentifier( incidence );
          pResName = pRes->labelForSubresource( pSubRes );
        }
      }
    }
  }

  bool success = false;
  if ( stdcal && pRes && !pRes->readOnly() && pRes->subresourceWritable( pSubRes ) ) {
    success = stdcal->addIncidence( incidence, pRes, pSubRes );
  } else {
    success = mCalendar->addIncidence( incidence );
  }

  if ( !success ) {
    // We can have a failure if the user pressed [cancel] in the resource
    // selectdialog, so check the exception.
    ErrorFormat *e = stdcal ? stdcal->exception() : 0;
    if ( !e ||
         ( e && ( e->errorCode() != KCal::ErrorFormat::UserCancel &&
                  e->errorCode() != KCal::ErrorFormat::NoWritableFound ) ) ) {
      QString errMessage;
      if ( pResName.isEmpty() ) {
        errMessage = i18n( "Unable to save %1 \"%2\"." ).
                     arg( i18n( incidence->type() ) ).
                     arg( incidence->summary() );
      } else {
        errMessage = i18n( "Unable to save %1 \"%2\" to calendar %3." ).
                     arg( i18n( incidence->type() ) ).
                     arg( incidence->summary() ).
                     arg( pResName );
      }
      KMessageBox::sorry( parent, errMessage );
    }
    kdDebug(5850) << "IncidenceChanger: Can't add incidence" << endl;
    return false;
  }

  if ( KOPrefs::instance()->mUseGroupwareCommunication ) {
    if ( !KOGroupware::instance()->sendICalMessage(
           parent,
           KCal::Scheduler::Request,
           incidence, KOGlobals::INCIDENCEADDED, false, useLastDialogAnswer ) ) {
      KMessageBox::sorry(
        parent,
        i18n( "Attempt to send the scheduling message failed. "
              "Please check your Group Scheduling settings. "
              "Contact your system administrator for more help.") );
    }
  }

  emit incidenceAdded( incidence );
  return true;
}


#include "incidencechanger.moc"
#include "incidencechangerbase.moc"
