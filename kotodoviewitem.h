/*
    This file is part of KOrganizer.
    Copyright (c) 2000, 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#ifndef KOTODOVIEWITEM_H
#define KOTODOVIEWITEM_H
// $Id$

#include <qfont.h>
#include <qfontmetrics.h>
#include <qlineedit.h>
#include <qptrlist.h>
#include <qstrlist.h>
#include <qlistbox.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qmap.h>
#include <qlistview.h>

#include <libkcal/calendar.h>
#include <libkcal/todo.h>

using namespace KCal;

/**
  This class provides a way of displaying a single Event of Todo-Type in a
  KTodoView.
  
  @author Cornelius Schumacher <schumacher@kde.org>
  @see KOTodoView
*/
class KOTodoViewItem : public QObject, public QCheckListItem
{
    Q_OBJECT
  public:
    /**
      Constructor.

      @param parent is the list view to which this item belongs.
      @param ev is the event to have the item display information for.
    */
    KOTodoViewItem(QListView *parent, Todo *todo);
    KOTodoViewItem(KOTodoViewItem *parent, Todo *todo);
    virtual ~KOTodoViewItem() {}

    void construct();

    Todo *todo() { return mTodo; }

    QString key(int, bool) const;

    void setSortKey(int column,const QString &key);
    
    bool isAlternate();

    virtual void paintCell(QPainter *p, const QColorGroup &cg,
      int column, int width, int alignment);

  signals:
    void isModified(bool);

  protected:
#if QT_VERSION >= 300
    void paintBranches(QPainter *p,const QColorGroup & cg,int w,int y,int h);
#else
#endif
  virtual void stateChange(bool);

  private:
    Todo *mTodo;

    QMap<int,QString> mKeyMap;
    uint m_odd : 1;
    uint m_known : 1;
    uint m_unused : 30;

};

#endif
