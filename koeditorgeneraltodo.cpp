/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
#include <qfiledialog.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <krestrictedline.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

#include <libkcal/todo.h>

#include "koprefs.h"

#include "koeditorgeneraltodo.h"
#include "koeditorgeneraltodo.moc"

KOEditorGeneralTodo::KOEditorGeneralTodo(int spacing,QWidget* parent,
                                         const char* name)
  : KOEditorGeneral( parent, name)
{
  mSpacing = spacing;

  QBoxLayout *topLayout = new QVBoxLayout(this);
  topLayout->setSpacing(mSpacing);
  initHeader(topLayout);
  initTime(topLayout);
  initStatus(topLayout);
  QBoxLayout *alarmLineLayout = new QHBoxLayout(topLayout);
  initAlarm(alarmLineLayout);
  initDescription(topLayout);

  QWidget::setTabOrder(mSummaryEdit, mCompletedCombo);
  QWidget::setTabOrder(mCompletedCombo, mPriorityCombo);
  QWidget::setTabOrder(mPriorityCombo, mDescriptionEdit);
  QWidget::setTabOrder(mDescriptionEdit, mCategoriesButton);
  QWidget::setTabOrder(mCategoriesButton, mSecrecyCombo);
  QWidget::setTabOrder(mSecrecyCombo, mAlarmButton);
  
  mSummaryEdit->setFocus();
}

KOEditorGeneralTodo::~KOEditorGeneralTodo()
{
}

void KOEditorGeneralTodo::initTime(QBoxLayout *topLayout)
{
  QBoxLayout *timeLayout = new QVBoxLayout(topLayout);

  QGroupBox *timeGroupBox = new QGroupBox(1,QGroupBox::Horizontal,
                                          i18n("Due Date "),this);
  timeLayout->addWidget(timeGroupBox);

  QFrame *timeBoxFrame = new QFrame(timeGroupBox);

  QGridLayout *layoutTimeBox = new QGridLayout(timeBoxFrame,1,1);
  layoutTimeBox->setSpacing(mSpacing);

  mNoDueCheck = new QCheckBox(i18n("No due date"),timeBoxFrame);
  layoutTimeBox->addWidget(mNoDueCheck,0,0);
  connect(mNoDueCheck,SIGNAL(toggled(bool)),SLOT(dueStuffDisable(bool)));

  mDueLabel = new QLabel(i18n("Due Date:"),timeBoxFrame);
  layoutTimeBox->addWidget(mDueLabel,1,0);
  
  mDueDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mDueDateEdit,1,1);

  mDueTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mDueTimeEdit,1,2);


  mNoStartCheck = new QCheckBox(i18n("No start date"),timeBoxFrame);
  layoutTimeBox->addWidget(mNoStartCheck,2,0);
  connect(mNoStartCheck,SIGNAL(toggled(bool)),SLOT(startStuffDisable(bool)));

  mStartLabel = new QLabel(i18n("Start Date:"),timeBoxFrame);
  layoutTimeBox->addWidget(mStartLabel,3,0);
  
  mStartDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mStartDateEdit,3,1);

  mStartTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mStartTimeEdit,3,2);


  mNoTimeButton = new QCheckBox(i18n("No time associated"),timeBoxFrame);
  layoutTimeBox->addWidget(mNoTimeButton,0,4);

  connect(mNoTimeButton,SIGNAL(toggled(bool)),SLOT(timeStuffDisable(bool)));
  
  // some more layouting
  layoutTimeBox->setColStretch(3,1);
}


