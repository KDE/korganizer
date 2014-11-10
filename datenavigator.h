/*
  This file is part of KOrganizer.

  Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  Author: Sergio Martins <sergio@kdab.com>

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

#ifndef KORG_DATENAVIGATOR_H
#define KORG_DATENAVIGATOR_H

#include <KCalCore/IncidenceBase> // for KCalCore::DateList typedef

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
    explicit DateNavigator(QObject *parent = 0);
    ~DateNavigator();

    KCalCore::DateList selectedDates();

    int datesCount() const;

public slots:
    void selectDates(const KCalCore::DateList &, const QDate &preferredMonth = QDate());
    void selectDate(const QDate &);

    void selectDates(int count);
    void selectDates(const QDate &, int count, const QDate &preferredMonth = QDate());

    void selectWeek();
    void selectWeek(const QDate &, const QDate &preferredMonth = QDate());

    void selectWorkWeek();
    void selectWorkWeek(const QDate &);

    void selectWeekByDay(int weekDay, const QDate &, const QDate &preferredMonth = QDate());

    void selectToday();

    void selectPreviousYear();
    void selectPreviousMonth(const QDate &currentMonth = QDate(),
                             const QDate &selectionLowerLimit = QDate(),
                             const QDate &selectionUpperLimit = QDate());
    void selectPreviousWeek();
    void selectNextWeek();
    void selectNextMonth(const QDate &currentMonth = QDate(),
                         const QDate &selectionLowerLimit = QDate(),
                         const QDate &selectionUpperLimit = QDate());
    void selectNextYear();

    void selectPrevious();
    void selectNext();

    void selectMonth(int month);
    void selectYear(int year);

signals:
    /* preferredMonth is useful when the datelist crosses months,
       if valid, any month-like component should honour it
    */
    void datesSelected(const KCalCore::DateList &, const QDate &preferredMonth);

protected:
    void emitSelected(const QDate &preferredMonth = QDate());

private:

    /*
      Selects next month if offset equals 1, or previous month
      if offset equals -1.
      Bigger offsets are accepted.
    */
    void shiftMonth(const QDate &date,
                    const QDate &selectionLowerLimit,
                    const QDate &selectionUpperLimit,
                    int offset);

    KCalCore::DateList mSelectedDates;

    enum {
        MAX_SELECTABLE_DAYS = 50
    };
};

#endif
