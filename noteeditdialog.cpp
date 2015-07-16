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

#include "noteeditdialog.h"

#include <KLocalizedString>
#include <KSharedConfig>
#include <QLineEdit>
#include <KRichTextEdit>
#include <QIcon>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>

#include <AkonadiWidgets/CollectionComboBox>
#include <QPushButton>

#include <incidenceeditor-ng/incidencedialogfactory.h>
#include <incidenceeditor-ng/incidencedialog.h>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>
#include "korganizer_debug.h"

QAbstractItemModel *NoteEditDialog::_k_noteEditStubModel = Q_NULLPTR;

NoteEditDialog::NoteEditDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Create Note"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &NoteEditDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    buttonBox->button(QDialogButtonBox::Cancel)->setText(i18nc("@action:discard creating note", "Cancel"));

    mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    mOkButton->setObjectName(QStringLiteral("save-button"));
    mOkButton->setDefault(true);
    mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    mOkButton->setText(i18nc("@action:save note", "Save"));
    mOkButton->setIcon(QIcon::fromTheme(QStringLiteral("view-pim-notes")));
    mOkButton->setEnabled(false);

    QGridLayout *layout = new QGridLayout(mainWidget);
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(2);

    mNoteTitle = new QLineEdit;
    mNoteTitle->setClearButtonEnabled(true);
    mNoteTitle->setObjectName(QStringLiteral("notetitle"));
    mNoteTitle->setFocus();
    connect(mNoteTitle, &QLineEdit::textChanged, this, &NoteEditDialog::slotUpdateButtons);

    mCollectionCombobox = new Akonadi::CollectionComboBox(_k_noteEditStubModel);
    mCollectionCombobox->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    mCollectionCombobox->setMinimumWidth(250);
    mCollectionCombobox->setMimeTypeFilter(QStringList() << Akonadi::NoteUtils::noteMimeType());
    mCollectionCombobox->setObjectName(QStringLiteral("akonadicombobox"));
#ifndef QT_NO_ACCESSIBILITY
    mCollectionCombobox->setAccessibleDescription(i18n("Calendar where the new note will be stored."));
#endif
    mCollectionCombobox->setToolTip(i18n("Calendar where the new note will be stored."));

    connect(mCollectionCombobox, static_cast<void (Akonadi::CollectionComboBox::*)(int)>(&Akonadi::CollectionComboBox::currentIndexChanged), this, &NoteEditDialog::slotCollectionChanged);
    connect(mCollectionCombobox, static_cast<void (Akonadi::CollectionComboBox::*)(int)>(&Akonadi::CollectionComboBox::activated), this, &NoteEditDialog::slotCollectionChanged);

    mNoteText = new KRichTextEdit(parent);
    mNoteText->setObjectName(QStringLiteral("notetext"));
    connect(mNoteText, &KRichTextEdit::textChanged, this, &NoteEditDialog::slotUpdateButtons);

    //First line
    hbox->addWidget(mNoteTitle);
    hbox->addSpacing(5);
    hbox->addWidget(mCollectionCombobox);

    QLabel *lab = new QLabel(i18n("Title:"));
    layout->addWidget(lab, 0, 0);
    layout->addLayout(hbox, 0, 1);

    //Second Line
    lab = new QLabel(i18n("Text:"));
    layout->addWidget(lab, 1, 0);
    layout->addWidget(mNoteText, 1, 1);

    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    readConfig();
}

NoteEditDialog::~NoteEditDialog()
{
    writeConfig();
}

void NoteEditDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "NoteEditDialog");

    const QSize size = group.readEntry("Size", QSize(500, 300));
    if (size.isValid()) {
        resize(size);
    }
}

void NoteEditDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "NoteEditDialog");
    group.writeEntry("Size", size());
    group.sync();
}

void NoteEditDialog::slotUpdateButtons()
{
    if (mNoteTitle->text().trimmed().isEmpty() && mNoteText->toPlainText().isEmpty()) {
        mOkButton->setEnabled(false);
    } else {
        mOkButton->setEnabled(true);
    }
}

Akonadi::Collection NoteEditDialog::collection() const
{
    return mCollection;
}

void NoteEditDialog::slotCollectionChanged(int index)
{
    Q_UNUSED(index);
    setCollection(mCollectionCombobox->currentCollection());
}

void NoteEditDialog::setCollection(const Akonadi::Collection &value)
{
    if (mCollection != value) {
        mCollection = value;
        Q_EMIT collectionChanged(mCollection);
    }
}

void NoteEditDialog::accept()
{
    QDialog::accept();
    const Akonadi::Collection collection = mCollectionCombobox->currentCollection();
    if (!collection.isValid()) {
        qCDebug(KORGANIZER_LOG) << " Collection is not valid";
        return;
    }

    if (mNoteTitle->text().isEmpty() && mNoteText->toPlainText().isEmpty()) {
        qCDebug(KORGANIZER_LOG) << " empty note do not save it";
        return;
    }

    Akonadi::NoteUtils::NoteMessageWrapper note(mItem.payload<KMime::Message::Ptr>());
    note.setTitle(mNoteTitle->text());
    Qt::TextFormat format = Qt::PlainText;
    if (mNoteText->textMode() == KRichTextEdit::Rich) {
        format = Qt::RichText;
    }
    note.setText(mNoteText->textOrHtml(), format);
    mItem.setPayload<KMime::Message::Ptr>(note.message());
    emit createNote(mItem, collection);
}

void NoteEditDialog::load(const Akonadi::Item &item)
{
    mItem = item;
    Akonadi::NoteUtils::NoteMessageWrapper note(item.payload<KMime::Message::Ptr>());
    mNoteText->setTextOrHtml(note.text());
    if (note.textFormat() == Qt::RichText) {
        mNoteText->enableRichTextMode();
    }
    mNoteTitle->setText(note.title());
}

KMime::Message::Ptr NoteEditDialog::note() const
{
    if (mItem.hasPayload<KMime::Message::Ptr>()) {
        return mItem.payload<KMime::Message::Ptr>();
    } else {
        return KMime::Message::Ptr();
    }
}
