/* $Id$ */

#ifndef _KOTODOVIEW_H
#define _KOTODOVIEW_H

#include <qtableview.h>
#include <qfont.h>
#include <qfontmetrics.h>
//#include <ktablistbox.h>
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

private:
  KOEvent *mEvent;
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
    void changeEventDisplay(KOEvent *which, int action);
    void editItem(QListViewItem *item);
    void popupMenu(QListViewItem *item,const QPoint &,int);
    void newTodo();
    void newSubTodo();
    void editTodo();
    void deleteTodo();
    void purgeCompleted();
    
  signals:
    void newTodoSignal();
    void newSubTodoSignal(KOEvent *);
    void editEventSignal(KOEvent *);
    void deleteEventSignal(KOEvent *);

  private:
    QMap<KOEvent *,KOTodoViewItem *>::ConstIterator
      insertTodoItem(KOEvent *todo);

    CalObject *mCalendar;
    
    QListView *mTodoListView;
    QPopupMenu *mItemPopupMenu;
    QPopupMenu *mPopupMenu;
    KOTodoViewItem *mActiveItem;

    QMap<KOEvent *,KOTodoViewItem *> mTodoMap;
};


// all code below is obsolete

class todoViewIn : public QTableView
{
  Q_OBJECT
    public:
        todoViewIn( CalObject *, QWidget* parent=0, const char* name=0 );
        ~todoViewIn();

    // old ToDo functions
    public slots:
        void updateView() ;
        void updateConfig();
        KOEvent *getSelected() { return aTodo; }
        void changeEventDisplay(KOEvent *which, int action) { updateView(); }

    protected slots:
        void updateItem(int r, int c) { updateCell(r,c); }
        void changeSummary(const QString &newsum);
        void updateSummary();
        void changePriority(int pri);
        void lowPriority() { changePriority(3); }
        void mediumPriority() { changePriority(2); }
        void highPriority() { changePriority(1); }
	void sortPriority();
        void doPopup(int, int) {}
        void newTodo();
        void editTodo();
        void cleanWindow(QWidget *) {}
        void deleteTodo();
        void purgeCompleted();
        void headerAction(int) {}
        void hiliteAction(int, int) {}
        void showDates(bool) {}
        inline void showDates() {}
        inline void hideDates() {}

    protected:
        void paintCell( QPainter*, int row, int col );
        void mousePressEvent( QMouseEvent* );
        void keyPressEvent( QKeyEvent* );
        void focusInEvent( QFocusEvent* );
        void focusOutEvent( QFocusEvent* );
        void resizeEvent(QResizeEvent *);

    private:
        int indexOf( int row, int col ) const;
        QString* contents;
        int curRow;
        int curCol;
        QFont todoFont;

    signals:
        void editEventSignal(KOEvent *);

    protected:
        enum { PRIORITY, DESCRIPTION, COMPLETED, DUEDATE };
        void adjustColumns() {}

	int curSortMode;
        bool editingFlag;
        int prevRow, updatingRow;
        QString tmpSummary;
        QLineEdit *editor;
        QListBox *priList;
        QPopupMenu *rmbMenu1;
        QPopupMenu *rmbMenu2;
        KOEvent *aTodo;
        CalObject *calendar;
        QList<int> todoIdList;
};

// old todoview class, to be replaced by KOTodoView
class TodoView : public QWidget
{
  Q_OBJECT

    public:
        TodoView(CalObject *, QWidget* parent=0, const char* name=0 );
        ~TodoView() {}

    public slots:
        void updateView() { todoBox->updateView(); }
        void updateConfig() { todoBox->updateConfig(); }
        KOEvent *getSelected() { return todoBox->getSelected(); }
        void changeEventDisplay(KOEvent *which, int action) 
	  { todoBox->changeEventDisplay(which, action); }
        
    signals:
        void editEventSignal(KOEvent *);

    private:
        void resizeEvent(QResizeEvent *);
	//        void paintEvent(QPaintEvent *) {};

        todoViewIn *todoBox;
        int topH;
        QLabel *label;
};


#endif
