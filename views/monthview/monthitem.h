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

#ifndef MONTHITEM_H
#define MONTHITEM_H

#include <QObject>
#include <QGraphicsItem>
#include <QHash>
#include <QDate>

class QPaintEvent;

namespace KCal {
class Incidence;
}

using namespace KCal;

namespace KOrg {

class MonthScene;
class MonthGraphicsItem;
class MonthItem;


/**
 * A MonthGraphicsItem representing a part of an event. There should be one part 
 * per row = week
 */
class MonthGraphicsItem : public QObject, public QGraphicsItem
{
  Q_OBJECT

public:
  typedef QList<MonthGraphicsItem*> List;

  MonthGraphicsItem( MonthScene *monthWidget, MonthItem *manager );
  ~MonthGraphicsItem();

  /**
   * Change QGraphicsItem pos and boundingRect in the scene
   * according to the incidence start and end date
   */
  void updateGeometry();

  /**
   * @returns the associated MonthItem
   */
  MonthItem* monthItem() const { return mMonthItem; }

  /**
   * Shortcut for monthItem()->incidence()
   */
  Incidence *incidence() const;

  /**
   * @returns the date this item starts at
   */
  QDate startDate() const;

  /**
   * @returns the number of day this item spans on minus one
   * to be compatible with QDate::addDays()
   */
  int daySpan() const;

  /**
   * Computed from startDate() and daySpan()
   */
  QDate endDate() const;

  void setStartDate( const QDate &d);  
  void setDaySpan( int span );
  
  /**
   * @returns true if this item is currently being moved (ie. the
   * associated MonthItem is being moved).
   */
  bool isMoving() const;
  
  /**
   * @returns true if this item is currently being resized (ie. the 
   * associated MonthItem is being moved).
   */
  bool isResizing() const;

  /**
   * @returns true if this MonthGraphicsItem is the first one of the
   * MonthItem ones.
   */
  bool isBeginItem() const;
  /**
   * @returns true if this MonthGraphicsItem is the last one of the
   * MonthItem ones.
   */
  bool isEndItem() const;


  /**
   * Reimplemented from QGraphicsItem
   */
  virtual QRectF boundingRect() const;
  virtual void paint( QPainter*, const QStyleOptionGraphicsItem*, QWidget* ); 
  virtual QPainterPath shape() const;

private:
  // Shape of the item, see shape()
  QPainterPath widgetPath( bool mask = false ) const;

  // MonthScene this item is being drawn on
  MonthScene *mMonthScene;
  
  // See startDate()
  QDate mStartDate;

  // See daySpan()
  int mDaySpan;
  
  // The current item is part of a MonthItem
  MonthItem *mMonthItem;
};

/**
 * A month item manages different MonthGraphicsItems.
 */

class MonthItem : public QObject
{
   Q_OBJECT

 public:
  typedef QList<MonthItem*> List;
  
  MonthItem( MonthScene *monthWidget, Incidence *e );
  ~MonthItem();

  /**
   * Compares two events
   *
   * The month view displays a list of items. When loading (which occurs each time there is a change),
   * the items are sorted :
   *  - smallest date
   *  - bigger span
   *  - floating
   *  - finally, time in the day
   */
  static bool greaterThan( const MonthItem* e1,  
                           const MonthItem* e2 );  

    
  /**
   * The start date of the incidence, generally the event->dtStart().
   * But it reflect changes, even during move. Generally, you want to use this one.
   */
  QDate startDate() const;
  
  /**
   * This is the real start date, the incidence start date.
   */
  QDate realStartDate() const;

  /**
   * The end date from the event : start date + day span
   */
  QDate endDate() const;
  
  int daySpan() const;
  
  /**
   * Updates geometry of all MonthGraphicsItems
   */
  void updateGeometry();

  /**
   * Find the lower possible height for this event. 
   *
   * Height of an event in a cell : its vertical position. This is used
   * to avoia overlapping of items. An item keeps the same height in every
   * it crosses. Height is from top to bottom.
   */
  void updateHeight();

  /**
   * @returns the incidence associated with this item
   */
  Incidence *incidence() const { return mIncidence; }

  /**
   * @returns true if this item is selected.
   */
  bool selected() { return mSelected; }

  /**
   * @returns the height of the item ( > 0 )
   */
  int height() const;
  
  /**
   * If @p move is true, begin the move, else end the move  
   */
  void move( bool move );

  /**
   * If @p resize is true, begin the resize, else end the resize
   */
  void resize( bool resize );

  /**
   * Called during move to move the item a bit, relative to the previous move step
   */ 
  void moving( int offsetFromPreviousDate );
  /**
   * Called during resize to rezie the item a bit, relative to the previous resize step.
   */
  void resizing( int offsetFromPreviousDate );
  
  /** 
   * Sets the value of all MonthGraphicsItem to @param z 
   */
  void setZValue( qreal z );

  /**
   * Returns true if the item is being moved
   */
  bool isMoving() const { return mMoving; }
  
  /**
   * Returns true if the item is being resized
   */
  bool isResizing() const { return mResizing; }
 
  /**
   * Deletes all MonthGraphicsItem this item handles. Clear the list.
   */
  void deleteAll( );

  /**
   * Update the monthgraphicsitems
   *
   * This basically deletes and rebuild all the MonthGraphicsItems but tries to do it wisely
   * - If there is a moving item, it won't be deleted because then the new item won't receive anymore
   *   the MouseMove events
   * - If there is an item on a line where the new state needs an item, it is used and not deleted. This
   *   will avoid flickers.
   */
  void updateMonthGraphicsItems();
protected slots:
  /**
   * Update the selected state of this item.
   * If will be selected if incidence is the incidence managed by this item. Else
   * it will be deselected.
   */
  void updateSelection( Incidence* incidence );

private:
  // Factorized function shared by moving() and resizing()
  void movingOrResizing( int offsetFromPreviousDate);

  /*
   * Sets the resizing dayspan
   */
  void setResizingDaySpan( int daySpan ) { mResizingDaySpan = daySpan; }

  MonthGraphicsItem::List mMonthGraphicsItemList;

  MonthScene *mMonthScene;
  Incidence *mIncidence;
  
  bool mSelected;
  bool mMoving; // during move
  QDate mMovingStartDate;

  bool mResizing; // during resize
  QDate mResizingStartDate;
  int mResizingDaySpan;
 
  int mHeight;
};


/**
 * Keeps information about a month cell.
 */
class MonthCell
{
public:
  MonthCell( int id, QDate date ) : mId( id ), mDate( date ) {};

  /*
    This is used to get the height of the minimum height (vertical position) in 
    the month cells
  */
   MonthItem::List mMonthItemList;

  QHash<int, MonthItem*> mHeightHash;

  int firstFreeSpace();
  void addMonthItem( MonthItem* manager, int height );

  int id() const { return mId; }
  QDate date() const { return mDate; }
  int x() const {
    return mId % 7;
  }
  int y() const {
    return mId / 7;
  }
  static int topMargin();
  // returns true if the cell contains events below the height @p height
  bool hasEventBelow( int height );
  
private:
  int mId;
  QDate mDate;
};


}

#endif
