#ifndef KOAGENDA_H
#define KOAGENDA_H
// $Id$

#include <qscrollview.h>
#include <qtimer.h>
#include <qarray.h>

#include "koagendaitem.h"

class QPopupMenu;
class QTime;
class KOEvent;
class KConfig;

class KOAgenda : public QScrollView
{
    Q_OBJECT
  public:
    KOAgenda ( int columns, int rows, int columnSize, QWidget * parent=0,
               const char * name=0, WFlags f=0 );
    KOAgenda ( int columns, QWidget * parent=0,
               const char * name=0, WFlags f=0 );
    virtual ~KOAgenda();

    KOEvent *selectedEvent();

    virtual bool eventFilter ( QObject *, QEvent * );   

    void contentsToGrid (int x, int y, int& gx, int& gy);
    void gridToContents (int gx, int gy, int& x, int& y);

    int timeToY (const QTime &time);
    QTime gyToTime (int y);

    void setStartHour(int startHour);

    KOAgendaItem *insertItem (KOEvent *event,int X,int YTop,int YBottom);
    KOAgendaItem *insertAllDayItem (KOEvent *event,int XBegin,int XEnd);
    void insertMultiItem (KOEvent *event,int XBegin,int XEnd,
                          int YTop,int YBottom);

    void changeColumns(int columns);
    
    int columns() { return mColumns; }
    int rows() { return mRows; }
    
    int gridSpacingX() const { return mGridSpacingX; }
    int gridSpacingY() const { return mGridSpacingY; }

//    virtual QSizePolicy sizePolicy() const;

    void clear();

    /** Calculates the minimum width */
    virtual int minimumWidth() const;
    /** Update configuration from preference settings */
    void updateConfig();

    void checkScrollBoundaries();

    void setHolidayMask(QArray<bool> *);

  public slots:
    void scrollUp();
    void scrollDown();

    void popupAlarm();

    void checkScrollBoundaries(int);
    
    /** Deselect selected items. This function does not emit any signals. */
    void deselectItem();
    /** Select item. If the argument is 0, the currently selected item gets
     deselected. This function emits the itemSelected(bool) signal to inform
     about selection/deseelction of events. */
    void selectItem(KOAgendaItem *);
    
  signals:
    void newEventSignal();
    void newEventSignal(int gx,int gy);
    void editEventSignal(KOEvent *event);
    void showEventSignal(KOEvent *event);
    void deleteEventSignal(KOEvent *event);

    void itemModified(KOAgendaItem *item);
    void itemSelected(bool);

    void showEventPopupSignal(KOEvent *);

    void lowerYChanged(int);
    void upperYChanged(int);

    void startDragSignal(KOEvent *);

  protected:
    void drawContents(QPainter *p,int cx, int cy, int cw, int ch);        
    virtual void resizeEvent ( QResizeEvent * );

    /** Start moving/resizing agenda item */
    void startItemAction(QPoint viewportPos);
    
    /** Move/resize agenda item */
    void performItemAction(QPoint viewportPos);

    /** End moving/resizing agenda item */
    void endItemAction();

    /** Set cursor, when no item action is in progress */
    void setNoActionCursor(KOAgendaItem *moveItem,QPoint viewportPos);

    /** Place agenda item in agenda and adjust other cells if necessary */
    void placeSubCells(KOAgendaItem *placeItem);

    /** Process the keyevent, including the ignored keyevents of eventwidgets.
     * Implements pgup/pgdn and cursor key navigation in the view.
     */
    void keyPressEvent( QKeyEvent * );

    void calculateWorkingHours();
    
  private:
    void init();
    bool mAllDayMode;
  
    // Width and height of agenda cells
    int mGridSpacingX;
    int mGridSpacingY;

    // size of border, where mouse action will resize the KOAgendaItem
    int mResizeBorderWidth;

    // size of border, where mouse mve will cause a scroll of the agenda
    int mScrollBorderWidth;
    int mScrollDelay;
    int mScrollOffset;

    QTimer mScrollUpTimer;
    QTimer mScrollDownTimer;

    // Number of Columns/Rows of agenda grid
    int mColumns;
    int mRows;

    // Cells to store Move and Resize coordiantes
    int mStartCellX;
    int mStartCellY;
    int mCurrentCellX;
    int mCurrentCellY;

    // Working Hour coordiantes
    bool mWorkingHoursEnable;
    int mWorkingHoursYTop;
    int mWorkingHoursYBottom;
    
    // The KOAgendaItem, which has been right-clicked last
    KOAgendaItem *mClickedItem;

    // The KOAgendaItem, which is being moved/resized    
    KOAgendaItem *mActionItem;

    // Currently selected item
    KOAgendaItem *mSelectedItem;

    enum MouseActionType {NOP,MOVE,RESIZETOP,RESIZEBOTTOM,RESIZELEFT,
                          RESIZERIGHT};

    MouseActionType mActionType;    
    
    bool mItemMoved;
    
    // List of all Items contained in agenda
    QList<KOAgendaItem> mItems;

    QPopupMenu *mItemPopup; // Right mouse button popup menu for KOAgendaItems

    int mOldLowerScrollValue;
    int mOldUpperScrollValue;

    QArray<bool> *mHolidayMask;
};

#endif // KOAGENDA_H
