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

#include <qwidget.h>
#include <qtooltip.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>

#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>

#include <libkcal/event.h>

#include "koprefs.h"

#include "koeditorgeneral.h"
#include "koeditorgeneral.moc"

KOEditorGeneral::KOEditorGeneral(QWidget* parent, const char* name) :
  QWidget( parent, name)
{
}

KOEditorGeneral::~KOEditorGeneral()
{
}

QBoxLayout *KOEditorGeneral::initHeader()
{
  QBoxLayout *headerLayout = new QVBoxLayout;

  mOwnerLabel = new QLabel(i18n("Owner:"),this);
  headerLayout->addWidget(mOwnerLabel);

  QBoxLayout *summaryLayout = new QHBoxLayout;
  headerLayout->addLayout(summaryLayout);

  QLabel *summaryLabel = new QLabel(i18n("Summary:"),this);
  summaryLayout->addWidget(summaryLabel);

  mSummaryEdit = new QLineEdit(this);
  summaryLayout->addWidget(mSummaryEdit);

  return headerLayout;
}


QBoxLayout *KOEditorGeneral::initDescription()
{
  QBoxLayout *descriptionLayout = new QVBoxLayout;

  mDescriptionEdit = new QMultiLineEdit(this);
  mDescriptionEdit->insertLine("");
  mDescriptionEdit->setReadOnly(false);
  mDescriptionEdit->setOverwriteMode(false);
  mDescriptionEdit->setWordWrap(QMultiLineEdit::WidgetWidth);
  descriptionLayout->addWidget(mDescriptionEdit);

  QBoxLayout *detailsLayout = new QHBoxLayout;
  descriptionLayout->addLayout(detailsLayout);

  mCategoriesButton = new QPushButton(this);
  mCategoriesButton->setText(i18n("Categories..."));
  connect(mCategoriesButton,SIGNAL(clicked()),SIGNAL(openCategoryDialog()));
  detailsLayout->addWidget(mCategoriesButton);

  mCategoriesLabel = new QLabel(this);
  mCategoriesLabel->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  detailsLayout->addWidget(mCategoriesLabel);

  QLabel *secrecyLabel = new QLabel("Access:",this);
  detailsLayout->addWidget(secrecyLabel);

  mSecrecyCombo = new QComboBox(this);
  mSecrecyCombo->insertStringList(Incidence::secrecyList());
  detailsLayout->addWidget(mSecrecyCombo);

  return descriptionLayout;
}

QBoxLayout *KOEditorGeneral::initAlarm()
{
  QBoxLayout *alarmLayout = new QHBoxLayout;

  QLabel *alarmBell = new QLabel(this);
  alarmBell->setPixmap(SmallIcon("bell"));
  alarmLayout->addWidget(alarmBell);

  mAlarmButton = new QCheckBox(i18n("Reminder:"),this);
  connect(mAlarmButton, SIGNAL(toggled(bool)), SLOT(alarmStuffEnable(bool)));
  alarmLayout->addWidget(mAlarmButton);

  mAlarmTimeEdit = new KRestrictedLine(this, "alarmTimeEdit",
				       "1234567890");
  mAlarmTimeEdit->setText("");
  alarmLayout->addWidget(mAlarmTimeEdit);

  mAlarmIncrCombo = new QComboBox(false, this);
  mAlarmIncrCombo->insertItem(i18n("minute(s)"));
  mAlarmIncrCombo->insertItem(i18n("hour(s)"));
  mAlarmIncrCombo->insertItem(i18n("day(s)"));
//  mAlarmIncrCombo->setMinimumHeight(20);
  alarmLayout->addWidget(mAlarmIncrCombo);

  mAlarmSoundButton = new QPushButton(this);
  mAlarmSoundButton->setPixmap(SmallIcon("playsound"));
  mAlarmSoundButton->setToggleButton(true);
  QToolTip::add(mAlarmSoundButton, i18n("No sound set"));
  connect(mAlarmSoundButton, SIGNAL(clicked()), SLOT(pickAlarmSound()));
  alarmLayout->addWidget(mAlarmSoundButton);

  mAlarmProgramButton = new QPushButton(this);
  mAlarmProgramButton->setPixmap(SmallIcon("runprog"));
  mAlarmProgramButton->setToggleButton(true);
  QToolTip::add(mAlarmProgramButton, i18n("No program set"));
  connect(mAlarmProgramButton, SIGNAL(clicked()), SLOT(pickAlarmProgram()));
  alarmLayout->addWidget(mAlarmProgramButton);

  return alarmLayout;
}

void KOEditorGeneral::pickAlarmSound()
{
  QString prefix = KGlobal::dirs()->findResourceDir("appdata", "sounds/alert.wav");
  if (!mAlarmSoundButton->isOn()) {
    mAlarmSound = "";
    QToolTip::remove(mAlarmSoundButton);
    QToolTip::add(mAlarmSoundButton, i18n("No sound set"));
  } else {
    QString fileName(KFileDialog::getOpenFileName(prefix,
						  i18n("*.wav|Wav Files"), this));
    if (!fileName.isEmpty()) {
      mAlarmSound = fileName;
      QToolTip::remove(mAlarmSoundButton);
      QString dispStr = i18n("Playing '%1'").arg(fileName);
      QToolTip::add(mAlarmSoundButton, dispStr);
    }
  }
  if (mAlarmSound.isEmpty())
    mAlarmSoundButton->setOn(false);
}

