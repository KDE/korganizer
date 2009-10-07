/*
    This file is part of Akonadi.

    Copyright (c) 2009 KDAB
    Authors: Sebastian Sauer <sebsauer@kdab.net>
             Till Adam <till@kdab.net>

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

#ifndef AKONADICALENDAR_P_H
#define AKONADICALENDAR_P_H

#include "akonadicalendar.h"
#include "kohelper.h"

#include <QObject>
#include <QCoreApplication>
#include <QDBusInterface>

#include <akonadi/entity.h>
#include <akonadi/collection.h>
#include <akonadi/collectionview.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/collectiondialog.h>
#include <akonadi/item.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/agentinstance.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agenttype.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/monitor.h>
#include <akonadi/session.h>

#include <KCal/Incidence>

using namespace boost;
using namespace KCal;
using namespace KOrg;

class AkonadiCalendarCollection : public QObject
{
    Q_OBJECT
  public:
    AkonadiCalendar *m_calendar;
    Akonadi::Collection m_collection;

    AkonadiCalendarCollection(AkonadiCalendar *calendar, const Akonadi::Collection &collection)
      : QObject()
      , m_calendar(calendar)
      , m_collection(collection)
    {
    }

    ~AkonadiCalendarCollection()
    {
    }
};

class AkonadiCalendarItem
{
  public:
    Akonadi::Item m_item; //needed to keep an instance to increment shared_ptr ref-counter

    AkonadiCalendarItem(const Akonadi::Item &item)
      : m_item(item)
    {
    }

    ~AkonadiCalendarItem()
    {
    }

    KCal::Incidence::Ptr incidence() const
    {
      Q_ASSERT( m_item.hasPayload<KCal::Incidence::Ptr>() );
      return m_item.payload<KCal::Incidence::Ptr>();
    }
};

class KOrg::AkonadiCalendar::Private : public QObject
{
    Q_OBJECT
  public:
    explicit Private(AkonadiCalendar *q)
      : q(q)
      , m_monitor( new Akonadi::Monitor() )
      , m_session( new Akonadi::Session( QCoreApplication::instance()->applicationName().toUtf8() + QByteArray("-AkonadiCal-") + QByteArray::number(qrand()) ) )
      , m_incidenceBeingChanged()
    {
      m_monitor->itemFetchScope().fetchFullPayload();
      m_monitor->itemFetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
      m_monitor->ignoreSession( m_session );

      connect( m_monitor, SIGNAL(itemChanged( const Akonadi::Item&, const QSet<QByteArray>& )),
               this, SLOT(itemChanged( const Akonadi::Item&, const QSet<QByteArray>& )) );
      connect( m_monitor, SIGNAL(itemMoved( const Akonadi::Item&, const Akonadi::Collection&, const Akonadi::Collection& )),
               this, SLOT(itemMoved( const Akonadi::Item&, const Akonadi::Collection&, const Akonadi::Collection& ) ) );
      connect( m_monitor, SIGNAL(itemAdded( const Akonadi::Item&, const Akonadi::Collection& )),
               this, SLOT(itemAdded( const Akonadi::Item& )) );
      connect( m_monitor, SIGNAL(itemRemoved( const Akonadi::Item& )),
               this, SLOT(itemRemoved( const Akonadi::Item& )) );
      /*
      connect( m_monitor, SIGNAL(itemLinked(const Akonadi::Item&, const Akonadi::Collection&)),
               this, SLOT(itemAdded(const Akonadi::Item&, const Akonadi::Collection&)) );
      connect( m_monitor, SIGNAL(itemUnlinked( const Akonadi::Item&, const Akonadi::Collection& )),
               this, SLOT(itemRemoved( const Akonadi::Item&, const Akonadi::Collection& )) );
      */
    }

    ~Private()
    {
      delete m_monitor;
      delete m_session;
    }

    /*
    void clear()
    {
      kDebug();
      mEvents.clear();
      mTodos.clear();
      mJournals.clear();
      m_map.clear();
      qDeleteAll(m_itemMap);
      qDeleteAll(m_collectionMap);
    }
    */

    bool addIncidence( Incidence *incidence )
    {
      kDebug();
      Akonadi::CollectionDialog dlg( 0 );
      dlg.setMimeTypeFilter( QStringList() << QString::fromLatin1( "text/calendar" ) );
      if ( ! dlg.exec() ) {
        return false;
      }
      const Akonadi::Collection collection = dlg.selectedCollection();
      Q_ASSERT( collection.isValid() );
      //Q_ASSERT( m_collectionMap.contains( collection.id() ) ); //we can add items to collections we don't show yet
      Q_ASSERT( ! m_uidToItemId.contains( incidence->uid() ) ); //but we can not have the same incidence in 2 collections

      Akonadi::Item item;
      //the sub-mimetype of text/calendar as defined at kdepim/akonadi/kcal/kcalmimetypevisitor.cpp
      item.setMimeType( QString("application/x-vnd.akonadi.calendar.%1").arg(QString(incidence->type().toLower())) );
      KCal::Incidence::Ptr incidencePtr( incidence ); //no clone() needed
      item.setPayload<KCal::Incidence::Ptr>( incidencePtr );
      Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, collection, m_session );
      connect( job, SIGNAL( result( KJob* ) ), this, SLOT( createDone( KJob* ) ) );
      return true;
    }

    bool addIncidenceFORAKONADI( const Akonadi::Item & item_ )
    {
      kDebug();
      Akonadi::Item item = item_;
      const Incidence::Ptr incidence = KOHelper::incidence( item );
      Akonadi::CollectionDialog dlg( 0 ); //PENDING(AKONADI_PORT) we really need a parent here
      dlg.setMimeTypeFilter( QStringList() << QString::fromLatin1( "text/calendar" ) );
      if ( ! dlg.exec() ) {
        return false;
      }
      const Akonadi::Collection collection = dlg.selectedCollection();
      Q_ASSERT( collection.isValid() );
      //Q_ASSERT( m_collectionMap.contains( collection.id() ) ); //we can add items to collections we don't show yet
      Q_ASSERT( ! m_uidToItemId.contains( incidence->uid() ) ); //but we can not have the same incidence in 2 collections //PENDING(AKONADI_PORT) remove this assert (and the map)

      //the sub-mimetype of text/calendar as defined at kdepim/akonadi/kcal/kcalmimetypevisitor.cpp
      item.setMimeType( QString::fromLatin1("application/x-vnd.akonadi.calendar.%1").arg(QLatin1String(incidence->type().toLower())) ); //PENDING(AKONADI_PORT) shouldn't be hardcoded?
      Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, collection, m_session );
      connect( job, SIGNAL( result( KJob* ) ), this, SLOT( createDone( KJob* ) ) );
      return true;
    }

    bool deleteIncidence( Incidence *incidence )
    {
      kDebug();
      m_changes.removeAll( incidence->uid() ); //abort changes to this incidence cause we will just delete it
      Q_ASSERT( m_uidToItemId.contains( incidence->uid() ) );
      Akonadi::Item item = itemForUid( incidence->uid() );
      Q_ASSERT( item.isValid() );
      Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob( item, m_session );
      connect( job, SIGNAL( result( KJob* ) ), this, SLOT( deleteDone( KJob* ) ) );
      return true;
    }

    bool deleteIncidenceFORAKONADI( const Akonadi::Item &item )
    {
      kDebug();
      Incidence::Ptr incidence = KOHelper::incidence( item );
      if ( !incidence )
        return false;
      m_changes.removeAll( incidence->uid() ); //abort changes to this incidence cause we will just delete it
      Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob( item, m_session );
      connect( job, SIGNAL( result( KJob* ) ), this, SLOT( deleteDone( KJob* ) ) );
      return true;
    }

    Akonadi::Item itemForUid( const QString& uid ) const
    {
      if ( m_uidToItemId.contains( uid ) ) {
        const Akonadi::Item::Id id = m_uidToItemId.value( uid );
        Q_ASSERT( m_itemMap.contains( id ) );
        return m_itemMap.value( id );
      }
      return Akonadi::Item();
    }

    void removeIncidenceFromMultiHashByUID(const Incidence::Ptr &incidence, const QString &key)
    {
      const QList<KCal::Incidence::Ptr> values = m_incidenceForDate.values( key );
      QListIterator<KCal::Incidence::Ptr> it(values);
      while( it.hasNext() ) {
        KCal::Incidence::Ptr inc = it.next();
        if( inc->uid() == incidence->uid() ) {
          m_incidenceForDate.remove( key, inc );
        }
      }
    }

    void assertInvariants() const
    {
      Q_ASSERT(  m_itemMap.size() == m_uidToItemId.size() );
    }

    AkonadiCalendar *q;
    Akonadi::Monitor *m_monitor;
    Akonadi::Session *m_session;
    QHash<Akonadi::Entity::Id, AkonadiCalendarCollection*> m_collectionMap;
    QHash<Akonadi::Item::Id, Akonadi::Item> m_itemMap; // akonadi id to items
    QMap<QString, Akonadi::Item::Id> m_uidToItemId;
    QList<QString> m_changes; //list of Incidence->uid() that are modified atm
    KCal::Incidence::Ptr m_incidenceBeingChanged; // clone of the incidence currently being modified, for rollback and to check if something actually changed

    //CalFormat *mFormat;                    // calendar format
    //QHash<QString, Event *>mEvents;        // hash on uids of all Events
    QMultiHash<QString, KCal::Incidence::Ptr> m_incidenceForDate;// on start dates of non-recurring, single-day Incidences
