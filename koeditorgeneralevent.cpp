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
#include <klocale.h>
#include <kiconloader.h>
#include <kbuttonbox.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kstddirs.h>

#include <libkcal/event.h>

#include "koprefs.h"

#include "koeditorgeneralevent.h"
#include "koeditorgeneralevent.moc"

KOEditorGeneralEvent::KOEditorGeneralEvent(int spacing,QWidget* parent,
                                           const char* name) :
  QWidget( parent, name)
{
  mSpacing = spacing;

//  alarmProgram = "";
  initTimeBox();
  initAlarmBox();
  initMisc();

  initLayout();

  QWidget::setTabOrder(summaryEdit, startDateEdit);
  QWidget::setTabOrder(startDateEdit, startTimeEdit);
  QWidget::setTabOrder(startTimeEdit, endDateEdit);
  QWidget::setTabOrder(endDateEdit, endTimeEdit);
  QWidget::setTabOrder(endTimeEdit, noTimeButton);
  QWidget::setTabOrder(noTimeButton, recursButton);
  QWidget::setTabOrder(recursButton, alarmButton);
  QWidget::setTabOrder(alarmButton, freeTimeCombo);
  QWidget::setTabOrder(freeTimeCombo, descriptionEdit);
  QWidget::setTabOrder(descriptionEdit, categoriesButton);
  QWidget::setTabOrder(categoriesButton, mSecrecyCombo);

  summaryEdit->setFocus();

  // time widgets are checked if they contain a valid time
  connect(startTimeEdit, SIGNAL(timeChanged(QTime)),
	  this, SLOT(startTimeChanged(QTime)));
  connect(endTimeEdit, SIGNAL(timeChanged(QTime)),
	  this, SLOT(endTimeChanged(QTime)));

  // date widgets are checked if they contain a valid date
  connect(startDateEdit, SIGNAL(dateChanged(QDate)),
	  this, SLOT(startDateChanged(QDate)));
  connect(endDateEdit, SIGNAL(dateChanged(QDate)),
	  this, SLOT(endDateChanged(QDate)));

  connect(this,SIGNAL(dateTimesChanged(QDateTime,QDateTime)),
          SLOT(setDuration()));
  connect(this,SIGNAL(dateTimesChanged(QDateTime,QDateTime)),
          SLOT(emitDateTimeStr()));
}

KOEditorGeneralEvent::~KOEditorGeneralEvent()
{
}

void KOEditorGeneralEvent::initTimeBox()
{
  timeGroupBox = new QGroupBox( 1,QGroupBox::Horizontal,
                                i18n("Appointment Time "),this, "User_2" );

  QFrame *timeBoxFrame = new QFrame(timeGroupBox,"TimeBoxFrame");

  QGridLayout *layoutTimeBox = new QGridLayout(timeBoxFrame,2,3);
  layoutTimeBox->setSpacing(mSpacing);


  startDateLabel = new QLabel( timeBoxFrame );
  startDateLabel->setText( i18n("Start Date:") );
  layoutTimeBox->addWidget(startDateLabel,0,0);
  
  startDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(startDateEdit,0,1);

  startTimeLabel = new QLabel( timeBoxFrame, "Label_2" );
  startTimeLabel->setText( i18n("Start Time:") );
  layoutTimeBox->addWidget(startTimeLabel,0,2);
  
  startTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(startTimeEdit,0,3);

  
  endDateLabel = new QLabel( timeBoxFrame, "Label_3" );
  endDateLabel->setText( i18n("End Date:") );
  layoutTimeBox->addWidget(endDateLabel,1,0);

  endDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(endDateEdit,1,1);

  endTimeLabel = new QLabel( timeBoxFrame, "Label_3" );
  endTimeLabel->setText( i18n("End Time:") );
  layoutTimeBox->addWidget(endTimeLabel,1,2);

  endTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(endTimeEdit,1,3);


  noTimeButton = new QCheckBox(timeBoxFrame, "CheckBox_1" );
  noTimeButton->setText( i18n("No time associated") );
  layoutTimeBox->addMultiCellWidget(noTimeButton,2,2,2,3);

  connect(noTimeButton, SIGNAL(toggled(bool)),SLOT(dontAssociateTime(bool)));

  recursButton = new QCheckBox(timeBoxFrame);
  recursButton->setText(i18n("Recurring event"));
  layoutTimeBox->addMultiCellWidget(recursButton,2,2,0,1);

  QObject::connect(recursButton,SIGNAL(toggled(bool)),
                   SIGNAL(recursChanged(bool)));

  durationLabel = new QLabel(timeBoxFrame);
  layoutTimeBox->addMultiCellWidget(durationLabel,0,1,5,5);

  // add stretch space around duration label
  layoutTimeBox->setColStretch(4,1);
  layoutTimeBox->setColStretch(6,1);
}

