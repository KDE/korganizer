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
#include "koevent.h"
#include "kobaseview.h"
#include "xQTask.h"

class xQGantt;

/**
  This class provides an item of the project view. It is a xQTask with
  an additional KOEvent attribute.
*/
class KOProjectViewItem : public xQTask {
  public:
    KOProjectViewItem(Todo *,xQTask* parentTask, const QString& text, 
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

    void changeEventDisplay(KOEvent *, int);
  
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
    void selectEvents(QList<KOEvent> eventList);

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
    void taskChanged(xQTask *task,xQTask::Change change);
  
  signals:
    void newTodoSignal();
    void newSubTodoSignal(KOEvent *);
    void showTodoSignal(KOEvent *);

    void editEventSignal(KOEvent *);
    void deleteEventSignal(KOEvent *);

  private:
    void createMainTask();
    xQTask *createTask(xQTask *,Todo *);
  
    xQGantt *mGantt;
    xQTask *mMainTask;

    QMap<Todo *,xQTask *>::ConstIterator insertTodoItem(Todo *todo);

    QMap<Todo *,xQTask *> mTodoMap;
};

#endif
