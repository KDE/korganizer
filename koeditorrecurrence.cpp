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

#include <qtooltip.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>
#include <qlistbox.h>
#include <qspinbox.h>
#include <qcheckbox.h>

#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <knumvalidator.h>
#include <kcalendarsystem.h>

#include <libkcal/event.h>

#include <libkdepim/kdateedit.h>

#include "koprefs.h"
#include "koglobals.h"

#include "koeditorrecurrence.h"
#include "koeditorrecurrence.moc"

/////////////////////////// RecurBase ///////////////////////////////

RecurBase::RecurBase( QWidget *parent, const char *name ) :
  QWidget( parent, name )
{
  mFrequencyEdit = new QSpinBox( 1, 9999, 1, this );
  mFrequencyEdit->setValue( 1 );
}

QWidget *RecurBase::frequencyEdit()
{
  return mFrequencyEdit;
}

void RecurBase::setFrequency( int f )
{
  if ( f < 1 ) f = 1;

  mFrequencyEdit->setValue( f );
}

int RecurBase::frequency()
{
  return mFrequencyEdit->value();
}

/////////////////////////// RecurDaily ///////////////////////////////

RecurDaily::RecurDaily( QWidget *parent, const char *name ) :
  RecurBase( parent, name )
{
  QBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  QLabel *preLabel = new QLabel( i18n("&Recur every"), this );
  topLayout->addWidget( preLabel );

  topLayout->addWidget( frequencyEdit() );
  preLabel->setBuddy( frequencyEdit() );

  QLabel *postLabel = new QLabel( i18n("day(s)"), this );
  topLayout->addWidget( postLabel );
}


/////////////////////////// RecurWeekly ///////////////////////////////

RecurWeekly::RecurWeekly( QWidget *parent, const char *name ) :
  RecurBase( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  topLayout->addStretch( 1 );

  QBoxLayout *weeksLayout = new QHBoxLayout( topLayout );

  QLabel *preLabel = new QLabel( i18n("&Recur every"), this );
  weeksLayout->addWidget( preLabel );

  weeksLayout->addWidget( frequencyEdit() );
  preLabel->setBuddy( frequencyEdit() );

  QLabel *postLabel = new QLabel( i18n("week(s) on:"), this );
  weeksLayout->addWidget( postLabel );

  QHBox *dayBox = new QHBox( this );
  topLayout->addWidget( dayBox, 1, AlignVCenter );
  // Respect start of week setting
  int weekStart=KGlobal::locale()->weekStartDay();
  for ( int i = 0; i < 7; ++i ) {
    // i is the nr of the combobox, not the day of week!
    // label=(i+weekStart+6)%7 + 1;
    // index in CheckBox array(=day): label-1
    const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
    QString weekDayName = calSys->weekDayName(
      (i + weekStart + 6)%7 + 1, true );
    if ( KOPrefs::instance()->mCompactDialogs ) {
      weekDayName = weekDayName.left( 1 );
    }
    mDayBoxes[ (i + weekStart + 6)%7 ] = new QCheckBox( weekDayName, dayBox );
  }

  topLayout->addStretch( 1 );
}

void RecurWeekly::setDays( const QBitArray &days )
{
  for ( int i = 0; i < 7; ++i ) {
    mDayBoxes[ i ]->setChecked( days.testBit( i ) );
  }
}

QBitArray RecurWeekly::days()
{
  QBitArray days( 7 );

  for ( int i = 0; i < 7; ++i ) {
    days.setBit( i, mDayBoxes[ i ]->isChecked() );
  }

  return days;
}

/////////////////////////// RecurMonthly ///////////////////////////////