void KOEditorGeneralTodo::initStatus(QBoxLayout *topLayout)
{
  QBoxLayout *statusLayout = new QHBoxLayout(topLayout);

  mCompletedCombo = new QComboBox(this);
  // xgettext:no-c-format
  mCompletedCombo->insertItem(i18n("0 %"));
  // xgettext:no-c-format
  mCompletedCombo->insertItem(i18n("20 %"));
  // xgettext:no-c-format
  mCompletedCombo->insertItem(i18n("40 %"));
  // xgettext:no-c-format
  mCompletedCombo->insertItem(i18n("60 %"));
  // xgettext:no-c-format
  mCompletedCombo->insertItem(i18n("80 %"));
  // xgettext:no-c-format
  mCompletedCombo->insertItem(i18n("100 %"));
  connect(mCompletedCombo,SIGNAL(activated(int)),SLOT(completedChanged(int)));
  statusLayout->addWidget(mCompletedCombo);

  statusLayout->addStretch(1);

  mCompletedLabel = new QLabel(i18n("completed"),this);
  statusLayout->addWidget(mCompletedLabel);

  QLabel *priorityLabel = new QLabel(i18n("Priority:"),this);
  statusLayout->addWidget(priorityLabel);

  mPriorityCombo = new QComboBox(this);
  mPriorityCombo->insertItem(i18n("1 (Highest)"));
  mPriorityCombo->insertItem(i18n("2"));
  mPriorityCombo->insertItem(i18n("3"));
  mPriorityCombo->insertItem(i18n("4"));
  mPriorityCombo->insertItem(i18n("5 (lowest)"));
  statusLayout->addWidget(mPriorityCombo);
}


void KOEditorGeneralTodo::setDefaults(QDateTime due,bool allDay)
{
  KOEditorGeneral::setDefaults(allDay);

  mNoTimeButton->setChecked(allDay);
  timeStuffDisable(allDay);
  
  mNoDueCheck->setChecked(true);
  dueStuffDisable(true);

  mNoStartCheck->setChecked(true);
  startStuffDisable(true);

  mDueDateEdit->setDate(due.date());
  mDueTimeEdit->setTime(due.time());
  
  mStartDateEdit->setDate(QDate::currentDate());
  mStartTimeEdit->setTime(QTime::currentTime());  

  mPriorityCombo->setCurrentItem(2);
  
  mCompletedCombo->setCurrentItem(0);
}

void KOEditorGeneralTodo::readTodo(Todo *todo)
{
  KOEditorGeneral::readIncidence(todo);

  QDateTime dueDT;
  
  if (todo->hasDueDate()) {
    dueDT = todo->dtDue();
    mDueDateEdit->setDate(todo->dtDue().date());
    mDueTimeEdit->setTime(todo->dtDue().time());
    mNoDueCheck->setChecked(false);
  } else {
    mDueDateEdit->setDate(QDate::currentDate());
    mDueTimeEdit->setTime(QTime::currentTime());
    mNoDueCheck->setChecked(true);
  }

  if (todo->hasStartDate()) {
    mStartDateEdit->setDate(todo->dtStart().date());
    mStartTimeEdit->setTime(todo->dtStart().time());
    mNoStartCheck->setChecked(false);
  } else {
    mStartDateEdit->setDate(QDate::currentDate());
    mStartTimeEdit->setTime(QTime::currentTime());
    mNoStartCheck->setChecked(true);
  }

  mNoTimeButton->setChecked(todo->doesFloat());

  mCompletedCombo->setCurrentItem(todo->percentComplete() / 20);
  if (todo->isCompleted() && todo->hasCompletedDate()) {
    mCompleted = todo->completed();
  }
  setCompletedDate();

  mPriorityCombo->setCurrentItem(todo->priority()-1);
}

void KOEditorGeneralTodo::writeTodo(Todo *todo)
{
  KOEditorGeneral::writeIncidence(todo);

  // temp. until something better happens.
  QString tmpStr;
  
  todo->setHasDueDate(!mNoDueCheck->isChecked());
  todo->setHasStartDate(!mNoStartCheck->isChecked());

  QDate tmpDate;
  QTime tmpTime;
  QDateTime tmpDT;
  if (mNoTimeButton->isChecked()) {
    todo->setFloats(true);

    // need to change this.
    tmpDate = mDueDateEdit->getDate();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    todo->setDtDue(tmpDT);
    
    tmpDate = mStartDateEdit->getDate();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    todo->setDtStart(tmpDT);
  } else {
    todo->setFloats(false);
    
    // set due date/time
    tmpDate = mDueDateEdit->getDate();
    tmpTime = mDueTimeEdit->getTime();
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    todo->setDtDue(tmpDT);

    // set start date/time
    tmpDate = mStartDateEdit->getDate();
    tmpTime = mStartTimeEdit->getTime();
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    todo->setDtStart(tmpDT);
  } // check for float
  
  todo->setPriority(mPriorityCombo->currentItem()+1);

  // set completion state
  todo->setPercentComplete(mCompletedCombo->currentItem() * 20);

  if (mCompletedCombo->currentItem() == 5 && mCompleted.isValid()) {
    todo->setCompleted(mCompleted);
  }
}

