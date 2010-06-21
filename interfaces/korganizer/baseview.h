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

#include <kcal/event.h>
#include <akonadi/kcal/incidencechanger.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <QtCore/QPair>
#include <QtGui/QWidget>

class QModelIndex;
class QPoint;

class KConfigGroup;

namespace Akonadi {
  class CalendarSearch;
  class CollectionSelection;
  class CollectionSelectionProxyModel;
  class Calendar;
}

namespace KCal {
  class CalFilter;
}

namespace KOrg {


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
    explicit BaseView( QWidget *parent = 0 );

    /**
      Destructor.  Views will do view-specific cleanups here.
    */
    virtual ~BaseView();

    virtual void setCalendar( Akonadi::Calendar *cal );
    /**
      Return calendar object of this view.
    */
    virtual Akonadi::Calendar *calendar();

    Akonadi::CalendarSearch* calendarSearch() const;

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
    virtual KCal::DateList selectedIncidenceDates() = 0;

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
    virtual int currentDateCount() const = 0;

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

    /**
     * returns whether this view supports zoom.
     * Base implementation returns false.
     */
    virtual bool supportsZoom();

    /**
     * returns whether this view supports date range selection
     * Base implementation returns true.
     */
    virtual bool supportsDateRangeSelection();

    virtual bool hasConfigurationDialog() const;

    virtual void showConfigurationDialog( QWidget* parent );

    QByteArray identifier() const;
    void setIdentifier( const QByteArray& identifier );

    /**
     * reads the view configuration. View-specific configuration can be
     * restored via doRestoreConfig()
     *
     * @param configGroup the group to read settings from
     * @see doRestoreConfig()
     */
    void restoreConfig( const KConfigGroup &configGroup );

    /**
     * writes out the view configuration. View-specific configuration can be
     * saved via doSaveConfig()
     *
     * @param configGroup the group to store settings in
     * @see doSaveConfig()
     */
    void saveConfig( KConfigGroup &configGroup );

    Akonadi::CollectionSelectionProxyModel *takeCustomCollectionSelectionProxyModel();
    Akonadi::CollectionSelectionProxyModel *customCollectionSelectionProxyModel() const;
    void setCustomCollectionSelectionProxyModel( Akonadi::CollectionSelectionProxyModel* model );

    Akonadi::CollectionSelection *customCollectionSelection() const;

    static Akonadi::CollectionSelection* globalCollectionSelection();
    static void setGlobalCollectionSelection( Akonadi::CollectionSelection* selection );

    /**
     * returns the view at the given widget coordinate. This is usually the view itself,
     * except for composite views, where a subview will be returned. The default implementation returns @p this .
     */
    virtual BaseView* viewAt( const QPoint &p );

    /**
      Show incidences for the given date range. The date range actually shown
      may be different from the requested range, depending on the particular
      requirements of the view.

      @param start Start of date range.
      @param end   End of date range.
    */
    void setDateRange( const KDateTime &start, const KDateTime &end );

    KDateTime startDateTime() const;
    KDateTime endDateTime() const;

    KDateTime actualStartDateTime() const;
    KDateTime actualEndDateTime() const;

    /** Returns true if the view supports navigation through the date navigator
        ( selecting a date range, changing month, changing year, etc. )
     */
    virtual bool supportsDateNavigation() const { return false; }

  public Q_SLOTS:
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
    virtual void setIncidenceChanger( Akonadi::IncidenceChanger *changer );

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
    void incidenceSelected( const Akonadi::Item &, const QDate );

    /**
     * instructs the receiver to show the incidence in read-only mode.
     */
    void showIncidenceSignal( const Akonadi::Item & );

    /**
     * instructs the receiver to begin editing the incidence specified in
     * some manner.  Doesn't make sense to connect to more than one
     * receiver.
     */
    void editIncidenceSignal( const Akonadi::Item & );

    /**
     * instructs the receiver to delete the Incidence in some manner; some
     * possibilities include automatically, with a confirmation dialog
     * box, etc.  Doesn't make sense to connect to more than one receiver.
     */
    void deleteIncidenceSignal( const Akonadi::Item & );

    /**
    * instructs the receiver to cut the Incidence
    */
    void cutIncidenceSignal( const Akonadi::Item & );

