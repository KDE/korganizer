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

#include "koeditorrecurrence.h"
#include "koprefs.h"
#include "koglobals.h"

#include <libkdepim/kdateedit.h>

#include <kcal/todo.h>

#include <KComboBox>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <knumvalidator.h>
#include <kcalendarsystem.h>
#include <kmessagebox.h>
#include <kvbox.h>

#include <QLayout>
#include <QGroupBox>
#include <QStackedWidget>
#include <QDateTime>
#include <QListWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>

#include "koeditorrecurrence.moc"

/////////////////////////// RecurBase ///////////////////////////////

RecurBase::RecurBase( QWidget *parent ) : QWidget( parent )
{
  mFrequencyEdit = new QSpinBox( this );
  mFrequencyEdit->setRange( 1, 9999 );
  mFrequencyEdit->setValue( 1 );
}

QWidget *RecurBase::frequencyEdit()
{
  return mFrequencyEdit;
}

void RecurBase::setFrequency( int f )
{
  if ( f < 1 ) {
    f = 1;
  }

  mFrequencyEdit->setValue( f );
}

int RecurBase::frequency()
{
  return mFrequencyEdit->value();
}

KComboBox *RecurBase::createWeekCountCombo( QWidget *parent )
{
  KComboBox *combo = new KComboBox( parent );
  if ( !combo ) {
    return 0;
  }

  combo->setWhatsThis( i18nc( "@info:whatsthis",
                              "The number of the week from the beginning "
                              "of the month on which this event or to-do "
                              "should recur." ) );
  combo->addItem( i18nc( "@item:inlistbox", "1st" ) );
  combo->addItem( i18nc( "@item:inlistbox", "2nd" ) );
  combo->addItem( i18nc( "@item:inlistbox", "3rd" ) );
  combo->addItem( i18nc( "@item:inlistbox", "4th" ) );
  combo->addItem( i18nc( "@item:inlistbox", "5th" ) );
  combo->addItem( i18nc( "@item:inlistbox last week of the month", "Last" ) );
  combo->addItem( i18nc( "@item:inlistbox", "2nd Last" ) );
  combo->addItem( i18nc( "@item:inlistbox", "3rd Last" ) );
  combo->addItem( i18nc( "@item:inlistbox", "4th Last" ) );
  combo->addItem( i18nc( "@item:inlistbox", "5th Last" ) );
  return combo;
}

KComboBox *RecurBase::createWeekdayCombo( QWidget *parent )
{
  KComboBox *combo = new KComboBox( parent );
  if ( !combo ) {
    return 0;
  }

  combo->setWhatsThis( i18nc( "@info:whatsthis",
                              "The weekday on which this event or to-do "
                              "should recur." ) );
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
  for ( int i=1; i <= 7; ++i ) {
    combo->addItem( calSys->weekDayName( i ) );
  }
  return combo;
}

KComboBox *RecurBase::createMonthNameCombo( QWidget *parent )
{
  KComboBox *combo = new KComboBox( parent );
  combo->setWhatsThis( i18nc( "@info:whatsthis",
                              "The month during which this event or to-do "
                              "should recur." ) );
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
  for ( int i=1; i <= 12; ++i ) {
    // use an arbitrary year, we just need the month name...
    QDate dt( 2005, i, 1 );
    combo->addItem( calSys->monthName( dt ) );
  }
  return combo;
}

QBoxLayout *RecurBase::createFrequencySpinBar( QWidget *parent, QLayout *layout,
                                               const QString &everyText,
                                               const QString &unitText )
{
  QBoxLayout *freqLayout = new QHBoxLayout();
  freqLayout->setSpacing( layout->spacing() );
  layout->addItem( freqLayout );

  QString whatsThis = i18nc( "@info:whatsthis",
                             "Sets how often this event or to-do should recur." );
  QLabel *preLabel = new QLabel( everyText, parent );
  preLabel->setWhatsThis( whatsThis );
  freqLayout->addWidget( preLabel );

  freqLayout->addWidget( frequencyEdit() );
  preLabel->setBuddy( frequencyEdit() );
  preLabel->buddy()->setWhatsThis( whatsThis );

  QLabel *postLabel = new QLabel( unitText, parent );
  postLabel->setWhatsThis( whatsThis );
  freqLayout->addWidget( postLabel );
  freqLayout->addStretch();
  return freqLayout;
}

/////////////////////////// RecurDaily ///////////////////////////////

RecurDaily::RecurDaily( QWidget *parent ) : RecurBase( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  createFrequencySpinBar( this, topLayout,
                          i18nc( "@label", "&Recur every" ),
                          i18nc( "@label recurrence expressed in days", "day(s)" ) );
}

/////////////////////////// RecurWeekly ///////////////////////////////

