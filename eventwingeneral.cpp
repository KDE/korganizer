// 	$Id$	

#include <qtooltip.h>
#include <qfiledlg.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kstddirs.h>

#include "eventwingeneral.h"
#include "eventwingeneral.moc"

EventWinGeneral::EventWinGeneral(QWidget* parent, const char* name)
  : WinGeneral( parent, name)
{
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
  QWidget::setTabOrder(categoriesButton, privateButton);

  summaryEdit->setFocus();
}


void EventWinGeneral::initTimeBox()
{
  timeGroupBox = new QGroupBox( 1,QGroupBox::Horizontal,
                                i18n("Appointment Time "),this, "User_2" );

  QFrame *timeBoxFrame = new QFrame(timeGroupBox,"TimeBoxFrame");

  QGridLayout *layoutTimeBox = new QGridLayout(timeBoxFrame,1,1,0,5);

  startLabel = new QLabel( timeBoxFrame, "Label_2" );
  startLabel->setText( i18n("Start Time:") );
  startLabel->setAlignment( 289 );
  startLabel->setMargin( -1 );
  layoutTimeBox->addWidget(startLabel,0,0);
  
  startDateEdit = new KDateEdit(timeBoxFrame);
  connect(startDateEdit, SIGNAL(dateChanged(QDate)),
    this, SLOT(setModified()));
  layoutTimeBox->addWidget(startDateEdit,0,1);

  startTimeEdit = new KTimeEdit(timeBoxFrame);
  connect(startTimeEdit, SIGNAL(timeChanged(QTime, int)),
    this, SLOT(setModified()));
  layoutTimeBox->addWidget(startTimeEdit,0,2);

  noTimeButton = new QCheckBox(timeBoxFrame, "CheckBox_1" );
  noTimeButton->setText( i18n("No time associated") );
  layoutTimeBox->addWidget(noTimeButton,0,4);

  connect(noTimeButton, SIGNAL(toggled(bool)), 
	  this, SLOT(timeStuffDisable(bool)));
  connect(noTimeButton, SIGNAL(toggled(bool)),
	  this, SLOT(alarmStuffDisable(bool)));
  
  endLabel = new QLabel( timeBoxFrame, "Label_3" );
  endLabel->setText( i18n("End Time:") );
  endLabel->setAlignment( 289 );
  endLabel->setMargin( -1 );
  layoutTimeBox->addWidget(endLabel,1,0);

  endDateEdit = new KDateEdit(timeBoxFrame);
  connect(endDateEdit, SIGNAL(dateChanged(QDate)),
    this, SLOT(setModified()));
  layoutTimeBox->addWidget(endDateEdit,1,1);

  endTimeEdit = new KTimeEdit(timeBoxFrame);
  connect(endTimeEdit, SIGNAL(timeChanged(QTime, int)),
    this, SLOT(setModified()));
  layoutTimeBox->addWidget(endTimeEdit,1,2);

  recursButton = new QCheckBox(timeBoxFrame);
  recursButton->setText(i18n("Recurring event"));
  layoutTimeBox->addWidget(recursButton,1,4);

  // some more layouting
  layoutTimeBox->setColStretch(3,1);
}

void EventWinGeneral::initMisc()
{
  summaryLabel = new QLabel( this, "Label_1" );
  summaryLabel->setText( i18n("Summary:") );
  summaryLabel->setAlignment( 289 );
  summaryLabel->setMargin( -1 );

  summaryEdit = new QLineEdit( this, "LineEdit_1" );
  connect(summaryEdit, SIGNAL(textChanged(const QString &)),
    this, SLOT(setModified()));

  freeTimeLabel = new QLabel( this, "Label_6" );
  freeTimeLabel->setText( i18n("Show Time As:") );
  freeTimeLabel->setAlignment( 289 );
  freeTimeLabel->setMargin( -1 );

  freeTimeCombo = new QComboBox( FALSE, this, "ComboBox_1" );
//  freeTimeCombo->setSizeLimit( 10 );  // that's the default value anyway
  freeTimeCombo->insertItem( i18n("Busy") );
  freeTimeCombo->insertItem( i18n("Free") );
  connect(freeTimeCombo, SIGNAL(activated(int)),
    this, SLOT(setModified()));

  descriptionEdit = new QMultiLineEdit( this, "MultiLineEdit_1" );
  descriptionEdit->insertLine( "" );
  descriptionEdit->setReadOnly( FALSE );
  descriptionEdit->setOverwriteMode( FALSE );
  connect(descriptionEdit, SIGNAL(textChanged()),
    this, SLOT(setModified()));


  ownerLabel = new QLabel( this, "Label_7" );
  ownerLabel->setText( i18n("Owner:") );
  ownerLabel->setAlignment( 289 );
  ownerLabel->setMargin( -1 );

  privateButton = new QCheckBox( this, "CheckBox_3" );
  privateButton->setText( i18n("Private") );
  connect(privateButton, SIGNAL(toggled(bool)),
    this, SLOT(setModified()));

  categoriesButton = new QPushButton( this, "PushButton_6" );
  categoriesButton->setText( i18n("Categories...") );

  categoriesLabel = new QLabel( this, "LineEdit_7" );
  categoriesLabel->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  categoriesLabel->setGeometry( 120, 360, 390, 20 );
}

