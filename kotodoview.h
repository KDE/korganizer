/* $Id$ */
#ifndef _KOTODOVIEW_H
#define _KOTODOVIEW_H

#include <qtableview.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qstrlist.h>
#include <qlistbox.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qmap.h>
#include <qlistview.h>

#include "calobject.h"
#include "todo.h"
#include "kobaseview.h"

/**
 * This class provides a way of displaying a single Event of Todo-Type in a
 * KTodoView.
 *
 * @author Cornelius Schumacher <schumacher@kde.org>
 * @see KOTodoView
 */
class KOTodoViewItem : public QCheckListItem
{
  public:
    /**
     * Constructor.
     *
     * @param parent is the list view to which this item belongs.
     * @param ev is the event to have the item display information for.
     */
    KOTodoViewItem(QListView *parent, Todo *ev);
    KOTodoViewItem(KOTodoViewItem *parent, Todo *ev);
    virtual ~KOTodoViewItem() {}

    void construct();

    Todo *event() { return mEvent; }

  protected:
    void paintBranches(QPainter *p,const QColorGroup & cg,int w,int y,int h,
                       GUIStyle s);

  private:
    Todo *mEvent;
};


class KOTodoListView : public QListView
{
    Q_OBJECT
  public:
    KOTodoListView(CalObject *,QWidget *parent=0,const char *name=0);
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
    CalObject *mCalendar;
  
    QPoint mPressPos;
    bool mMousePressed;
    QListViewItem *mOldCurrent;
};


/**
 * This class provides a multi-column list view of todo events.
 *
 * @short multi-column list view of todo events.
 * @author Cornelius Schumacher <schumacher@kde.org>
 */
class KOTodoView : public KOBaseView
{
    Q_OBJECT
  public:
    KOTodoView(CalObject *, QWidget* parent=0, const char* name=0 );
    ~KOTodoView() {}

    QList<Incidence> getSelected();
    QList<Todo> selectedTodos();

    /** Return number of shown dates. TodoView does not show dates, */
    int currentDateCount() { return 0; }

    void printPreview(CalPrinter *calPrinter, const QDate &fd, const QDate &td);

  public slots:
    void updateView();
    void updateConfig();

    void changeEventDisplay(Event *, int);
  
    /**
     * selects the dates specified in the list.  If the view cannot support
     * displaying all the dates requested, or it needs to change the dates
     * in some manner, it may call @see datesSelected.
     * @param dateList is the list of dates to try and select.
     */
    void selectDates(const QDateList dateList);
  
    /**
     * Select events visible in the current display
     * @param eventList a list of events to select.
     */
    void selectEvents(QList<Event> eventList);

    void editItem(QListViewItem *item);
    void showItem(QListViewItem *item);
    void popupMenu(QListViewItem *item,const QPoint &,int);
    void newTodo();
    void newSubTodo();
    void showTodo();
    void editTodo();
    void deleteTodo();
    void purgeCompleted();
    void itemClicked(QListViewItem *);
    
  signals:
    void newTodoSignal();
    void newSubTodoSignal(Todo *);
    void showTodoSignal(Todo *);

    void editTodoSignal(Todo *);
    void deleteTodoSignal(Todo *);

  private:
    QMap<Todo *,KOTodoViewItem *>::ConstIterator insertTodoItem(Todo *todo);

    KOTodoListView *mTodoListView;
    QPopupMenu *mItemPopupMenu;
    QPopupMenu *mPopupMenu;
    KOTodoViewItem *mActiveItem;

    QMap<Todo *,KOTodoViewItem *> mTodoMap;
};

#endif