void KOEditorGeneral::pickAlarmProgram()
{
  if (!mAlarmProgramButton->isOn()) {
    mAlarmProgram = "";
    QToolTip::remove(mAlarmProgramButton);
    QToolTip::add(mAlarmProgramButton, i18n("No program set"));
  } else {
    QString fileName(KFileDialog::getOpenFileName(QString::null, QString::null, this));
    if (!fileName.isEmpty()) {
      mAlarmProgram = fileName;
      QToolTip::remove(mAlarmProgramButton);
      QString dispStr = i18n("Running '%1'").arg(fileName);
      QToolTip::add(mAlarmProgramButton, dispStr);
    }
  }
  if (mAlarmProgram.isEmpty())
    mAlarmProgramButton->setOn(false);
}

void KOEditorGeneral::alarmStuffEnable(bool enable)
{
  mAlarmTimeEdit->setEnabled(enable);
  mAlarmSoundButton->setEnabled(enable);
  mAlarmProgramButton->setEnabled(enable);
  mAlarmIncrCombo->setEnabled(enable);
}

void KOEditorGeneral::alarmStuffDisable(bool disable)
{
  alarmStuffEnable(!disable);
}

void KOEditorGeneral::setCategories(const QString &str)
{
  mCategoriesLabel->setText(str);
}

void KOEditorGeneral::setDefaults(bool allDay)
{
  mOwnerLabel->setText(i18n("Owner: ") + KOPrefs::instance()->fullName());

  alarmStuffDisable(allDay);

  // TODO: Implement a KPrefsComboItem to solve this in a clean way.
  int alarmTime;
  int a[] = { 1,5,10,15,30 };
  int index = KOPrefs::instance()->mAlarmTime;
  if (index < 0 || index > 4) {
    alarmTime = 0;
  } else {
    alarmTime = a[index];
  }
  mAlarmTimeEdit->setText(QString::number(alarmTime));
  alarmStuffEnable(false);

  mSecrecyCombo->setCurrentItem(Incidence::SecrecyPublic);
}

void KOEditorGeneral::readIncidence(Incidence *event)
{
  QString tmpStr;
  QDateTime tmpDT;
  int i;

  mSummaryEdit->setText(event->summary());
  mDescriptionEdit->setText(event->description());

  // organizer information
  mOwnerLabel->setText(i18n("Owner: ") + event->organizer());

  alarmStuffDisable(event->doesFloat());

  mSecrecyCombo->setCurrentItem(event->secrecy());

  // set up alarm stuff
  const Alarm* alarm;
  for (QPtrListIterator<Alarm> it(event->alarms());
       (alarm = it.current()) != 0;  ++it) {
    mAlarmButton->setChecked(alarm->enabled());
    if (mAlarmButton->isChecked()) {
      alarmStuffEnable(true);
      tmpDT = alarm->time();
      if (tmpDT.isValid()) {
        i = tmpDT.secsTo(event->dtStart());
        i = i / 60; // make minutes
        if (i % 60 == 0) { // divides evenly into hours?
          i = i / 60;
          mAlarmIncrCombo->setCurrentItem(1);
        }
        if (i % 24 == 0) { // divides evenly into days?
          i = i / 24;
          mAlarmIncrCombo->setCurrentItem(2);
        }
      } else {
        i = 5;
      }
      mAlarmTimeEdit->setText(QString::number(i));

      if (!alarm->programFile().isEmpty()) {
        mAlarmProgram = alarm->programFile();
        mAlarmProgramButton->setOn(true);
        QString dispStr = i18n("Running '%1'").arg(mAlarmProgram);
        QToolTip::add(mAlarmProgramButton, dispStr);
      }
      if (!alarm->audioFile().isEmpty()) {
        mAlarmSound = alarm->audioFile();
        mAlarmSoundButton->setOn(true);
        QString dispStr = i18n("Playing '%1'").arg(mAlarmSound);
        QToolTip::add(mAlarmSoundButton, dispStr);
      }
    } else {
      alarmStuffEnable(false);
    }
// TODO: Deal with multiple alarms
    break; // For now, stop after the first alarm
  }

  setCategories(event->categoriesStr());
}

void KOEditorGeneral::writeIncidence(Incidence *event)
{
//  kdDebug() << "KOEditorGeneral::writeEvent()" << endl;

  QDateTime tmpDT;

  // temp. until something better happens.
  QString tmpStr;
  int j;

  event->setSummary(mSummaryEdit->text());
  event->setDescription(mDescriptionEdit->text());
  event->setCategories(mCategoriesLabel->text());
  event->setSecrecy(mSecrecyCombo->currentItem());

  // alarm stuff
  Alarm* alarm;
  for (QPtrListIterator<Alarm> it(event->alarms());
       (alarm = it.current()) != 0;  ++it) {
    if (mAlarmButton->isChecked()) {
      alarm->setEnabled(true);
      tmpStr = mAlarmTimeEdit->text();
      j = tmpStr.toInt() * -60;
      if (mAlarmIncrCombo->currentItem() == 1)
        j = j * 60;
      else if (mAlarmIncrCombo->currentItem() == 2)
        j = j * (60 * 24);

      tmpDT = event->dtStart();
      tmpDT = tmpDT.addSecs(j);
      alarm->setTime(tmpDT);
      if (!mAlarmProgram.isEmpty() && mAlarmProgramButton->isOn())
        alarm->setProgramFile(mAlarmProgram);
      else
        alarm->setProgramFile("");
      if (!mAlarmSound.isEmpty() && mAlarmSoundButton->isOn())
        alarm->setAudioFile(mAlarmSound);
      else
        alarm->setAudioFile("");
    } else {
      alarm->setEnabled(false);
      alarm->setProgramFile("");
      alarm->setAudioFile("");
    }
// TODO: Deal with multiple alarms
    break; // For now, stop after the first alarm
  }
}