RecurMonthly::RecurMonthly( QWidget *parent, const char *name ) :
  RecurBase( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );


  QBoxLayout *freqLayout = new QHBoxLayout( topLayout );

  QLabel *preLabel = new QLabel( i18n("every"), this );
  freqLayout->addWidget( preLabel );

  freqLayout->addWidget( frequencyEdit() );
  preLabel->setBuddy( frequencyEdit() );

  QLabel *postLabel = new QLabel( i18n("month(s)"), this );
  freqLayout->addWidget( postLabel );


  QButtonGroup *buttonGroup = new QButtonGroup( this );
  buttonGroup->setFrameStyle( QFrame::NoFrame );
  topLayout->addWidget( buttonGroup, 1, AlignVCenter );

  QGridLayout *buttonLayout = new QGridLayout( buttonGroup, 3, 2 );
  buttonLayout->setSpacing( KDialog::spacingHint() );


  QString recurOnText;
  if ( !KOPrefs::instance()->mCompactDialogs ) {
    recurOnText = i18n("&Recur on the");
  }

  mByDayRadio = new QRadioButton( recurOnText, buttonGroup );
  buttonLayout->addWidget( mByDayRadio, 0, 0 );

  mByDayCombo = new QComboBox( buttonGroup );
  mByDayCombo->setSizeLimit( 7 );
  mByDayCombo->insertItem( i18n("1st") );
  mByDayCombo->insertItem( i18n("2nd") );
  mByDayCombo->insertItem( i18n("3rd") );
  mByDayCombo->insertItem( i18n("4th") );
  mByDayCombo->insertItem( i18n("5th") );
  mByDayCombo->insertItem( i18n("6th") );
  mByDayCombo->insertItem( i18n("7th") );
  mByDayCombo->insertItem( i18n("8th") );
  mByDayCombo->insertItem( i18n("9th") );
  mByDayCombo->insertItem( i18n("10th") );
  mByDayCombo->insertItem( i18n("11th") );
  mByDayCombo->insertItem( i18n("12th") );
  mByDayCombo->insertItem( i18n("13th") );
  mByDayCombo->insertItem( i18n("14th") );
  mByDayCombo->insertItem( i18n("15th") );
  mByDayCombo->insertItem( i18n("16th") );
  mByDayCombo->insertItem( i18n("17th") );
  mByDayCombo->insertItem( i18n("18th") );
  mByDayCombo->insertItem( i18n("19th") );
  mByDayCombo->insertItem( i18n("20th") );
  mByDayCombo->insertItem( i18n("21st") );
  mByDayCombo->insertItem( i18n("22nd") );
  mByDayCombo->insertItem( i18n("23rd") );
  mByDayCombo->insertItem( i18n("24th") );
  mByDayCombo->insertItem( i18n("25th") );
  mByDayCombo->insertItem( i18n("26th") );
  mByDayCombo->insertItem( i18n("27th") );
  mByDayCombo->insertItem( i18n("28th") );
  mByDayCombo->insertItem( i18n("29th") );
  mByDayCombo->insertItem( i18n("30th") );
  mByDayCombo->insertItem( i18n("31st") );
  buttonLayout->addWidget( mByDayCombo, 0, 1 );

  QLabel *byDayLabel = new QLabel( i18n("day"), buttonGroup );
  buttonLayout->addWidget( byDayLabel, 0, 2 );


  mByPosRadio = new QRadioButton( recurOnText, buttonGroup);
  buttonLayout->addWidget( mByPosRadio, 1, 0 );

  mByPosCountCombo = new QComboBox( buttonGroup );
  mByPosCountCombo->insertItem( i18n("1st") );
  mByPosCountCombo->insertItem( i18n("2nd") );
  mByPosCountCombo->insertItem( i18n("3rd") );
  mByPosCountCombo->insertItem( i18n("4th") );
  mByPosCountCombo->insertItem( i18n("5th") );
  mByPosCountCombo->insertItem( i18n("Last") );
  mByPosCountCombo->insertItem( i18n("2nd Last") );
  mByPosCountCombo->insertItem( i18n("3rd Last") );
  mByPosCountCombo->insertItem( i18n("4th Last") );
  mByPosCountCombo->insertItem( i18n("5th Last") );

  buttonLayout->addWidget( mByPosCountCombo, 1, 1 );

  mByPosWeekdayCombo = new QComboBox( buttonGroup );
  for( int i = 1; i <= 7; ++i ) {
    mByPosWeekdayCombo->insertItem( KGlobal::locale()->weekDayName( i ) );
  }
  buttonLayout->addWidget( mByPosWeekdayCombo, 1, 2 );
}

void RecurMonthly::setByDay( int day )
{
  mByDayRadio->setChecked( true );
  mByDayCombo->setCurrentItem( day-1 );
}

void RecurMonthly::setByPos( int count, int weekday )
{
  mByPosRadio->setChecked( true );
  if (count>0)
    mByPosCountCombo->setCurrentItem( count - 1 );
  else
    // negative weeks means counted from the end of month
    mByPosCountCombo->setCurrentItem( -count + 4 );
  mByPosWeekdayCombo->setCurrentItem( weekday );
}

bool RecurMonthly::byDay()
{
  return mByDayRadio->isChecked();
}

bool RecurMonthly::byPos()
{
  return mByPosRadio->isChecked();
}

int RecurMonthly::day()
{
  return mByDayCombo->currentItem() + 1;
}

int RecurMonthly::count()
{
  int pos=mByPosCountCombo->currentItem();
  if (pos<=4) // positive  count
    return pos+1;
  else
    return -pos+4;
}

int RecurMonthly::weekday()
{
  return mByPosWeekdayCombo->currentItem();
}

/////////////////////////// RecurYearly ///////////////////////////////

