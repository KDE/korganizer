/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef KORG_VIEWS_KOTODOVIEWVIEW_H
#define KORG_VIEWS_KOTODOVIEWVIEW_H

#include <QTreeView>
#include <QTimer>

class KMenu;

class KOTodoViewView : public QTreeView
{
  Q_OBJECT

  public:
    KOTodoViewView( QWidget *parent = 0 );

#ifdef __GNUC__
#warning QTreeView should now set State_Editing correctly, remove the workaround
#endif
    bool isEditing( const QModelIndex &index ) const;

    virtual bool eventFilter( QObject *watched, QEvent *event );

  protected:
    virtual QModelIndex moveCursor( CursorAction cursorAction, Qt::KeyboardModifiers modifiers );
    virtual void mousePressEvent( QMouseEvent * );
    virtual void mouseReleaseEvent( QMouseEvent * );
    virtual void mouseMoveEvent( QMouseEvent * );

  private:
    QModelIndex getNextEditableIndex( const QModelIndex &cur, int inc );

    KMenu *mHeaderPopup;
    QList<QAction *> mColumnActions;
    QTimer mExpandTimer;
    bool mIgnoreNextMouseRelease;

  private slots:
    void toggleColumnHidden( QAction *action );
    void expandParent();
};

#endif
