/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef _KOMONTHVIEW_H
#define _KOMONTHVIEW_H

#include <q3listbox.h>
#include <q3ptrvector.h>
#include <QToolTip>
//Added by qt3to4:
#include <QPixmap>
#include <QKeyEvent>
#include <QLabel>
#include <QResizeEvent>
#include <QMouseEvent>
#include "koeventview.h"

class KNoScrollListBox;

#if 0
class KOMonthCellToolTip : public QToolTip
{
  public:
    KOMonthCellToolTip (QWidget* parent, KNoScrollListBox* lv );

  protected:
    void maybeTip( const QPoint & pos);

  private:
    KNoScrollListBox* eventlist;
};
#endif


class KNoScrollListBox: public Q3ListBox
{
    Q_OBJECT
  public:
    KNoScrollListBox( QWidget *parent=0 );
    ~KNoScrollListBox() {}

    void setBackground( bool primary, bool workday );

  signals:
    void shiftDown();
    void shiftUp();
    void rightClick();

  protected slots:
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void mousePressEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);
    void contentsMouseDoubleClickEvent( QMouseEvent * e );

  private:
    bool mSqueezing;
};


class MonthViewItem: public Q3ListBoxItem
{
  public:
    MonthViewItem( Incidence *, const QDateTime &qd, const QString & title );

    void setTodo(bool on)  { mTodo  = on; }
    void setTodoDone(bool on) { mTodoDone = on; }
    void setRecur(bool on) { mRecur = on; }
    void setAlarm(bool on) { mAlarm = on; }
    void setReply(bool on) { mReply = on; }

    void setPalette(const QPalette &p) { mPalette = p; }
    QPalette palette() const { return mPalette; }

    Incidence *incidence() const { return mIncidence; }
    QDateTime incidenceDateTime() { return mDateTime; }

    void setResourceColor( QColor& color ) { mResourceColor = color; }
    QColor &resourceColor() { return mResourceColor; }
  protected:
    virtual void paint(QPainter *);
    virtual int height(const Q3ListBox *) const;
    virtual int width(const Q3ListBox *) const;
    //Color of the resource
    QColor mResourceColor;
  private:
    bool mTodo;
    bool mTodoDone;
    bool mRecur;
    bool mAlarm;
    bool mReply;

    QPixmap mTodoPixmap;
    QPixmap mTodoDonePixmap;
    QPixmap mAlarmPixmap;
    QPixmap mRecurPixmap;
    QPixmap mReplyPixmap;

    QPalette mPalette;
    QDateTime mDateTime;

    Incidence *mIncidence;
};


class KOMonthView;

/** This class represents one day in KOrganizer's month view.

@see KOMonthView
*/
class MonthViewCell : public QWidget
{
    Q_OBJECT
  public:
    MonthViewCell( KOMonthView * );

    /** Sets the date of the cell */
    void setDate( const QDate & );
    /** @return Date of cell */
    QDate date() const;

    /**
      Set this cell as primary if @p primary is true. A primary cell belongs
      to the current month. A non-primary cell belongs to the month before or
      after the current month.
      @param primary If true, the cell will be set as primary. Else it will be
      set as non-primary.
     */
    void setPrimary( bool primary );
    /**
       @return True if this cell is primary, else false.
    */
    bool isPrimary() const;

    /** Make this cell show as a holiday
      @param isHoliday Whether this day is a holiday or not
    */
    void setHoliday( bool isHoliday );
    /**
      Sets the holiday name to this cell. This will not call
      setHoliday( true ).
      @param name The name of the holiday.
    */
    void setHolidayString( const QString &name );

    void updateCell();
    /** Adds an incidence to the cell.
        Sets the right text and icons for this incidence.
    */
    void addIncidence( Incidence * );
    /** Removes an incidence from the cell.
        @return True if successful, false if deletion failed
       (e.g. when given incidence doesn't exist in the cell.
    */
    bool removeIncidence( Incidence * );

    void updateConfig();

    void enableScrollBars( bool );

    Incidence *selectedIncidence();
    QDate selectedIncidenceDate();

    void deselect();

    void setCalendar( Calendar*cal ) { mCalendar = cal; }
  signals:
    void defaultAction( Incidence * );
    /**
      Notify the view manager that we want to create a new event, so an editor
      will pop up. Pass the date for the new event as parameter.
    */
    void newEventSignal( const QDate & );

  public slots:
    void select();

  protected:
    void setFrameWidth();
    void resizeEvent( QResizeEvent * );

  protected slots:
    void defaultAction( Q3ListBoxItem * );
    void contextMenu( Q3ListBoxItem * );

  private:
    class CreateItemVisitor;
    KOMonthView *mMonthView;
  // We need the calendar for paint the ResourceColor
    Calendar *mCalendar;

    QDate mDate;
    bool mPrimary;
    bool mHoliday;
    QString mHolidayString;

    QLabel *mLabel;
    KNoScrollListBox *mItemList;

    QSize mLabelSize;
//    QPalette mOriginalPalette;
    QPalette mHolidayPalette;
    QPalette mStandardPalette;
    QPalette mTodayPalette;
};

/**
  The class KOMonthView represents the monthly view in KOrganizer.
  It holds several instances of the class MonthViewCell.

  @short KOMonthview represents the montly view in KOrganizer.
  @see KOBaseView, KODayListView, MonthViewCell
*/
class KOMonthView: public KOEventView
{
    Q_OBJECT
  public:
    KOMonthView(Calendar *cal, QWidget *parent = 0 );
    ~KOMonthView();

    /** Returns maximum number of days supported by the komonthview */
    virtual int maxDatesHint();

    /** Returns number of currently shown dates. */
    virtual int currentDateCount();

    /** Returns the currently selected events */
    virtual Incidence::List selectedIncidences();

    /** Returns dates of the currently selected events */
    virtual DateList selectedDates();

    virtual bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay);

  public slots:
    virtual void updateView();
    virtual void updateConfig();
    virtual void showDates(const QDate &start, const QDate &end);
    virtual void showIncidences( const Incidence::List &incidenceList );

    void changeIncidenceDisplay(Incidence *, int);
    void changeIncidenceDisplayAdded(Incidence *);

    void clearSelection();

    void showEventContextMenu( Incidence *, const QDate & );
    void showGeneralContextMenu();

    void setSelectedCell( MonthViewCell * );

  protected slots:
    void processSelectionChange();

  protected:
    void resizeEvent(QResizeEvent *);

    void viewChanged();
    void updateDayLabels();

    /** @param date The date of the cell which should be returned.
    @return Pointer of monthcell with given date, otherwise 0 */
    MonthViewCell *lookupCellByDate( const QDate &date );

  private:
    class GetDateVisitor;
    int mDaysPerWeek;
    int mNumWeeks;
    int mNumCells;
    int mWeekStartDay;

    QVector<MonthViewCell*> mCells;
    QVector<QLabel*> mDayLabels;

    bool mShortDayLabels;
    int mWidthLongDayLabel;

    QDate mStartDate;
    QDate mSelectedDate;

    MonthViewCell *mSelectedCell;

    KOEventPopupMenu *mEventContextMenu;
    QLabel *mLabel;
};

#endif
