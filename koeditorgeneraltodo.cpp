// $Id$	

#include <qtooltip.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kstddirs.h>
#include <kbuttonbox.h>
#include <kmessagebox.h>

#include "koprefs.h"
#include "todo.h"

#include "koeditorgeneraltodo.h"
#include "koeditorgeneraltodo.moc"

KOEditorGeneralTodo::KOEditorGeneralTodo(int spacing,QWidget* parent,
                                         const char* name)
  : QWidget( parent, name)
{
  mSpacing = spacing;

  initTimeBox();
  initMisc();

  initLayout();

  QWidget::setTabOrder(summaryEdit, completedButton);
  QWidget::setTabOrder(completedButton, priorityCombo);
  QWidget::setTabOrder(priorityCombo, descriptionEdit);
  QWidget::setTabOrder(descriptionEdit, categoriesButton);
  QWidget::setTabOrder(categoriesButton, mSecrecyCombo);
  summaryEdit->setFocus();
}

KOEditorGeneralTodo::~KOEditorGeneralTodo()
{
}

void KOEditorGeneralTodo::initTimeBox()
{
  timeGroupBox = new QGroupBox( 1,QGroupBox::Horizontal,
                                i18n("Due Date "),this, "User_2" );

  QFrame *timeBoxFrame = new QFrame(timeGroupBox,"TimeBoxFrame");

  QGridLayout *layoutTimeBox = new QGridLayout(timeBoxFrame,1,1);
  layoutTimeBox->setSpacing(mSpacing);

  mNoDueCheck = new QCheckBox(timeBoxFrame);
  mNoDueCheck->setText(i18n("No due date"));
  layoutTimeBox->addWidget(mNoDueCheck,0,0);

  connect(mNoDueCheck, SIGNAL(toggled(bool)), 
	  this, SLOT(dueStuffDisable(bool)));

  mDueLabel = new QLabel( timeBoxFrame, "Label_2" );
  mDueLabel->setText( i18n("Due Date:") );
  layoutTimeBox->addWidget(mDueLabel,1,0);
  
  mDueDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mDueDateEdit,1,1);

  mDueTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mDueTimeEdit,1,2);


  mNoStartCheck = new QCheckBox(timeBoxFrame);
  mNoStartCheck->setText(i18n("No start date"));
  layoutTimeBox->addWidget(mNoStartCheck,2,0);

  connect(mNoStartCheck, SIGNAL(toggled(bool)), 
	  this, SLOT(startStuffDisable(bool)));

  mStartLabel = new QLabel(timeBoxFrame);
  mStartLabel->setText(i18n("Start Date:"));
  layoutTimeBox->addWidget(mStartLabel,3,0);
  
  mStartDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mStartDateEdit,3,1);

  mStartTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(mStartTimeEdit,3,2);


  noTimeButton = new QCheckBox(timeBoxFrame);
  noTimeButton->setText(i18n("No time associated"));
  layoutTimeBox->addWidget(noTimeButton,0,4);

  connect(noTimeButton, SIGNAL(toggled(bool)), 
	  this, SLOT(timeStuffDisable(bool)));
//  connect(noTimeButton, SIGNAL(toggled(bool)),
//	  this, SLOT(alarmStuffDisable(bool)));
  
  // some more layouting
  layoutTimeBox->setColStretch(3,1);
}


