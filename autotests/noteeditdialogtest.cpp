/*
 * Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * As a special exception, permission is given to link this program
 * with any edition of Qt, and distribute the resulting executable,
 * without including the source code for Qt in the source distribution.
 */

#include "noteeditdialogtest.h"
#include "dialog/noteeditdialog.h"
#include <AkonadiCore/Collection>
#include <AkonadiWidgets/CollectionComboBox>
#include <AkonadiCore/EntityTreeModel>

#include <KMime/KMimeMessage>
#include <QStandardItemModel>
#include <QPushButton>
#include <KRichTextEdit>
#include <QTest>
#include <QSignalSpy>
#include <qtestkeyboard.h>
#include <qtestmouse.h>
#include <kpimtextedit/richtexteditorwidget.h>
#include <kpimtextedit/richtexteditor.h>

#include <QLineEdit>
#include <QTextEdit>
#include <QShortcut>

NoteEditDialogTest::NoteEditDialogTest()
{
    qRegisterMetaType<Akonadi::Collection>();
    qRegisterMetaType<Akonadi::Item>();
    qRegisterMetaType<KMime::Message::Ptr>();

    QStandardItemModel *model = new QStandardItemModel;
    for (int id = 42; id < 51; ++id) {
        Akonadi::Collection collection(id);
        collection.setRights(Akonadi::Collection::AllRights);
        collection.setName(QString::number(id));
        collection.setContentMimeTypes(QStringList() << Akonadi::NoteUtils::noteMimeType());

        QStandardItem *item = new QStandardItem(collection.name());
        item->setData(QVariant::fromValue(collection),
                      Akonadi::EntityTreeModel::CollectionRole);
        item->setData(QVariant::fromValue(collection.id()),
                      Akonadi::EntityTreeModel::CollectionIdRole);

        model->appendRow(item);
    }
    NoteEditDialog::_k_noteEditStubModel = model;
}

void NoteEditDialogTest::shouldHaveDefaultValuesOnCreation()
{
    NoteEditDialog edit;
    QVERIFY(!edit.note());
    QLineEdit *notetitle = edit.findChild<QLineEdit *>(QStringLiteral("notetitle"));
    KPIMTextEdit::RichTextEditorWidget *notetext = edit.findChild<KPIMTextEdit::RichTextEditorWidget *>(QStringLiteral("notetext"));
    QPushButton *ok = edit.findChild<QPushButton *>(QStringLiteral("save-button"));
    QVERIFY(notetitle);
    QCOMPARE(notetitle->text(), QString());
    QVERIFY(notetext);
    QCOMPARE(notetext->toPlainText(), QString());
    QVERIFY(ok);
    QCOMPARE(ok->isEnabled(), false);
}

void NoteEditDialogTest::shouldEmitCollectionChanged()
{
    NoteEditDialog edit;
    QSignalSpy spy(&edit, SIGNAL(collectionChanged(Akonadi::Collection)));
    edit.setCollection(Akonadi::Collection(42));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).value<Akonadi::Collection>(), Akonadi::Collection(42));
}

void NoteEditDialogTest::shouldNotEmitWhenCollectionIsNotChanged()
{
    NoteEditDialog edit;
    edit.setCollection(Akonadi::Collection(42));
    QSignalSpy spy(&edit, SIGNAL(collectionChanged(Akonadi::Collection)));
    edit.setCollection(Akonadi::Collection(42));
    QCOMPARE(spy.count(), 0);
}

void NoteEditDialogTest::shouldHaveSameValueAfterSet()
{
    NoteEditDialog edit;

    Akonadi::NoteUtils::NoteMessageWrapper note;
    Akonadi::Item item;
    item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    KMime::Message::Ptr message(note.message());
    item.setPayload(message);

    edit.setCollection(Akonadi::Collection(42));
    edit.load(item);
    QCOMPARE(edit.collection(), Akonadi::Collection(42));
    QCOMPARE(edit.note()->encodedContent(), message->encodedContent());
}

void NoteEditDialogTest::shouldHaveFilledText()
{
    NoteEditDialog edit;

    Akonadi::NoteUtils::NoteMessageWrapper note;
    QString title = QStringLiteral("title");
    QString text = QStringLiteral("text");
    note.setTitle(title);
    note.setText(text);
    Akonadi::Item item;
    item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    item.setPayload(note.message());

    edit.load(item);
    QLineEdit *notetitle = edit.findChild<QLineEdit *>(QStringLiteral("notetitle"));
    KPIMTextEdit::RichTextEditorWidget *notetext = edit.findChild<KPIMTextEdit::RichTextEditorWidget *>(QStringLiteral("notetext"));
    QCOMPARE(notetitle->text(), title);
    QCOMPARE(notetext->toPlainText(), text);
}

