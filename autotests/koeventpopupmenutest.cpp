/*
  Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "koeventpopupmenutest.h"
#include "noteeditdialog.h"
#include "koeventpopupmenu.h"
#include <incidenceeditor-ng/incidencedialog.h>
#include <incidenceeditor-ng/incidenceattendee.h>
#include <calendarsupport/utils.h>
#include <AkonadiCore/EntityTreeModel>
#include <Akonadi/Notes/NoteUtils>
#include <KCalCore/Event>
#include <KCalCore/Todo>

#include <KMime/KMimeMessage>
#include <QPushButton>
#include <qtest.h>
#include <qtestkeyboard.h>
#include <qtestmouse.h>

#include <QShortcut>
#include <QAction>

KoEventPopupMenuTest::KoEventPopupMenuTest()
{
}

void KoEventPopupMenuTest::createEventFromEvent()
{
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(0);

    KCalCore::Event::Ptr event(new KCalCore::Event());
    Akonadi::Item item;
    item.setMimeType(KCalCore::Event::eventMimeType());
    item.setPayload<KCalCore::Event::Ptr>(event);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    QAction *createevent = menu.findChild<QAction *>(QStringLiteral("createevent"));
    createevent->trigger();

    IncidenceEditorNG::IncidenceDialog *dlg = menu.findChild<IncidenceEditorNG::IncidenceDialog *>(QStringLiteral("incidencedialog"));
    QVERIFY(!dlg);

}

void KoEventPopupMenuTest::createTodoFromTodo()
{
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(0);

    KCalCore::Todo::Ptr todo(new KCalCore::Todo());
    Akonadi::Item item;
    item.setMimeType(KCalCore::Todo::todoMimeType());
    item.setPayload<KCalCore::Todo::Ptr>(todo);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    QAction *createtodo = menu.findChild<QAction *>(QStringLiteral("createtodo"));
    createtodo->trigger();

    IncidenceEditorNG::IncidenceDialog *dlg = menu.findChild<IncidenceEditorNG::IncidenceDialog *>(QStringLiteral("incidencedialog"));
    QVERIFY(!dlg);
}

void KoEventPopupMenuTest::createEventFromTodo()
{
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(0);

    KCalCore::Todo::Ptr todo(new KCalCore::Todo());
    Akonadi::Item item;
    item.setMimeType(KCalCore::Todo::todoMimeType());
    item.setPayload<KCalCore::Todo::Ptr>(todo);

    KDateTime start, end;
    QString summary(QStringLiteral("a test"));
    start.setTime_t(1402593346);
    end.setTime_t(1403593346);
    todo->setDtStart(start);
    todo->setDtDue(end);
    todo->setSummary(summary);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    QAction *createevent = menu.findChild<QAction *>(QStringLiteral("createevent"));
    createevent->trigger();
    IncidenceEditorNG::IncidenceDialog *dlg = qFindChild<IncidenceEditorNG::IncidenceDialog *>(&menu);
    QVERIFY(dlg);
    IncidenceEditorNG::IncidenceEditor *editor = qFindChild<IncidenceEditorNG::IncidenceEditor *>(&menu);
    QVERIFY(editor);
    KCalCore::Event::Ptr event(editor->incidence<KCalCore::Event>());
    QVERIFY(event->uid() != todo->uid());
    QCOMPARE(event->dtStart(), start);
    QCOMPARE(event->dtEnd(), end);
    QCOMPARE(event->allDay(), false);
    QCOMPARE(event->summary(), summary);
}

void KoEventPopupMenuTest::createTodoFromEvent()
{
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(0);

    KCalCore::Event::Ptr event(new KCalCore::Event());
    Akonadi::Item item;
    item.setMimeType(KCalCore::Event::eventMimeType());
    item.setPayload<KCalCore::Event::Ptr>(event);

    KDateTime start, end;
    QString summary(QStringLiteral("a test"));
    start.setTime_t(1402593346);
    end.setTime_t(1403593346);
    event->setDtStart(start);
    event->setDtEnd(end);
    event->setSummary(summary);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    QAction *createtodo = menu.findChild<QAction *>(QStringLiteral("createtodo"));
    createtodo->trigger();
    IncidenceEditorNG::IncidenceDialog *dlg = qFindChild<IncidenceEditorNG::IncidenceDialog *>(&menu);
    QVERIFY(dlg);
    IncidenceEditorNG::IncidenceEditor *editor = qFindChild<IncidenceEditorNG::IncidenceEditor *>(&menu);
    QVERIFY(editor);
    KCalCore::Todo::Ptr todo(editor->incidence<KCalCore::Todo>());
    QVERIFY(todo->uid() != event->uid());
    QCOMPARE(todo->dtStart(), start);
    QCOMPARE(todo->dtDue(), end);
    QCOMPARE(todo->allDay(), false);
    QCOMPARE(todo->summary(), summary);
}

void KoEventPopupMenuTest::createNoteFromEvent()
{
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(0);

    KCalCore::Event::Ptr event(new KCalCore::Event());
    Akonadi::Item item;
    item.setMimeType(KCalCore::Event::eventMimeType());
    item.setPayload<KCalCore::Event::Ptr>(event);

    KDateTime start, end;
    QString summary(QStringLiteral("A test"));
    QString description(QStringLiteral("A long description"));
    start.setTime_t(1402593346);
    end.setTime_t(1403593346);
    event->setDtStart(start);
    event->setDtEnd(end);
    event->setSummary(summary);
    event->setDescription(description, true);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    QAction *createnote = menu.findChild<QAction *>(QStringLiteral("createnote"));
    NoteEditDialog *noteedit = qFindChild<NoteEditDialog *>(&menu);
    QVERIFY(!noteedit);
    createnote->trigger();
    noteedit = qFindChild<NoteEditDialog *>(&menu);
    QVERIFY(noteedit);
    Akonadi::NoteUtils::NoteMessageWrapper note(noteedit->note());
    QCOMPARE(note.title(), summary);
    QCOMPARE(note.text(), description);
    QCOMPARE(note.textFormat(), Qt::RichText);
    QCOMPARE(note.attachments().count(), 1);
    QCOMPARE(note.attachments().at(0).mimetype(), KCalCore::Event::eventMimeType());
    QCOMPARE(note.attachments().at(0).url(), (QUrl)item.url());
    QCOMPARE(note.attachments().at(0).data(), QByteArray());
}

void KoEventPopupMenuTest::createNoteFromTodo()
{
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(0);

    KCalCore::Todo::Ptr todo(new KCalCore::Todo());
    Akonadi::Item item;
    item.setMimeType(KCalCore::Todo::todoMimeType());
    item.setPayload<KCalCore::Todo::Ptr>(todo);

    KDateTime start, end;
    QString summary(QStringLiteral("a test"));
    QString description(QStringLiteral("A long description"));
    start.setTime_t(1402593346);
    end.setTime_t(1403593346);
    todo->setDtStart(start);
    todo->setDtDue(end);
    todo->setSummary(summary);
    todo->setDescription(description);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    QAction *createnote = menu.findChild<QAction *>(QStringLiteral("createnote"));

    NoteEditDialog *noteedit = qFindChild<NoteEditDialog *>(&menu);
    QVERIFY(!noteedit);
    createnote->trigger();
    noteedit = qFindChild<NoteEditDialog *>(&menu);
    QVERIFY(noteedit);
    Akonadi::NoteUtils::NoteMessageWrapper note(noteedit->note());
    QCOMPARE(note.title(), summary);
    QCOMPARE(note.text(), description);
    QCOMPARE(note.attachments().count(), 1);
    QCOMPARE(note.attachments().at(0).mimetype(), KCalCore::Todo::todoMimeType());
    QCOMPARE(note.attachments().at(0).url(), (QUrl)item.url());
    QCOMPARE(note.attachments().at(0).data(), QByteArray());
}

void KoEventPopupMenuTest::defaultMenuEventVisible()
{
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(0);

    KCalCore::Event::Ptr event(new KCalCore::Event());
    Akonadi::Item item;
    item.setMimeType(KCalCore::Event::eventMimeType());
    item.setPayload<KCalCore::Event::Ptr>(event);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    QAction *createevent = menu.findChild<QAction *>(QStringLiteral("createevent"));
    QAction *createnote = menu.findChild<QAction *>(QStringLiteral("createnote"));
    QAction *createtodo = menu.findChild<QAction *>(QStringLiteral("createtodo"));
    QVERIFY(!createevent->isVisible());
    QVERIFY(createnote->isVisible());
    QVERIFY(createtodo->isVisible());
}

void KoEventPopupMenuTest::defaultMenuTodoVisible()
{
    Akonadi::ETMCalendar::Ptr calendar(new Akonadi::ETMCalendar());
    KOEventPopupMenu menu(0);

    KCalCore::Todo::Ptr todo(new KCalCore::Todo());

    Akonadi::Item item;
    item.setMimeType(KCalCore::Todo::todoMimeType());
    item.setPayload<KCalCore::Todo::Ptr>(todo);

    menu.setCalendar(calendar);
    menu.showIncidencePopup(item, QDate());
    QAction *createevent = menu.findChild<QAction *>(QStringLiteral("createevent"));
    QAction *createnote = menu.findChild<QAction *>(QStringLiteral("createnote"));
    QAction *createtodo = menu.findChild<QAction *>(QStringLiteral("createtodo"));
    QVERIFY(createevent->isVisible());
    QVERIFY(createnote->isVisible());
    QVERIFY(!createtodo->isVisible());
}

QTEST_MAIN(KoEventPopupMenuTest)