RecurYearly::RecurYearly( QWidget *parent, const char *name ) :
  RecurBase( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );


  QBoxLayout *freqLayout = new QHBoxLayout( topLayout );

  QLabel *preLabel = new QLabel( i18n("&every"), this );
  freqLayout->addWidget( preLabel );

  freqLayout->addWidget( frequencyEdit() );
  preLabel->setBuddy( frequencyEdit() );

  QLabel *postLabel = new QLabel( i18n("year(s)"), this );
  freqLayout->addWidget( postLabel );


  QButtonGroup *buttonGroup = new QButtonGroup( this );
  buttonGroup->setFrameStyle( QFrame::NoFrame );
  topLayout->addWidget( buttonGroup, 1, AlignVCenter );

  QGridLayout *buttonLayout = new QGridLayout( buttonGroup, 3, 2 );

  QString recurInMonthText;
  if ( !KOPrefs::instance()->mCompactDialogs ) {
    recurInMonthText = i18n("&Recur in the month of");
  }

  mByMonthRadio = new QRadioButton( recurInMonthText, buttonGroup);
  buttonLayout->addWidget( mByMonthRadio, 0, 0 );

  mByMonthCombo = new QComboBox( buttonGroup );
  for( int i = 1; i <= 12; ++i ) {
    mByMonthCombo->insertItem( KGlobal::locale()->monthName( i ) );
  }
  buttonLayout->addWidget( mByMonthCombo, 0, 1 );


  buttonLayout->setRowStretch( 1, 1 );

  QString recurOnDayText;
  if ( KOPrefs::instance()->mCompactDialogs ) {
    recurOnDayText = i18n("This day");
  } else {
    recurOnDayText = i18n("Recur on this &day");
  }

  mByDayRadio = new QRadioButton( recurOnDayText, buttonGroup);
  buttonLayout->addMultiCellWidget( mByDayRadio, 2, 2, 0, 1 );
}

void RecurYearly::setDateTimes( QDateTime start, QDateTime ) 
{
  QString recurOnDayText;
  if ( KOPrefs::instance()->mCompactDialogs ) {
    mByDayRadio->setText( i18n("This day") );
  } else {
    mByDayRadio->setText( i18n("Recur on &day %1 of the year").
        arg( start.date().dayOfYear() ) );
  }
//  mByMonthCombo->setCurrentItem( start.date().month() + 1 );  
  if ( !KOPrefs::instance()->mCompactDialogs ) {
    mByMonthRadio->setText( i18n("&Recur on day %1 of ").arg( start.date().day() ) );
  }
}

void RecurYearly::setByDay()
{
  mByDayRadio->setChecked( true );
}

void RecurYearly::setByMonth( int month )
{
  mByMonthRadio->setChecked( true );
  mByMonthCombo->setCurrentItem( month - 1 );
}

bool RecurYearly::byMonth()
{
  return mByMonthRadio->isChecked();
}

bool RecurYearly::byDay()
{
  return mByDayRadio->isChecked();
}

int RecurYearly::month()
{
  return mByMonthCombo->currentItem() + 1;
}

//////////////////////////// ExceptionsWidget //////////////////////////

ExceptionsWidget::ExceptionsWidget( QWidget *parent, const char *name ) :
  QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  QGroupBox *groupBox = new QGroupBox( 1, Horizontal, i18n("E&xceptions"),
                                       this );
  topLayout->addWidget( groupBox );

  QWidget *box = new QWidget( groupBox );

  QGridLayout *boxLayout = new QGridLayout( box );

  mExceptionDateEdit = new KDateEdit( box );
  mExceptionDateEdit->setDate( QDate::currentDate() );
  boxLayout->addWidget( mExceptionDateEdit, 0, 0 );

  QPushButton *addExceptionButton = new QPushButton( i18n("&Add"), box );
  boxLayout->addWidget( addExceptionButton, 1, 0 );
  QPushButton *changeExceptionButton = new QPushButton( i18n("&Change"), box );
  boxLayout->addWidget( changeExceptionButton, 2, 0 );
  QPushButton *deleteExceptionButton = new QPushButton( i18n("&Delete"), box );
  boxLayout->addWidget( deleteExceptionButton, 3, 0 );

  mExceptionList = new QListBox( box );
  boxLayout->addMultiCellWidget( mExceptionList, 0, 3, 1, 1 );

  boxLayout->setRowStretch( 4, 1 );
  boxLayout->setColStretch( 1, 3 );

  connect( addExceptionButton, SIGNAL( clicked() ),
           SLOT( addException() ) );
  connect( changeExceptionButton, SIGNAL( clicked() ),
           SLOT( changeException() ) );
  connect( deleteExceptionButton, SIGNAL( clicked() ),
           SLOT( deleteException() ) );
}

