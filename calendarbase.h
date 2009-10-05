/*
  This file is part of the kcal library.

  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (c) 2001,2003,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2006 David Jarvie <software@astrojar.org.uk>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Calendar class.

  @author Preston Brown \<pbrown@kde.org\>
  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
  @author David Jarvie \<software@astrojar.org.uk\>
 */

#ifndef KORGANIZER_CALENDAR_H
#define KORGANIZER_CALENDAR_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMultiHash>

#include <kdatetime.h>

#include <kcal/customproperties.h>
#include <kcal/event.h>
#include <kcal/todo.h>
#include <kcal/journal.h>

namespace KCal {
    class ICalTimeZone;
    class ICalTimeZones;
    class CalFilter;
    class Person;
}

namespace KOrganizer {

/**
  Calendar KCal::Incidence sort directions.
*/
enum SortDirection {
  SortDirectionAscending,  /**< Sort in ascending order (first to last) */
  SortDirectionDescending  /**< Sort in descending order (last to first) */
};

/**
  Calendar Event sort keys.
*/
enum EventSortField {
  EventSortUnsorted,       /**< Do not sort Events */
  EventSortStartDate,      /**< Sort Events chronologically, by start date */
  EventSortEndDate,        /**< Sort Events chronologically, by end date */
  EventSortSummary         /**< Sort Events alphabetically, by summary */
};

/**
  Calendar Todo sort keys.
*/
enum TodoSortField {
  TodoSortUnsorted,        /**< Do not sort Todos */
  TodoSortStartDate,       /**< Sort Todos chronologically, by start date */
  TodoSortDueDate,         /**< Sort Todos chronologically, by due date */
  TodoSortPriority,        /**< Sort Todos by priority */
  TodoSortPercentComplete, /**< Sort Todos by percentage completed */
  TodoSortSummary          /**< Sort Todos alphabetically, by summary */
};

/**
  Calendar Journal sort keys.
*/
enum JournalSortField {
  JournalSortUnsorted,     /**< Do not sort Journals */
  JournalSortDate,         /**< Sort Journals chronologically by date */
  JournalSortSummary       /**< Sort Journals alphabetically, by summary */
};

/**
  @brief
  Represents the main calendar class.

  A calendar contains information like incidences (events, to-dos, journals),
  alarms, time zones, and other useful information.

  This is an abstract base class defining the interface to a calendar.
  It is implemented by subclasses like CalendarLocal, which use different
  methods to store and access the data.

  <b>Ownership of KCal::Incidences</b>:

  KCal::Incidence ownership is handled by the following policy: as soon as an
  incidence (or any other subclass of KCal::IncidenceBase) is added to the
  Calendar by an add...() method it is owned by the Calendar object.
  The Calendar takes care of deleting the incidence using the delete...()
  methods. All KCal::Incidences returned by the query functions are returned
  as pointers so that changes to the returned KCal::Incidences are immediately
  visible in the Calendar.  Do <em>Not</em> attempt to 'delete' any KCal::Incidence
  object you get from Calendar -- use the delete...() methods.
*/
class CalendarBase : public QObject, public KCal::CustomProperties,
                     public KCal::IncidenceBase::IncidenceObserver
{
  Q_OBJECT

  public:

    /**
      Constructs a calendar with a specified time zone @p timeZoneid.
      The time specification is used as the default for creating or
      modifying incidences in the Calendar. The time specification does
      not alter existing incidences.

      The constructor also calls setViewTimeSpec(@p timeSpec).

      @param timeSpec time specification
    */
    explicit CalendarBase( const KDateTime::Spec &timeSpec );

    /**
      Construct Calendar object using a time zone ID.
      The time zone ID is used as the default for creating or modifying
      incidences in the Calendar. The time zone does not alter existing
      incidences.

      The constructor also calls setViewTimeZoneId(@p timeZoneId).

      @param timeZoneId is a string containing a time zone ID, which is
      assumed to be valid.  If no time zone is found, the viewing time
      specification is set to local clock time.
      @e Example: "Europe/Berlin"
    */
    explicit CalendarBase( const QString &timeZoneId );

    /**
      Destroys the calendar.
    */
    virtual ~CalendarBase();

    /**
      Sets the calendar Product ID to @p id.

      @param id is a string containing the Product ID.

      @see productId() const
    */
    void setProductId( const QString &id );

    /**
      Returns the calendar's Product ID.

      @see setProductId()
    */
    QString productId() const;

    /**
      Sets the owner of the calendar to @p owner.

      @param owner is a Person object.

      @see owner()
    */
    void setOwner( const KCal::Person &owner );

    /**
      Returns the owner of the calendar.

      @return the owner Person object.

      @see setOwner()
    */
    KCal::Person owner() const;

    /**
      Sets the default time specification (time zone, etc.) used for creating
      or modifying incidences in the Calendar.

      The method also calls setViewTimeSpec(@p timeSpec).

      @param timeSpec time specification
    */
    void setTimeSpec( const KDateTime::Spec &timeSpec );

    /**
       Get the time specification (time zone etc.) used for creating or
       modifying incidences in the Calendar.

       @return time specification
    */
    KDateTime::Spec timeSpec() const;

    /**
      Sets the time zone ID used for creating or modifying incidences in the
      Calendar. This method has no effect on existing incidences.

      The method also calls setViewTimeZoneId(@p timeZoneId).

      @param timeZoneId is a string containing a time zone ID, which is
      assumed to be valid. The time zone ID is used to set the time zone
      for viewing KCal::Incidence date/times. If no time zone is found, the
      viewing time specification is set to local clock time.
      @e Example: "Europe/Berlin"
      @see setTimeSpec()
    */
    void setTimeZoneId( const QString &timeZoneId );

    /**
      Returns the time zone ID used for creating or modifying incidences in
      the calendar.

      @return the string containing the time zone ID, or empty string if the
              creation/modification time specification is not a time zone.
    */
    QString timeZoneId() const;

    /**
      Notes the time specification which the client application intends to
      use for viewing the incidences in this calendar. This is simply a
      convenience method which makes a note of the new time zone so that
      it can be read back by viewTimeSpec(). The client application must
      convert date/time values to the desired time zone itself.

      The time specification is not used in any way by the Calendar or its
      incidences; it is solely for use by the client application.

      @param timeSpec time specification

      @see viewTimeSpec()
    */
    void setViewTimeSpec( const KDateTime::Spec &timeSpec ) const;

    /**
      Notes the time zone Id which the client application intends to use for
      viewing the incidences in this calendar. This is simply a convenience
      method which makes a note of the new time zone so that it can be read
      back by viewTimeId(). The client application must convert date/time
      values to the desired time zone itself.

      The Id is not used in any way by the Calendar or its incidences.
      It is solely for use by the client application.

      @param timeZoneId is a string containing a time zone ID, which is
      assumed to be valid. The time zone ID is used to set the time zone
      for viewing KCal::Incidence date/times. If no time zone is found, the
      viewing time specification is set to local clock time.
      @e Example: "Europe/Berlin"

      @see viewTimeZoneId()
    */
    void setViewTimeZoneId( const QString &timeZoneId ) const;

    /**
      Returns the time specification used for viewing the incidences in
      this calendar. This simply returns the time specification last
      set by setViewTimeSpec().
      @see setViewTimeSpec().
    */
    KDateTime::Spec viewTimeSpec() const;

    /**
      Returns the time zone Id used for viewing the incidences in this
      calendar. This simply returns the time specification last set by
      setViewTimeSpec().
      @see setViewTimeZoneId().
    */
    QString viewTimeZoneId() const;

    /**
      Shifts the times of all incidences so that they appear at the same clock
      time as before but in a new time zone. The shift is done from a viewing
      time zone rather than from the actual incidence time zone.

      For example, shifting an incidence whose start time is 09:00 America/New York,
      using an old viewing time zone (@p oldSpec) of Europe/London, to a new time
      zone (@p newSpec) of Europe/Paris, will result in the time being shifted
      from 14:00 (which is the London time of the incidence start) to 14:00 Paris
      time.

      @param oldSpec the time specification which provides the clock times
      @param newSpec the new time specification

      @see isLocalTime()
    */
    void shiftTimes( const KDateTime::Spec &oldSpec, const KDateTime::Spec &newSpec );

    /**
      Returns the time zone collection used by the calendar.

      @return the time zones collection.

      @see setLocalTime()
    */
    KCal::ICalTimeZones *timeZones() const;

    /**
       Set the time zone collection used by the calendar.

       @param zones time zones collection. Important: all time zones references
                    in the calendar must be included in the collection.
     */
    void setTimeZones( const KCal::ICalTimeZones &zones );

    /**
      Sets if the calendar has been modified.

      @param modified is true if the calendar has been modified since open
      or last save.

      @see isModified()
    */
    void setModified( bool modified );

    /**
      Determine the calendar's modification status.

      @return true if the calendar has been modified since open or last save.

      @see setModified()
    */
    bool isModified() const;

    /**
      Clears out the current calendar, freeing all used memory etc.
    */
    virtual void close() = 0;

    /**
      Syncs changes in memory to persistent storage.

      @return true if the save was successful; false otherwise.
    */
    virtual bool save() = 0;

    /**
      Loads the calendar contents from storage. This requires that the
      calendar has been previously loaded (initialized).

      @return true if the reload was successful; otherwise false.
    */
    virtual bool reload() = 0;

    /**
      Determine if the calendar is currently being saved.

      @return true if the calendar is currently being saved; false otherwise.
    */
    virtual bool isSaving();

    /**
      Returns a list of all categories used by KCal::Incidences in this Calendar.

      @return a QStringList containing all the categories.
    */
    QStringList categories();

  // KCal::Incidence Specific Methods //

    /**
      Inserts an KCal::Incidence into the calendar.

      @param incidence is a pointer to the KCal::Incidence to insert.

      @return true if the KCal::Incidence was successfully inserted; false otherwise.

      @see deleteIncidence()
    */
    virtual bool addIncidence( KCal::Incidence *incidence );

    /**
      Removes an KCal::Incidence from the calendar.

      @param incidence is a pointer to the KCal::Incidence to remove.

      @return true if the KCal::Incidence was successfully removed; false otherwise.

      @see addIncidence()
    */
    virtual bool deleteIncidence( KCal::Incidence *incidence );

    /**
      Returns a filtered list of all KCal::Incidences for this Calendar.

      @return the list of all filtered KCal::Incidences.
    */
    virtual KCal::Incidence::List incidences();

    /**
      Returns a filtered list of all KCal::Incidences which occur on the given date.

      @param date request filtered KCal::Incidence list for this QDate only.

      @return the list of filtered KCal::Incidences occurring on the specified date.
    */
    virtual KCal::Incidence::List incidences( const QDate &date );

    /**
      Returns an unfiltered list of all KCal::Incidences for this Calendar.

      @return the list of all unfiltered KCal::Incidences.
    */
    virtual KCal::Incidence::List rawIncidences();

    /**
      Returns the KCal::Incidence associated with the given unique identifier.

      @param uid is a unique identifier string.

      @return a pointer to the KCal::Incidence.
      A null pointer is returned if no such KCal::Incidence exists.
    */
    KCal::Incidence *incidence( const QString &uid );

    /**
      Returns the KCal::Incidence associated with the given scheduling identifier.

      @param sid is a unique scheduling identifier string.

      @return a pointer to the KCal::Incidence.
      A null pointer is returned if no such KCal::Incidence exists.
    */
    KCal::Incidence *incidenceFromSchedulingID( const QString &sid );

    /**
      Searches all events and todos for an incidence with this
      scheduling identifiere. Returns a list of matching results.

      @param sid is a unique scheduling identifier string.
     */
    KCal::Incidence::List incidencesFromSchedulingID( const QString &sid );

    /**
      Create a merged list of KCal::Events, KCal::Todos, and KCal::Journals.

      @param events is an KCal::Event list to merge.
      @param todos is a KCal::Todo list to merge.
      @param journals is a KCal::Journal list to merge.

      @return a list of merged KCal::Incidences.
    */
    static KCal::Incidence::List mergeIncidenceList( const KCal::Event::List &events,
                                               const KCal::Todo::List &todos,
                                               const KCal::Journal::List &journals );

    /**
      Flag that a change to a Calendar KCal::Incidence is starting.

      @param incidence is a pointer to the KCal::Incidence that will be changing.
    */
    virtual bool beginChange( KCal::Incidence *incidence );

    /**
      Flag that a change to a Calendar KCal::Incidence has completed.

      @param incidence is a pointer to the KCal::Incidence that was changed.
    */
    virtual bool endChange( KCal::Incidence *incidence );

    /**
      Dissociate an KCal::Incidence from a recurring KCal::Incidence.
      By default, only one single KCal::Incidence for the specified @a date
      will be dissociated and returned.  If @a single is false, then
      the recurrence will be split at @a date, the old KCal::Incidence will
      have its recurrence ending at @a date and the new KCal::Incidence
      will have all recurrences past the @a date.

      @param incidence is a pointer to a recurring KCal::Incidence.
      @param date is the QDate within the recurring KCal::Incidence on which
      the dissociation will be performed.
      @param spec is the spec in which the @a date is formulated.
      @param single is a flag meaning that a new KCal::Incidence should be created
      from the recurring KCal::Incidences after @a date.

      @return a pointer to a new recurring KCal::Incidence if @a single is false.
    */
    KCal::Incidence *dissociateOccurrence( KCal::Incidence *incidence, const QDate &date,
                                     const KDateTime::Spec &spec,
                                     bool single = true );

  // KCal::Event Specific Methods //

    /**
      Inserts an KCal::Event into the calendar.

      @param event is a pointer to the KCal::Event to insert.

      @return true if the KCal::Event was successfully inserted; false otherwise.

      @see deleteEvent()
    */
    virtual bool addEvent( KCal::Event *event ) = 0;

    /**
      Removes an KCal::Event from the calendar.

      @param event is a pointer to the KCal::Event to remove.

      @return true if the KCal::Event was successfully remove; false otherwise.

      @see addEvent(), deleteAllEvents()
    */
    virtual bool deleteEvent( KCal::Event *event ) = 0;

    /**
      Removes all KCal::Events from the calendar.
      @see deleteEvent()
    */
    virtual void deleteAllEvents() = 0;

    /**
      Sort a list of KCal::Events.

      @param eventList is a pointer to a list of KCal::Events.
      @param sortField specifies the EventSortField.
      @param sortDirection specifies the SortDirection.

      @return a list of KCal::Events sorted as specified.
    */
    static KCal::Event::List sortEvents( KCal::Event::List *eventList,
                                   EventSortField sortField,
                                   SortDirection sortDirection );
    /**
      Returns a sorted, filtered list of all KCal::Events for this Calendar.

      @param sortField specifies the EventSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of all filtered KCal::Events sorted as specified.
    */
    virtual KCal::Event::List events(
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      Returns a filtered list of all KCal::Events which occur on the given timestamp.

      @param dt request filtered KCal::Event list for this KDateTime only.

      @return the list of filtered KCal::Events occurring on the specified timestamp.
    */
    KCal::Event::List events( const KDateTime &dt );

    /**
      Returns a filtered list of all KCal::Events occurring within a date range.

      @param start is the starting date.
      @param end is the ending date.
      @param timeSpec time zone etc. to interpret @p start and @p end,
                      or the calendar's default time spec if none is specified
      @param inclusive if true only KCal::Events which are completely included
      within the date range are returned.

      @return the list of filtered KCal::Events occurring within the specified
      date range.
    */
    KCal::Event::List events( const QDate &start, const QDate &end,
                        const KDateTime::Spec &timeSpec = KDateTime::Spec(),
                        bool inclusive = false );

    /**
      Returns a sorted, filtered list of all KCal::Events which occur on the given
      date.  The KCal::Events are sorted according to @a sortField and
      @a sortDirection.

      @param date request filtered KCal::Event list for this QDate only.
      @param timeSpec time zone etc. to interpret @p start and @p end,
                      or the calendar's default time spec if none is specified
      @param sortField specifies the EventSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of sorted, filtered KCal::Events occurring on @a date.
    */
    KCal::Event::List events(
      const QDate &date,
      const KDateTime::Spec &timeSpec = KDateTime::Spec(),
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      Returns a sorted, unfiltered list of all KCal::Events for this Calendar.

      @param sortField specifies the EventSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of all unfiltered KCal::Events sorted as specified.
    */
    virtual KCal::Event::List rawEvents(
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
      Returns an unfiltered list of all KCal::Events which occur on the given
      timestamp.

      @param dt request unfiltered KCal::Event list for this KDateTime only.

      @return the list of unfiltered KCal::Events occurring on the specified
      timestamp.
    */
    virtual KCal::Event::List rawEventsForDate( const KDateTime &dt ) = 0;

    /**
      Returns an unfiltered list of all KCal::Events occurring within a date range.

      @param start is the starting date
      @param end is the ending date
      @param timeSpec time zone etc. to interpret @p start and @p end,
                      or the calendar's default time spec if none is specified
      @param inclusive if true only KCal::Events which are completely included
      within the date range are returned.

      @return the list of unfiltered KCal::Events occurring within the specified
      date range.
    */
    virtual KCal::Event::List rawEvents( const QDate &start, const QDate &end,
                                   const KDateTime::Spec &timeSpec = KDateTime::Spec(),
                                   bool inclusive = false ) = 0;

    /**
      Returns a sorted, unfiltered list of all KCal::Events which occur on the given
      date.  The KCal::Events are sorted according to @a sortField and
      @a sortDirection.

      @param date request unfiltered KCal::Event list for this QDate only
      @param timeSpec time zone etc. to interpret @p date,
                      or the calendar's default time spec if none is specified
      @param sortField specifies the EventSortField
      @param sortDirection specifies the SortDirection

      @return the list of sorted, unfiltered KCal::Events occurring on @p date
    */
    virtual KCal::Event::List rawEventsForDate(
      const QDate &date, const KDateTime::Spec &timeSpec = KDateTime::Spec(),
      EventSortField sortField = EventSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
      Returns the KCal::Event associated with the given unique identifier.

      @param uid is a unique identifier string.

      @return a pointer to the KCal::Event.
      A null pointer is returned if no such KCal::Event exists.
    */
    virtual KCal::Event *event( const QString &uid ) = 0;

  // KCal::Todo Specific Methods //

    /**
      Inserts a KCal::Todo into the calendar.

      @param todo is a pointer to the KCal::Todo to insert.

      @return true if the KCal::Todo was successfully inserted; false otherwise.

      @see deleteTodo()
    */
    virtual bool addTodo( KCal::Todo *todo ) = 0;

    /**
      Removes a KCal::Todo from the calendar.

      @param todo is a pointer to the KCal::Todo to remove.

      @return true if the KCal::Todo was successfully removed; false otherwise.

      @see addTodo(), deleteAllTodos()
    */
    virtual bool deleteTodo( KCal::Todo *todo ) = 0;

    /**
      Removes all To-dos from the calendar.
      @see deleteTodo()
    */
    virtual void deleteAllTodos() = 0;

    /**
      Sort a list of KCal::Todos.

      @param todoList is a pointer to a list of KCal::Todos.
      @param sortField specifies the TodoSortField.
      @param sortDirection specifies the SortDirection.

      @return a list of KCal::Todos sorted as specified.
    */
    static KCal::Todo::List sortTodos( KCal::Todo::List *todoList,
                                 TodoSortField sortField,
                                 SortDirection sortDirection );

    /**
      Returns a sorted, filtered list of all KCal::Todos for this Calendar.

      @param sortField specifies the TodoSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of all filtered KCal::Todos sorted as specified.
    */
    virtual KCal::Todo::List todos(
      TodoSortField sortField = TodoSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      Returns a filtered list of all KCal::Todos which are due on the specified date.

      @param date request filtered KCal::Todos due on this QDate.

      @return the list of filtered KCal::Todos due on the specified date.
    */
    virtual KCal::Todo::List todos( const QDate &date );

    /**
      Returns a sorted, unfiltered list of all KCal::Todos for this Calendar.

      @param sortField specifies the TodoSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of all unfiltered KCal::Todos sorted as specified.
    */
    virtual KCal::Todo::List rawTodos(
      TodoSortField sortField = TodoSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
      Returns an unfiltered list of all KCal::Todos which due on the specified date.

      @param date request unfiltered KCal::Todos due on this QDate.

      @return the list of unfiltered KCal::Todos due on the specified date.
    */
    virtual KCal::Todo::List rawTodosForDate( const QDate &date ) = 0;

    /**
      Returns the KCal::Todo associated with the given unique identifier.

      @param uid is a unique identifier string.

      @return a pointer to the KCal::Todo.
      A null pointer is returned if no such KCal::Todo exists.
    */
    virtual KCal::Todo *todo( const QString &uid ) = 0;

  // KCal::Journal Specific Methods //

    /**
      Inserts a KCal::Journal into the calendar.

      @param journal is a pointer to the KCal::Journal to insert.

      @return true if the KCal::Journal was successfully inserted; false otherwise.

      @see deleteJournal()
    */
    virtual bool addJournal( KCal::Journal *journal ) = 0;

    /**
      Removes a KCal::Journal from the calendar.

      @param journal is a pointer to the KCal::Journal to remove.

      @return true if the KCal::Journal was successfully removed; false otherwise.

      @see addJournal(), deleteAllJournals()
    */
    virtual bool deleteJournal( KCal::Journal *journal ) = 0;

    /**
      Removes all KCal::Journals from the calendar.
      @see deleteJournal()
    */
    virtual void deleteAllJournals() = 0;

    /**
      Sort a list of KCal::Journals.

      @param journalList is a pointer to a list of KCal::Journals.
      @param sortField specifies the JournalSortField.
      @param sortDirection specifies the SortDirection.

      @return a list of KCal::Journals sorted as specified.
    */
    static KCal::Journal::List sortJournals( KCal::Journal::List *journalList,
                                       JournalSortField sortField,
                                       SortDirection sortDirection );
    /**
      Returns a sorted, filtered list of all KCal::Journals for this Calendar.

      @param sortField specifies the JournalSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of all filtered KCal::Journals sorted as specified.
    */
    virtual KCal::Journal::List journals(
      JournalSortField sortField = JournalSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending );

    /**
      Returns a filtered list of all KCal::Journals for on the specified date.

      @param date request filtered KCal::Journals for this QDate only.

      @return the list of filtered KCal::Journals for the specified date.
    */
    virtual KCal::Journal::List journals( const QDate &date );

    /**
      Returns a sorted, unfiltered list of all KCal::Journals for this Calendar.

      @param sortField specifies the JournalSortField.
      @param sortDirection specifies the SortDirection.

      @return the list of all unfiltered KCal::Journals sorted as specified.
    */
    virtual KCal::Journal::List rawJournals(
      JournalSortField sortField = JournalSortUnsorted,
      SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
      Returns an unfiltered list of all KCal::Journals for on the specified date.

      @param date request unfiltered KCal::Journals for this QDate only.

      @return the list of unfiltered KCal::Journals for the specified date.
    */
    virtual KCal::Journal::List rawJournalsForDate( const QDate &date ) = 0;

    /**
      Returns the KCal::Journal associated with the given unique identifier.

      @param uid is a unique identifier string.

      @return a pointer to the KCal::Journal.
      A null pointer is returned if no such KCal::Journal exists.
    */
    virtual KCal::Journal *journal( const QString &uid ) = 0;

    /**
      Emits the beginBatchAdding() signal.

      This should be called before adding a batch of incidences with
      addIncidence( KCal::Incidence *), addTodo( KCal::Todo *), addEvent( KCal::Event *)
      or addJournal( KCal::Journal *). Some Calendars are connected to this
      signal, e.g: CalendarResources uses it to know a series of
      incidenceAdds are related so the user isn't prompted multiple
      times which resource to save the incidence to

      @since 4.4
    */
    void beginBatchAdding();

    /**
      Emits the endBatchAdding() signal.

      Used with beginBatchAdding(). Should be called after
      adding all incidences.

      @since 4.4
    */
    void endBatchAdding();

  // Relations Specific Methods //

    /**
      Setup Relations for an KCal::Incidence.

      @param incidence is a pointer to the KCal::Incidence to have a
      Relation setup.
    */
    virtual void setupRelations( KCal::Incidence *incidence );

    /**
      Removes all Relations from an KCal::Incidence.

      @param incidence is a pointer to the KCal::Incidence to have a
      Relation removed.
    */
    virtual void removeRelations( KCal::Incidence *incidence );

  // Filter Specific Methods //

    /**
      Sets the calendar filter.

      @param filter a pointer to a CalFilter object which will be
      used to filter Calendar KCal::Incidences.

      @see filter()
    */
    void setFilter( KCal::CalFilter *filter );

    /**
      Returns the calendar filter.

      @return a pointer to the calendar CalFilter.
      A null pointer is returned if no such CalFilter exists.

      @see setFilter()
    */
    KCal::CalFilter *filter();

  // KCal::Alarm Specific Methods //

    /**
      Returns a list of KCal::Alarms within a time range for this Calendar.

      @param from is the starting timestamp.
      @param to is the ending timestamp.

      @return the list of KCal::Alarms for the for the specified time range.
    */
    virtual KCal::Alarm::List alarms( const KDateTime &from,
                                const KDateTime &to ) = 0;

  // Observer Specific Methods //

    /**
      @class CalendarObserver

      The CalendarObserver class.
    */
    class CalendarObserver //krazy:exclude=dpointer
    {
      public:
        /**
          Destructor.
        */
        virtual ~CalendarObserver() {}

        /**
          Notify the Observer that a Calendar has been modified.

          @param modified set if the calendar has been modified.
          @param calendar is a pointer to the Calendar object that
          is being observed.
        */
        virtual void calendarModified( bool modified, CalendarBase *calendar );

        /**
          Notify the Observer that an KCal::Incidence has been inserted.

          @param incidence is a pointer to the KCal::Incidence that was inserted.
        */
        virtual void calendarIncidenceAdded( KCal::Incidence *incidence );

        /**
          Notify the Observer that an KCal::Incidence has been modified.

          @param incidence is a pointer to the KCal::Incidence that was modified.
        */
        virtual void calendarIncidenceChanged( KCal::Incidence *incidence );

        /**
          Notify the Observer that an KCal::Incidence has been removed.

          @param incidence is a pointer to the KCal::Incidence that was removed.
        */
        virtual void calendarIncidenceDeleted( KCal::Incidence *incidence );
    };

    /**
      Registers an Observer for this Calendar.

      @param observer is a pointer to an Observer object that will be
      watching this Calendar.

      @see unregisterObserver()
     */
    void registerObserver( CalendarObserver *observer );

    /**
      Unregisters an Observer for this Calendar.

      @param observer is a pointer to an Observer object that has been
      watching this Calendar.

      @see registerObserver()
     */
    void unregisterObserver( CalendarObserver *observer );

    using QObject::event;   // prevent warning about hidden virtual method

  Q_SIGNALS:
    /**
      Signals that the calendar has been modified.
     */
    void calendarChanged();

    /**
      Signals that the calendar has been saved.
     */
    void calendarSaved();

    /**
      Signals that the calendar has been loaded into memory.
     */
    void calendarLoaded();

    /**
      @see beginBatchAdding()
      @since 4.4
     */
    void batchAddingBegins();

    /**
      @see endBatchAdding()
      @since 4.4
     */
    void batchAddingEnds();

  protected:
    /**
      The Observer interface. So far not implemented.

      @param incidenceBase is a pointer an KCal::IncidenceBase object.
    */
    void incidenceUpdated( KCal::IncidenceBase *incidenceBase );

    /**
      Let Calendar subclasses set the time specification.

      @param timeSpec is the time specification (time zone, etc.) for
                      viewing KCal::Incidence dates.\n
    */
    virtual void doSetTimeSpec( const KDateTime::Spec &timeSpec );

    /**
      Let Calendar subclasses notify that they inserted an KCal::Incidence.

      @param incidence is a pointer to the KCal::Incidence object that was inserted.
    */
    void notifyIncidenceAdded( KCal::Incidence *incidence );

    /**
      Let Calendar subclasses notify that they modified an KCal::Incidence.

      @param incidence is a pointer to the KCal::Incidence object that was modified.
    */
    void notifyIncidenceChanged( KCal::Incidence *incidence );

    /**
      Let Calendar subclasses notify that they removed an KCal::Incidence.

      @param incidence is a pointer to the KCal::Incidence object that was removed.
    */
    void notifyIncidenceDeleted( KCal::Incidence *incidence );

    /**
      @copydoc
      CustomProperties::customPropertyUpdated()
    */
    virtual void customPropertyUpdated();

    /**
      Let Calendar subclasses notify that they enabled an Observer.

      @param enabled if true tells the calendar that a subclass has
      enabled an Observer.
    */
    void setObserversEnabled( bool enabled );

    /**
      Appends alarms of incidence in interval to list of alarms.

      @param alarms is a List of KCal::Alarms to be appended onto.
      @param incidence is a pointer to an KCal::Incidence containing the KCal::Alarm
      to be appended.
      @param from is the lower range of the next KCal::Alarm repitition.
      @param to is the upper range of the next KCal::Alarm repitition.
    */
    void appendAlarms( KCal::Alarm::List &alarms, KCal::Incidence *incidence,
                       const KDateTime &from, const KDateTime &to );

    /**
      Appends alarms of recurring events in interval to list of alarms.

      @param alarms is a List of KCal::Alarms to be appended onto.
      @param incidence is a pointer to an KCal::Incidence containing the KCal::Alarm
      to be appended.
      @param from is the lower range of the next KCal::Alarm repitition.
      @param to is the upper range of the next KCal::Alarm repitition.
    */
    void appendRecurringAlarms( KCal::Alarm::List &alarms, KCal::Incidence *incidence,
                                const KDateTime &from, const KDateTime &to );

  private:
    //@cond PRIVATE
    class Private;
    Private *const d;
    //@endcond

    Q_DISABLE_COPY( CalendarBase )
};

}

#endif
