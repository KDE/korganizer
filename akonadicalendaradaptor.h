/*
    This file is part of Akonadi.

    Copyright (c) 2009 KDAB
    Author: Sebastian Sauer <sebsauer@kdab.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#ifndef AKONADICALENDARADAPTOR_H
#define AKONADICALENDARADAPTOR_H

#include "korganizer_export.h"

#include "kogroupware.h"
#include "koprefs.h"
#include "mailscheduler.h"

#include <akonadi/kcal/akonadicalendar.h>
#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemDeleteJob>
#include <Akonadi/Collection>
#include <akonadi/kcal/utils.h>

#include <KCal/CalendarResources>
#include <KCal/CalFilter>
#include <KCal/DndFactory>
#include <KCal/FileStorage>
#include <KCal/FreeBusy>
#include <KCal/ICalDrag>
#include <KCal/ICalFormat>
#include <KCal/VCalFormat>
#include <kcal/listbase.h>

#include <KDE/KMessageBox>

namespace KOrg {

template<class T> inline T* itemToIncidence(const Akonadi::Item &item) {
  if ( !item.hasPayload< boost::shared_ptr<T> >() )
    return 0;
  return item.payload< boost::shared_ptr<T> >().get();
}

template<class T> inline KCal::ListBase<T> itemsToIncidences(QList<Akonadi::Item> items) {
  KCal::ListBase<T> list;
  foreach(const Akonadi::Item &item, items)
    list.append( itemToIncidence<T>(item) );
  return list;
}

template<class T> inline Akonadi::Item incidenceToItem(T* incidence) {
  Akonadi::Item item;
  item.setPayload< boost::shared_ptr<T> >( boost::shared_ptr<T>(incidence->clone()) );
  return item;
}

//FIXME: Find a way to set a QWidget parent for user interaction
//Probably an extra member with a setter
class KORGANIZER_CORE_EXPORT AkonadiCalendarAdaptor : public KCal::Calendar
{
  Q_OBJECT

  public:
    explicit AkonadiCalendarAdaptor(AkonadiCalendar *calendar)
      : KCal::Calendar( KOPrefs::instance()->timeSpec() )
      , mCalendar( calendar )
    {
      Q_ASSERT(mCalendar);
    }

    virtual ~AkonadiCalendarAdaptor();

    virtual bool save() { kDebug(); return true; } //unused
    virtual bool reload() { kDebug(); return true; } //unused
    virtual void close() { kDebug(); } //unused

    virtual bool addEvent( Event *event )
    {
      return addIncidence( Incidence::Ptr( event->clone() ) );
    }

    virtual bool deleteEvent( Event *event )
    {
      return deleteIncidence( incidenceToItem( event ) );
    }

    virtual void deleteAllEvents() { Q_ASSERT(false); } //unused

    virtual Event::List rawEvents(KCal::EventSortField sortField = KCal::EventSortUnsorted, KCal::SortDirection sortDirection = KCal::SortDirectionAscending )
    {
      return itemsToIncidences<Event>( mCalendar->rawEvents( (KOrg::EventSortField)sortField, (KOrg::SortDirection)sortDirection ) );
    }

    virtual Event::List rawEventsForDate( const KDateTime &dt )
    {
      return itemsToIncidences<Event>( mCalendar->rawEventsForDate( dt ) );
    }

    virtual Event::List rawEvents( const QDate &start, const QDate &end, const KDateTime::Spec &timeSpec = KDateTime::Spec(), bool inclusive = false )
    {
      return itemsToIncidences<Event>( mCalendar->rawEvents( start, end, timeSpec, inclusive ) );
    }

    virtual Event::List rawEventsForDate( const QDate &date, const KDateTime::Spec &timeSpec = KDateTime::Spec(), KCal::EventSortField sortField = KCal::EventSortUnsorted, KCal::SortDirection sortDirection = KCal::SortDirectionAscending )
    {
      return itemsToIncidences<Event>( mCalendar->rawEventsForDate( date, timeSpec, (KOrg::EventSortField)sortField, (KOrg::SortDirection)sortDirection ) );
    }

    virtual Event *event( const QString &uid )
    {
      return itemToIncidence<Event>( mCalendar->event( mCalendar->itemIdForIncidenceUid(uid) ) );
    }

    virtual bool addTodo( Todo *todo )
    {
      return addIncidence( Incidence::Ptr( todo->clone() ) );
    }

    virtual bool deleteTodo( Todo *todo )
    {
      return deleteIncidence( incidenceToItem( todo ) );
    }

    virtual void deleteAllTodos() { Q_ASSERT(false); } //unused

    virtual Todo::List rawTodos( KCal::TodoSortField sortField = KCal::TodoSortUnsorted, KCal::SortDirection sortDirection = KCal::SortDirectionAscending )
    {
      return itemsToIncidences<Todo>( mCalendar->rawTodos( (KOrg::TodoSortField)sortField, (KOrg::SortDirection)sortDirection ) );
    }

    virtual Todo::List rawTodosForDate( const QDate &date )
    {
      return itemsToIncidences<Todo>( mCalendar->rawTodosForDate( date ) );
    }

    virtual Todo *todo( const QString &uid )
    {
      return itemToIncidence<Todo>( mCalendar->todo( mCalendar->itemIdForIncidenceUid(uid) ) );
    }

    virtual bool addJournal( Journal *journal )
    {
      return addIncidence( Incidence::Ptr( journal->clone() ) );
    }

    virtual bool deleteJournal( Journal *journal )
    {
      return deleteIncidence( incidenceToItem( journal ) );
    }

    virtual void deleteAllJournals() { Q_ASSERT(false); } //unused

    virtual Journal::List rawJournals( KCal::JournalSortField sortField = KCal::JournalSortUnsorted, KCal::SortDirection sortDirection = KCal::SortDirectionAscending )
    {
      return itemsToIncidences<Journal>( mCalendar->rawJournals( (KOrg::JournalSortField)sortField, (KOrg::SortDirection)sortDirection ) );
    }

    virtual Journal::List rawJournalsForDate( const QDate &dt )
    {
      return itemsToIncidences<Journal>( mCalendar->rawJournalsForDate( dt ) );
    }

    virtual Journal *journal( const QString &uid )
    {
      return itemToIncidence<Journal>( mCalendar->journal( mCalendar->itemIdForIncidenceUid(uid) ) );
    }

    virtual Alarm::List alarms( const KDateTime &from, const KDateTime &to )
    {
      return mCalendar->alarms( from, to );
    }


    // From IncidenceChanger
    bool addIncidence( const Incidence::Ptr &incidence )
    {
      if( !incidence )
        return false;

      Akonadi::Collection collection = Akonadi::selectCollection( 0 );
      if ( !collection.isValid() )
        return false;
      kDebug() << "\"" << incidence->summary() << "\"";

      Akonadi::Item item;
      item.setPayload( incidence );
      //the sub-mimetype of text/calendar as defined at kdepim/akonadi/kcal/kcalmimetypevisitor.cpp
      item.setMimeType( QString::fromLatin1("application/x-vnd.akonadi.calendar.%1").arg(QLatin1String(incidence->type().toLower())) ); //PENDING(AKONADI_PORT) shouldn't be hardcoded?
      Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, collection );
      // The connection needs to be queued to be sure addIncidenceFinished is called after the kjob finished
      // it's eventloop. That's needed cause KOGroupware uses synchron job->exec() calls.
      connect( job, SIGNAL( result(KJob*)), this, SLOT( addIncidenceFinished(KJob*) ), Qt::QueuedConnection );
      return true;
    }

    bool deleteIncidence( const Akonadi::Item &aitem )
    {
      const Incidence::Ptr incidence = Akonadi::incidence( aitem );
      if ( !incidence ) {
        return true;
      }

      kDebug() << "\"" << incidence->summary() << "\"";
      bool doDelete = sendGroupwareMessage( aitem, KCal::iTIPCancel,
                                            KOGlobals::INCIDENCEDELETED );
      if( !doDelete )
        return false;
      Akonadi::ItemDeleteJob* job = new Akonadi::ItemDeleteJob( aitem );
      connect( job, SIGNAL(result(KJob*)), this, SLOT(deleteIncidenceFinished(KJob*)) );
      return true;
    }

  private slots:
    void addIncidenceFinished( KJob* j ) {
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
        if ( !KOGroupware::instance()->sendICalMessage(
               0, //PENDING(AKONADI_PORT) set parent, ideally the one passed in addIncidence...
               KCal::iTIPRequest,
               incidence.get(), KOGlobals::INCIDENCEADDED, false ) ) {
          kError() << "sendIcalMessage failed.";
        }
      }
    }

    void deleteIncidenceFinished( KJob* j )
    {
      kDebug();
      const Akonadi::ItemDeleteJob* job = qobject_cast<const Akonadi::ItemDeleteJob*>( j );
      Q_ASSERT( job );
      const Akonadi::Item::List items = job->deletedItems();
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

        if ( !KOGroupware::instance()->doNotNotify() && notifyOrganizer ) {
          MailScheduler scheduler( static_cast<AkonadiCalendar*>(mCalendar) );
          scheduler.performTransaction( tmp.get(), KCal::iTIPReply );
        }
        //reset the doNotNotify flag
        KOGroupware::instance()->setDoNotNotify( false );
      }
    }

  private:
    bool sendGroupwareMessage( const Akonadi::Item &aitem,
                               KCal::iTIPMethod method,
                               KOGlobals::HowChanged action )
    {
      const Incidence::Ptr incidence = Akonadi::incidence( aitem );
      if ( !incidence )
        return false;
      if ( KOPrefs::instance()->thatIsMe( incidence->organizer().email() ) &&
           incidence->attendeeCount() > 0 &&
           !KOPrefs::instance()->mUseGroupwareCommunication ) {
        schedule( method, aitem );
        return true;
      } else if ( KOPrefs::instance()->mUseGroupwareCommunication ) {
        return
          KOGroupware::instance()->sendICalMessage( 0, method, incidence.get(), action,  false );
      }
      return true;
    }

    //Coming from CalendarView
    void schedule( iTIPMethod method, const Akonadi::Item &item )
    {
      Incidence::Ptr incidence = Akonadi::incidence( item );

      if ( incidence->attendeeCount() == 0 && method != iTIPPublish ) {
        KMessageBox::information( 0, i18n( "The item has no attendees." ), "ScheduleNoIncidences" );
        return;
      }

      Incidence *inc = incidence->clone();
      inc->registerObserver( 0 );
      inc->clearAttendees();

      // Send the mail
      MailScheduler scheduler( mCalendar );
      if ( scheduler.performTransaction( incidence.get(), method ) ) {
        KMessageBox::information( 0,
                                  i18n( "The groupware message for item '%1' "
                                        "was successfully sent.\nMethod: %2",
                                        incidence->summary(),
                                        Scheduler::methodName( method ) ),
                                  i18n( "Sending Free/Busy" ),
                                  "FreeBusyPublishSuccess" );
      } else {
        KMessageBox::error( 0,
                            i18nc( "Groupware message sending failed. "
                                   "%2 is request/reply/add/cancel/counter/etc.",
                                   "Unable to send the item '%1'.\nMethod: %2",
                                   incidence->summary(),
                                   Scheduler::methodName( method ) ) );
      }
    }

    AkonadiCalendar *mCalendar;

};

}

#endif
