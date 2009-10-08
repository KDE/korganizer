/*
  This file is part of the KOrganizer interfaces.

  Copyright (c) 1999,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004           Reinhold Kainhofer   <reinhold@kainhofer.com>

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
#ifndef KORG_BASEVIEW_H
#define KORG_BASEVIEW_H

#include "printplugin.h"
#include "korganizer/korganizer_export.h"
#include "korganizer/incidencechangerbase.h"

#include <kcal/event.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <Akonadi/Item>

#include <QtGui/QWidget>

using namespace KCal;

namespace KOrg {

class CalendarBase;

/**
  This class provides an interface for all views being displayed within the
  main calendar view. It has functions to update the view, to specify date
  range and other display parameter and to return selected objects. An
  important class, which inherits KOBaseView is KOEventView, which provides
  the interface for all views of event data like the agenda or the month view.

  @short Base class for calendar views
  @author Preston Brown, Cornelius Schumacher
  @see KOTodoView, KOEventView, KOListView, KOAgendaView, KOMonthView
*/
class KORGANIZER_INTERFACES_EXPORT BaseView : public QWidget
{
  Q_OBJECT
  public:
    /**
      Constructs a view.

      @param cal    Pointer to the calendar object from which events
                    will be retrieved for display.
      @param parent parent widget.
    */
    explicit BaseView( CalendarBase *cal, QWidget *parent = 0 );

    /**
      Destructor.  Views will do view-specific cleanups here.
    */
    virtual ~BaseView();

    virtual void setCalendar( CalendarBase *cal );
    /**
      Return calendar object of this view.
    */
    virtual CalendarBase *calendar();

    /**
      @return a list of selected events.  Most views can probably only
      select a single event at a time, but some may be able to select
      more than one.
    */
    virtual Akonadi::Item::List selectedIncidences() = 0;

    /**
      Returns a list of the dates of selected events. Most views can
      probably only select a single event at a time, but some may be able
      to select more than one.
    */
    virtual DateList selectedDates() = 0;

    /**
       Returns the start of the selection, or an invalid QDateTime if there is no selection
       or the view doesn't support selecting cells.
     */
    virtual QDateTime selectionStart() { return QDateTime(); }

    /**
       Returns the end of the selection, or an invalid QDateTime if there is no selection
       or the view doesn't support selecting cells.
     */
    virtual QDateTime selectionEnd() { return QDateTime(); }

    virtual CalPrinterBase::PrintType printType();

    /**
      Returns the number of currently shown dates.
      A return value of 0 means no idea.
    */
    virtual int currentDateCount() = 0;

    /**
      Returns if this view is a view for displaying events.
    */
    virtual bool isEventView();

    /**
     * Returns which incidence types should used to embolden
     * day numbers in the date navigator when this view is selected.
     *
     * BaseView provides a default implementation that only highlights
     * events because that's how the behaviour has always been, and
     * most views are event orientated, even one or two
     * which don't inherit KOEventView are about events (timespent).
     *
     * This function writes to these 3 parameters the result,
     * the original value is ignored
     */
    virtual void getHighlightMode( bool &highlightEvents,
                                   bool &highlightTodos,
                                   bool &highlightJournals );

    /**
     * returns whether this view should be displayed full window.
     * Base implementation returns false.
     */
    virtual bool usesFullWindow();

  public Q_SLOTS:
    /**
      Show incidences for the given date range. The date range actually shown
      may be different from the requested range, depending on the particular
      requirements of the view.

      @param start Start of date range.
      @param end   End of date range.
    */
    virtual void showDates( const QDate &start, const QDate &end ) = 0;

