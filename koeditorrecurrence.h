/*
    This file is part of KOrganizer.
    Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <kdialogbase.h>

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
    void setDateTimes( QDateTime /*start*/, QDateTime /*end*/ ) {}

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
    QCheckBox *mDayBoxes[7];
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
    void setDateTimes( QDateTime start, QDateTime end );
    
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

class RecurrenceChooser : public QWidget
{
    Q_OBJECT
  public:
    RecurrenceChooser( QWidget *parent = 0, const char *name = 0 );
    
    enum { Daily, Weekly, Monthly, Yearly };
    
    void setType( int );
    int type();
    
  signals:
    void chosen( int );

  protected slots:
    void emitChoice();
    
  private:
    QComboBox *mTypeCombo;
    
    QRadioButton *mDailyButton;
    QRadioButton *mWeeklyButton;
    QRadioButton *mMonthlyButton;
    QRadioButton *mYearlyButton;    
};

class ExceptionsBase
{
  public:
    virtual void setDates( const DateList & ) = 0;
    virtual DateList dates() = 0;
};

class ExceptionsWidget : public QWidget, public ExceptionsBase
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

class ExceptionsDialog : public KDialogBase, public ExceptionsBase
{
  public:
    ExceptionsDialog( QWidget *parent, const char *name = 0 );

    void setDates( const DateList & );
    DateList dates();

  private:
    ExceptionsWidget *mExceptions;
};

class RecurrenceRangeBase
{
  public:
    virtual void setDefaults( const QDateTime &from ) = 0;
  
    virtual void setDuration( int ) = 0;
    virtual int duration() = 0;

    virtual void setEndDate( const QDate & ) = 0;
    virtual QDate endDate() = 0;

    virtual void setDateTimes( const QDateTime &start,
                               const QDateTime &end = QDateTime() ) = 0;
};

class RecurrenceRangeWidget : public QWidget, public RecurrenceRangeBase
{
    Q_OBJECT
  public:
    RecurrenceRangeWidget( QWidget *parent = 0, const char *name = 0 );

    void setDefaults( const QDateTime &from );

    void setDuration( int );
    int duration();

    void setEndDate( const QDate & );
    QDate endDate();

    void setDateTimes( const QDateTime &start,
                       const QDateTime &end = QDateTime() );

  protected slots:
    void showCurrentRange();

  private:
    QGroupBox *mRangeGroupBox;
    QLabel *mStartDateLabel;
    QRadioButton *mNoEndDateButton;
    QRadioButton *mEndDurationButton;
    QSpinBox *mEndDurationEdit;
    QRadioButton *mEndDateButton;
    KDateEdit *mEndDateEdit;  
};

class RecurrenceRangeDialog : public KDialogBase, public RecurrenceRangeBase
{
  public:
    RecurrenceRangeDialog( QWidget *parent = 0, const char *name = 0 );

    void setDefaults( const QDateTime &from );

    void setDuration( int );
    int duration();

    void setEndDate( const QDate & );
    QDate endDate();

    void setDateTimes( const QDateTime &start,
                       const QDateTime &end = QDateTime() );
    
  private:
    RecurrenceRangeWidget *mRecurrenceRangeWidget;
};

class KOEditorRecurrence : public QWidget
{
    Q_OBJECT
  public:
    KOEditorRecurrence ( QWidget *parent = 0, const char *name = 0 );
    virtual ~KOEditorRecurrence();

    enum { Daily, Weekly, Monthly, Yearly };

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
    void showCurrentRule( int );
    void showExceptionsDialog();
    void showRecurrenceRangeDialog();
    
  private:
    QCheckBox *mEnabledCheck;
  
    QGroupBox *mTimeGroupBox;
    QLabel *mDateTimeLabel;
  
    QGroupBox *mRuleBox;
    QWidgetStack *mRuleStack;
    RecurrenceChooser *mRecurrenceChooser;
    
    RecurDaily *mDaily;
    RecurWeekly *mWeekly;
    RecurMonthly *mMonthly;
    RecurYearly *mYearly;

    RecurrenceRangeBase *mRecurrenceRange;
    RecurrenceRangeWidget *mRecurrenceRangeWidget;
    RecurrenceRangeDialog *mRecurrenceRangeDialog;
    QPushButton *mRecurrenceRangeButton;
    
    ExceptionsBase *mExceptions;
    ExceptionsDialog *mExceptionsDialog;
    ExceptionsWidget *mExceptionsWidget;
    QPushButton *mExceptionsButton;
		
		QDateTime mEventStartDt;
};

#endif
