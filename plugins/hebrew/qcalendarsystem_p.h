/*
  This file is part of the kholidays library.

  SPDX-FileCopyrightText: 2014 John Layt <john@layt.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QDate>
#include <QSharedPointer>

class QCalendarSystemPrivate;

class QCalendarSystem
{
public:
    enum CalendarSystem {
        DefaultCalendar = 0,
        GregorianCalendar = 1,
        ChineseCalendar = 2,
        CopticCalendar = 3,
        EthiopicCalendar = 4,
        EthiopicAmeteAlemCalendar = 5,
        HebrewCalendar = 6,
        IndianNationalCalendar = 7,
        IslamicCalendar = 8,
        IslamicCivilCalendar = 9,
        ISO8601Calendar = 10,
        JapaneseCalendar = 11,
        JulianCalendar = 12,
        PersianCalendar = 13,
        ROCCalendar = 14,
        ThaiCalendar = 15,
        LastCalendar = ThaiCalendar,
    };

    explicit QCalendarSystem(QCalendarSystem::CalendarSystem calendar = QCalendarSystem::DefaultCalendar);
    ~QCalendarSystem();

    QCalendarSystem &operator=(const QCalendarSystem &other);

    QCalendarSystem::CalendarSystem calendarSystem() const;

    QDate epoch() const;
    QDate earliestValidDate() const;
    QDate latestValidDate() const;
    int maximumMonthsInYear() const;
    int maximumDaysInYear() const;
    int maximumDaysInMonth() const;

    bool isValid(const QDate &date) const;
    bool isValid(int year, int month, int day) const;
    bool isValid(int year, int dayOfYear) const;

    QDate date(int year, int month, int day) const;
    QDate date(int year, int dayOfYear) const;

    void getDate(const QDate &date, int *year, int *month, int *day) const;

    int year(const QDate &date) const;
    int month(const QDate &date) const;
    int day(const QDate &date) const;

    int quarter(const QDate &date) const;
    int quarter(int year, int month, int day) const;

    int dayOfYear(const QDate &date) const;
    int dayOfYear(int year, int month, int day) const;

    int dayOfWeek(const QDate &date) const;
    int dayOfWeek(int year, int month, int day) const;

    int weekNumber(const QDate &date, int *yearNum = nullptr) const;
    int weekNumber(int year, int month, int day, int *yearNum = nullptr) const;

    int monthsInYear(const QDate &date) const;
    int monthsInYear(int year) const;

    int weeksInYear(const QDate &date) const;
    int weeksInYear(int year) const;

    int daysInYear(const QDate &date) const;
    int daysInYear(int year) const;

    int daysInMonth(const QDate &date) const;
    int daysInMonth(int year, int month) const;

    int daysInWeek() const;

    bool isLeapYear(const QDate &date) const;
    bool isLeapYear(int year) const;

    QDate addYears(const QDate &date, int years) const;
    QDate addMonths(const QDate &date, int months) const;
    QDate addDays(const QDate &date, int days) const;

    int yearsDifference(const QDate &fromDate, const QDate &toDate) const;
    int monthsDifference(const QDate &fromDate, const QDate &toDate) const;
    qint64 daysDifference(const QDate &fromDate, const QDate &toDate) const;

    void dateDifference(const QDate &fromDate, const QDate &toDate, int *years, int *months, int *days, int *direction) const;

    QDate firstDayOfYear(const QDate &date) const;
    QDate firstDayOfYear(int year) const;
    QDate lastDayOfYear(const QDate &date) const;
    QDate lastDayOfYear(int year) const;

    QDate firstDayOfMonth(const QDate &date) const;
    QDate firstDayOfMonth(int year, int month) const;
    QDate lastDayOfMonth(const QDate &date) const;
    QDate lastDayOfMonth(int year, int month) const;

private:
    QSharedDataPointer<QCalendarSystemPrivate> d;
};
