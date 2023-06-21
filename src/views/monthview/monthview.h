/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-FileContributor: Sergio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "koeventview.h"

namespace EventViews
{
class MonthView;
}
namespace KOrg
{
class MonthView : public KOEventView
{
    Q_OBJECT
public:
    explicit MonthView(QWidget *parent = nullptr);
    ~MonthView() override;

    Q_REQUIRED_RESULT int currentDateCount() const override;
    Q_REQUIRED_RESULT int currentMonth() const;

    Q_REQUIRED_RESULT Akonadi::Item::List selectedIncidences() override;

    /** Returns dates of the currently selected events */
    Q_REQUIRED_RESULT KCalendarCore::DateList selectedIncidenceDates() override;

    Q_REQUIRED_RESULT QDateTime selectionStart() override;

    Q_REQUIRED_RESULT QDateTime selectionEnd() override;
    Q_REQUIRED_RESULT bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay) override;

    /**
     * Returns the average date in the view
     */
    Q_REQUIRED_RESULT QDate averageDate() const;

    Q_REQUIRED_RESULT bool usesFullWindow() override;

    Q_REQUIRED_RESULT bool supportsDateRangeSelection() override;

    Q_REQUIRED_RESULT CalendarSupport::CalPrinterBase::PrintType printType() const override;

    Q_REQUIRED_RESULT int maxDatesHint() const override;

    void setTypeAheadReceiver(QObject *o) override;

    void setDateRange(const QDateTime &start, const QDateTime &end, const QDate &preferredMonth = QDate()) override;

    void setCalendar(const Akonadi::ETMCalendar::Ptr &cal) override;

    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) override;

public Q_SLOTS:
    void updateView() override;

    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) override;

    void changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType) override;

    void updateConfig() override;

    void calendarAdded(const Akonadi::CollectionCalendar::Ptr &calendar) override;
    void calendarRemoved(const Akonadi::CollectionCalendar::Ptr &calendar) override;

Q_SIGNALS:
    void fullViewChanged(bool enabled);

private:
    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate()) override;

    EventViews::MonthView *mMonthView = nullptr;
    KOEventPopupMenu *mPopup = nullptr;
};
}
