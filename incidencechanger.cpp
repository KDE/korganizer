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
#include "koglobals.h"
#include "koprefs.h"

#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/calendaradaptor.h>
#include <akonadi/kcal/groupware.h>
#include <akonadi/kcal/mailscheduler.h>
#include <akonadi/kcal/utils.h>
#include <akonadi/kcal/dndfactory.h>

#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/Collection>

#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>


#include <KCal/AssignmentVisitor>
#include <KCal/FreeBusy>
#include <KCal/Incidence>
#include <kcal/comparisonvisitor.h>

#include <KDebug>
#include <KLocale>
#include <KMessageBox>

using namespace KCal;
using namespace Akonadi;

class IncidenceChanger::Private {
public:
  Private() {
  }
  ~Private() {
  }
  QList<Akonadi::Item::Id> m_changes; //list of item ids that are modified atm
  KCal::Incidence::Ptr m_incidenceBeingChanged; // clone of the incidence currently being modified, for rollback and to check if something actually changed
  Item m_itemBeingChanged;

  QHash<Akonadi::Item::Id, int> m_latestVersionByItemId;
  QHash<const KJob*, Item> m_oldItemByJob;
};

IncidenceChanger::IncidenceChanger( Akonadi::Calendar *cal, QObject *parent )
  : IncidenceChangerBase( cal, parent ), d( new Private )
{
}

IncidenceChanger::~IncidenceChanger()
{
  delete d;
}

bool IncidenceChanger::beginChange( const Item &item )
{
  if ( !Akonadi::hasIncidence( item ) ) {
    kDebug() << "Skipping invalid item id=" << item.id();
    return false;
  }

  // Don't start two modify jobs over the same revision
  if ( d->m_latestVersionByItemId.contains( item.id() ) &&
       d->m_latestVersionByItemId[item.id()] > item.revision() ) {
    kDebug() << "Skipping item " << item.id() << " with revision " << item.revision();
    return false;
  }

  const Incidence::Ptr incidence = Akonadi::incidence( item );
  Q_ASSERT( incidence );
  kDebug() << "id=" << item.id() << "uid=" << incidence->uid() << "version=" << item.revision() << "summary=" << incidence->summary() << "type=" << incidence->type() << "storageCollectionId=" << item.storageCollectionId();
  
  if ( !d->m_changes.contains( item.id() ) ) { // no nested changes allowed
    d->m_changes.push_back( item.id() );
    d->m_incidenceBeingChanged = Incidence::Ptr( incidence->clone() );
    d->m_itemBeingChanged = item;
    return true;
  } else {
    kDebug() << "No nested changes allowed id = " << item.id();
    return false;
  }
}

bool IncidenceChanger::sendGroupwareMessage( const Item &aitem,
                                             KCal::iTIPMethod method,
                                             Akonadi::Groupware::HowChanged action,
                                             QWidget *parent )
{
  const Incidence::Ptr incidence = Akonadi::incidence( aitem );
  if ( !incidence )
    return false;
  if ( KOPrefs::instance()->thatIsMe( incidence->organizer().email() ) &&
       incidence->attendeeCount() > 0 &&
       !KOPrefs::instance()->mUseGroupwareCommunication ) {
    emit schedule( method, aitem );
    return true;
  } else if ( KOPrefs::instance()->mUseGroupwareCommunication ) {
    return
      Akonadi::Groupware::instance()->sendICalMessage( parent, method, incidence.get(), action,  false );
  }
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
      // don't use Akonadi::Groupware::sendICalMessage here, because that asks just
      // a very general question "Other people are involved, send message to
      // them?", which isn't helpful at all in this situation. Afterwards, it
      // would only call the Akonadi::MailScheduler::performTransaction, so do this
      // manually.
      // FIXME: Groupware scheduling should be factored out to it's own class
      //        anyway
      Akonadi::MailScheduler scheduler( static_cast<Akonadi::Calendar*>(mCalendar) );
      scheduler.performTransaction( incidence.get(), iTIPCancel );
    }
  }
}

