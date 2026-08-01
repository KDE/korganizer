/*
  SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-FileContributor: Sérgio Martins <sergio.martins@kdab.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "../pastehelper.cpp"

#include <KCalendarCore/MemoryCalendar>
#if KCALENDARCORE_VERSION >= QT_VERSION_CHECK(6, 29, 0)
#include <KCalendarCore/MimeData>
#endif

#include <QClipboard>
#include <QGuiApplication>
#include <QMimeData>
#include <QTest>
#include <QTimeZone>

using namespace KCalendarCore;

namespace
{
class PasteHelperTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPasteAllDayEvent()
    {
#if KCALENDARCORE_VERSION >= QT_VERSION_CHECK(6, 29, 0)
        const Event::Ptr allDayEvent(new Event());
        allDayEvent->setSummary(QStringLiteral("Summary 1"));
        allDayEvent->setDtStart(QDateTime(QDate(2010, 8, 8), {}));
        allDayEvent->setDtEnd(QDateTime(QDate(2010, 8, 9), {}));
        allDayEvent->setAllDay(true);
        const QString originalUid = allDayEvent->uid();
        const bool originalIsAllDay = allDayEvent->allDay();

        Incidence::List incidencesToPaste;
        incidencesToPaste.append(allDayEvent);

        auto mimeData = new QMimeData;
        KCalendarCore::MimeData::populate(mimeData, incidencesToPaste);
        qGuiApp->clipboard()->setMimeData(mimeData);

        Incidence::List pastedIncidences = PasteHelper::pasteIncidences();
        QVERIFY(pastedIncidences.size() == 1);

        const Incidence::Ptr &incidence = pastedIncidences.first();

        QVERIFY(incidence->type() == Incidence::TypeEvent);

        // check if a new uid was generated.
        QVERIFY(incidence->uid() != originalUid);

        // we passed an invalid KDateTime to pasteIncidences() so dates don't change.
        QVERIFY(incidence->allDay() == originalIsAllDay);

        const Event::Ptr pastedEvent = incidence.staticCast<Event>();

        QCOMPARE(pastedEvent->dtStart(), allDayEvent->dtStart());
        QCOMPARE(pastedEvent->dtEnd(), allDayEvent->dtEnd());
        QCOMPARE(pastedEvent->summary(), allDayEvent->summary());
#endif
    }

    void testPasteAllDayEvent2()
    {
#if KCALENDARCORE_VERSION >= QT_VERSION_CHECK(6, 29, 0)
        const Event::Ptr allDayEvent(new Event());
        allDayEvent->setSummary(QStringLiteral("Summary 2"));
        allDayEvent->setDtStart(QDateTime(QDate(2010, 8, 8), {}));
        allDayEvent->setDtEnd(QDateTime(QDate(2010, 8, 9), {}));
        allDayEvent->setAllDay(true);
        const QString originalUid = allDayEvent->uid();

        Incidence::List incidencesToPaste;
        incidencesToPaste.append(allDayEvent);

        auto mimeData = new QMimeData;
        KCalendarCore::MimeData::populate(mimeData, incidencesToPaste);
        qGuiApp->clipboard()->setMimeData(mimeData);

        const QDateTime newDateTime(QDate(2011, 1, 1).startOfDay());
        const uint originalLength = allDayEvent->dtStart().secsTo(allDayEvent->dtEnd());

        // paste at the new time
        Incidence::List pastedIncidences = PasteHelper::pasteIncidences(newDateTime);

        // we only copied one incidence
        QVERIFY(pastedIncidences.size() == 1);

        const Incidence::Ptr &incidence = pastedIncidences.first();

        QVERIFY(incidence->type() == Incidence::TypeEvent);

        // check if a new uid was generated.
        QVERIFY(incidence->uid() != originalUid);

        // the new dateTime didn't have time component
        QVERIFY(incidence->allDay());

        const Event::Ptr pastedEvent = incidence.staticCast<Event>();
        const uint newLength = pastedEvent->dtStart().secsTo(pastedEvent->dtEnd());
        /*
            qDebug() << "originalLength was " << originalLength << "; and newLength is "
                    << newLength << "; old dtStart was " << allDayEvent->dtStart()
                    << " and old dtEnd was " << allDayEvent->dtEnd() << endl
                    << "; new dtStart is " << pastedEvent->dtStart()
                    << " and new dtEnd is " << pastedEvent->dtEnd();
        */
        QCOMPARE(newLength, originalLength);
        QCOMPARE(newDateTime, pastedEvent->dtStart());
        QCOMPARE(allDayEvent->summary(), pastedEvent->summary());
#endif
    }

    void testPasteTodo()
    {
#if KCALENDARCORE_VERSION >= QT_VERSION_CHECK(6, 29, 0)
        const Todo::Ptr todo(new Todo());
        todo->setSummary(QStringLiteral("Summary 1"));
        todo->setDtDue(QDateTime(QDate(2010, 8, 9), {}));

        Incidence::List incidencesToPaste;
        incidencesToPaste.append(todo);

        auto mimeData = new QMimeData;
        KCalendarCore::MimeData::populate(mimeData, incidencesToPaste);
        qGuiApp->clipboard()->setMimeData(mimeData);

        const QDateTime newDateTime(QDate(2011, 1, 1), QTime(10, 10));

        Incidence::List pastedIncidences = PasteHelper::pasteIncidences(newDateTime);
        QVERIFY(pastedIncidences.size() == 1);

        const Incidence::Ptr &incidence = pastedIncidences.first();

        QVERIFY(incidence->type() == Incidence::TypeTodo);

        // check if a new uid was generated.
        QVERIFY(incidence->uid() != todo->uid());

        const Todo::Ptr pastedTodo = incidence.staticCast<Todo>();

        QCOMPARE(newDateTime, pastedTodo->dtDue());
        QCOMPARE(todo->summary(), pastedTodo->summary());
#endif
    }
};
}

QTEST_MAIN(PasteHelperTest) // clipboard() needs GUI

#include "pastehelpertest.moc"
