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
#include <QList>
#include <QDate>

class QPixmap;
class QColor;
class QString;

namespace KCal {
  class Incidence;
}
using namespace KCal;

namespace KOrg {

class MonthGraphicsItem;
class MonthScene;

/**
 * A month item manages different MonthGraphicsItems.
 */
class MonthItem : public QObject
{
  Q_OBJECT

  public:
    MonthItem( MonthScene *monthWidget );
    virtual ~MonthItem();

    /**
      Compares two events

      The month view displays a list of items. When loading (which occurs each
      time there is a change), the items are sorted :
      - smallest date
      - bigger span
      - floating
      - finally, time in the day
    */
    static bool greaterThan( const MonthItem *e1, const MonthItem *e2 );

    /**
      Compare this event with a second one, if the former function is not
      able to sort them.
    */
    virtual bool greaterThanFallback( const MonthItem *other ) const;

    /**
      The start date of the incidence, generally realStartDate. But it
      reflect changes, even during move.
    */
    QDate startDate() const;

    /**
      The end date of the incidence, generally realEndDate. But it
      reflect changes, even during move.
     */
    QDate endDate() const;

    /**
      The number of days this item spans.
    */
    int daySpan() const;

    /**
      This is the real start date, usually the start date of the incidence.
    */
    virtual QDate realStartDate() const = 0;

    /**
      This is the real end date, usually the end date of the incidence.
    */
    virtual QDate realEndDate() const = 0;

    /**
      True if this item last all the day.
    */
    virtual bool allDay() const = 0;

    /**
      Updates geometry of all MonthGraphicsItems.
    */
    void updateGeometry();

    /**
      Find the lowest possible position for this item.

      The position of an item in a cell is it's vertical position. This is used
      to avoid overlapping of items. An item keeps the same position in every
      cell it crosses. The position is measured from top to bottom.
    */
    void updatePosition();

    /**
      Returns true if this item is selected.
    */
    bool selected() const { return mSelected; }

    /**
      Returns the position of the item ( > 0 ).
    */
    int position() const { return mPosition; }

    /**
      Returns the associated month scene to this item.
    */
    MonthScene *monthScene() const { return mMonthScene; }

    /**
      Begin a move.
    */
    void beginMove();

    /**
      End a move.
    */
    void endMove();

    /**
      Begin a resize.
    */
    void beginResize();

    /**
      End a resize.
    */
    void endResize();

    /**
      Called during move to move the item a bit, relative to the previous
      move step.
    */
    void moveBy( int offsetFromPreviousDate );

    /**
      Called during resize to rezie the item a bit, relative to the previous
      resize step.
    */
    bool resizeBy( int offsetFromPreviousDate );

    /**
      Returns true if the item is being moved.
    */
    bool isMoving() const { return mMoving; }

    /**
      Returns true if the item is being resized.
    */
    bool isResizing() const { return mResizing; }

    /**
      Returns true if the item can be moved.
    */
    virtual bool isMoveable() const = 0;

    /**
      Returns true if the item can be resized.
    */
    virtual bool isResizable() const = 0;

    /**
      Deletes all MonthGraphicsItem this item handles. Clear the list.
    */
    void deleteAll( );

    /**
      Update the monthgraphicsitems

      This basically deletes and rebuild all the MonthGraphicsItems but tries
        to do it wisely:
      - If there is a moving item, it won't be deleted because then the
        new item won't receive anymore the MouseMove events.
      - If there is an item on a line where the new state needs an item,
        it is used and not deleted. This will avoid flickers.
    */
    void updateMonthGraphicsItems();

    /**
      Sets the selection state of this item.
    */
    void setSelected( bool selected ) { mSelected = selected; }

    // METHODS NEEDED TO PAINT ITEMS

    /**
      Returns the text to draw in an item.

     @param end True if the text at the end of an item should be returned.
    */
    virtual QString text( bool end ) const = 0;

    /**
       Returns the text for the tooltip of the item
     */
    virtual QString toolTipText() const = 0;

    /**
      Returns the background color of the item.
    */
    virtual QColor bgColor() const = 0;

    /**
      Returns the frame color of the item.
    */
    virtual QColor frameColor() const = 0;

    /**
      Returns a list of pixmaps to draw next to the items.
    */
    virtual QList<QPixmap *> icons() const = 0;

  protected:
    /**
      Called after a move operation.
    */
    virtual void finalizeMove( const QDate &newStartDate ) = 0;

    /**
      Called after a resize operation.
    */
    virtual void finalizeResize( const QDate &newStartDate,
                                 const QDate &newEndDate ) = 0;

  private:
    /**
      Sets the value of all MonthGraphicsItem to @param z.
    */
    void setZValue( qreal z );

    QList<MonthGraphicsItem *> mMonthGraphicsItemList;

    MonthScene *mMonthScene;

    bool mSelected;
    bool mMoving; // during move
    bool mResizing; // during resize
    QDate mOverrideStartDate;
    int mOverrideDaySpan;

    int mPosition;
};

class IncidenceMonthItem : public MonthItem
{
  Q_OBJECT

  public:
    IncidenceMonthItem( MonthScene *monthScene, Incidence *incidence,
                        const QDate &recurStartDate = QDate() );
    virtual ~IncidenceMonthItem();

    Incidence *incidence() const { return mIncidence; }

    virtual bool greaterThanFallback( const MonthItem *other ) const;

    virtual QDate realStartDate() const;
    virtual QDate realEndDate() const;
    virtual bool allDay() const;

    virtual bool isMoveable() const;
    virtual bool isResizable() const;

    QString text( bool end ) const;
    QString toolTipText() const;

    QColor bgColor() const;
    QColor frameColor() const;

    QList<QPixmap *> icons() const;

  protected:
    virtual void finalizeMove( const QDate &newStartDate );
    virtual void finalizeResize( const QDate &newStartDate,
                                 const QDate &newEndDate );

  protected slots:
    /**
      Update the selected state of this item.
      If will be selected if incidence is the incidence managed by this item.
      Else it will be deselected.
    */
    void updateSelection( Incidence *incidence );

  private:
    void updateDates( int startOffset, int endOffset );

    /**
      Returns the category color for this incidence.
    */
    QColor catColor() const;

    Incidence *mIncidence;
    int mRecurDayOffset;
    bool mIsEvent, mIsTodo, mIsJournal;
};

class HolidayMonthItem : public MonthItem
{
  Q_OBJECT

  public:
    HolidayMonthItem( MonthScene *monthScene, const QDate &date, const QString &name );
    virtual ~HolidayMonthItem();

    virtual bool greaterThanFallback( const MonthItem *other ) const;

    virtual QDate realStartDate() const { return mDate; }
    virtual QDate realEndDate() const { return mDate; }
    virtual bool allDay() const { return true; }

    virtual bool isMoveable() const { return false; }
    virtual bool isResizable() const { return false; }

    QString text( bool end ) const
    {
      Q_UNUSED( end );
      return mName;
    }
    QString toolTipText() const { return mName; }

    QColor bgColor() const;
    QColor frameColor() const;

    QList<QPixmap *> icons() const;

  protected:
    virtual void finalizeMove( const QDate &newStartDate );
    virtual void finalizeResize( const QDate &newStartDate,
                                 const QDate &newEndDate );

  private:
    QDate mDate;
    QString mName;
};

}

#endif
