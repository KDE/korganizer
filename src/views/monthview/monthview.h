/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-FileContributor: Sergio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
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

    [[nodiscard]] int currentDateCount() const override;
    [[nodiscard]] int currentMonth() const;

    [[nodiscard]] Akonadi::Item::List selectedIncidences() override;

    /** Returns dates of the currently selected events */
    [[nodiscard]] KCalendarCore::DateList selectedIncidenceDates() override;

    [[nodiscard]] QDateTime selectionStart() override;

    [[nodiscard]] QDateTime selectionEnd() override;
    [[nodiscard]] bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay) override;

    /**
     * Returns the average date in the view
     */
    [[nodiscard]] QDate averageDate() const;

    [[nodiscard]] bool usesFullWindow() override;

    [[nodiscard]] bool supportsDateRangeSelection() override;

    [[nodiscard]] CalendarSupport::CalPrinterBase::PrintType printType() const override;

    [[nodiscard]] int maxDatesHint() const override;

    void setTypeAheadReceiver(QObject *o) override;

    void setDateRange(const QDateTime &start, const QDateTime &end, const QDate &preferredMonth = QDate()) override;

    void setModel(QAbstractItemModel *model) override;

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
