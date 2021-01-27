/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#ifndef KORG_VIEWS_KOWHATSNEXTVIEW_H
#define KORG_VIEWS_KOWHATSNEXTVIEW_H

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

    Q_REQUIRED_RESULT int currentDateCount() const override;
    Q_REQUIRED_RESULT Akonadi::Item::List selectedIncidences() override
    {
        return Akonadi::Item::List();
    }

    Q_REQUIRED_RESULT KCalendarCore::DateList selectedIncidenceDates() override
    {
        return KCalendarCore::DateList();
    }

    Q_REQUIRED_RESULT bool supportsDateNavigation() const override
    {
        return true;
    }

    Q_REQUIRED_RESULT CalendarSupport::CalPrinterBase::PrintType printType() const override;
    void setCalendar(const Akonadi::ETMCalendar::Ptr &) override;

public Q_SLOTS:
    void updateView() override;
    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth) override;
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) override;

    void changeIncidenceDisplay(const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType) override;

private:
    EventViews::WhatsNextView *const mView;
};

#endif
