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

  QWidget::setTabOrder(summaryEdit, completedCombo);
  QWidget::setTabOrder(completedCombo, priorityCombo);
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


  noTimeButton = new QCheckBox(i18n("No time associated"),timeBoxFrame);
  layoutTimeBox->addWidget(noTimeButton,0,4);

  connect(noTimeButton,SIGNAL(toggled(bool)),SLOT(timeStuffDisable(bool)));
//  connect(noTimeButton, SIGNAL(toggled(bool)),
//	  this, SLOT(alarmStuffDisable(bool)));
  
  // some more layouting
  layoutTimeBox->setColStretch(3,1);
}


void KOEditorGeneralTodo::initMisc()
{
/*
  completedButton = new QCheckBox(this, "CheckBox_10" );
  completedButton->setText( i18n("Completed") );
  connect(completedButton,SIGNAL(clicked()),SLOT(completedClicked()));
*/

  completedCombo = new QComboBox(this);
  // xgettext:no-c-format
  completedCombo->insertItem(i18n("0 %"));
  // xgettext:no-c-format
  completedCombo->insertItem(i18n("20 %"));
  // xgettext:no-c-format
  completedCombo->insertItem(i18n("40 %"));
  // xgettext:no-c-format
  completedCombo->insertItem(i18n("60 %"));
  // xgettext:no-c-format
  completedCombo->insertItem(i18n("80 %"));
  // xgettext:no-c-format
  completedCombo->insertItem(i18n("100 %"));
  connect(completedCombo,SIGNAL(activated(int)),SLOT(completedChanged(int)));

  completedLabel = new QLabel(i18n("completed"),this);

  priorityLabel = new QLabel(i18n("Priority:"),this);

  priorityCombo = new QComboBox(this);
  priorityCombo->setSizeLimit(10);
  priorityCombo->insertItem(i18n("1 (Highest)"));
  priorityCombo->insertItem(i18n("2"));
  priorityCombo->insertItem(i18n("3"));
  priorityCombo->insertItem(i18n("4"));
  priorityCombo->insertItem(i18n("5 (lowest)"));

  summaryLabel = new QLabel(i18n("Summary:"),this);

  summaryEdit = new QLineEdit(this);

  descriptionEdit = new QMultiLineEdit(this);
  descriptionEdit->insertLine("");
  descriptionEdit->setReadOnly(false);
  descriptionEdit->setOverwriteMode(false);
  descriptionEdit->setWordWrap(QMultiLineEdit::WidgetWidth);

  ownerLabel = new QLabel(i18n("Owner:"),this);

  mSecrecyLabel = new QLabel("Access:",this);
  mSecrecyCombo = new QComboBox(this);
  mSecrecyCombo->insertStringList(Incidence::secrecyList());

  categoriesButton = new QPushButton(i18n("Categories..."),this);
  connect(categoriesButton,SIGNAL(clicked()),SIGNAL(openCategoryDialog()));

  categoriesLabel = new QLabel(this);
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
  layoutCompletion->addWidget(completedCombo);
  layoutCompletion->addWidget(completedLabel);
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

void KOEditorGeneralTodo::setCategories(const QString &str)
{
  categoriesLabel->setText(str);
}

void KOEditorGeneralTodo::setDefaults(QDateTime due,bool allDay)
{
  ownerLabel->setText(i18n("Owner: ") + KOPrefs::instance()->fullName());

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

  completedCombo->setCurrentItem(0);
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

  completedCombo->setCurrentItem(todo->percentComplete() / 20);
  if (todo->isCompleted() && todo->hasCompletedDate()) {
    mCompleted = todo->completed();
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
  todo->setPercentComplete(completedCombo->currentItem() * 20);

  if (completedCombo->currentItem() == 5 && mCompleted.isValid()) {
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

void KOEditorGeneralTodo::completedChanged(int index)
{
  if (index == 5) {
    mCompleted = QDateTime::currentDateTime();
  }
  setCompletedDate();
}

void KOEditorGeneralTodo::setCompletedDate()
{
  if (completedCombo->currentItem() == 5 && mCompleted.isValid()) {
    completedLabel->setText(i18n("completed on %1")
        .arg(KGlobal::locale()->formatDateTime(mCompleted)));
  } else {
    completedLabel->setText(i18n("completed"));
  }
}
