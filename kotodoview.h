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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOTODOVIEW_H
#define KOTODOVIEW_H

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
#include <klistview.h>

#include <libkcal/calendar.h>
#include <libkcal/todo.h>

#include <korganizer/baseview.h>

#include "kotodoviewitem.h"
#include "koprefs.h"
#include "koglobals.h"

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

class DocPrefs;

class KOTodoListView : public KListView
{
    Q_OBJECT
  public:
    KOTodoListView(Calendar *,QWidget *parent=0,const char *name=0);
    virtual ~KOTodoListView() {}

  signals:
    void todoDropped(Todo *);

  protected:
    void contentsDragEnterEvent(QDragEnterEvent *);
    void contentsDragMoveEvent(QDragMoveEvent *);
    void contentsDragLeaveEvent(QDragLeaveEvent *);
    void contentsDropEvent(QDropEvent *);

    void contentsMousePressEvent(QMouseEvent *);
    void contentsMouseMoveEvent(QMouseEvent *);
    void contentsMouseReleaseEvent(QMouseEvent *);
    void contentsMouseDoubleClickEvent(QMouseEvent *);

  private:
    Calendar *mCalendar;

    QPoint mPressPos;
    bool mMousePressed;
    QListViewItem *mOldCurrent;
};


/**
  This is the line-edit on top of the todoview for fast addition of new todos
*/
class KOQuickTodo : public QLineEdit
{
  public:
    KOQuickTodo(QWidget *parent=0);
  protected:
    void focusInEvent(QFocusEvent *ev);
    void focusOutEvent(QFocusEvent *ev);
};


/**
  This class provides a multi-column list view of todo events.

  @short multi-column list view of todo events.
  @author Cornelius Schumacher <schumacher@kde.org>
*/
class KOTodoView : public KOrg::BaseView
{
    Q_OBJECT
  public:
    KOTodoView(Calendar *, QWidget* parent=0, const char* name=0 );
    ~KOTodoView();

    QPtrList<Incidence> selectedIncidences();
    QPtrList<Todo> selectedTodos();

    DateList selectedDates()
      {DateList q;
       return q;}

    /** Return number of shown dates. TodoView does not show dates, */
    int currentDateCount() { return 0; }

    void printPreview(CalPrinter *calPrinter, const QDate &fd, const QDate &td);

    void setDocumentId( const QString & );

    void saveLayout(KConfig *config, const QString &group) const;
    void restoreLayout(KConfig *config, const QString &group);
    /** Create a popup menu to set categories */
    QPopupMenu *getCategoryPopupMenu (KOTodoViewItem *todoItem);

  public slots:
    void updateView();
    void updateConfig();

    void changeEventDisplay(Event *, int);

    void showDates(const QDate &start, const QDate &end);
    void showEvents(QPtrList<Event> eventList);

    void clearSelection();

    void editItem(QListViewItem *item,const QPoint &,int);
    void showItem(QListViewItem *item,const QPoint &,int);
    void popupMenu(QListViewItem *item,const QPoint &,int);
    void newTodo();
    void newSubTodo();
    void showTodo();
    void editTodo();
    void deleteTodo();

    void setNewPriority(int);
    void setNewPercentage(int);
    void changedCategories(int);

    void purgeCompleted();

    void itemClicked(QListViewItem *);
    void itemStateChanged(QListViewItem *);
    void modified(bool);

  signals:
    void newTodoSignal();
    void newSubTodoSignal(Todo *);
    void showTodoSignal(Todo *);

    void editTodoSignal(Todo *);
    void deleteTodoSignal(Todo *);
    void todoModifiedSignal (Todo *, int);

    void isModified(bool);

    void purgeCompletedSignal();

  protected slots:
    void processSelectionChange();
    void addQuickTodo();

  private:
    /*
     * the TodoEditor approach is rather unscaling in the long
     * run.
     * Korganizer keeps it in memory and we need to update
     * 1. make KOTodoViewItem a QObject again?
     * 2. add a public method for setting one todo modified?
     * 3. add a private method for setting a todo modified + friend here?
     *  -- zecke 2002-07-08
     */
    friend class KOTodoViewItem;
    void setTodoModified( Todo* );
    QMap<Todo *,KOTodoViewItem *>::ConstIterator insertTodoItem(Todo *todo);
    void restoreItemState( QListViewItem * );

    KOTodoListView *mTodoListView;
    QPopupMenu *mItemPopupMenu;
    QPopupMenu *mPopupMenu;
    QPopupMenu *mPriorityPopupMenu;
    QPopupMenu *mPercentageCompletedPopupMenu;
    QPopupMenu *mCategoryPopupMenu;

    QMap<int, int> mPercentage;
    QMap<int, int> mPriority;
    QMap<int, QString> mCategory;

    KOTodoViewItem *mActiveItem;

    QMap<Todo *,KOTodoViewItem *> mTodoMap;

    DocPrefs *mDocPrefs;
    QString mCurrentDoc;
    KOQuickTodo *mQuickAdd;
};

#endif
