/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
*/
#ifndef KOAGENDAITEM_H
#define KOAGENDAITEM_H

#include "cellitem.h"

#include <libkcal/incidence.h>

#include <qframe.h>
#include <qdatetime.h>

class QToolTipGroup;
class QDragEnterEvent;
class QDropEvent;

using namespace KCal;
class KOAgendaItem;

struct MultiItemInfo
{
  int mStartCellX;
  int mStartCellXWidth;
  int mStartCellYTop,mStartCellYBottom;
  KOAgendaItem *mFirstMultiItem;
  KOAgendaItem *mPrevMultiItem;
  KOAgendaItem *mNextMultiItem;
  KOAgendaItem *mLastMultiItem;
};

/*
  The KOAgendaItem has to make sure that it receives all mouse events, which are
  to be used for dragging and resizing. That means it has to be installed as
  eventfiler for its children, if it has children, and it has to pass mouse
  events from the cildren to itself. See eventFilter().
*/
class KOAgendaItem : public QWidget, public KOrg::CellItem
{
    Q_OBJECT
  public:
    KOAgendaItem(Incidence *incidence, QDate qd, QWidget *parent, const char *name=0,
                 WFlags f=0 );

    int cellX() const { return mCellX; }
    int cellXWidth() const { return mCellXWidth; }
    int cellYTop() const { return mCellYTop; }
    int cellYBottom() const { return mCellYBottom; }
    int cellHeight() const;
    int cellWidth() const;

    void setCellXY(int X, int YTop, int YBottom);
    void setCellY(int YTop, int YBottom);
    void setCellX(int XLeft, int XRight);
    void setCellXWidth(int xwidth);

    /** Start movement */
    void startMove();
    /** Reset to original values */
    void resetMove();
    /** End the movement (i.e. clean up) */
    void endMove();

    void moveRelative(int dx,int dy);
    void expandTop(int dy);
    void expandBottom(int dy);
    void expandLeft(int dx);
    void expandRight(int dx);

    bool isMultiItem();
    KOAgendaItem *prevMoveItem() const { return (mStartMoveInfo)?(mStartMoveInfo->mPrevMultiItem):0; }
    KOAgendaItem *nextMoveItem() const { return (mStartMoveInfo)?(mStartMoveInfo->mNextMultiItem):0; }
    MultiItemInfo *moveInfo() const { return mStartMoveInfo; }
    void setMultiItem(KOAgendaItem *first,KOAgendaItem *prev,
                      KOAgendaItem *next, KOAgendaItem *last);
    KOAgendaItem *prependMoveItem(KOAgendaItem*);
    KOAgendaItem *appendMoveItem(KOAgendaItem*);
    KOAgendaItem *removeMoveItem(KOAgendaItem*);
    KOAgendaItem *firstMultiItem() const { return (mMultiItemInfo)?(mMultiItemInfo->mFirstMultiItem):0; }
    KOAgendaItem *prevMultiItem() const { return (mMultiItemInfo)?(mMultiItemInfo->mPrevMultiItem):0; }
    KOAgendaItem *nextMultiItem() const { return (mMultiItemInfo)?(mMultiItemInfo->mNextMultiItem):0; }
    KOAgendaItem *lastMultiItem() const { return (mMultiItemInfo)?(mMultiItemInfo->mLastMultiItem):0; }

    Incidence *incidence() const { return mIncidence; }
    QDate itemDate() { return mDate; }

    /** Update the date of this item's occurrence (not in the event) */
    void setItemDate(QDate qd);

    void setText ( const QString & text ) { mLabelText = text; }
    QString text () { return mLabelText; }

    static QToolTipGroup *toolTipGroup();

    QPtrList<KOAgendaItem> conflictItems();
    void setConflictItems(QPtrList<KOAgendaItem>);
    void addConflictItem(KOAgendaItem *ci);

    QString label() const;

    bool overlaps( KOrg::CellItem * ) const;

  public slots:
    void updateIcons();
    void select(bool=true);
    void addAttendee(QString);

  protected:
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void paintEvent(QPaintEvent *e);
    void paintFrame(QPainter *p, const QColor &color);
    void paintTodoIcon(QPainter *p, int &x, int ft);
    /** private movement functions. startMove needs to be called of only one of
     *  the multitems. it will then loop through the whole series using
     *  startMovePrivate. Same for resetMove and endMove */
    void startMovePrivate();
    void resetMovePrivate();
    void endMovePrivate();


  private:
    int mCellX;
    int mCellXWidth;
    int mCellYTop,mCellYBottom;
    int mSubCell;  // subcell number of this item
    int mSubCells;  // Total number of subcells in cell of this item

    Incidence *mIncidence; // corresponding event or todo
    QDate mDate; //date this events occurs (for recurrence)
    QString mLabelText;
    bool mIconAlarm, mIconRecur, mIconReadonly;
    bool mIconReply, mIconGroup, mIconOrganizer;

    // Multi item pointers
    MultiItemInfo* mMultiItemInfo;
  protected:
    // Variables to remember start position
    MultiItemInfo* mStartMoveInfo;

  private:
    static QToolTipGroup *mToolTipGroup;

    bool mSelected;
    QPtrList<KOAgendaItem> mConflictItems;
};

#endif
