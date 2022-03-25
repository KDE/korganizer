/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  SPDX-FileCopyrightText: 2008 Thomas Thrainer <tom_t@gmx.at>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include "baseview.h"

#include <CalendarSupport/Utils>

#include <EventViews/TodoView>

using namespace KOrg;

class KOTodoView : public BaseView
{
    Q_OBJECT
public:
    KOTodoView(bool sidebarView, QWidget *parent);
    ~KOTodoView() override;

    void setCalendar(const Akonadi::ETMCalendar::Ptr &) override;

    Q_REQUIRED_RESULT Akonadi::Item::List selectedIncidences() override;
    Q_REQUIRED_RESULT KCalendarCore::DateList selectedIncidenceDates() override;
    Q_REQUIRED_RESULT int currentDateCount() const override
    {
        return 0;
    }

    void setDocumentId(const QString &)
    {
    }

    void saveLayout(KConfig *config, const QString &group) const;
    void restoreLayout(KConfig *config, const QString &group, bool minimalDefaults);

    /** documentation in baseview.h */
    void getHighlightMode(bool &highlightEvents, bool &highlightTodos, bool &highlightJournals) override;

    Q_REQUIRED_RESULT bool usesFullWindow() override;

    void saveViewState();
    void restoreViewState();

    Q_REQUIRED_RESULT bool supportsDateRangeSelection() override
    {
        return false;
    }

    Q_REQUIRED_RESULT CalendarSupport::CalPrinterBase::PrintType printType() const override;

public Q_SLOTS:
    void setIncidenceChanger(Akonadi::IncidenceChanger *changer) override;
    void showDates(const QDate &start, const QDate &end, const QDate &preferredMonth = QDate()) override;
    void showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date) override;
    void updateView() override;
    void changeIncidenceDisplay(const Akonadi::Item &incidence, Akonadi::IncidenceChanger::ChangeType changeType) override;
    void updateConfig() override;
    void clearSelection() override;

private Q_SLOTS:
    void printTodo(bool);
    void printTodo();
    void printPreviewTodo();

Q_SIGNALS:
    void purgeCompletedSignal();
    void unSubTodoSignal();
    void unAllSubTodoSignal();
    void configChanged();
    void fullViewChanged(bool enabled);

private:
    EventViews::TodoView *mView = nullptr;
};