void KOEditorGeneralTodo::initMisc()
{
  completedButton = new QCheckBox(this, "CheckBox_10" );
  completedButton->setText( i18n("Completed") );
  connect(completedButton,SIGNAL(clicked()),SLOT(completedClicked()));

  priorityLabel = new QLabel( this, "Label_3" );
  priorityLabel->setText( i18n("Priority:") );

  priorityCombo = new QComboBox( false, this, "ComboBox_10" );
  priorityCombo->setSizeLimit( 10 );
  priorityCombo->insertItem( i18n("1 (Highest)") );
  priorityCombo->insertItem( i18n("2") );
  priorityCombo->insertItem( i18n("3") );
  priorityCombo->insertItem( i18n("4") );
  priorityCombo->insertItem( i18n("5 (lowest)") );

  summaryLabel = new QLabel( this, "Label_1" );
  summaryLabel->setText( i18n("Summary:") );

  summaryEdit = new QLineEdit( this, "LineEdit_1" );

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

void KOEditorGeneralTodo::initLayout()
{
  QBoxLayout *layoutTop = new QVBoxLayout(this);
  layoutTop->setSpacing(mSpacing);
  
  layoutTop->addWidget(ownerLabel);

  QBoxLayout *layoutSummary = new QHBoxLayout;
  layoutTop->addLayout(layoutSummary);
  layoutSummary->addWidget(summaryLabel);
  layoutSummary->addWidget(summaryEdit);
  
  layoutTop->addWidget(timeGroupBox);

  QBoxLayout *layoutCompletion = new QHBoxLayout;
  layoutTop->addLayout(layoutCompletion);
  layoutCompletion->addWidget(completedButton);
  layoutCompletion->addStretch();
  layoutCompletion->addWidget(priorityLabel);
  layoutCompletion->addWidget(priorityCombo);
  
  layoutTop->addWidget(descriptionEdit);
  
  QBoxLayout *layoutCategories = new QHBoxLayout;
  layoutTop->addLayout(layoutCategories);
  layoutCategories->addWidget(categoriesButton);
  layoutCategories->addWidget(categoriesLabel,1);
  layoutCategories->addWidget(mSecrecyLabel);
  layoutCategories->addWidget(mSecrecyCombo);
}

void KOEditorGeneralTodo::setCategories(QString str)
{
  categoriesLabel->setText(str);
}

void KOEditorGeneralTodo::setDefaults(QDateTime due,bool allDay)
{
  ownerLabel->setText(i18n("Owner: ") + KOPrefs::instance()->mName);

  noTimeButton->setChecked(allDay);
  timeStuffDisable(allDay);
  
  mNoDueCheck->setChecked(true);
  dueStuffDisable(true);

  mNoStartCheck->setChecked(true);
  startStuffDisable(true);

  mDueDateEdit->setDate(due.date());
  mDueTimeEdit->setTime(due.time());
  
  mStartDateEdit->setDate(QDate::currentDate());
  mStartTimeEdit->setTime(QTime::currentTime());  

  mSecrecyCombo->setCurrentItem(Incidence::SecrecyPublic);
}

void KOEditorGeneralTodo::readTodo(Todo *todo)
{
  summaryEdit->setText(todo->summary());
  descriptionEdit->setText(todo->description());
  // organizer information
  ownerLabel->setText(i18n("Owner: ") + todo->organizer());

  if (todo->hasDueDate()) {
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

  noTimeButton->setChecked(todo->doesFloat());

  if (todo->isCompleted()) {
    completedButton->setChecked(true);
    if (todo->hasCompletedDate()) {
      mCompleted = todo->completed();
    }
  } else {
    completedButton->setChecked(false);
  }
  setCompletedDate();

  priorityCombo->setCurrentItem(todo->priority()-1);

  setCategories(todo->categoriesStr());

  mSecrecyCombo->setCurrentItem(todo->secrecy());
}

void KOEditorGeneralTodo::writeTodo(Todo *todo)
{
  todo->setSummary(summaryEdit->text());
  todo->setDescription(descriptionEdit->text());
  todo->setCategories(categoriesLabel->text());
  todo->setSecrecy(mSecrecyCombo->currentItem());
  
  todo->setHasDueDate(!mNoDueCheck->isChecked());
  todo->setHasStartDate(!mNoStartCheck->isChecked());

  QDate tmpDate;
  QTime tmpTime;
  QDateTime tmpDT;
  if (noTimeButton->isChecked()) {
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
  
  todo->setPriority(priorityCombo->currentItem()+1);

  // set completion state
  if (completedButton->isChecked()) {
    if (mCompleted.isValid()) {
      todo->setCompleted(mCompleted);
    } else {
      todo->setCompleted(true);
    }
  } else {
    todo->setCompleted(false);
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
    if (noTimeButton->isChecked()) mDueTimeEdit->hide();
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
    if (noTimeButton->isChecked()) mStartTimeEdit->hide();
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
    if (!noTimeButton->isChecked()) {
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
    if (!noTimeButton->isChecked()) {
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
    if (!noTimeButton->isChecked()) {
      startDate.setTime(mStartTimeEdit->getTime());
      dueDate.setTime(mDueTimeEdit->getTime());
    }
    if (startDate > dueDate) {
      KMessageBox::sorry(this,
                         i18n("The start date cannot be after the due date."));
      return false;
    }
  }

  return true;
}

void KOEditorGeneralTodo::completedClicked()
{
  if (completedButton->isChecked()) {
    mCompleted = QDateTime::currentDateTime();
  }
  setCompletedDate();
}

void KOEditorGeneralTodo::setCompletedDate()
{
  if (completedButton->isChecked() && mCompleted.isValid()) {
    completedButton->setText(i18n("Completed on %1")
        .arg(KGlobal::locale()->formatDateTime(mCompleted)));
  } else {
    completedButton->setText(i18n("Completed"));
  }
}
