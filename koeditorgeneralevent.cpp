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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$

#include <qtooltip.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>

#include <libkcal/event.h>

#include "koprefs.h"

#include "koeditorgeneralevent.h"
#include "koeditorgeneralevent.moc"

KOEditorGeneralEvent::KOEditorGeneralEvent(int spacing,QWidget* parent,
                                           const char* name) :
  KOEditorGeneral( parent, name)
{
  mSpacing = spacing;

  QBoxLayout *topLayout = new QVBoxLayout(this);
  topLayout->addLayout(initHeader());
  topLayout->addLayout(initTime());
  QBoxLayout *alarmLineLayout = new QHBoxLayout(topLayout);
  alarmLineLayout->addLayout(initAlarm());
  alarmLineLayout->addLayout(initClass());
  topLayout->addLayout(initDescription());

  QWidget::setTabOrder(mSummaryEdit, mStartDateEdit);
  QWidget::setTabOrder(mStartDateEdit, mStartTimeEdit);
  QWidget::setTabOrder(mStartTimeEdit, mEndDateEdit);
  QWidget::setTabOrder(mEndDateEdit, mEndTimeEdit);
  QWidget::setTabOrder(mEndTimeEdit, mNoTimeButton);
  QWidget::setTabOrder(mNoTimeButton, mRecursButton);
  QWidget::setTabOrder(mRecursButton, mAlarmButton);
  QWidget::setTabOrder(mAlarmButton, mFreeTimeCombo);
  QWidget::setTabOrder(mFreeTimeCombo, mDescriptionEdit);
  QWidget::setTabOrder(mDescriptionEdit, mCategoriesButton);
  QWidget::setTabOrder(mCategoriesButton, mSecrecyCombo);

  mSummaryEdit->setFocus();

  connect(this,SIGNAL(dateTimesChanged(QDateTime,QDateTime)),
          SLOT(setDuration()));
  connect(this,SIGNAL(dateTimesChanged(QDateTime,QDateTime)),
          SLOT(emitDateTimeStr()));
}

KOEditorGeneralEvent::~KOEditorGeneralEvent()
{
}

QBoxLayout *KOEditorGeneralEvent::initTime()
{
  QBoxLayout *timeLayout = new QVBoxLayout;

  QGroupBox *timeGroupBox = new QGroupBox(1,QGroupBox::Horizontal,
                                          i18n("Appointment Time "),this);
  timeLayout->addWidget(timeGroupBox);

  QFrame *timeBoxFrame = new QFrame(timeGroupBox);

  QGridLayout *layoutTimeBox = new QGridLayout(timeBoxFrame,2,3);
  layoutTimeBox->setSpacing(mSpacing);


  mStartDateLabel = new QLabel(i18n("Start Date:"),timeBoxFrame);
  layoutTimeBox->addWidget(mStartDateLabel,0,0);
  
  mStartDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mStartDateEdit,0,1);

  mStartTimeLabel = new QLabel(i18n("Start Time:"),timeBoxFrame);
  layoutTimeBox->addWidget(mStartTimeLabel,0,2);
  
  mStartTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mStartTimeEdit,0,3);


  mEndDateLabel = new QLabel(i18n("End Date:"),timeBoxFrame);
  layoutTimeBox->addWidget(mEndDateLabel,1,0);

  mEndDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mEndDateEdit,1,1);

  mEndTimeLabel = new QLabel(i18n("End Time:"),timeBoxFrame);
  layoutTimeBox->addWidget(mEndTimeLabel,1,2);

  mEndTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mEndTimeEdit,1,3);


  mNoTimeButton = new QCheckBox(i18n("No time associated"),timeBoxFrame);
  connect(mNoTimeButton, SIGNAL(toggled(bool)),SLOT(dontAssociateTime(bool)));
  layoutTimeBox->addMultiCellWidget(mNoTimeButton,2,2,2,3);

  mRecursButton = new QCheckBox(i18n("Recurring event"),timeBoxFrame);
  connect(mRecursButton,SIGNAL(toggled(bool)),SIGNAL(recursChanged(bool)));
  layoutTimeBox->addMultiCellWidget(mRecursButton,2,2,0,1);

  mDurationLabel = new QLabel(timeBoxFrame);
  layoutTimeBox->addMultiCellWidget(mDurationLabel,0,1,5,5);

  // add stretch space around duration label
  layoutTimeBox->setColStretch(4,1);
  layoutTimeBox->setColStretch(6,1);

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

  return timeLayout;  
}

