/*
  SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "koeventpopupmenutest.h"
#include "koeventpopupmenu.h"

#include <Akonadi/CollectionCalendar>

#include <IncidenceEditor/IncidenceDialog>
#include <IncidenceEditor/IncidenceEditor-Ng>

#include <QStandardPaths>

#include <QTest>
KoEventPopupMenuTest::KoEventPopupMenuTest(QObject *parent)
    : QObject(parent)
{
    QStandardPaths::setTestModeEnabled(true);
}

void KoEventPopupMenuTest::createEventFromEvent()
{
    auto calendar = Akonadi::CollectionCalendar::Ptr::create(Akonadi::Collection{});
    KOEventPopupMenu menu;

    const KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    Akonadi::Item item;
    item.setMimeType(KCalendarCore::Event::eventMimeType());
    item.setPayload<KCalendarCore::Event::Ptr>(event);

    menu.showIncidencePopup(calendar, item, QDate());
    auto createevent = menu.findChild<QAction *>(QStringLiteral("createevent"));
    createevent->trigger();

    auto dlg = menu.findChild<IncidenceEditorNG::IncidenceDialog *>(QStringLiteral("incidencedialog"));
    QVERIFY(!dlg);
}

void KoEventPopupMenuTest::createTodoFromTodo()
{
    auto calendar = Akonadi::CollectionCalendar::Ptr::create(Akonadi::Collection());
    KOEventPopupMenu menu;

    const KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());
    Akonadi::Item item;
    item.setMimeType(KCalendarCore::Todo::todoMimeType());
    item.setPayload<KCalendarCore::Todo::Ptr>(todo);

    menu.showIncidencePopup(calendar, item, QDate());
    auto createtodo = menu.findChild<QAction *>(QStringLiteral("createtodo"));
    createtodo->trigger();

    auto dlg = menu.findChild<IncidenceEditorNG::IncidenceDialog *>(QStringLiteral("incidencedialog"));
    QVERIFY(!dlg);
}

void KoEventPopupMenuTest::createEventFromTodo()
{
    auto calendar = Akonadi::CollectionCalendar::Ptr::create(Akonadi::Collection());
    KOEventPopupMenu menu;

    const KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());
    Akonadi::Item item;
    item.setMimeType(KCalendarCore::Todo::todoMimeType());
    item.setPayload<KCalendarCore::Todo::Ptr>(todo);

    QDateTime start;
    QDateTime end;
    const QString summary(QStringLiteral("a test"));
    start = QDateTime::fromSecsSinceEpoch(1402593346);
    end = QDateTime::fromSecsSinceEpoch(1403593346);
    todo->setDtStart(start);
    todo->setDtDue(end);
    todo->setSummary(summary);

    menu.showIncidencePopup(calendar, item, QDate());
    auto createevent = menu.findChild<QAction *>(QStringLiteral("createevent"));
    createevent->trigger();
    auto dlg = menu.findChild<IncidenceEditorNG::IncidenceDialog *>();
    QVERIFY(dlg);
    auto editor = menu.findChild<IncidenceEditorNG::IncidenceEditor *>();
    QVERIFY(editor);
    const KCalendarCore::Event::Ptr event(editor->incidence<KCalendarCore::Event>());
    QVERIFY(event->uid() != todo->uid());
    QCOMPARE(event->dtStart(), start);
    QCOMPARE(event->dtEnd(), end);
    QCOMPARE(event->allDay(), false);
    QCOMPARE(event->summary(), summary);
}

void KoEventPopupMenuTest::createTodoFromEvent()
{
    auto calendar = Akonadi::CollectionCalendar::Ptr::create(Akonadi::Collection());
    KOEventPopupMenu menu;

    const KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    Akonadi::Item item;
    item.setMimeType(KCalendarCore::Event::eventMimeType());
    item.setPayload<KCalendarCore::Event::Ptr>(event);

    QDateTime start;
    QDateTime end;
    const QString summary(QStringLiteral("a test"));
    start = QDateTime::fromSecsSinceEpoch(1402593346);
    end = QDateTime::fromSecsSinceEpoch(1403593346);
    event->setDtStart(start);
    event->setDtEnd(end);
    event->setSummary(summary);

    menu.showIncidencePopup(calendar, item, QDate());
    auto createtodo = menu.findChild<QAction *>(QStringLiteral("createtodo"));
    createtodo->trigger();
    auto dlg = menu.findChild<IncidenceEditorNG::IncidenceDialog *>();
    QVERIFY(dlg);
    auto editor = menu.findChild<IncidenceEditorNG::IncidenceEditor *>();
    QVERIFY(editor);
    const KCalendarCore::Todo::Ptr todo(editor->incidence<KCalendarCore::Todo>());
    QVERIFY(todo->uid() != event->uid());
    QCOMPARE(todo->dtStart(), start);
    QCOMPARE(todo->dtDue(), end);
    QCOMPARE(todo->allDay(), false);
    QCOMPARE(todo->summary(), summary);
}

void KoEventPopupMenuTest::defaultMenuEventVisible()
{
    auto calendar = Akonadi::CollectionCalendar::Ptr::create(Akonadi::Collection());
    KOEventPopupMenu menu;

    const KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    Akonadi::Item item;
    item.setMimeType(KCalendarCore::Event::eventMimeType());
    item.setPayload<KCalendarCore::Event::Ptr>(event);

    menu.showIncidencePopup(calendar, item, QDate());
    auto createevent = menu.findChild<QAction *>(QStringLiteral("createevent"));
    auto createtodo = menu.findChild<QAction *>(QStringLiteral("createtodo"));
    QVERIFY(!createevent->isVisible());
    QVERIFY(createtodo->isVisible());
}

void KoEventPopupMenuTest::defaultMenuTodoVisible()
{
    auto calendar = Akonadi::CollectionCalendar::Ptr::create(Akonadi::Collection());
    KOEventPopupMenu menu;

    const KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());

    Akonadi::Item item;
    item.setMimeType(KCalendarCore::Todo::todoMimeType());
    item.setPayload<KCalendarCore::Todo::Ptr>(todo);

    menu.showIncidencePopup(calendar, item, QDate());
    auto createevent = menu.findChild<QAction *>(QStringLiteral("createevent"));
    auto createtodo = menu.findChild<QAction *>(QStringLiteral("createtodo"));
    QVERIFY(createevent->isVisible());
    QVERIFY(!createtodo->isVisible());
}

QTEST_MAIN(KoEventPopupMenuTest)

#include "moc_koeventpopupmenutest.cpp"
