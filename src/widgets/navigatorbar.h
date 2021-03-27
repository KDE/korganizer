/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
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

public Q_SLOTS:
    void selectDates(const KCalendarCore::DateList &);

Q_SIGNALS:
    void nextMonthClicked();
    void prevMonthClicked();
    void nextYearClicked();
    void prevYearClicked();
    void monthSelected(int month);
    void yearSelected(int year);

protected:
    QToolButton *createNavigationButton(const QString &icon, const QString &toolTip, const QString &whatsThis);

private Q_SLOTS:
    void selectMonthFromMenu();
    void selectYearFromMenu();

private:
    QDate mDate;

    QToolButton *mPrevYear = nullptr;
    QToolButton *mPrevMonth = nullptr;
    QToolButton *mMonth = nullptr;
    QToolButton *mYear = nullptr;
    QToolButton *mNextMonth = nullptr;
    QToolButton *mNextYear = nullptr;
};

