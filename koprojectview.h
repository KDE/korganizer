/* $Id$ */
#ifndef _KOPROJECTVIEW_H
#define _KOPROJECTVIEW_H

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
#include "event.h"
#include "kobaseview.h"
#include "KGanttItem.h"

class KGantt;

/**
  This class provides an item of the project view. It is a xQTask with
  an additional Event attribute.
*/
class KOProjectViewItem : public KGanttItem {
  public:
    KOProjectViewItem(Todo *,KGanttItem* parentTask, const QString& text, 
	              const QDateTime& start, const QDateTime& end);
    ~KOProjectViewItem();
    
    Todo *event();
    
  private:
    Todo *mEvent;
};


/**
 * This class provides a Gantt-like project view on todo items 
 *
 * @short project view on todo items.
 * @author Cornelius Schumacher <schumacher@kde.org>
 */
class KOProjectView : public KOBaseView
{
    Q_OBJECT
  public:
    KOProjectView(CalObject *, QWidget* parent=0, const char* name=0 );
    ~KOProjectView() {}

    QList<Incidence> getSelected();

    /** Return number of shown dates. */
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

/*
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
*/
    
  protected slots:
    void showModeMenu();  
    void zoomIn();
    void zoomOut();
    void taskChanged(KGanttItem *task,KGanttItem::Change change);
  
  signals:
    void newTodoSignal();
    void newSubTodoSignal(Todo *);
    void showTodoSignal(Todo *);
    void editTodoSignal(Todo *);
    void deleteTodoSignal(Todo *);

  private:
    void createMainTask();
    KGanttItem *createTask(KGanttItem *,Todo *);
  
    KGantt *mGantt;
    KGanttItem *mMainTask;

    QMap<Todo *,KGanttItem *>::ConstIterator insertTodoItem(Todo *todo);

    QMap<Todo *,KGanttItem *> mTodoMap;
};

#endif
