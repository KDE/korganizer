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
#include <kabapi.h>

#include "koevent.h"
#include "koprefs.h"

#include "koeditorwidgets.h"
#include "koeditorwidgets.moc"

KOEditorGeneralEvent::KOEditorGeneralEvent(int spacing,QWidget* parent,
                                           const char* name) :
  QWidget( parent, name)
{
  mSpacing = spacing;

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

  // time widgets on General and Recurrence tab are synchronized
  connect(startTimeEdit, SIGNAL(timeChanged(QTime, int)),
	  this, SLOT(startTimeChanged(QTime, int)));
  connect(endTimeEdit, SIGNAL(timeChanged(QTime, int)),
	  this, SLOT(endTimeChanged(QTime, int)));

  // date widgets on General and Recurrence tab are synchronized
  connect(startDateEdit, SIGNAL(dateChanged(QDate)),
	  this, SLOT(startDateChanged(QDate)));
  connect(endDateEdit, SIGNAL(dateChanged(QDate)),
	  this, SLOT(endDateChanged(QDate)));

  // recursion on/off
  connect(this,SIGNAL(recursChanged(bool)),SLOT(recurStuffEnable(bool)));
}

KOEditorGeneralEvent::~KOEditorGeneralEvent()
{
}

void KOEditorGeneralEvent::initTimeBox()
{
  timeGroupBox = new QGroupBox( 1,QGroupBox::Horizontal,
                                i18n("Appointment Time "),this, "User_2" );

  QFrame *timeBoxFrame = new QFrame(timeGroupBox,"TimeBoxFrame");

  QGridLayout *layoutTimeBox = new QGridLayout(timeBoxFrame,1,1);
  layoutTimeBox->setSpacing(mSpacing);

  startLabel = new QLabel( timeBoxFrame, "Label_2" );
  startLabel->setText( i18n("Start Time:") );
  layoutTimeBox->addWidget(startLabel,0,0);
  
  startDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(startDateEdit,0,1);

  startTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(startTimeEdit,0,2);

  noTimeButton = new QCheckBox(timeBoxFrame, "CheckBox_1" );
  noTimeButton->setText( i18n("No time associated") );
  layoutTimeBox->addWidget(noTimeButton,0,4);

  connect(noTimeButton, SIGNAL(toggled(bool)),SLOT(timeStuffDisable(bool)));
  connect(noTimeButton, SIGNAL(toggled(bool)),SLOT(alarmStuffDisable(bool)));
  connect(noTimeButton, SIGNAL(toggled(bool)),SIGNAL(allDayChanged(bool)));
  
  endLabel = new QLabel( timeBoxFrame, "Label_3" );
  endLabel->setText( i18n("End Time:") );
  layoutTimeBox->addWidget(endLabel,1,0);

  endDateEdit = new KDateEdit(timeBoxFrame);
  layoutTimeBox->addWidget(endDateEdit,1,1);

  endTimeEdit = new KTimeEdit(timeBoxFrame);
  layoutTimeBox->addWidget(endTimeEdit,1,2);

  recursButton = new QCheckBox(timeBoxFrame);
  recursButton->setText(i18n("Recurring event"));
  layoutTimeBox->addWidget(recursButton,1,4);

  QObject::connect(recursButton,SIGNAL(toggled(bool)),
                   SIGNAL(recursChanged(bool)));

  // some more layouting
  layoutTimeBox->setColStretch(3,1);
}

void KOEditorGeneralEvent::initMisc()
{
  summaryLabel = new QLabel( this, "Label_1" );
  summaryLabel->setText( i18n("Summary:") );
//  summaryLabel->setAlignment( 289 );
//  summaryLabel->setMargin( -1 );

  summaryEdit = new QLineEdit( this, "LineEdit_1" );

  freeTimeLabel = new QLabel( this, "Label_6" );
  freeTimeLabel->setText( i18n("Show Time As:") );
//  freeTimeLabel->setAlignment( 289 );
//  freeTimeLabel->setMargin( -1 );

  freeTimeCombo = new QComboBox( false, this, "ComboBox_1" );
//  freeTimeCombo->setSizeLimit( 10 );  // that's the default value anyway
  freeTimeCombo->insertItem( i18n("Busy") );
  freeTimeCombo->insertItem( i18n("Free") );

  descriptionEdit = new QMultiLineEdit( this, "MultiLineEdit_1" );
  descriptionEdit->insertLine( "" );
  descriptionEdit->setReadOnly( false );
  descriptionEdit->setOverwriteMode( false );

  ownerLabel = new QLabel( this, "Label_7" );
  ownerLabel->setText( i18n("Owner:") );
//  ownerLabel->setAlignment( 289 );
//  ownerLabel->setMargin( -1 );

  privateButton = new QCheckBox( this, "CheckBox_3" );
  privateButton->setText( i18n("Private") );

  categoriesButton = new QPushButton( this, "PushButton_6" );
  categoriesButton->setText( i18n("Categories...") );
  connect(categoriesButton,SIGNAL(clicked()),SIGNAL(openCategoryDialog()));

  categoriesLabel = new QLabel( this, "LineEdit_7" );
  categoriesLabel->setFrameStyle(QFrame::Panel|QFrame::Sunken);
}

void KOEditorGeneralEvent::initAlarmBox()
{
  QPixmap pixmap;

  alarmBell = new QLabel(this);
  alarmBell->setPixmap(UserIcon("bell"));

  alarmButton = new QCheckBox( this, "CheckBox_2" );
  alarmButton->setText( i18n("Reminder:") );

  alarmTimeEdit = new KRestrictedLine( this, "alarmTimeEdit",
				       "1234567890");
  alarmTimeEdit->setText("");

  alarmIncrCombo = new QComboBox(false, this);
  alarmIncrCombo->insertItem("minute(s)");
  alarmIncrCombo->insertItem("hour(s)");
  alarmIncrCombo->insertItem("day(s)");
  alarmIncrCombo->setMinimumHeight(20);

  alarmSoundButton = new QPushButton( this, "PushButton_4" );
  pixmap = UserIcon("playsound");
  //  alarmSoundButton->setText( i18n("WAV") );
  alarmSoundButton->setPixmap(pixmap);
  alarmSoundButton->setToggleButton(true);
  QToolTip::add(alarmSoundButton, "No sound set");

  alarmProgramButton = new QPushButton( this, "PushButton_5" );
  pixmap = UserIcon("runprog");
  //  alarmProgramButton->setText( i18n("PROG") );
  alarmProgramButton->setPixmap(pixmap);
  alarmProgramButton->setToggleButton(true);
  QToolTip::add(alarmProgramButton, "No program set");

  connect(alarmButton, SIGNAL(toggled(bool)),
	  this, SLOT(alarmStuffEnable(bool)));

  connect(alarmSoundButton, SIGNAL(clicked()),
	  this, SLOT(pickAlarmSound()));
  connect(alarmProgramButton, SIGNAL(clicked()),
	  this, SLOT(pickAlarmProgram()));
}

void KOEditorGeneralEvent::initLayout()
{
  QBoxLayout *layoutTop = new QVBoxLayout(this);
  layoutTop->setSpacing(mSpacing);
  
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

  layoutTop->addWidget(descriptionEdit,1);

  QBoxLayout *layoutCategories = new QHBoxLayout;
  layoutTop->addLayout(layoutCategories);
  layoutCategories->addWidget(categoriesButton);
  layoutCategories->addWidget(categoriesLabel,1);
  layoutCategories->addWidget(privateButton);
}

void KOEditorGeneralEvent::pickAlarmSound()
{
  QString prefix = KGlobal::dirs()->findResourceDir("appdata", "sounds/alert.wav"); 
  if (!alarmSoundButton->isOn()) {
    alarmSound = "";
    QToolTip::remove(alarmSoundButton);
    QToolTip::add(alarmSoundButton, "No sound set");
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
    }
  }
  if (alarmSound.isEmpty())
    alarmSoundButton->setOn(false);
}

void KOEditorGeneralEvent::pickAlarmProgram()
{
  if (!alarmProgramButton->isOn()) {
    alarmProgram = "";
    QToolTip::remove(alarmProgramButton);
    QToolTip::add(alarmProgramButton, "No program set");
  } else {
    QString fileName(QFileDialog::getOpenFileName(QString::null, "*", this));
    if (!fileName.isEmpty()) {
      alarmProgram = fileName;
      QToolTip::remove(alarmProgramButton);
      QString dispStr = "Running \"";
      dispStr += fileName.data();
      dispStr += "\"";
      QToolTip::add(alarmProgramButton, dispStr.data());
    }
  }
  if (alarmProgram.isEmpty())
    alarmProgramButton->setOn(false);
}

void KOEditorGeneralEvent::setEnabled(bool enabled)
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
}

void KOEditorGeneralEvent::timeStuffDisable(bool disable)
{
  if (disable) {
    startTimeEdit->hide();
    endTimeEdit->hide();
  } else {
    startTimeEdit->show();
    endTimeEdit->show();
  }
}