RecurWeekly::RecurWeekly( QWidget *parent ) : RecurBase( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

//  topLayout->addStretch( 1 );

  createFrequencySpinBar( this, topLayout,
                          i18nc( "@label", "&Recur every" ), i18nc( "@label", "week(s) on:" ) );

  KHBox *dayBox = new KHBox( this );
  topLayout->addWidget( dayBox, 1, Qt::AlignVCenter );
  // Respect start of week setting
  int weekStart=KGlobal::locale()->weekStartDay();
  for ( int i = 0; i < 7; ++i ) {
    // i is the nr of the combobox, not the day of week!
    // label=(i+weekStart+6)%7 + 1;
    // index in CheckBox array(=day): label-1
    const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
    QString weekDayName = calSys->weekDayName( ( i + weekStart + 6 ) % 7 + 1,
                                               KCalendarSystem::ShortDayName );
    QString longDayName = calSys->weekDayName( ( i + weekStart + 6 ) % 7 + 1,
                                               KCalendarSystem::LongDayName );
    if ( KOPrefs::instance()->mCompactDialogs ) {
      weekDayName = weekDayName.left( 1 );
    }
    mDayBoxes[ ( i + weekStart + 6 ) % 7 ] = new QCheckBox( weekDayName, dayBox );
    mDayBoxes[ ( i + weekStart + 6 ) % 7 ]->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Set %1 as the day when this event or to-do should recur.", longDayName ) );

    mDayBoxes[ ( i + weekStart + 6 ) % 7 ]->setToolTip(
      i18nc( "@info:tooltip", "Recur on %1", longDayName ) );
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

RecurMonthly::RecurMonthly( QWidget *parent ) : RecurBase( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  createFrequencySpinBar( this, topLayout,
                          i18nc( "@label", "&Recur every" ), i18nc( "@label", "month(s)" ) );

  QFrame *buttonGroup = new QFrame( this );
  topLayout->addWidget( buttonGroup, 1, Qt::AlignVCenter );

  QGridLayout *buttonLayout = new QGridLayout( buttonGroup );
  buttonLayout->setSpacing( KDialog::spacingHint() );

  QString recurOnText;
  if ( !KOPrefs::instance()->mCompactDialogs ) {
    recurOnText = i18nc( "@option:radio", "&Recur on the" );
  }

  mByDayRadio = new QRadioButton( recurOnText, buttonGroup );
  mByDayRadio->setWhatsThis( i18nc( "@info:whatsthis",
                                    "Sets a specific day of the month on which "
                                    "this event or to-do should recur." ) );

  buttonLayout->addWidget( mByDayRadio, 0, 0 );

  QString whatsThis = i18nc( "@info:whatsthis",
                             "The day of the month on which this event "
                             "or to-do should recur." );
  mByDayCombo = new KComboBox( buttonGroup );
  mByDayCombo->setWhatsThis( whatsThis );
  mByDayCombo->setMaxVisibleItems( 7 );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "1st" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "2nd" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "3rd" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "4th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "5th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "6th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "7th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "8th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "9th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "10th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "11th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "12th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "13th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "14th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "15th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "16th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "17th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "18th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "19th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "20th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "21st" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "22nd" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "23rd" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "24th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "25th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "26th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "27th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "28th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "29th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "30th" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "31st" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox last day of the month", "Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "2nd Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "3rd Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "4th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "5th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "6th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "7th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "8th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "9th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "10th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "11th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "12th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "13th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "14th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "15th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "16th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "17th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "18th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "19th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "20th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "21st Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "22nd Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "23rd Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "24th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "25th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "26th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "27th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "28th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "29th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "30th Last" ) );
  mByDayCombo->addItem( i18nc( "@item:inlistbox", "31st Last" ) );
  buttonLayout->addWidget( mByDayCombo, 0, 1 );

  QLabel *byDayLabel = new QLabel( i18nc( "@label", "day" ), buttonGroup );
  byDayLabel->setWhatsThis( whatsThis );
  buttonLayout->addWidget( byDayLabel, 0, 2 );

  mByPosRadio = new QRadioButton( recurOnText, buttonGroup );
  mByPosRadio->setWhatsThis( i18nc( "@info:whatsthis",
                                    "Sets a weekday and specific week in the month "
                                    "on which this event or to-do should recur" ) );
  buttonLayout->addWidget( mByPosRadio, 1, 0 );

  mByPosCountCombo = createWeekCountCombo( buttonGroup );
  buttonLayout->addWidget( mByPosCountCombo, 1, 1 );

  mByPosWeekdayCombo = createWeekdayCombo( buttonGroup );
  buttonLayout->addWidget( mByPosWeekdayCombo, 1, 2 );

  topLayout->addStretch( 1 );
}

void RecurMonthly::setByDay( int day )
{
  mByDayRadio->setChecked( true );
  // Days from the end are after the ones from the begin, so correct for the
  // negative sign and add 30 (index starting at 0)
  if ( day > 0 && day <= 31 ) {
    mByDayCombo->setCurrentIndex( day - 1 );
  } else if ( day < 0 ) {
    mByDayCombo->setCurrentIndex( 31 - 1 - day );
  }
}

void RecurMonthly::setByPos( int count, int weekday )
{
  mByPosRadio->setChecked( true );
  if ( count > 0 ) {
    mByPosCountCombo->setCurrentIndex( count - 1 );
  } else {
    // negative weeks means counted from the end of month
    mByPosCountCombo->setCurrentIndex( -count + 4 );
  }
  mByPosWeekdayCombo->setCurrentIndex( weekday - 1 );
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
  int day = mByDayCombo->currentIndex();
  if ( day >= 31 ) {
    day = 31-day-1;
  } else {
    ++day;
  }
  return day;
}

int RecurMonthly::count()
{
  int pos = mByPosCountCombo->currentIndex();
  if ( pos <= 4 ) { // positive count
    return pos + 1;
  } else {
    return -pos + 4;
  }
}

int RecurMonthly::weekday()
{
  return mByPosWeekdayCombo->currentIndex() + 1;
}

/////////////////////////// RecurYearly ///////////////////////////////

RecurYearly::RecurYearly( QWidget *parent ) : RecurBase( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  createFrequencySpinBar( this, topLayout,
                          i18nc( "@label", "&Recur every" ), i18nc( "@label", "year(s)" ) );

  QFrame *buttonGroup = new QFrame( this );
  topLayout->addWidget( buttonGroup, 1, Qt::AlignVCenter );

  QBoxLayout *buttonLayout = new QVBoxLayout( buttonGroup );

  /* YearlyMonth (day n of Month Y) */
  QBoxLayout *monthLayout = new QHBoxLayout();
  buttonLayout->addItem( monthLayout );
  QString recurInMonthText(
    i18nc( "@option:radio part before XXX of 'Recur on day XXX of month YYY'", "&Recur on day " ) );
  if ( KOPrefs::instance()->mCompactDialogs ) {
    recurInMonthText = i18nc( "@option:radio", "&Day " );
  }
  mByMonthRadio = new QRadioButton( recurInMonthText, buttonGroup );
  mByMonthRadio->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Sets a specific day in a specific month "
           "on which this event or to-do should recur." ) );
  monthLayout->addWidget( mByMonthRadio );
  mByMonthSpin = new QSpinBox( buttonGroup );
  mByMonthSpin->setRange( 1, 31 );
  mByMonthSpin->setWhatsThis(
    i18nc( "@info:whatsthis",
           "The day of the month on which this event "
           "or to-do should recur." ) );
  monthLayout->addWidget( mByMonthSpin );
  QLabel *ofLabel =
    new QLabel(
      i18nc( "@label part between XXX and YYY of 'Recur on day XXX of month YYY'", " &of " ),
      buttonGroup );
  //What do I do here? I'm not sure if this label should have What's This in it... - Antonio
  monthLayout->addWidget( ofLabel );

  mByMonthCombo = createMonthNameCombo( buttonGroup );
  monthLayout->addWidget( mByMonthCombo );
  ofLabel->setBuddy( mByMonthCombo );

  monthLayout->addStretch( 1 );

  /* YearlyPos (weekday X of week N of month Y) */
  QBoxLayout *posLayout = new QHBoxLayout();
  buttonLayout->addItem( posLayout );
  QString recurOnPosText(
    i18nc( "@option:radio Part before XXX in 'Recur on NNN. WEEKDAY of MONTH', short version",
           "&On" ) );
  if ( !KOPrefs::instance()->mCompactDialogs ) {
    recurOnPosText =
      i18nc( "@option:radio Part before XXX in 'Recur on NNN. WEEKDAY of MONTH'",
             "&On the" );
  }
  mByPosRadio = new QRadioButton( recurOnPosText, buttonGroup );
  mByPosRadio->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Sets a specific day in a specific week "
           "of a specific month on which this event "
           "or to-do should recur." ) );
  posLayout->addWidget( mByPosRadio );

  mByPosDayCombo = createWeekCountCombo( buttonGroup );
  posLayout->addWidget( mByPosDayCombo );

  mByPosWeekdayCombo = createWeekdayCombo( buttonGroup );
  posLayout->addWidget( mByPosWeekdayCombo );

  ofLabel =
    new QLabel(
      i18nc( "@label part between WEEKDAY and MONTH in 'Recur on NNN. WEEKDAY of MONTH'", " o&f " ),
      buttonGroup );
  posLayout->addWidget( ofLabel );

  mByPosMonthCombo  = createMonthNameCombo( buttonGroup );
  posLayout->addWidget( mByPosMonthCombo );
  ofLabel->setBuddy( mByPosMonthCombo );

  posLayout->addStretch( 1 );

  /* YearlyDay (day N of the year) */
  QBoxLayout *dayLayout = new QHBoxLayout();
  buttonLayout->addItem( dayLayout );
  QString recurOnDayText;
  if ( KOPrefs::instance()->mCompactDialogs ) {
    recurOnDayText = i18nc( "@option:radio", "Day #" );
  } else {
    recurOnDayText = i18nc( "@option:radio", "Recur on &day #" );
  }
  QString whatsThis = i18nc( "@info:whatsthis",
                             "Sets a specific day within the year on which "
                             "this event or to-do should recur." );
  mByDayRadio = new QRadioButton( recurOnDayText, buttonGroup );
  mByDayRadio->setWhatsThis( whatsThis );
  dayLayout->addWidget( mByDayRadio );

  mByDaySpin = new QSpinBox( buttonGroup );
  mByDaySpin->setRange( 1, 366 );
  mByDaySpin->setWhatsThis( whatsThis );

  dayLayout->addWidget( mByDaySpin );

  QString ofTheYear(
    i18nc( "@label part after NNN of 'Recur on day #NNN of the year'",
           " of the &year" ) );
  if ( KOPrefs::instance()->mCompactDialogs ) {
    ofTheYear =
      i18nc( "@label part after NNN of 'Recur on day #NNN of the year', short version",
             " of the year" );
  }
  ofLabel = new QLabel( ofTheYear, buttonGroup );
  ofLabel->setWhatsThis( whatsThis );
  dayLayout->addWidget( ofLabel );
  ofLabel->setBuddy( mByDaySpin );

  dayLayout->addStretch( 1 );
  topLayout->addStretch( 1 );
}

