/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qwhatsthis.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <krestrictedline.h>
#include <ktextedit.h>

#include <libkcal/event.h>

#include "ktimeedit.h"
#include <libkdepim/kdateedit.h>

#include "koprefs.h"

#include "koeditorgeneralevent.h"
#include "koeditorgeneralevent.moc"

KOEditorGeneralEvent::KOEditorGeneralEvent(QObject* parent,
                                           const char* name) :
  KOEditorGeneral( parent, name)
{
  connect(this,SIGNAL(dateTimesChanged(QDateTime,QDateTime)),
          SLOT(setDuration()));
  connect(this,SIGNAL(dateTimesChanged(QDateTime,QDateTime)),
          SLOT(emitDateTimeStr()));
}

KOEditorGeneralEvent::~KOEditorGeneralEvent()
{
}

void KOEditorGeneralEvent::finishSetup()
{
  QWidget::setTabOrder( mSummaryEdit, mLocationEdit );
  QWidget::setTabOrder( mLocationEdit, mStartDateEdit );
  QWidget::setTabOrder( mStartDateEdit, mStartTimeEdit );
  QWidget::setTabOrder( mStartTimeEdit, mEndDateEdit );
  QWidget::setTabOrder( mEndDateEdit, mEndTimeEdit );
  QWidget::setTabOrder( mEndTimeEdit, mTimeAssociateButton );
  QWidget::setTabOrder( mTimeAssociateButton, mAlarmButton );
  QWidget::setTabOrder( mAlarmButton, mAlarmTimeEdit );
  QWidget::setTabOrder( mAlarmTimeEdit, mAlarmIncrCombo );
  QWidget::setTabOrder( mAlarmIncrCombo, mAlarmSoundButton );
  QWidget::setTabOrder( mAlarmSoundButton, mAlarmProgramButton );
  QWidget::setTabOrder( mAlarmProgramButton, mFreeTimeCombo );
  QWidget::setTabOrder( mFreeTimeCombo, mDescriptionEdit );
  QWidget::setTabOrder( mDescriptionEdit, mCategoriesButton );
  QWidget::setTabOrder( mCategoriesButton, mSecrecyCombo );
//  QWidget::setTabOrder( mSecrecyCombo, mDescriptionEdit );

  mSummaryEdit->setFocus();
}

void KOEditorGeneralEvent::initTime(QWidget *parent,QBoxLayout *topLayout)
{
  QBoxLayout *timeLayout = new QVBoxLayout(topLayout);

  QGroupBox *timeGroupBox = new QGroupBox(1,QGroupBox::Horizontal,
                                          i18n("Date && Time"),parent);
  QWhatsThis::add( timeGroupBox,
		   i18n("Sets options related to the date and time of the "
			"event or to-do.") );
  timeLayout->addWidget(timeGroupBox);

  QFrame *timeBoxFrame = new QFrame(timeGroupBox);

  QGridLayout *layoutTimeBox = new QGridLayout(timeBoxFrame,2,3);
  layoutTimeBox->setSpacing(topLayout->spacing());


  mStartDateLabel = new QLabel(i18n("&Start:"),timeBoxFrame);
  layoutTimeBox->addWidget(mStartDateLabel,0,0);

  mStartDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mStartDateEdit,0,1);
  mStartDateLabel->setBuddy( mStartDateEdit );

  mStartTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mStartTimeEdit,0,2);


  mEndDateLabel = new QLabel(i18n("&End:"),timeBoxFrame);
  layoutTimeBox->addWidget(mEndDateLabel,1,0);

  mEndDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mEndDateEdit,1,1);
  mEndDateLabel->setBuddy( mEndDateEdit );

  mEndTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mEndTimeEdit,1,2);

  QHBoxLayout *flagsBox = new QHBoxLayout( timeBoxFrame );

  mTimeAssociateButton = new QCheckBox(i18n("T&ime associated"),timeBoxFrame);
  flagsBox->addWidget(mTimeAssociateButton);
  connect(mTimeAssociateButton, SIGNAL(toggled(bool)),SLOT(associateTime(bool)));

  mDurationLabel = new QLabel( timeBoxFrame );
  if ( KOPrefs::instance()->mCompactDialogs ) {
    layoutTimeBox->addMultiCellWidget( mDurationLabel, 3, 3, 0, 3 );
  } else {
    flagsBox->addWidget( mDurationLabel, 0, 2 );
  }

  layoutTimeBox->addMultiCellLayout( flagsBox, 2, 2, 0, 3 );

  // time widgets are checked if they contain a valid time
  connect(mStartTimeEdit, SIGNAL(timeChanged(QTime)),
          this, SLOT(startTimeChanged(QTime)));
  connect(mEndTimeEdit, SIGNAL(timeChanged(QTime)),
          this, SLOT(endTimeChanged(QTime)));

  // date widgets are checked if they contain a valid date
  connect(mStartDateEdit, SIGNAL(dateChanged(QDate)),
          this, SLOT(startDateChanged(QDate)));
  connect(mEndDateEdit, SIGNAL(dateChanged(QDate)),
          this, SLOT(endDateChanged(QDate)));
}

