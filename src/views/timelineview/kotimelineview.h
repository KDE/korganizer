/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2007 Till Adam <adam@kde.org>

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
  SPDX-FileCopyrightText: 2010 Andras Mantia <andras@kdab.com>
  SPDX-FileCopyrightText: 2010 Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once

#include "koeventview.h"

namespace Akonadi
{
class IncidenceChanger;
}

namespace EventViews
{
class TimelineView;
}

/**
  This class provides a view ....
*/
class KOTimelineView : public KOEventView
{
    Q_OBJECT
public:
    explicit KOTimelineView(QWidget *parent = nullptr);
    ~KOTimelineView() override;

    [[nodiscard]] Akonadi::Item::List selectedIncidences() override;
    [[nodiscard]] KCalendarCore::DateList selectedIncidenceDates() override;
    [[nodiscard]] int currentDateCount() const override;
    void showDates(const QDate &, const QDate &, const QDate &preferredMonth = QDate()) override;
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) override;
    void updateView() override;
    void changeIncidenceDisplay(const Akonadi::Item &incidence, Akonadi::IncidenceChanger::ChangeType) override;
    [[nodiscard]] int maxDatesHint() const override
    {
        return 0;
    }

    [[nodiscard]] bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay) override;
    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) override;

    [[nodiscard]] CalendarSupport::CalPrinterBase::PrintType printType() const override;

public Q_SLOTS:
    void calendarAdded(const Akonadi::CollectionCalendar::Ptr &calendar) override;
    void calendarRemoved(const Akonadi::CollectionCalendar::Ptr &calendar) override;

private:
    KOEventPopupMenu *mEventPopup = nullptr;
    EventViews::TimelineView *mTimeLineView = nullptr;
};
