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
#include <kfiledialog.h>
#include <kdebug.h>

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
  initAlarmBox(); 
  initMisc();
  initLayout();

  QWidget::setTabOrder(summaryEdit, completedCombo);
  QWidget::setTabOrder(completedCombo, priorityCombo);
  QWidget::setTabOrder(priorityCombo, descriptionEdit);
  QWidget::setTabOrder(descriptionEdit, categoriesButton);
  QWidget::setTabOrder(categoriesButton, mSecrecyCombo);
  QWidget::setTabOrder(mSecrecyCombo, alarmButton);
  
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

void KOEditorGeneralTodo::initAlarmBox()
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

  layoutTop->addWidget(descriptionEdit,1);

  QBoxLayout *layoutCategories = new QHBoxLayout;
  layoutTop->addLayout(layoutCategories);
  layoutCategories->addWidget(categoriesButton);
  layoutCategories->addWidget(categoriesLabel,1);
  layoutCategories->addWidget(mSecrecyLabel);
  layoutCategories->addWidget(mSecrecyCombo);
}

void KOEditorGeneralTodo::pickAlarmSound()
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

void KOEditorGeneralTodo::pickAlarmProgram()
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

void KOEditorGeneralTodo::alarmStuffEnable(bool enable)
{
  alarmTimeEdit->setEnabled(enable);
  alarmSoundButton->setEnabled(enable);
  alarmProgramButton->setEnabled(enable);
  alarmIncrCombo->setEnabled(enable);
}

void KOEditorGeneralTodo::alarmStuffDisable(bool disable)
{
  alarmStuffEnable(!disable);
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
  alarmStuffDisable(allDay);
  
  mNoDueCheck->setChecked(true);
  dueStuffDisable(true);

  mNoStartCheck->setChecked(true);
  startStuffDisable(true);

  mDueDateEdit->setDate(due.date());
  mDueTimeEdit->setTime(due.time());
  
  mStartDateEdit->setDate(QDate::currentDate());
  mStartTimeEdit->setTime(QTime::currentTime());  

  mSecrecyCombo->setCurrentItem(Incidence::SecrecyPublic);

  priorityCombo->setCurrentItem(2);
  
  completedCombo->setCurrentItem(0);

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

void KOEditorGeneralTodo::readTodo(Todo *todo)
{
  QDateTime tmpDT, dueDT;
  int i;
  
  summaryEdit->setText(todo->summary());
  descriptionEdit->setText(todo->description());
  // organizer information
  ownerLabel->setText(i18n("Owner: ") + todo->organizer());

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

  noTimeButton->setChecked(todo->doesFloat());

  completedCombo->setCurrentItem(todo->percentComplete() / 20);
  if (todo->isCompleted() && todo->hasCompletedDate()) {
    mCompleted = todo->completed();
  }
  setCompletedDate();

  priorityCombo->setCurrentItem(todo->priority()-1);

  setCategories(todo->categoriesStr());

  mSecrecyCombo->setCurrentItem(todo->secrecy());

  // set up alarm stuff  
  alarmButton->setChecked(todo->alarm()->enabled());
  if (alarmButton->isChecked()) {
    alarmStuffEnable(true);
    tmpDT = todo->alarm()->time();
    if (tmpDT.isValid()) {
      i = tmpDT.secsTo(dueDT);
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
    
    if (!todo->alarm()->programFile().isEmpty()) {
      alarmProgram = todo->alarm()->programFile();
      alarmProgramButton->setOn(true);
      QString dispStr = i18n("Running '%1'").arg(alarmProgram);
      QToolTip::add(alarmProgramButton, dispStr);
    }
    if (!todo->alarm()->audioFile().isEmpty()) {
      alarmSound = todo->alarm()->audioFile();
      alarmSoundButton->setOn(true);
      QString dispStr = i18n("Playing '%1'").arg(alarmSound);
      QToolTip::add(alarmSoundButton, dispStr);
    }
  }
  else {
    alarmStuffEnable(false);
  }
}

void KOEditorGeneralTodo::writeTodo(Todo *todo)
{
  // temp. until something better happens.
  QString tmpStr;
  int j;
  
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

  // alarm stuff
  if (alarmButton->isChecked()) {
    todo->alarm()->setEnabled(true);
    tmpStr = alarmTimeEdit->text();
    j = tmpStr.toInt() * -60;
    if (alarmIncrCombo->currentItem() == 1)
      j = j * 60;
    else if (alarmIncrCombo->currentItem() == 2)
      j = j * (60 * 24);

    tmpDT = todo->dtDue();
    tmpDT = tmpDT.addSecs(j);
    todo->alarm()->setTime(tmpDT);
    if (!alarmProgram.isEmpty() && alarmProgramButton->isOn())
      todo->alarm()->setProgramFile(alarmProgram);
    else
      todo->alarm()->setProgramFile("");
    if (!alarmSound.isEmpty() && alarmSoundButton->isOn())
      todo->alarm()->setAudioFile(alarmSound);
    else
      todo->alarm()->setAudioFile("");
  } else {
    todo->alarm()->setEnabled(false);
    todo->alarm()->setProgramFile("");
    todo->alarm()->setAudioFile("");
  }

  // note, that if on the details tab the "Transparency" option is implemented,
  // we will have to change this to suit.
  //todo->setTransparency(freeTimeCombo->currentItem());
  
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
