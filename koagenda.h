/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOAGENDA_H
#define KOAGENDA_H

#include <qscrollview.h>
#include <qtimer.h>
#include <qguardedptr.h>
#include <libkcal/incidencebase.h>


class QPopupMenu;
class QTime;
class QLabel;
class KConfig;
class KOAgenda;
class KOAgendaItem;

using namespace KOrg;
namespace KOrg {
class IncidenceChangerBase;
}

using namespace KCal;
namespace KCal {
class Event;
class Todo;
class Calendar;
}

class MarcusBains : public QFrame {
    Q_OBJECT
  public:
    MarcusBains( KOAgenda *agenda = 0, const char *name = 0 );
    virtual ~MarcusBains();

  public slots:
    void updateLocation( bool recalculate = false );

  private:
    int todayColumn();
    QTimer *minutes;
    QLabel *mTimeBox;
    KOAgenda *agenda;
    QTime oldTime;
    int oldToday;
};


class KOAgenda : public QScrollView
{
    Q_OBJECT
  public:
    KOAgenda ( int columns, int rows, int columnSize, QWidget *parent=0,
               const char *name = 0, WFlags f = 0 );
    KOAgenda ( int columns, QWidget *parent = 0,
               const char *name = 0, WFlags f = 0 );
    virtual ~KOAgenda();

    Incidence *selectedIncidence() const;
    QDate selectedIncidenceDate() const;
    /**
     * Returns the uid of the last incidence that was selected. This
     * persists across reloads and clear, so that if the same uid
     * reappears, it can be reselected. */
    const QString lastSelectedUid() const;

    virtual bool eventFilter ( QObject *, QEvent * );

    QPoint contentsToGrid ( const QPoint &pos ) const;
    QPoint gridToContents ( const QPoint &gpos ) const;

    int timeToY ( const QTime &time );
    QTime gyToTime ( int y );

    QMemArray<int> minContentsY();
    QMemArray<int> maxContentsY();

    void setStartTime( const QTime &startHour );

    KOAgendaItem *insertItem ( Incidence *incidence, const QDate &qd, int X, int YTop,
                               int YBottom );
    KOAgendaItem *insertAllDayItem ( Incidence *event, const QDate &qd, int XBegin,
                                     int XEnd );
    void insertMultiItem ( Event *event, const QDate &qd, int XBegin, int XEnd,
                           int YTop, int YBottom );

    /** remove an event and all its multi-items from the agenda.
     *  This function removes the items from the view, but doesn't delete them immediately.
     *  Instead, they are queued in mItemsToDelete and later deleted by
     *  the slot deleteItemsToDelete() (called by QTimer::singleShot ) */
    void removeIncidence( Incidence *incidence );

    void changeColumns( int columns );

    int columns() { return mColumns; }
    int rows() { return mRows; }

    double gridSpacingX() const { return mGridSpacingX; }
    double gridSpacingY() const { return mGridSpacingY; }

//    virtual QSizePolicy sizePolicy() const;

    void clear();

    /** Calculates the minimum width */
    virtual int minimumWidth() const;
    /** Update configuration from preference settings */
    void updateConfig();

    void checkScrollBoundaries();

    void setHolidayMask( QMemArray<bool> * );

    void setDateList( const DateList &selectedDates );
    DateList dateList() const;

    void setTypeAheadReceiver( QObject * );
    QObject *typeAheadReceiver() const;
    void finishTypeAhead();

    void setCalendar( Calendar*cal ) { mCalendar = cal; }
    void setIncidenceChanger( IncidenceChangerBase *changer ) { mChanger = changer; }

  public slots:
    void scrollUp();
    void scrollDown();

    void checkScrollBoundaries( int );

    /** Deselect selected items. This function does not emit any signals. */
    void deselectItem();

    void clearSelection();

    /**
      Select item. If the argument is 0, the currently selected item gets
      deselected. This function emits the itemSelected(bool) signal to inform
      about selection/deselection of events.
    */
    void selectItem( KOAgendaItem * );
    /**
      Select the item associated with a given uid. Linear search, use carefully.
    */
    void selectItemByUID( const QString& uid );
    bool removeAgendaItem( KOAgendaItem *item );
    void showAgendaItem( KOAgendaItem *item );

  signals:
    void newEventSignal();
    void newEventSignal( const QPoint &pos );
    void newEventSignal( const QPoint &start, const QPoint &end );
    void newTimeSpanSignal( const QPoint &, const QPoint & );
    void newStartSelectSignal();

    void showIncidenceSignal( Incidence * );
    void editIncidenceSignal( Incidence * );
    void deleteIncidenceSignal( Incidence * );
    void showIncidencePopupSignal( Incidence *, const QDate &);
    void showNewEventPopupSignal();

    void itemModified( KOAgendaItem *item );
    void incidenceSelected( Incidence * );
    void startMultiModify( const QString & );
    void endMultiModify();

    void lowerYChanged( int );
    void upperYChanged( int );

    void startDragSignal(Incidence *);
    void droppedToDo( Todo*todo, const QPoint &gpos, bool allDay );

    void enableAgendaUpdate( bool enable );
    void zoomView( const int delta, const QPoint &pos, const Qt::Orientation );

    void mousePosSignal(const QPoint &pos);
    void enterAgenda();
    void leaveAgenda();

  private:
    enum MouseActionType { NOP, MOVE, SELECT,
                           RESIZETOP, RESIZEBOTTOM, RESIZELEFT, RESIZERIGHT };