void RecurYearly::setByDay( int day )
{
  mByDayRadio->setChecked( true );
  mByDaySpin->setValue( day );
}

void RecurYearly::setByPos( int count, int weekday, int month )
{
  mByPosRadio->setChecked( true );
  if ( count > 0 ) {
    mByPosDayCombo->setCurrentIndex( count - 1 );
  } else {
    mByPosDayCombo->setCurrentIndex( -count + 4 );
  }
  mByPosWeekdayCombo->setCurrentIndex( weekday - 1 );
  mByPosMonthCombo->setCurrentIndex( month-1 );
}

void RecurYearly::setByMonth( int day, int month )
{
  mByMonthRadio->setChecked( true );
  mByMonthSpin->setValue( day );
  mByMonthCombo->setCurrentIndex( month - 1 );
}

RecurYearly::YearlyType RecurYearly::getType()
{
  if ( mByMonthRadio->isChecked() ) {
    return byMonth;
  }
  if ( mByPosRadio->isChecked() ) {
    return byPos;
  }
  if ( mByDayRadio->isChecked() ) {
    return byDay;
  }
  return byMonth;
}

int RecurYearly::monthDay()
{
  return mByMonthSpin->value();
}

int RecurYearly::month()
{
  return mByMonthCombo->currentIndex() + 1;
}

int RecurYearly::posCount()
{
  int pos = mByPosDayCombo->currentIndex();
  if ( pos <= 4 ) { // positive count
    return pos + 1;
  } else {
    return -pos + 4;
  }
}

