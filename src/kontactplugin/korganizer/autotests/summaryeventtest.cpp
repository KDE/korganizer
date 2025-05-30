/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2008 Thomas McGuire <mcguire@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "summaryeventtest.h"
#include "../summaryeventinfo.h"

#include <KCalendarCore/MemoryCalendar>

#include <QTest>
QTEST_GUILESS_MAIN(SummaryEventTester)

void SummaryEventTester::test_Multiday()
{
    QDate today = QDate::currentDate();

    KCalendarCore::MemoryCalendar *cal = new KCalendarCore::MemoryCalendar(QTimeZone::systemTimeZone());

    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    event->setDtStart(QDateTime(today.addDays(-1)));
    event->setDtEnd(QDateTime(today.addDays(5)));
    event->setAllDay(true);
    QString multiDayAllDayStartingYesterday = QStringLiteral("Multiday, allday, in progress (day 2/6)");
    event->setSummary(multiDayAllDayStartingYesterday);
    QVERIFY(cal->addEvent(event));

    event = KCalendarCore::Event::Ptr(new KCalendarCore::Event());
    event->setDtStart(QDateTime(today.addDays(-1), QTime(12, 00)));
    event->setDtEnd(QDateTime(today.addDays(5), QTime(12, 00)));
    QString multidayWithTimeInProgress = QStringLiteral("Multiday, time specified, in progress");
    event->setSummary(multidayWithTimeInProgress);
    QVERIFY(cal->addEvent(event));
    for (int i = 0; i < 5; ++i) {
        SummaryEventInfo::List events4 = SummaryEventInfo::eventsForDate(today.addDays(i), cal);
        QCOMPARE(2, events4.size());
        const SummaryEventInfo *ev4 = events4.at(1);

        QCOMPARE(ev4->summaryText, QString(multidayWithTimeInProgress + QString::fromLatin1(" (%1/7)").arg(i + 2)));
        QCOMPARE(ev4->timeRange,
                 QStringLiteral("%1 - %2").arg(QLocale::system().toString(QTime(0, 0), QLocale::ShortFormat),
                                               QLocale::system().toString(QTime(23, 59), QLocale::ShortFormat)));
        // QCOMPARE( ev4->startDate, KLocale::global()->formatDate( QDate( today.addDays( i ) ), KLocale::FancyLongDate ) );
        QCOMPARE(ev4->makeBold, i == 0);

        qDeleteAll(events4);
    }

    // Test date a multiday event in the future has to correct DaysTo set
    QString multiDayWithTimeFuture = QStringLiteral("Multiday, with time, in the future");
    event = KCalendarCore::Event::Ptr(new KCalendarCore::Event());
    event->setDtStart(QDateTime(today.addDays(100), QTime::fromString(QStringLiteral("12:00"), QStringLiteral("hh:mm"))));
    event->setDtEnd(QDateTime(today.addDays(106), QTime::fromString(QStringLiteral("12:00"), QStringLiteral("hh:mm"))));
    event->setSummary(multiDayWithTimeFuture);
    QVERIFY(cal->addEvent(event));
    for (int i = 100; i <= 106; ++i) {
        SummaryEventInfo::List events5 = SummaryEventInfo::eventsForDate(today.addDays(i), cal);
        QCOMPARE(1, events5.size());
        const SummaryEventInfo *ev5 = events5.at(0);
        /*qDebug() << ev5->summaryText;
        qDebug() << ev5->daysToGo;
        qDebug() << i;*/

        QCOMPARE(ev5->summaryText, QString(multiDayWithTimeFuture + QString::fromLatin1(" (%1/7)").arg(i - 100 + 1)));
        QCOMPARE(ev5->daysToGo, QStringLiteral("in %1 days").arg(i));

        qDeleteAll(events5);
    }

    QString multiDayAllDayInFuture = QStringLiteral("Multiday, allday, in future");
    int multiDayFuture = 30;
    event = KCalendarCore::Event::Ptr(new KCalendarCore::Event());
    event->setDtStart(QDateTime(today.addDays(multiDayFuture)));
    event->setAllDay(true);
    event->setDtEnd(QDateTime(today.addDays(multiDayFuture + 5)));
    event->setSummary(multiDayAllDayInFuture);
    QVERIFY(cal->addEvent(event));

    event = KCalendarCore::Event::Ptr(new KCalendarCore::Event());
    event->setDtStart(QDateTime(today.addDays(2), QTime::fromString(QStringLiteral("12:00"), QStringLiteral("hh:mm"))));
    event->setDtEnd(QDateTime(today.addDays(5), QTime::fromString(QStringLiteral("12:00"), QStringLiteral("hh:mm"))));
    event->setSummary(QStringLiteral("Multiday, time specified, in future"));
    QVERIFY(cal->addEvent(event));

    QString multiDayAllDayStartingToday = QStringLiteral("Multiday, allday, starting today");
    event = KCalendarCore::Event::Ptr(new KCalendarCore::Event());
    event->setDtStart(QDateTime(today));
    event->setDtEnd(QDateTime(today.addDays(5)));
    event->setAllDay(true);
    event->setSummary(multiDayAllDayStartingToday);
    QVERIFY(cal->addEvent(event));

    event = KCalendarCore::Event::Ptr(new KCalendarCore::Event());
    event->setDtStart(QDateTime(today.addDays(-10), QTime::fromString(QStringLiteral("12:00"), QStringLiteral("hh:mm"))));
    event->setDtEnd(QDateTime(today.addDays(-5), QTime::fromString(QStringLiteral("10:00"), QStringLiteral("hh:mm"))));
    event->setSummary(QStringLiteral("Some event in the past"));
    QVERIFY(cal->addEvent(event));

    const SummaryEventInfo::List eventsToday = SummaryEventInfo::eventsForDate(today, cal);
    QCOMPARE(3, eventsToday.size());
    for (const SummaryEventInfo *ev : eventsToday) {
        if (ev->summaryText == multidayWithTimeInProgress + QLatin1StringView(" (2/7)")) {
            QCOMPARE(ev->timeRange,
                     QStringLiteral("%1 - %2").arg(QLocale::system().toString(QTime(0, 0), QLocale::ShortFormat),
                                                   QLocale::system().toString(QTime(23, 59), QLocale::ShortFormat)));
            QCOMPARE(ev->startDate, QStringLiteral("Today"));
            QCOMPARE(ev->daysToGo, QStringLiteral("now"));
            QCOMPARE(ev->makeBold, true);
        } else if (ev->summaryText == multiDayAllDayStartingToday) {
            QVERIFY(ev->timeRange.isEmpty());
            QCOMPARE(ev->startDate, QStringLiteral("Today"));
            QCOMPARE(ev->daysToGo, QStringLiteral("all day"));
            QCOMPARE(ev->makeBold, true);
        } else if (ev->summaryText == multiDayAllDayStartingYesterday) {
            QVERIFY(ev->timeRange.isEmpty());
            QCOMPARE(ev->startDate, QStringLiteral("Today"));
            QCOMPARE(ev->daysToGo, QStringLiteral("all day"));
            QCOMPARE(ev->makeBold, true);
        } else {
            qDebug() << "Unexpected " << ev->summaryText << ev->startDate << ev->timeRange << ev->daysToGo;
            QVERIFY(false); // unexpected event!
        }
    }

    SummaryEventInfo::List events2 = SummaryEventInfo::eventsForDate(today.addDays(multiDayFuture), cal);
    QCOMPARE(1, events2.size());
    SummaryEventInfo *ev1 = events2.at(0);
    QCOMPARE(ev1->summaryText, multiDayAllDayInFuture);
    QVERIFY(ev1->timeRange.isEmpty());
    QCOMPARE(ev1->startDate, QLocale::system().toString(today.addDays(multiDayFuture)));
    QCOMPARE(ev1->daysToGo, QString::fromLatin1("in %1 days").arg(multiDayFuture));
    QCOMPARE(ev1->makeBold, false);
    // Make sure multiday is only displayed once
    for (int i = 1; i < 30; ++i) {
        const SummaryEventInfo::List events3 = SummaryEventInfo::eventsForDate(today.addDays(multiDayFuture + i), cal);
        for (SummaryEventInfo *ev : events3) {
            QVERIFY(ev->summaryText.contains(multiDayAllDayInFuture));
        }
        qDeleteAll(events3);
    }

    qDeleteAll(eventsToday);
    qDeleteAll(events2);
}