void ExceptionsWidget::addException()
{
  QDate date = mExceptionDateEdit->date();
  QString dateStr = KGlobal::locale()->formatDate( date );
  if( !mExceptionList->findItem( dateStr ) ) {
    mExceptionDates.append( date );
    mExceptionList->insertItem( dateStr );
  }
}

void ExceptionsWidget::changeException()
{
  int pos = mExceptionList->currentItem();
  if ( pos < 0 ) return;

  QDate date = mExceptionDateEdit->date();
  mExceptionDates[ pos ] = date;
  mExceptionList->changeItem( KGlobal::locale()->formatDate( date ), pos );
}

void ExceptionsWidget::deleteException()
{
  int pos = mExceptionList->currentItem();
  if ( pos < 0 ) return;

  mExceptionDates.remove( mExceptionDates.at( pos ) );
  mExceptionList->removeItem( pos );
}

void ExceptionsWidget::setDates( const DateList &dates )
{
  mExceptionList->clear();
  mExceptionDates.clear();
  DateList::ConstIterator dit;
  for ( dit = dates.begin(); dit != dates.end(); ++dit ) {
    mExceptionList->insertItem( KGlobal::locale()->formatDate(* dit ) );
    mExceptionDates.append( *dit );
  }
}

DateList ExceptionsWidget::dates()
{
  return mExceptionDates;
}

///////////////////////// ExceptionsDialog ///////////////////////////

ExceptionsDialog::ExceptionsDialog( QWidget *parent, const char *name ) :
  KDialogBase( parent, name, true, i18n("Edit Exceptions"), Ok|Cancel )
{
  mExceptions = new ExceptionsWidget( this );
  setMainWidget( mExceptions );
}

void ExceptionsDialog::setDates( const DateList &dates )
{
  mExceptions->setDates( dates );
}

DateList ExceptionsDialog::dates()
{
  return mExceptions->dates();
}

///////////////////////// RecurrenceRangeWidget ///////////////////////////

RecurrenceRangeWidget::RecurrenceRangeWidget( QWidget *parent,
                                              const char *name )
  : QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  mRangeGroupBox = new QGroupBox( 1, Horizontal, i18n("Recurrence Range"),
                                  this );
  topLayout->addWidget( mRangeGroupBox );

  QWidget *rangeBox = new QWidget( mRangeGroupBox );
  QVBoxLayout *rangeLayout = new QVBoxLayout( rangeBox );
  rangeLayout->setSpacing( KDialog::spacingHint() );

  mStartDateLabel = new QLabel( i18n("Begin on:"), rangeBox );
  rangeLayout->addWidget( mStartDateLabel );

  QButtonGroup *rangeButtonGroup = new QButtonGroup;

  mNoEndDateButton = new QRadioButton( i18n("&No ending date"), rangeBox );
  rangeButtonGroup->insert( mNoEndDateButton );
  rangeLayout->addWidget( mNoEndDateButton );

  QBoxLayout *durationLayout = new QHBoxLayout( rangeLayout );
  durationLayout->setSpacing( KDialog::spacingHint() );

  mEndDurationButton = new QRadioButton( i18n("End &after"), rangeBox );
  rangeButtonGroup->insert( mEndDurationButton );
  durationLayout->addWidget( mEndDurationButton );

  mEndDurationEdit = new QSpinBox( 1, 9999, 1, rangeBox );
  durationLayout->addWidget( mEndDurationEdit );

  QLabel *endDurationLabel = new QLabel( i18n("&occurrence(s)"), rangeBox );
  durationLayout ->addWidget( endDurationLabel );
  endDurationLabel->setBuddy( mEndDurationEdit );

  QBoxLayout *endDateLayout = new QHBoxLayout( rangeLayout );
  endDateLayout->setSpacing( KDialog::spacingHint() );

  mEndDateButton = new QRadioButton( i18n("End &by:"), rangeBox );
  rangeButtonGroup->insert( mEndDateButton );
  endDateLayout->addWidget( mEndDateButton );

  mEndDateEdit = new KDateEdit( rangeBox );
  endDateLayout->addWidget( mEndDateEdit );

  endDateLayout->addStretch( 1 );

  connect( mNoEndDateButton, SIGNAL( toggled( bool ) ),
           SLOT( showCurrentRange() ) );
  connect( mEndDurationButton, SIGNAL( toggled( bool ) ),
           SLOT( showCurrentRange() ) );
  connect( mEndDateButton, SIGNAL( toggled( bool ) ),
           SLOT( showCurrentRange() ) );
}

