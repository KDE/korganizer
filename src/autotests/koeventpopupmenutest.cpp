/*
  SPDX-FileCopyrightText: 2014 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "koeventpopupmenutest.h"
#include "koeventpopupmenu.h"

#include <Akonadi/Notes/NoteUtils>

#include <CalendarSupport/NoteEditDialog>

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
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(nullptr);

    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    Akonadi::Item item;
    item.setMimeType(KCalendarCore::Event::eventMimeType());
    item.setPayload<KCalendarCore::Event::Ptr>(event);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    auto createevent = menu.findChild<QAction *>(QStringLiteral("createevent"));
    createevent->trigger();

    auto dlg = menu.findChild<IncidenceEditorNG::IncidenceDialog *>(QStringLiteral("incidencedialog"));
    QVERIFY(!dlg);
}

void KoEventPopupMenuTest::createTodoFromTodo()
{
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(nullptr);

    KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());
    Akonadi::Item item;
    item.setMimeType(KCalendarCore::Todo::todoMimeType());
    item.setPayload<KCalendarCore::Todo::Ptr>(todo);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    auto createtodo = menu.findChild<QAction *>(QStringLiteral("createtodo"));
    createtodo->trigger();

    auto dlg = menu.findChild<IncidenceEditorNG::IncidenceDialog *>(QStringLiteral("incidencedialog"));
    QVERIFY(!dlg);
}

void KoEventPopupMenuTest::createEventFromTodo()
{
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(nullptr);

    KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());
    Akonadi::Item item;
    item.setMimeType(KCalendarCore::Todo::todoMimeType());
    item.setPayload<KCalendarCore::Todo::Ptr>(todo);

    QDateTime start, end;
    QString summary(QStringLiteral("a test"));
    start = QDateTime::fromSecsSinceEpoch(1402593346);
    end = QDateTime::fromSecsSinceEpoch(1403593346);
    todo->setDtStart(start);
    todo->setDtDue(end);
    todo->setSummary(summary);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    auto createevent = menu.findChild<QAction *>(QStringLiteral("createevent"));
    createevent->trigger();
    auto dlg = menu.findChild<IncidenceEditorNG::IncidenceDialog *>();
    QVERIFY(dlg);
    auto editor = menu.findChild<IncidenceEditorNG::IncidenceEditor *>();
    QVERIFY(editor);
    KCalendarCore::Event::Ptr event(editor->incidence<KCalendarCore::Event>());
    QVERIFY(event->uid() != todo->uid());
    QCOMPARE(event->dtStart(), start);
    QCOMPARE(event->dtEnd(), end);
    QCOMPARE(event->allDay(), false);
    QCOMPARE(event->summary(), summary);
}

void KoEventPopupMenuTest::createTodoFromEvent()
{
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(nullptr);

    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    Akonadi::Item item;
    item.setMimeType(KCalendarCore::Event::eventMimeType());
    item.setPayload<KCalendarCore::Event::Ptr>(event);

    QDateTime start, end;
    QString summary(QStringLiteral("a test"));
    start = QDateTime::fromSecsSinceEpoch(1402593346);
    end = QDateTime::fromSecsSinceEpoch(1403593346);
    event->setDtStart(start);
    event->setDtEnd(end);
    event->setSummary(summary);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    auto createtodo = menu.findChild<QAction *>(QStringLiteral("createtodo"));
    createtodo->trigger();
    auto dlg = menu.findChild<IncidenceEditorNG::IncidenceDialog *>();
    QVERIFY(dlg);
    auto editor = menu.findChild<IncidenceEditorNG::IncidenceEditor *>();
    QVERIFY(editor);
    KCalendarCore::Todo::Ptr todo(editor->incidence<KCalendarCore::Todo>());
    QVERIFY(todo->uid() != event->uid());
    QCOMPARE(todo->dtStart(), start);
    QCOMPARE(todo->dtDue(), end);
    QCOMPARE(todo->allDay(), false);
    QCOMPARE(todo->summary(), summary);
}

void KoEventPopupMenuTest::createNoteFromEvent()
{
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(nullptr);

    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    Akonadi::Item item;
    item.setMimeType(KCalendarCore::Event::eventMimeType());
    item.setPayload<KCalendarCore::Event::Ptr>(event);

    QDateTime start, end;
    QString summary(QStringLiteral("A test"));
    QString description(QStringLiteral("A long description"));
    start = QDateTime::fromSecsSinceEpoch(1402593346);
    end = QDateTime::fromSecsSinceEpoch(1403593346);
    event->setDtStart(start);
    event->setDtEnd(end);
    event->setSummary(summary);
    event->setDescription(description, true);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    auto createnoteforevent = menu.findChild<QAction *>(QStringLiteral("createnoteforevent"));
    auto noteedit = menu.findChild<CalendarSupport::NoteEditDialog *>();
    QVERIFY(!noteedit);
    createnoteforevent->trigger();
    noteedit = menu.findChild<CalendarSupport::NoteEditDialog *>();
    QVERIFY(noteedit);
    Akonadi::NoteUtils::NoteMessageWrapper note(noteedit->note());
    QCOMPARE(note.title(), summary);
    QCOMPARE(note.text(), description);
    QCOMPARE(note.textFormat(), Qt::RichText);
    QCOMPARE(note.attachments().count(), 1);
    QCOMPARE(note.attachments().at(0).mimetype(), KCalendarCore::Event::eventMimeType());
    QCOMPARE(note.attachments().at(0).url(), (QUrl)item.url());
    QCOMPARE(note.attachments().at(0).data(), QByteArray());
}

void KoEventPopupMenuTest::createNoteFromTodo()
{
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(nullptr);

    KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());
    Akonadi::Item item;
    item.setMimeType(KCalendarCore::Todo::todoMimeType());
    item.setPayload<KCalendarCore::Todo::Ptr>(todo);

    QDateTime start, end;
    QString summary(QStringLiteral("a test"));
    QString description(QStringLiteral("A long description"));
    start = QDateTime::fromSecsSinceEpoch(1402593346);
    end = QDateTime::fromSecsSinceEpoch(1403593346);
    todo->setDtStart(start);
    todo->setDtDue(end);
    todo->setSummary(summary);
    todo->setDescription(description);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    auto createnotefortodo = menu.findChild<QAction *>(QStringLiteral("createnotefortodo"));

    auto noteedit = menu.findChild<CalendarSupport::NoteEditDialog *>();
    QVERIFY(!noteedit);
    createnotefortodo->trigger();
    noteedit = menu.findChild<CalendarSupport::NoteEditDialog *>();
    QVERIFY(noteedit);
    Akonadi::NoteUtils::NoteMessageWrapper note(noteedit->note());
    QCOMPARE(note.title(), summary);
    QCOMPARE(note.text(), description);
    QCOMPARE(note.attachments().count(), 1);
    QCOMPARE(note.attachments().at(0).mimetype(), KCalendarCore::Todo::todoMimeType());
    QCOMPARE(note.attachments().at(0).url(), (QUrl)item.url());
    QCOMPARE(note.attachments().at(0).data(), QByteArray());
}

void KoEventPopupMenuTest::defaultMenuEventVisible()
{
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(nullptr);

    KCalendarCore::Event::Ptr event(new KCalendarCore::Event());
    Akonadi::Item item;
    item.setMimeType(KCalendarCore::Event::eventMimeType());
    item.setPayload<KCalendarCore::Event::Ptr>(event);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    auto createevent = menu.findChild<QAction *>(QStringLiteral("createevent"));
    auto createnoteforevent = menu.findChild<QAction *>(QStringLiteral("createnoteforevent"));
    auto createtodo = menu.findChild<QAction *>(QStringLiteral("createtodo"));
    auto createnotefortodo = menu.findChild<QAction *>(QStringLiteral("createnotefortodo"));
    QVERIFY(!createevent->isVisible());
    QVERIFY(createnoteforevent->isVisible());
    QVERIFY(createtodo->isVisible());
    QVERIFY(!createnotefortodo->isVisible());
}

void KoEventPopupMenuTest::defaultMenuTodoVisible()
{
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(nullptr);

    KCalendarCore::Todo::Ptr todo(new KCalendarCore::Todo());

    Akonadi::Item item;
    item.setMimeType(KCalendarCore::Todo::todoMimeType());
    item.setPayload<KCalendarCore::Todo::Ptr>(todo);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    auto createevent = menu.findChild<QAction *>(QStringLiteral("createevent"));
    auto createnoteforevent = menu.findChild<QAction *>(QStringLiteral("createnoteforevent"));
    auto createtodo = menu.findChild<QAction *>(QStringLiteral("createtodo"));
    auto createnotefortodo = menu.findChild<QAction *>(QStringLiteral("createnotefortodo"));
    QVERIFY(createevent->isVisible());
    QVERIFY(!createnoteforevent->isVisible());
    QVERIFY(!createtodo->isVisible());
    QVERIFY(createnotefortodo->isVisible());
}

QTEST_MAIN(KoEventPopupMenuTest)
