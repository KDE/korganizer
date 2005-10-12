/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

#include <q3cstring.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QLabel>
#include <Q3ValueList>
#include <QDropEvent>
#include <kdialogbase.h>
#include <kmimetype.h>
#include <kurl.h>

namespace KCal {
class Incidence;
class Attachment;
}

class AttachmentIconItem;
class Q3IconDragItem;
class Q3IconViewItem;
class QLabel;
class QLineEdit;
class KIconView;
class KMenu;
class KURLRequester;

namespace KIO {
class Job;
}

class AttachmentEditDialog : public KDialogBase
{
    Q_OBJECT
  public:
    AttachmentEditDialog( AttachmentIconItem *item, 
                          QWidget *parent, const char *name = 0, 
                          bool modal=true );
    
    void accept();
  
  protected slots:
    void urlChanged( const QString &url );
    virtual void slotApply();

  private:
    KMimeType::Ptr mMimeType;
    AttachmentIconItem *mItem;
    QLabel *mTypeLabel, *mIcon;
    QLineEdit *mLabelEdit;
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
                        const QString &mimeType = QString::null,
                        const QString &label = QString::null,
                        bool local = false );
    void addAttachment( KCal::Attachment *attachment );
    void addAttachment( const QByteArray &data, 
                        const QString &mimeType = QString::null, 
                        const QString &label = QString::null );

    /** Set widgets to default values */
    void setDefaults();
    /** Read event object and setup widgets accordingly */
    void readIncidence( KCal::Incidence * );
    /** Write event settings to event object */
    void writeIncidence( KCal::Incidence * );

    bool hasAttachments();
    
  public slots:
    /** Applies all deferred delete and copy operations */
    void applyChanges();

  protected slots:
    void showAttachment( Q3IconViewItem *item );
    void slotAdd();
    void slotEdit();
    void slotRemove();
    void slotShow();
    void dragEnterEvent( QDragEnterEvent *event );
    void dropEvent( QDropEvent *event );
    void slotItemRenamed ( Q3IconViewItem * item, const QString & text );
    void showAttachmentContextMenu( Q3IconViewItem *item, const QPoint &pos );
    void dropped ( QDropEvent * e, const Q3ValueList<Q3IconDragItem> & lst );
    void copyComplete( KIO::Job *job );
  protected:
    QString generateLocalAttachmentPath( QString filename, 
                                         const KMimeType::Ptr mimeType );
  
  signals:
    void openURL( const KURL &url );

  private:
    KIconView *mAttachments;
    KMenu *mPopupMenu, *mPopupNew;
    QString mUid; // used only to generate attachments' filenames
    KURL::List mDeferredDelete;
    KURL::List mDeferredCopy;
};

#endif