void KOEditorGeneralEvent::initMisc()
{
  summaryLabel = new QLabel( this, "Label_1" );
  summaryLabel->setText( i18n("Summary:") );

  summaryEdit = new QLineEdit( this, "LineEdit_1" );

  freeTimeLabel = new QLabel( this, "Label_6" );
  freeTimeLabel->setText( i18n("Show Time As:") );

  freeTimeCombo = new QComboBox( false, this, "ComboBox_1" );
  freeTimeCombo->insertItem( i18n("Busy") );
  freeTimeCombo->insertItem( i18n("Free") );

  descriptionEdit = new QMultiLineEdit( this, "MultiLineEdit_1" );
  descriptionEdit->insertLine( "" );
  descriptionEdit->setReadOnly( false );
  descriptionEdit->setOverwriteMode( false );
  descriptionEdit->setWordWrap(QMultiLineEdit::WidgetWidth);

  ownerLabel = new QLabel( this, "Label_7" );
  ownerLabel->setText( i18n("Owner:") );

  mSecrecyLabel = new QLabel("Access:",this);
  mSecrecyCombo = new QComboBox(this);
  mSecrecyCombo->insertStringList(Incidence::secrecyList());

  categoriesButton = new QPushButton( this, "PushButton_6" );
  categoriesButton->setText( i18n("Categories...") );
  connect(categoriesButton,SIGNAL(clicked()),SIGNAL(openCategoryDialog()));

  categoriesLabel = new QLabel( this, "LineEdit_7" );
  categoriesLabel->setFrameStyle(QFrame::Panel|QFrame::Sunken);
}

void KOEditorGeneralEvent::initAlarmBox()
{
  QPixmap pixmap;

  alarmBell = new QLabel(this);
  alarmBell->setPixmap(SmallIcon("bell"));

  alarmButton = new QCheckBox( this, "CheckBox_2" );
  alarmButton->setText( i18n("Reminder:") );

  alarmTimeEdit = new KRestrictedLine( this, "alarmTimeEdit",
				       "1234567890");
  alarmTimeEdit->setText("");

  alarmIncrCombo = new QComboBox(false, this);
  alarmIncrCombo->insertItem(i18n("minute(s)"));
  alarmIncrCombo->insertItem(i18n("hour(s)"));
  alarmIncrCombo->insertItem(i18n("day(s)"));
  alarmIncrCombo->setMinimumHeight(20);

  alarmSoundButton = new QPushButton( this, "PushButton_4" );
  pixmap = SmallIcon("playsound");
  //  alarmSoundButton->setText( i18n("WAV") );
  alarmSoundButton->setPixmap(pixmap);
  alarmSoundButton->setToggleButton(true);
  QToolTip::add(alarmSoundButton, i18n("No sound set"));

  alarmProgramButton = new QPushButton( this, "PushButton_5" );
  pixmap = SmallIcon("runprog");
  //  alarmProgramButton->setText( i18n("PROG") );
  alarmProgramButton->setPixmap(pixmap);
  alarmProgramButton->setToggleButton(true);
  QToolTip::add(alarmProgramButton, i18n("No program set"));

  connect(alarmButton, SIGNAL(toggled(bool)),
	  this, SLOT(alarmStuffEnable(bool)));

  connect(alarmSoundButton, SIGNAL(clicked()),
	  this, SLOT(pickAlarmSound()));
  connect(alarmProgramButton, SIGNAL(clicked()),
	  this, SLOT(pickAlarmProgram()));
}