    /**
      Shows given incidences. Depending on the actual view it might not
      be possible to show all given events.

      @param incidenceList a list of incidences to show.
      @param date is the QDate on which the incidences are being shown.
    */
    virtual void showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date ) = 0;

    /**
      Updates the current display to reflect changes that may have happened
      in the calendar since the last display refresh.
    */
    virtual void updateView() = 0;
    virtual void dayPassed( const QDate & );

    /**
      Assign a new incidence change helper object.
     */
    virtual void setIncidenceChanger( IncidenceChangerBase *changer );

    /**
      Write all unsaved data back to calendar store.
    */
    virtual void flushView();

    /**
      Updates the current display to reflect the changes to one particular incidence.
    */
    virtual void changeIncidenceDisplay( const Akonadi::Item &, int ) = 0;

    /**
      Re-reads the KOrganizer configuration and picks up relevant
      changes which are applicable to the view.
    */
    virtual void updateConfig();

    /**
      Clear selection. The incidenceSelected signal is not emitted.
    */
    virtual void clearSelection();

    /**
      Sets the default start/end date/time for new events.
      Return true if anything was changed
    */
    virtual bool eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay );

  Q_SIGNALS:
    void incidenceSelected( Incidence *, const QDate );
    void incidenceSelected( const Akonadi::Item &, const QDate );

    /**
     * instructs the receiver to show the incidence in read-only mode.
     */
    void showIncidenceSignal( Incidence * );
    void showIncidenceSignal( const Akonadi::Item & );

    /**
     * instructs the receiver to begin editing the incidence specified in
     * some manner.  Doesn't make sense to connect to more than one
     * receiver.
     */
    void editIncidenceSignal( Incidence * );
    void editIncidenceSignal( const Akonadi::Item & );

    /**
     * instructs the receiver to delete the Incidence in some manner; some
     * possibilities include automatically, with a confirmation dialog
     * box, etc.  Doesn't make sense to connect to more than one receiver.
     */
    void deleteIncidenceSignal( Incidence * );
    void deleteIncidenceSignal( const Akonadi::Item & );

    /**
    * instructs the receiver to cut the Incidence
    */
    void cutIncidenceSignal( Incidence * );
    void cutIncidenceSignal( const Akonadi::Item & );

    /**
    * instructs the receiver to copy the incidence
    */
    void copyIncidenceSignal( Incidence * );
    void copyIncidenceSignal( const Akonadi::Item & );

    /**
    * instructs the receiver to paste the incidence
    */
    void pasteIncidenceSignal();

    /**
     * instructs the receiver to toggle the alarms of the Incidence.
     */
    void toggleAlarmSignal( Incidence * );
    void toggleAlarmSignal( const Akonadi::Item & );

    /**
     * instructs the receiver to toggle the completion state of the Incidence
     * (which must be a  Todo type).
     */
    void toggleTodoCompletedSignal( Incidence * );
    void toggleTodoCompletedSignal( const Akonadi::Item & );

    /** Dissociate from a recurring incidence the occurrence on the given
     *  date to a new incidence or dissociate all occurrences from the
     *  given date onwards.
     */
    void dissociateOccurrencesSignal( Incidence *, const QDate & );

    void startMultiModify( const QString & );
    void endMultiModify();

    /**
     * instructs the receiver to create a new event.  Doesn't make
     * sense to connect to more than one receiver.
     */
    void newEventSignal();
    /**
     * instructs the receiver to create a new event with the specified beginning
     * time. Doesn't make sense to connect to more than one receiver.
     */
    void newEventSignal( const QDate & );
    /**
     * instructs the receiver to create a new event with the specified beginning
     * time. Doesn't make sense to connect to more than one receiver.
     */
    void newEventSignal( const QDateTime & );
    /**
     * instructs the receiver to create a new event, with the specified
     * beginning end ending times.  Doesn't make sense to connect to more
     * than one receiver.
     */
    void newEventSignal( const QDateTime &, const QDateTime & );

    void newTodoSignal( const QDate & );
    void newSubTodoSignal( Todo * );
    void newSubTodoSignal( const Akonadi::Item & );

    void newJournalSignal( const QDate & );

  private:
    CalendarBase *mCalendar;
  protected:
    IncidenceChangerBase *mChanger;
};

}

#endif