void KOEditorGeneralTodo::dueStuffDisable(bool disable)
{
  if (disable) {
    mDueDateEdit->hide();
    mDueLabel->hide();
//    noTimeButton->hide();
    mDueTimeEdit->hide();
  } else {
    mDueDateEdit->show();
    mDueLabel->show();
//    noTimeButton->show();
    if (mNoTimeButton->isChecked()) mDueTimeEdit->hide();
    else mDueTimeEdit->show();
  }
}

void KOEditorGeneralTodo::startStuffDisable(bool disable)
{
  if (disable) {
    mStartDateEdit->hide();
    mStartLabel->hide();
    mStartTimeEdit->hide();
  } else {
    mStartDateEdit->show();
    mStartLabel->show();
    if (mNoTimeButton->isChecked()) mStartTimeEdit->hide();
    else mStartTimeEdit->show();
  }
}

void KOEditorGeneralTodo::timeStuffDisable(bool disable)
{
  if (disable) {
    mStartTimeEdit->hide();
    mDueTimeEdit->hide();
  } else {
    if(!mNoStartCheck->isChecked()) mStartTimeEdit->show();
    if(!mNoDueCheck->isChecked()) mDueTimeEdit->show();
  }
}

bool KOEditorGeneralTodo::validateInput()
{
  if (!mNoDueCheck->isChecked()) {
    if (!mDueDateEdit->inputIsValid()) {
      KMessageBox::sorry(this,i18n("Please specify a valid due date."));
      return false;
    }
    if (!mNoTimeButton->isChecked()) {
      if (!mDueTimeEdit->inputIsValid()) {
        KMessageBox::sorry(this,i18n("Please specify a valid due time."));
        return false;
      }
    }
  }

  if (!mNoStartCheck->isChecked()) {
    if (!mStartDateEdit->inputIsValid()) {
      KMessageBox::sorry(this,i18n("Please specify a valid start date."));
      return false;
    }
    if (!mNoTimeButton->isChecked()) {
      if (!mStartTimeEdit->inputIsValid()) {
        KMessageBox::sorry(this,i18n("Please specify a valid start time."));
        return false;
      }
    }
  }

  if (!mNoStartCheck->isChecked() && !mNoDueCheck->isChecked()) {
    QDateTime startDate;
    QDateTime dueDate;
    startDate.setDate(mStartDateEdit->getDate());
    dueDate.setDate(mDueDateEdit->getDate());
    if (!mNoTimeButton->isChecked()) {
      startDate.setTime(mStartTimeEdit->getTime());
      dueDate.setTime(mDueTimeEdit->getTime());
    }
    if (startDate > dueDate) {
      KMessageBox::sorry(this,
                         i18n("The start date cannot be after the due date."));
      return false;
    }
  }

  return KOEditorGeneral::validateInput();
}

void KOEditorGeneralTodo::completedChanged(int index)
{
  if (index == 5) {
    mCompleted = QDateTime::currentDateTime();
  }
  setCompletedDate();
}

void KOEditorGeneralTodo::setCompletedDate()
{
  if (mCompletedCombo->currentItem() == 5 && mCompleted.isValid()) {
    mCompletedLabel->setText(i18n("completed on %1")
        .arg(KGlobal::locale()->formatDateTime(mCompleted)));
  } else {
    mCompletedLabel->setText(i18n("completed"));
  }
}