void KOEditorGeneralEvent::initLayout()
{
  QBoxLayout *layoutTop = new QVBoxLayout(this);
  layoutTop->setSpacing(mSpacing);
  
  layoutTop->addWidget(ownerLabel);

  QBoxLayout *layoutSummary = new QHBoxLayout;
  layoutTop->addLayout(layoutSummary);
  layoutSummary->addWidget(summaryLabel);
  layoutSummary->addWidget(summaryEdit);

  layoutTop->addWidget(timeGroupBox);

  QBoxLayout *layoutAlarmLine = new QHBoxLayout;
  layoutTop->addLayout(layoutAlarmLine);

  QBoxLayout *layoutAlarmBox = new QHBoxLayout;
  layoutAlarmLine->addLayout(layoutAlarmBox);
  layoutAlarmBox->addWidget(alarmBell);
  layoutAlarmBox->addWidget(alarmButton);
  layoutAlarmBox->addWidget(alarmTimeEdit);
  layoutAlarmBox->addWidget(alarmIncrCombo);
  layoutAlarmBox->addWidget(alarmSoundButton);
  layoutAlarmBox->addWidget(alarmProgramButton);

  layoutAlarmLine->addStretch(1);

  QBoxLayout *layoutFreeTime = new QHBoxLayout;
  layoutAlarmLine->addLayout(layoutFreeTime);
  layoutFreeTime->addStretch(1);
  layoutFreeTime->addWidget(freeTimeLabel);
  layoutFreeTime->addWidget(freeTimeCombo);

  layoutTop->addWidget(descriptionEdit,1);

  QBoxLayout *layoutCategories = new QHBoxLayout;
  layoutTop->addLayout(layoutCategories);
  layoutCategories->addWidget(categoriesButton);
  layoutCategories->addWidget(categoriesLabel,1);
  layoutCategories->addWidget(mSecrecyLabel);
  layoutCategories->addWidget(mSecrecyCombo);
}

void KOEditorGeneralEvent::pickAlarmSound()
{
  QString prefix = KGlobal::dirs()->findResourceDir("appdata", "sounds/alert.wav"); 
  if (!alarmSoundButton->isOn()) {
    alarmSound = "";
    QToolTip::remove(alarmSoundButton);
    QToolTip::add(alarmSoundButton, i18n("No sound set"));
  } else {
    QString fileName(KFileDialog::getOpenFileName(prefix,
						  i18n("*.wav|Wav Files"), this));
    if (!fileName.isEmpty()) {
      alarmSound = fileName;
      QToolTip::remove(alarmSoundButton);
      QString dispStr = i18n("Playing '%1'").arg(fileName);
      QToolTip::add(alarmSoundButton, dispStr);
    }
  }
  if (alarmSound.isEmpty())
    alarmSoundButton->setOn(false);
}

void KOEditorGeneralEvent::pickAlarmProgram()
{
  if (!alarmProgramButton->isOn()) {
    alarmProgram = "";
    QToolTip::remove(alarmProgramButton);
    QToolTip::add(alarmProgramButton, i18n("No program set"));
  } else {
    QString fileName(KFileDialog::getOpenFileName(QString::null, QString::null, this));
    if (!fileName.isEmpty()) {
      alarmProgram = fileName;
      QToolTip::remove(alarmProgramButton);
      QString dispStr = i18n("Running '%1'").arg(fileName);
      QToolTip::add(alarmProgramButton, dispStr);
    }
  }
  if (alarmProgram.isEmpty())
    alarmProgramButton->setOn(false);
}

