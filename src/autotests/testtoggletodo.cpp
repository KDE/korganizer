/*
 *  SPDX-FileCopyrightText: 2022 Glen Ditchfield <GJDitchfield@acm.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "calendarview.h"

#include <QTest>

const auto TEST_TZ = "UTC";

class TestToggleTodo : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void initTestCase()
    {
        qputenv("TZ", TEST_TZ);
    }

    void testToggleNonRecurringComplete()
    {
        const KCalendarCore::Todo::Ptr todo{new KCalendarCore::Todo};
        QVERIFY(!todo->isCompleted());

        CalendarView::toggleCompleted(todo, QDate::currentDate());
        QVERIFY(todo->isCompleted());

        CalendarView::toggleCompleted(todo, QDate::currentDate());
        QVERIFY(!todo->isCompleted());
    }

    void testToggleRecurringComplete()
    {
        const QDateTime day1{{2022, 12, 01}, {12, 00, 00}, QTimeZone(TEST_TZ)};
        const QDateTime day2 = day1.addDays(1);
        const QDateTime day3 = day2.addDays(1);

        const KCalendarCore::Todo::Ptr todo{new KCalendarCore::Todo};
        todo->setDtStart(day1);
        todo->setDtRecurrence(day1);
        todo->recurrence()->setDaily(1);
        todo->recurrence()->setDuration(3);
        QVERIFY(KCalendarCore::identical(todo->dtRecurrence(), day1));
        QVERIFY(!todo->isCompleted());

        // Toggle the first occurrence.
        CalendarView::toggleCompleted(todo, day1.date());

        QVERIFY(KCalendarCore::identical(todo->dtRecurrence(), day2));
        QVERIFY(!todo->isCompleted());
        CalendarView::toggleCompleted(todo, day1.date());
        QVERIFY(KCalendarCore::identical(todo->dtRecurrence(), day1));
        QVERIFY(!todo->isCompleted());

        // Toggle the second occurrence.
        CalendarView::toggleCompleted(todo, day2.date());
        QVERIFY(KCalendarCore::identical(todo->dtRecurrence(), day3));
        QVERIFY(!todo->isCompleted());
        CalendarView::toggleCompleted(todo, day2.date());
        QVERIFY(KCalendarCore::identical(todo->dtRecurrence(), day2));
        QVERIFY(!todo->isCompleted());

        // Complete the second occurrence, then discomplete the first (and
        // hence the second).
        CalendarView::toggleCompleted(todo, day2.date());
        QVERIFY(KCalendarCore::identical(todo->dtRecurrence(), day3));
        QVERIFY(!todo->isCompleted());
        CalendarView::toggleCompleted(todo, day1.date());
        QVERIFY(KCalendarCore::identical(todo->dtRecurrence(), day1));
        QVERIFY(!todo->isCompleted());

        // Complete the final occurrence, and hence the entire todo.
        CalendarView::toggleCompleted(todo, day3.date());
        QVERIFY(todo->isCompleted());

        CalendarView::toggleCompleted(todo, day3.date());
        QVERIFY(KCalendarCore::identical(todo->dtRecurrence(), day3));
        QVERIFY(!todo->isCompleted());
    }

    void testToggleRecurringEastern()
    {
        // This date-time would appear on the 12th in a calendar running in the
        // UTC time zone.
        const QDateTime day1{{2022, 10, 13}, {00, 00, 00}, QTimeZone("Asia/Tokyo")};
        const QDateTime day2 = day1.addDays(1);
        const QDateTime day3 = day2.addDays(1);

        const KCalendarCore::Todo::Ptr todo{new KCalendarCore::Todo};
        todo->setDtStart(day1);
        todo->setDtRecurrence(day1);
        todo->recurrence()->setDaily(1);
        todo->recurrence()->setDuration(3);
        QVERIFY(KCalendarCore::identical(todo->dtRecurrence(), day1));
        QVERIFY(!todo->isCompleted());

        // Toggle the first occurrence.
        CalendarView::toggleCompleted(todo, day1.toLocalTime().date());
        QVERIFY(KCalendarCore::identical(todo->dtRecurrence(), day2));
        QVERIFY(!todo->isCompleted());
        CalendarView::toggleCompleted(todo, day1.toLocalTime().date());
        QVERIFY(KCalendarCore::identical(todo->dtRecurrence(), day1));
        QVERIFY(!todo->isCompleted());
    }

    void testToggleRecurringWestern()
    {
        // This date-time would appear on the 14th in a calendar running in the
        // UTC time zone.
        const QDateTime day1{{2022, 10, 13}, {19, 00, 00}, QTimeZone("America/Winnipeg")};
        const QDateTime day2 = day1.addDays(1);
        const QDateTime day3 = day2.addDays(1);

        const KCalendarCore::Todo::Ptr todo{new KCalendarCore::Todo};
        todo->setDtStart(day1);
        todo->setDtRecurrence(day1);
        todo->recurrence()->setDaily(1);
        todo->recurrence()->setDuration(3);
        QVERIFY(KCalendarCore::identical(todo->dtRecurrence(), day1));
        QVERIFY(!todo->isCompleted());

        // Complete the final occurrence, and hence the entire todo.
        CalendarView::toggleCompleted(todo, day3.toLocalTime().date());
        QVERIFY(todo->isCompleted());

        CalendarView::toggleCompleted(todo, day3.toLocalTime().date());
        QVERIFY(KCalendarCore::identical(todo->dtRecurrence(), day3));
        QVERIFY(!todo->isCompleted());
    }
};

QTEST_MAIN(TestToggleTodo)

#include "testtoggletodo.moc"