QBoxLayout *KOEditorGeneralEvent::initClass()
{
  QBoxLayout *classLayout = new QHBoxLayout(this);

  QLabel *freeTimeLabel = new QLabel(i18n("Show Time As:"),this);
  classLayout->addWidget(freeTimeLabel);

  mFreeTimeCombo = new QComboBox(false, this);
  mFreeTimeCombo->insertItem(i18n("Busy"));
  mFreeTimeCombo->insertItem(i18n("Free"));
  classLayout->addWidget(mFreeTimeCombo);

  return classLayout;
}

void KOEditorGeneralEvent::timeStuffDisable(bool disable)
{
  if (disable) {
    mStartTimeLabel->hide();
    mEndTimeLabel->hide();
    mStartTimeEdit->hide();
    mEndTimeEdit->hide();
  } else {
    mStartTimeLabel->show();
    mEndTimeLabel->show();
    mStartTimeEdit->show();
    mEndTimeEdit->show();
  }
  setDuration();
  emitDateTimeStr();
}

void KOEditorGeneralEvent::dontAssociateTime(bool noTime)
{
  timeStuffDisable(noTime);
  //if(alarmButton->isChecked()) alarmStuffDisable(noTime);
  allDayChanged(noTime);
}
		
void KOEditorGeneralEvent::setDateTimes(QDateTime start, QDateTime end)
{
//  kdDebug() << "KOEditorGeneralEvent::setDateTimes(): Start DateTime: " << start.toString() << endl;

  mStartDateEdit->setDate(start.date());
  mStartTimeEdit->setTime(start.time());
  mEndDateEdit->setDate(end.date());
  mEndTimeEdit->setTime(end.time());

  mCurrStartDateTime = start;
  mCurrEndDateTime = end;

  setDuration();
  emitDateTimeStr();
}

void KOEditorGeneralEvent::startTimeChanged(QTime newtime)
{
  kdDebug() << "KOEditorGeneralEvent::startTimeChanged() " << newtime.toString() << endl;

  int secsep = mCurrStartDateTime.secsTo(mCurrEndDateTime);
  
  mCurrStartDateTime.setTime(newtime);

  // adjust end time so that the event has the same duration as before.
  mCurrEndDateTime = mCurrStartDateTime.addSecs(secsep);
  mEndTimeEdit->setTime(mCurrEndDateTime.time());
  
  emit dateTimesChanged(mCurrStartDateTime,mCurrEndDateTime);
}

void KOEditorGeneralEvent::endTimeChanged(QTime newtime)
{
//  kdDebug() << "KOEditorGeneralEvent::endTimeChanged " << newtime.toString() << endl;

  QDateTime newdt(mCurrEndDateTime.date(), newtime);

  if(newdt < mCurrStartDateTime) {
    // oops, can't let that happen.
    newdt = mCurrStartDateTime;
    mEndTimeEdit->setTime(newdt.time());
  }
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

  mNoTimeButton->setChecked(allDay);
  timeStuffDisable(allDay);

  setDateTimes(from,to);

  mRecursButton->setChecked(false);
}

void KOEditorGeneralEvent::readEvent(Event *event)
{
  QString tmpStr;

  // the rest is for the events only
  mNoTimeButton->setChecked(event->doesFloat());
  timeStuffDisable(event->doesFloat());

  setDateTimes(event->dtStart(),event->dtEnd());

  mRecursButton->setChecked(event->recurrence()->doesRecur());

  if (event->transparency() > 0)
    mFreeTimeCombo->setCurrentItem(1);
  // else it is implicitly 0 (i.e. busy)

  readIncidence(event);  
}

