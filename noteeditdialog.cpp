/*
  Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>*

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

#include "noteeditdialog.h"

#include <KLocalizedString>
#include <KLineEdit>
#include <KRichTextEdit>
#include <KIcon>
#include <KDateTimeEdit>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>

#include <Akonadi/CollectionComboBox>
#include <KPushButton>

#include <incidenceeditor-ng/incidencedialogfactory.h>
#include <incidenceeditor-ng/incidencedialog.h>

QAbstractItemModel *NoteEditDialog::_k_noteEditStubModel = 0;

NoteEditDialog::NoteEditDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption(i18nc("@title:window", "Create Note"));
    setButtons(KDialog::Ok | KDialog::Cancel );
    setButtonText(KDialog::Ok, i18nc("@action:save note", "Create note"));
    setButtonText(KDialog::Cancel, i18nc("@action:discard creating note", "Discard"));
    setButtonIcon(KDialog::Ok, KIcon(QLatin1String("view-pim-notes")));

    enableButtonOk(false);

    QGridLayout *layout = new QGridLayout(mainWidget());

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setMargin(0);
    hbox->setSpacing(2);

    mNoteTitle = new KLineEdit;
    mNoteTitle->setClearButtonShown(true);
    mNoteTitle->setObjectName(QLatin1String("notetitle"));
    mNoteTitle->setFocus();
    connect(mNoteTitle, SIGNAL(textChanged(QString)), SLOT(slotUpdateButtons()));

    mCollectionCombobox = new Akonadi::CollectionComboBox(_k_noteEditStubModel);
    mCollectionCombobox->setAccessRightsFilter(Akonadi::Collection::CanCreateItem);
    mCollectionCombobox->setMinimumWidth(250);
    mCollectionCombobox->setMimeTypeFilter( QStringList() << Akonadi::NoteUtils::noteMimeType() );
    mCollectionCombobox->setObjectName(QLatin1String("akonadicombobox"));
#ifndef QT_NO_ACCESSIBILITY
    mCollectionCombobox->setAccessibleDescription(i18n("Calendar where the new note will be stored."));
#endif
    mCollectionCombobox->setToolTip(i18n("Calendar where the new note will be stored."));

    connect(mCollectionCombobox, SIGNAL(currentIndexChanged(int)), SLOT(slotCollectionChanged(int)));
    connect(mCollectionCombobox, SIGNAL(activated(int)), SLOT(slotCollectionChanged(int)));

    mNoteText = new KRichTextEdit(parent);
    mNoteText->setObjectName(QLatin1String("notetext"));
    connect(mNoteText, SIGNAL(textChanged()), SLOT(slotUpdateButtons()));

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

    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed ) );
}

NoteEditDialog::~NoteEditDialog()
{
}

void NoteEditDialog::slotUpdateButtons()
{
    if (mNoteTitle->text().isEmpty() && mNoteText->toPlainText().isEmpty()) {
        enableButtonOk(false);
    } else {
        enableButtonOk(true);
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
        kDebug()<<" Collection is not valid";
        return;
    }

    if (mNoteTitle->text().isEmpty() && mNoteText->toPlainText().isEmpty()) {
        kDebug() << " empty note do not save it";
        return;
    }

    Akonadi::NoteUtils::NoteMessageWrapper note(mItem.payload<KMime::Message::Ptr>());
    note.setTitle(mNoteTitle->text());
    Qt::TextFormat format= Qt::PlainText;
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
