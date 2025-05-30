/*
  This file is part of the KOrganizer interfaces.

  SPDX-FileCopyrightText: 1999, 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
// clazy:excludeall=overloaded-signal

#pragma once

#include "korganizer_interfaces_export.h"

#include <CalendarSupport/PrintPlugin>
#include <EventViews/EventView>

#include <Akonadi/ETMCalendar>
#include <Akonadi/IncidenceChanger>

#include <Akonadi/Collection>
#include <Akonadi/Item>

#include <QWidget>

#include <memory>

namespace KOrg
{
class BaseViewPrivate;

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
    explicit BaseView(QWidget *parent = nullptr);

    /**
      Destructor.  Views will do view-specific cleanups here.
    */
    ~BaseView() override;

    /**
     * Sets the model used to display events.
     */
    virtual void setModel(QAbstractItemModel *model);

    QAbstractItemModel *model() const;

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
    virtual KCalendarCore::DateList selectedIncidenceDates() = 0;

    /**
       Returns the start of the selection, or an invalid QDateTime if there is no selection
       or the view doesn't support selecting cells.
     */
    virtual QDateTime selectionStart();

    /**
       Returns the end of the selection, or an invalid QDateTime if there is no selection
       or the view doesn't support selecting cells.
     */
    virtual QDateTime selectionEnd();

    virtual CalendarSupport::CalPrinterBase::PrintType printType() const;

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
    virtual void getHighlightMode(bool &highlightEvents, bool &highlightTodos, bool &highlightJournals);

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

    virtual void showConfigurationDialog(QWidget *parent);

    QByteArray &identifier();
    virtual void setIdentifier(const QByteArray &identifier);

    /**
     * reads the view configuration. View-specific configuration can be
     * restored via doRestoreConfig()
     *
     * @param configGroup the group to read settings from
     * @see doRestoreConfig()
     */
    virtual void restoreConfig(const KConfigGroup &configGroup);

    /**
     * writes out the view configuration. View-specific configuration can be
     * saved via doSaveConfig()
     *
     * @param configGroup the group to store settings in
     * @see doSaveConfig()
     */
    virtual void saveConfig(KConfigGroup &configGroup);

    /**
     * returns the view at the given widget coordinate. This is usually the view
     * itself, except for composite views, where a subview will be returned.
     * The default implementation returns @p this .
     */
    virtual BaseView *viewAt(const QPoint &p);

    /**
      Show incidences for the given date range. The date range actually shown
      may be different from the requested range, depending on the particular
      requirements of the view.

      @param start Start of date range.
      @param end   End of date range.
    */
    virtual void setDateRange(const QDateTime &start, const QDateTime &end, const QDate &preferredMonth = QDate());

    QDateTime startDateTime() const;

    /** Returns true if the view supports navigation through the date navigator
        ( selecting a date range, changing month, changing year, etc. )
     */
    virtual bool supportsDateNavigation() const
    {
        return false;
    }

    virtual Akonadi::Collection::Id collectionId() const
    {
        return -1;
    }

public Q_SLOTS:
    /**
      Shows given incidences. Depending on the actual view it might not
      be possible to show all given events.

      @param incidenceList a list of incidences to show.
      @param date is the QDate on which the incidences are being shown.
    */
    virtual void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) = 0;

    /**
      Updates the current display to reflect changes that may have happened
      in the calendar since the last display refresh.
    */
    virtual void updateView() = 0;
    virtual void dayPassed(const QDate &);

    /**
      Assign a new incidence change helper object.
     */
    virtual void setIncidenceChanger(Akonadi::IncidenceChanger *changer);

    /**
      Write all unsaved data back to calendar store.
    */
    virtual void flushView();

    /**
      Updates the current display to reflect the changes to one particular incidence.
    */
    virtual void changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType) = 0;

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
    virtual bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay);

    virtual void calendarAdded(const Akonadi::CollectionCalendar::Ptr &calendar);
    virtual void calendarRemoved(const Akonadi::CollectionCalendar::Ptr &calendar);