void KOEditorGeneralEvent::alarmStuffEnable(bool enable)
{
  alarmTimeEdit->setEnabled(enable);
  alarmSoundButton->setEnabled(enable);
  alarmProgramButton->setEnabled(enable);
}

void KOEditorGeneralEvent::alarmStuffDisable(bool disable)
{
  alarmTimeEdit->setEnabled(!disable);
  alarmSoundButton->setEnabled(!disable);
  alarmProgramButton->setEnabled(!disable);
}

void KOEditorGeneralEvent::recurStuffEnable(bool enable)
{
  if (enable) {
    startDateEdit->hide();
    endDateEdit->hide();
  } else {
    startDateEdit->show();
    endDateEdit->show();
  }
}

void KOEditorGeneralEvent::setDateTimes(QDateTime start, QDateTime end)
{
//  qDebug("KOEditorGeneralEvent::setDateTimes(): Start DateTime: %s",
//         start.toString().latin1());

  startDateEdit->setDate(start.date());
  startTimeEdit->setTime(start.time());
  endDateEdit->setDate(end.date());
  endTimeEdit->setTime(end.time());

  currStartDateTime = start;
  currEndDateTime = end;
}

void KOEditorGeneralEvent::setCategories(QString str)
{
  categoriesLabel->setText(str);
}