void EventWinGeneral::initAlarmBox()
{
  QPixmap pixmap;

  alarmBell = new QLabel(this);
  alarmBell->setPixmap(BarIcon("bell"));

  alarmButton = new QCheckBox( this, "CheckBox_2" );
  alarmButton->setText( i18n("Reminder:") );
  connect(alarmButton, SIGNAL(toggled(bool)),
    this, SLOT(setModified()));

  alarmTimeEdit = new KRestrictedLine( this, "alarmTimeEdit",
				       "1234567890");
  alarmTimeEdit->setText("");
  connect(alarmTimeEdit, SIGNAL(textChanged(const QString &)),
    this, SLOT(setModified()));

  alarmIncrCombo = new QComboBox(FALSE, this);
  alarmIncrCombo->insertItem("minute(s)");
  alarmIncrCombo->insertItem("hour(s)");
  alarmIncrCombo->insertItem("day(s)");
  alarmIncrCombo->setMinimumHeight(20);
  connect(alarmIncrCombo, SIGNAL(activated(int)),
    this, SLOT(setModified()));

  alarmSoundButton = new QPushButton( this, "PushButton_4" );
  pixmap = BarIcon("playsound");
  //  alarmSoundButton->setText( i18n("WAV") );
  alarmSoundButton->setPixmap(pixmap);
  alarmSoundButton->setToggleButton(TRUE);
  QToolTip::add(alarmSoundButton, "No sound set");

  alarmProgramButton = new QPushButton( this, "PushButton_5" );
  pixmap = BarIcon("runprog");
  //  alarmProgramButton->setText( i18n("PROG") );
  alarmProgramButton->setPixmap(pixmap);
  alarmProgramButton->setToggleButton(TRUE);
  QToolTip::add(alarmProgramButton, "No program set");

  connect(alarmButton, SIGNAL(toggled(bool)),
	  this, SLOT(alarmStuffEnable(bool)));

  connect(alarmSoundButton, SIGNAL(clicked()),
	  this, SLOT(pickAlarmSound()));
  connect(alarmProgramButton, SIGNAL(clicked()),
	  this, SLOT(pickAlarmProgram()));
}

void EventWinGeneral::initLayout()
{
  QBoxLayout *layoutTop = new QVBoxLayout(this,5);
  
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

  layoutTop->addWidget(descriptionEdit);

  QBoxLayout *layoutCategories = new QHBoxLayout;
  layoutTop->addLayout(layoutCategories);
  layoutCategories->addWidget(categoriesButton);
  layoutCategories->addWidget(categoriesLabel,1);
  layoutCategories->addWidget(privateButton);
}

void EventWinGeneral::pickAlarmSound()
{
  QString prefix = KGlobal::dirs()->findResourceDir("appdata", "sounds/alert.wav"); 
  if (!alarmSoundButton->isOn()) {
    alarmSound = "";
    QToolTip::remove(alarmSoundButton);
    QToolTip::add(alarmSoundButton, "No sound set");

    emit modifiedEvent();
  } else {
    QString fileName(QFileDialog::getOpenFileName(prefix.data(),
						  "*.wav", this));
    if (!fileName.isEmpty()) {
      alarmSound = fileName;
      QToolTip::remove(alarmSoundButton);
      QString dispStr = "Playing \"";
      dispStr += fileName.data();
      dispStr += "\"";
      QToolTip::add(alarmSoundButton, dispStr.data());
      emit modifiedEvent();
    }
  }
  if (alarmSound.isEmpty())
    alarmSoundButton->setOn(FALSE);
}

void EventWinGeneral::pickAlarmProgram()
{
  if (!alarmProgramButton->isOn()) {
    alarmProgram = "";
    QToolTip::remove(alarmProgramButton);
    QToolTip::add(alarmProgramButton, "No program set");
    emit modifiedEvent();
  } else {
    QString fileName(QFileDialog::getOpenFileName(QString::null, "*", this));
    if (!fileName.isEmpty()) {
      alarmProgram = fileName;
      QToolTip::remove(alarmProgramButton);
      QString dispStr = "Running \"";
      dispStr += fileName.data();
      dispStr += "\"";
      QToolTip::add(alarmProgramButton, dispStr.data());
      emit modifiedEvent();
    }
  }
  if (alarmProgram.isEmpty())
    alarmProgramButton->setOn(FALSE);
}

EventWinGeneral::~EventWinGeneral()
{
}

void EventWinGeneral::setEnabled(bool enabled)
{
  noTimeButton->setEnabled(enabled);
  recursButton->setEnabled(enabled);

  summaryEdit->setEnabled(enabled);
  startDateEdit->setEnabled(enabled);
  endDateEdit->setEnabled(enabled);

  startTimeEdit->setEnabled(enabled);
  endTimeEdit->setEnabled(enabled);

  alarmButton->setEnabled(enabled);
  alarmTimeEdit->setEnabled(enabled);
  alarmSoundButton->setEnabled(enabled);
  alarmProgramButton->setEnabled(enabled);

  descriptionEdit->setEnabled(enabled);
  freeTimeCombo->setEnabled(enabled);
  privateButton->setEnabled(enabled);
  categoriesButton->setEnabled(enabled);
  categoriesLabel->setEnabled(enabled);

  emit modifiedEvent();
}

void EventWinGeneral::timeStuffDisable(bool disable)
{
  if (disable) {
    startTimeEdit->hide();
    endTimeEdit->hide();
  } else {
    startTimeEdit->show();
    endTimeEdit->show();
  }
  
  emit modifiedEvent();
}

void EventWinGeneral::alarmStuffEnable(bool enable)
{
  alarmTimeEdit->setEnabled(enable);
  alarmSoundButton->setEnabled(enable);
  alarmProgramButton->setEnabled(enable);

  emit modifiedEvent();
}

void EventWinGeneral::alarmStuffDisable(bool disable)
{
  alarmTimeEdit->setEnabled(!disable);
  alarmSoundButton->setEnabled(!disable);
  alarmProgramButton->setEnabled(!disable);

  emit modifiedEvent();
}