void KOEditorGeneralEvent::initClass(QWidget *parent,QBoxLayout *topLayout)
{
  QBoxLayout *classLayout = new QHBoxLayout(topLayout);

  QLabel *freeTimeLabel = new QLabel(i18n("S&how time as:"),parent);
  QString whatsThis = i18n("Sets how this time will appear on your Free/Busy "
		  	   "information.");
  QWhatsThis::add( freeTimeLabel, whatsThis );
  classLayout->addWidget(freeTimeLabel);

  mFreeTimeCombo = new QComboBox(false, parent);
  QWhatsThis::add( mFreeTimeCombo, whatsThis );
  mFreeTimeCombo->insertItem(i18n("Busy"));
  mFreeTimeCombo->insertItem(i18n("Free"));
  classLayout->addWidget(mFreeTimeCombo);
  freeTimeLabel->setBuddy( mFreeTimeCombo );
}

void KOEditorGeneralEvent::timeStuffDisable(bool disable)
{
  mStartTimeEdit->setEnabled( !disable );
  mEndTimeEdit->setEnabled( !disable );

  setDuration();
  emitDateTimeStr();
}

void KOEditorGeneralEvent::associateTime(bool time)
{
  timeStuffDisable(!time);
  //if(alarmButton->isChecked()) alarmStuffDisable(noTime);
  allDayChanged(!time);
}

void KOEditorGeneralEvent::setDateTimes(QDateTime start, QDateTime end)
{
//  kdDebug(5850) << "KOEditorGeneralEvent::setDateTimes(): Start DateTime: " << start.toString() << endl;

  mStartDateEdit->setDate(start.date());
  // KTimeEdit seems to emit some signals when setTime() is called.
  mStartTimeEdit->blockSignals( true );
  mStartTimeEdit->setTime(start.time());
  mStartTimeEdit->blockSignals( false );
  mEndDateEdit->setDate(end.date());
  mEndTimeEdit->setTime(end.time());

  mCurrStartDateTime = start;
  mCurrEndDateTime = end;

  setDuration();
  emitDateTimeStr();
}

void KOEditorGeneralEvent::startTimeChanged(QTime newtime)
{
  kdDebug(5850) << "KOEditorGeneralEvent::startTimeChanged() " << newtime.toString() << endl;

  int secsep = mCurrStartDateTime.secsTo(mCurrEndDateTime);

  mCurrStartDateTime.setTime(newtime);

  // adjust end time so that the event has the same duration as before.
  mCurrEndDateTime = mCurrStartDateTime.addSecs(secsep);
  mEndTimeEdit->setTime(mCurrEndDateTime.time());
  mEndDateEdit->setDate(mCurrEndDateTime.date());

  emit dateTimesChanged(mCurrStartDateTime,mCurrEndDateTime);
}