void KOEditorGeneralEvent::startTimeChanged(QTime newtime, int wrapval)
{
//  qDebug("KOEditorGeneralEvent::startTimeChanged");

  int secsep;

  secsep = currStartDateTime.secsTo(currEndDateTime);
  
  currStartDateTime = currStartDateTime.addDays(wrapval);
  currStartDateTime.setTime(newtime);

  currEndDateTime = currStartDateTime.addSecs(secsep);
  endTimeEdit->setTime(currEndDateTime.time());
  
  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorGeneralEvent::endTimeChanged(QTime newtime, int wrapval)
{
  QDateTime newdt(currEndDateTime.addDays(wrapval).date(), newtime);

  if(newdt < currStartDateTime) {
    // oops, can't let that happen.
    newdt = currStartDateTime;
    endTimeEdit->setTime(newdt.time());
  }
  currEndDateTime = newdt;
  
  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorGeneralEvent::startDateChanged(QDate newdate)
{
  int daysep;
  daysep = currStartDateTime.daysTo(currEndDateTime);
  
  currStartDateTime.setDate(newdate);
  currEndDateTime.setDate(currStartDateTime.date().addDays(daysep));

  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorGeneralEvent::endDateChanged(QDate newdate)
{
  QDateTime newdt(newdate, currEndDateTime.time());

  if(newdt < currStartDateTime) {
    // oops, we can't let that happen.
    newdt = currStartDateTime;
    endDateEdit->setDate(newdt.date());
    endTimeEdit->setTime(newdt.time());
  }
  currEndDateTime = newdt;

  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorGeneralEvent::setDefaults(QDateTime from,QDateTime to,bool allDay)
{
  ownerLabel->setText(i18n("Owner: ") + KOPrefs::instance()->mName);

  noTimeButton->setChecked(allDay);
  timeStuffDisable(allDay);
  alarmStuffDisable(allDay);

  setDateTimes(from,to);

  recursButton->setChecked(false);
//  recurStuffEnable(false);

  QString alarmText(QString::number(KOPrefs::instance()->mAlarmTime));
  int pos = alarmText.find(' ');
  if (pos >= 0)
    alarmText.truncate(pos);
  alarmTimeEdit->setText(alarmText.data());
  alarmStuffEnable(false);
}

void KOEditorGeneralEvent::readEvent(KOEvent *event)
{
  QString tmpStr;
  QDateTime tmpDT;
  int i;

  summaryEdit->setText(event->getSummary());
  descriptionEdit->setText(event->getDescription());

  // organizer information
  ownerLabel->setText(i18n("Owner: ") + event->getOrganizer());

  // the rest is for the events only
  noTimeButton->setChecked(event->doesFloat());
  timeStuffDisable(event->doesFloat());
  alarmStuffDisable(event->doesFloat());

  setDateTimes(event->getDtStart(),event->getDtEnd());

  recursButton->setChecked(event->doesRecur());
  recurStuffEnable(event->doesRecur());

  privateButton->setChecked((event->getSecrecy() > 0) ? true : false);

  // set up alarm stuff
  alarmButton->setChecked(event->getAlarmRepeatCount());
  if (alarmButton->isChecked()) {
    alarmStuffEnable(true);
    tmpDT = event->getAlarmTime();
    i = tmpDT.secsTo(currStartDateTime);
    i = i / 60; // make minutes
    if (i % 60 == 0) { // divides evenly into hours?
      i = i / 60;
      alarmIncrCombo->setCurrentItem(1);
    }
    if (i % 24 == 0) { // divides evenly into days?
      i = i / 24;
      alarmIncrCombo->setCurrentItem(2);
    }

    alarmTimeEdit->setText(QString::number(i));

    if (!event->getProgramAlarmFile().isEmpty()) {
      alarmProgram = event->getProgramAlarmFile();
      alarmProgramButton->setOn(true);
      QString dispStr = "Running \"";
      dispStr += alarmProgram.data();
      dispStr += "\"";
      QToolTip::add(alarmProgramButton, dispStr.data());
    }
    if (!event->getAudioAlarmFile().isEmpty()) {
      alarmSound = event->getAudioAlarmFile();
      alarmSoundButton->setOn(true);
      QString dispStr = "Playing \"";
      dispStr += alarmSound.data();
      dispStr += "\"";
      QToolTip::add(alarmSoundButton, dispStr.data());
    }
  } else {
    alarmStuffEnable(false);
  }

  if (event->getTransparency() > 0)
    freeTimeCombo->setCurrentItem(1);
  // else it is implicitly 0 (i.e. busy)

  setCategories(event->getCategoriesStr());
}

void KOEditorGeneralEvent::writeEvent(KOEvent *event)
{
  QDate tmpDate;
  QTime tmpTime;
  QDateTime tmpDT;

  // temp. until something better happens.
  QString tmpStr;
  bool ok;
  int j;

  event->setSummary(summaryEdit->text());
  event->setDescription(descriptionEdit->text());
  event->setCategories(categoriesLabel->text());
  event->setSecrecy(privateButton->isChecked() ? 1 : 0);

  if (noTimeButton->isChecked()) {
    event->setFloats(true);
    // need to change this.
    tmpDate = startDateEdit->getDate();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtStart(tmpDT);

    tmpDate = endDateEdit->getDate();
    tmpTime.setHMS(0,0,0);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtEnd(tmpDT);
  } else {
    event->setFloats(false);
    
    // set date/time end
    tmpDate = endDateEdit->getDate();
    tmpTime = endTimeEdit->getTime(ok);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtEnd(tmpDT);

    // set date/time start
    tmpDate = startDateEdit->getDate();
    tmpTime = startTimeEdit->getTime(ok);
    tmpDT.setDate(tmpDate);
    tmpDT.setTime(tmpTime);
    event->setDtStart(tmpDT);    
  } // check for float

  // alarm stuff
  if (alarmButton->isChecked()) {
    event->setAlarmRepeatCount(1);
    tmpStr = alarmTimeEdit->text();
    j = tmpStr.toInt() * -60;
    if (alarmIncrCombo->currentItem() == 1)
      j = j * 60;
    else if (alarmIncrCombo->currentItem() == 2)
      j = j * (60 * 24);

    tmpDT = event->getDtStart();
    tmpDT = tmpDT.addSecs(j);
    event->setAlarmTime(tmpDT);
    if (!alarmProgram.isEmpty() && alarmProgramButton->isOn())
      event->setProgramAlarmFile(alarmProgram);
    else
      event->setProgramAlarmFile("");
    if (!alarmSound.isEmpty() && alarmSoundButton->isOn())
      event->setAudioAlarmFile(alarmSound);
    else
      event->setAudioAlarmFile("");
  } else {
    event->setAlarmRepeatCount(0);
    event->setProgramAlarmFile("");
    event->setAudioAlarmFile("");
  }
  
  // note, that if on the details tab the "Transparency" option is implemented,
  // we will have to change this to suit.
  event->setTransparency(freeTimeCombo->currentItem());
}


AttendeeListItem::AttendeeListItem(Attendee *a, QListView *parent) :
  QListViewItem(parent)
{
  mAttendee = new Attendee(*a);
  updateItem();
}

AttendeeListItem::~AttendeeListItem()
{
  delete mAttendee;
}

void AttendeeListItem::updateItem()
{
  setText(0,mAttendee->getName());
  setText(1,(!mAttendee->getEmail().isEmpty()) ? mAttendee->getEmail().data() :
                                                 " ");
  setText(2,mAttendee->getRoleStr());
  setText(3,mAttendee->getStatusStr());
//  setText(4,(mAttendee->RSVP() && !mAttendee->getEmail().isEmpty()) ?
//            "Y" : "N");
  if (mAttendee->RSVP() && !mAttendee->getEmail().isEmpty())
    setPixmap(4,UserIcon("mailappt"));
  else
    setPixmap(4,UserIcon("nomailappt"));
}


KOEditorDetails::KOEditorDetails (int spacing,QWidget* parent,const char* name)
  : QWidget( parent, name)
{
  mSpacing = spacing;

  topLayout = new QVBoxLayout(this);
  topLayout->setSpacing(mSpacing);

  initAttendee();
  //initAttach();
  initMisc();
}

void KOEditorDetails::initAttendee()
{
  attendeeGroupBox = new QGroupBox(1,Horizontal,i18n("Attendee Information"),
                                   this);
  topLayout->addWidget(attendeeGroupBox);

  attendeeListBox = new QListView( attendeeGroupBox, "attendeeListBox" );
  attendeeListBox->addColumn(i18n("Name"),180);
  attendeeListBox->addColumn(i18n("Email"),180);
  attendeeListBox->addColumn(i18n("Role"),60);
  attendeeListBox->addColumn(i18n("Status"),100);
  attendeeListBox->addColumn(i18n("RSVP"),35);

  connect(attendeeListBox, SIGNAL(clicked(QListViewItem *)),
	  this, SLOT(attendeeListHilite(QListViewItem *)));
  connect(attendeeListBox, SIGNAL(doubleClicked(QListViewItem *)),
	  this, SLOT(attendeeListAction(QListViewItem *)));

  QHBox *nameBox = new QHBox(attendeeGroupBox);

  attendeeLabel = new QLabel(nameBox);
  attendeeLabel->setText(i18n("Attendee Name:"));

  attendeeEdit = new QLineEdit(nameBox);
  attendeeEdit->setText( "" );

  QLabel *emailLabel = new QLabel(nameBox);
  emailLabel->setText(i18n("Email Address:"));
  
  emailEdit = new QLineEdit(nameBox);
  emailEdit->setText("");


  QHBox *roleBox = new QHBox(attendeeGroupBox);
  
  attendeeRoleLabel = new QLabel(roleBox);
  attendeeRoleLabel->setText(i18n("Role:"));
//  attendeeRoleLabel->setAlignment(AlignVCenter|AlignRight);

  attendeeRoleCombo = new QComboBox(false,roleBox);
  attendeeRoleCombo->insertItem( i18n("Attendee") );
  attendeeRoleCombo->insertItem( i18n("Organizer") );
  attendeeRoleCombo->insertItem( i18n("Owner") );
  attendeeRoleCombo->insertItem( i18n("Delegate") );

  statusLabel = new QLabel(roleBox);
  statusLabel->setText( i18n("Status:") );
//  statusLabel->setAlignment(AlignVCenter|AlignRight);

  statusCombo = new QComboBox(false,roleBox);
  statusCombo->insertItem( i18n("Needs Action") );
  statusCombo->insertItem( i18n("Accepted") );
  statusCombo->insertItem( i18n("Sent") );
  statusCombo->insertItem( i18n("Tentative") );
  statusCombo->insertItem( i18n("Confirmed") );
  statusCombo->insertItem( i18n("Declined") );
  statusCombo->insertItem( i18n("Completed") );
  statusCombo->insertItem( i18n("Delegated") );

//  subLayout->addStretch();

  attendeeRSVPButton = new QCheckBox(roleBox);
  attendeeRSVPButton->setText(i18n("Request Response"));

  KButtonBox *buttonBox = new KButtonBox(attendeeGroupBox);

  addAttendeeButton = buttonBox->addButton(i18n("&Add"));
  connect(addAttendeeButton, SIGNAL(clicked()),
	  this, SLOT(addNewAttendee()));

  addAttendeeButton = buttonBox->addButton(i18n("&Modify"));
  connect(addAttendeeButton, SIGNAL(clicked()),
	  this, SLOT(updateAttendee()));

  addressBookButton = buttonBox->addButton(i18n("Address &Book..."));
  connect(addressBookButton, SIGNAL(clicked()),
          this, SLOT(openAddressBook()));

  removeAttendeeButton = buttonBox->addButton(i18n("&Remove"));
  connect(removeAttendeeButton, SIGNAL(clicked()),
	  this, SLOT(removeAttendee()));
//  buttonBox->layout();

//  layout->addWidget(buttonBox);
}
    
void KOEditorDetails::initAttach()
{
/*
  attachGroupBox = new QGroupBox( this, "User_2" );
  attachGroupBox->setGeometry( 10, 190, 580, 100 );
  attachGroupBox->setMinimumSize( 10, 10 );
  attachGroupBox->setMaximumSize( 32767, 32767 );
  attachGroupBox->setEnabled(false);

  attachFileButton = new QPushButton( this, "PushButton_3" );
  attachFileButton->setGeometry( 20, 200, 100, 20 );
  attachFileButton->setMinimumSize( 10, 10 );
  attachFileButton->setMaximumSize( 32767, 32767 );
  attachFileButton->setText( i18n("Attach...") );
  attachFileButton->setEnabled(false);

  removeFileButton = new QPushButton( this, "PushButton_4" );
  removeFileButton->setGeometry( 20, 260, 100, 20 );
  removeFileButton->setMinimumSize( 10, 10 );
  removeFileButton->setMaximumSize( 32767, 32767 );
  removeFileButton->setText( i18n("Remove") );
  removeFileButton->setEnabled(false);

  attachListBox = new KTabListBox( this, "ListBox_2" );
  attachListBox->setGeometry( 140, 200, 440, 80 );
  attachListBox->setMinimumSize( 10, 10 );
  attachListBox->setMaximumSize( 32767, 32767 );
  attachListBox->setEnabled(false);

  saveFileAsButton = new QPushButton( this, "PushButton_5" );
  saveFileAsButton->setGeometry( 20, 230, 100, 20 );
  saveFileAsButton->setMinimumSize( 10, 10 );
  saveFileAsButton->setMaximumSize( 32767, 32767 );
  saveFileAsButton->setText( i18n("Save As...") );
  saveFileAsButton->setEnabled(false);
*/
}

void KOEditorDetails::initMisc()
{
  QGroupBox *groupBox = new QGroupBox(1,Horizontal,this);
  topLayout->addWidget(groupBox);

  QHBox *catBox = new QHBox(groupBox);

  categoriesButton = new QPushButton(catBox);
  categoriesButton->setText(i18n("Categories..."));
  connect(categoriesButton,SIGNAL(clicked()),SIGNAL(openCategoryDialog()));

  categoriesLabel = new QLabel(catBox);
  categoriesLabel->setFrameStyle(QFrame::Panel|QFrame::Sunken);
  categoriesLabel->setText( "" );

  /*  locationLabel = new QLabel(groupBox);
  locationLabel->setText( i18n("Location:") );
  locationLabel->setFixedSize(locationLabel->sizeHint());
  layout->addWidget(locationLabel);*/


  /*  subLayout = new QHBoxLayout();
  layout->addLayout(subLayout);

    resourceButton = new QPushButton(groupBox);
  resourceButton->setText( i18n("Resources...") );
  resourceButton->setFixedSize(resourceButton->sizeHint());
  subLayout->addWidget(resourceButton);

  resourcesEdit = new QLineEdit(groupBox);
  resourcesEdit->setText( "" );
  resourcesEdit->setFixedHeight(resourcesEdit->sizeHint().height());
  resourcesEdit->setMinimumWidth(resourcesEdit->sizeHint().width());
  subLayout->addWidget(resourcesEdit);

  subLayout = new QHBoxLayout();
  layout->addLayout(subLayout);
  transparencyLabel = new QLabel( groupBox);
  transparencyLabel->setText( i18n("Transparency:") );
  transparencyLabel->setFixedSize(transparencyLabel->sizeHint());
  subLayout->addWidget(transparencyLabel);

  transparencyAmountLabel = new QLabel(groupBox);
  transparencyAmountLabel->setText( "0" );
  transparencyAmountLabel->setMinimumSize(transparencyAmountLabel->sizeHint());
  subLayout->addWidget(transparencyAmountLabel);*/
}

KOEditorDetails::~KOEditorDetails()
{
}

void KOEditorDetails::setEnabled(bool enabled)
{
// This doesn't correspond to the available widgets at the moment
/*
  attendeeEdit->setEnabled(enabled);
  addAttendeeButton->setEnabled(enabled);
  removeAttendeeButton->setEnabled(enabled);
  attachFileButton->setEnabled(enabled);
  saveFileAsButton->setEnabled(enabled);
  addressBookButton->setEnabled(enabled);
*/
  categoriesButton->setEnabled(enabled);
  categoriesLabel->setEnabled(enabled);
/*
  attendeeRoleCombo->setEnabled(enabled);
  //  attendeeRSVPButton->setEnabled(enabled);
  statusCombo->setEnabled(enabled);
  resourceButton->setEnabled(enabled);
  resourcesEdit->setEnabled(enabled);
*/
}

void KOEditorDetails::removeAttendee()
{
  AttendeeListItem *aItem = (AttendeeListItem *)attendeeListBox->currentItem();
  if (!aItem) return;

  delete aItem;
}

void KOEditorDetails::attendeeListHilite(QListViewItem *item)
{
  Attendee *a = ((AttendeeListItem *)item)->attendee(); 

  attendeeEdit->setText(a->getName());
  emailEdit->setText(a->getEmail());
  attendeeRoleCombo->setCurrentItem(a->getRole());
  statusCombo->setCurrentItem(a->getStatus());
  attendeeRSVPButton->setChecked(a->RSVP());
}

void KOEditorDetails::attendeeListAction(QListViewItem *item)
{
  return;

  /*  switch (col) {
  case 0:
    // do something with the attendee here.
    break;
  case 4:
    if (strcmp(attendeeListBox->text(row, col), "Y") == 0)
      attendeeListBox->changeItemPart("N", row, col);
    else
      attendeeListBox->changeItemPart("Y", row, col);
    break;
    }*/
}

void KOEditorDetails::openAddressBook()
{
  KabAPI addrDialog(this);

  if (addrDialog.init() != AddressBook::NoError) {
    QMessageBox::critical(this, i18n("KOrganizer Error"),
			  i18n("Unable to open address book."));
    return;
  }
  KabKey key;
  AddressBook::Entry entry;
  if (addrDialog.exec()) {
    if (addrDialog.getEntry(entry, key) == AddressBook::NoError) {
      // get name -- combo of first and last names
      QString nameStr;
      addrDialog.addressbook()->literalName(entry, nameStr, true, false);
      attendeeEdit->setText(nameStr);

      // take first email address
      if (!entry.emails.isEmpty() && entry.emails.first().length()>0)
      	emailEdit->setText(entry.emails.first());      
    } else {
      QMessageBox::warning(this, i18n("KOrganizer Error"),
			   i18n("Error getting entry from address book."));
    }
  }
}


void KOEditorDetails::updateAttendee()
{
  AttendeeListItem *aItem = (AttendeeListItem *)attendeeListBox->currentItem();
  if (!aItem) return;

  delete aItem;
  addNewAttendee();
}

void KOEditorDetails::addNewAttendee()
{
  // don;t do anything on a blank name
  if (QString(attendeeEdit->text()).stripWhiteSpace().isEmpty())
    return;

  Attendee *a;
  
  a = new Attendee(attendeeEdit->text());

  // this is cool.  If they didn't enter an email address,
  // try to look it up in the address book and fill it in for them.
  if (QString(emailEdit->text()).stripWhiteSpace().isEmpty()) {
    KabAPI addrBook;
    QString name;
    list<AddressBook::Entry> entries;
    name = attendeeEdit->text();
    if (addrBook.init() == AddressBook::NoError) {
      if (addrBook.getEntryByName(name, entries, 1) == AddressBook::NoError) {
	debug("positive match");
	// take first email address
	if (!entries.front().emails.isEmpty() && 
	    entries.front().emails.first().length()>0)
	  emailEdit->setText(entries.front().emails.first());
      }
    }
  }

  a->setEmail(emailEdit->text());
  a->setRole(attendeeRoleCombo->currentItem());
  a->setStatus(statusCombo->currentItem());
  a->setRSVP(attendeeRSVPButton->isChecked() ? true : false);

  insertAttendee(a);

  // zero everything out for a new one
  attendeeEdit->setText("");
  emailEdit->setText("");
  attendeeRoleCombo->setCurrentItem(0);
  statusCombo->setCurrentItem(0);
  attendeeRSVPButton->setChecked(true);
}

void KOEditorDetails::insertAttendee(Attendee *a)
{
  mAttendeeList.append(new AttendeeListItem(a,attendeeListBox));
}

void KOEditorDetails::setCategories(QString str)
{
  categoriesLabel->setText(str);
}

void KOEditorDetails::setDefaults()
{
  attendeeRSVPButton->setChecked(true);
}

void KOEditorDetails::readEvent(KOEvent *event)
{
  // attendee information
  // first remove whatever might be here
  mAttendeeList.clear();
  QList<Attendee> tmpAList = event->getAttendeeList();
  Attendee *a;
  for (a = tmpAList.first(); a; a = tmpAList.next())
    insertAttendee(new Attendee (*a));

  //  Details->attachListBox->insertItem(i18n("Not implemented yet."));
  
  // set the status combobox
  statusCombo->setCurrentItem(event->getStatus());

  setCategories(event->getCategoriesStr());
}

void KOEditorDetails::writeEvent(KOEvent *event)
{
  event->clearAttendees();
  unsigned int i;
  for (i = 0; i < mAttendeeList.count(); i++)
    event->addAttendee(new Attendee(*(mAttendeeList.at(i)->attendee())));

  // we should remove this.
  event->setStatus(statusCombo->currentItem());
}


KOEditorRecurrence::KOEditorRecurrence(int spacing,QWidget* parent,
                                       const char* name) :
  QWidget( parent, name, 0 )
{
  mSpacing = spacing;

  mEnabled = false;

  initMain();
  initExceptions();

  initLayout();

  // time widgets on General and Recurrence tab are synchronized
  connect(startTimeEdit, SIGNAL(timeChanged(QTime, int)),
	  this, SLOT(startTimeChanged(QTime, int)));
  connect(endTimeEdit, SIGNAL(timeChanged(QTime, int)),
	  this, SLOT(endTimeChanged(QTime, int)));

  // date widgets on General and Recurrence tab are synchronized
  connect(startDateEdit, SIGNAL(dateChanged(QDate)),
	  this, SLOT(startDateChanged(QDate)));
  connect(endDateEdit, SIGNAL(dateChanged(QDate)),
	  this, SLOT(endDateChanged(QDate)));
}

KOEditorRecurrence::~KOEditorRecurrence()
{  
}

void KOEditorRecurrence::initLayout()
{
  QGridLayout *layoutTop = new QGridLayout(this,1,1);
  layoutTop->setSpacing(mSpacing);
  
  layoutTop->addMultiCellWidget(timeGroupBox,0,0,0,1);
  layoutTop->addMultiCellWidget(ruleGroupBox,1,1,0,1);
  layoutTop->addWidget(rangeGroupBox,2,0);
  layoutTop->addWidget(exceptionGroupBox,2,1);
}

void KOEditorRecurrence::initMain()
{
  // Create the appointement time group box, which is 
  timeGroupBox = new QGroupBox( 1,QGroupBox::Horizontal,
                                i18n("Appointment Time "),this, "User_2" );

  QFrame *timeFrame = new QFrame(timeGroupBox,"timeFrame");
  QBoxLayout *layoutTimeFrame = new QHBoxLayout(timeFrame);
  layoutTimeFrame->setSpacing(mSpacing);
  
  startLabel    = new QLabel(i18n("Start:  "), timeFrame);
  endLabel      = new QLabel(i18n("End:  "), timeFrame);

  startTimeEdit = new KTimeEdit(timeFrame);
  endTimeEdit   = new KTimeEdit(timeFrame);
  durationLabel = new QLabel(timeFrame);

  layoutTimeFrame->addWidget(startLabel);
  layoutTimeFrame->addWidget(startTimeEdit);
  layoutTimeFrame->addSpacing(10);
  layoutTimeFrame->addWidget(endLabel);
  layoutTimeFrame->addWidget(endTimeEdit);
  layoutTimeFrame->addSpacing(10);
  layoutTimeFrame->addWidget(durationLabel);
  layoutTimeFrame->addStretch(1);


  // Create the recursion rule Group box. This will also hold the
  // daily, weekly, monthly and yearly recursion rule frames which
  // specify options individual to each of these distinct sections of
  // the recursion rule. Note that each frame will be made visible by
  // the selection of a radio button.
  ruleGroupBox = new QGroupBox( 1,QGroupBox::Horizontal,
                                i18n("Recurrence Rule"),this,
				 "ruleGroupBox" );

  ruleFrame = new QFrame(ruleGroupBox,"ruleFrame");
  QBoxLayout *layoutRuleFrame = new QVBoxLayout(ruleFrame);

  ruleButtonGroup = new QButtonGroup(1,Horizontal,ruleFrame);
  ruleButtonGroup->setFrameStyle(QFrame::NoFrame);
  dailyButton     = new QRadioButton(i18n("Daily"), ruleButtonGroup);
  weeklyButton    = new QRadioButton(i18n("Weekly"), ruleButtonGroup);
  monthlyButton   = new QRadioButton(i18n("Monthly"), ruleButtonGroup);
  yearlyButton    = new QRadioButton(i18n("Yearly"), ruleButtonGroup);

  ruleSepFrame = new QFrame(ruleFrame);
  ruleSepFrame->setFrameStyle(QFrame::VLine|QFrame::Sunken);

  initDaily();
  initWeekly();
  initMonthly();
  initYearly();

  ruleStack = new QWidgetStack(ruleFrame);
  ruleStack->addWidget(dailyFrame,0);
  ruleStack->addWidget(weeklyFrame,1);
  ruleStack->addWidget(monthlyFrame,2);
  ruleStack->addWidget(yearlyFrame,3);

  QBoxLayout *layoutRule = new QHBoxLayout;
  layoutRuleFrame->addLayout(layoutRule);
  layoutRule->addWidget(ruleButtonGroup);
  layoutRule->addWidget(ruleSepFrame);
  layoutRule->addStretch(1);
  layoutRule->addWidget(ruleStack);
  layoutRule->addStretch(1);

  advancedRuleButton = new QCheckBox(i18n("Enable Advanced Rule:"),
                                     ruleFrame);
  advancedRuleButton->setEnabled(false);

  advancedRuleEdit = new QLineEdit(ruleFrame);
  advancedRuleEdit->setText( "" );
  advancedRuleEdit->setMaxLength( 32767 );
  advancedRuleEdit->setEnabled(false);

  QBoxLayout *layoutAdvancedRule = new QHBoxLayout;
  layoutRuleFrame->addLayout(layoutAdvancedRule);
  layoutAdvancedRule->addWidget(advancedRuleButton);
  layoutAdvancedRule->addWidget(advancedRuleEdit);

  connect(dailyButton, SIGNAL(toggled(bool)), 
	  this, SLOT(showDaily(bool)));
  connect(weeklyButton, SIGNAL(toggled(bool)), 
	  this, SLOT(showWeekly(bool)));
  connect(monthlyButton, SIGNAL(toggled(bool)), 
	  this, SLOT(showMonthly(bool)));
  connect(yearlyButton, SIGNAL(toggled(bool)),
	  this, SLOT(showYearly(bool)));

  // Create the recursion range Group Box which contains the controls
  // specific to determining how long the recursion is to last.
  rangeGroupBox = new QGroupBox( 1,QGroupBox::Horizontal,
                                 i18n("Recurrence Range"),this,
				 "rangeGroupBox" );

  rangeButtonGroup = new QButtonGroup( rangeGroupBox,"rangeButtonGroup");
  rangeButtonGroup->setFrameStyle(QFrame::NoFrame);
//  rangeButtonGroup->setExclusive(true);

  startDateLabel = new QLabel(i18n("Begin On:"), rangeButtonGroup);
  startDateEdit = new KDateEdit(rangeButtonGroup);
  noEndDateButton = new QRadioButton(i18n("No Ending Date"), rangeButtonGroup);
  endDurationButton = new QRadioButton(i18n("End after"), rangeButtonGroup);
  endDurationEdit = new QLineEdit(rangeButtonGroup);
  endDurationLabel = new QLabel(i18n("occurrence(s)"), rangeButtonGroup);
  endDateButton = new QRadioButton(i18n("End by:"), rangeButtonGroup);
  endDateEdit = new KDateEdit(rangeButtonGroup);

  // Construct layout for recurrence range box
  QBoxLayout *layoutRange = new QVBoxLayout(rangeButtonGroup,5);
    
  QBoxLayout *layoutStart = new QHBoxLayout;
  layoutRange->addLayout(layoutStart);
  layoutStart->addWidget(startDateLabel);
  layoutStart->addWidget(startDateEdit,AlignVCenter);

  layoutRange->addWidget(noEndDateButton);
  
  QBoxLayout *layoutEndDuration = new QHBoxLayout;
  layoutRange->addLayout(layoutEndDuration);
  layoutEndDuration->addWidget(endDurationButton);
  layoutEndDuration->addWidget(endDurationEdit);
  layoutEndDuration->addWidget(endDurationLabel);

  QBoxLayout *layoutEndDate = new QHBoxLayout;
  layoutRange->addLayout(layoutEndDate);
  layoutEndDate->addWidget(endDateButton);
  layoutEndDate->addWidget(endDateEdit,AlignLeft);
  
  connect(noEndDateButton, SIGNAL(toggled(bool)),
	  this, SLOT(disableRange(bool)));
  connect(endDurationButton, SIGNAL(toggled(bool)),
	  this, SLOT(enableDurationRange(bool)));
  connect(endDateButton, SIGNAL(toggled(bool)),
	  this, SLOT(enableDateRange(bool)));
}

void KOEditorRecurrence::showDaily(bool on)
{
  if (on) ruleStack->raiseWidget(dailyFrame);
}

void KOEditorRecurrence::showWeekly(bool on)
{
  if (on) ruleStack->raiseWidget(weeklyFrame);
}

void KOEditorRecurrence::showMonthly(bool on)
{
  if (on) ruleStack->raiseWidget(monthlyFrame);
}

void KOEditorRecurrence::showYearly(bool on)
{
  if (on) ruleStack->raiseWidget(yearlyFrame);
}

void KOEditorRecurrence::disableRange(bool on)
{
  if (on) {
    endDateEdit->setEnabled(false);
    endDurationEdit->setEnabled(false);
    endDurationLabel->setEnabled(false);
    return;
  }
}

void KOEditorRecurrence::enableDurationRange(bool on)
{
  if (on) {
    endDurationEdit->setEnabled(true);
    endDurationLabel->setEnabled(true);
    endDateEdit->setEnabled(false);
    return;
  }
}

void KOEditorRecurrence::enableDateRange(bool on)
{
  if (on) {
    endDateEdit->setEnabled(true);
    endDurationEdit->setEnabled(false);
    endDurationLabel->setEnabled(false);
    return;
  }
}

void KOEditorRecurrence::initDaily()
{
  dailyFrame = new QFrame(ruleFrame);
  dailyFrame->setFrameStyle(QFrame::NoFrame);

  everyNDays = new QLabel(i18n("Recur every"), dailyFrame);

  nDaysEntry = new QLineEdit(dailyFrame);
  nDaysEntry->setText( "" );
  nDaysEntry->setMaxLength( 3 );

  nDaysLabel = new QLabel(i18n("day(s)"), dailyFrame);

  QBoxLayout *layoutDaily = new QHBoxLayout(dailyFrame,10);
  layoutDaily->addWidget(everyNDays);
  layoutDaily->addWidget(nDaysEntry);
  layoutDaily->addWidget(nDaysLabel);
}

void KOEditorRecurrence::initWeekly()
{
  weeklyFrame = new QFrame(ruleFrame);
  weeklyFrame->setFrameStyle(QFrame::NoFrame);

  everyNWeeks = new QLabel(i18n("Recur every"), weeklyFrame);

  nWeeksEntry = new QLineEdit(weeklyFrame);
  nWeeksEntry->setText("");
  nWeeksEntry->setMaxLength(2);

  nWeeksLabel = new QLabel(i18n("week(s) on:"), weeklyFrame);

  sundayBox    = new QCheckBox(i18n("Sun"), weeklyFrame);
  mondayBox    = new QCheckBox(i18n("Mon"), weeklyFrame);
  tuesdayBox   = new QCheckBox(i18n("Tue"), weeklyFrame);
  wednesdayBox = new QCheckBox(i18n("Wed"), weeklyFrame);
  thursdayBox  = new QCheckBox(i18n("Thu"), weeklyFrame);
  fridayBox    = new QCheckBox(i18n("Fri"), weeklyFrame);
  saturdayBox  = new QCheckBox(i18n("Sat"), weeklyFrame);

  QBoxLayout *layoutWeekly = new QVBoxLayout(weeklyFrame,10);

  QBoxLayout *layoutEveryN = new QHBoxLayout;
  layoutWeekly->addLayout(layoutEveryN);
  layoutEveryN->addWidget(everyNWeeks);
  layoutEveryN->addWidget(nWeeksEntry);
  layoutEveryN->addWidget(nWeeksLabel);
  
  QBoxLayout *layoutDays = new QHBoxLayout;
  layoutWeekly->addLayout(layoutDays);
  layoutDays->addWidget(sundayBox);
  layoutDays->addWidget(mondayBox);
  layoutDays->addWidget(tuesdayBox);
  layoutDays->addWidget(wednesdayBox);
  layoutDays->addWidget(thursdayBox);
  layoutDays->addWidget(fridayBox);
  layoutDays->addWidget(saturdayBox);
}

void KOEditorRecurrence::initMonthly()
{
  monthlyFrame = new QVBox(ruleFrame);
  monthlyFrame->setFrameStyle(QFrame::NoFrame);
//  monthlyFrame->hide();

  monthlyButtonGroup = new QButtonGroup(monthlyFrame);
  monthlyButtonGroup->setFrameStyle(QFrame::NoFrame);

  onNthDay          = new QRadioButton(i18n("Recur on the"), monthlyButtonGroup);
  nthDayEntry       = new QComboBox(false, monthlyButtonGroup);
  nthDayLabel       = new QLabel(i18n("day"), monthlyButtonGroup);

  onNthTypeOfDay    = new QRadioButton(i18n("Recur on the"), monthlyButtonGroup);
  nthNumberEntry    = new QComboBox(false, monthlyButtonGroup);
  nthTypeOfDayEntry = new QComboBox(false, monthlyButtonGroup);

  monthCommonLabel  = new QLabel(i18n("every"), monthlyButtonGroup);
  nMonthsEntry      = new QLineEdit(monthlyButtonGroup);
  nMonthsLabel      = new QLabel(i18n("month(s)"), monthlyButtonGroup);

  nthDayEntry->setSizeLimit( 7 );
  nthDayEntry->insertItem( i18n("1st") ); 
  nthDayEntry->insertItem( i18n("2nd") ); 
  nthDayEntry->insertItem( i18n("3rd") ); 
  nthDayEntry->insertItem( i18n("4th") ); 
  nthDayEntry->insertItem( i18n("5th") ); 
  nthDayEntry->insertItem( i18n("6th") ); 
  nthDayEntry->insertItem( i18n("7th") ); 
  nthDayEntry->insertItem( i18n("8th") ); 
  nthDayEntry->insertItem( i18n("9th") ); 
  nthDayEntry->insertItem( i18n("10th") );
  nthDayEntry->insertItem( i18n("11th") );
  nthDayEntry->insertItem( i18n("12th") );
  nthDayEntry->insertItem( i18n("13th") );
  nthDayEntry->insertItem( i18n("14th") );
  nthDayEntry->insertItem( i18n("15th") );
  nthDayEntry->insertItem( i18n("16th") );
  nthDayEntry->insertItem( i18n("17th") );
  nthDayEntry->insertItem( i18n("18th") );
  nthDayEntry->insertItem( i18n("19th") );
  nthDayEntry->insertItem( i18n("20th") );
  nthDayEntry->insertItem( i18n("21st") );
  nthDayEntry->insertItem( i18n("22nd") );
  nthDayEntry->insertItem( i18n("23rd") );
  nthDayEntry->insertItem( i18n("24th") );
  nthDayEntry->insertItem( i18n("25th") );
  nthDayEntry->insertItem( i18n("26th") );
  nthDayEntry->insertItem( i18n("27th") );
  nthDayEntry->insertItem( i18n("28th") );
  nthDayEntry->insertItem( i18n("29th") );
  nthDayEntry->insertItem( i18n("30th") );
  nthDayEntry->insertItem( i18n("31st") );

  nthNumberEntry->insertItem( i18n("1st") ); 
  nthNumberEntry->insertItem( i18n("2nd") );
  nthNumberEntry->insertItem( i18n("3rd") ); 
  nthNumberEntry->insertItem( i18n("4th") );
  nthNumberEntry->insertItem( i18n("5th") ); 

  nthTypeOfDayEntry->insertItem( i18n("Monday") );  
  nthTypeOfDayEntry->insertItem( i18n("Tuesday") ); 
  nthTypeOfDayEntry->insertItem( i18n("Wednesday") );
  nthTypeOfDayEntry->insertItem( i18n("Thursday") );
  nthTypeOfDayEntry->insertItem( i18n("Friday") );  
  nthTypeOfDayEntry->insertItem( i18n("Saturday") );
  nthTypeOfDayEntry->insertItem( i18n("Sunday") );  
  nthTypeOfDayEntry->adjustSize();

  // Construct layout for monthly recurrence rule
  QGridLayout *layoutMonthly = new QGridLayout(monthlyButtonGroup,1,1,0,10);
 
  layoutMonthly->addWidget(onNthDay,0,0);
  layoutMonthly->addWidget(nthDayEntry,0,1);
  layoutMonthly->addWidget(nthDayLabel,0,2);
 
  layoutMonthly->addWidget(onNthTypeOfDay,1,0);
  layoutMonthly->addWidget(nthNumberEntry,1,1);
  layoutMonthly->addWidget(nthTypeOfDayEntry,1,2);
  
  layoutMonthly->addMultiCellWidget(monthCommonLabel,0,1,3,3);
  layoutMonthly->addMultiCellWidget(nMonthsEntry,0,1,4,4);
  layoutMonthly->addMultiCellWidget(nMonthsLabel,0,1,5,5);
}

void KOEditorRecurrence::initYearly()
{
  yearlyFrame = new QVBox(ruleFrame);
  yearlyFrame->setFrameStyle(QFrame::NoFrame);

  yearlyButtonGroup = new QButtonGroup(yearlyFrame);
  yearlyButtonGroup->setFrameStyle(QFrame::NoFrame);
  
  yearMonthButton = new QRadioButton(i18n("Recur in the month of"),
                                     yearlyButtonGroup);
  yearMonthComboBox = new QComboBox(yearlyButtonGroup);

  yearDayButton = new QRadioButton(i18n("Recur on this day"),
                                   yearlyButtonGroup);

  yearCommonLabel = new QLabel(i18n("every"), yearlyButtonGroup);
  nYearsEntry = new QLineEdit(yearlyButtonGroup);
  nYearsEntry->setMaxLength(3);
  yearsLabel = new QLabel(i18n("year(s)"), yearlyButtonGroup);

  yearMonthComboBox->insertItem(i18n("January"));
  yearMonthComboBox->insertItem(i18n("February"));
  yearMonthComboBox->insertItem(i18n("March"));
  yearMonthComboBox->insertItem(i18n("April"));
  yearMonthComboBox->insertItem(i18n("May"));
  yearMonthComboBox->insertItem(i18n("June"));
  yearMonthComboBox->insertItem(i18n("July"));
  yearMonthComboBox->insertItem(i18n("August"));
  yearMonthComboBox->insertItem(i18n("September"));
  yearMonthComboBox->insertItem(i18n("October"));
  yearMonthComboBox->insertItem(i18n("November"));
  yearMonthComboBox->insertItem(i18n("December"));

  //yearDayLineEdit = new QLineEdit(yearlyButtonGroup);

  // Construct layout for yearly recurrence rule
  QGridLayout *layoutYearly = new QGridLayout(yearlyButtonGroup,1,1,0,10);
 
  layoutYearly->addWidget(yearMonthButton,0,0);
  layoutYearly->addWidget(yearMonthComboBox,0,1);

  layoutYearly->addWidget(yearDayButton,1,0);
 
  layoutYearly->addMultiCellWidget(yearCommonLabel,0,1,3,3);
  layoutYearly->addMultiCellWidget(nYearsEntry,0,1,4,4);
  layoutYearly->addMultiCellWidget(yearsLabel,0,1,5,5);
}

void KOEditorRecurrence::initExceptions()
{
  // Create the exceptions group box, which holds controls for
  // specifying dates which are exceptions to the rule specified on
  // this tab.
  exceptionGroupBox = new QGroupBox(1,QGroupBox::Horizontal,i18n("Exceptions"),
                                    this,"execeptionGroupBox");

  QFrame *exceptionFrame = new QFrame(exceptionGroupBox,"timeFrame");

  exceptionDateEdit = new KDateEdit(exceptionFrame);
  addExceptionButton = new QPushButton(i18n("Add"), exceptionFrame);
  changeExceptionButton = new QPushButton(i18n("Change"), exceptionFrame);
  deleteExceptionButton = new QPushButton(i18n("Delete"), exceptionFrame);
  exceptionList = new QListBox(exceptionFrame);

  QGridLayout *layoutExceptionFrame = new QGridLayout(exceptionFrame,1,1,0,5);
  layoutExceptionFrame->addWidget(exceptionDateEdit,0,0);
  layoutExceptionFrame->addWidget(addExceptionButton,1,0);
  layoutExceptionFrame->addWidget(changeExceptionButton,2,0);
  layoutExceptionFrame->addWidget(deleteExceptionButton,3,0);
  layoutExceptionFrame->addMultiCellWidget(exceptionList,0,3,1,1);

  connect(addExceptionButton, SIGNAL(clicked()),
	  this, SLOT(addException()));
  connect(changeExceptionButton, SIGNAL(clicked()),
	  this, SLOT(changeException()));
  connect(deleteExceptionButton, SIGNAL(clicked()),
	  this, SLOT(deleteException()));
}

void KOEditorRecurrence::setEnabled(bool enabled)
{
//  qDebug("KOEditorRecurrence::setEnabled(): %s",enabled ? "on" : "off");

  mEnabled = enabled;

/*
  nDaysEntry->setEnabled(enabled);
  nWeeksEntry->setEnabled(enabled);
  sundayBox->setEnabled(enabled);
  mondayBox->setEnabled(enabled);
  tuesdayBox->setEnabled(enabled);
  wednesdayBox->setEnabled(enabled);
  thursdayBox->setEnabled(enabled);
  fridayBox->setEnabled(enabled);
  saturdayBox->setEnabled(enabled);
  onNthDay->setEnabled(enabled);
  nthDayEntry->setEnabled(enabled);
  onNthTypeOfDay->setEnabled(enabled);
  nthNumberEntry->setEnabled(enabled);
  nthTypeOfDayEntry->setEnabled(enabled);
  nMonthsEntry->setEnabled(enabled);
  yearMonthButton->setEnabled(enabled);
  yearDayButton->setEnabled(enabled);
  //  yearDayLineEdit->setEnabled(enabled);
  nYearsEntry->setEnabled(enabled);
  //  advancedRuleButton->setEnabled(enabled);
  //  advancedRuleEdit->setEnabled(enabled);
  
  startDateEdit->setEnabled(enabled);
  noEndDateButton->setEnabled(enabled);
  endDurationButton->setEnabled(enabled);
  endDurationEdit->setEnabled(enabled);
  endDateButton->setEnabled(enabled);
  endDateEdit->setEnabled(enabled);
  exceptionDateEdit->setEnabled(enabled);
  addExceptionButton->setEnabled(enabled);
  changeExceptionButton->setEnabled(enabled);
  deleteExceptionButton->setEnabled(enabled);
  //  exceptionDateButton->setEnabled(enabled);
  exceptionList->setEnabled(enabled);
*/
}

void KOEditorRecurrence::addException()
{
  QDate tmpDate;

  tmpDate = exceptionDateEdit->getDate();
  exceptionList->insertItem(tmpDate.toString().data());
}

void KOEditorRecurrence::changeException()
{
  QDate tmpDate;
  tmpDate = exceptionDateEdit->getDate();

  exceptionList->changeItem(tmpDate.toString().data(), 
			    exceptionList->currentItem());
}

void KOEditorRecurrence::deleteException()
{
  exceptionList->removeItem(exceptionList->currentItem());
}

void KOEditorRecurrence::timeStuffDisable(bool disable)
{
  if (disable) {
    startTimeEdit->hide();
    startLabel->hide();
    endTimeEdit->hide();
    endLabel->hide();
    durationLabel->hide();
  } else {
    startTimeEdit->show();
    startLabel->show();
    endTimeEdit->show();
    endLabel->show();
    durationLabel->show();
  }
}

void KOEditorRecurrence::unsetAllCheckboxes()
{
  dailyButton->setChecked(false);
  weeklyButton->setChecked(false);
  monthlyButton->setChecked(false);
  yearlyButton->setChecked(false);

  onNthDay->setChecked(false);
  onNthTypeOfDay->setChecked(false);
  yearMonthButton->setChecked(false);
  yearDayButton->setChecked(false);

  mondayBox->setChecked(false);
  tuesdayBox->setChecked(false);
  wednesdayBox->setChecked(false);
  thursdayBox->setChecked(false);
  fridayBox->setChecked(false);
  saturdayBox->setChecked(false);
  sundayBox->setChecked(false);

  endDateButton->setChecked(false);
  noEndDateButton->setChecked(false);
  endDurationButton->setChecked(false);
}


void KOEditorRecurrence::checkDay(int day)
{
  switch (day) {
  case 1:
    mondayBox->setChecked(true);
    break;
  case 2:
    tuesdayBox->setChecked(true);
    break;
  case 3:
    wednesdayBox->setChecked(true);
    break;
  case 4:
    thursdayBox->setChecked(true);
    break;
  case 5:
    fridayBox->setChecked(true);
    break;
  case 6:
    saturdayBox->setChecked(true);
    break;
  case 7:
    sundayBox->setChecked(true);
    break;
  }
}

void KOEditorRecurrence::getCheckedDays(QBitArray &rDays)
{
  rDays.fill(false);
  if (mondayBox->isChecked())
    rDays.setBit(0, 1);
  if (tuesdayBox->isChecked())
    rDays.setBit(1, 1);
  if (wednesdayBox->isChecked())
    rDays.setBit(2, 1);
  if (thursdayBox->isChecked())
    rDays.setBit(3, 1);
  if (fridayBox->isChecked())
    rDays.setBit(4, 1);
  if (saturdayBox->isChecked())
    rDays.setBit(5, 1);
  if (sundayBox->isChecked())
  rDays.setBit(6, 1);    
}

void KOEditorRecurrence::setCheckedDays(QBitArray &rDays)
{
  if (rDays.testBit(0))
    mondayBox->setChecked(true);
  if (rDays.testBit(1))
    tuesdayBox->setChecked(true);
  if (rDays.testBit(2))
    wednesdayBox->setChecked(true);
  if (rDays.testBit(3))
    thursdayBox->setChecked(true);
  if (rDays.testBit(4))
    fridayBox->setChecked(true);
  if (rDays.testBit(5))
    saturdayBox->setChecked(true);
  if (rDays.testBit(6))
    sundayBox->setChecked(true);
}


void KOEditorRecurrence::setDateTimes(QDateTime start,QDateTime end)
{
//  qDebug ("KOEditorRecurrence::setDateTimes");

  startTimeEdit->setTime(start.time());
  endTimeEdit->setTime(end.time());

  startDateEdit->setDate(start.date());
  if (end.date() > endDateEdit->getDate()) endDateEdit->setDate(end.date());

  currStartDateTime = start;
  currEndDateTime = end;
  
  setDuration();
}

void KOEditorRecurrence::setAllDay(bool allDay)
{
  mAllDay = allDay;
  
  timeStuffDisable(allDay);  
}

void KOEditorRecurrence::setDuration()
{
  QString tmpStr, catStr;
  int hourdiff, minutediff;

  if (mAllDay) {
    tmpStr.sprintf((const char*)i18n("Duration: all day"));
  } else {
    hourdiff = currStartDateTime.date().daysTo(currEndDateTime.date()) * 24;
    hourdiff += currEndDateTime.time().hour() - 
      currStartDateTime.time().hour();
    minutediff = currEndDateTime.time().minute() -
      currStartDateTime.time().minute();
    // If minutediff is negative, "borrow" 60 minutes from hourdiff
    if (minutediff < 0 && hourdiff > 0) {
      hourdiff -= 1;
      minutediff += 60;
    }
    if (hourdiff || minutediff){
      tmpStr.sprintf((const char*)i18n("Duration: "));
      if (hourdiff){
        if (hourdiff > 1)
          catStr.sprintf((const char*)i18n("%i hours"), hourdiff);
        else if (hourdiff == 1)
            catStr.sprintf((const char*)i18n("%i hour"), hourdiff);
        tmpStr.append(catStr);
      }
      if (hourdiff && minutediff){
        catStr.sprintf((const char*)i18n(", "));
        tmpStr += catStr;
      }
      if (minutediff){
        if (minutediff > 1)
          catStr.sprintf((const char*)i18n("%i minutes"), minutediff);
        else if (minutediff == 1)
          catStr.sprintf((const char*)i18n("%i minute"), minutediff);
        tmpStr += catStr;
      }
    } else tmpStr = "";
  }
  durationLabel->setText(tmpStr);
  durationLabel->adjustSize();
}

void KOEditorRecurrence::startTimeChanged(QTime newtime, int wrapval)
{
  int secsep;

  secsep = currStartDateTime.secsTo(currEndDateTime);
  
  currStartDateTime = currStartDateTime.addDays(wrapval);
  currStartDateTime.setTime(newtime);

  currEndDateTime = currStartDateTime.addSecs(secsep);
  endTimeEdit->setTime(currEndDateTime.time());
  
  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorRecurrence::endTimeChanged(QTime newtime, int wrapval)
{
  QDateTime newdt(currEndDateTime.addDays(wrapval).date(), newtime);

  if(newdt < currStartDateTime) {
    // oops, can't let that happen.
    newdt = currStartDateTime;
    endTimeEdit->setTime(newdt.time());
  }
  currEndDateTime = newdt;

  setDuration();
  
  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorRecurrence::startDateChanged(QDate newdate)
{
  int daysep;
  daysep = currStartDateTime.daysTo(currEndDateTime);
  
  currStartDateTime.setDate(newdate);
  currEndDateTime.setDate(currStartDateTime.date().addDays(daysep));

  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorRecurrence::endDateChanged(QDate newdate)
{
  QDateTime newdt(newdate, currEndDateTime.time());

  if(newdt < currStartDateTime) {
    // oops, we can't let that happen.
    newdt = currStartDateTime;
    endTimeEdit->setTime(newdt.time());
  }
  
  endDateEdit->setDate(newdt.date());  

  setDuration();

  emit dateTimesChanged(currStartDateTime,currEndDateTime);
}

void KOEditorRecurrence::setDefaults(QDateTime from, QDateTime to,bool allDay)
{
  // unset everything
  unsetAllCheckboxes();

  setDateTimes(from,to);
  setAllDay(allDay);
  
  noEndDateButton->setChecked(true);
  weeklyButton->setChecked(true);

  nDaysEntry->setText("1");
  nWeeksEntry->setText("1");

  checkDay(from.date().dayOfWeek());
  onNthDay->setChecked(true);
  nthDayEntry->setCurrentItem(from.date().day()-1);
  nMonthsEntry->setText("1");
  yearDayButton->setChecked(true);
  nYearsEntry->setText(QString::number(from.date().dayOfYear()));
}

void KOEditorRecurrence::readEvent(KOEvent *event)
{
  QBitArray rDays;
  QList<KOEvent::rMonthPos> rmp;
  QList<int> rmd;
  int i;

  setDateTimes(event->getDtStart(),event->getDtEnd());

  // unset everything
  unsetAllCheckboxes();
  switch (event->doesRecur()) {
  case KOEvent::rNone:
    break;
  case KOEvent::rDaily:
    dailyButton->setChecked(true);
    nDaysEntry->setText(QString::number(event->getRecursFrequency()));
    break;
  case KOEvent::rWeekly:
    weeklyButton->setChecked(true);
    nWeeksEntry->setText(QString::number(event->getRecursFrequency()));
    
    rDays = event->getRecursDays();
    setCheckedDays(rDays);
    break;
  case KOEvent::rMonthlyPos:
    // we only handle one possibility in the list right now,
    // so I have hardcoded calls with first().  If we make the GUI
    // more extended, this can be changed.
    monthlyButton->setChecked(true);
    onNthTypeOfDay->setChecked(true);
    rmp = event->getRecursMonthPositions();
    if (rmp.first()->negative)
      i = 5 - rmp.first()->rPos - 1;
    else
      i = rmp.first()->rPos - 1;
    nthNumberEntry->setCurrentItem(i);
    i = 0;
    while (!rmp.first()->rDays.testBit(i))
      ++i;
    nthTypeOfDayEntry->setCurrentItem(i);
    nMonthsEntry->setText(QString::number(event->getRecursFrequency()));
    break;
  case KOEvent::rMonthlyDay:
    monthlyButton->setChecked(true);
    onNthDay->setChecked(true);
    rmd = event->getRecursMonthDays();
    i = *rmd.first() - 1;
    nthDayEntry->setCurrentItem(i);
    nMonthsEntry->setText(QString::number(event->getRecursFrequency()));
    break;
  case KOEvent::rYearlyMonth:
    yearlyButton->setChecked(true);
    yearMonthButton->setChecked(true);
    rmd = event->getRecursYearNums();
    yearMonthComboBox->setCurrentItem(*rmd.first() - 1);
    nYearsEntry->setText(QString::number(event->getRecursFrequency()));
    break;
  case KOEvent::rYearlyDay:
    yearlyButton->setChecked(true);
    yearDayButton->setChecked(true);
    nYearsEntry->setText(QString::number(event->getRecursFrequency()));
    break;
  default:
    break;
  }

  if (event->doesRecur()) {
    // get range information
    if (event->getRecursDuration() == -1)
      noEndDateButton->setChecked(true);
    else if (event->getRecursDuration() == 0) {
      endDateButton->setChecked(true);
      endDateEdit->setDate(event->getRecursEndDate());
    } else {
      endDurationButton->setChecked(true);
      endDurationEdit->setText(QString::number(event->getRecursDuration()));
    }
  } else {
    // the event doesn't recur, but we should provide some logical
    // defaults in case they go and make it recur.
    noEndDateButton->setChecked(true);
    weeklyButton->setChecked(true);
    nDaysEntry->setText("1");
    nWeeksEntry->setText("1");
    checkDay(currStartDateTime.date().dayOfWeek());
    onNthDay->setChecked(true);
    nthDayEntry->setCurrentItem(currStartDateTime.date().day()-1);
    nMonthsEntry->setText("1");
    yearDayButton->setChecked(true);
//    mpStr.sprintf("%i",currStartDateTime.date().dayOfYear());
    nYearsEntry->setText("1");
  }

  QDateList exDates(false);
  exDates = event->getExDates();
  QDate *curDate;
  exceptionDateEdit->setDate(QDate::currentDate());
  for (curDate = exDates.first(); curDate;
       curDate = exDates.next())
    exceptionList->insertItem(curDate->toString().data());
}

void KOEditorRecurrence::writeEvent(KOEvent *event)
{
  // temp. until something better happens.
  QString tmpStr;
  uint i;

  // get recurrence information
  // need a check to see if recurrence is enabled...
  if (mEnabled) {
    int rDuration;
    QDate rEndDate;
    
    // clear out any old settings;
    event->unsetRecurs();

    // first get range information.  It is common to all types
    // of recurring events.
    if (noEndDateButton->isChecked()) {
      rDuration = -1;
    } else if (endDurationButton->isChecked()) {
      tmpStr = endDurationEdit->text();
      rDuration = tmpStr.toInt();
    } else {
      rDuration = 0;
      rEndDate = endDateEdit->getDate();
    }
    
    // check for daily recurrence
    if (dailyButton->isChecked()) {
      int rFreq;
      
      tmpStr = nDaysEntry->text();
      rFreq = tmpStr.toInt();
      if (rDuration != 0)
	event->setRecursDaily(rFreq, rDuration);
      else
	event->setRecursDaily(rFreq, rEndDate);
      // check for weekly recurrence
    } else if (weeklyButton->isChecked()) {
      int rFreq;
      QBitArray rDays(7);
      
      tmpStr = nWeeksEntry->text();
      rFreq = tmpStr.toInt();

      getCheckedDays(rDays);
      
      if (rDuration != 0)
	event->setRecursWeekly(rFreq, rDays, rDuration);
      else
	event->setRecursWeekly(rFreq, rDays, rEndDate);
    } else if (monthlyButton->isChecked()) {
      if (onNthTypeOfDay->isChecked()) {
	// it's by position
	int rFreq, rPos;
	QBitArray rDays(7);
	
	tmpStr = nMonthsEntry->text();
	rFreq = tmpStr.toInt();
	rDays.fill(false);
	rPos = nthNumberEntry->currentItem() + 1;
	rDays.setBit(nthTypeOfDayEntry->currentItem());
	if (rDuration != 0)
	  event->setRecursMonthly(KOEvent::rMonthlyPos, rFreq, rDuration);
	else
	  event->setRecursMonthly(KOEvent::rMonthlyPos, rFreq, rEndDate);
	event->addRecursMonthlyPos(rPos, rDays);
      } else {
	// it's by day
	int rFreq;
	short rDay;
	
	tmpStr = nMonthsEntry->text();
	rFreq = tmpStr.toInt();
	
	rDay = nthDayEntry->currentItem() + 1;
	
	if (rDuration != 0)
	  event->setRecursMonthly(KOEvent::rMonthlyDay, rFreq, rDuration);
	else
	  event->setRecursMonthly(KOEvent::rMonthlyDay, rFreq, rEndDate);
	event->addRecursMonthlyDay(rDay);
      }
    } else if (yearlyButton->isChecked()) {
      if (yearMonthButton->isChecked()) {
	int rFreq, rMonth;

	tmpStr = nYearsEntry->text();
	rFreq = tmpStr.toInt();
	rMonth = yearMonthComboBox->currentItem() + 1;
	if (rDuration != 0)
	  event->setRecursYearly(KOEvent::rYearlyMonth, rFreq, rDuration);
	else
	  event->setRecursYearly(KOEvent::rYearlyMonth, rFreq, rEndDate);
	event->addRecursYearlyNum(rMonth);
      } else {
	// it's by day
	int rFreq;
	int rDay;

	tmpStr = nYearsEntry->text();
	rFreq = tmpStr.toInt();
	
	//tmpStr = Recurrence->yearDayLineEdit->text();
	rDay = event->getDtStart().date().dayOfYear();

	if (rDuration != 0)
	  event->setRecursYearly(KOEvent::rYearlyDay, rFreq, rDuration);
	else
	  event->setRecursYearly(KOEvent::rYearlyDay, rFreq, rEndDate);
	event->addRecursYearlyNum(rDay);
      }
    } // yearly
  } else
    event->unsetRecurs();

  QDateList exDates;
  for (i = 0; i < exceptionList->count(); i++)
    exDates.inSort(dateFromText(exceptionList->text(i)));

  event->setExDates(exDates);
  exDates.clear();
}

QDate *KOEditorRecurrence::dateFromText(QString text)
{
  QString tmpStr = text;
  tmpStr.remove(0,4);
  QString name = tmpStr.left(3);

  int y, m, d;

  name = name.upper();

  y = tmpStr.right(4).toInt();
  d = tmpStr.mid(4,2).toInt();
  if (name == "JAN")
    m = 1;
  else if (name == "FEB")
    m = 2;
  else if (name == "MAR")
    m = 3;
  else if (name == "APR")
    m = 4;
  else if (name == "MAY")
    m = 5;
  else if (name == "JUN")
    m = 6;
  else if (name == "JUL")
    m = 7;
  else if (name == "AUG")
    m = 8;
  else if (name == "SEP")
    m = 9;
  else if (name == "OCT")
    m = 10;
  else if (name == "NOV")
    m = 11;
  else if (name == "DEC")
    m = 12;
  else
    // should never get here!
    m = 0;  
  
  return new QDate(y,m,d);
}

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
  priorityLabel->setText( i18n("Priority") );

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
  layoutCompletion->addWidget(completedLabel);
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
  bool ok;
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
    tmpTime = startTimeEdit->getTime(ok);
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
