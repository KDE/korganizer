/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <Akonadi/ETMCalendar>
#include <KCalendarCore/IncidenceBase> //for DateList typedef
#include <QDate>
#include <QFrame>

class KODayMatrix;
class NavigatorBar;

namespace Akonadi
{
class Item;
}

class QLabel;

class KDateNavigator : public QFrame
{
    Q_OBJECT
public:
    explicit KDateNavigator(QWidget *parent = nullptr);
    ~KDateNavigator() override;

    /**
      Associate date navigator with a calendar. It is used by KODayMatrix.
    */
    void setCalendar(const Akonadi::ETMCalendar::Ptr &);

    void setBaseDate(const QDate &);

    Q_REQUIRED_RESULT KCalendarCore::DateList selectedDates() const
    {
        return mSelectedDates;
    }

    Q_REQUIRED_RESULT QSizePolicy sizePolicy() const;

    NavigatorBar *navigatorBar() const
    {
        return mNavigatorBar;
    }

    Q_REQUIRED_RESULT QDate startDate() const;
    Q_REQUIRED_RESULT QDate endDate() const;
    void setHighlightMode(bool highlightEvents, bool highlightTodos, bool highlightJournals) const;

    /**
       Returns the current displayed month.
       It's a QDate instead of uint so it can be easily feed to KCalendarSystem's
       functions.
    */
    Q_REQUIRED_RESULT QDate month() const;

public Q_SLOTS:
    void selectDates(const KCalendarCore::DateList &);
    void selectPreviousMonth();
    void selectNextMonth();
    void updateView();
    void updateConfig();
    void updateDayMatrix();
    void updateToday();
    void setUpdateNeeded();

Q_SIGNALS:
    void datesSelected(const KCalendarCore::DateList &);
    void incidenceDropped(const Akonadi::Item &, const QDate &);
    void incidenceDroppedMove(const Akonadi::Item &, const QDate &);
    void newEventSignal(const QDate &);
    void newTodoSignal(const QDate &);
    void newJournalSignal(const QDate &);
    void weekClicked(const QDate &week, const QDate &month);

    void goPrevious();
    void goNext();
    void nextMonthClicked();
    void prevMonthClicked();
    void nextYearClicked();
    void prevYearClicked();

    void monthSelected(int month);
    void yearSelected(int year);

protected:
    void updateDates();

    void wheelEvent(QWheelEvent *) override;

    bool eventFilter(QObject *, QEvent *) override;

    void setShowWeekNums(bool enabled);

private:
    void selectMonthHelper(int monthDifference);
    NavigatorBar *const mNavigatorBar;

    QLabel *mHeadings[7];
    QLabel *mWeeknos[7];

    KODayMatrix *mDayMatrix = nullptr;

    KCalendarCore::DateList mSelectedDates;
    QDate mBaseDate;
    Akonadi::ETMCalendar::Ptr mCalendar;

    // Disabling copy constructor and assignment operator
    KDateNavigator(const KDateNavigator &);
    KDateNavigator &operator=(const KDateNavigator &);
};

