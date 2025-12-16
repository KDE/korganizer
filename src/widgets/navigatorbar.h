/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/
#pragma once

#include <KCalendarCore/IncidenceBase> // for KCalendarCore::DateList typedef

#include <QDate>
#include <QWidget>
class QToolButton;

class NavigatorBar : public QWidget
{
    Q_OBJECT
public:
    explicit NavigatorBar(QWidget *parent = nullptr);
    ~NavigatorBar() override;

    void showButtons(bool left, bool right);
    /**
     * Settings for the sidebar open/close button.
     *
     * @param isShown indicates is the sidebar is shown and toggles for full window button for closing.
     * else the full window button is toggled for opening.
     */
    void setSideBarMode(bool isShown);

public Q_SLOTS:
    void selectDates(const KCalendarCore::DateList &);

Q_SIGNALS:
    void fullWindowClicked();
    void nextWeekClicked();
    void prevWeekClicked();
    void nextMonthClicked();
    void prevMonthClicked();
    void nextYearClicked();
    void prevYearClicked();
    void monthSelected(int month);
    void yearSelected(int year);

protected:
    QToolButton *createNavigationButton(const QString &icon, const QString &toolTip, const QString &whatsThis);

private:
    void selectMonthFromMenu();
    void selectYearFromMenu();
    QDate mDate;

    QToolButton *mFullWindow = nullptr;
    QToolButton *mPrevYear = nullptr;
    QToolButton *mPrevMonth = nullptr;
    QToolButton *mPrevWeek = nullptr;
    QToolButton *const mMonth;
    QToolButton *const mYear;
    QToolButton *mNextWeek = nullptr;
    QToolButton *mNextMonth = nullptr;
    QToolButton *mNextYear = nullptr;
};
