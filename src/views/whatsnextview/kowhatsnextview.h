/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "baseview.h"
#include <EventViews/WhatsNextView>

/**
  This class provides a view of the next events and todos
*/
class KOWhatsNextView : public KOrg::BaseView
{
    Q_OBJECT
public:
    explicit KOWhatsNextView(QWidget *parent = nullptr);
    ~KOWhatsNextView() override;

    [[nodiscard]] int currentDateCount() const override;
    [[nodiscard]] Akonadi::Item::List selectedIncidences() override
    {
        return {};
    }

    [[nodiscard]] KCalendarCore::DateList selectedIncidenceDates() override
    {
        return {};
    }

    [[nodiscard]] bool supportsDateNavigation() const override
    {
        return true;
    }

    [[nodiscard]] CalendarSupport::CalPrinterBase::PrintType printType() const override;

public Q_SLOTS:
    void updateView() override;
    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth) override;
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) override;

    void changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType) override;

    void calendarAdded(const Akonadi::CollectionCalendar::Ptr &calendar) override;
    void calendarRemoved(const Akonadi::CollectionCalendar::Ptr &calendar) override;

private:
    EventViews::WhatsNextView *const mView;
};