void RecurrenceRangeWidget::setDefaults( const QDateTime &from  )
{
  mNoEndDateButton->setChecked( true );

  setDateTimes( from );
}

void RecurrenceRangeWidget::setDuration( int duration )
{
  if ( duration == -1 ) {
    mNoEndDateButton->setChecked( true );
  } else if ( duration == 0 ) {
    mEndDateButton->setChecked( true );
  } else {
    mEndDurationButton->setChecked( true );
    mEndDurationEdit->setValue( duration );
  }
}

int RecurrenceRangeWidget::duration()
{
  if ( mNoEndDateButton->isChecked() ) {
    return -1;
  } else if ( mEndDurationButton->isChecked() ) {
    return mEndDurationEdit->value();
  } else {
    return 0;
  }
}

void RecurrenceRangeWidget::setEndDate( const QDate &date )
{
  mEndDateEdit->setDate( date );
}

QDate RecurrenceRangeWidget::endDate()
{
  return mEndDateEdit->date();
}

void RecurrenceRangeWidget::showCurrentRange()
{
  mEndDurationEdit->setEnabled( mEndDurationButton->isChecked() );
  mEndDateEdit->setEnabled( mEndDateButton->isChecked() );
}

void RecurrenceRangeWidget::setDateTimes( const QDateTime &start,
                                          const QDateTime & )
{
  mStartDateLabel->setText( i18n("Begins on: %1")
      .arg( KGlobal::locale()->formatDate( start.date() ) ) );
}

///////////////////////// RecurrenceRangeDialog ///////////////////////////

RecurrenceRangeDialog::RecurrenceRangeDialog( QWidget *parent,
                                              const char *name ) :
  KDialogBase( parent, name, true, i18n("Edit Recurrence Range"), Ok|Cancel )
{
  mRecurrenceRangeWidget = new RecurrenceRangeWidget( this );
  setMainWidget( mRecurrenceRangeWidget );
}

void RecurrenceRangeDialog::setDefaults( const QDateTime &from )
{
  mRecurrenceRangeWidget->setDefaults( from );
}

void RecurrenceRangeDialog::setDuration( int duration )
{
  mRecurrenceRangeWidget->setDuration( duration );
}

int RecurrenceRangeDialog::duration()
{
  return mRecurrenceRangeWidget->duration();
}

void RecurrenceRangeDialog::setEndDate( const QDate &date )
{
  mRecurrenceRangeWidget->setEndDate( date );
}

QDate RecurrenceRangeDialog::endDate()
{
  return mRecurrenceRangeWidget->endDate();
}

void RecurrenceRangeDialog::setDateTimes( const QDateTime &start,
                                          const QDateTime &end )
{
  mRecurrenceRangeWidget->setDateTimes( start, end );
}

//////////////////////////// RecurrenceChooser ////////////////////////

RecurrenceChooser::RecurrenceChooser( QWidget *parent, const char *name ) :
  QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  if ( KOPrefs::instance()->mCompactDialogs ) {
    mTypeCombo = new QComboBox( this );
    mTypeCombo->insertItem( i18n("Daily") );
    mTypeCombo->insertItem( i18n("Weekly") );
    mTypeCombo->insertItem( i18n("Monthly") );
    mTypeCombo->insertItem( i18n("Yearly") );

    topLayout->addWidget( mTypeCombo );

    connect( mTypeCombo, SIGNAL( activated( int ) ), SLOT( emitChoice() ) );
  } else {
    mTypeCombo = 0;

    QButtonGroup *ruleButtonGroup = new QButtonGroup( 1, Horizontal, this );
    ruleButtonGroup->setFrameStyle( QFrame::NoFrame );
    topLayout->addWidget( ruleButtonGroup );

    mDailyButton = new QRadioButton( i18n("&Daily"), ruleButtonGroup );
    mWeeklyButton = new QRadioButton( i18n("&Weekly"), ruleButtonGroup );
    mMonthlyButton = new QRadioButton( i18n("&Monthly"), ruleButtonGroup );
    mYearlyButton = new QRadioButton( i18n("&Yearly"), ruleButtonGroup );

    connect( mDailyButton, SIGNAL( toggled( bool ) ),
             SLOT( emitChoice() ) );
    connect( mWeeklyButton, SIGNAL( toggled( bool ) ),
             SLOT( emitChoice() ) );
    connect( mMonthlyButton, SIGNAL( toggled( bool ) ),
             SLOT( emitChoice() ) );
    connect( mYearlyButton, SIGNAL( toggled( bool ) ),
             SLOT( emitChoice() ) );
  }
}