bool IncidenceChanger::endChange( const Item &item )
{
  if ( !Akonadi::hasIncidence( item ) ) {
    return false;
  }

  // FIXME: if that's a groupware incidence, and I'm not the organizer,
  // send out a mail to the organizer with a counterproposal instead
  // of actually changing the incidence. Then no locking is needed.
  // FIXME: if that's a groupware incidence, and the incidence was
  // never locked, we can't unlock it with endChange().

  const Incidence::Ptr incidence = Akonadi::incidence( item );
  Q_ASSERT( incidence );

  const bool isModification = d->m_changes.contains( item.id() );

  if ( !isModification || !d->m_incidenceBeingChanged ) {
    // only if beginChange() with the incidence was called then this is a modification else it
    // is e.g. a new event/todo/journal that was not added yet or an existing one got deleted.
    kDebug() << "Skipping modify uid=" << incidence->uid() << "summary=" << incidence->summary() << "type=" << incidence->type();

    d->m_changes.removeAll( item.id() );

    return false;
  }

  // check if there was an actual change to the incidence since beginChange
  // if not, don't kick off a modify job. The reason this is useful is that
  // begin/endChange is used for locking as well, so it is called quite often
  // without any actual changes happening. Nested modify jobs confuse the
  // conflict detection in Akonadi, so let's avoid them.
  KCal::ComparisonVisitor v;
  Incidence::Ptr incidencePtr( d->m_incidenceBeingChanged );
  d->m_incidenceBeingChanged.reset();
  const Item oldItem = d->m_itemBeingChanged;
  d->m_itemBeingChanged = Item();
  if ( v.compare( incidence.get(), incidencePtr.get() ) ) {
    kDebug()<<"Incidence is unmodified";

    d->m_changes.removeAll( item.id() );

    return true;
  }

  kDebug() << "modify id=" << item.id() << "uid=" << incidence->uid() << "version=" << item.revision() << "summary=" << incidence->summary() << "type=" << incidence->type() << "storageCollectionId=" << item.storageCollectionId();
  ItemModifyJob *job = new ItemModifyJob( item );
  d->m_oldItemByJob.insert( job, oldItem );
  connect( job, SIGNAL(result( KJob*)), this, SLOT(changeIncidenceFinished(KJob*)) );
  return true;
}

bool IncidenceChanger::deleteIncidence( const Item &aitem, QWidget *parent )
{
  const Incidence::Ptr incidence = Akonadi::incidence( aitem );
  if ( !incidence ) {
    return true;
  }

  kDebug() << "\"" << incidence->summary() << "\"";
  bool doDelete = sendGroupwareMessage( aitem, KCal::iTIPCancel,
                                        Akonadi::Groupware::INCIDENCEDELETED, parent );
  if( !doDelete ) {
    return false;
  }
  emit incidenceToBeDeleted( aitem );
  d->m_changes.removeAll( aitem.id() ); //abort changes to this incidence cause we will just delete it
  ItemDeleteJob* job = new ItemDeleteJob( aitem );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(deleteIncidenceFinished(KJob*)) );
  return true;
}

void IncidenceChanger::changeIncidenceFinished( KJob* j )
{
  //AKONADI_PORT this is from the respective method in the old Akonadi::Calendar, so I leave it here: --Frank
  kDebug();

  // we should probably update the revision number here,or internally in the Event
  // itself when certain things change. need to verify with ical documentation.
  const ItemModifyJob* job = qobject_cast<const ItemModifyJob*>( j );
  Q_ASSERT( job );

  const Item oldItem = d->m_oldItemByJob.value( job );
  d->m_oldItemByJob.remove( job );
  const Item newItem = job->item();
  Incidence::Ptr tmp = Akonadi::incidence( newItem );
  Q_ASSERT( tmp );

  if ( job->error() ) {
    kWarning( 5250 ) << "Item modify failed:" << job->errorString();
    KMessageBox::sorry( 0, //PENDING(AKONADI_PORT) set parent
                        i18n( "Unable to save changes for incidence %1 \"%2\": %3",
                              i18n( tmp->type() ),
                              tmp->summary(),
                              job->errorString( )) );
  } else {
    //PENDING(AKONADI_PORT) emit a real action here, not just UNKNOWN_MODIFIED
    emit incidenceChanged( oldItem, newItem, KOGlobals::UNKNOWN_MODIFIED );
  }

  d->m_latestVersionByItemId[newItem.id()] = newItem.revision();
  d->m_changes.removeAll( oldItem.id() );
}

void IncidenceChanger::deleteIncidenceFinished( KJob* j )
{
  kDebug();
  const ItemDeleteJob* job = qobject_cast<const ItemDeleteJob*>( j );
  Q_ASSERT( job );
  const Item::List items = job->deletedItems();
  Q_ASSERT( items.count() == 1 );
  Incidence::Ptr tmp = Akonadi::incidence( items.first() );
  Q_ASSERT( tmp );
  if ( job->error() ) {
    KMessageBox::sorry( 0, //PENDING(AKONADI_PORT) set parent
                        i18n( "Unable to delete incidence %1 \"%2\": %3",
                              i18n( tmp->type() ),
                              tmp->summary(),
                              job->errorString( )) );
    return;
  }
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

    if ( !Akonadi::Groupware::instance()->doNotNotify() && notifyOrganizer ) {
      Akonadi::MailScheduler scheduler( static_cast<Akonadi::Calendar*>(mCalendar) );
      scheduler.performTransaction( tmp.get(), KCal::iTIPReply );
    }
    //reset the doNotNotify flag
    Akonadi::Groupware::instance()->setDoNotNotify( false );
  }
  emit incidenceDeleted( items.first() );
}

