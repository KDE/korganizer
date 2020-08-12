/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2002 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-FileContributor: Sergio Martins <sergio@kdab.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KORG_DATENAVIGATOR_H
#define KORG_DATENAVIGATOR_H

#include <KCalendarCore/IncidenceBase> // for KCalendarCore::DateList typedef

#include <QObject>
#include <QDate>
/**
  This class controls date navigation. All requests to move the views to another
  date are sent to the DateNavigator. The DateNavigator processes the new
  selection of dates and emits the required signals for the views.
*/
class DateNavigator : public QObject
{
    Q_OBJECT
public:
    explicit DateNavigator(QObject *parent = nullptr);
    ~DateNavigator();

    Q_REQUIRED_RESULT KCalendarCore::DateList selectedDates();

    Q_REQUIRED_RESULT int datesCount() const;

public Q_SLOTS:
    void selectDates(const KCalendarCore::DateList &, QDate preferredMonth = QDate());
    void selectDate(QDate);

    void selectDates(int count);
    void selectDates(const QDate &, int count, const QDate &preferredMonth = QDate());

    void selectWeek();
    void selectWeek(const QDate &, const QDate &preferredMonth = QDate());

    void selectWorkWeek();
    void selectWorkWeek(QDate);

    void selectWeekByDay(int weekDay, QDate, QDate preferredMonth = QDate());

    void selectToday();

    void selectPreviousYear();
    void selectPreviousMonth(
        const QDate &currentMonth = QDate(), const QDate &selectionLowerLimit = QDate(), const QDate &selectionUpperLimit = QDate());
    void selectPreviousWeek();
    void selectNextWeek();
    void selectNextMonth(
        const QDate &currentMonth = QDate(), const QDate &selectionLowerLimit = QDate(), const QDate &selectionUpperLimit = QDate());
    void selectNextYear();

    void selectPrevious();
    void selectNext();

    void selectMonth(int month);
    void selectYear(int year);

Q_SIGNALS:
    /* preferredMonth is useful when the datelist crosses months,
       if valid, any month-like component should honour it
    */
    void datesSelected(const KCalendarCore::DateList &, const QDate &preferredMonth);

protected:
    void emitSelected(const QDate &preferredMonth = QDate());

private:

    /*
      Selects next month if offset equals 1, or previous month
      if offset equals -1.
      Bigger offsets are accepted.
    */
    void shiftMonth(const QDate &date, const QDate &selectionLowerLimit, const QDate &selectionUpperLimit, int offset);

    KCalendarCore::DateList mSelectedDates;

    enum {
        MAX_SELECTABLE_DAYS = 50
    };
};

#endif