int RecurrenceChooser::type()
{
  if ( mTypeCombo ) {
    return mTypeCombo->currentItem();
  } else {
    if ( mDailyButton->isChecked() ) return Daily;
    else if ( mWeeklyButton->isChecked() ) return Weekly;
    else if ( mMonthlyButton->isChecked() ) return Monthly;
    else return Yearly;
  }
}

void RecurrenceChooser::setType( int type )
{
  if ( mTypeCombo ) {
    mTypeCombo->setCurrentItem( type );
  } else {
    switch ( type ) {
      case Daily:
        mDailyButton->setChecked( true );
        break;
      case Weekly:
        mWeeklyButton->setChecked( true );
        break;
      case Monthly:
        mMonthlyButton->setChecked( true );
        break;
      case Yearly:
      default:
        mYearlyButton->setChecked( true );
        break;
    }
  }
}

void RecurrenceChooser::emitChoice()
{
  emit chosen ( type() );
}

/////////////////////////////// Main Widget /////////////////////////////

KOEditorRecurrence::KOEditorRecurrence( QWidget* parent, const char *name ) :
  QWidget( parent, name )
{
  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  mEnabledCheck = new QCheckBox( i18n("&Enable recurrence"), this );
  connect( mEnabledCheck, SIGNAL( toggled( bool ) ),
           SLOT( setEnabled( bool ) ) );
  topLayout->addMultiCellWidget( mEnabledCheck, 0, 0, 0, 1 );


  mTimeGroupBox = new QGroupBox( 1, Horizontal, i18n("Appointment Time "),
                                 this );
  topLayout->addMultiCellWidget( mTimeGroupBox, 1, 1 , 0 , 1 );

  if ( KOPrefs::instance()->mCompactDialogs ) {
    mTimeGroupBox->hide();
  }

//  QFrame *timeFrame = new QFrame( mTimeGroupBox );
//  QBoxLayout *layoutTimeFrame = new QHBoxLayout( timeFrame );
//  layoutTimeFrame->setSpacing( KDialog::spacingHint() );

  mDateTimeLabel = new QLabel( mTimeGroupBox );
//  mDateTimeLabel = new QLabel( timeFrame );
//  layoutTimeFrame->addWidget( mDateTimeLabel );

  Qt::Orientation orientation;
  if ( KOPrefs::instance()->mCompactDialogs ) orientation = Horizontal;
  else orientation = Vertical;

  mRuleBox = new QGroupBox( 1, orientation, i18n("Recurrence Rule"), this );
  if ( KOPrefs::instance()->mCompactDialogs ) {
    topLayout->addWidget( mRuleBox, 2, 0 );
  } else {
    topLayout->addMultiCellWidget( mRuleBox, 2, 2, 0, 1 );
  }

  mRecurrenceChooser = new RecurrenceChooser( mRuleBox );
  connect( mRecurrenceChooser, SIGNAL( chosen( int ) ),
           SLOT( showCurrentRule( int ) ) );

  if ( !KOPrefs::instance()->mCompactDialogs ) {
    QFrame *ruleSepFrame = new QFrame( mRuleBox );
    ruleSepFrame->setFrameStyle( QFrame::VLine | QFrame::Sunken );
  }

  mRuleStack = new QWidgetStack( mRuleBox );

  mDaily = new RecurDaily( mRuleStack );
  mRuleStack->addWidget( mDaily, 0 );

  mWeekly = new RecurWeekly( mRuleStack );
  mRuleStack->addWidget( mWeekly, 0 );

  mMonthly = new RecurMonthly( mRuleStack );
  mRuleStack->addWidget( mMonthly, 0 );

  mYearly = new RecurYearly( mRuleStack );
  mRuleStack->addWidget( mYearly, 0 );

  showCurrentRule( mRecurrenceChooser->type() );

  if ( KOPrefs::instance()->mCompactDialogs ) {
    mRecurrenceRangeWidget = 0;
    mRecurrenceRangeDialog = new RecurrenceRangeDialog( this );
    mRecurrenceRange = mRecurrenceRangeDialog;
    mRecurrenceRangeButton = new QPushButton( i18n("Recurrence Range..."),
                                              this );
    topLayout->addWidget( mRecurrenceRangeButton, 3, 0 );
    connect( mRecurrenceRangeButton, SIGNAL( clicked() ),
             SLOT( showRecurrenceRangeDialog() ) );

    mExceptionsWidget = 0;
    mExceptionsDialog = new ExceptionsDialog( this );
    mExceptions = mExceptionsDialog;
    mExceptionsButton = new QPushButton( i18n("Exceptions..."), this );
    topLayout->addWidget( mExceptionsButton, 4, 0 );
    connect( mExceptionsButton, SIGNAL( clicked() ),
             SLOT( showExceptionsDialog() ) );

  } else {
    mRecurrenceRangeWidget = new RecurrenceRangeWidget( this );
    mRecurrenceRangeDialog = 0;
    mRecurrenceRange = mRecurrenceRangeWidget;
    mRecurrenceRangeButton = 0;
    topLayout->addWidget( mRecurrenceRangeWidget, 3, 0 );

    mExceptionsWidget = new ExceptionsWidget( this );
    mExceptionsDialog = 0;
    mExceptions = mExceptionsWidget;
    mExceptionsButton = 0;
    topLayout->addWidget( mExceptionsWidget, 3, 1 );
  }
}

