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

#ifndef NOTEEDITDIALOG_H
#define NOTEEDITDIALOG_H

#include <QDialog>

#include <AkonadiCore/Collection>
#include <AkonadiCore/Item>
#include <Akonadi/Notes/NoteUtils>
#include <KMime/KMimeMessage>

class QLineEdit;
class KRichTextEdit;
class QAbstractItemModel;
namespace Akonadi
{
class CollectionComboBox;
}

class NoteEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NoteEditDialog(QWidget *parent = Q_NULLPTR);
    ~NoteEditDialog();

    Akonadi::Collection collection() const;
    void setCollection(const Akonadi::Collection &value);

    void load(const Akonadi::Item &item);
    KMime::Message::Ptr note() const;

    //Used for tests
    static QAbstractItemModel *_k_noteEditStubModel;

public Q_SLOTS:
    void accept() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotCollectionChanged(int);
    void slotUpdateButtons();

Q_SIGNALS:
    void createNote(const Akonadi::Item &note, const Akonadi::Collection &collection);
    void collectionChanged(const Akonadi::Collection &col);

private:
    Akonadi::Collection mCollection;
    Akonadi::Item mItem;
    QLineEdit *mNoteTitle;
    KRichTextEdit *mNoteText;
    Akonadi::CollectionComboBox *mCollectionCombobox;
};

#endif // NOTEEDITDIALOG_H
