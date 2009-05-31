/*
  This file is part of KOrganizer.
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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

#include "koeditorgeneralevent.h"
#include "koprefs.h"

#include <libkdepim/kdateedit.h>
#include <libkdepim/ktimeedit.h>
#include <libkdepim/ktimezonecombobox.h>

#include <KCal/Event>
#include <KCal/IncidenceFormatter>

#include <KDialog>
#include <KMessageBox>
#include <KRichTextWidget>

#include <QBoxLayout>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>

KOEditorGeneralEvent::KOEditorGeneralEvent( Calendar *calendar, QObject *parent )
  : KOEditorGeneral( calendar, parent )
{
  connect( this, SIGNAL(dateTimesChanged(const QDateTime &,const QDateTime &)),
           SLOT(setDuration()) );
  connect( this, SIGNAL(dateTimesChanged(const QDateTime &,const QDateTime &)),
           SLOT(emitDateTimeStr()) );
}

KOEditorGeneralEvent::~KOEditorGeneralEvent()
{
}

void KOEditorGeneralEvent::finishSetup()
{
  QWidget::setTabOrder( mSummaryEdit, mLocationEdit );
  QWidget::setTabOrder( mLocationEdit, mCategoriesButton );
  QWidget::setTabOrder( mCategoriesButton, mStartDateEdit );
  QWidget::setTabOrder( mStartDateEdit, mStartTimeEdit );
  QWidget::setTabOrder( mStartTimeEdit, mTimeZoneComboStart );
  QWidget::setTabOrder( mTimeZoneComboStart, mEndDateEdit );
  QWidget::setTabOrder( mEndDateEdit, mEndTimeEdit );
  QWidget::setTabOrder( mEndTimeEdit, mTimeZoneComboEnd );
  QWidget::setTabOrder( mTimeZoneComboEnd, mHasTimeCheckbox );
  QWidget::setTabOrder( mHasTimeCheckbox, mSecrecyCombo );
  QWidget::setTabOrder( mSecrecyCombo, mFreeTimeCombo );
  QWidget::setTabOrder( mFreeTimeCombo, mAlarmButton );
  QWidget::setTabOrder( mAlarmButton, mAlarmTimeEdit );
  QWidget::setTabOrder( mAlarmTimeEdit, mAlarmIncrCombo );
  QWidget::setTabOrder( mAlarmIncrCombo, mAlarmEditButton );
  QWidget::setTabOrder( mAlarmEditButton, mDescriptionEdit );

  mSummaryEdit->setFocus();
}

void KOEditorGeneralEvent::initTime( QWidget *parent, QBoxLayout *topLayout )
{
  QBoxLayout *timeLayout = new QVBoxLayout();
  topLayout->addItem( timeLayout );

  QGroupBox *timeGroupBox = new QGroupBox( i18n( "Date && Time" ), parent );
  timeGroupBox->setWhatsThis(
    i18n( "Sets options related to the date and time of the event or to-do." ) );
  timeLayout->addWidget( timeGroupBox );

  QGridLayout *layoutTimeBox = new QGridLayout( timeGroupBox );
  layoutTimeBox->setSpacing( KDialog::spacingHint() );

  mStartDateLabel = new QLabel( i18nc( "@label event start time", "&Start:" ), timeGroupBox );
  layoutTimeBox->addWidget( mStartDateLabel, 0, 0 );
  layoutTimeBox->setColumnStretch( 3, 1 );

  mStartDateEdit = new KPIM::KDateEdit( timeGroupBox );
  mStartDateEdit->setToolTip(
    i18nc( "@info:tooltip", "Set the start date" ) );
  mStartDateEdit->setWhatsThis(
    i18nc( "@info:whatsthis", "Select the starting date for this event." ) );
  layoutTimeBox->addWidget( mStartDateEdit, 0, 1 );
  mStartDateLabel->setBuddy( mStartDateEdit );

  mStartTimeEdit = new KPIM::KTimeEdit( timeGroupBox );
  mStartTimeEdit->setToolTip(
    i18nc( "@info:tooltip", "Set the start time" ) );
  mStartTimeEdit->setWhatsThis(
    i18nc( "@info:whatsthis", "Select the starting time for this event." ) );
  layoutTimeBox->addWidget( mStartTimeEdit, 0, 2 );

  // Timezone
  QString whatsThis = i18nc( "@info:whatsthis",
                             "Select the timezone for this event. "
                             "It will also affect recurrences" );
  mTimeZoneComboStart = new KPIM::KTimeZoneComboBox( mCalendar, timeGroupBox );
  mTimeZoneComboEnd = new KPIM::KTimeZoneComboBox( mCalendar, timeGroupBox );
  if ( !KOPrefs::instance()->showTimeZoneSelectorInIncidenceEditor() ) {
    mTimeZoneComboStart->hide();
    mTimeZoneComboEnd->hide();
  }
  layoutTimeBox->addWidget( mTimeZoneComboStart, 0, 3 );
  layoutTimeBox->addWidget( mTimeZoneComboEnd, 1, 3 );

  mTimeZoneComboStart->selectLocalTimeSpec();
  mTimeZoneComboStart->setToolTip(
    i18nc( "@info:tooltip", "Select the timezone for the start date/time" ) );
  mTimeZoneComboStart->setWhatsThis( whatsThis );

  mTimeZoneComboEnd->selectLocalTimeSpec();
  mTimeZoneComboEnd->setToolTip(
    i18nc( "@info:tooltip", "Select the timezone for the end date/time" ) );
  mTimeZoneComboEnd->setWhatsThis( whatsThis );

  mStartSpec = mTimeZoneComboStart->selectedTimeSpec();
  mEndSpec = mTimeZoneComboEnd->selectedTimeSpec();

  mEndDateLabel = new QLabel( i18n( "&End:" ), timeGroupBox );
  layoutTimeBox->addWidget( mEndDateLabel, 1, 0 );

  mEndDateEdit = new KPIM::KDateEdit( timeGroupBox );
  mEndDateEdit->setToolTip( i18nc( "@info:tooltip", "Set the end date" ) );
  mEndDateEdit->setWhatsThis(
    i18nc( "@info:whatsthis", "Select the ending date for this event." ) );
  layoutTimeBox->addWidget( mEndDateEdit, 1, 1 );
  mEndDateLabel->setBuddy( mEndDateEdit );

  mEndTimeEdit = new KPIM::KTimeEdit( timeGroupBox );
  mEndTimeEdit->setToolTip( i18nc( "@info:tooltip", "Set the end time" ) );
  mEndTimeEdit->setWhatsThis(
    i18nc( "@info:whatsthis", "Select the starting time for this event." ) );
  layoutTimeBox->addWidget( mEndTimeEdit, 1, 2 );

  mHasTimeCheckbox = new QCheckBox( i18n( "T&ime associated" ), timeGroupBox );
  layoutTimeBox->addWidget( mHasTimeCheckbox, 0, 4 );
  connect( mHasTimeCheckbox, SIGNAL(toggled(bool)), SLOT(slotHasTimeCheckboxToggled(bool)) );

  mDurationLabel = new QLabel( timeGroupBox );
  layoutTimeBox->addWidget( mDurationLabel, 1, 4 );

  // time widgets are checked if they contain a valid time
  connect( mStartTimeEdit, SIGNAL(timeChanged(QTime)),
           this, SLOT(startTimeChanged(QTime)) );
  connect( mEndTimeEdit, SIGNAL(timeChanged(QTime)),
           this, SLOT(endTimeChanged(QTime)) );

  // date widgets are checked if they contain a valid date
  connect( mStartDateEdit, SIGNAL(dateChanged(const QDate&)),
           this, SLOT(startDateChanged(const QDate&)) );
  connect( mEndDateEdit, SIGNAL(dateChanged(const QDate&)),
           this, SLOT(endDateChanged(const QDate&)) );

  connect( mTimeZoneComboStart, SIGNAL(currentIndexChanged(int)),
           this, SLOT(startSpecChanged()) );
  connect( mTimeZoneComboEnd, SIGNAL(currentIndexChanged(int)),
           this, SLOT(endSpecChanged()) );

  QBoxLayout *recLayout = new QHBoxLayout();
  layoutTimeBox->addLayout( recLayout, 2, 1, 1, 3 );
  mRecurrenceSummary = new QLabel( QString(), timeGroupBox );
  recLayout->addWidget( mRecurrenceSummary );
  QPushButton *recEditButton = new QPushButton( i18n( "Edit..." ), timeGroupBox );
  recLayout->addWidget( recEditButton );
  connect( recEditButton, SIGNAL(clicked()), SIGNAL(editRecurrence()) );
  recLayout->addStretch( 1 );

  QLabel *label = new QLabel( i18n( "Reminder:" ), timeGroupBox );
  layoutTimeBox->addWidget( label, 3, 0 );
  QBoxLayout *alarmLineLayout = new QHBoxLayout();
  layoutTimeBox->addLayout( alarmLineLayout, 3, 1, 1, 3 );
  initAlarm( timeGroupBox, alarmLineLayout );
  alarmLineLayout->addStretch( 1 );

  QBoxLayout *secLayout = new QHBoxLayout();
  layoutTimeBox->addLayout( secLayout, 0, 5 );
  initSecrecy( timeGroupBox, secLayout );

  QBoxLayout *classLayout = new QHBoxLayout();
  layoutTimeBox->addLayout( classLayout, 1, 5 );
  initClass( timeGroupBox, classLayout );
}

void KOEditorGeneralEvent::initClass( QWidget *parent, QBoxLayout *topLayout )
{
  QBoxLayout *classLayout = new QHBoxLayout();
  classLayout->setSpacing( topLayout->spacing() );
  topLayout->addItem( classLayout );

  QLabel *freeTimeLabel = new QLabel( i18n( "S&how time as:" ), parent );
  QString whatsThis =
    i18nc( "@info:whatsthis",
           "Sets how this time will appear on your Free/Busy information." );
  freeTimeLabel->setWhatsThis( whatsThis );
  classLayout->addWidget( freeTimeLabel );

  mFreeTimeCombo = new KComboBox( parent );
  mFreeTimeCombo->setEditable( false );
  mFreeTimeCombo->setToolTip(
    i18nc( "@info:tooltip", "Set this event as Busy or Free time" ) );
  mFreeTimeCombo->setWhatsThis( whatsThis );
  mFreeTimeCombo->addItem( i18nc( "show event as busy time", "Busy" ) );
  mFreeTimeCombo->addItem( i18nc( "show event as free time", "Free" ) );
  classLayout->addWidget( mFreeTimeCombo );
  freeTimeLabel->setBuddy( mFreeTimeCombo );
}

void KOEditorGeneralEvent::initInvitationBar( QWidget *parent, QBoxLayout *layout )
{
  mInvitationBar = new QFrame( parent );
  layout->addWidget( mInvitationBar );

  QHBoxLayout *barLayout = new QHBoxLayout( mInvitationBar );
  barLayout->setSpacing( layout->spacing() );
  QLabel *label =
    new QLabel( i18nc(
                  "@info",
                  "You have not yet definitely responded to this invitation." ),
                mInvitationBar );
  barLayout->addWidget( label );
  barLayout->addStretch( 1 );

  QPushButton *button = new QPushButton( i18n( "Accept" ), mInvitationBar );
  button->setToolTip(
    i18nc( "@info:tooltip", "Accept the invitation" ) );
  button->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Click this button to accept the invitation." ) );
  connect( button, SIGNAL(clicked()), SIGNAL(acceptInvitation()) );
  connect( button, SIGNAL(clicked()), mInvitationBar, SLOT(hide()) );
  barLayout->addWidget( button );

  button = new QPushButton( i18n( "Decline" ), mInvitationBar );
  button->setToolTip(
    i18nc( "@info:tooltip", "Decline the invitation" ) );
  button->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Click this button to decline the invitation." ) );
  connect( button, SIGNAL(clicked()), SIGNAL(declineInvitation()) );
  connect( button, SIGNAL(clicked()), mInvitationBar, SLOT(hide()) );
  barLayout->addWidget( button );

  mInvitationBar->hide();
}

void KOEditorGeneralEvent::setTimeEditorsEnabled( bool enabled )
{
  mStartTimeEdit->setEnabled( enabled );
  mEndTimeEdit->setEnabled( enabled );

  if ( !enabled ) {
    mTimeZoneComboStart->setFloating( true );
    mTimeZoneComboEnd->setFloating( true );
  } else {
    mTimeZoneComboStart->selectLocalTimeSpec();
    mTimeZoneComboEnd->selectLocalTimeSpec();
    mStartSpec = mTimeZoneComboStart->selectedTimeSpec();
    mEndSpec = mTimeZoneComboEnd->selectedTimeSpec();
  }
  mTimeZoneComboStart->setEnabled( enabled );
  mTimeZoneComboEnd->setEnabled( enabled );

  setDuration();
  emitDateTimeStr();
}

void KOEditorGeneralEvent::slotHasTimeCheckboxToggled( bool checked )
{
  setTimeEditorsEnabled( checked );
  //if(alarmButton->isChecked()) alarmStuffDisable(noTime);
  emit allDayChanged( !checked );
}

void KOEditorGeneralEvent::setDateTimes( const QDateTime &start, const QDateTime &end )
{
  setDateTimes( KDateTime( start, KOPrefs::instance()->timeSpec() ),
                KDateTime( end, KOPrefs::instance()->timeSpec() ) );
}

void KOEditorGeneralEvent::setDateTimes( const KDateTime &start, const KDateTime &end )
{
  mStartDateEdit->setDate( start.date() );
  // KTimeEdit seems to emit some signals when setTime() is called.
  mStartTimeEdit->blockSignals( true );
  mStartTimeEdit->setTime( start.time() );
  mStartTimeEdit->blockSignals( false );
  mEndDateEdit->setDate( end.date() );
  mEndTimeEdit->setTime( end.time() );

  mCurrStartDateTime = start.dateTime();
  mCurrEndDateTime = end.dateTime();

  mTimeZoneComboStart->selectTimeSpec( start.timeSpec() );
  mTimeZoneComboEnd->selectTimeSpec( end.timeSpec() );

  setDuration();
  emitDateTimeStr();
}

void KOEditorGeneralEvent::setTimes( const QDateTime &start, const QDateTime &end )
{
  setDateTimes( KDateTime( start, KOPrefs::instance()->timeSpec() ),
                KDateTime( end, KOPrefs::instance()->timeSpec() ) );
}

void KOEditorGeneralEvent::setTimes( const KDateTime &start, const KDateTime &end )
{
  // like setDateTimes(), but it set only the start/end time, not the date
  // it is used while applying a template to an event.
  mStartTimeEdit->blockSignals( true );
  mStartTimeEdit->setTime( start.time() );
  mStartTimeEdit->blockSignals( false );

  mEndTimeEdit->setTime( end.time() );

  mTimeZoneComboStart->selectTimeSpec( start.timeSpec() );
  mTimeZoneComboEnd->selectTimeSpec( end.timeSpec() );

  setDuration();
  emitDateTimeStr();
}

void KOEditorGeneralEvent::startTimeChanged( QTime newtime )
{
  int secsep = mCurrStartDateTime.secsTo( mCurrEndDateTime );

  mCurrStartDateTime.setTime( newtime );

  // adjust end time so that the event has the same duration as before.
  mCurrEndDateTime = mCurrStartDateTime.addSecs( secsep );
  mEndTimeEdit->setTime( mCurrEndDateTime.time() );
  mEndDateEdit->setDate( mCurrEndDateTime.date() );

  emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
}

void KOEditorGeneralEvent::endTimeChanged( QTime newtime )
{
  QDateTime newdt( mCurrEndDateTime.date(), newtime );
  mCurrEndDateTime = newdt;

  emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
}

void KOEditorGeneralEvent::startDateChanged( const QDate &newdate )
{
  if ( !newdate.isValid() ) {
    return;
  }

  int daysep = mCurrStartDateTime.daysTo( mCurrEndDateTime );
  mCurrStartDateTime.setDate( newdate );

  // adjust end date so that the event has the same duration as before
  mCurrEndDateTime.setDate( mCurrStartDateTime.date().addDays( daysep ) );
  mEndDateEdit->setDate( mCurrEndDateTime.date() );

  emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
}

void KOEditorGeneralEvent::endDateChanged( const QDate &newdate )
{
  if ( !newdate.isValid() ) {
    return;
  }

  QDateTime newdt( newdate, mCurrEndDateTime.time() );
  mCurrEndDateTime = newdt;

  emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
}

void KOEditorGeneralEvent::startSpecChanged()
{
  if ( mEndSpec == mStartSpec ) {
    mTimeZoneComboEnd->selectTimeSpec( mTimeZoneComboStart->selectedTimeSpec() );
  }
  mStartSpec = mTimeZoneComboStart->selectedTimeSpec();

  emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
}

void KOEditorGeneralEvent::endSpecChanged()
{
  mEndSpec = mTimeZoneComboEnd->selectedTimeSpec();

  emit dateTimesChanged( mCurrStartDateTime, mCurrEndDateTime );
}

void KOEditorGeneralEvent::setDefaults( const QDateTime &from,
                                        const QDateTime &to, bool allDay )
{
  KOEditorGeneral::setDefaults( allDay );
  mHasTimeCheckbox->setChecked( !allDay );
  setTimeEditorsEnabled( !allDay );

  mTimeZoneComboStart->selectLocalTimeSpec();
  mTimeZoneComboEnd->selectLocalTimeSpec();
  mStartSpec = mTimeZoneComboStart->selectedTimeSpec();
  mEndSpec = mTimeZoneComboEnd->selectedTimeSpec();

  setDateTimes( from, to );
}

void KOEditorGeneralEvent::readEvent( Event *event, bool isTemplate )
{
  mHasTimeCheckbox->setChecked( !event->allDay() );
  setTimeEditorsEnabled( !event->allDay() );

  if ( !isTemplate ) {
    // the rest is for the events only
    setDateTimes( event->dtStart(), event->dtEnd() );
  } else {
    // set the start/end time from the template, only as a last resort #190545
    if ( !event->dtStart().isValid() || !event->dtEnd().isValid() ) {
      setTimes( event->dtStart(), event->dtEnd() );
    }
  }

  switch( event->transparency() ) {
  case Event::Transparent:
    mFreeTimeCombo->setCurrentIndex( 1 );
    break;
  case Event::Opaque:
    mFreeTimeCombo->setCurrentIndex( 0 );
    break;
  }

  mRecurrenceSummary->setText( IncidenceFormatter::recurrenceString( event ) );
  Attendee *me = event->attendeeByMails( KOPrefs::instance()->allEmails() );
  if ( event->attendeeCount() > 1 &&
       me && ( me->status() == Attendee::NeedsAction ||
               me->status() == Attendee::Tentative ||
               me->status() == Attendee::InProcess ) ) {
    mInvitationBar->show();
  } else {
    mInvitationBar->hide();
  }

  readIncidence( event );
}

void KOEditorGeneralEvent::fillEvent( Event *event )
{
  fillIncidence( event );

  QDate tmpDate;
  QTime tmpTime;
  KDateTime tmpDT;

  // temp. until something better happens.
  QString tmpStr;

  if ( !mHasTimeCheckbox->isChecked() ) {
    event->setAllDay( true );

    // need to change this.
    tmpDate = mStartDateEdit->date();
    tmpDT.setDate( tmpDate );
    tmpDT.setDateOnly( true );
    tmpDT.setTimeSpec( mTimeZoneComboStart->selectedTimeSpec() );
    event->setDtStart( tmpDT );

    tmpDT.setTimeSpec( mTimeZoneComboEnd->selectedTimeSpec( ) );
    tmpDT.setDate( mEndDateEdit->date() );
    event->setDtEnd( tmpDT );
  } else {
    event->setAllDay( false );

    // set date/time end
    tmpDate = mEndDateEdit->date();
    tmpTime = mEndTimeEdit->getTime();
    tmpDT.setDate( tmpDate );
    tmpDT.setTime( tmpTime );
    tmpDT.setTimeSpec( mTimeZoneComboEnd->selectedTimeSpec() );
    event->setDtEnd( tmpDT );

    // set date/time start
    tmpDate = mStartDateEdit->date();
    tmpTime = mStartTimeEdit->getTime();
    event->setDtStart( KDateTime( tmpDate, tmpTime, mTimeZoneComboStart->selectedTimeSpec() ) );
  } // check for all-day

  event->setTransparency( mFreeTimeCombo->currentIndex() > 0 ?
                          KCal::Event::Transparent : KCal::Event::Opaque );
}

void KOEditorGeneralEvent::setDuration()
{
  QString tmpStr, catStr;
  int hourdiff, minutediff;
  // end<date is an accepted temporary state while typing, but don't show
  // any duration if this happens
  KDateTime startDateTime =
    KDateTime( mCurrStartDateTime, mTimeZoneComboStart->selectedTimeSpec() );
  KDateTime endDateTime =
    KDateTime( mCurrEndDateTime, mTimeZoneComboEnd->selectedTimeSpec() ).
    toTimeSpec( startDateTime.timeSpec() );
  if ( startDateTime < endDateTime ) {

    if ( !mHasTimeCheckbox->isChecked() ) {
      int daydiff = startDateTime.date().daysTo( endDateTime.date() ) + 1;
      tmpStr = i18n( "Duration: " );
      tmpStr.append( i18np( "1 Day", "%1 Days", daydiff ) );
    } else {
      hourdiff = startDateTime.date().daysTo( endDateTime.date() ) * 24;
      hourdiff += endDateTime.time().hour() - startDateTime.time().hour();
      minutediff = endDateTime.time().minute() - startDateTime.time().minute();
      // If minutediff is negative, "borrow" 60 minutes from hourdiff
      if ( minutediff < 0 && hourdiff > 0 ) {
        hourdiff -= 1;
        minutediff += 60;
      }
      if ( hourdiff || minutediff ) {
        tmpStr = i18n( "Duration: " );
        if ( hourdiff ){
          catStr = i18np( "1 hour", "%1 hours", hourdiff );
          tmpStr.append( catStr );
        }
        if ( hourdiff && minutediff ) {
          tmpStr += i18n( ", " );
        }
        if ( minutediff ){
          catStr = i18np( "1 minute", "%1 minutes", minutediff );
          tmpStr += catStr;
        }
      } else {
        tmpStr = "";
      }
    }
  }
  mDurationLabel->setText( tmpStr );
  mDurationLabel->setWhatsThis(
    i18n( "Shows the duration of the event or to-do with the "
          "current start and end dates and times." ) );
}

void KOEditorGeneralEvent::emitDateTimeStr()
{
  KLocale *l = KGlobal::locale();

  QString from, to;
  if ( !mHasTimeCheckbox->isChecked() ) {
    from = l->formatDate( mCurrStartDateTime.date() );
    to = l->formatDate( mCurrEndDateTime.date() );
  } else {
    from = l->formatDateTime( mCurrStartDateTime );
    to = l->formatDateTime( mCurrEndDateTime );
  }
  QString str = i18n( "From: %1   To: %2   %3", from, to, mDurationLabel->text() );

  emit dateTimeStrChanged( str );
}

bool KOEditorGeneralEvent::validateInput()
{
  if ( mHasTimeCheckbox->isChecked() ) {
    if ( !mStartTimeEdit->inputIsValid() ) {
      KMessageBox::sorry( mParent,
                          i18n( "Please specify a valid start time, for example '%1'.",
                                KGlobal::locale()->formatTime( QTime::currentTime() ) ) );
      return false;
    }

    if ( !mEndTimeEdit->inputIsValid() ) {
      KMessageBox::sorry( mParent,
                          i18n( "Please specify a valid end time, for example '%1'.",
                                KGlobal::locale()->formatTime( QTime::currentTime() ) ) );
      return false;
    }
  }

  if ( !mStartDateEdit->date().isValid() ) {
    KMessageBox::sorry( mParent,
                        i18n( "Please specify a valid start date, for example '%1'.",
                              KGlobal::locale()->formatDate( QDate::currentDate() ) ) );
    return false;
  }

  if ( !mEndDateEdit->date().isValid() ) {
    KMessageBox::sorry( mParent,
                        i18n( "Please specify a valid end date, for example '%1'.",
                              KGlobal::locale()->formatDate( QDate::currentDate() ) ) );
    return false;
  }

  KDateTime startDt, endDt;
  startDt.setTimeSpec( mTimeZoneComboStart->selectedTimeSpec() );
  endDt.setTimeSpec( mTimeZoneComboEnd->selectedTimeSpec() );
  startDt.setDate( mStartDateEdit->date() );
  endDt.setDate( mEndDateEdit->date() );
  if ( mHasTimeCheckbox->isChecked() ) {
    startDt.setTime( mStartTimeEdit->getTime() );
    endDt.setTime( mEndTimeEdit->getTime() );
  }

  if ( startDt > endDt ) {
    KMessageBox::sorry( mParent, i18n( "The event ends before it starts.\n"
                                       "Please correct dates and times." ) );
    return false;
  }

  return KOEditorGeneral::validateInput();
}

bool KOEditorGeneralEvent::setAlarmOffset( Alarm *alarm, int value ) const
{
  alarm->setStartOffset( value );
  return true;
}

void KOEditorGeneralEvent::updateRecurrenceSummary( const QString &summary )
{
  mRecurrenceSummary->setText( summary );
}

#include "koeditorgeneralevent.moc"