int RecurYearly::posWeekday()
{
  return mByPosWeekdayCombo->currentIndex() + 1;
}

int RecurYearly::posMonth()
{
  return mByPosMonthCombo->currentIndex() + 1;
}

int RecurYearly::day()
{
  return mByDaySpin->value();
}

//////////////////////////// ExceptionsWidget //////////////////////////

ExceptionsWidget::ExceptionsWidget( QWidget *parent ) : QWidget( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( 0 );
  topLayout->setMargin( 0 );

  QGroupBox *groupBox = new QGroupBox( i18nc( "@title:group", "E&xceptions" ), this );
  topLayout->addWidget( groupBox );

  QGridLayout *boxLayout = new QGridLayout( groupBox );

  mExceptionDateEdit = new KPIM::KDateEdit( groupBox );
  mExceptionDateEdit->setWhatsThis(
    i18nc( "@info:whatsthis",
           "A date that should be considered an exception "
           "to the recurrence rules for this event or to-do." ) );
  mExceptionDateEdit->setDate( QDate::currentDate() );
  boxLayout->addWidget( mExceptionDateEdit, 0, 0 );

  QPushButton *addExceptionButton =
    new QPushButton( i18nc( "@action:button", "&Add" ), groupBox );
  addExceptionButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Add this date as an exception "
           "to the recurrence rules for this event or to-do." ) );
  boxLayout->addWidget( addExceptionButton, 1, 0 );

  QPushButton *changeExceptionButton =
    new QPushButton( i18nc( "@action:button", "&Change" ), groupBox );
  changeExceptionButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Replace the currently selected date with this date." ) );
  boxLayout->addWidget( changeExceptionButton, 2, 0 );

  QPushButton *deleteExceptionButton =
    new QPushButton( i18nc( "@action:button", "&Delete" ), groupBox );
  deleteExceptionButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Delete the currently selected date from the list of dates "
           "that should be considered exceptions to the recurrence rules "
           "for this event or to-do." ) );
  boxLayout->addWidget( deleteExceptionButton, 3, 0 );

  mExceptionList = new QListWidget( groupBox );
  mExceptionList->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Displays current dates that are being considered "
           "exceptions to the recurrence rules for this event "
           "or to-do." ) );
  boxLayout->addWidget( mExceptionList, 0, 1, 4, 1 );

  boxLayout->setRowStretch( 4, 1 );
  boxLayout->setColumnStretch( 1, 3 );

  connect( addExceptionButton, SIGNAL(clicked()), SLOT(addException()) );
  connect( changeExceptionButton, SIGNAL(clicked()), SLOT(changeException()) );
  connect( deleteExceptionButton, SIGNAL(clicked()), SLOT(deleteException()) );
}

void ExceptionsWidget::addException()
{
  QDate date = mExceptionDateEdit->date();
  QString dateStr = KGlobal::locale()->formatDate( date );
  if( mExceptionList->findItems( dateStr, Qt::MatchExactly ).isEmpty() ) {
    mExceptionDates.append( date );
    mExceptionList->addItem( dateStr );
  }
}

void ExceptionsWidget::changeException()
{
  int pos = mExceptionList->currentRow();
  if ( pos < 0 ) {
    return;
  }

  QDate date = mExceptionDateEdit->date();
  mExceptionDates[ pos ] = date;
  QListWidgetItem *item = mExceptionList->item( pos );
  item->setText( KGlobal::locale()->formatDate( date ) );
}

void ExceptionsWidget::deleteException()
{
  int pos = mExceptionList->currentRow();
  if ( pos < 0 ) {
    return;
  }

  mExceptionDates.removeAt( pos );
  delete( mExceptionList->takeItem( pos ) );
}

void ExceptionsWidget::setDates( const DateList &dates )
{
  mExceptionList->clear();
  mExceptionDates.clear();
  DateList::ConstIterator dit;
  for ( dit = dates.begin(); dit != dates.end(); ++dit ) {
    mExceptionList->addItem( KGlobal::locale()->formatDate(* dit ) );
    mExceptionDates.append( *dit );
  }
}

DateList ExceptionsWidget::dates()
{
  return mExceptionDates;
}

///////////////////////// ExceptionsDialog ///////////////////////////

