// 	$Id$	

#include <qtooltip.h>
#include <qfiledlg.h>

#include <klocale.h>
#include <kiconloader.h>

#include "wingeneral.h"
#include "wingeneral.moc"

WinGeneral::WinGeneral(QWidget* parent, const char* name)
  : QFrame( parent, name, 0 )
{
  alarmProgram = "";
/*
  resize( 600,400 );
  if(isTodo) {
      initTodoSpecific();
  } else {
      initTimeBox();
      initAlarmBox();
  }
  initMisc();

  setMinimumSize( 600, 400 );
  setMaximumSize( 32767, 32767 );

  summaryEdit->setFocus();
  */
}


void WinGeneral::initTimeBox()
{
/*
  timeGroupBox = new QGroupBox( this, "User_2" );
  timeGroupBox->setTitle(i18n("Appointment Time"));
  timeGroupBox->setGeometry( 10, 70, 580, 80 );

  startLabel = new QLabel( this, "Label_2" );
  startLabel->setGeometry( 70, 90, 70, 20 );
  startLabel->setText( i18n("Start Time:") );
  startLabel->setAlignment( 289 );
  startLabel->setMargin( -1 );

  startDateEdit = new KDateEdit(this);
  startDateEdit->setGeometry( 140, 90, 130, 20 );

  endLabel = new QLabel( this, "Label_3" );
  endLabel->setGeometry( 70, 120, 70, 20 );
  endLabel->setText( i18n("End Time:") );
  endLabel->setAlignment( 289 );
  endLabel->setMargin( -1 );

  
  endDateEdit = new KDateEdit(this);
  endDateEdit->setGeometry( 140, 120, 130, 20 );

  startTimeEdit = new KTimeEdit(this);
  startTimeEdit->setGeometry( 280, 90, 105, 25 );
      
  endTimeEdit = new KTimeEdit(this);
  endTimeEdit->setGeometry( 280, 120, 105, 25 );

  noTimeButton = new QCheckBox(this, "CheckBox_1" );
  noTimeButton->setGeometry( 400, 90, 140, 20 );
  noTimeButton->setText( i18n("No time associated") );

  connect(noTimeButton, SIGNAL(toggled(bool)), 
	  this, SLOT(timeStuffDisable(bool)));
  connect(noTimeButton, SIGNAL(toggled(bool)),
	  this, SLOT(alarmStuffDisable(bool)));
  connect(noTimeButton, SIGNAL(toggled(bool)),
	  this, SLOT(alarmStuffDisable(bool)));
  
  recursButton = new QCheckBox(this);
  recursButton->setGeometry( 400, 120, 140, 20);
  recursButton->setText(i18n("Recurring event"));
*/
}



void WinGeneral::initMisc()
{
}


void WinGeneral::pickAlarmSound()
{
}

void WinGeneral::pickAlarmProgram()
{
}

WinGeneral::~WinGeneral()
{
}

void WinGeneral::setEnabled(bool enabled)
{
  qDebug("WinGeneral::setEnabled()");
// This is the method used for eventwingeneral. It fails for the todowingeneral
/*
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
*/
}

void WinGeneral::timeStuffDisable(bool disable)
{
}

void WinGeneral::alarmStuffEnable(bool enable)
{
}

void WinGeneral::alarmStuffDisable(bool disable)
{
}

void WinGeneral::setModified()
{
  emit modifiedEvent();
}
