/* $Id$ */
#ifndef _KOTODOVIEW_H
#define _KOTODOVIEW_H

#include <qtableview.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qlined.h>
#include <qlist.h>
#include <qstrlist.h>
#include <qlistbox.h>
#include <qpopmenu.h>
#include <qlabel.h>
#include <qmap.h>
#include <qlistview.h>

#include "calobject.h"
#include "koevent.h"

/**
 * This class provides a way of displaying a single KOEvent of Todo-Type in a
 * KTodoView.
 *
 * @author Cornelius Schumacher <schumacher@asic.uni-heidelberg.de>
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
    KOTodoViewItem(QListView *parent, KOEvent *ev);
    KOTodoViewItem(KOTodoViewItem *parent, KOEvent *ev);
    virtual ~KOTodoViewItem() {}

    void construct();

    KOEvent *event() { return mEvent; }

  protected:
    void paintBranches(QPainter *p,const QColorGroup & cg,int w,int y,int h,
                       GUIStyle s);

  private:
    KOEvent *mEvent;
};


class KOTodoListView : public QListView
{
    Q_OBJECT
  public:
    KOTodoListView(CalObject *,QWidget *parent=0,const char *name=0);
    virtual ~KOTodoListView() {}

  signals:
    void todoDropped(KOEvent *);
    
  protected:
    void contentsDragEnterEvent(QDragEnterEvent *);
    void contentsDragMoveEvent(QDragMoveEvent *);
    void contentsDragLeaveEvent(QDragLeaveEvent *);
    void contentsDropEvent(QDropEvent *);
  
    void contentsMousePressEvent(QMouseEvent *);
    void contentsMouseMoveEvent(QMouseEvent *);
    void contentsMouseReleaseEvent(QMouseEvent *);

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
 * @author Cornelius Schumacher <schumacher@asic.uni-heidelberg.de>
 */
class KOTodoView : public QWidget
{
    Q_OBJECT
  public:
    KOTodoView(CalObject *, QWidget* parent=0, const char* name=0 );
    ~KOTodoView() {}

  public slots:
    void updateView();
    void updateConfig();
    KOEvent *getSelected();
    void editItem(QListViewItem *item);
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
    void newSubTodoSignal(KOEvent *);
    void showTodoSignal(KOEvent *);

    void editEventSignal(KOEvent *);
    void deleteEventSignal(KOEvent *);

  private:
    QMap<KOEvent *,KOTodoViewItem *>::ConstIterator
      insertTodoItem(KOEvent *todo);

    CalObject *mCalendar;
    
    KOTodoListView *mTodoListView;
    QPopupMenu *mItemPopupMenu;
    QPopupMenu *mPopupMenu;
    KOTodoViewItem *mActiveItem;

    QMap<KOEvent *,KOTodoViewItem *> mTodoMap;
};

#endif
