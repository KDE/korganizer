// 	$Id$	

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

#include "koevent.h"
#include "koprefs.h"

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
  QWidget::setTabOrder(categoriesButton, privateButton);
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

  noDueButton = new QCheckBox(timeBoxFrame, "CheckBox_1" );
  noDueButton->setText( i18n("No due date") );
  layoutTimeBox->addWidget(noDueButton,0,0);

  connect(noDueButton, SIGNAL(toggled(bool)), 
	  this, SLOT(dueStuffDisable(bool)));

  startLabel = new QLabel( timeBoxFrame, "Label_2" );
  startLabel->setText( i18n("Due Date:") );
  layoutTimeBox->addWidget(startLabel,1,0);
  
  startDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(startDateEdit,1,1);

  startTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(startTimeEdit,1,2);

  noTimeButton = new QCheckBox(timeBoxFrame, "CheckBox_2" );
  noTimeButton->setText( i18n("No time associated") );
  layoutTimeBox->addWidget(noTimeButton,1,4);

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

  privateButton = new QCheckBox( this, "CheckBox_3" );
  privateButton->setText( i18n("Private") );

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
  layoutCategories->addWidget(privateButton);
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
  
  noDueButton->setChecked(true);
  dueStuffDisable(true);

  startDateEdit->setDate(due.date());
  startTimeEdit->setTime(due.time());
}

void KOEditorGeneralTodo::readTodo(KOEvent *todo)
{
  summaryEdit->setText(todo->getSummary());
  descriptionEdit->setText(todo->getDescription());
  // organizer information
  ownerLabel->setText(i18n("Owner: ") + todo->getOrganizer());

  if (todo->hasDueDate()) {
    startDateEdit->setDate(todo->getDtDue().date());
    startTimeEdit->setTime(todo->getDtDue().time());
    noDueButton->setChecked(false);
  } else {
    startDateEdit->setDate(QDate::currentDate());
    startTimeEdit->setTime(QTime::currentTime());
    noDueButton->setChecked(true);
  } 

  noTimeButton->setChecked(todo->doesFloat());

  if (todo->getStatusStr() == "NEEDS ACTION")
    completedButton->setChecked(FALSE);
  else
    completedButton->setChecked(TRUE);

  priorityCombo->setCurrentItem(todo->getPriority()-1);

  setCategories(todo->getCategoriesStr());
}

void KOEditorGeneralTodo::writeTodo(KOEvent *todo)
{
  todo->setSummary(summaryEdit->text());
  todo->setDescription(descriptionEdit->text());
  todo->setCategories(categoriesLabel->text());
  todo->setSecrecy(privateButton->isChecked() ? 1 : 0);

  todo->setHasDueDate(!noDueButton->isChecked());

  QDate tmpDate;
  QTime tmpTime;
  QDateTime tmpDT;
  if (noTimeButton->isChecked()) {
    todo->setFloats(true);

    // need to change this.
    tmpDate = startDateEdit->getDate();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    todo->setDtDue(tmpDT);
  } else {
    todo->setFloats(false);
    
    // set date/time start
    tmpDate = startDateEdit->getDate();
    tmpTime = startTimeEdit->getTime();
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    todo->setDtDue(tmpDT);
  } // check for float
  
  todo->setPriority(priorityCombo->currentItem()+1);

  if (completedButton->isChecked()) {
    todo->setStatus(QString("COMPLETED"));
  } else {
    todo->setStatus(QString("NEEDS ACTION"));
  }
}

void KOEditorGeneralTodo::setEnabled(bool enabled)
{
  // Enable all widgets, which are created in the initMisc method.
  // Labels are not enabled, since they are not active input controls.

  completedButton->setEnabled(enabled);
  priorityCombo->setEnabled(enabled);
  summaryEdit->setEnabled(enabled);
  descriptionEdit->setEnabled(enabled);
  privateButton->setEnabled(enabled);
  categoriesButton->setEnabled(enabled);
}

void KOEditorGeneralTodo::dueStuffDisable(bool disable)
{
  if (disable) {
    startDateEdit->hide();
    startLabel->hide();
    noTimeButton->hide();
    startTimeEdit->hide();
  } else {
    startDateEdit->show();
    startLabel->show();
    noTimeButton->show();
    if (noTimeButton->isChecked()) startTimeEdit->hide();
    else startTimeEdit->show();
  }
}

void KOEditorGeneralTodo::timeStuffDisable(bool disable)
{
  if (disable) {
    startTimeEdit->hide();
  } else {
    startTimeEdit->show();
  }
}

bool KOEditorGeneralTodo::validateInput()
{
  if (!noDueButton->isChecked()) {
    if (!startDateEdit->inputIsValid()) {
      KMessageBox::sorry(this,i18n("Please specify a valid due date."));
      return false;
    }
    if (!noTimeButton->isChecked()) {
      if (!startTimeEdit->inputIsValid()) {
        KMessageBox::sorry(this,i18n("Please specify a valid due time."));
        return false;
      }
    }
  }

  return true;
}