void NoteEditDialogTest::shouldHaveRichText()
{
    NoteEditDialog edit;

    Akonadi::NoteUtils::NoteMessageWrapper note;
    QString title = QStringLiteral("title");
    QString text = QStringLiteral("text");
    note.setTitle(title);
    note.setText(text, Qt::RichText);
    Akonadi::Item item;
    item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    item.setPayload(note.message());

    edit.load(item);
    KPIMTextEdit::RichTextEditorWidget *notetext = edit.findChild<KPIMTextEdit::RichTextEditorWidget *>(QStringLiteral("notetext"));
    QCOMPARE(notetext->toPlainText(), text);
    QVERIFY(notetext->editor()->acceptRichText());
}

void NoteEditDialogTest::shouldDefaultCollectionIsValid()
{
    NoteEditDialog edit;
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    QVERIFY(akonadicombobox);
    QVERIFY(akonadicombobox->currentCollection().isValid());
}

void NoteEditDialogTest::shouldEmitCollectionChangedWhenCurrentCollectionWasChanged()
{
    NoteEditDialog edit;
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));
    akonadicombobox->setCurrentIndex(0);
    QCOMPARE(akonadicombobox->currentIndex(), 0);
    QSignalSpy spy(&edit, SIGNAL(collectionChanged(Akonadi::Collection)));
    akonadicombobox->setCurrentIndex(3);
    QCOMPARE(akonadicombobox->currentIndex(), 3);
    QCOMPARE(spy.count(), 1);
}

void NoteEditDialogTest::shouldEmitCorrectCollection()
{
    NoteEditDialog edit;
    Akonadi::CollectionComboBox *akonadicombobox = edit.findChild<Akonadi::CollectionComboBox *>(QStringLiteral("akonadicombobox"));

    Akonadi::NoteUtils::NoteMessageWrapper note;
    QString title = QStringLiteral("title");
    QString text = QStringLiteral("text");
    note.setTitle(title);
    note.setText(text);
    Akonadi::Item item;
    item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    item.setPayload(note.message());

    edit.load(item);
    akonadicombobox->setCurrentIndex(3);
    Akonadi::Collection col = akonadicombobox->currentCollection();
    QSignalSpy spy(&edit, SIGNAL(createNote(Akonadi::Item,Akonadi::Collection)));
    QPushButton *ok = edit.findChild<QPushButton *>(QStringLiteral("save-button"));
    QTest::mouseClick(ok, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(1).value<Akonadi::Collection>(), col);
}