void KOEditorGeneralEvent::endTimeChanged(QTime newtime)
{
//  kdDebug(5850) << "KOEditorGeneralEvent::endTimeChanged " << newtime.toString() << endl;

  QDateTime newdt(mCurrEndDateTime.date(), newtime);
  mCurrEndDateTime = newdt;

  emit dateTimesChanged(mCurrStartDateTime,mCurrEndDateTime);
}

void KOEditorGeneralEvent::startDateChanged(QDate newdate)
{
  int daysep = mCurrStartDateTime.daysTo(mCurrEndDateTime);

  mCurrStartDateTime.setDate(newdate);

  // adjust end date so that the event has the same duration as before
  mCurrEndDateTime.setDate(mCurrStartDateTime.date().addDays(daysep));
  mEndDateEdit->setDate(mCurrEndDateTime.date());

  emit dateTimesChanged(mCurrStartDateTime,mCurrEndDateTime);
}

void KOEditorGeneralEvent::endDateChanged(QDate newdate)
{
  QDateTime newdt(newdate, mCurrEndDateTime.time());

  if(newdt < mCurrStartDateTime) {
    // oops, we can't let that happen.
    newdt = mCurrStartDateTime;
    mEndDateEdit->setDate(newdt.date());
    mEndTimeEdit->setTime(newdt.time());
  }
  mCurrEndDateTime = newdt;

  emit dateTimesChanged(mCurrStartDateTime,mCurrEndDateTime);
}

void KOEditorGeneralEvent::setDefaults(QDateTime from,QDateTime to,bool allDay)
{
  KOEditorGeneral::setDefaults(allDay);

  mTimeAssociateButton->setChecked(!allDay);
  timeStuffDisable(allDay);

  setDateTimes(from,to);
}

void KOEditorGeneralEvent::readEvent( Event *event, bool tmpl )
{
  QString tmpStr;

  mTimeAssociateButton->setChecked(!event->doesFloat());
  timeStuffDisable(event->doesFloat());

  if ( !tmpl ) {
    // the rest is for the events only
    setDateTimes(event->dtStart(),event->dtEnd());
  }

  switch( event->transparency() ) {
  case Event::Transparent:
    mFreeTimeCombo->setCurrentItem(1);
    break;
  case Event::Opaque:
    mFreeTimeCombo->setCurrentItem(0);
    break;
  }

  readIncidence(event);
}

void KOEditorGeneralEvent::writeEvent(Event *event)
{
//  kdDebug(5850) << "KOEditorGeneralEvent::writeEvent()" << endl;

  writeIncidence(event);

  QDate tmpDate;
  QTime tmpTime;
  QDateTime tmpDT;

  // temp. until something better happens.
  QString tmpStr;

  if (!mTimeAssociateButton->isChecked()) {
    event->setFloats(true);
    // need to change this.
    tmpDate = mStartDateEdit->date();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtStart(tmpDT);

    tmpDate = mEndDateEdit->date();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtEnd(tmpDT);
  } else {
    event->setFloats(false);

    // set date/time end
    tmpDate = mEndDateEdit->date();
    tmpTime = mEndTimeEdit->getTime();
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtEnd(tmpDT);

    // set date/time start
    tmpDate = mStartDateEdit->date();
    tmpTime = mStartTimeEdit->getTime();
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtStart(tmpDT);
  } // check for float

  event->setTransparency(mFreeTimeCombo->currentItem() > 0
                         ? KCal::Event::Transparent
                         : KCal::Event::Opaque);

//  kdDebug(5850) << "KOEditorGeneralEvent::writeEvent() done" << endl;
}