void KOEditorGeneralEvent::writeEvent(Event *event)
{
//  kdDebug() << "KOEditorGeneralEvent::writeEvent()" << endl;

  writeIncidence(event);

  QDate tmpDate;
  QTime tmpTime;
  QDateTime tmpDT;

  // temp. until something better happens.
  QString tmpStr;

  if (mNoTimeButton->isChecked()) {
    event->setFloats(true);
    // need to change this.
    tmpDate = mStartDateEdit->getDate();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtStart(tmpDT);

    tmpDate = mEndDateEdit->getDate();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtEnd(tmpDT);
  } else {
    event->setFloats(false);

    // set date/time end
    tmpDate = mEndDateEdit->getDate();
    tmpTime = mEndTimeEdit->getTime();
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtEnd(tmpDT);

    // set date/time start
    tmpDate = mStartDateEdit->getDate();
    tmpTime = mStartTimeEdit->getTime();
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtStart(tmpDT);
  } // check for float
  
  event->setTransparency(mFreeTimeCombo->currentItem());

//  kdDebug() << "KOEditorGeneralEvent::writeEvent() done" << endl;
}

void KOEditorGeneralEvent::setDuration()
{
  QString tmpStr, catStr;
  int hourdiff, minutediff;

  if (mNoTimeButton->isChecked()) {
    int daydiff = mCurrStartDateTime.date().daysTo(mCurrEndDateTime.date()) + 1;
    if (daydiff == 1) tmpStr = i18n("Duration: 1 day");
    else tmpStr = i18n("Duration: %1 days").arg(daydiff);
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
        if (hourdiff > 1)
          catStr = i18n("%1 hours").arg(QString::number(hourdiff));
        else if (hourdiff == 1)
          catStr = i18n("%1 hour").arg(QString::number(hourdiff));
        tmpStr.append(catStr);
      }
      if (hourdiff && minutediff){
        tmpStr += i18n(", ");
      }
      if (minutediff){
        if (minutediff > 1)
          catStr = i18n("%1 minutes").arg(QString::number(minutediff));
        else if (minutediff == 1)
          catStr = i18n("%1 minute").arg(QString::number(minutediff));
        tmpStr += catStr;
      }
    } else tmpStr = "";
  }
  mDurationLabel->setText(tmpStr);
}

void KOEditorGeneralEvent::emitDateTimeStr()
{
  KLocale *l = KGlobal::locale();
  
  QString from,to;
  if (mNoTimeButton->isChecked()) {
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
//  kdDebug() << "KOEditorGeneralEvent::validateInput()" << endl;

  if (!mNoTimeButton->isChecked()) {
    if (!mStartTimeEdit->inputIsValid()) {
      KMessageBox::sorry(this,i18n("Please specify a valid start time."));
      return false;
    }

    if (!mEndTimeEdit->inputIsValid()) {
      KMessageBox::sorry(this,i18n("Please specify a valid end time."));
      return false;
    }
  }

  if (!mStartDateEdit->inputIsValid()) {
    KMessageBox::sorry(this,i18n("Please specify a valid start date."));
    return false;
  }

  if (!mEndDateEdit->inputIsValid()) {
    KMessageBox::sorry(this,i18n("Please specify a valid end date."));
    return false;
  }

  QDateTime startDt,endDt;
  startDt.setDate(mStartDateEdit->getDate());
  endDt.setDate(mEndDateEdit->getDate());
  if (!mNoTimeButton->isChecked()) {
    startDt.setTime(mStartTimeEdit->getTime());
    endDt.setTime(mEndTimeEdit->getTime());
  }

  if (startDt > endDt) {
    KMessageBox::sorry(this,i18n("The event ends before it starts.\n"
                                 "Please correct dates and times."));
    return false;
  }

  return KOEditorGeneral::validateInput();
}
