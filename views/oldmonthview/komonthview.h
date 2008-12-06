/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2008      Thomas Thrainer <tom_t@gmx.at>

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

#ifndef KOMONTHVIEW_H
#define KOMONTHVIEW_H

#include "koeventview.h"

#include <KHBox>

#include <QListWidget>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QResizeEvent>

class CaptureClickListBox: public QListWidget
{
  Q_OBJECT
  public:
    explicit CaptureClickListBox( QWidget *parent=0 );
    ~CaptureClickListBox() {}

  protected:
    virtual void mousePressEvent( QMouseEvent * );

  signals:
    /** Emitted when the list box is clicked anywhere.
     *
     *  This signal is not only emitted when the user
     *  clicks on an item like clicked() but also when
     *  only the backgroud is clicked.
     *  clicked() is emitted separately.
    */
    void listBoxPressed();
};

/** This class represents one item in the list of events of a day */
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

    /** Update the items appearance (icons and colors) */
    void drawIt();

  protected:
    virtual int height( const QListWidget * ) const;
    virtual int width( const QListWidget * ) const;

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

    //Color of the resource, overrides the palette color if valid.
    QColor mResourceColor;
    //The palette to draw the item. The category color is in there.
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
  Q_PROPERTY( QDate date READ date WRITE setDate )
  Q_PROPERTY( bool evenMonth READ isEvenMonth )
  Q_PROPERTY( bool firstDay READ isFirstDay )
  Q_PROPERTY( bool today READ isToday )
  Q_PROPERTY( bool workday READ isWorkday )
  Q_PROPERTY( bool holiday READ holiday WRITE setHoliday )
  Q_PROPERTY( QString holidayString READ holidayString WRITE setHolidayString )
  Q_PROPERTY( bool selected READ isSelected WRITE setSelected )

  public:
    explicit MonthViewCell( KOMonthView * );

    /** Sets the date of the cell */
    void setDate( const QDate & );

    /** @return Date of cell */
    QDate date() const { return mDate; }

    /** @return true if this cell is in an even month */
    bool isEvenMonth() const { return mEvenMonth; }

    /** @return true if this cell is the first day of the month */
    bool isFirstDay() const { return mFirstDay; }

    /** @return true if this cell's date is today */
    bool isToday() const { return mToday; }

    /** @return true if this cell is a workday */
    bool isWorkday() const { return mWorkday; }

    /** Make this cell show as a holiday
      @param isHoliday Whether this day is a holiday or not
    */
    void setHoliday( bool isHoliday );

    /** @return true if this cell represents a holiday */
    bool holiday() const { return mHoliday; }

    /**
      Sets the holiday name to this cell. This will not call
      setHoliday( true ).
      @param name The name of the holiday.
    */
    void setHolidayString( const QString &name );

    /** @return The current holiday string */
    QString holidayString() const { return mHolidayString; }

    /**
       Sets the selection status of the cell
       @param selected Whether this day is selected of not
    */
    void setSelected( bool selected );

    /** @return true if this cell is selected */
    bool isSelected() { return mSelected; }

    /**
       Initializes the day to only display the current
       holiday if set.
     */
    void updateCell();

    /** Adds an incidence to the cell.
        Sets the right text and icons for this incidence.
        @param incidence The incidence to be added.
        @param multiDay Specifies which day of a multi-day event is added to the
        cell. The first day is 0 (default).
    */
    void addIncidence( Incidence *, int multDay=0 );

    /** Removes an incidence from the cell. */
    void removeIncidence( Incidence * );

    /**
       Forces reapplying the stylesheet for this day.

       This method should be called whenever the status of the cell changes
       (selected, date, holiday, etc.)
     */
    void updateStyles();
    void updateConfig();

    /** @return A pointer to the selected incidence or 0 is no selection */
    Incidence *selectedIncidence();
    /** @return The date of the selected incidence of QDate() if no selection */
    QDate selectedIncidenceDate();

    void setCalendar( Calendar *cal ) { mCalendar = cal; }
  signals:
    void defaultAction( Incidence * );

    /**
      Notify the view manager that we want to create a new event, so an editor
      will pop up. Pass the date for the new event as parameter.
    */
    void newEventSignal( const QDate & );

  public slots:
    /** Set this cell as selected. Calls setSelected( true ) */
    void select();

  protected:
    void enableScrollBars( bool );
    void resizeEvent( QResizeEvent * );
    virtual void mousePressEvent( QMouseEvent * );
    virtual void paintEvent( QPaintEvent * );

  protected slots:
    void defaultAction( QListWidgetItem * );
    void contextMenu( const QPoint &pos );

  private:
    class CreateItemVisitor;
    KOMonthView *mMonthView;
    // We need the calendar to paint the ResourceColor
    Calendar *mCalendar;

    QDate mDate;
    bool mEvenMonth;
    bool mFirstDay;
    bool mToday;
    bool mWorkday;
    bool mHoliday;
    bool mSelected;
    QString mHolidayString;

    QLabel *mLabel;
    CaptureClickListBox *mItemList;

    QSize mLabelSize;
};

// forward declaration for KOMonthView
class QGridLayout;

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
    void changeIncidenceDisplayAdded( Incidence *,
                                      int start = -1, int end = -1 );

    void showEventContextMenu( Incidence *, const QDate & );
    void showGeneralContextMenu();

    /** Called by MonthViewCell to indicate a new selection. */
    void setSelectedCell( MonthViewCell * );
    /** Deselects all selected cells. */
    void clearSelection();

  protected slots:
    void processSelectionChange();
    void moveBackMonth();
    void moveBackWeek();
    void moveFwdWeek();
    void moveFwdMonth();

  protected:
    void resizeEvent( QResizeEvent * );

    void viewChanged();
    void updateDayLabels();

    void swapCells( int srcRow, int srcCol, int dstRow, int dstCol );
    void moveStartDate( int weeks, int months );
    void setStartDate( const QDate &start );

    void updateView( int start, int end );
    void updateCells( int start, int end );

    virtual void wheelEvent( QWheelEvent *event );
    virtual void keyPressEvent( QKeyEvent *event );

  private:
    class GetDateVisitor;
    Calendar *mCalendar;
    int mDaysPerWeek;
    int mNumWeeks;
    int mNumCells;
    int mWeekStartDay;

    QVector<MonthViewCell*> mCells;
    MonthViewCell *getCell( const QDate &date );

    QVector<QLabel*> mDayLabels;

    bool mShortDayLabels;
    int mWidthLongDayLabel;

    QDate mStartDate;

    /* TODO: add support for selecting a range of dates, not only one */
    QDate mSelectedDate;

    QGridLayout *mDayLayout;

    KOEventPopupMenu *mEventContextMenu;
    KHBox *mTopBox;
    QLabel *mTitle;
    QFrame *mDecorationsFrame;
};

#endif
