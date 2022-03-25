/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#pragma once

#include <EventViews/ViewCalendar>

#include <KCalendarCore/FreeBusy>
#include <QDialog>

class Ui_quickview;

namespace EventViews
{
class AgendaView;
}

class Quickview : public QDialog
{
    Q_OBJECT
public:
    Quickview(const Akonadi::Collection &col);
    ~Quickview() override;

private Q_SLOTS:
    void onTodayClicked();
    void onNextClicked();
    void onPreviousClicked();

private:
    void readConfig();
    void writeConfig();

    Ui_quickview *const mUi;
    EventViews::AgendaView *mAgendaView = nullptr;
    const Akonadi::Collection mCollection;
    int mDayRange = 7;
};
