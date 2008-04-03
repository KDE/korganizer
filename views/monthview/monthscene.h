/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Bruno Virlet <bruno.virlet@gmail.com>

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

#ifndef MONTHSCENE_H
#define MONTHSCENE_H

#include "monthitem.h"

#include <QGraphicsScene>
#include <QGraphicsView>

class QResizeEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;
namespace KCal {
  class Incidence;
  class Calendar;
}
using namespace KCal;

namespace KOrg {

class MonthGraphicsItem;
class MonthGraphicsView;
class MonthCell;
class KONewMonthView;

class MonthScene : public QGraphicsScene
{
  Q_OBJECT

  enum ActionType {
    None,
    Move,
    Resize
  };

  public:
    enum ResizeType {
      ResizeLeft,
      ResizeRight
    };

    MonthScene( KONewMonthView *parent, Calendar *calendar );
    ~MonthScene();

    int columnWidth() const;
    int rowHeight() const;

    MonthCell *firstCellForMonthItem( MonthItem *manager );
    int height( MonthItem *manager );
    int itemHeight();
    MonthItem::List mManagerList;
    KONewMonthView *mMonthView;

    QMap<QDate, MonthCell*> mMonthCellMap;

    bool initialized() { return mInitialized; }
    void setInitialized( bool i ) { mInitialized = i; }
    void resetAll();
    Calendar *calendar() { return mCalendar; }

    virtual bool eventFilter ( QObject *, QEvent * );

    int totalHeight();

    /**
     * Returns the vertical position where the top of the cell should be painted
     * taking in account margins, rowHeight
     */
    int cellVerticalPos( const MonthCell *cell ) const;
    
    /**
     * Idem, for the horizontal position
     */
    int cellHorizontalPos( const MonthCell *cell ) const;

    /**
      Select item. If the argument is 0, the currently selected item gets
      deselected. This function emits the itemSelected(bool) signal to inform
      about selection/deselection of events.
    */
    void selectItem( MonthItem * );
    int maxRowCount();

    MonthCell *selectedCell() const;

    /**
      Get the space on the right of the cell associated to the date @p date.
    */
    int getRightSpan( const QDate &date ) const;

    /**
      Get the space on the left of the cell associated to the date @p date.
    */
    int getLeftSpan( const QDate &date ) const;

    /**
      Returns the date in the first column of the row given by @p row.
    */
    QDate firstDateOnRow( int row ) const;

    /**
      Calls updateGeometry() on each MonthItem
    */
    void updateGeometry();

    /**
      Returns the first height. Used for scrolling

      @see MonthItem::height()
    */
    int startHeight() { return mStartHeight; }

    /**
      Set the current height using @p height.
      If height = 0, then the view is not scrolled. Else it will be scrolled
      by step of one item.
    */
    void setStartHeight( int height ) { mStartHeight = height; }

    /**
      Returns the MonthGraphicsItem being moved.
    */
    MonthGraphicsItem *movingMonthGraphicsItem()
    { return mMovingMonthGraphicsItem; }

    /**
      Sets the MonthGraphicsItem being moved to @p incidenceItem.
    */
    void setMovingMonthGraphicsItem( MonthGraphicsItem *incidenceItem )
    { mMovingMonthGraphicsItem = incidenceItem; }

    /**
      Returns the resize type.
    */
    ResizeType resizeType() { return mResizeType; }

    /**
      Returns the currently selected item.
    */
    MonthItem *selectedItem() { return mSelectedItem; }

  signals:
    void incidenceSelected( Incidence *incidence );
    void showIncidencePopupSignal( Incidence *, const QDate &);
    void showNewEventPopupSignal();

  protected:

    /**
      Handles mouse events. Called from eventFilter.
    */
    virtual bool eventFilterMouse ( QObject *, QGraphicsSceneMouseEvent * );

    /**
      Handles mousewheel events. Called from eventFilter.
    */
    virtual bool eventFilterWheel ( QObject *, QGraphicsSceneWheelEvent * );

    /**
      Handles drag and drop events. Called from eventFilter.
    */
//    virtual bool eventFilter_drag( QObject *, QDropEvent * );

    /**
      Returns true if the last item is visible in the given @p cell.
    */
    bool lastItemFit( MonthCell *cell );

  private:
    /**
     * Returns the height of the header of the view
     */
    int headerHeight() const;

    int availableWidth() const;

    /**
     * Height available to draw the cells. Doesn't include header.
     */
    int availableHeight() const;

    /**
     * Removes all the margins, frames, etc. to give the 
     * X coordinate in the MonthGrid.
     */ 
    int sceneXToMonthGridX( int xScene );
    
    /**     
     * Removes all the margins, frames, headers etc. to give the 
     * Y coordinate in the MonthGrid.
     */
    int sceneYToMonthGridY( int yScene );

    /**
     * Given a pos in the scene coordinates, 
     * returns the cell containing @p pos.
     */
    MonthCell *getCellFromPos( const QPointF &pos );

    /**
       Returns true if (x, y) is in the monthgrid, false else.
     */
    bool isInMonthGrid( int x, int y );

    bool mInitialized;

    // Calendar associated to the view
    Calendar *mCalendar;

    // User interaction.
    MonthItem *mClickedItem; // todo ini in ctor
    MonthItem *mActionItem;
    bool mActionInitiated;

    MonthItem *mSelectedItem;
    QDate mSelectedCellDate;
    MonthCell *mStartCell; // start cell when dragging
    MonthCell *mPreviousCell; // the cell before that one during dragging

    MonthGraphicsItem *mMovingMonthGraphicsItem;
    MonthGraphicsItem *mActionMonthGraphicsItem;

    ActionType mActionType;
    ResizeType mResizeType;

    // The item height at the top of the cell. This is generally 0 unless
    // the user scroll the view when there are too many items.
    int mStartHeight;

    friend class MonthGraphicsView;
};

/**
 * Renders a MonthScene
 */
class MonthGraphicsView : public QGraphicsView
{
  public:
    MonthGraphicsView( KONewMonthView *parent, Calendar *calendar );

    /**
      Draws the cells.
    */
    void drawBackground( QPainter *painter, const QRectF &rect );

    void setScene( MonthScene *scene );

    /**
      Change the cursor according to @p actionType.
    */
    void setActionCursor( MonthScene::ActionType actionType );

  protected:
    virtual void resizeEvent( QResizeEvent * );

  private:
    MonthScene *mScene;
    KONewMonthView *mMonthView;
};

}

#endif
