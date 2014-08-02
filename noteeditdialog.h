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


#ifndef NOTEEDITDIALOG_H
#define NOTEEDITDIALOG_H

#include <KDialog>

#include <Akonadi/Collection>
#include <Akonadi/Item>
#include <Akonadi/Notes/NoteUtils>
#include <KMime/KMimeMessage>

class KLineEdit;
class KRichTextEdit;
class QAbstractItemModel;
namespace Akonadi {
class CollectionComboBox;
}


class NoteEditDialog : public KDialog
{
    Q_OBJECT
public:
    explicit NoteEditDialog(QWidget *parent = 0);
    ~NoteEditDialog();

    Akonadi::Collection collection() const;
    void setCollection(const Akonadi::Collection &value);

    void load(const Akonadi::Item &item);
    KMime::Message::Ptr note() const;

    //Used for tests
    static QAbstractItemModel *_k_noteEditStubModel;
private Q_SLOTS:
    void slotCollectionChanged(int);
    void slotUpdateButtons();
    virtual void accept();

Q_SIGNALS:
    void createNote(const Akonadi::Item &note, const Akonadi::Collection &collection);
    void collectionChanged(const Akonadi::Collection &col);

private:
    Akonadi::Collection mCollection;
    Akonadi::Item mItem;
    KLineEdit *mNoteTitle;
    KRichTextEdit *mNoteText;
    Akonadi::CollectionComboBox *mCollectionCombobox;
};

#endif // NOTEEDITDIALOG_H
