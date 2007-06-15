/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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
#ifndef KOTODOVIEW_H
#define KOTODOVIEW_H

#include <QMap>
#include <QMenu>
#include <QList>
#include <QEvent>

#include <k3listview.h>

#include <kcal/todo.h>
#include <korganizer/baseview.h>
#include "calprinter.h"

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QMenu;
class QAction;
class QSplitter;
class QMouseEvent;

class KLineEdit;

class KOTodoListView;
class KOTodoListViewQuickSearch;
class KOTodoViewItem;
class KDatePickerPopup;

class DocPrefs;

namespace KCal {
class Incidence;
class Calendar;
}
using namespace KCal;
using namespace KOrg;

#ifdef __GNUC__
#warning port QToolTip usage
#endif
class KOTodoListViewToolTip
{
  public:
    KOTodoListViewToolTip( QWidget *parent, KOTodoListView *lv );

  protected:
    void maybeTip( const QPoint &pos );

  private:
    KOTodoListView *todolist;
};


class KOTodoListView : public K3ListView
{
  Q_OBJECT
  public:
    KOTodoListView( QWidget *parent );
    ~KOTodoListView();

    void setCalendar( Calendar * );

    void setIncidenceChanger( IncidenceChangerBase *changer ) { mChanger = changer; }

  protected:
    virtual bool event( QEvent * );

    void contentsDragEnterEvent( QDragEnterEvent * );
    void contentsDragMoveEvent( QDragMoveEvent * );
    void contentsDragLeaveEvent( QDragLeaveEvent * );
    void contentsDropEvent( QDropEvent * );

    void contentsMousePressEvent( QMouseEvent * );
    void contentsMouseMoveEvent( QMouseEvent * );
    void contentsMouseReleaseEvent( QMouseEvent * );
    void contentsMouseDoubleClickEvent( QMouseEvent * );

  private:
    Calendar *mCalendar;
    KOrg::IncidenceChangerBase *mChanger;

    QPoint mPressPos;
    bool mMousePressed;
    Q3ListViewItem *mOldCurrent;
    KOTodoListViewToolTip *tooltip;
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
    KOTodoView( Calendar *cal, QWidget *parent );
    ~KOTodoView();

    void setCalendar( Calendar * );

    Incidence::List selectedIncidences();
    Todo::List selectedTodos();

    DateList selectedDates() { return DateList(); }

    /** Return number of shown dates. TodoView does not show dates, */
    int currentDateCount() { return 0; }

    CalPrinter::PrintType printType();

    void setDocumentId( const QString & );

    void saveLayout( KConfig *config, const QString &group ) const;
    void restoreLayout( KConfig *config, const QString &group );
    /** Create a popup menu to set categories */
    QMenu *getCategoryPopupMenu( KOTodoViewItem *todoItem );
    void setIncidenceChanger( IncidenceChangerBase *changer );

  public slots:
    void updateView();
    void updateConfig();

    void changeIncidenceDisplay( Incidence *, int );

    void showDates( const QDate &start, const QDate &end );
    void showIncidences( const Incidence::List &incidenceList );

    void clearSelection();

    void editItem( Q3ListViewItem *item, const QPoint &, int );
    void editItem( Q3ListViewItem *item );
    void showItem( Q3ListViewItem *item, const QPoint &, int );
    void showItem( Q3ListViewItem *item );
    void popupMenu( Q3ListViewItem *item, const QPoint &, int );
    void newTodo();
    void newSubTodo();
    void showTodo();
    void editTodo();
    void printTodo();
    void deleteTodo();

    void setNewPercentage( KOTodoViewItem *item, int percentage );

    void setNewPriority( QAction* );
    void setNewPercentage( QAction* );
    void changedCategories( QAction* );
    void setNewDate( const QDate& );
    void copyTodoToDate( const QDate& );

    void purgeCompleted();

    void itemStateChanged( Q3ListViewItem * );

    void setNewPercentageDelayed( KOTodoViewItem *item, int percentage );
    void processDelayedNewPercentage();

    void updateCategories();

  signals:
    void unSubTodoSignal();
    void unAllSubTodoSignal();

    void purgeCompletedSignal();

  protected slots:
    void processSelectionChange();
    void addQuickTodo();
    void removeTodoItems();

  protected:
    void fillViews();

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

    QMap<Todo *, KOTodoViewItem *>::ConstIterator insertTodoItem( Todo *todo );
    bool scheduleRemoveTodoItem( KOTodoViewItem *todoItem );
    void restoreListViewState( Q3ListView * );
    void saveListViewState( Q3ListView * );
    void setupListViews();

    QStackedWidget   *mWidgetStack;
    QSplitter      *mSplitter;
    KOTodoListView *mMyTodoListView;
    KOTodoListView *mOneTodoListView;
    KOTodoListView *mYourTodoListView;
    KOTodoListView *mOtherTodoListView;

    enum { eOneListView, eSplitListViews };

    QMenu *mItemPopupMenu;
    QMenu *mPopupMenu;
    QMenu *mPriorityPopupMenu;
    QMenu *mPercentageCompletedPopupMenu;
    QMenu *mCategoryPopupMenu;
    KDatePickerPopup *mMovePopupMenu;
    KDatePickerPopup *mCopyPopupMenu;

    QMap<QAction*, int> mPercentage;
    QMap<QAction*, int> mPriority;
    QMap<QAction*, QString> mCategory;

    KOTodoViewItem *mActiveItem;

    QMap<Todo *,KOTodoViewItem *> mTodoMap;
    QList<KOTodoViewItem*> mItemsToDelete;
    QList< QPair<KOTodoViewItem *, int> > mPercentChangedMap;

    DocPrefs *mDocPrefs;
    QString mCurrentDoc;
    KLineEdit *mQuickAdd;
    KOTodoListViewQuickSearch *mSearchToolBar;

    QStringList mAllEmailAddrs;
    // List of all actions that should be disabled when no item is selected
    QList<QAction*> mActionsOnSelection;
    QAction *mMakeThisTodoIndependent;
    QAction *mMakeChildTodosIndependent;

  public:
    enum {
      eSummaryColumn = 0,
      eRecurColumn = 1,
      ePriorityColumn = 2,
      ePercentColumn = 3,
      eDueDateColumn = 4,
      eCategoriesColumn = 5,
      eDescriptionColumn = 6
    };
/*    enum {
      ePopupEdit = 1300,
      ePopupDelete = 1301,
      ePopupMoveTo = 1302,
      ePopupCopyTo = 1303,
      ePopupUnSubTodo = 1304,
      ePopupUnAllSubTodo = 1305
    };*/

};

#endif
