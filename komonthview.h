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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef _KOMONTHVIEW_H
#define _KOMONTHVIEW_H

#include "koeventview.h"

#include <KHBox>

#include <QListWidget>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QResizeEvent>

class KNoScrollListBox: public QListWidget
{
  Q_OBJECT
  public:
    explicit KNoScrollListBox( QWidget *parent=0 );
    ~KNoScrollListBox() {}

    void setBackground( bool primary, bool workday );
};

class MonthViewItem: public QListWidgetItem
{
  public:
    MonthViewItem( Incidence *, const KDateTime &dt, const QString &title );

    void setEvent( bool on ) { mEvent  = on; }
    void setJournal( bool on ) { mJournal  = on; }
    void setTodo( bool on ) { mTodo  = on; }
    void setTodoDone( bool on ) { mTodoDone = on; }
    void setRecur( bool on ) { mRecur = on; }
    void setAlarm( bool on ) { mAlarm = on; }
    void setReply( bool on ) { mReply = on; }
    void setHoliday( bool on ) { mHoliday = on; }

    void setPalette( const QPalette &p ) { mPalette = p; }
    QPalette palette() const { return mPalette; }

    Incidence *incidence() const { return mIncidence; }
    KDateTime incidenceDateTime() { return mDateTime; }

    void setResourceColor( QColor &color ) { mResourceColor = color; }
    QColor &resourceColor() { return mResourceColor; }
    void drawIt();

  protected:
    virtual int height( const QListWidget * ) const;
    virtual int width( const QListWidget * ) const;
    //Color of the resource
    QColor mResourceColor;

  private:
    bool mEvent;
    bool mTodo;
    bool mTodoDone;
    bool mJournal;
    bool mRecur;
    bool mAlarm;
    bool mReply;
    bool mHoliday;

    QPixmap mEventPixmap;
    QPixmap mTodoPixmap;
    QPixmap mTodoDonePixmap;
    QPixmap mJournalPixmap;
    QPixmap mAlarmPixmap;
    QPixmap mRecurPixmap;
    QPixmap mReplyPixmap;
    QPixmap mHolidayPixmap;

    QPalette mPalette;
    KDateTime mDateTime;

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
    explicit MonthViewCell( KOMonthView * );

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
        @param incidence The incidence to be added.
        @param multiDay Specifies which day of a multi-day event is added to the
        cell. The first day is 0 (default).
    */
    void addIncidence( Incidence *, int multDay=0 );

    /** Removes an incidence from the cell.
        @return True if successful, false if deletion failed
       (e.g. when given incidence doesn't exist in the cell.
    */
    void removeIncidence( Incidence * );

    void updateConfig();

    void enableScrollBars( bool );

    Incidence *selectedIncidence();
    QDate selectedIncidenceDate();

    void deselect();

    void setCalendar( Calendar *cal ) { mCalendar = cal; }
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
    void defaultAction( QListWidgetItem * );
    void contextMenu( const QPoint &pos );

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

  @short KOMonthview represents the monthly view in KOrganizer.
  @see KOBaseView, KODayListView, MonthViewCell
*/
class KOMonthView: public KOEventView
{
  Q_OBJECT
  public:
    explicit KOMonthView( Calendar *cal, QWidget *parent = 0 );
    ~KOMonthView();

    /** Returns maximum number of days supported by the komonthview */
    virtual int maxDatesHint();

    /** Returns number of currently shown dates. */
    virtual int currentDateCount();

    /** Returns the currently selected events */
    virtual Incidence::List selectedIncidences();

    /** Returns dates of the currently selected events */
    virtual DateList selectedDates();

    virtual bool eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay );

  public slots:
    virtual void updateView();
    virtual void updateConfig();
    virtual void showDates( const QDate &start, const QDate &end );
    virtual void showIncidences( const Incidence::List &incidenceList );

    void changeIncidenceDisplay( Incidence *, int );
    void changeIncidenceDisplayAdded( Incidence * );

    void clearSelection();

    void showEventContextMenu( Incidence *, const QDate & );
    void showGeneralContextMenu();

    void setSelectedCell( MonthViewCell * );

  protected slots:
    void processSelectionChange();

  protected:
    void resizeEvent( QResizeEvent * );

    void viewChanged();
    void updateDayLabels();

  private:
    class GetDateVisitor;
    int mDaysPerWeek;
    int mNumWeeks;
    int mNumCells;
    int mWeekStartDay;

    QVector<MonthViewCell*> mCells;
    QMap<QDate, MonthViewCell *>mDateToCell;
    QVector<QLabel*> mDayLabels;

    bool mShortDayLabels;
    int mWidthLongDayLabel;

    QDate mStartDate;
    QDate mSelectedDate;

    MonthViewCell *mSelectedCell;

    KOEventPopupMenu *mEventContextMenu;
    KHBox *mTopBox;
    QLabel *mTitle;
    QFrame *mDecorationsFrame;
};

#endif
