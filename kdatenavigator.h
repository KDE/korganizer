/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef KORG_KDATENAVIGATOR_H
#define KORG_KDATENAVIGATOR_H

#include <QFrame>
#include <QDate>
#include <KCalCore/IncidenceBase> //for DateList typedef
#include <Akonadi/Calendar/ETMCalendar>

class KODayMatrix;
class NavigatorBar;

namespace Akonadi
{
class Item;
}

class QLabel;

class KDateNavigator: public QFrame
{
    Q_OBJECT
public:
    explicit KDateNavigator(QWidget *parent = 0);
    ~KDateNavigator();

    /**
      Associate date navigator with a calendar. It is used by KODayMatrix.
    */
    void setCalendar(const Akonadi::ETMCalendar::Ptr &);

    void setBaseDate(const QDate &);

    KCalCore::DateList selectedDates() const
    {
        return mSelectedDates;
    }

    QSizePolicy sizePolicy() const;

    NavigatorBar *navigatorBar() const
    {
        return mNavigatorBar;
    }

    QDate startDate() const;
    QDate endDate() const;
    void setHighlightMode(bool highlightEvents,
                          bool highlightTodos,
                          bool highlightJournals) const;

    /**
       Returns the current displayed month.
       It's a QDate instead of uint so it can be easily feed to KCalendarSystem's
       functions.
    */
    QDate month() const;

public slots:
    void selectDates(const KCalCore::DateList &);
    void selectPreviousMonth();
    void selectNextMonth();
    void updateView();
    void updateConfig();
    void updateDayMatrix();
    void updateToday();
    void setUpdateNeeded();

signals:
    void datesSelected(const KCalCore::DateList &);
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

    void wheelEvent(QWheelEvent *);

    bool eventFilter(QObject *, QEvent *);

    void setShowWeekNums(bool enabled);

private:
    void selectMonthHelper(int monthDifference);
    NavigatorBar *mNavigatorBar;

    QLabel *mHeadings[ 7 ];
    QLabel *mWeeknos[ 7 ];

    KODayMatrix *mDayMatrix;

    KCalCore::DateList mSelectedDates;
    QDate mBaseDate;
    Akonadi::ETMCalendar::Ptr mCalendar;

    // Disabling copy constructor and assignment operator
    KDateNavigator(const KDateNavigator &);
    KDateNavigator &operator=(const KDateNavigator &);
};

#endif
