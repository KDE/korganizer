/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001,2002 Cornelius Schumacher <schumacher@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _KOEDITORRECURRENCE_H
#define _KOEDITORRECURRENCE_H

#include <qframe.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qmultilineedit.h>
#include <qlistview.h>
#include <qradiobutton.h>

#include <libkcal/event.h>

#include "ktimeedit.h"

class QWidgetStack;
class QSpinBox;

class KDateEdit;

using namespace KCal;

class RecurBase : public QWidget
{
  public:
    RecurBase( QWidget *parent = 0, const char *name = 0 );
  
    void setFrequency( int );
    int frequency();

    QWidget *frequencyEdit();

  private:
    QSpinBox *mFrequencyEdit;
};

class RecurDaily : public RecurBase
{
  public:
    RecurDaily( QWidget *parent = 0, const char *name = 0 );
};

class RecurWeekly : public RecurBase
{
  public:
    RecurWeekly( QWidget *parent = 0, const char *name = 0 );
  
    void setDays( const QBitArray & );
    QBitArray days();
    
  private:
    QCheckBox *mSundayBox;
    QCheckBox *mMondayBox;
    QCheckBox *mTuesdayBox;
    QCheckBox *mWednesdayBox;
    QCheckBox *mThursdayBox;
    QCheckBox *mFridayBox;
    QCheckBox *mSaturdayBox;
};

class RecurMonthly : public RecurBase
{
  public:
    RecurMonthly( QWidget *parent = 0, const char *name = 0 );

    void setByDay( int day );
    void setByPos( int count, int weekday );
    
    bool byDay();
    bool byPos();
    
    int day();

    int count();
    int weekday();
    
  private:
    QRadioButton *mByDayRadio;
    QComboBox *mByDayCombo;

    QRadioButton *mByPosRadio;
    QComboBox *mByPosCountCombo;
    QComboBox *mByPosWeekdayCombo;
};

class RecurYearly : public RecurBase
{
  public:
    RecurYearly( QWidget *parent = 0, const char *name = 0 );
    
    void setByDay();
    void setByMonth( int month );
    
    bool byMonth();
    bool byDay();

    int month();
    
  private:
    QRadioButton *mByMonthRadio;
    QComboBox *mByMonthCombo;
    
    QRadioButton *mByDayRadio;
};

class ExceptionsWidget : public QWidget
{
    Q_OBJECT
  public:
    ExceptionsWidget( QWidget *parent = 0, const char *name = 0 );
    
    void setDates( const DateList & );
    DateList dates();
    
  protected slots:
    void addException();
    void changeException();
    void deleteException();
    
  private:
    KDateEdit *mExceptionDateEdit;
    QListBox *mExceptionList;
    DateList mExceptionDates;  
};

class KOEditorRecurrence : public QWidget
{
    Q_OBJECT
  public:
    KOEditorRecurrence ( QWidget *parent = 0, const char *name = 0 );
    virtual ~KOEditorRecurrence();

    /** Set widgets to default values */
    void setDefaults( QDateTime from, QDateTime to, bool allday );
    /** Read event object and setup widgets accordingly */
    void readEvent( Event * );
    /** Write event settings to event object */
    void writeEvent( Event * );

    /** Check if the input is valid. */
    bool validateInput();

  public slots:
    void setEnabled( bool );
    void setDateTimes( QDateTime start, QDateTime end );
    void setDateTimeStr( const QString & );
  
  signals:
    void dateTimesChanged( QDateTime start, QDateTime end );
  
  protected slots:
    void showCurrentRule();
    void showCurrentRange();

  private:
    QCheckBox *mEnabledCheck;
  
    QGroupBox *mTimeGroupBox;
    QLabel *mDateTimeLabel;
  
    QGroupBox *mRuleBox;
    QWidgetStack *mRuleStack;
    
    QRadioButton *mDailyButton;
    QRadioButton *mWeeklyButton;
    QRadioButton *mMonthlyButton;
    QRadioButton *mYearlyButton;
    
    RecurDaily *mDaily;
    RecurWeekly *mWeekly;
    RecurMonthly *mMonthly;
    RecurYearly *mYearly;
    
    QGroupBox *mRangeGroupBox;
    QLabel *mStartDateLabel;
    QRadioButton *mNoEndDateButton;
    QRadioButton *mEndDurationButton;
    QSpinBox *mEndDurationEdit;
    QRadioButton *mEndDateButton;
    KDateEdit *mEndDateEdit;
  
    ExceptionsWidget *mExceptions;
};

#endif
