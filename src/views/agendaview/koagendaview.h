/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "koeventview.h"

#include <CalendarSupport/CalPrinter>

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
    Q_REQUIRED_RESULT int maxDatesHint() const override;

    /** Returns number of currently shown dates. */
    Q_REQUIRED_RESULT int currentDateCount() const override;

    /** returns the currently selected events */
    Q_REQUIRED_RESULT Akonadi::Item::List selectedIncidences() override;

    /** returns the currently selected incidence's dates */
    Q_REQUIRED_RESULT KCalendarCore::DateList selectedIncidenceDates() override;

    /** return the default start/end date/time for new events   */
    Q_REQUIRED_RESULT bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay) override;

    Q_REQUIRED_RESULT CalendarSupport::CalPrinter::PrintType printType() const override;

    /** start-datetime of selection */
    Q_REQUIRED_RESULT QDateTime selectionStart() override;

    /** end-datetime of selection */
    Q_REQUIRED_RESULT QDateTime selectionEnd() override;

    /** returns true if selection is for whole day */
    Q_REQUIRED_RESULT bool selectedIsAllDay();
    /** make selected start/end invalid */
    void deleteSelectedDateTime();
    /** returns if only a single cell is selected, or a range of cells */
    Q_REQUIRED_RESULT bool selectedIsSingleCell();

    /* reimp from BaseView */
    void setCalendar(const Akonadi::ETMCalendar::Ptr &cal) override;

    void setTypeAheadReceiver(QObject *o) override;

    void setChanges(EventViews::EventView::Changes changes) override;

    void setDateRange(const QDateTime &start, const QDateTime &end, const QDate &preferredMonth = QDate()) override;

public Q_SLOTS:
    void updateView() override;
    void updateConfig() override;
    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate()) override;
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) override;

    void changeIncidenceDisplayAdded(const Akonadi::Item &incidence);
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

Q_SIGNALS:
    void zoomViewHorizontally(const QDate &, int count);
    void timeSpanSelectionChanged();

private:
    class Private;
    Private *const d;
};