ExceptionsDialog::ExceptionsDialog( QWidget *parent ) :
  KDialog( parent )
{
  setCaption( i18nc( "@title:window", "Edit Exceptions" ) );
  setButtons( Ok | Cancel );
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

RecurrenceRangeWidget::RecurrenceRangeWidget( QWidget *parent ) : QWidget( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( 0 );
  topLayout->setMargin( 0 );

  mRangeGroupBox = new QGroupBox( i18nc( "@title:group", "Recurrence Range" ), this );
  mRangeGroupBox->setWhatsThis( i18nc( "@info:whatsthis",
                                       "Sets a range for which these recurrence "
                                       "rules will apply to this event or to-do." ) );
  topLayout->addWidget( mRangeGroupBox );

  QVBoxLayout *rangeLayout = new QVBoxLayout( mRangeGroupBox );
  rangeLayout->setSpacing( KDialog::spacingHint() );

  mStartDateLabel = new QLabel( i18nc( "@label", "Begin on:" ), mRangeGroupBox );
  mStartDateLabel->setWhatsThis( i18nc( "@info:whatsthis",
                                        "The date on which the recurrences "
                                        "for this event or to-do should begin." ) );
  rangeLayout->addWidget( mStartDateLabel );

  QButtonGroup *rangeButtonGroup = new QButtonGroup( mRangeGroupBox );

  mNoEndDateButton = new QRadioButton(
    i18nc( "@option radio", "&No ending date" ), mRangeGroupBox );
  mNoEndDateButton->setWhatsThis(
    i18nc( "@info:whatsthis", "Sets the event or to-do to recur forever." ) );
  rangeButtonGroup->addButton( mNoEndDateButton );
  rangeLayout->addWidget( mNoEndDateButton );

  // The "After N occurrences" is a widget with a horizontal layout
  QBoxLayout *durationLayout = new QHBoxLayout();
  rangeLayout->addItem( durationLayout );
  durationLayout->setSpacing( KDialog::spacingHint() );

  QString whatsthis = i18nc(
    "@info:whatsthis",
    "Sets the event or to-do to stop recurring after "
    "a certain number of occurrences." );
  mEndDurationButton = new QRadioButton( i18nc( "@option:radio", "End &after" ), mRangeGroupBox );
  mEndDurationButton->setWhatsThis( whatsthis );
  rangeButtonGroup->addButton( mEndDurationButton );
  durationLayout->addWidget( mEndDurationButton );

  mEndDurationEdit = new QSpinBox( mRangeGroupBox );
  mEndDurationEdit->setRange( 1, 9999 );
  durationLayout->addWidget( mEndDurationEdit );

  QLabel *endDurationLabel = new QLabel( i18nc( "@label", "&occurrence(s)" ), mRangeGroupBox );
  durationLayout ->addWidget( endDurationLabel );
  endDurationLabel->setBuddy( mEndDurationEdit );
  durationLayout->addStretch(1);

  // The "End on" is a widget with a horizontal layout
  QBoxLayout *endDateLayout = new QHBoxLayout();
  rangeLayout->addItem( endDateLayout );
  whatsthis = i18nc( "@info:whatsthis",
                     "Sets the event or to-do to stop recurring on a certain date." );

  mEndDateButton = new QRadioButton( i18nc( "@option:radio", "End &on:" ), mRangeGroupBox );
  rangeButtonGroup->addButton( mEndDateButton );
  endDateLayout->addWidget( mEndDateButton );

  mEndDateEdit = new KPIM::KDateEdit( mRangeGroupBox );
  mEndDateEdit->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Date after which the event or to-do should stop recurring" ) );
  endDateLayout->addWidget( mEndDateEdit );
  endDateLayout->addStretch( 1 );

  rangeLayout->addStretch( 1 );

  connect( mNoEndDateButton, SIGNAL(toggled(bool)), SLOT(showCurrentRange()) );
  connect( mEndDurationButton, SIGNAL(toggled(bool)), SLOT(showCurrentRange()) );
  connect( mEndDateButton, SIGNAL(toggled(bool)), SLOT(showCurrentRange()) );
}

void RecurrenceRangeWidget::setDefaults( const QDateTime &from )
{
  mNoEndDateButton->setChecked( true );

  setDateTimes( from );
  setEndDate( from.date() );
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
  mStartDateLabel->setText(
    i18nc( "@label", "Begins on: %1", KGlobal::locale()->formatDate( start.date() ) ) );
}

///////////////////////// RecurrenceRangeDialog ///////////////////////////

RecurrenceRangeDialog::RecurrenceRangeDialog( QWidget *parent ) :
  KDialog( parent )
{
  setCaption( i18nc( "@title:window", "Edit Recurrence Range" ) );
  setButtons( Ok | Cancel );
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

RecurrenceChooser::RecurrenceChooser( QWidget *parent ) : QWidget( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  if ( KOPrefs::instance()->mCompactDialogs ) {
    mTypeCombo = new KComboBox( this );
    mTypeCombo->setWhatsThis( i18nc( "@info:whatsthis",
                                     "Sets the type of recurrence this event "
                                     "or to-do should have." ) );
    mTypeCombo->addItem( i18nc( "@item:inlistbox recur daily", "Daily" ) );
    mTypeCombo->addItem( i18nc( "@item:inlistbox recur weekly", "Weekly" ) );
    mTypeCombo->addItem( i18nc( "@item:inlistbox recur monthly", "Monthly" ) );
    mTypeCombo->addItem( i18nc( "@item:inlistbox recur yearly", "Yearly" ) );

    topLayout->addWidget( mTypeCombo );

    connect( mTypeCombo, SIGNAL(activated(int)), SLOT(emitChoice()) );
  } else {
    mTypeCombo = 0;

    QGroupBox *ruleButtonGroup = new QGroupBox( i18nc( "@title:group", "Recurrence Types" ), this );
    QBoxLayout *buttonLayout = new QVBoxLayout( ruleButtonGroup );

    ruleButtonGroup->setFlat( true );
    topLayout->addWidget( ruleButtonGroup );

    mDailyButton = new QRadioButton(
      i18nc( "@option:radio recur daily", "&Daily" ), ruleButtonGroup );
    mDailyButton->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Sets the event or to-do to recur daily "
             "according to the specified rules." ) );
    buttonLayout->addWidget( mDailyButton );
    mWeeklyButton = new QRadioButton(
      i18nc( "@option:radio recur weekly", "&Weekly" ), ruleButtonGroup );
    mWeeklyButton->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Sets the event or to-do to recur weekly "
             "according to the specified rules." ) );
    buttonLayout->addWidget( mWeeklyButton );
    mMonthlyButton = new QRadioButton(
      i18nc( "@option:radio recur monthly", "&Monthly" ), ruleButtonGroup );
    mMonthlyButton->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Sets the event or to-do to recur monthly "
             "according to the specified rules." ) );
    buttonLayout->addWidget( mMonthlyButton );
    mYearlyButton = new QRadioButton(
      i18nc( "@option:radio recur yearly", "&Yearly" ), ruleButtonGroup );
    mYearlyButton->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Sets the event or to-do to recur yearly "
             "according to the specified rules." ) );
    buttonLayout->addWidget( mYearlyButton );

    connect( mDailyButton, SIGNAL(toggled(bool)), SLOT(emitChoice()) );
    connect( mWeeklyButton, SIGNAL(toggled(bool)), SLOT(emitChoice()) );
    connect( mMonthlyButton, SIGNAL(toggled(bool)), SLOT(emitChoice()) );
    connect( mYearlyButton, SIGNAL(toggled(bool)), SLOT(emitChoice()) );
  }
}

