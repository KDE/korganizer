/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef _KOMONTHVIEW_H
#define _KOMONTHVIEW_H

#include <qlabel.h>
#include <qframe.h>
#include <qdatetime.h>
#include <qlistbox.h>
#include <qlayout.h>
#include <qintdict.h>
#include <qpushbutton.h>
#include <qvaluelist.h>
#include <qptrvector.h>
#include <qtooltip.h>

#include <libkcal/calendar.h>
#include <libkcal/event.h>

#include "koeventview.h"

class KNoScrollListBox;

class KOMonthCellToolTip : public QToolTip
{
  public:
    KOMonthCellToolTip (QWidget* parent, KNoScrollListBox* lv );

  protected:
    void maybeTip( const QPoint & pos);

  private:
    KNoScrollListBox* eventlist;
};


class KNoScrollListBox: public QListBox
{
    Q_OBJECT
  public:
    KNoScrollListBox(QWidget *parent=0, const char *name=0);
    ~KNoScrollListBox() {}

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


class MonthViewItem: public QListBoxItem
{
  public:
    MonthViewItem( Incidence *, QDate qd, const QString & title );

    void setRecur(bool on) { mRecur = on; }
    void setAlarm(bool on) { mAlarm = on; }
    void setReply(bool on) { mReply = on; }

    void setPalette(const QPalette &p) { mPalette = p; }
    QPalette palette() const { return mPalette; }

    Incidence *incidence() const { return mIncidence; }
    QDate incidenceDate() { return mDate; }

  protected:
    virtual void paint(QPainter *);
    virtual int height(const QListBox *) const;
    virtual int width(const QListBox *) const;

  private:
    bool mRecur;
    bool mAlarm;
    bool mReply;

    QPixmap mAlarmPixmap;
    QPixmap mRecurPixmap;
    QPixmap mReplyPixmap;

    QPalette mPalette;
    QDate mDate;

    Incidence *mIncidence;
};


class KOMonthView;

class MonthViewCell : public QWidget
{
    Q_OBJECT
  public:
    MonthViewCell( KOMonthView * );

    void setDate( const QDate & );
    QDate date() const;

    void setPrimary( bool );
    bool isPrimary() const;

    void setHoliday( bool );
    void setHoliday( const QString & );

    void updateCell();

    void updateConfig();

    void enableScrollBars( bool );

    Incidence *selectedIncidence();
    QDate selectedIncidenceDate();

    void deselect();

  signals:
    void defaultAction( Incidence * );
    void newEventSignal( QDate );

  protected:
    void resizeEvent( QResizeEvent * );

  protected slots:
    void defaultAction( QListBoxItem * );
    void contextMenu( QListBoxItem * );
    void selection( QListBoxItem * );
    void cellClicked( QListBoxItem * );

  private:
    KOMonthView *mMonthView;

    QDate mDate;
    bool mPrimary;
    bool mHoliday;
    QString mHolidayString;

    QLabel *mLabel;
    QListBox *mItemList;

    QSize mLabelSize;
//    QPalette mOriginalPalette;
    QPalette mHolidayPalette;
    QPalette mStandardPalette;
    QPalette mTodayPalette;
};


class KOMonthView: public KOEventView
{
    Q_OBJECT
  public:
    KOMonthView(Calendar *cal, QWidget *parent = 0, const char *name = 0 );
    ~KOMonthView();

    /** Returns maximum number of days supported by the komonthview */
    virtual int maxDatesHint();

    /** Returns number of currently shown dates. */
    virtual int currentDateCount();

    /** returns the currently selected events */
    virtual Incidence::List selectedIncidences();

    /** returns dates of the currently selected events */
    virtual DateList selectedDates();

    virtual bool eventDurationHint(QDateTime &startDt, QDateTime &endDt, bool &allDay);

    virtual void printPreview(CalPrinter *calPrinter,
                              const QDate &, const QDate &);

  public slots:
    virtual void updateView();
    virtual void updateConfig();
    virtual void showDates(const QDate &start, const QDate &end);
    virtual void showEvents( const Event::List &eventList );

    void changeEventDisplay(Event *, int);

    void clearSelection();

    void showEventContextMenu( Incidence * );
    void showGeneralContextMenu();

    void setSelectedCell( MonthViewCell * );

  protected slots:
    void processSelectionChange();

  protected:
    void resizeEvent(QResizeEvent *);

    void viewChanged();
    void updateDayLabels();

  private:
    int mDaysPerWeek;
    int mNumWeeks;
    int mNumCells;
    int mWeekStartDay;

    QPtrVector<MonthViewCell> mCells;
    QPtrVector<QLabel> mDayLabels;

    bool mShortDayLabels;
    int mWidthLongDayLabel;

    QDate mStartDate;
    QDate mSelectedDate;

    MonthViewCell *mSelectedCell;

    KOEventPopupMenu *mEventContextMenu;
    QPopupMenu *mGeneralContextMenu;
};

#endif
