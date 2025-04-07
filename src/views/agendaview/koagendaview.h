/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "koeventview.h"

#include <CalendarSupport/CalPrinter>

#include <memory>

class KOAgendaViewPrivate;

/**
  KOAgendaView is the agenda-like view that displays events in a single
  or multi-day view.
*/
class KOAgendaView : public KOEventView
{
    Q_OBJECT
public:
    explicit KOAgendaView(QWidget *parent = nullptr, bool isSideBySide = false);
    ~KOAgendaView() override;

    /** Returns maximum number of days supported by the koagendaview */
    [[nodiscard]] int maxDatesHint() const override;

    /** Returns number of currently shown dates. */
    [[nodiscard]] int currentDateCount() const override;

    /** returns the currently selected events */
    [[nodiscard]] Akonadi::Item::List selectedIncidences() override;

    /** returns the currently selected incidence's dates */
    [[nodiscard]] KCalendarCore::DateList selectedIncidenceDates() override;

    /** return the default start/end date/time for new events   */
    [[nodiscard]] bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay) override;

    [[nodiscard]] CalendarSupport::CalPrinter::PrintType printType() const override;

    /** start-datetime of selection */
    [[nodiscard]] QDateTime selectionStart() override;

    /** end-datetime of selection */
    [[nodiscard]] QDateTime selectionEnd() override;

    /** returns true if selection is for whole day */
    [[nodiscard]] bool selectedIsAllDay();
    /** make selected start/end invalid */
    void deleteSelectedDateTime();
    /** returns if only a single cell is selected, or a range of cells */
    [[nodiscard]] bool selectedIsSingleCell();

    /* reimp from BaseView */
    void setModel(QAbstractItemModel *model) override;

    void setTypeAheadReceiver(QObject *o) override;

    void setChanges(EventViews::EventView::Changes changes) override;

    void setDateRange(const QDateTime &start, const QDateTime &end, const QDate &preferredMonth = QDate()) override;

public Q_SLOTS:
    void updateView() override;
    void updateConfig() override;
    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate()) override;
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) override;

    void changeIncidenceDisplay(const Akonadi::Item &incidence, Akonadi::IncidenceChanger::ChangeType) override;

    void clearSelection() override;

    void readSettings();
    void readSettings(KConfig *);
    void writeSettings(KConfig *);

    void enableAgendaUpdate(bool enable);
    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) override;

    void zoomInHorizontally(QDate date = QDate());
    void zoomOutHorizontally(QDate date = QDate());

    void zoomInVertically();
    void zoomOutVertically();

    void zoomView(const int delta, const QPoint &pos, const Qt::Orientation orient = Qt::Horizontal);

    void calendarAdded(const Akonadi::CollectionCalendar::Ptr &calendar) override;
    void calendarRemoved(const Akonadi::CollectionCalendar::Ptr &calendar) override;

Q_SIGNALS:
    void zoomViewHorizontally(const QDate &, int count);
    void timeSpanSelectionChanged();

private:
    std::unique_ptr<KOAgendaViewPrivate> const d;
};
