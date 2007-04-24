/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KOTODOVIEW_H
#define KOTODOVIEW_H

#include <qmap.h>
#include <qtooltip.h>

#include <klistview.h>

#include <libkcal/todo.h>
#include <korganizer/baseview.h>

class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;
class QPopupMenu;

class KOTodoListView;
class KOTodoViewItem;
class KDatePickerPopup;

class DocPrefs;

namespace KPIM {
  class ClickLineEdit;
}
namespace KCal {
class Incidence;
class Calendar;
}
using namespace KCal;
using namespace KOrg;

class KOTodoListViewToolTip : public QToolTip
{
  public:
    KOTodoListViewToolTip( QWidget *parent, KOTodoListView *lv );

  protected:
    void maybeTip( const QPoint &pos );

  private:
    KOTodoListView *todolist;
};


class KOTodoListView : public KListView
{
    Q_OBJECT
  public:
    KOTodoListView( QWidget *parent = 0, const char *name = 0 );
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
    QListViewItem *mOldCurrent;
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
    KOTodoView( Calendar *cal, QWidget *parent = 0, const char *name = 0 );
    ~KOTodoView();

    void setCalendar( Calendar * );

    Incidence::List selectedIncidences();
    Todo::List selectedTodos();

    DateList selectedDates() { return DateList(); }

    /** Return number of shown dates. TodoView does not show dates, */
    int currentDateCount() { return 0; }

    CalPrinterBase::PrintType printType();

    void setDocumentId( const QString & );

    void saveLayout( KConfig *config, const QString &group ) const;
    void restoreLayout( KConfig *config, const QString &group );
    /** Create a popup menu to set categories */
    QPopupMenu *getCategoryPopupMenu( KOTodoViewItem *todoItem );
    void setIncidenceChanger( IncidenceChangerBase *changer );

  public slots:
    void updateView();
    void updateConfig();

    void changeIncidenceDisplay( Incidence *, int );

    void showDates( const QDate &start, const QDate &end );
    void showIncidences( const Incidence::List &incidenceList );

    void clearSelection();

    void editItem( QListViewItem *item, const QPoint &, int );
    void editItem( QListViewItem *item );
    void showItem( QListViewItem *item, const QPoint &, int );
    void showItem( QListViewItem *item );
    void popupMenu( QListViewItem *item, const QPoint &, int );
    void newTodo();
    void newSubTodo();
    void showTodo();
    void editTodo();
    void printTodo();
    void deleteTodo();

    void setNewPercentage( KOTodoViewItem *item, int percentage );

    void setNewPriority( int );
    void setNewPercentage( int );
    void setNewDate( QDate );
    void copyTodoToDate( QDate );
    void changedCategories( int );

    void purgeCompleted();

    void itemStateChanged( QListViewItem * );

    void setNewPercentageDelayed( KOTodoViewItem *item, int percentage );
    void processDelayedNewPercentage();

  signals:
    void unSubTodoSignal();
    void unAllSubTodoSignal();

    void purgeCompletedSignal();

  protected slots:
    void processSelectionChange();
    void addQuickTodo();
    void removeTodoItems();

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

    QMap<Todo *,KOTodoViewItem *>::ConstIterator insertTodoItem( Todo *todo );
    bool scheduleRemoveTodoItem( KOTodoViewItem *todoItem );
    void restoreItemState( QListViewItem * );

    KOTodoListView *mTodoListView;
    QPopupMenu *mItemPopupMenu;
    QPopupMenu *mPopupMenu;
    QPopupMenu *mPriorityPopupMenu;
    QPopupMenu *mPercentageCompletedPopupMenu;
    QPopupMenu *mCategoryPopupMenu;
    KDatePickerPopup *mMovePopupMenu;
    KDatePickerPopup *mCopyPopupMenu;

    QMap<int, int> mPercentage;
    QMap<int, int> mPriority;
    QMap<int, QString> mCategory;

    KOTodoViewItem *mActiveItem;

    QMap<Todo *,KOTodoViewItem *> mTodoMap;
    QPtrList<KOTodoViewItem> mItemsToDelete;
    QValueList< QPair<KOTodoViewItem *, int> > mPercentChangedMap;

    DocPrefs *mDocPrefs;
    QString mCurrentDoc;
    KPIM::ClickLineEdit *mQuickAdd;

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
    enum {
      ePopupEdit = 1300,
      ePopupDelete = 1301,
      ePopupMoveTo = 1302,
      ePopupCopyTo = 1303,
      ePopupUnSubTodo = 1304,
      ePopupUnAllSubTodo = 1305
    };

};

#endif
