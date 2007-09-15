/*
  This file is part of KOrganizer.
  Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
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
#ifndef _KOEDITORRECURRENCE_H
#define _KOEDITORRECURRENCE_H

#include <kcal/incidencebase.h>

#include <kdialog.h>
#include <KComboBox>

#include <QDateTime>
#include <QWidget>
#include <QBitArray>
#include <QBoxLayout>
#include <QLabel>

class QCheckBox;
class QListWidget;
class QGroupBox;
class QRadioButton;
class QSpinBox;
class QStackedWidget;

namespace KPIM {
class KDateEdit;
}
namespace KCal {
class Incidence;
}
using namespace KCal;

class RecurBase : public QWidget
{
  public:
    explicit RecurBase( QWidget *parent = 0 );

    void setFrequency( int );
    int frequency();
    // FIXME: If we want to adjust the recurrence when the start/due date change,
    // we need to reimplement this method in the derived classes!
    void setDateTimes( const QDateTime &/*start*/, const QDateTime &/*end*/ ) {}

    QWidget *frequencyEdit();

  protected:
    static KComboBox *createWeekCountCombo( QWidget *parent=0 );
    static KComboBox *createWeekdayCombo( QWidget *parent=0 );
    static KComboBox *createMonthNameCombo( QWidget *parent=0 );
    QBoxLayout *createFrequencySpinBar( QWidget *parent, QLayout *layout,
    const QString &everyText, const QString &unitText );

  private:
    QSpinBox *mFrequencyEdit;
};

class RecurDaily : public RecurBase
{
  public:
    explicit RecurDaily( QWidget *parent = 0 );
};

class RecurWeekly : public RecurBase
{
  public:
    explicit RecurWeekly( QWidget *parent = 0 );

    void setDays( const QBitArray & );
    QBitArray days();

  private:
    QCheckBox *mDayBoxes[7];
};

class RecurMonthly : public RecurBase
{
  public:
    explicit RecurMonthly( QWidget *parent = 0 );

    void setByDay( int day );
    void setByPos( int count, int weekday );

    bool byDay();
    bool byPos();

    int day();

    int count();
    int weekday();

  private:
    QRadioButton *mByDayRadio;
    KComboBox *mByDayCombo;

    QRadioButton *mByPosRadio;
    KComboBox *mByPosCountCombo;
    KComboBox *mByPosWeekdayCombo;
};

class RecurYearly : public RecurBase
{
  public:
    enum YearlyType { byDay, byPos, byMonth };

    explicit RecurYearly( QWidget *parent = 0 );

    void setByDay( int day );
    void setByPos( int count, int weekday, int month );
    void setByMonth( int day, int month );

    YearlyType getType();

    int day();
    int posCount();
    int posWeekday();
    int posMonth();
    int monthDay();
    int month();

  private:
    QRadioButton *mByMonthRadio;
    QRadioButton *mByPosRadio;
    QRadioButton *mByDayRadio;

    QSpinBox *mByMonthSpin;
    KComboBox *mByMonthCombo;

    KComboBox *mByPosDayCombo;
    KComboBox *mByPosWeekdayCombo;
    KComboBox *mByPosMonthCombo;

    QSpinBox *mByDaySpin;
};

class RecurrenceChooser : public QWidget
{
  Q_OBJECT
  public:
    explicit RecurrenceChooser( QWidget *parent = 0 );

    enum { Daily, Weekly, Monthly, Yearly };

    void setType( int );
    int type();

  signals:
    void chosen( int );

  protected slots:
    void emitChoice();

  private:
    KComboBox *mTypeCombo;

    QRadioButton *mDailyButton;
    QRadioButton *mWeeklyButton;
    QRadioButton *mMonthlyButton;
    QRadioButton *mYearlyButton;
};

class ExceptionsBase
{
  public:
	virtual ~ExceptionsBase(){}
    virtual void setDates( const DateList & ) = 0;
    virtual DateList dates() = 0;
};

class ExceptionsWidget : public QWidget, public ExceptionsBase
{
  Q_OBJECT
  public:
    explicit ExceptionsWidget( QWidget *parent = 0 );

    void setDates( const DateList & );
    DateList dates();

  protected slots:
    void addException();
    void changeException();
    void deleteException();

  private:
    KPIM::KDateEdit *mExceptionDateEdit;
    QListWidget *mExceptionList;
    DateList mExceptionDates;
};

class ExceptionsDialog : public KDialog, public ExceptionsBase
{
  public:
    explicit ExceptionsDialog( QWidget *parent );

    void setDates( const DateList & );
    DateList dates();

  private:
    ExceptionsWidget *mExceptions;
};

class RecurrenceRangeBase
{
  public:
	virtual ~RecurrenceRangeBase() {}
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
    explicit RecurrenceRangeWidget( QWidget *parent = 0 );

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
    KPIM::KDateEdit *mEndDateEdit;
};

class RecurrenceRangeDialog : public KDialog, public RecurrenceRangeBase
{
  public:
    explicit RecurrenceRangeDialog( QWidget *parent = 0 );

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
    explicit KOEditorRecurrence ( QWidget *parent = 0 );
    virtual ~KOEditorRecurrence();

    enum { Daily, Weekly, Monthly, Yearly };

    /** Set widgets to default values */
    void setDefaults( const QDateTime &from, const QDateTime &to, bool allday );
    /** Read event object and setup widgets accordingly */
    void readIncidence( Incidence * );
    /** Write event settings to event object */
    void writeIncidence( Incidence * );

    /** Check if the input is valid. */
    bool validateInput();

    bool recurs();

  public slots:
    void setRecurrenceEnabled( bool );
    void setDateTimes( const QDateTime &start, const QDateTime &end );
    void setDateTimeStr( const QString & );

  signals:
    void dateTimesChanged( const QDateTime &start, const QDateTime &end );

  protected slots:
    void showCurrentRule( int );
    void showExceptionsDialog();
    void showRecurrenceRangeDialog();

  private:
    QCheckBox *mEnabledCheck;

    QGroupBox *mTimeGroupBox;
    QLabel *mDateTimeLabel;

    QGroupBox *mRuleBox;
    QStackedWidget *mRuleStack;
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