Q_SIGNALS:
    void incidenceSelected(const Akonadi::Item &, const QDate);

    /**
     * instructs the receiver to show the incidence in read-only mode.
     */
    void showIncidenceSignal(const Akonadi::Item &);

    /**
     * instructs the receiver to show the occurrence of the
     * incidence on the given date in read-only mode.
     */
    void showOccurrenceSignal(const Akonadi::Item &, const QDate &);

    /**
     * instructs the receiver to begin editing the incidence specified in
     * some manner.  Doesn't make sense to connect to more than one
     * receiver.
     */
    void editIncidenceSignal(const Akonadi::Item &);

    /**
     * instructs the receiver to delete the Incidence in some manner; some
     * possibilities include automatically, with a confirmation dialog
     * box, etc.  Doesn't make sense to connect to more than one receiver.
     */
    void deleteIncidenceSignal(const Akonadi::Item &);

    /**
     * instructs the receiver to cut the Incidence
     */
    void cutIncidenceSignal(const Akonadi::Item &);

    /**
     * instructs the receiver to copy the incidence
     */
    void copyIncidenceSignal(const Akonadi::Item &);

    /**
     * instructs the receiver to paste the incidence
     */
    void pasteIncidenceSignal();

    /**
     * instructs the receiver to toggle the alarms of the Incidence.
     */
    void toggleAlarmSignal(const Akonadi::Item &);

    /**
     * instructs the receiver to toggle the completion state of the Incidence
     * (which must be a  Todo type).
     */
    void toggleTodoCompletedSignal(const Akonadi::Item &);

    /**
     * Toggle the completion state of the incidence contained in the item
     * (which must be a Todo) on the given date.  If the todo recurs, the
     * QDate identifies the occurrence to toggle.
     */
    void toggleOccurrenceCompletedSignal(const Akonadi::Item &, const QDate &);

    /**
     * Copy the incidence to the specified resource.
     */
    void copyIncidenceToResourceSignal(const Akonadi::Item &, const Akonadi::Collection &);

    /**
     * Move the incidence to the specified resource.
     */
    void moveIncidenceToResourceSignal(const Akonadi::Item &, const Akonadi::Collection &);

    /** Dissociate from a recurring incidence the occurrence on the given
     *  date to a new incidence or dissociate all occurrences from the
     *  given date onwards.
     */
    void dissociateOccurrencesSignal(const Akonadi::Item &, const QDate &);

    void startMultiModify(const QString &);
    void endMultiModify();

    /**
     * instructs the receiver to create a new event in given collection. Doesn't make
     * sense to connect to more than one receiver.
     */
    void newEventSignal();

    /**
     * instructs the receiver to create a new event with the specified beginning
     * time. Doesn't make sense to connect to more than one receiver.
     */
    void newEventSignal(const QDate &);

    /**
     * instructs the receiver to create a new event with the specified beginning
     * time. Doesn't make sense to connect to more than one receiver.
     */
    void newEventSignal(const QDateTime &);

    /**
     * instructs the receiver to create a new event, with the specified
     * beginning end ending times.  Doesn't make sense to connect to more
     * than one receiver.
     */
    void newEventSignal(const QDateTime &, const QDateTime &);

    void newTodoSignal(const QDate &);
    void newSubTodoSignal(const Akonadi::Item &);

    void newJournalSignal(const QDate &);

public:
    /**
       Notifies the view that there are pending changes so a redraw is needed.

       @param changes Types of changes that were made
    */
    virtual void setChanges(EventViews::EventView::Changes changes);

    /**
       Returns if there are pending changes and a redraw is needed.
    */
    EventViews::EventView::Changes changes() const;

protected:
    /**
     * reimplement to read view-specific settings
     */
    virtual void doRestoreConfig(const KConfigGroup &configGroup);

    /**
     * reimplement to write view specific settings
     */
    virtual void doSaveConfig(KConfigGroup &configGroup);

    /**
      @deprecated
     */
    virtual void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate()) = 0;

    /**
     * From the requested date range (passed via setDateRange()), calculates the
     * adjusted date range actually displayed by the view, depending on the view's
     * supported range (e.g., a month view always displays one month).
     * The default implementation returns the range unmodified
     */
    virtual QPair<QDateTime, QDateTime> actualDateRange(const QDateTime &start, const QDateTime &end, const QDate &preferredMonth = QDate()) const;

    Akonadi::CollectionCalendar::Ptr calendarForCollection(Akonadi::Collection::Id collectionId) const;
    Akonadi::CollectionCalendar::Ptr calendarForIncidence(const KCalendarCore::Incidence::Ptr &incidence) const;

protected Q_SLOTS:
    virtual void calendarReset();

protected:
    Akonadi::IncidenceChanger *mChanger = nullptr;

private:
    std::unique_ptr<BaseViewPrivate> const d;
    friend class KOrg::BaseViewPrivate;
};
}
