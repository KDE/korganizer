#ifndef KOAGENDAITEM_H
#define KOAGENDAITEM_H
// $Id$

#include <qframe.h>
#include <qlabel.h>

#include "koevent.h"

class QToolTipGroup;

/*
  The KOAgendaItem has to make sure that it receives all mouse events, which are
  to be used for dragging and resizing. That means it has to be installed as
  eventfiler for its children, if it has children, and it has to pass mouse
  events from the cildren to itself. See eventFilter().
*/
class KOAgendaItem : public QFrame
{
    Q_OBJECT
  public:
    KOAgendaItem(KOEvent *event, QWidget *parent, const char *name=0,
                 WFlags f=0 );

    int cellX() { return mCellX; }
    int cellXWidth() { return mCellXWidth; }
    int cellYTop() { return mCellYTop; }
    int cellYBottom() { return mCellYBottom; }
    int cellHeight();
    int cellWidth();
    int subCell() { return mSubCell; }
    int subCells() { return mSubCells; }

    void setCellXY(int X, int YTop, int YBottom);
    void setCellY(int YTop, int YBottom);
    void setCellX(int XLeft, int XRight);
    void setCellXWidth(int xwidth);
    void setSubCell(int subCell);
    void setSubCells(int subCells);

    /** Start movement */
    void startMove();
    /** Reset to original values */
    void resetMove();

    void moveRelative(int dx,int dy);
    void expandTop(int dy);
    void expandBottom(int dy);
    void expandLeft(int dx);
    void expandRight(int dx);
        
    void setMultiItem(KOAgendaItem *first,KOAgendaItem *next,
                      KOAgendaItem *last);
    KOAgendaItem *firstMultiItem() { return mFirstMultiItem; }
    KOAgendaItem *nextMultiItem() { return mNextMultiItem; }
    KOAgendaItem *lastMultiItem() { return mLastMultiItem; }

    KOEvent *itemEvent() { return mEvent; }

    void setText ( const QString & text ) { mItemLabel->setText(text); }
    QString text () { return mItemLabel->text(); }

    virtual bool eventFilter ( QObject *, QEvent * ); 

    static QToolTipGroup *toolTipGroup();

  public slots:
    void updateIcons();
    void select(bool=true);

  private:
    int mCellX;
    int mCellXWidth;
    int mCellYTop,mCellYBottom;
    int mSubCell;  // subcell number of this item
    int mSubCells;  // Total number of subcells in cell of this item

    // Variables to remember start position
    int mStartCellX;
    int mStartCellXWidth;
    int mStartCellYTop,mStartCellYBottom;
    
    // Multi item pointers
    KOAgendaItem *mFirstMultiItem;
    KOAgendaItem *mNextMultiItem;
    KOAgendaItem *mLastMultiItem;
    
    KOEvent *mEvent; // corresponding event
    
    QLabel *mItemLabel;
    QLabel *mIconAlarm,*mIconRecur,*mIconReadonly;
    
    static QToolTipGroup *mToolTipGroup;

    bool mSelected;
};

#endif // KOAGENDAITEM_H
