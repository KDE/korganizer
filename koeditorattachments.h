/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOEDITORATTACHMENTS_H
#define KOEDITORATTACHMENTS_H

#include <qwidget.h>
#include <qmap.h>
#include <kdialogbase.h>
#include <kmimetype.h>
#include <kurl.h>
#include <kiconview.h>

#include <set>

class AttachmentListItem;
class AttachmentIconView;

namespace KCal {
class Incidence;
class Attachment;
}

class QCheckBox;
class QIconViewItem;
class QLabel;
class QMimeSource;
class QPushButton;
class QPopupMenu;

class KAction;
class KLineEdit;
class KURLRequester;
class KTempDir;

class AttachmentEditDialog : public KDialogBase
{
  Q_OBJECT
  public:
    AttachmentEditDialog( AttachmentListItem *item, QWidget *parent=0 );

    void accept();

  protected slots:
    void urlSelected( const QString &url );
    void urlChanged( const QString & url );
    virtual void slotApply();

  private:
    friend class KOEditorAttachments;
    KMimeType::Ptr mMimeType;
    AttachmentListItem *mItem;
    QLabel *mTypeLabel, *mIcon;
    QCheckBox *mInline;
    KLineEdit *mLabelEdit;
    KURLRequester *mURLRequester;
};

class KOEditorAttachments : public QWidget
{
    Q_OBJECT
  public:
    KOEditorAttachments( int spacing = 8, QWidget *parent = 0,
                         const char *name = 0 );
    ~KOEditorAttachments();

    void addAttachment( const QString &uri,
                        const QString &mimeType = QString(),
                        const QString &label = QString(),
                        bool binary = false );
    void addAttachment( KCal::Attachment *attachment );
    void addAttachment( const QByteArray &data,
                        const QString &mimeType = QString(),
                        const QString &label = QString() );

    /** Set widgets to default values */
    void setDefaults();
    /** Read event object and setup widgets accordingly */
    void readIncidence( KCal::Incidence * );
    /** Write event settings to event object */
    void writeIncidence( KCal::Incidence * );

    bool hasAttachments();

  protected slots:
    void showAttachment( QIconViewItem *item );
    void slotAdd();
    void slotAddData();
    void slotEdit();
    void slotRemove();
    void slotShow();
    void dragEnterEvent( QDragEnterEvent *event );
    void dragMoveEvent( QDragMoveEvent *event );
    void dropEvent( QDropEvent *event );
    void slotCopy();
    void slotCut();
    void slotPaste();
    void selectionChanged();
    void contextMenu( QIconViewItem* item, const QPoint &pos );
  signals:
    void openURL( const KURL &url );

  protected:
    enum {
      DRAG_COPY = 0,
      DRAG_LINK = 1,
      DRAG_CANCEL = 2
    };

  private:
    friend class AttachmentIconView;
    void handlePasteOrDrop( QMimeSource* source );
    QString randomString( int length ) const;
    AttachmentIconView *mAttachments;
    QPushButton *mRemoveBtn;
    QPopupMenu *mContextMenu, *mAddMenu;
    KAction *mOpenAction, *mCopyAction, *mCutAction;
    KAction *mDeleteAction, *mEditAction;
};


class AttachmentIconView : public KIconView
{
  Q_OBJECT

  friend class KOEditorAttachments;
  public:
    AttachmentIconView( KOEditorAttachments* parent=0 );
    KURL tempFileForAttachment( KCal::Attachment *attachment );
    QDragObject *mimeData();
    ~AttachmentIconView();

  protected:
    QDragObject * dragObject();

    void dragMoveEvent( QDragMoveEvent *event );
    void contentsDragMoveEvent( QDragMoveEvent *event );
    void contentsDragEnterEvent( QDragEnterEvent *event );
    void dragEnterEvent( QDragEnterEvent *event );

  protected slots:

    void handleDrop( QDropEvent *event, const QValueList<QIconDragItem> & list );

  private:
    std::set<KTempDir*> mTempDirs;
    QMap<KCal::Attachment *, KURL> mTempFiles;
    KOEditorAttachments* mParent;
};

#endif
