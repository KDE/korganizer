// 	$Id$	

#include <qtooltip.h>
#include <qfiledlg.h>
#include <qlayout.h>

#include <klocale.h>
#include <kiconloader.h>

#include "todowingeneral.h"
#include "todowingeneral.moc"

TodoWinGeneral::TodoWinGeneral(QWidget* parent, const char* name)
  : WinGeneral( parent, name)
{
//  alarmProgram = "";
//  resize( 600,400 );

  initTimeBox();
  initMisc();

  initLayout();

//  setMinimumSize( 600, 400 );
//  setMaximumSize( 32767, 32767 );

  QWidget::setTabOrder(summaryEdit, completedButton);
  QWidget::setTabOrder(completedButton, completedCombo);
  QWidget::setTabOrder(completedCombo, priorityCombo);
  QWidget::setTabOrder(priorityCombo, descriptionEdit);
  QWidget::setTabOrder(descriptionEdit, categoriesButton);
  QWidget::setTabOrder(categoriesButton, privateButton);
  summaryEdit->setFocus();
}


void TodoWinGeneral::initTimeBox()
{
  timeGroupBox = new QGroupBox( 1,QGroupBox::Horizontal,
                                i18n("Due Date "),this, "User_2" );

  QFrame *timeBoxFrame = new QFrame(timeGroupBox,"TimeBoxFrame");

  QGridLayout *layoutTimeBox = new QGridLayout(timeBoxFrame,1,1,0,5);

  noDueButton = new QCheckBox(timeBoxFrame, "CheckBox_1" );
  noDueButton->setText( i18n("No due date") );
  layoutTimeBox->addWidget(noDueButton,0,0);

  connect(noDueButton, SIGNAL(toggled(bool)), 
	  this, SLOT(dueStuffDisable(bool)));

  startLabel = new QLabel( timeBoxFrame, "Label_2" );
  startLabel->setText( i18n("Due Date:") );
  startLabel->setAlignment( 289 );
  startLabel->setMargin( -1 );
  layoutTimeBox->addWidget(startLabel,1,0);
  
  startDateEdit = new KDateEdit(timeBoxFrame);
  connect(startDateEdit, SIGNAL(dateChanged(QDate)),
    this, SLOT(setModified()));
  layoutTimeBox->addWidget(startDateEdit,1,1);

  startTimeEdit = new KTimeEdit(timeBoxFrame);
  connect(startTimeEdit, SIGNAL(timeChanged(QTime, int)),
    this, SLOT(setModified()));
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


void TodoWinGeneral::initMisc() {
//    int ypos = 70;

    completedButton = new QCheckBox(this, "CheckBox_10" );
//    completedButton->setGeometry( 10, ypos, 140, 20 );
    completedButton->setText( i18n("Completed") );
    connect(completedButton, SIGNAL(toggled(bool)),
	    this, SLOT(setModified()));

    completedLabel = new QLabel( this, "Label_3" );
//    completedLabel->setGeometry( 130, ypos, 70, 20 );
    completedLabel->setText( i18n("% Completed") );
    completedLabel->setAlignment( 289 );
    completedLabel->setMargin( -1 );

    completedCombo = new QComboBox( FALSE, this, "ComboBox_10" );
//    completedCombo->setGeometry( 220, ypos, 100, 20 );
    completedCombo->setSizeLimit( 10 );
    completedCombo->insertItem( i18n("0 %") );
    completedCombo->insertItem( i18n("25 %") );
    completedCombo->insertItem( i18n("50 %") );
    completedCombo->insertItem( i18n("75 %") );
    completedCombo->insertItem( i18n("Completed") );
    connect(completedCombo, SIGNAL(activated(int)),
	    this, SLOT(setModified()));

    //    ypos += ystep;

    priorityLabel = new QLabel( this, "Label_3" );
//    priorityLabel->setGeometry( 360, ypos, 70, 20 );
    priorityLabel->setText( i18n("Priority") );
    priorityLabel->setAlignment( 289 );
    priorityLabel->setMargin( -1 );

    priorityCombo = new QComboBox( FALSE, this, "ComboBox_10" );
//    priorityCombo->setGeometry( 430, ypos, 100, 20 );
    priorityCombo->setSizeLimit( 10 );
    priorityCombo->insertItem( i18n("1 (Highest)") );
    priorityCombo->insertItem( i18n("2") );
    priorityCombo->insertItem( i18n("3") );
    priorityCombo->insertItem( i18n("4") );
    priorityCombo->insertItem( i18n("5 (lowest)") );
    connect(priorityCombo, SIGNAL(activated(int)),
	    this, SLOT(setModified()));

    summaryLabel = new QLabel( this, "Label_1" );
//    summaryLabel->setGeometry( 10, 40, 70, 20 );
    summaryLabel->setText( i18n("Summary:") );
    summaryLabel->setAlignment( 289 );
    summaryLabel->setMargin( -1 );

    summaryEdit = new QLineEdit( this, "LineEdit_1" );
//    summaryEdit->setGeometry( 80, 40, 510, 20 );
    connect(summaryEdit, SIGNAL(textChanged(const QString &)),
	    this, SLOT(setModified()));

    descriptionEdit = new QMultiLineEdit( this, "MultiLineEdit_1" );
//    descriptionEdit->setGeometry( 10, 100, 580, 250 );
    descriptionEdit->insertLine( "" );
    descriptionEdit->setReadOnly( FALSE );
    descriptionEdit->setOverwriteMode( FALSE );
    connect(descriptionEdit, SIGNAL(textChanged()),
	    this, SLOT(setModified()));

    ownerLabel = new QLabel( this, "Label_7" );
//    ownerLabel->setGeometry( 10, 10, 200, 20 );
    ownerLabel->setText( i18n("Owner:") );
    ownerLabel->setAlignment( 289 );
    ownerLabel->setMargin( -1 );

    privateButton = new QCheckBox( this, "CheckBox_3" );
//    privateButton->setGeometry( 520, 360, 70, 20 );
    privateButton->setText( i18n("Private") );
    connect(privateButton, SIGNAL(toggled(bool)),
	    this, SLOT(setModified()));

    categoriesButton = new QPushButton( this, "PushButton_6" );
//    categoriesButton->setGeometry( 10, 360, 100, 20 );
    categoriesButton->setText( i18n("Categories...") );

    categoriesLabel = new QLabel( this, "LineEdit_7" );
    categoriesLabel->setFrameStyle(QFrame::Panel|QFrame::Sunken);
//    categoriesLabel->setGeometry( 120, 360, 390, 20 );
}

void TodoWinGeneral::initLayout()
{
  QBoxLayout *layoutTop = new QVBoxLayout(this,10);
  
  layoutTop->addWidget(ownerLabel);

  QBoxLayout *layoutSummary = new QHBoxLayout;
  layoutTop->addLayout(layoutSummary);
  layoutSummary->addWidget(summaryLabel);
  layoutSummary->addWidget(summaryEdit);
  
  layoutTop->addWidget(timeGroupBox);

  QBoxLayout *layoutCompletion = new QHBoxLayout;
  layoutTop->addLayout(layoutCompletion);
  layoutCompletion->addWidget(completedButton);
  layoutCompletion->addWidget(completedLabel);
  layoutCompletion->addWidget(completedCombo);
  layoutCompletion->addWidget(priorityLabel);
  layoutCompletion->addWidget(priorityCombo);
  
  layoutTop->addWidget(descriptionEdit);
  
  QBoxLayout *layoutCategories = new QHBoxLayout;
  layoutTop->addLayout(layoutCategories);
  layoutCategories->addWidget(categoriesButton);
  layoutCategories->addWidget(categoriesLabel,1);
  layoutCategories->addWidget(privateButton);
}

/*
TodoWinGeneral::~TodoWinGeneral()
{
}
  */

void TodoWinGeneral::setEnabled(bool enabled)
{
  // Enable all widgets, which are created in the initMisc method.
  // Labels are not enabled, since they are not active input controls.

  completedButton->setEnabled(enabled);
  completedCombo->setEnabled(enabled);
  priorityCombo->setEnabled(enabled);
  summaryEdit->setEnabled(enabled);
  descriptionEdit->setEnabled(enabled);
  privateButton->setEnabled(enabled);
  categoriesButton->setEnabled(enabled);

  emit modifiedEvent();
}

void TodoWinGeneral::dueStuffDisable(bool disable)
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

  emit modifiedEvent();
}

void TodoWinGeneral::timeStuffDisable(bool disable)
{
  if (disable) {
    startTimeEdit->hide();
  } else {
    startTimeEdit->show();
  }
  
  emit modifiedEvent();
}
