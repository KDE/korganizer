/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Bruno Virlet <bruno.virlet@gmail.com>
  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

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

#ifndef MONTHGRAPHICSITEMS_H
#define MONTHGRAPHICSITEMS_H

#include <QHash>
#include <QGraphicsItem>
#include <QRect>
#include <QDate>

class QGraphicsScene;
class QStyleOptionGraphicsItem;
class QPainterPath;

namespace KOrg {

class MonthItem;

/**
 * Graphics items which indicates that the view can be scrolled to display
 * more events.
 */
class ScrollIndicator : public QGraphicsItem
{
  public:
    enum ArrowDirection {
      UpArrow,
      DownArrow
    };

    ScrollIndicator( ArrowDirection direction );

    QRectF boundingRect() const;
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );

    ArrowDirection direction() const { return mDirection; }

  private:
    ArrowDirection mDirection;

    static const int mWidth = 30;
    static const int mHeight = 10;
};

/**
 * Keeps information about a month cell.
 */
class MonthCell
{
  public:
    MonthCell( int id, QDate date, QGraphicsScene *scene );
    ~MonthCell();

    /**
      This is used to get the height of the minimum height (vertical position)
      in the month cells.
    */
    QList<MonthItem *> mMonthItemList;

    QHash<int, MonthItem *> mHeightHash;

    int firstFreeSpace();
    void addMonthItem( MonthItem *manager, int height );

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

    // TODO : move this to a new GUI class (monthcell could be GraphicsItems)
    ScrollIndicator *upArrow() { return mUpArrow; }
    ScrollIndicator *downArrow() { return mDownArrow; }

  private:
    int mId;
    QDate mDate;

    QGraphicsScene *mScene;

    ScrollIndicator *mUpArrow;
    ScrollIndicator *mDownArrow;
};

/**
 * A MonthGraphicsItem representing a part of an event. There should be
 * one part per row = week
 */
class MonthGraphicsItem : public QObject, public QGraphicsItem
{
  Q_OBJECT
  Q_INTERFACES(QGraphicsItem)

  public:
    typedef QList<MonthGraphicsItem *> List;

    MonthGraphicsItem( MonthItem *manager );
    ~MonthGraphicsItem();

    /**
      Change QGraphicsItem pos and boundingRect in the scene
      according to the incidence start and end date.
    */
    void updateGeometry();

    /**
      Returns the associated MonthItem.
    */
    MonthItem *monthItem() const { return mMonthItem; }

    /**
      Returns the starting date of this item.
    */
    QDate startDate() const;

    /**
      Returns the number of day this item spans on minus one
      to be compatible with QDate::addDays().
    */
    int daySpan() const;

    /**
      Computed from startDate() and daySpan().
    */
    QDate endDate() const;

    void setStartDate( const QDate &d );
    void setDaySpan( int span );

    /**
      Returns true if this item is currently being moved (ie. the
      associated MonthItem is being moved).
    */
    bool isMoving() const;

    /**
      Returns true if this item is currently being resized (ie. the
      associated MonthItem is being moved).
    */
    bool isResizing() const;

    /**
      Returns true if this MonthGraphicsItem is the first one of the
      MonthItem ones.
    */
    bool isBeginItem() const;

    /**
      Returns true if this MonthGraphicsItem is the last one of the
      MonthItem ones.
    */
    bool isEndItem() const;

    /**
      Reimplemented from QGraphicsItem
    */
    virtual QRectF boundingRect() const;
    virtual void paint( QPainter *, const QStyleOptionGraphicsItem *, QWidget * );
    virtual QPainterPath shape() const;

  private:
    // Shape of the item, see shape()
    QPainterPath widgetPath( bool border ) const;

    // See startDate()
    QDate mStartDate;

    // See daySpan()
    int mDaySpan;

    // The current item is part of a MonthItem
    MonthItem *mMonthItem;
};

}

#endif //MONTHGRAPHICSITEMS_H