int RecurrenceChooser::type()
{
  if ( mTypeCombo ) {
    return mTypeCombo->currentIndex();
  } else {
    if ( mDailyButton->isChecked() ) {
      return Daily;
    }
    else if ( mWeeklyButton->isChecked() ) {
      return Weekly;
    }
    else if ( mMonthlyButton->isChecked() ) {
      return Monthly;
    } else {
      return Yearly;
    }
  }
}

void RecurrenceChooser::setType( int type )
{
  if ( mTypeCombo ) {
    mTypeCombo->setCurrentIndex( type );
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

KOEditorRecurrence::KOEditorRecurrence( QWidget *parent ) : QWidget( parent )
{
  mParent = parent;

  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  mEnabledCheck = new QCheckBox( i18nc( "@option:check", "&Enable recurrence" ), this );
  mEnabledCheck->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Enables recurrence for this event or to-do according "
           "to the specified rules." ) );
  connect( mEnabledCheck, SIGNAL(toggled(bool)), SLOT(setRecurrenceEnabled(bool)) );
  topLayout->addWidget( mEnabledCheck, 0, 0, 1, 2 );

  mTimeGroupBox = new QGroupBox( i18nc( "@title:group", "Appointment Time " ), this );
  mTimeGroupBox->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Displays appointment time information." ) );
  topLayout->addWidget( mTimeGroupBox, 1, 0, 1, 2 );

  if ( KOPrefs::instance()->mCompactDialogs ) {
    mTimeGroupBox->hide();
  }

  QBoxLayout *layoutTimeBox = new QHBoxLayout( mTimeGroupBox );
  layoutTimeBox->setSpacing( KDialog::spacingHint() );

  mDateTimeLabel = new QLabel( mTimeGroupBox );
  layoutTimeBox->addWidget( mDateTimeLabel );

  mRuleBox = new QGroupBox( i18nc( "@title:group", "Recurrence Rule" ), this );
  mRuleBox->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Options concerning the type of recurrence this event "
           "or to-do should have." ) );
  QBoxLayout *boxlayout = new QHBoxLayout( mRuleBox );

  if ( KOPrefs::instance()->mCompactDialogs ) {
    topLayout->addWidget( mRuleBox, 2, 0 );
  } else {
    topLayout->addWidget( mRuleBox, 2, 0, 1, 2 );
  }

  mRecurrenceChooser = new RecurrenceChooser( mRuleBox );
  connect( mRecurrenceChooser, SIGNAL(chosen(int)), SLOT(showCurrentRule(int)) );
  boxlayout->addWidget( mRecurrenceChooser );

  if ( !KOPrefs::instance()->mCompactDialogs ) {
    QFrame *ruleSepFrame = new QFrame( mRuleBox );
    ruleSepFrame->setFrameStyle( QFrame::VLine | QFrame::Sunken );
    boxlayout->addWidget( ruleSepFrame );
  }

  mRuleStack = new QStackedWidget( mRuleBox );
  boxlayout->addWidget( mRuleStack );

  mDaily = new RecurDaily( mRuleStack );
  mRuleStack->insertWidget( 0, mDaily );

  mWeekly = new RecurWeekly( mRuleStack );
  mRuleStack->insertWidget( 0, mWeekly );

  mMonthly = new RecurMonthly( mRuleStack );
  mRuleStack->insertWidget( 0, mMonthly );

  mYearly = new RecurYearly( mRuleStack );
  mRuleStack->insertWidget( 0, mYearly );

  showCurrentRule( mRecurrenceChooser->type() );

  if ( KOPrefs::instance()->mCompactDialogs ) {
    mRecurrenceRangeWidget = 0;
    mRecurrenceRangeDialog = new RecurrenceRangeDialog( this );
    mRecurrenceRange = mRecurrenceRangeDialog;
    mRecurrenceRangeButton =
      new QPushButton( i18nc( "@action:button", "Recurrence Range..." ), this );
    mRecurrenceRangeButton->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Options concerning the time range during which "
             "this event or to-do should recur." ) );
    topLayout->addWidget( mRecurrenceRangeButton, 3, 0 );
    connect( mRecurrenceRangeButton, SIGNAL(clicked()), SLOT(showRecurrenceRangeDialog()) );

    mExceptionsWidget = 0;
    mExceptionsDialog = new ExceptionsDialog( this );
    mExceptions = mExceptionsDialog;
    mExceptionsButton =
      new QPushButton( i18nc( "@action:button", "Exceptions..." ), this );
    topLayout->addWidget( mExceptionsButton, 4, 0 );
    connect( mExceptionsButton, SIGNAL(clicked()), SLOT(showExceptionsDialog()) );

  } else {
    mRecurrenceRangeWidget = new RecurrenceRangeWidget( this );
    mRecurrenceRangeWidget->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Options concerning the time range during which "
             "this event or to-do should recur." ) );
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

void KOEditorRecurrence::setRecurrenceEnabled( bool enabled )
{
  mTimeGroupBox->setEnabled( enabled );
  mRuleBox->setEnabled( enabled );
  if ( mRecurrenceRangeWidget ) {
    mRecurrenceRangeWidget->setEnabled( enabled );
  }
  if ( mRecurrenceRangeButton ) {
    mRecurrenceRangeButton->setEnabled( enabled );
  }
  if ( mExceptionsWidget ) {
    mExceptionsWidget->setEnabled( enabled );
  }
  if ( mExceptionsButton ) {
    mExceptionsButton->setEnabled( enabled );
  }
}