//QMultiHash<QString, Event *>mEventsForDate;// on start dates of non-recurring, single-day Events
    //QHash<QString, Todo *>mTodos;          // hash on uids of all Todos
//QMultiHash<QString, Todo*>mTodosForDate;// on due dates for all Todos
    //QHash<QString, Journal *>mJournals;    // hash on uids of all Journals
//QMultiHash<QString, Journal *>mJournalsForDate; // on dates of all Journals
    //Incidence::List mDeletedIncidences;    // list of all deleted Incidences

  public Q_SLOTS:
  
    void listingDone( KJob *job )
    {
        kDebug();
        Akonadi::ItemFetchJob *fetchjob = static_cast<Akonadi::ItemFetchJob*>( job );
        if ( job->error() ) {
            kWarning( 5250 ) << "Item query failed:" << job->errorString();
            emit q->signalErrorMessage( job->errorString() );
            return;
        }
        itemsAdded( fetchjob->items() );
    }

    void agentCreated( KJob *job )
    {
        kDebug();
        Akonadi::AgentInstanceCreateJob *createjob = dynamic_cast<Akonadi::AgentInstanceCreateJob*>( job );
        if ( createjob->error() ) {
            kWarning( 5250 ) << "Agent create failed:" << createjob->errorString();
            emit q->signalErrorMessage( createjob->errorString() );
            return;
        }
        Akonadi::AgentInstance instance = createjob->instance();
        //instance.setName( CalendarName );
        QDBusInterface iface("org.freedesktop.Akonadi.Resource." + instance.identifier(), "/Settings");
        if( ! iface.isValid() ) {
            kWarning( 5250 ) << "Failed to obtain D-Bus interface for remote configuration.";
            emit q->signalErrorMessage( "Failed to obtain D-Bus interface for remote configuration." );
            return;
        }
        QString path = createjob->property("path").toString();
        Q_ASSERT( ! path.isEmpty() );
        iface.call("setPath", path);
        instance.reconfigure();
    }

    void createDone( KJob *job )
    {
        kDebug();
        if ( job->error() ) {
            kWarning( 5250 ) << "Item create failed:" << job->errorString();
            emit q->signalErrorMessage( job->errorString() );
            return;
        }
        Akonadi::ItemCreateJob *createjob = static_cast<Akonadi::ItemCreateJob*>( job );
        if ( m_collectionMap.contains( createjob->item().parentCollection().id() ) ) {
          // yes, adding to an un-viewed collection happens
          itemAdded( createjob->item() );
        } else {
          // FIXME show dialog indicating that the creation worked, but the incidence will
          // not show, since the collection isn't
          kWarning() << "Collection with id=" << createjob->item().parentCollection() << " not in m_collectionMap";
        }
    }

    void deleteDone( KJob *job )
    {
        kDebug();
        if ( job->error() ) {
            kWarning( 5250 ) << "Item delete failed:" << job->errorString();
            emit q->signalErrorMessage( job->errorString() );
            return;
        }
        Akonadi::ItemDeleteJob *deletejob = static_cast<Akonadi::ItemDeleteJob*>( job );
        itemsRemoved( deletejob->deletedItems() );
    }

    void modifyDone( KJob *job )
    {
        // we should probably update the revision number here,or internally in the Event
        // itself when certain things change. need to verify with ical documentation.

        assertInvariants();
        Akonadi::ItemModifyJob *modifyjob = static_cast<Akonadi::ItemModifyJob*>( job );
        if ( modifyjob->error() ) {
            kWarning( 5250 ) << "Item modify failed:" << job->errorString();
            emit q->signalErrorMessage( job->errorString() );
            return;
        }
        Akonadi::Item item = modifyjob->item();
        Q_ASSERT( item.hasPayload<KCal::Incidence::Ptr>() );
        const KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();
        Q_ASSERT( incidence );
        const Akonadi::Item::Id uid = item.id();
        //kDebug()<<"Old storageCollectionId="<<m_itemMap[uid]->m_item.storageCollectionId();
        kDebug() << "Item modify done uid=" << uid << "storageCollectionId=" << item.storageCollectionId();
        Q_ASSERT( m_itemMap.contains(uid) );
        Q_ASSERT( item.storageCollectionId() == m_itemMap.value(uid).storageCollectionId() ); // there was once a bug that resulted in items forget there collectionId...
        m_itemMap.insert( uid, item );
        q->notifyIncidenceChanged( incidence.get() );
        q->setModified( true );
        emit q->calendarChanged();
        assertInvariants();
    }

    void itemChanged( const Akonadi::Item& item, const QSet<QByteArray>& )
    {
        assertInvariants();
        Q_ASSERT( item.isValid() );
        Q_ASSERT( item.hasPayload<KCal::Incidence::Ptr>() );
        const KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();
        Q_ASSERT( incidence );
        const Akonadi::Item::Id uid = item.id();
        kDebug() << "Item changed uid=" << uid << "summary=" << incidence->summary() << "type=" << incidence->type() << "storageCollectionId=" << item.storageCollectionId();
        Q_ASSERT( m_itemMap.contains(uid) );
        m_itemMap.insert( uid, item );
        q->notifyIncidenceChanged( incidence.get() );
        q->setModified( true );
        emit q->calendarChanged();
        assertInvariants();
    }

    void itemMoved( const Akonadi::Item &item, const Akonadi::Collection& colSrc, const Akonadi::Collection& colDst )
    {
        kDebug();
        if( m_collectionMap.contains(colSrc.id()) && ! m_collectionMap.contains(colDst.id()) )
            itemRemoved( item );
        else if( m_collectionMap.contains(colDst.id()) && ! m_collectionMap.contains(colSrc.id()) )
            itemAdded( item );
    }

    void itemsAdded( const Akonadi::Item::List &items )
    {
        kDebug();
        assertInvariants();
        foreach( const Akonadi::Item &item, items ) {
            if ( !m_collectionMap.contains( item.parentCollection().id() ) )  // collection got deselected again in the meantime
              continue;
            Q_ASSERT( item.isValid() );
            if ( !item.hasPayload<KCal::Incidence::Ptr>() )
              continue;
            const KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();
            kDebug() << "Add akonadi id=" << item.id() << "uid=" << incidence->uid() << "summary=" << incidence->summary() << "type=" << incidence->type();
            const Akonadi::Item::Id uid = item.id();
            Q_ASSERT( ! m_itemMap.contains( uid ) ); //uh, 2 incidences with the same uid?
            Q_ASSERT( ! m_uidToItemId.contains( incidence->uid() ) ); // If this triggers, we have the same items in different collections (violates equal map size assertion in assertInvariants())
            if( const Event::Ptr e = dynamic_pointer_cast<Event>( incidence ) ) {
              if ( !e->recurs() && !e->isMultiDay() )
                m_incidenceForDate.insert( e->dtStart().date().toString(), incidence );
            } else if( const Todo::Ptr t = dynamic_pointer_cast<Todo>( incidence ) ) {
              if ( t->hasDueDate() )
                m_incidenceForDate.insert( t->dtDue().date().toString(), incidence );
            } else if( const Journal::Ptr j = dynamic_pointer_cast<Journal>( incidence ) ) {
                m_incidenceForDate.insert( j->dtStart().date().toString(), incidence );
            } else {
              Q_ASSERT(false);
              continue;
            }
    
            m_itemMap.insert( uid, item );
            m_incidenceForDate.insert( incidence->dtStart().date().toString(), incidence );
            m_uidToItemId.insert( incidence->uid(), uid );
            assertInvariants();
            incidence->registerObserver( q );
            q->notifyIncidenceAdded( incidence.get() );
        }
        q->setModified( true );
        emit q->calendarChanged();
        assertInvariants();
    }

    void itemAdded( const Akonadi::Item &item )
    {
        kDebug();
        Q_ASSERT( item.isValid() );
        if( ! m_itemMap.contains( item.id() ) ) {
          itemsAdded( Akonadi::Item::List() << item );
        }
    }

    void itemsRemoved( const Akonadi::Item::List &items )
    {
        assertInvariants();
        //kDebug()<<items.count();
        foreach(const Akonadi::Item& item, items) {
            Q_ASSERT( item.isValid() );
            Akonadi::Item ci( m_itemMap.take( item.id() ) );
            Q_ASSERT( ci.hasPayload<KCal::Incidence::Ptr>() );
            const KCal::Incidence::Ptr incidence = ci.payload<KCal::Incidence::Ptr>();
            kDebug() << "Remove uid=" << incidence->uid() << "summary=" << incidence->summary() << "type=" << incidence->type();

            if( const Event::Ptr e = dynamic_pointer_cast<Event>(incidence) ) {
              if ( !e->recurs() )
                removeIncidenceFromMultiHashByUID( incidence, e->dtStart().date().toString() );
            } else if( const Todo::Ptr t = dynamic_pointer_cast<Todo>( incidence ) ) {
              if ( t->hasDueDate() )
                removeIncidenceFromMultiHashByUID( incidence, t->dtDue().date().toString() );
            } else if( const Journal::Ptr j = dynamic_pointer_cast<Journal>( incidence ) ) {
              removeIncidenceFromMultiHashByUID( incidence, j->dtStart().date().toString() );
            } else {
              Q_ASSERT(false);
              continue;
            }

            //incidence->unregisterObserver( q );
            q->notifyIncidenceDeleted( incidence.get() );
            m_uidToItemId.remove( incidence->uid() );
        }
        q->setModified( true );
        emit q->calendarChanged();
        assertInvariants();
    }

    void itemRemoved( const Akonadi::Item &item )
    {
        kDebug();
        itemsRemoved( Akonadi::Item::List() << item );
    }
  
};

#endif
