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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "incidencechanger.h"
#include "koprefs.h"
#include "kogroupware.h"
#include "mailscheduler.h"

#include <libkcal/freebusy.h>
#include <libkcal/dndfactory.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>



bool IncidenceChanger::beginChange( Incidence * incidence )
{
  if ( !incidence ) return false;
kdDebug(5850)<<"IncidenceChanger::beginChange for incidence \""<<incidence->summary()<<"\""<<endl;
  return mCalendar->beginChange( incidence );
}

bool IncidenceChanger::sendGroupwareMessage( Incidence *incidence, KCal::Scheduler::Method method, bool deleting )
{
  if ( KOPrefs::instance()->thatIsMe( incidence->organizer().email() ) && incidence->attendeeCount()>0
      && !KOPrefs::instance()->mUseGroupwareCommunication ) {
    emit schedule( method, incidence );
    return true;
  } else if( KOPrefs::instance()->mUseGroupwareCommunication ) {
  // FIXME: Find a widget to use as parent, instead of 0
    return KOGroupware::instance()->sendICalMessage( 0, method, incidence, deleting );
  }
  return true;
}

void IncidenceChanger::cancelAttendees( Incidence *incidence )
{
  if ( KOPrefs::instance()->mUseGroupwareCommunication ) {
    if ( KMessageBox::questionYesNo( 0, i18n("Some attendees were removed "
       "from the incidence. Shall cancel messages be sent to these attendees?"),
       i18n( "Attendees Removed" ) ) == KMessageBox::Yes ) {
      // don't use KOGroupware::sendICalMessage here, because that asks just
      // a very general question "Other people are involved, send message to
      // them?", which isn't helpful at all in this situation. Afterwards, it
      // would only call the MailScheduler::performTransaction, so do this
      // manually.
      // FIXME: Groupware schedulling should be factored out to it's own class
      //        anyway
      KCal::MailScheduler scheduler( mCalendar );
      scheduler.performTransaction( incidence, Scheduler::Cancel );
    }
  }
}

bool IncidenceChanger::endChange( Incidence *incidence )
{
  // FIXME: if that's a groupware incidence, and I'm not the organizer, 
  // send out a mail to the organizer with a counterproposal instead
  // of actually changing the incidence. Then no locking is needed.
  // FIXME: if that's a groupware incidence, and the incidence was
  // never locked, we can't unlock it with endChange().
  if ( !incidence ) return false;
kdDebug(5850)<<"IncidenceChanger::endChange for incidence \""<<incidence->summary()<<"\""<<endl;
  return mCalendar->endChange( incidence );
}

bool IncidenceChanger::deleteIncidence( Incidence *incidence )
{
  if ( !incidence ) return true;
kdDebug(5850)<<"IncidenceChanger::deleteIncidence for incidence \""<<incidence->summary()<<"\""<<endl;
  bool doDelete = sendGroupwareMessage( incidence, KCal::Scheduler::Cancel );
  if( doDelete ) {
    // @TODO: let Calendar::deleteIncidence do the locking...
    emit incidenceToBeDeleted( incidence );
    doDelete = mCalendar->deleteIncidence( incidence );
    emit incidenceDeleted( incidence );
  }
  return doDelete;
}

bool IncidenceChanger::cutIncidence( Incidence *incidence )
{
  if ( !incidence ) return true;
kdDebug(5850)<<"IncidenceChanger::deleteIncidence for incidence \""<<incidence->summary()<<"\""<<endl;
  bool doDelete = sendGroupwareMessage( incidence, KCal::Scheduler::Cancel );
  if( doDelete ) {
    // @TODO: the factory needs to do the locking!
    DndFactory factory( mCalendar );
    emit incidenceToBeDeleted( incidence );
    factory.cutIncidence( incidence );
    emit incidenceDeleted( incidence );
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

/*bool IncidenceChanger::assignIncidence( Incidence *inc1, Incidence *inc2 )
{
  AssignmentVisitor v;
  return v.act( inc1, inc2 );
}*/

bool IncidenceChanger::myAttendeeStatusChanged( Incidence *oldInc, Incidence *newInc )
{
  Attendee *oldMe = oldInc->attendeeByMails( KOPrefs::instance()->allEmails() );
  Attendee *newMe = newInc->attendeeByMails( KOPrefs::instance()->allEmails() );
  if ( oldMe && newMe && ( oldMe->status() != newMe->status() ) )
    return true;

  return false;
}

bool IncidenceChanger::changeIncidence( Incidence *oldinc, Incidence *newinc, 
                                        int action )
{
kdDebug(5850)<<"IncidenceChanger::changeChange for incidence \""<<newinc->summary()<<"\" ( old one was \""<<oldinc->summary()<<"\")"<<endl;
  if( incidencesEqual( newinc, oldinc ) ) {
    // Don't do anything
    kdDebug(5850) << "Incidence not changed\n";
  } else {
    kdDebug(5850) << "Incidence changed\n";
    bool statusChanged = myAttendeeStatusChanged( oldinc, newinc );
    int revision = newinc->revision();
    newinc->setRevision( revision + 1 );
    // FIXME: Use a generic method for this! Ideally, have an interface class 
    //        for group cheduling. Each implementation could then just do what 
    //        it wants with the event. If no groupware is used,use the null 
    //        pattern...
    if( !KOPrefs::instance()->mUseGroupwareCommunication ||
        KOGroupware::instance()->sendICalMessage( 0,
                                                  KCal::Scheduler::Request,
                                                  newinc, false, statusChanged ) ) {
      // Accept the event changes
      if ( action<0 ) {
        emit incidenceChanged( oldinc, newinc );
      } else {
        emit incidenceChanged( oldinc, newinc, action );
      }
    } else {
      // FIXME: Revert the changes
//      assignIncidence( newinc, oldinc );
      return false;
    }
  }
  return true;
}

bool IncidenceChanger::addIncidence( Incidence *incidence )
{
kdDebug(5850)<<"IncidenceChanger::addIncidence for incidence \""<<incidence->summary()<<"\""<<endl;
  if ( KOPrefs::instance()->mUseGroupwareCommunication ) {
    if ( !KOGroupware::instance()->sendICalMessage( 0,
                                                    KCal::Scheduler::Request,
                                                    incidence ) ) {
      kdError() << "sendIcalMessage failed." << endl;
    }
  }
  if ( !mCalendar->addIncidence( incidence ) ) {
    KMessageBox::sorry( 0, i18n("Unable to save %1 \"%2\".")
                        .arg( i18n( incidence->type() ) )
                        .arg( incidence->summary() ) );
    return false;
  }
  emit incidenceAdded( incidence );
  return true;
}

#include "incidencechanger.moc"
#include "incidencechangerbase.moc"
