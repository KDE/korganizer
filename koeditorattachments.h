/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOEDITORATTACHMENTS_H
#define KOEDITORATTACHMENTS_H

#include <qwidget.h>

namespace KCal {
class Incidence;
}
using namespace KCal;

class QListViewItem;
class QListView;
class QDragonEnterEvent;
class QDropEvent;

class KOEditorAttachments : public QWidget
{
    Q_OBJECT
  public:
    KOEditorAttachments( int spacing = 8, QWidget *parent = 0,
                         const char *name = 0 );
    ~KOEditorAttachments();

    void addAttachment( const QString &uri,
                        const QString &mimeType = QString::null );

    /** Set widgets to default values */
    void setDefaults();
    /** Read event object and setup widgets accordingly */
    void readIncidence( Incidence * );
    /** Write event settings to event object */
    void writeIncidence( Incidence * );

  protected slots:
    void showAttachment( QListViewItem *item );
    void slotAdd();
    void slotEdit();
    void slotRemove();
    void slotShow();
    void dragEnterEvent( QDragEnterEvent *event );
    void dropEvent( QDropEvent *event );

  private:
    QListView *mAttachments;
};

#endif