KOEditorRecurrence::~KOEditorRecurrence()
{
}

void KOEditorRecurrence::setEnabled( bool enabled )
{
//  kdDebug(5850) << "KOEditorRecurrence::setEnabled(): " << (enabled ? "on" : "off") << endl;

  mTimeGroupBox->setEnabled( enabled );
  mRuleBox->setEnabled( enabled );
  if ( mRecurrenceRangeWidget ) mRecurrenceRangeWidget->setEnabled( enabled );
  if ( mRecurrenceRangeButton ) mRecurrenceRangeButton->setEnabled( enabled );
  if ( mExceptionsWidget ) mExceptionsWidget->setEnabled( enabled );
  if ( mExceptionsButton ) mExceptionsButton->setEnabled( enabled );
}

void KOEditorRecurrence::showCurrentRule( int current )
{
  switch ( current ) {
    case Daily:
      mRuleStack->raiseWidget( mDaily );
      break;
    case Weekly:
      mRuleStack->raiseWidget( mWeekly );
      break;
    case Monthly:
      mRuleStack->raiseWidget( mMonthly );
      break;
    default:
    case Yearly:
      mRuleStack->raiseWidget( mYearly );
      break;
  }
}

void KOEditorRecurrence::setDateTimes( QDateTime start, QDateTime end )
{
//  kdDebug(5850) << "KOEditorRecurrence::setDateTimes" << endl;

  mRecurrenceRange->setDateTimes( start, end );
  mDaily->setDateTimes( start, end );
  mWeekly->setDateTimes( start, end );
  mMonthly->setDateTimes( start, end );
  mYearly->setDateTimes( start, end );
}

void KOEditorRecurrence::setDefaults( QDateTime from, QDateTime to, bool )
{
  setDateTimes( from, to );

  bool enabled = false;
  mEnabledCheck->setChecked( enabled );
  setEnabled( enabled );

  mRecurrenceRange->setDefaults( from );

  mRecurrenceChooser->setType( RecurrenceChooser::Weekly );
  showCurrentRule( mRecurrenceChooser->type() );

  mDaily->setFrequency( 1 );

  mWeekly->setFrequency( 1 );
  QBitArray days( 7 );
  days.fill( 0 );
  days.setBit( (from.date().dayOfWeek()+6) % 7 );
  mWeekly->setDays( days );

  mMonthly->setFrequency( 1 );
  mMonthly->setByPos( from.date().day()/7 + 1, from.date().dayOfWeek() );
  mMonthly->setByDay( from.date().day() );

  mYearly->setFrequency( 1 );
  mYearly->setByMonth( from.date().month() );
}

void KOEditorRecurrence::readEvent(Event *event)
{
  QBitArray rDays( 7 );
  QPtrList<Recurrence::rMonthPos> rmp;
  QPtrList<int> rmd;
  int day = 0;
  int count = 0;
  int month = 0;
  setDefaults( event->dtStart(), event->dtEnd(), event->doesFloat() );

  setDateTimes( event->dtStart(), event->dtEnd() );

  int recurs = event->doesRecur();
  int f = 0;
  Recurrence *r = 0;

  if ( recurs )
  {
    r = event->recurrence();
    f = r->frequency();
  }


  mEnabledCheck->setChecked( recurs );
  setEnabled( recurs );

  int recurrenceType = RecurrenceChooser::Weekly;

  switch ( recurs ) {
    case Recurrence::rNone:
      break;
    case Recurrence::rDaily:
      recurrenceType = RecurrenceChooser::Daily;
      mDaily->setFrequency( f );
      break;
    case Recurrence::rWeekly:
      recurrenceType = RecurrenceChooser::Weekly;
      mWeekly->setFrequency( f );
      mWeekly->setDays( r->days() );
      break;
    case Recurrence::rMonthlyPos:
      // we only handle one possibility in the list right now,
      // so I have hardcoded calls with first().  If we make the GUI
      // more extended, this can be changed.
      recurrenceType = RecurrenceChooser::Monthly;

      rmp = r->monthPositions();
      if ( rmp.first()->negative )
        count=-rmp.first()->rPos;
      else
        // give the week as -5 to -1 and 1 to 5. the widget will do the rest
        count = rmp.first()->rPos;
      day = 0;
      while ( !rmp.first()->rDays.testBit( day ) ) ++day;
      mMonthly->setByPos( count, day );

      mMonthly->setFrequency( f );

      break;
    case Recurrence::rMonthlyDay:
      recurrenceType = RecurrenceChooser::Monthly;

      rmd = r->monthDays();
      day = *rmd.first();
      mMonthly->setByDay( day );

      mMonthly->setFrequency( f );

      break;
    case Recurrence::rYearlyMonth:
    case Recurrence::rYearlyDay:
      recurrenceType = RecurrenceChooser::Yearly;

      rmd = r->yearNums();
      month = *rmd.first();
      if ( month == event->dtStart().date().month() ) {
        mYearly->setByDay();
      } else {
        mYearly->setByMonth( month );
      }

      mYearly->setFrequency( f );
      break;
    default:
      break;
  }

  mRecurrenceChooser->setType( recurrenceType );
  showCurrentRule( recurrenceType );

  mRecurrenceRange->setDateTimes( event->dtStart() );

  if ( event->doesRecur() ) {
    mRecurrenceRange->setDuration( r->duration() );
    if ( r->duration() == 0 ) mRecurrenceRange->setEndDate( r->endDate() );
  }

  mExceptions->setDates( event->exDates() );
}

