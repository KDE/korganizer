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

#ifndef AKONADICALENDAR_H
#define AKONADICALENDAR_H

#include "korganizer/korganizer_export.h"
#include <kcal/calendar.h>

namespace Akonadi {
  class Collection;
  class Item;
}

namespace KCal {
    class CalFormat;
}

namespace KOrg {

/**
 * Implements a KCal::Calendar that uses Akonadi as backend.
 */
class KORGANIZER_INTERFACES_EXPORT AkonadiCalendar : public KCal::Calendar
{
    Q_OBJECT
  public:
    explicit AkonadiCalendar( const KDateTime::Spec &timeSpec );
    ~AkonadiCalendar();

    bool hasCollection( const Akonadi::Collection &collection ) const;
    void addCollection( const Akonadi::Collection &collection );
    void removeCollection( const Akonadi::Collection &collection );

    Akonadi::Item itemForIncidence(KCal::Incidence *incidence) const;

    bool beginChange( KCal::Incidence *incidence );
    bool endChange( KCal::Incidence *incidence );

    bool reload(); //TODO remove, atm abstract in Calendar
    bool save(); //TODO remove, atm abstract in Calendar
    void close(); //TODO remove, atm abstract in Calendar

    bool addAgent( const KUrl &mUrl );
    bool addIncidence( KCal::Incidence *incidence );
    bool deleteIncidence( KCal::Incidence *incidence );
    void incidenceUpdated( KCal::IncidenceBase *incidenceBase );

    bool addEvent( KCal::Event *event );
    bool deleteEvent( KCal::Event *event );
    void deleteAllEvents() { Q_ASSERT(false); } //TODO remove, atm abstract in Calendar

    KCal::Event::List rawEvents( KCal::EventSortField sortField = KCal::EventSortUnsorted, KCal::SortDirection sortDirection = KCal::SortDirectionAscending );
    KCal::Event::List rawEvents( const QDate &start, const QDate &end, const KDateTime::Spec &timeSpec = KDateTime::Spec(), bool inclusive = false );
    KCal::Event::List rawEventsForDate( const QDate &date, const KDateTime::Spec &timeSpec = KDateTime::Spec(), KCal::EventSortField sortField = KCal::EventSortUnsorted, KCal::SortDirection sortDirection = KCal::SortDirectionAscending );
    KCal::Event::List rawEventsForDate( const KDateTime &dt );

    KCal::Event *event( const QString &uid );

    bool addTodo( KCal::Todo *todo );
    bool deleteTodo( KCal::Todo *todo );
    void deleteAllTodos() { Q_ASSERT(false); } //TODO remove, atm abstract in Calendar

    KCal::Todo::List rawTodos( KCal::TodoSortField sortField = KCal::TodoSortUnsorted, KCal::SortDirection sortDirection = KCal::SortDirectionAscending );
    KCal::Todo::List rawTodosForDate( const QDate &date );

    KCal::Todo *todo( const QString &uid );

    bool addJournal( KCal::Journal *journal );
    bool deleteJournal( KCal::Journal *journal );
    void deleteAllJournals() { Q_ASSERT(false); } //TODO remove, atm abstract in Calendar

    KCal::Journal::List rawJournals( KCal::JournalSortField sortField = KCal::JournalSortUnsorted, KCal::SortDirection sortDirection = KCal::SortDirectionAscending );
    KCal::Journal::List rawJournalsForDate( const QDate &date );

    KCal::Journal *journal( const QString &uid );

    KCal::Alarm::List alarms( const KDateTime &from, const KDateTime &to );
    KCal::Alarm::List alarmsTo( const KDateTime &to );

    using QObject::event;   // prevent warning about hidden virtual method

  public Q_SLOTS:
    void deleteIncidenceProxyMethod( KCal::Incidence *incidence ) { deleteIncidence(incidence); }

  Q_SIGNALS:
    void signalErrorMessage( const QString& );
    // Same signals Akonadi::Monitor provides to allow later to refactor code to
    // use Collection+Monitor+etc direct rather then the AkonadiCalendar class.
    /*
    void itemChanged( const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers );
    void itemMoved( const Akonadi::Item &item, const Akonadi::Collection &collectionSource, const Akonadi::Collection &collectionDestination );
    void itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection );
    void itemRemoved( const Akonadi::Item &item );
    void itemLinked( const Akonadi::Item &item, const Akonadi::Collection &collection );
    void itemUnlinked( const Akonadi::Item &item, const Akonadi::Collection &collection );
    void collectionAdded( const Akonadi::Collection &collection, const Akonadi::Collection &parent );
    void collectionChanged( const Akonadi::Collection &collection );
    void collectionRemoved( const Akonadi::Collection &collection );
    void collectionStatisticsChanged( Akonadi::Collection::Id id, const Akonadi::CollectionStatistics &statistics );
    void collectionMonitored( const Akonadi::Collection &collection, bool monitored );
    void itemMonitored( const Akonadi::Item &item, bool monitored );
    void resourceMonitored( const QByteArray &identifier, bool monitored );
    void mimeTypeMonitored( const QString &mimeType, bool monitored );
    void allMonitored( bool monitored );
    */

  private:
    Q_DISABLE_COPY( AkonadiCalendar )
    class Private;
    Private *const d;
};

}

#endif
