/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#include <qlabel.h>
#include <qtimer.h>
#include <qmemarray.h>
#include <qguardedptr.h>

#include "koagendaitem.h"

class QPopupMenu;
class QTime;
class KConfig;
class QFrame;
class KOAgenda;
class KCal::Event;
class KCal::Todo;

using namespace KCal;

class MarcusBains : public QFrame {
    Q_OBJECT
  public:
    MarcusBains(KOAgenda *agenda=0,const char *name=0);
    virtual ~MarcusBains();

  public slots:
    void updateLocation(bool recalculate=false);

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
    KOAgenda ( int columns, int rows, int columnSize, QWidget * parent=0,
               const char * name=0, WFlags f=0 );
    KOAgenda ( int columns, QWidget * parent=0,
               const char * name=0, WFlags f=0 );
    virtual ~KOAgenda();

    Incidence *selectedIncidence() const;
    QDate selectedIncidenceDate() const;

    virtual bool eventFilter ( QObject *, QEvent * );

    void contentsToGrid (int x, int y, int& gx, int& gy);
    void gridToContents (int gx, int gy, int& x, int& y);

    int timeToY (const QTime &time);
    QTime gyToTime (int y);

    void setStartHour(int startHour);

    KOAgendaItem *insertItem (Incidence *event,QDate qd,int X,int YTop,int YBottom);
    KOAgendaItem *insertAllDayItem (Incidence *event,QDate qd,int XBegin,int XEnd);
    void insertMultiItem (Event *event,QDate qd,int XBegin,int XEnd,
                          int YTop,int YBottom);

    void changeColumns(int columns);

    int columns() { return mColumns; }
    int rows() { return mRows; }

    int gridSpacingX() const { return mGridSpacingX; }
    int gridSpacingY() const { return mGridSpacingY; }

//    virtual QSizePolicy sizePolicy() const;

    void clear();

    void clearSelection();

    /** Calculates the minimum width */
    virtual int minimumWidth() const;
    /** Update configuration from preference settings */
    void updateConfig();

    void checkScrollBoundaries();

    void setHolidayMask(QMemArray<bool> *);

    void setDateList(const DateList &selectedDates);
    DateList dateList() const;

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
    void newEventSignal(int gxStart, int gyStart, int gxEnd, int gyEnd);
    void newTimeSpanSignal(int gxStart, int gyStart, int gxEnd, int gyEnd);
    void newStartSelectSignal();

    void showIncidenceSignal(Incidence *);
    void editIncidenceSignal(Incidence *);
    void deleteIncidenceSignal(Incidence *);
    void showIncidencePopupSignal(Incidence *);
    void showNewEventPopupSignal();

    void itemModified(KOAgendaItem *item);
    void incidenceSelected(Incidence *);

    void lowerYChanged(int);
    void upperYChanged(int);

    void startDragSignal(Incidence *);

  protected:
    void drawContents(QPainter *p,int cx, int cy, int cw, int ch);
    virtual void resizeEvent ( QResizeEvent * );

    /** Handles mouse events. Called from eventFilter */
    virtual bool eventFilter_mouse ( QObject *, QMouseEvent * );

    /** Start selecting time span. */
    void startSelectAction(const QPoint& viewportPos);

    /** Select time span. */
    void performSelectAction(const QPoint& viewportPos);

    /** Emd selecting time span. */
    void endSelectAction();

    /** Start moving/resizing agenda item */
    void startItemAction(const QPoint& viewportPos);

    /** Move/resize agenda item */
    void performItemAction(const QPoint& viewportPos);

    /** End moving/resizing agenda item */
    void endItemAction();

    /** Set cursor, when no item action is in progress */
    void setNoActionCursor(KOAgendaItem *moveItem,const QPoint& viewportPos);

    /** Place agenda item in agenda and adjust other cells if necessary */
    void placeSubCells(KOAgendaItem *placeItem);

    /** Process the keyevent, including the ignored keyevents of eventwidgets.
     * Implements pgup/pgdn and cursor key navigation in the view.
     */
    void keyPressEvent( QKeyEvent * );

    void calculateWorkingHours();

    virtual void contentsMousePressEvent ( QMouseEvent * );

  private:
    void init();
    void marcus_bains();
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

    // Selection
    int mSelectionCellX;
    int mSelectionYTop;
    int mSelectionHeight;

    // List of dates to be displayed
    DateList mSelectedDates;

    // The KOAgendaItem, which has been right-clicked last
    KOAgendaItem *mClickedItem;

    // The KOAgendaItem, which is being moved/resized
    QGuardedPtr<KOAgendaItem> mActionItem;

    // Currently selected item
    QGuardedPtr<KOAgendaItem> mSelectedItem;

    // The Marcus Bains Line widget.
    MarcusBains *mMarcusBains;

    enum MouseActionType { NOP, MOVE, SELECT,
                           RESIZETOP, RESIZEBOTTOM, RESIZELEFT, RESIZERIGHT };

    MouseActionType mActionType;

    bool mItemMoved;

    // List of all Items contained in agenda
    QPtrList<KOAgendaItem> mItems;

    QPopupMenu *mItemPopup; // Right mouse button popup menu for KOAgendaItems

    int mOldLowerScrollValue;
    int mOldUpperScrollValue;

    QMemArray<bool> *mHolidayMask;
};

#endif // KOAGENDA_H
