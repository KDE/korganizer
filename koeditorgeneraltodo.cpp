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

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <krestrictedline.h>
#include <kstandarddirs.h>
#include <kfiledialog.h>

#include <libkcal/todo.h>

#include <libkdepim/kdateedit.h>

#include "koprefs.h"

#include "koeditorgeneraltodo.h"
#include "koeditorgeneraltodo.moc"

KOEditorGeneralTodo::KOEditorGeneralTodo(QObject* parent,
                                         const char* name)
  : KOEditorGeneral( parent, name)
{
}

KOEditorGeneralTodo::~KOEditorGeneralTodo()
{
}

void KOEditorGeneralTodo::finishSetup()
{
  QWidget::setTabOrder(mSummaryEdit, mLocationEdit);
  QWidget::setTabOrder(mLocationEdit, mDueCheck);
  QWidget::setTabOrder(mDueCheck, mDueDateEdit);
  QWidget::setTabOrder(mDueDateEdit, mDueTimeEdit);
  QWidget::setTabOrder(mDueTimeEdit, mStartCheck);
  QWidget::setTabOrder(mStartCheck, mStartDateEdit);
  QWidget::setTabOrder(mStartDateEdit, mStartTimeEdit);
  QWidget::setTabOrder(mStartTimeEdit, mTimeButton);
  QWidget::setTabOrder(mTimeButton, mCompletedCombo);
  QWidget::setTabOrder(mCompletedCombo, mPriorityCombo);
  QWidget::setTabOrder(mPriorityCombo, mAlarmButton);
  QWidget::setTabOrder(mAlarmButton, mCategoriesButton);
  QWidget::setTabOrder(mCategoriesButton, mSecrecyCombo);
  QWidget::setTabOrder(mSecrecyCombo, mDescriptionEdit);
  
  mSummaryEdit->setFocus();
}

void KOEditorGeneralTodo::initTime(QWidget *parent,QBoxLayout *topLayout)
{
  QBoxLayout *timeLayout = new QVBoxLayout(topLayout);

  QGroupBox *timeGroupBox = new QGroupBox(1,QGroupBox::Horizontal,
                                          i18n("Date && Time"),parent);
  timeLayout->addWidget(timeGroupBox);

  QFrame *timeBoxFrame = new QFrame(timeGroupBox);

  QGridLayout *layoutTimeBox = new QGridLayout(timeBoxFrame,1,1);
  layoutTimeBox->setSpacing(topLayout->spacing());


  mDueCheck = new QCheckBox(i18n("Due:"),timeBoxFrame);
  layoutTimeBox->addWidget(mDueCheck,0,0);
  connect(mDueCheck,SIGNAL(toggled(bool)),SLOT(enableDueEdit(bool)));
  connect(mDueCheck,SIGNAL(toggled(bool)),SLOT(showAlarm(bool)));
  
  
  mDueDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mDueDateEdit,0,1);

  mDueTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mDueTimeEdit,0,2);


  mStartCheck = new QCheckBox(i18n("Start:"),timeBoxFrame);
  layoutTimeBox->addWidget(mStartCheck,1,0);
  connect(mStartCheck,SIGNAL(toggled(bool)),SLOT(enableStartEdit(bool)));

  mStartDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mStartDateEdit,1,1);

  mStartTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mStartTimeEdit,1,2);


  mTimeButton = new QCheckBox(i18n("Time associated"),timeBoxFrame);
  layoutTimeBox->addMultiCellWidget(mTimeButton,2,2,0,2);

  connect(mTimeButton,SIGNAL(toggled(bool)),SLOT(enableTimeEdits(bool)));
  connect(mTimeButton,SIGNAL(toggled(bool)),SLOT(showAlarm(bool)));
  
  // some more layouting
  layoutTimeBox->setColStretch(3,1);
}


void KOEditorGeneralTodo::initCompletion(QWidget *parent, QBoxLayout *topLayout)
{
  mCompletedCombo = new QComboBox(parent);
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
  topLayout->addWidget(mCompletedCombo);

  mCompletedLabel = new QLabel(i18n("completed"),parent);
  topLayout->addWidget(mCompletedLabel);
}

void KOEditorGeneralTodo::initPriority(QWidget *parent, QBoxLayout *topLayout)
{
  QLabel *priorityLabel = new QLabel(i18n("Priority:"),parent);
  topLayout->addWidget(priorityLabel);

  mPriorityCombo = new QComboBox(parent);
  mPriorityCombo->insertItem(i18n("1 (Highest)"));
  mPriorityCombo->insertItem(i18n("2"));
  mPriorityCombo->insertItem(i18n("3"));
  mPriorityCombo->insertItem(i18n("4"));
  mPriorityCombo->insertItem(i18n("5 (lowest)"));
  topLayout->addWidget(mPriorityCombo);
}

void KOEditorGeneralTodo::initStatus(QWidget *parent,QBoxLayout *topLayout)
{
  QBoxLayout *statusLayout = new QHBoxLayout(topLayout);

  initCompletion( parent, statusLayout );

  statusLayout->addStretch( 1 );

  initPriority( parent, statusLayout );
}