void KOEditorRecurrence::showCurrentRule( int current )
{
  switch ( current ) {
  case Daily:
    mRuleStack->setCurrentWidget( mDaily );
    break;
  case Weekly:
    mRuleStack->setCurrentWidget( mWeekly );
    break;
  case Monthly:
    mRuleStack->setCurrentWidget( mMonthly );
    break;
  default:
  case Yearly:
    mRuleStack->setCurrentWidget( mYearly );
    break;
  }
}

void KOEditorRecurrence::setDateTimes( const QDateTime &start, const QDateTime &end )
{
  mEventStartDt = start;
  mRecurrenceRange->setDateTimes( start, end );
  mDaily->setDateTimes( start, end );
  mWeekly->setDateTimes( start, end );
  mMonthly->setDateTimes( start, end );
  mYearly->setDateTimes( start, end );

  // Now set the defaults for all unused types, use the start time for it
  bool enabled = mEnabledCheck->isChecked();
  int type = mRecurrenceChooser->type();

  if ( !enabled || type != RecurrenceChooser::Weekly ) {
    QBitArray days( 7 );
    days.fill( 0 );
    days.setBit( ( start.date().dayOfWeek() + 6 ) % 7 );
    mWeekly->setDays( days );
  }
  if ( !enabled || type != RecurrenceChooser::Monthly ) {
    mMonthly->setByPos( ( start.date().day() - 1 ) / 7 + 1, start.date().dayOfWeek() - 1 );
    mMonthly->setByDay( start.date().day() );
  }
  if ( !enabled || type != RecurrenceChooser::Yearly ) {
    mYearly->setByDay( start.date().dayOfYear() );
    mYearly->setByPos( ( start.date().day() - 1 ) / 7 + 1,
                       start.date().dayOfWeek(), start.date().month() );
    mYearly->setByMonth( start.date().day(), start.date().month() );
  }
}

void KOEditorRecurrence::setDefaults( const QDateTime &from, const QDateTime &to, bool )
{
  setDateTimes( from, to );

  bool enabled = false;
  mEnabledCheck->setChecked( enabled );
  setRecurrenceEnabled( enabled );

  mRecurrenceRange->setDefaults( from );

  mRecurrenceChooser->setType( RecurrenceChooser::Weekly );
  showCurrentRule( mRecurrenceChooser->type() );

  mDaily->setFrequency( 1 );

  mWeekly->setFrequency( 1 );
  QBitArray days( 7 );
  days.fill( 0 );
  days.setBit( ( from.date().dayOfWeek() + 6 ) % 7 );
  mWeekly->setDays( days );

  mMonthly->setFrequency( 1 );
  mMonthly->setByPos( ( from.date().day() - 1 ) / 7 + 1, from.date().dayOfWeek() );
  mMonthly->setByDay( from.date().day() );

  mYearly->setFrequency( 1 );
  mYearly->setByDay( from.date().dayOfYear() );
  mYearly->setByPos( ( from.date().day() - 1 ) / 7 + 1,
                     from.date().dayOfWeek() - 1, from.date().month() );
  mYearly->setByMonth( from.date().day(), from.date().month() );
}

void KOEditorRecurrence::readIncidence( Incidence *incidence )
{
  if ( !incidence ) {
    return;
  }

  QBitArray rDays( 7 );
  int day = 0;
  int count = 0;
  int month = 0;

  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
  if ( incidence->type() == "Todo" ) {
    Todo *todo = static_cast<Todo *>( incidence );
    setDefaults( todo->dtStart( true ).toTimeSpec( timeSpec ).dateTime(),
                 todo->dtDue().toTimeSpec( timeSpec ).dateTime(), todo->allDay() );
  } else {
    setDefaults( incidence->dtStart().toTimeSpec( timeSpec ).dateTime(),
                 incidence->dtEnd().toTimeSpec( timeSpec ).dateTime(), incidence->allDay() );
  }

  uint recurs = incidence->recurrenceType();
  int f = 0;
  Recurrence *r = 0;

  if ( recurs ) {
    r = incidence->recurrence();
    f = r->frequency();
  }

  mEnabledCheck->setChecked( recurs );
  setRecurrenceEnabled( recurs );

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
  {
    // TODO: we only handle one possibility in the list right now,
    // so I have hardcoded calls with first().  If we make the GUI
    // more extended, this can be changed.
    recurrenceType = RecurrenceChooser::Monthly;

    QList<RecurrenceRule::WDayPos> rmp = r->monthPositions();
    if ( !rmp.isEmpty() ) {
      mMonthly->setByPos( rmp.first().pos(), rmp.first().day() );
    }

    mMonthly->setFrequency( f );

    break;
  }
  case Recurrence::rMonthlyDay:
  {
    recurrenceType = RecurrenceChooser::Monthly;

    QList<int> rmd = r->monthDays();
    // check if we have any setting for which day (vcs import is broken and
    // does not set any day, thus we need to check)
    if ( rmd.isEmpty() ) {
      day = incidence->dtStart().date().day();
    } else {
      day = rmd.first();
    }
    mMonthly->setByDay( day );

    mMonthly->setFrequency( f );

    break;
  }
  case Recurrence::rYearlyMonth:
  {
    recurrenceType = RecurrenceChooser::Yearly;
    QList<int> rmd = r->yearDates();
    if ( rmd.isEmpty() ) {
      day = incidence->dtStart().date().day();
    } else {
      day = rmd.first();
    }
    int month = incidence->dtStart().date().month();
    rmd = r->yearMonths();
    if ( !rmd.isEmpty() ) {
      month = rmd.first();
    }
    mYearly->setByMonth( day, month );
    mYearly->setFrequency( f );
    break;
  }
  case Recurrence::rYearlyPos:
  {
    recurrenceType = RecurrenceChooser::Yearly;

    QList<int> months = r->yearMonths();
    if ( months.isEmpty() ) {
      month = incidence->dtStart().date().month();
    } else {
      month = months.first();
    }

    QList<RecurrenceRule::WDayPos> pos = r->yearPositions();

    if ( pos.isEmpty() ) {
      // Use dtStart if nothing is given (shouldn't happen!)
      count = ( incidence->dtStart().date().day() - 1 ) / 7;
      day = incidence->dtStart().date().dayOfWeek();
    } else {
      count = pos.first().pos();
      day = pos.first().day();
    }
    mYearly->setByPos( count, day, month );
    mYearly->setFrequency( f );
    break;
  }
  case Recurrence::rYearlyDay:
  {
    recurrenceType = RecurrenceChooser::Yearly;
    QList<int> days = r->yearDays();
    if ( days.isEmpty() ) {
      day = incidence->dtStart().date().dayOfYear();
    } else {
      day = days.first();
    }
    mYearly->setByDay( day );

    mYearly->setFrequency( f );
    break;
  }
  default:
    break;
  }

  mRecurrenceChooser->setType( recurrenceType );
  showCurrentRule( recurrenceType );

  mRecurrenceRange->setDateTimes(
    incidence->recurrence()->startDateTime().toTimeSpec( timeSpec ).dateTime() );

  if ( incidence->recurs() && r ) {
    mRecurrenceRange->setDuration( r->duration() );
    if ( r->duration() == 0 ) {
      mRecurrenceRange->setEndDate( r->endDate() );
    }
  }

  mExceptions->setDates( incidence->recurrence()->exDates() );
}