  protected:
    void drawContents( QPainter *p, int cx, int cy, int cw, int ch );
    int columnWidth( int column );
    virtual void resizeEvent ( QResizeEvent * );

    /** Handles mouse events. Called from eventFilter */
    virtual bool eventFilter_mouse ( QObject *, QMouseEvent * );
#ifndef QT_NO_WHEELEVENT    
    /** Handles mousewheel events. Called from eventFilter */
    virtual bool eventFilter_wheel ( QObject *, QWheelEvent * );
#endif    
    /** Handles key events. Called from eventFilter */
    virtual bool eventFilter_key ( QObject *, QKeyEvent * );

    /** Handles drag and drop events. Called from eventFilter */
    virtual bool eventFilter_drag( QObject *, QDropEvent * );

    /** returns RESIZELEFT if pos is near the lower edge of the action item,
      RESIZERIGHT if pos is near the higher edge, and MOVE otherwise.
      If --reverse is used, RESIZELEFT still means resizing the beginning of
      the event, although that means moving to the right!
      horizontal is the same as mAllDayAgenda.
    */
    MouseActionType isInResizeArea( bool horizontal, const QPoint &pos, KOAgendaItem *item );
    /** Return whether the cell specified by the grid point belongs to the current select
    */
    bool ptInSelection( QPoint gpos ) const;


    /** Start selecting time span. */
    void startSelectAction( const QPoint &viewportPos );

    /** Select time span. */
    void performSelectAction( const QPoint &viewportPos );

    /** Emd selecting time span. */
    void endSelectAction( const QPoint &viewportPos );

    /** Start moving/resizing agenda item */
    void startItemAction(const QPoint& viewportPos);

    /** Move/resize agenda item */
    void performItemAction(const QPoint& viewportPos);

    /** End moving/resizing agenda item */
    void endItemAction();

    /** Set cursor, when no item action is in progress */
    void setNoActionCursor( KOAgendaItem *moveItem, const QPoint &viewportPos );
    /** Sets the cursor according to the given action type. If acting==true,
      the corresponding action is running (i.e. the item is really moved). If
      acting==false the cursor should just indicate that the corresponding action
      is possible */
    void setActionCursor( int actionType, bool acting=false );

    /** calculate the width of the column subcells of the given item */
    double calcSubCellWidth( KOAgendaItem *item );
    /** Move and resize the given item to the correct position */
    void placeAgendaItem( KOAgendaItem *item, double subCellWidth );
    /** Place agenda item in agenda and adjust other cells if necessary */
    void placeSubCells( KOAgendaItem *placeItem );
    /** Place the agenda item at the correct position (ignoring conflicting items) */
    void adjustItemPosition( KOAgendaItem *item );

    /** Process the keyevent, including the ignored keyevents of eventwidgets.
     * Implements pgup/pgdn and cursor key navigation in the view.
     */
    void keyPressEvent( QKeyEvent * );

    void calculateWorkingHours();

    virtual void contentsMousePressEvent ( QMouseEvent * );

    void emitNewEventForSelection();

  protected slots:
    /** delete the items that are queued for deletion */
    void deleteItemsToDelete();

  private:
    void init();
    void marcus_bains();
    bool mAllDayMode;

    // We need the calendar for drag'n'drop and for paint the ResourceColor
    Calendar *mCalendar;

    // Width and height of agenda cells. mDesiredGridSpacingY is the height
    // set in the config. The actual height might be larger since otherwise
    // more than 24 hours might be displayed.
    double mGridSpacingX;
    double mGridSpacingY;
    double mDesiredGridSpacingY;

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

    // Cells to store Move and Resize coordiantes while performing the action
    QPoint mStartCell;
    QPoint mEndCell;

    // Working Hour coordiantes
    bool mWorkingHoursEnable;
    QMemArray<bool> *mHolidayMask;
    int mWorkingHoursYTop;
    int mWorkingHoursYBottom;

    // Selection
    bool mHasSelection;
    QPoint mSelectionStartPoint;
    QPoint mSelectionStartCell;
    QPoint mSelectionEndCell;

    // List of dates to be displayed
    DateList mSelectedDates;

    // The KOAgendaItem, which has been right-clicked last
    QGuardedPtr<KOAgendaItem> mClickedItem;

    // The KOAgendaItem, which is being moved/resized
    QGuardedPtr<KOAgendaItem> mActionItem;

    // Currently selected item
    QGuardedPtr<KOAgendaItem> mSelectedItem;
    // Uid of the last selected item. Used for reselecting in situations
    // where the selected item points to a no longer valid incidence, for
    // example during resource reload.
    QString mSelectedUid;

    // The Marcus Bains Line widget.
    MarcusBains *mMarcusBains;

    MouseActionType mActionType;

    bool mItemMoved;

    // List of all Items contained in agenda
    QPtrList<KOAgendaItem> mItems;
    QPtrList<KOAgendaItem> mItemsToDelete;

    QPopupMenu *mItemPopup; // Right mouse button popup menu for KOAgendaItems

    int mOldLowerScrollValue;
    int mOldUpperScrollValue;

    bool mTypeAhead;
    QObject *mTypeAheadReceiver;
    QPtrList<QEvent> mTypeAheadEvents;

    bool mReturnPressed;
    KOrg::IncidenceChangerBase *mChanger;
};

#endif // KOAGENDA_H