void KOEditorGeneralTodo::setDefaults(QDateTime due,bool allDay)
{
  KOEditorGeneral::setDefaults(allDay);

  mTimeButton->setChecked( !allDay );
  enableTimeEdits( !allDay );
  
  mDueCheck->setChecked(false);
  enableDueEdit(false);

  alarmDisable(true);
  
  mStartCheck->setChecked(false);
  enableStartEdit(false);

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
    alarmDisable(false);
    dueDT = todo->dtDue();
    mDueDateEdit->setDate(todo->dtDue().date());
    mDueTimeEdit->setTime(todo->dtDue().time());
    mDueCheck->setChecked(true);
  } else {
    alarmDisable(true);
    mDueDateEdit->setDate(QDate::currentDate());
    mDueTimeEdit->setTime(QTime::currentTime());
    mDueCheck->setChecked(false);
  }

  if (todo->hasStartDate()) {
    mStartDateEdit->setDate(todo->dtStart().date());
    mStartTimeEdit->setTime(todo->dtStart().time());
    mStartCheck->setChecked(true);
  } else {
    mStartDateEdit->setDate(QDate::currentDate());
    mStartTimeEdit->setTime(QTime::currentTime());
    mStartCheck->setChecked(false);
  }

  mTimeButton->setChecked( !todo->doesFloat() );

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
  
  todo->setHasDueDate(mDueCheck->isChecked());
  todo->setHasStartDate(mStartCheck->isChecked());

  QDate tmpDate;
  QTime tmpTime;
  QDateTime tmpDT;
  if ( mTimeButton->isChecked() ) {
    todo->setFloats(false);
    
    // set due date/time
    tmpDate = mDueDateEdit->date();
    tmpTime = mDueTimeEdit->getTime();
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    todo->setDtDue(tmpDT);

    // set start date/time
    tmpDate = mStartDateEdit->date();
    tmpTime = mStartTimeEdit->getTime();
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    todo->setDtStart(tmpDT);
  } else {
    todo->setFloats(true);

    // need to change this.
    tmpDate = mDueDateEdit->date();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    todo->setDtDue(tmpDT);
    
    tmpDate = mStartDateEdit->date();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    todo->setDtStart(tmpDT);
  }
  
  todo->setPriority(mPriorityCombo->currentItem()+1);

  // set completion state
  todo->setPercentComplete(mCompletedCombo->currentItem() * 20);

  if (mCompletedCombo->currentItem() == 5 && mCompleted.isValid()) {
    todo->setCompleted(mCompleted);
  }
}

void KOEditorGeneralTodo::enableDueEdit(bool enable)
{
  mDueDateEdit->setEnabled( enable );
  
  if (enable) {
    mDueTimeEdit->setEnabled( mTimeButton->isChecked() );
  } else {
    mDueTimeEdit->setEnabled( false );
  }
}

void KOEditorGeneralTodo::enableStartEdit( bool enable )
{
  mStartDateEdit->setEnabled( enable );

  if (enable) {
    mStartTimeEdit->setEnabled( mTimeButton->isChecked() );
  } else {
    mStartTimeEdit->setEnabled( false );
  }
}

void KOEditorGeneralTodo::enableTimeEdits(bool enable)
{
  mStartTimeEdit->setEnabled( enable );
  mDueTimeEdit->setEnabled( enable );
}

void KOEditorGeneralTodo::showAlarm(bool show) {
  if(show && mDueCheck->isChecked() && mTimeButton->isChecked()) {
    alarmDisable(false);
  }
  else {
    alarmDisable(true);
  }
}

bool KOEditorGeneralTodo::validateInput()
{
  if (mDueCheck->isChecked()) {
    if (!mDueDateEdit->inputIsValid()) {
      KMessageBox::sorry(0,i18n("Please specify a valid due date."));
      return false;
    }
    if (mTimeButton->isChecked()) {
      if (!mDueTimeEdit->inputIsValid()) {
        KMessageBox::sorry(0,i18n("Please specify a valid due time."));
        return false;
      }
    }
  }

  if (mStartCheck->isChecked()) {
    if (!mStartDateEdit->inputIsValid()) {
      KMessageBox::sorry(0,i18n("Please specify a valid start date."));
      return false;
    }
    if (mTimeButton->isChecked()) {
      if (!mStartTimeEdit->inputIsValid()) {
        KMessageBox::sorry(0,i18n("Please specify a valid start time."));
        return false;
      }
    }
  }

  if (mStartCheck->isChecked() && mDueCheck->isChecked()) {
    QDateTime startDate;
    QDateTime dueDate;
    startDate.setDate(mStartDateEdit->date());
    dueDate.setDate(mDueDateEdit->date());
    if (mTimeButton->isChecked()) {
      startDate.setTime(mStartTimeEdit->getTime());
      dueDate.setTime(mDueTimeEdit->getTime());
    }
    if (startDate > dueDate) {
      KMessageBox::sorry(0,
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

void KOEditorGeneralTodo::modified (Todo* todo, int modification) 
{
  switch (modification) {
  case KOGlobals::PRIORITY_MODIFIED:
    mPriorityCombo->setCurrentItem(todo->priority()-1);
    break;
  case KOGlobals::COMPLETION_MODIFIED:
    mCompletedCombo->setCurrentItem(todo->percentComplete() / 20);
    if (todo->isCompleted() && todo->hasCompletedDate()) {
      mCompleted = todo->completed();
    }
    setCompletedDate();
    break;
  case KOGlobals::CATEGORY_MODIFIED:
    setCategories (todo->categoriesStr ());
    break;
  case KOGlobals::UNKNOWN_MODIFIED: // fall through
  default:
    readTodo( todo );  
    break;
  }
}