void NoteEditDialogTest::shouldNotEmitNoteWhenTitleIsEmpty()
{
    NoteEditDialog edit;
    Akonadi::NoteUtils::NoteMessageWrapper note;
    Akonadi::Item item;
    item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    item.setPayload(note.message());

    edit.load(item);
    QSignalSpy spy(&edit, SIGNAL(createNote(Akonadi::Item,Akonadi::Collection)));
    QPushButton *ok = edit.findChild<QPushButton *>(QStringLiteral("save-button"));

    QLineEdit *notetitle = edit.findChild<QLineEdit *>(QStringLiteral("notetitle"));
    notetitle->setText(QString());
    QTest::mouseClick(ok, Qt::LeftButton);
    QCOMPARE(spy.count(), 0);
    notetitle->setText(QStringLiteral("F"));
    QTest::mouseClick(ok, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
}

void NoteEditDialogTest::shouldNotEmitNoteWhenTextIsEmpty()
{
    NoteEditDialog edit;
    Akonadi::NoteUtils::NoteMessageWrapper note;
    Akonadi::Item item;
    item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    item.setPayload(note.message());

    edit.load(item);
    QSignalSpy spy(&edit, SIGNAL(createNote(Akonadi::Item,Akonadi::Collection)));
    QPushButton *ok = edit.findChild<QPushButton *>(QStringLiteral("save-button"));

    //Need to set title to empty, 'cause NoteUtils uses default title: "New Note"
    QLineEdit *notetitle = edit.findChild<QLineEdit *>(QStringLiteral("notetitle"));
    notetitle->setText(QString());

    QTest::mouseClick(ok, Qt::LeftButton);
    QCOMPARE(spy.count(), 0);
    KPIMTextEdit::RichTextEditorWidget *notetext = edit.findChild<KPIMTextEdit::RichTextEditorWidget *>(QStringLiteral("notetext"));
    notetext->editor()->setText(QStringLiteral("F"));
    QTest::mouseClick(ok, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
}

void NoteEditDialogTest::shouldNoteHasCorrectText()
{
    NoteEditDialog edit;
    Akonadi::NoteUtils::NoteMessageWrapper note;
    QString text(QStringLiteral("text"));
    note.setText(text);
    Akonadi::Item item;
    item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    item.setPayload(note.message());

    edit.load(item);
    QSignalSpy spy(&edit, SIGNAL(createNote(Akonadi::Item,Akonadi::Collection)));
    QPushButton *ok = edit.findChild<QPushButton *>(QStringLiteral("save-button"));

    QTest::mouseClick(ok, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
    Akonadi::NoteUtils::NoteMessageWrapper rNote(spy.at(0).at(0).value<Akonadi::Item>().payload<KMime::Message::Ptr>());
    QCOMPARE(rNote.text(), text);
    KPIMTextEdit::RichTextEditorWidget *notetext = edit.findChild<KPIMTextEdit::RichTextEditorWidget *>(QStringLiteral("notetext"));
    QString text2 = QStringLiteral("F");
    notetext->editor()->setText(text2);
    QTest::mouseClick(ok, Qt::LeftButton);
    QCOMPARE(spy.count(), 2);
    Akonadi::NoteUtils::NoteMessageWrapper r2Note(spy.at(1).at(0).value<Akonadi::Item>().payload<KMime::Message::Ptr>());
    QCOMPARE(r2Note.text(), text2);

}

void NoteEditDialogTest::shouldNoteHasCorrectTitle()
{
    NoteEditDialog edit;
    Akonadi::NoteUtils::NoteMessageWrapper note;
    QString text(QStringLiteral("text"));
    note.setTitle(text);
    Akonadi::Item item;
    item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    item.setPayload(note.message());

    edit.load(item);
    QSignalSpy spy(&edit, SIGNAL(createNote(Akonadi::Item,Akonadi::Collection)));
    QPushButton *ok = edit.findChild<QPushButton *>(QStringLiteral("save-button"));

    QTest::mouseClick(ok, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
    Akonadi::NoteUtils::NoteMessageWrapper rNote(spy.at(0).at(0).value<Akonadi::Item>().payload<KMime::Message::Ptr>());
    QCOMPARE(rNote.title(), text);
    QLineEdit *notetitle = edit.findChild<QLineEdit *>(QStringLiteral("notetitle"));
    QString text2 = QStringLiteral("F");
    notetitle->setText(text2);
    QTest::mouseClick(ok, Qt::LeftButton);
    QCOMPARE(spy.count(), 2);
    Akonadi::NoteUtils::NoteMessageWrapper r2Note(spy.at(1).at(0).value<Akonadi::Item>().payload<KMime::Message::Ptr>());
    QCOMPARE(r2Note.title(), text2);

}

void NoteEditDialogTest::shouldNoteHasCorrectTextFormat()
{
    NoteEditDialog edit;
    Akonadi::NoteUtils::NoteMessageWrapper note;
    QString text(QStringLiteral("text"));
    note.setText(text);
    Akonadi::Item item;
    item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    item.setPayload(note.message());

    edit.load(item);
    QSignalSpy spy(&edit, SIGNAL(createNote(Akonadi::Item,Akonadi::Collection)));
    QPushButton *ok = edit.findChild<QPushButton *>(QStringLiteral("save-button"));

    QTest::mouseClick(ok, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
    Akonadi::NoteUtils::NoteMessageWrapper rNote(spy.at(0).at(0).value<Akonadi::Item>().payload<KMime::Message::Ptr>());
    QCOMPARE(rNote.textFormat(), Qt::PlainText);
    KPIMTextEdit::RichTextEditorWidget *notetext = edit.findChild<KPIMTextEdit::RichTextEditorWidget *>(QStringLiteral("notetext"));
    notetext->editor()->setAcceptRichText(true);
    QTest::mouseClick(ok, Qt::LeftButton);
    QCOMPARE(spy.count(), 2);
    Akonadi::NoteUtils::NoteMessageWrapper r2Note(spy.at(1).at(0).value<Akonadi::Item>().payload<KMime::Message::Ptr>());
    QCOMPARE(r2Note.textFormat(), Qt::RichText);

}

void NoteEditDialogTest::shouldShouldEnabledSaveEditorButton()
{
    NoteEditDialog edit;
    Akonadi::NoteUtils::NoteMessageWrapper note;
    QString text(QStringLiteral("text"));
    note.setTitle(text);
    Akonadi::Item item;
    item.setMimeType(Akonadi::NoteUtils::noteMimeType());
    item.setPayload(note.message());

    edit.load(item);

    QPushButton *ok = edit.findChild<QPushButton *>(QStringLiteral("save-button"));
    QLineEdit *notetitle = edit.findChild<QLineEdit *>(QStringLiteral("notetitle"));

    QCOMPARE(ok->isEnabled(), true);
    notetitle->clear();

    QCOMPARE(ok->isEnabled(), false);
}

QTEST_MAIN(NoteEditDialogTest)