void SummaryEventTester::test_eventsForRange_data()
{
    QTest::addColumn<int>("start"); // event start, in days from now
    QTest::addColumn<int>("end"); // event end, in days from now
    QTest::addColumn<bool>("inside"); // is the event inside the range (today until 7 days from now)?

    QTest::newRow("completely in the past") << -5 << -2 << false;
    QTest::newRow("fully inside the range") << 1 << 3 << true;
    QTest::newRow("completely after the range") << 8 << 10 << false;
    QTest::newRow("start in the past, end inside") << -2 << 3 << true;
    QTest::newRow("start inside, end after the range") << 2 << 10 << true;
    QTest::newRow("start in the past, end after the range") << -2 << 10 << true;
}

void SummaryEventTester::test_eventsForRange()
{
    QFETCH(int, start);
    QFETCH(int, end);
    QFETCH(bool, inside);

    QDate today = QDate::currentDate();

    KCalendarCore::MemoryCalendar *cal = new KCalendarCore::MemoryCalendar(QTimeZone::systemTimeZone());

    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    event->setDtStart(QDateTime(today.addDays(start)));
    event->setDtEnd(QDateTime(today.addDays(end)));
    event->setAllDay(true);
    QVERIFY(cal->addEvent(event));

    SummaryEventInfo::List events = SummaryEventInfo::eventsForRange(today, today.addDays(7), cal);
    QCOMPARE(events.count() == 1, inside);

    qDeleteAll(events);
}

#include "moc_summaryeventtest.cpp"
