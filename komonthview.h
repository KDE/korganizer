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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$

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

#include <libkcal/calendar.h>
#include <libkcal/event.h>

#include "koeventview.h"

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
};


class MonthViewItem: public QListBoxItem
{
  public:
    MonthViewItem( Incidence *, const QString & title );

    void setRecur(bool on) { mRecur = on; }
    void setAlarm(bool on) { mAlarm = on; }
    void setReply(bool on) { mReply = on; }

    void setPalette(const QPalette &p) { mPalette = p; }
    QPalette palette() const { return mPalette; }

    Incidence *incidence() const { return mIncidence; }

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

    void deselect();

  signals:
    void defaultAction( Incidence * );
    void newEventSignal( QDateTime );
    
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
    QPalette mHolidayPalette;
    QPalette mStandardPalette;
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
    virtual QPtrList<Incidence> selectedIncidences();

    virtual void printPreview(CalPrinter *calPrinter,
                              const QDate &, const QDate &);

  public slots:
    virtual void updateView();
    virtual void updateConfig();
    virtual void showDates(const QDate &start, const QDate &end);
    virtual void showEvents(QPtrList<Event> eventList);

    void changeEventDisplay(Event *, int);

    void showContextMenu( Incidence * );

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
    bool mWeekStartsMonday;
    
    QPtrVector<MonthViewCell> mCells;
    QPtrVector<QLabel> mDayLabels;

    bool mShortDayLabels;
    int mWidthLongDayLabel;

    QDate mStartDate;

    MonthViewCell *mSelectedCell;

    KOEventPopupMenu *mContextMenu;
};

#endif