void KOEditorRecurrence::fillIncidence( Incidence *incidence )
{
  if ( !mEnabledCheck->isChecked() || !isEnabled() ) {
    if ( incidence->recurs() ) {
      incidence->recurrence()->unsetRecurs();
    }
    return;
  }

  Recurrence *r = incidence->recurrence();

  // clear out any old settings;
  r->unsetRecurs();

  int duration = mRecurrenceRange->duration();
  QDate endDate;
  if ( duration == 0 ) {
    endDate = mRecurrenceRange->endDate();
  }

  int recurrenceType = mRecurrenceChooser->type();
  if ( recurrenceType == RecurrenceChooser::Daily ) {
    r->setDaily( mDaily->frequency() );
  } else if ( recurrenceType == RecurrenceChooser::Weekly ) {
    r->setWeekly( mWeekly->frequency(), mWeekly->days() );
  } else if ( recurrenceType == RecurrenceChooser::Monthly ) {
    r->setMonthly( mMonthly->frequency() );

    if ( mMonthly->byPos() ) {
      int pos = mMonthly->count();

      QBitArray days( 7 );
      days.fill( false );
      days.setBit( mMonthly->weekday() - 1 );
      r->addMonthlyPos( pos, days );
    } else {
      // it's by day
      r->addMonthlyDate( mMonthly->day() );
    }
  } else if ( recurrenceType == RecurrenceChooser::Yearly ) {
    r->setYearly( mYearly->frequency() );

    switch ( mYearly->getType() ) {
    case RecurYearly::byMonth:
      r->addYearlyDate( mYearly->monthDay() );
      r->addYearlyMonth( mYearly->month() );
      break;
    case RecurYearly::byPos:
    {
      r->addYearlyMonth( mYearly->posMonth() );
      QBitArray days( 7 );
      days.fill( false );
      days.setBit( mYearly->posWeekday() - 1 );
      r->addYearlyPos( mYearly->posCount(), days );
      break;
    }
    case RecurYearly::byDay:
      r->addYearlyDay( mYearly->day() );
      break;
    }
  } // end "Yearly"

  if ( duration > 0 ) {
    r->setDuration( duration );
  } else if ( duration == 0 ) {
    r->setEndDate( endDate );
  }
  incidence->recurrence()->setExDates( mExceptions->dates() );
}

void KOEditorRecurrence::setDateTimeStr( const QString &str )
{
  mDateTimeLabel->setText( str );
}

bool KOEditorRecurrence::validateInput()
{
  // Check input here.
  // Check if the recurrence (if set to end at a date) is scheduled to end before the event starts.
  if ( mEnabledCheck->isChecked() && ( mRecurrenceRange->duration() == 0 ) &&
       mEventStartDt.isValid() && ( mRecurrenceRange->endDate() < mEventStartDt.date() ) ) {
    KMessageBox::sorry(
      mParent,
      i18nc( "@info",
             "The end date '%1' of the recurrence must be after the start date '%2' of the event.",
             KGlobal::locale()->formatDate( mRecurrenceRange->endDate() ),
             KGlobal::locale()->formatDate( mEventStartDt.date() ) ) );
    return false;
  }
  int recurrenceType = mRecurrenceChooser->type();
  // Check if a weekly recurrence has at least one day selected
  // TODO: Get rid of this, it's not really needed (by default the day should be taken from dtStart)
  if( mEnabledCheck->isChecked() && recurrenceType == RecurrenceChooser::Weekly ) {
    const QBitArray &days = mWeekly->days();
    bool valid = false;
    for ( int i=0; i < 7; ++i ) {
      valid = valid || days.testBit( i );
    }
    if ( !valid ) {
      KMessageBox::sorry( mParent,
        i18nc( "@info",
               "A weekly recurring event or task has to have at least one "
               "weekday associated with it." ) );
      return false;
    }
  }
  return true;
}

void KOEditorRecurrence::showExceptionsDialog()
{
  DateList dates = mExceptions->dates();
  int result = mExceptionsDialog->exec();
  if ( result == QDialog::Rejected ) {
    mExceptions->setDates( dates );
  }
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

bool KOEditorRecurrence::recurs()
{
  return mEnabledCheck->isChecked();
}

KOEditorRecurrenceDialog::KOEditorRecurrenceDialog( QWidget *parent )
  : KDialog( parent )
{
  setModal( false );
  setCaption( i18nc( "@title:window", "Recurrence" ) );
  setButtons( Ok | Cancel );

  mRecurrence = new KOEditorRecurrence( this );
  setMainWidget( mRecurrence );
}
