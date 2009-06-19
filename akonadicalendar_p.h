/*
    This file is part of Akonadi.

    Copyright (c) 2009 Sebastian Sauer <sebsauer@kdab.net>

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

#include <QObject>
#include <QCoreApplication>

#include <akonadi/collection.h>
#include <akonadi/collectionview.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/monitor.h>
#include <akonadi/session.h>

#include <KCal/Incidence>

using namespace KCal;

// to provide access to the Item::List instance
class MyAkonadiItemDeleteJob : public Akonadi::ItemDeleteJob {
    public:
        MyAkonadiItemDeleteJob(const Akonadi::Item &item) : Akonadi::ItemDeleteJob(item), m_items(Akonadi::Item::List() << item) {}
        MyAkonadiItemDeleteJob(const Akonadi::Item::List &items) : Akonadi::ItemDeleteJob(items), m_items(items) {}
        Akonadi::Item::List items() const { return m_items; }
    private:
        Akonadi::Item::List m_items;
};

class KCal::AkonadiCalendar::Private : public QObject
{
    Q_OBJECT
  public:
    explicit Private(AkonadiCalendar *q)
      : q(q)
      , m_monitor( new Akonadi::Monitor() )
      , m_session( new Akonadi::Session( QCoreApplication::instance()->applicationName().toUtf8() + QByteArray("-AkonadiCalendar-") + QByteArray::number(qrand()) ) )
    {
      m_monitor->ignoreSession( m_session );
      m_monitor->itemFetchScope().fetchFullPayload();

      connect( m_monitor, SIGNAL(itemChanged( const Akonadi::Item&, const QSet<QByteArray>& )),
               this, SLOT(itemChanged( const Akonadi::Item&, const QSet<QByteArray>& )) );
      connect( m_monitor, SIGNAL(itemMoved( const Akonadi::Item&, const Akonadi::Collection&, const Akonadi::Collection& )),
               this, SLOT(itemMoved( const Akonadi::Item&, const Akonadi::Collection&, const Akonadi::Collection& ) ) );
      connect( m_monitor, SIGNAL(itemAdded( const Akonadi::Item&, const Akonadi::Collection& )),
               this, SLOT(itemAdded( const Akonadi::Item& )) );
      connect( m_monitor, SIGNAL(itemRemoved(Akonadi::Item)),
               this, SLOT(itemRemoved(Akonadi::Item)) );
      connect( m_monitor, SIGNAL(itemLinked(const Akonadi::Item&, const Akonadi::Collection&)),
               this, SLOT(itemAdded(const Akonadi::Item&)) );
      connect( m_monitor, SIGNAL(itemUnlinked(const Akonadi::Item&, const Akonadi::Collection&)),
               this, SLOT(itemRemoved(const Akonadi::Item&)) );
    }

    ~Private()
    {
      delete m_monitor;
      delete m_session;
    }

    void clear()
    {
      mEvents.clear();
      mTodos.clear();
      mJournals.clear();
      m_items.clear();
    }

#if 0
    CalFormat *mFormat;                    // calendar format
#endif
    QHash<QString, Event *>mEvents;        // hash on uids of all Events
#if 0
    QMultiHash<QString, Event *>mEventsForDate;// on start dates of non-recurring, single-day Events
#endif
    QHash<QString, Todo *>mTodos;          // hash on uids of all Todos
#if 0
    QMultiHash<QString, Todo*>mTodosForDate;// on due dates for all Todos
#endif
    QHash<QString, Journal *>mJournals;    // hash on uids of all Journals
#if 0
    QMultiHash<QString, Journal *>mJournalsForDate; // on dates of all Journals
    Incidence::List mDeletedIncidences;    // list of all deleted Incidences
#endif

    AkonadiCalendar *q;
    Akonadi::Monitor *m_monitor;
    Akonadi::Session *m_session;
    Akonadi::Collection m_collection;

    Akonadi::Item::List m_items;

  public Q_SLOTS:
  
    void listingDone( KJob *job )
    {
        kDebug();
        //Akonadi::ItemFetchJob *fetchjob = static_cast<Akonadi::ItemFetchJob*>( job );
        if ( job->error() ) {
            kWarning( 5250 ) << "Item query failed:" << job->errorString();
            return;
        }
        emit q->calendarChanged();
    }

    void deletionDone( KJob *job )
    {
        kDebug();
        if ( job->error() ) {
            kWarning( 5250 ) << "Item delete failed:" << job->errorString();
            return;
        }
        MyAkonadiItemDeleteJob *deletejob = static_cast<MyAkonadiItemDeleteJob*>( job );

        foreach(const Akonadi::Item& item, deletejob->items()) {
          const KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();
    
          mEvents.remove(incidence->uid());
          mTodos.remove(incidence->uid());
          mJournals.remove( incidence->uid());
          m_items.removeAll(item);
        }

        emit q->calendarChanged();
    }

    void itemChanged( const Akonadi::Item &item, const QSet<QByteArray>& )
    {
        kDebug();
#if 0
        int row = rowForItem( item );
        if ( row < 0 ) return;
        items[ row ]->item = item;
        itemHash.remove( item );
        itemHash[ item ] = items[ row ];
        QModelIndex start = mParent->index( row, 0, QModelIndex() );
        QModelIndex end = mParent->index( row, mParent->columnCount( QModelIndex() ) - 1 , QModelIndex() );
        mParent->dataChanged( start, end );
#endif
    }

    void itemMoved( const Akonadi::Item &item, const Akonadi::Collection& colSrc, const Akonadi::Collection& colDst )
    {
        kDebug();
        if ( colSrc == m_collection && colDst != m_collection )
            itemRemoved( item );
        else if ( colDst == m_collection && colSrc != m_collection )
            itemAdded( item );
    }

    void itemsAdded( const Akonadi::Item::List &list )
    {
        kDebug();
        foreach( const Akonadi::Item &item, list ) {
            Q_ASSERT( item.isValid() );
            Q_ASSERT( item.hasPayload() );
            const KCal::Incidence::Ptr incidence = item.payload<KCal::Incidence::Ptr>();
            kDebug() << "uid=" << incidence->uid() << "summary=" << incidence->summary() << "type=" << incidence->type();
            q->addIncidence( incidence.get() ); //dispatches to addEvent/addToDo/addJournal
        }
        m_items << list; //keep instance to increment shared_ptr ref-counter
    }

    void itemAdded( const Akonadi::Item &item )
    {
        kDebug();
        itemsAdded( Akonadi::Item::List() << item );
    }

    void itemRemoved( const Akonadi::Item &_item )
    {
        kDebug();
#if 0
        int row = rowForItem( _item );
        if ( row < 0 )
            return;
        mParent->beginRemoveRows( QModelIndex(), row, row );
        const Item item = items.at( row )->item;
        Q_ASSERT( item.isValid() );
        itemHash.remove( item );
        delete items.takeAt( row );
        mParent->endRemoveRows();
#endif
    }
  
};

#endif
