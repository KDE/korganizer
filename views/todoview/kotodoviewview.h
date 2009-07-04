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

#ifndef KOTODOVIEWVIEW_H
#define KOTODOVIEWVIEW_H

#include <QTreeView>
#include <QList>

class QWidget;
class QEvent;
class QModelIndex;
class KMenu;
class QAction;

class KOTodoViewView : public QTreeView
{
  Q_OBJECT

  public:
    KOTodoViewView( QWidget *parent = 0 );

#if QT_VERSION >= 0x040600
#ifdef __GNUC__
#warning QTreeView should now set State_Editing correctly, remove the workaround
#endif
#endif
    bool isEditing( const QModelIndex &index ) const;

    virtual bool eventFilter( QObject *watched, QEvent *event );

  protected:
    virtual QModelIndex moveCursor( CursorAction cursorAction, Qt::KeyboardModifiers modifiers );

  private:
    QModelIndex getNextEditableIndex( const QModelIndex &cur, int inc );

    KMenu *mHeaderPopup;
    QList<QAction *> mColumnActions;

  private slots:
    void toggleColumnHidden( QAction *action );
};

#endif