    /**
    * instructs the receiver to copy the incidence
    */
    void copyIncidenceSignal( const Akonadi::Item & );

    /**
    * instructs the receiver to paste the incidence
    */
    void pasteIncidenceSignal();

    /**
     * instructs the receiver to toggle the alarms of the Incidence.
     */
    void toggleAlarmSignal( const Akonadi::Item & );

    /**
     * instructs the receiver to toggle the completion state of the Incidence
     * (which must be a  Todo type).
     */
    void toggleTodoCompletedSignal( const Akonadi::Item & );

    /**
     * Copy the incidence to the specified resource.
     */
    void copyIncidenceToResourceSignal( const Akonadi::Item &, const QString & );

    /**
     * Move the incidence to the specified resource.
     */
    void moveIncidenceToResourceSignal( const Akonadi::Item &, const QString & );

    /** Dissociate from a recurring incidence the occurrence on the given
     *  date to a new incidence or dissociate all occurrences from the
     *  given date onwards.
     */
    void dissociateOccurrencesSignal( const Akonadi::Item &, const QDate & );

    void startMultiModify( const QString & );
    void endMultiModify();

    /**
     * instructs the receiver to create a new event in given collection. Doesn't make
     * sense to connect to more than one receiver.
     */
    void newEventSignal( const Akonadi::Collection::List & );
    /**
     * instructs the receiver to create a new event with the specified beginning
     * time. Doesn't make sense to connect to more than one receiver.
     */
    void newEventSignal( const Akonadi::Collection::List &, const QDate & );
    /**
     * instructs the receiver to create a new event with the specified beginning
     * time. Doesn't make sense to connect to more than one receiver.
     */
    void newEventSignal( const Akonadi::Collection::List &, const QDateTime & );
    /**
     * instructs the receiver to create a new event, with the specified
     * beginning end ending times.  Doesn't make sense to connect to more
     * than one receiver.
     */
    void newEventSignal( const Akonadi::Collection::List &, const QDateTime &, const QDateTime & );

    void newTodoSignal( const QDate & );
    void newSubTodoSignal( const Akonadi::Item & );

    void newJournalSignal( const QDate & );

  public:
    /**
     * returns the selection of collection to be used by this view (custom if set, or global otherwise)
     */
    Akonadi::CollectionSelection* collectionSelection() const;

    /**
     * sets the kcal filter on the calendarSearch object, this method can be removed from here when
     * calendarsearch stuff is removed from baseview, do we need a calendarsearch object per view?
     */
    void setFilter( KCal::CalFilter *filter );

  protected:
    /**
     * reimplement to read view-specific settings
     */
    virtual void doRestoreConfig( const KConfigGroup &configGroup );

    /**
     * reimplement to write vie- specific settings
     */
    virtual void doSaveConfig( KConfigGroup &configGroup );

    /**
      @deprecated
     */
    virtual void showDates( const QDate& start, const QDate& end ) = 0;

    /**
     * from the requested date range (passed via setDateRange()), calculates the adjusted date range actually displayed by the view, depending
     * on the view's supported range (e.g., a month view always displays one month)
     * The default implementation returns the range unmodified
     */
    virtual QPair<KDateTime,KDateTime> actualDateRange( const KDateTime& start, const KDateTime& end ) const;

    virtual void incidencesAdded( const Akonadi::Item::List& incidences );
    virtual void incidencesAboutToBeRemoved( const Akonadi::Item::List& incidences );
    virtual void incidencesChanged( const Akonadi::Item::List& incidences );

    virtual void handleBackendError( const QString &error );

  protected Q_SLOTS:
    virtual void collectionSelectionChanged();
    virtual void calendarReset();

  private Q_SLOTS:
    void backendErrorOccurred();
    void dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight );
    void rowsInserted( const QModelIndex& parent, int start, int end );
    void rowsAboutToBeRemoved( const QModelIndex& parent, int start, int end );

  protected:
    Akonadi::IncidenceChanger *mChanger;

  private:
    class Private;
    Private *const d;
    friend class KOrg::BaseView::Private;
    static Akonadi::CollectionSelection* sGlobalCollectionSelection;
};

}

#endif