bool IncidenceChanger::cutIncidence( const Item& aitem, QWidget *parent )
{
  if ( !aitem.isValid() ) {
    return true;
  }

  //kDebug() << "\"" << incidence->summary() << "\"";
  bool doDelete = sendGroupwareMessage( aitem, KCal::iTIPCancel,
                                        Akonadi::Groupware::INCIDENCEDELETED, parent );
  if( doDelete ) {
    // @TODO: the factory needs to do the locking!
    Akonadi::CalendarAdaptor *cal = new Akonadi::CalendarAdaptor( mCalendar, parent );
    Akonadi::DndFactory factory( cal, true /*delete calendarAdaptor*/ );
    emit incidenceToBeDeleted( aitem );
    factory.cutIncidence( aitem );
    emit incidenceDeleted( aitem );
  }
  return doDelete;
}

namespace {
class YetAnotherComparisonVisitor : public IncidenceBase::Visitor
{
  public:
    YetAnotherComparisonVisitor() {}
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
}

bool IncidenceChanger::incidencesEqual( Incidence *inc1, Incidence *inc2 )
{
  YetAnotherComparisonVisitor v;
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

bool IncidenceChanger::changeIncidence( const KCal::Incidence::Ptr &oldinc,
                                        const Item &newItem,
                                        KOGlobals::WhatChanged action,
                                        QWidget *parent )
{
  const Incidence::Ptr newinc = Akonadi::incidence( newItem );

  kDebug() << "for incidence \"" << newinc->summary() << "\""
           << "( old one was \"" << oldinc->summary() << "\")";

  if ( incidencesEqual( newinc.get(), oldinc.get() ) ) {
    // Don't do anything
    kDebug() << "Incidence not changed";
  } else {
    kDebug() << "Changing incidence";
    bool attendeeStatusChanged = myAttendeeStatusChanged( oldinc.get(), newinc.get() );
    int revision = newinc->revision();
    newinc->setRevision( revision + 1 );
    // FIXME: Use a generic method for this! Ideally, have an interface class
    //        for group cheduling. Each implementation could then just do what
    //        it wants with the event. If no groupware is used,use the null
    //        pattern...
    bool success = true;
    if ( KOPrefs::instance()->mUseGroupwareCommunication ) {
      success = Akonadi::Groupware::instance()->sendICalMessage(
        parent,
        KCal::iTIPRequest,
        newinc.get(), Akonadi::Groupware::INCIDENCEEDITED, attendeeStatusChanged );
    }

    if ( !success ) {
      kDebug() << "Changing incidence failed. Reverting changes.";
      assignIncidence( newinc.get(), oldinc.get() );
      return false;
    }
  }
  return true;
}

bool IncidenceChanger::addIncidence( const KCal::Incidence::Ptr &incidence, QWidget *parent )
{
  kDebug()<<" KOPrefs::instance()->defaultCollection() :"<<KOPrefs::instance()->defaultCollection();
  const Akonadi::Collection c = Akonadi::selectCollection(parent, KOPrefs::instance()->defaultCollection());
  if ( !c.isValid() ) {
    return false;
  }
  return addIncidence( incidence, c, parent );
}

bool IncidenceChanger::addIncidence( const Incidence::Ptr &incidence, const Collection &collection, QWidget* parent )
{
  if( !incidence || !collection.isValid() ) {
    return false;
  }
  kDebug() << "\"" << incidence->summary() << "\"";

  Item item;
  item.setPayload( incidence );
  //the sub-mimetype of text/calendar as defined at kdepim/akonadi/kcal/kcalmimetypevisitor.cpp
  item.setMimeType( QString::fromLatin1("application/x-vnd.akonadi.calendar.%1").arg(QLatin1String(incidence->type().toLower())) ); //PENDING(AKONADI_PORT) shouldn't be hardcoded?
  ItemCreateJob *job = new ItemCreateJob( item, collection);
  // The connection needs to be queued to be sure addIncidenceFinished is called after the kjob finished
  // it's eventloop. That's needed cause Akonadi::Groupware uses synchron job->exec() calls.
  connect( job, SIGNAL( result(KJob*)), this, SLOT( addIncidenceFinished(KJob*) ), Qt::QueuedConnection );
  return true;
}

void IncidenceChanger::addIncidenceFinished( KJob* j ) {
  kDebug();
  const Akonadi::ItemCreateJob* job = qobject_cast<const Akonadi::ItemCreateJob*>( j );
  Q_ASSERT( job );
  Incidence::Ptr incidence = Akonadi::incidence( job->item() );

  if  ( job->error() ) {
    KMessageBox::sorry(
      0, //PENDING(AKONADI_PORT) set parent, ideally the one passed in addIncidence...
      i18n( "Unable to save %1 \"%2\": %3",
            i18n( incidence->type() ),
            incidence->summary(),
            job->errorString() ) );
    return;
  }

  Q_ASSERT( incidence );
  if ( KOPrefs::instance()->mUseGroupwareCommunication ) {
    if ( !Akonadi::Groupware::instance()->sendICalMessage(
           0, //PENDING(AKONADI_PORT) set parent, ideally the one passed in addIncidence...
           KCal::iTIPRequest,
           incidence.get(), Akonadi::Groupware::INCIDENCEADDED, false ) ) {
      kError() << "sendIcalMessage failed.";
    }
  }
}

#include "incidencechanger.moc"
