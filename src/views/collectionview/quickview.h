/*
 * SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#pragma once

#include <EventViews/ViewCalendar>

#include <QDialog>

class Ui_quickview;

namespace Akonadi
{
}

namespace EventViews
{
class AgendaView;
}

class Quickview : public QDialog
{
    Q_OBJECT
public:
    Quickview(const Akonadi::CollectionCalendar::Ptr &calendar, const QString &title);
    ~Quickview() override;

private:
    void onTodayClicked();
    void onNextClicked();
    void onPreviousClicked();
    void readConfig();
    void writeConfig();

    Ui_quickview *const mUi;
    EventViews::AgendaView *mAgendaView = nullptr;
    int mDayRange = 7;
};