void KOEditorRecurrence::writeEvent( Event *event )
{
  if ( !mEnabledCheck->isChecked() )
  {
    if (event->doesRecur())
        event->recurrence()->unsetRecurs();
    return;
  }

  Recurrence *r = event->recurrence();

  // clear out any old settings;
  r->unsetRecurs();

  int duration = mRecurrenceRange->duration();
  QDate endDate;
  if ( duration == 0 ) endDate = mRecurrenceRange->endDate();

  int recurrenceType = mRecurrenceChooser->type();

  if ( recurrenceType == RecurrenceChooser::Daily ) {
      int freq = mDaily->frequency();
      if ( duration != 0 ) r->setDaily( freq, duration );
      else  r->setDaily( freq, endDate );
  } else if ( recurrenceType == RecurrenceChooser::Weekly ) {
      int freq = mWeekly->frequency();
      QBitArray days = mWeekly->days();
      if ( duration != 0 ) r->setWeekly( freq, days, duration );
      else r->setWeekly( freq, days, endDate );
  } else if ( recurrenceType == RecurrenceChooser::Monthly ) {
      int freq = mMonthly->frequency();
      if ( mMonthly->byPos() ) {
          int pos = mMonthly->count();

          QBitArray days( 7 );
          days.fill( false );

          days.setBit( mMonthly->weekday() );
          if ( duration != 0 )
              r->setMonthly( Recurrence::rMonthlyPos, freq, duration );
          else
              r->setMonthly( Recurrence::rMonthlyPos, freq, endDate );
          r->addMonthlyPos( pos, days );
      } else {
          // it's by day
          int day = mMonthly->day();

          if ( duration != 0 ) {
              r->setMonthly( Recurrence::rMonthlyDay, freq, duration );
          } else {
              r->setMonthly( Recurrence::rMonthlyDay, freq, endDate );
          }
          r->addMonthlyDay( day );
      }
  } else if ( recurrenceType == RecurrenceChooser::Yearly ) {
      int freq = mYearly->frequency();

      int month;
      if ( mYearly->byMonth() ) {
          month = mYearly->month();
      } else {
          month = event->dtStart().date().month();
      }
      if ( duration != 0 ) {
        r->setYearly( Recurrence::rYearlyMonth, freq, duration );
      } else {
        r->setYearly( Recurrence::rYearlyMonth, freq, endDate );
      }

      r->addYearlyNum( month );
    }

    event->setExDates( mExceptions->dates() );
}

void KOEditorRecurrence::setDateTimeStr( const QString &str )
{
  mDateTimeLabel->setText( str );
}

bool KOEditorRecurrence::validateInput()
{
  // Check input here

  return true;
}

void KOEditorRecurrence::showExceptionsDialog()
{
  DateList dates = mExceptions->dates();
  int result = mExceptionsDialog->exec();
  if ( result == QDialog::Rejected ) mExceptions->setDates( dates );
}

void KOEditorRecurrence::showRecurrenceRangeDialog()
{
  int duration = mRecurrenceRange->duration();
  QDate endDate = mRecurrenceRange->endDate();

  int result = mRecurrenceRangeDialog->exec();
  if ( result == QDialog::Rejected ) {
    mRecurrenceRange->setDuration( duration );
    mRecurrenceRange->setEndDate( endDate );
  }
}