void KOEditorGeneralEvent::setDuration()
{
  QString tmpStr, catStr;
  int hourdiff, minutediff;
  // end<date is an accepted temporary state while typing, but don't show
  // any duration if this happens
  if(mCurrEndDateTime >= mCurrStartDateTime) {

    if (!mTimeAssociateButton->isChecked()) {
      int daydiff = mCurrStartDateTime.date().daysTo(mCurrEndDateTime.date()) + 1;
      tmpStr = i18n("Duration: ");
      tmpStr.append(i18n("1 Day","%n Days",daydiff));
    } else {
      hourdiff = mCurrStartDateTime.date().daysTo(mCurrEndDateTime.date()) * 24;
      hourdiff += mCurrEndDateTime.time().hour() -
                  mCurrStartDateTime.time().hour();
      minutediff = mCurrEndDateTime.time().minute() -
                   mCurrStartDateTime.time().minute();
      // If minutediff is negative, "borrow" 60 minutes from hourdiff
      if (minutediff < 0 && hourdiff > 0) {
        hourdiff -= 1;
        minutediff += 60;
      }
      if (hourdiff || minutediff){
        tmpStr = i18n("Duration: ");
        if (hourdiff){
          catStr = i18n("1 hour","%n hours",hourdiff);
          tmpStr.append(catStr);
        }
        if (hourdiff && minutediff){
          tmpStr += i18n(", ");
        }
        if (minutediff){
          catStr = i18n("1 minute","%n minutes",minutediff);
          tmpStr += catStr;
        }
      } else tmpStr = "";
    }
  }
  mDurationLabel->setText(tmpStr);
  QWhatsThis::add( mDurationLabel,
		   i18n("Shows the duration of the event or to-do with the "
			"current start and end dates and times.") );
}

void KOEditorGeneralEvent::emitDateTimeStr()
{
  KLocale *l = KGlobal::locale();

  QString from,to;
  if (!mTimeAssociateButton->isChecked()) {
    from = l->formatDate(mCurrStartDateTime.date());
    to = l->formatDate(mCurrEndDateTime.date());
  } else {
    from = l->formatDateTime(mCurrStartDateTime);
    to = l->formatDateTime(mCurrEndDateTime);
  }

  QString str = i18n("From: %1   To: %2   %3").arg(from).arg(to)
                .arg(mDurationLabel->text());

  emit dateTimeStrChanged(str);
}

bool KOEditorGeneralEvent::validateInput()
{
//  kdDebug(5850) << "KOEditorGeneralEvent::validateInput()" << endl;

  if (mTimeAssociateButton->isChecked()) {
    if (!mStartTimeEdit->inputIsValid()) {
      KMessageBox::sorry( 0,
          i18n("Please specify a valid start time, for example '%1'.")
          .arg( KGlobal::locale()->formatTime( QTime::currentTime() ) ) );
      return false;
    }

    if (!mEndTimeEdit->inputIsValid()) {
      KMessageBox::sorry( 0,
          i18n("Please specify a valid end time, for example '%1'.")
          .arg( KGlobal::locale()->formatTime( QTime::currentTime() ) ) );
      return false;
    }
  }

  if (!mStartDateEdit->inputIsValid()) {
    KMessageBox::sorry( 0,
        i18n("Please specify a valid start date, for example '%1'.")
        .arg( KGlobal::locale()->formatDate( QDate::currentDate() ) ) );
    return false;
  }

  if (!mEndDateEdit->inputIsValid()) {
    KMessageBox::sorry( 0,
        i18n("Please specify a valid end date, for example '%1'.")
        .arg( KGlobal::locale()->formatDate( QDate::currentDate() ) ) );
    return false;
  }

  QDateTime startDt,endDt;
  startDt.setDate(mStartDateEdit->date());
  endDt.setDate(mEndDateEdit->date());
  if (mTimeAssociateButton->isChecked()) {
    startDt.setTime(mStartTimeEdit->getTime());
    endDt.setTime(mEndTimeEdit->getTime());
  }

  if (startDt > endDt) {
    KMessageBox::sorry(0,i18n("The event ends before it starts.\n"
                                 "Please correct dates and times."));
    return false;
  }

  return KOEditorGeneral::validateInput();
}