void KOEditorGeneralEvent::timeStuffDisable(bool disable)
{
  if (disable) {
    startTimeLabel->hide();
    endTimeLabel->hide();
    startTimeEdit->hide();
    endTimeEdit->hide();
  } else {
    startTimeLabel->show();
    endTimeLabel->show();
    startTimeEdit->show();
    endTimeEdit->show();
  }
  setDuration();
  emitDateTimeStr();
}

void KOEditorGeneralEvent::alarmStuffEnable(bool enable)
{
  alarmTimeEdit->setEnabled(enable);
  alarmSoundButton->setEnabled(enable);
  alarmProgramButton->setEnabled(enable);
  alarmIncrCombo->setEnabled(enable);
}

void KOEditorGeneralEvent::alarmStuffDisable(bool disable)
{
  alarmStuffEnable(!disable);
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

  startDateEdit->setDate(start.date());
  startTimeEdit->setTime(start.time());
  endDateEdit->setDate(end.date());
  endTimeEdit->setTime(end.time());

  currStartDateTime = start;
  currEndDateTime = end;

  setDuration();
  emitDateTimeStr();
}

void KOEditorGeneralEvent::setCategories(const QString &str)
{
  categoriesLabel->setText(str);
}

void KOEditorGeneralEvent::startTimeChanged(QTime newtime)
{
  kdDebug() << "KOEditorGeneralEvent::startTimeChanged() " << newtime.toString() << endl;

  int secsep = currStartDateTime.secsTo(currEndDateTime);
  
  currStartDateTime.setTime(newtime);

  // adjust end time so that the event has the same duration as before.
  currEndDateTime = currStartDateTime.addSecs(secsep);
  endTimeEdit->setTime(currEndDateTime.time());
  
  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorGeneralEvent::endTimeChanged(QTime newtime)
{
//  kdDebug() << "KOEditorGeneralEvent::endTimeChanged " << newtime.toString() << endl;

  QDateTime newdt(currEndDateTime.date(), newtime);

  if(newdt < currStartDateTime) {
    // oops, can't let that happen.
    newdt = currStartDateTime;
    endTimeEdit->setTime(newdt.time());
  }
  currEndDateTime = newdt;
  
  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorGeneralEvent::startDateChanged(QDate newdate)
{
  int daysep = currStartDateTime.daysTo(currEndDateTime);
  
  currStartDateTime.setDate(newdate);
  
  // adjust end date so that the event has the same duration as before
  currEndDateTime.setDate(currStartDateTime.date().addDays(daysep));
  endDateEdit->setDate(currEndDateTime.date());

  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorGeneralEvent::endDateChanged(QDate newdate)
{
  QDateTime newdt(newdate, currEndDateTime.time());

  if(newdt < currStartDateTime) {
    // oops, we can't let that happen.
    newdt = currStartDateTime;
    endDateEdit->setDate(newdt.date());
    endTimeEdit->setTime(newdt.time());
  }
  currEndDateTime = newdt;

  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorGeneralEvent::setDefaults(QDateTime from,QDateTime to,bool allDay)
{
  ownerLabel->setText(i18n("Owner: ") + KOPrefs::instance()->fullName());

  noTimeButton->setChecked(allDay);
  timeStuffDisable(allDay);
  alarmStuffDisable(allDay);

  setDateTimes(from,to);

  recursButton->setChecked(false);
//  recurStuffEnable(false);

  // TODO: Implement a KPrefsComboItem to solve this in a clean way.
  int alarmTime;
  int a[] = { 1,5,10,15,30 };
  int index = KOPrefs::instance()->mAlarmTime;
  if (index < 0 || index > 4) {
    alarmTime = 0;
  } else {
    alarmTime = a[index];
  }
  alarmTimeEdit->setText(QString::number(alarmTime));
  alarmStuffEnable(false);

  mSecrecyCombo->setCurrentItem(Incidence::SecrecyPublic);
}

void KOEditorGeneralEvent::readEvent(Event *event)
{
  QString tmpStr;
  QDateTime tmpDT;
  int i;

  summaryEdit->setText(event->summary());
  descriptionEdit->setText(event->description());

  // organizer information
  ownerLabel->setText(i18n("Owner: ") + event->organizer());

  // the rest is for the events only
  noTimeButton->setChecked(event->doesFloat());
  timeStuffDisable(event->doesFloat());
  alarmStuffDisable(event->doesFloat());

  setDateTimes(event->dtStart(),event->dtEnd());

  recursButton->setChecked(event->recurrence()->doesRecur());
//  recurStuffEnable(event->doesRecur());

  mSecrecyCombo->setCurrentItem(event->secrecy());

  // set up alarm stuff
  alarmButton->setChecked(event->alarm()->enabled());
  if (alarmButton->isChecked()) {
    alarmStuffEnable(true);
    tmpDT = event->alarm()->time();
    if (tmpDT.isValid()) {
      i = tmpDT.secsTo(currStartDateTime);
      i = i / 60; // make minutes
      if (i % 60 == 0) { // divides evenly into hours?
        i = i / 60;
        alarmIncrCombo->setCurrentItem(1);
      }
      if (i % 24 == 0) { // divides evenly into days?
        i = i / 24;
        alarmIncrCombo->setCurrentItem(2);
      }
    } else {
      i = 5;
    }
    alarmTimeEdit->setText(QString::number(i));

    if (!event->alarm()->programFile().isEmpty()) {
      alarmProgram = event->alarm()->programFile();
      alarmProgramButton->setOn(true);
      QString dispStr = i18n("Running '%1'").arg(alarmProgram);
      QToolTip::add(alarmProgramButton, dispStr);
    }
    if (!event->alarm()->audioFile().isEmpty()) {
      alarmSound = event->alarm()->audioFile();
      alarmSoundButton->setOn(true);
      QString dispStr = i18n("Playing '%1'").arg(alarmSound);
      QToolTip::add(alarmSoundButton, dispStr);
    }
  } else {
    alarmStuffEnable(false);
  }

  if (event->transparency() > 0)
    freeTimeCombo->setCurrentItem(1);
  // else it is implicitly 0 (i.e. busy)

  setCategories(event->categoriesStr());
}

void KOEditorGeneralEvent::writeEvent(Event *event)
{
//  kdDebug() << "KOEditorGeneralEvent::writeEvent()" << endl;

  QDate tmpDate;
  QTime tmpTime;
  QDateTime tmpDT;

  // temp. until something better happens.
  QString tmpStr;
  int j;

  event->setSummary(summaryEdit->text());
  event->setDescription(descriptionEdit->text());
  event->setCategories(categoriesLabel->text());
  event->setSecrecy(mSecrecyCombo->currentItem());
  
  if (noTimeButton->isChecked()) {
    event->setFloats(true);
    // need to change this.
    tmpDate = startDateEdit->getDate();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtStart(tmpDT);

    tmpDate = endDateEdit->getDate();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtEnd(tmpDT);
  } else {
    event->setFloats(false);

    // set date/time end
    tmpDate = endDateEdit->getDate();
    tmpTime = endTimeEdit->getTime();
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtEnd(tmpDT);

    // set date/time start
    tmpDate = startDateEdit->getDate();
    tmpTime = startTimeEdit->getTime();
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtStart(tmpDT);
  } // check for float

  // alarm stuff
  if (alarmButton->isChecked()) {
    event->alarm()->setEnabled(true);
    tmpStr = alarmTimeEdit->text();
    j = tmpStr.toInt() * -60;
    if (alarmIncrCombo->currentItem() == 1)
      j = j * 60;
    else if (alarmIncrCombo->currentItem() == 2)
      j = j * (60 * 24);

    tmpDT = event->dtStart();
    tmpDT = tmpDT.addSecs(j);
    event->alarm()->setTime(tmpDT);
    if (!alarmProgram.isEmpty() && alarmProgramButton->isOn())
      event->alarm()->setProgramFile(alarmProgram);
    else
      event->alarm()->setProgramFile("");
    if (!alarmSound.isEmpty() && alarmSoundButton->isOn())
      event->alarm()->setAudioFile(alarmSound);
    else
      event->alarm()->setAudioFile("");
  } else {
    event->alarm()->setEnabled(false);
    event->alarm()->setProgramFile("");
    event->alarm()->setAudioFile("");
  }
  
  // note, that if on the details tab the "Transparency" option is implemented,
  // we will have to change this to suit.
  event->setTransparency(freeTimeCombo->currentItem());

//  kdDebug() << "KOEditorGeneralEvent::writeEvent() done" << endl;
}

void KOEditorGeneralEvent::setDuration()
{
  QString tmpStr, catStr;
  int hourdiff, minutediff;

  if (noTimeButton->isChecked()) {
    int daydiff = currStartDateTime.date().daysTo(currEndDateTime.date()) + 1;
    if (daydiff == 1) tmpStr = i18n("Duration: 1 day");
    else tmpStr = i18n("Duration: %1 days").arg(daydiff);
  } else {
    hourdiff = currStartDateTime.date().daysTo(currEndDateTime.date()) * 24;
    hourdiff += currEndDateTime.time().hour() - 
      currStartDateTime.time().hour();
    minutediff = currEndDateTime.time().minute() -
      currStartDateTime.time().minute();
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
  durationLabel->setText(tmpStr);
}

void KOEditorGeneralEvent::emitDateTimeStr()
{
  KLocale *l = KGlobal::locale();
  
  QString from,to;
  if (noTimeButton->isChecked()) {
    from = l->formatDate(currStartDateTime.date());
    to = l->formatDate(currEndDateTime.date());
  } else {
    from = l->formatDateTime(currStartDateTime);
    to = l->formatDateTime(currEndDateTime);
  }
  
  QString str = i18n("From: %1   To: %2   %3").arg(from).arg(to)
                .arg(durationLabel->text());
//  QString str = i18n("<b>From:</b> %1  <b>To:</b> %2, %3").arg(from).arg(to)
//                .arg(durationLabel->text());
                 
  emit dateTimeStrChanged(str);
}

bool KOEditorGeneralEvent::validateInput()
{
//  kdDebug() << "KOEditorGeneralEvent::validateInput()" << endl;

  if (!noTimeButton->isChecked()) {
    if (!startTimeEdit->inputIsValid()) {
      KMessageBox::sorry(this,i18n("Please specify a valid start time."));
      return false;
    }

    if (!endTimeEdit->inputIsValid()) {
      KMessageBox::sorry(this,i18n("Please specify a valid end time."));
      return false;
    }
  }

  if (!startDateEdit->inputIsValid()) {
    KMessageBox::sorry(this,i18n("Please specify a valid start date."));
    return false;
  }

  if (!endDateEdit->inputIsValid()) {
    KMessageBox::sorry(this,i18n("Please specify a valid end date."));
    return false;
  }

  QDateTime startDt,endDt;
  startDt.setDate(startDateEdit->getDate());
  endDt.setDate(endDateEdit->getDate());
  if (!noTimeButton->isChecked()) {
    startDt.setTime(startTimeEdit->getTime());
    endDt.setTime(endTimeEdit->getTime());
  }

  if (startDt > endDt) {
    KMessageBox::sorry(this,i18n("The event ends before it starts.\n"
                                 "Please correct dates and times."));
    return false;
  }

  return true;
}
