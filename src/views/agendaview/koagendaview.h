/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef KORG_VIEWS_KOAGENDAVIEW_H
#define KORG_VIEWS_KOAGENDAVIEW_H

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
    void changeIncidenceDisplay(const Akonadi::Item &incidence,
                                Akonadi::IncidenceChanger::ChangeType) override;

    void clearSelection() override;

    void readSettings();
    void readSettings(KConfig *);
    void writeSettings(KConfig *);

    void enableAgendaUpdate(bool enable);
    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) override;

    void zoomInHorizontally(const QDate &date = QDate());
    void zoomOutHorizontally(const QDate &date = QDate());

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

#endif
