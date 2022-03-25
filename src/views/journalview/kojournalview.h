/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "baseview.h"
#include <KCalendarCore/Incidence> // for KCalendarCore::DateList typedef

namespace EventViews
{
class JournalView;
}

/**
 * This class provides a journal view.

 * @short View for Journal components.
 * @author Cornelius Schumacher <schumacher@kde.org>, Reinhold Kainhofer <reinhold@kainhofer.com>
 * @see KOBaseView
 */
class KOJournalView : public KOrg::BaseView
{
    Q_OBJECT
public:
    explicit KOJournalView(QWidget *parent = nullptr);
    ~KOJournalView() override;

    Q_REQUIRED_RESULT int currentDateCount() const override;
    Q_REQUIRED_RESULT Akonadi::Item::List selectedIncidences() override;

    Q_REQUIRED_RESULT KCalendarCore::DateList selectedIncidenceDates() override
    {
        return {};
    }

    void setCalendar(const Akonadi::ETMCalendar::Ptr &) override;

    void getHighlightMode(bool &highlightEvents, bool &highlightTodos, bool &highlightJournals) override;

    Q_REQUIRED_RESULT CalendarSupport::CalPrinterBase::PrintType printType() const override;

public Q_SLOTS:
    void updateView() override;
    void flushView() override;

    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate()) override;
    void showIncidences(const Akonadi::Item::List &incidences, const QDate &date) override;

    void changeIncidenceDisplay(const Akonadi::Item &incidence, Akonadi::IncidenceChanger::ChangeType) override;
    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) override;
    void printJournal(const KCalendarCore::Journal::Ptr &journal, bool preview);

private:
    EventViews::JournalView *const mJournalView;
};
