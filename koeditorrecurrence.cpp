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

// $Id$	

#include <qtooltip.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvgroupbox.h>
#include <qwidgetstack.h>
#include <qdatetime.h>
#include <qlistbox.h>

#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

#include <libkcal/event.h>

#include "koprefs.h"

#include "koeditorrecurrence.h"
#include "koeditorrecurrence.moc"


KOEditorRecurrence::KOEditorRecurrence(int spacing,QWidget* parent,
                                       const char* name) :
  QWidget( parent, name, 0 )
{
  mSpacing = spacing;

  mEnabled = false;

  initMain();
  initExceptions();

  initLayout();
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
  layoutTop->setColStretch(1,1);
}

void KOEditorRecurrence::initMain()
{
  // Create the appointement time group box, which is 
  timeGroupBox = new QGroupBox( 1,QGroupBox::Horizontal,
                                i18n("Appointment Time "),this, "User_2" );

  QFrame *timeFrame = new QFrame(timeGroupBox,"timeFrame");
  QBoxLayout *layoutTimeFrame = new QHBoxLayout(timeFrame);
  layoutTimeFrame->setSpacing(mSpacing);

  dateTimeLabel = new QLabel(timeFrame);
  layoutTimeFrame->addWidget(dateTimeLabel);
  
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

// Disable advanced rule input, because it is not used.
  advancedRuleButton->hide();
  advancedRuleEdit->hide();

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
//  startDateEdit = new KDateEdit(rangeButtonGroup);
  noEndDateButton = new QRadioButton(i18n("No Ending Date"), rangeButtonGroup);
  endDurationButton = new QRadioButton(i18n("End after"), rangeButtonGroup);
  endDurationEdit = new QLineEdit(rangeButtonGroup);
  endDurationLabel = new QLabel(i18n("occurrence(s)"), rangeButtonGroup);
  endDateButton = new QRadioButton(i18n("End by:"), rangeButtonGroup);
  endDateEdit = new KDateEdit(rangeButtonGroup);

  // Construct layout for recurrence range box
  QBoxLayout *layoutRange = new QVBoxLayout(rangeButtonGroup,5);
    
//  QBoxLayout *layoutStart = new QHBoxLayout;
//  layoutRange->addLayout(layoutStart);
//  layoutStart->addWidget(startDateLabel);
//  layoutStart->addWidget(startDateEdit,AlignVCenter);

  layoutRange->addWidget(startDateLabel);

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
  
  layoutRange->addStretch(1);
  
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
  nDaysEntry->setText( "1" );
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
  nWeeksEntry->setText("1");
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
  nMonthsEntry->setText("1");
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
  nYearsEntry->setText("1");
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
  layoutExceptionFrame->addMultiCellWidget(exceptionList,0,4,1,1);
  layoutExceptionFrame->setRowStretch(4,1);
  layoutExceptionFrame->setColStretch(1,3);

  connect(addExceptionButton, SIGNAL(clicked()),
	  this, SLOT(addException()));
  connect(changeExceptionButton, SIGNAL(clicked()),
	  this, SLOT(changeException()));
  connect(deleteExceptionButton, SIGNAL(clicked()),
	  this, SLOT(deleteException()));
}

void KOEditorRecurrence::setEnabled(bool enabled)
{
//  kdDebug() << "KOEditorRecurrence::setEnabled(): " << (enabled ? "on" : "off") << endl;

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
  QDate tmpDate = exceptionDateEdit->getDate();
  exceptionList->insertItem(KGlobal::locale()->formatDate(tmpDate));
  mExceptionDates.append(tmpDate);
}

void KOEditorRecurrence::changeException()
{
  int pos = exceptionList->currentItem();
  if (pos < 0) return;
  
  QDate tmpDate = exceptionDateEdit->getDate();
  mExceptionDates[pos] = tmpDate;
  exceptionList->changeItem(KGlobal::locale()->formatDate(tmpDate),pos);
}

void KOEditorRecurrence::deleteException()
{
  int pos = exceptionList->currentItem();
  if (pos < 0) return;

  mExceptionDates.remove( mExceptionDates.at(pos) );
  exceptionList->removeItem(pos);
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
//  kdDebug() << "KOEditorRecurrence::setDateTimes" << endl;

  currStartDateTime = start;
  currEndDateTime = end;
  
  startDateLabel->setText(i18n("Begins On: %1")
      .arg(KGlobal::locale()->formatDate(start.date()))); 
}


void KOEditorRecurrence::setDefaults(QDateTime from, QDateTime to,bool)
{
  // unset everything
  unsetAllCheckboxes();

  setDateTimes(from,to);

  startDateLabel->setText(i18n("Begins On: %1")
                          .arg(KGlobal::locale()->formatDate(from.date())));
  
  noEndDateButton->setChecked(true);
  weeklyButton->setChecked(true);

  nDaysEntry->setText("1");
  nWeeksEntry->setText("1");

  checkDay(from.date().dayOfWeek());
  onNthDay->setChecked(true);
  nthDayEntry->setCurrentItem(from.date().day()-1);
  nMonthsEntry->setText("1");
  yearDayButton->setChecked(true);
  nYearsEntry->setText("1");
}

void KOEditorRecurrence::readEvent(Event *event)
{
  QBitArray rDays;
  QPtrList<Recurrence::rMonthPos> rmp;
  QPtrList<int> rmd;
  int i;

  setDateTimes(event->dtStart(),event->dtEnd());

  // unset everything
  unsetAllCheckboxes();
  switch (event->recurrence()->doesRecur()) {
  case Recurrence::rNone:
    break;
  case Recurrence::rDaily:
    dailyButton->setChecked(true);
    nDaysEntry->setText(QString::number(event->recurrence()->frequency()));
    break;
  case Recurrence::rWeekly:
    weeklyButton->setChecked(true);
    nWeeksEntry->setText(QString::number(event->recurrence()->frequency()));
    
    rDays = event->recurrence()->days();
    setCheckedDays(rDays);
    break;
  case Recurrence::rMonthlyPos:
    // we only handle one possibility in the list right now,
    // so I have hardcoded calls with first().  If we make the GUI
    // more extended, this can be changed.
    monthlyButton->setChecked(true);
    onNthTypeOfDay->setChecked(true);
    rmp = event->recurrence()->monthPositions();
    if (rmp.first()->negative)
      i = 5 - rmp.first()->rPos - 1;
    else
      i = rmp.first()->rPos - 1;
    nthNumberEntry->setCurrentItem(i);
    i = 0;
    while (!rmp.first()->rDays.testBit(i))
      ++i;
    nthTypeOfDayEntry->setCurrentItem(i);
    nMonthsEntry->setText(QString::number(event->recurrence()->frequency()));
    break;
  case Recurrence::rMonthlyDay:
    monthlyButton->setChecked(true);
    onNthDay->setChecked(true);
    rmd = event->recurrence()->monthDays();
    i = *rmd.first() - 1;
    nthDayEntry->setCurrentItem(i);
    nMonthsEntry->setText(QString::number(event->recurrence()->frequency()));
    break;
  case Recurrence::rYearlyMonth:
    yearlyButton->setChecked(true);
    yearMonthButton->setChecked(true);
    rmd = event->recurrence()->yearNums();
    yearMonthComboBox->setCurrentItem(*rmd.first() - 1);
    nYearsEntry->setText(QString::number(event->recurrence()->frequency()));
    break;
  case Recurrence::rYearlyDay:
    yearlyButton->setChecked(true);
    yearDayButton->setChecked(true);
    nYearsEntry->setText(QString::number(event->recurrence()->frequency()));
    break;
  default:
    break;
  }

  startDateLabel->setText(i18n("Begins On: %1")
      .arg(KGlobal::locale()->formatDate(event->dtStart().date())));

  if (event->recurrence()->doesRecur()) {

    // get range information
    if (event->recurrence()->duration() == -1)
      noEndDateButton->setChecked(true);
    else if (event->recurrence()->duration() == 0) {
      endDateButton->setChecked(true);
      endDateEdit->setDate(event->recurrence()->endDate());
    } else {
      endDurationButton->setChecked(true);
      endDurationEdit->setText(QString::number(event->recurrence()->duration()));
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
    nYearsEntry->setText("1");
  }

  exceptionDateEdit->setDate(QDate::currentDate());

  DateList exDates = event->exDates();
  DateList::ConstIterator dit;
  for (dit = exDates.begin(); dit != exDates.end(); ++dit ) {
    exceptionList->insertItem(KGlobal::locale()->formatDate(*dit));
    mExceptionDates.append(*dit);
  }
}

void KOEditorRecurrence::writeEvent(Event *event)
{
  // get recurrence information
  // need a check to see if recurrence is enabled...
  if (mEnabled) {
    int rDuration;
    QDate rEndDate;
    QString tmpStr;
    
    // clear out any old settings;
    event->recurrence()->unsetRecurs();

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
      if (rFreq < 1) rFreq = 1;
      if (rDuration != 0)
	event->recurrence()->setDaily(rFreq, rDuration);
      else
	event->recurrence()->setDaily(rFreq, rEndDate);
      // check for weekly recurrence
    } else if (weeklyButton->isChecked()) {
      int rFreq;
      QBitArray rDays(7);
      
      tmpStr = nWeeksEntry->text();
      rFreq = tmpStr.toInt();
      if (rFreq < 1) rFreq = 1;

      getCheckedDays(rDays);
      
      if (rDuration != 0)
	event->recurrence()->setWeekly(rFreq, rDays, rDuration);
      else
	event->recurrence()->setWeekly(rFreq, rDays, rEndDate);
    } else if (monthlyButton->isChecked()) {
      if (onNthTypeOfDay->isChecked()) {
	// it's by position
	int rFreq, rPos;
	QBitArray rDays(7);
	
	tmpStr = nMonthsEntry->text();
	rFreq = tmpStr.toInt();
        if (rFreq < 1) rFreq = 1;
	rDays.fill(false);
	rPos = nthNumberEntry->currentItem() + 1;
	rDays.setBit(nthTypeOfDayEntry->currentItem());
	if (rDuration != 0)
	  event->recurrence()->setMonthly(Recurrence::rMonthlyPos, rFreq, rDuration);
	else
	  event->recurrence()->setMonthly(Recurrence::rMonthlyPos, rFreq, rEndDate);
	event->recurrence()->addMonthlyPos(rPos, rDays);
      } else {
	// it's by day
	int rFreq;
	short rDay;
	
	tmpStr = nMonthsEntry->text();
	rFreq = tmpStr.toInt();
        if (rFreq < 1) rFreq = 1;

	rDay = nthDayEntry->currentItem() + 1;

	if (rDuration != 0)
	  event->recurrence()->setMonthly(Recurrence::rMonthlyDay, rFreq, rDuration);
	else
	  event->recurrence()->setMonthly(Recurrence::rMonthlyDay, rFreq, rEndDate);
	event->recurrence()->addMonthlyDay(rDay);
      }
    } else if (yearlyButton->isChecked()) {
      if (yearMonthButton->isChecked()) {
	int rFreq, rMonth;

	tmpStr = nYearsEntry->text();
	rFreq = tmpStr.toInt();
        if (rFreq < 1) rFreq = 1;
	rMonth = yearMonthComboBox->currentItem() + 1;
	if (rDuration != 0)
	  event->recurrence()->setYearly(Recurrence::rYearlyMonth, rFreq, rDuration);
	else
	  event->recurrence()->setYearly(Recurrence::rYearlyMonth, rFreq, rEndDate);
	event->recurrence()->addYearlyNum(rMonth);
      } else {
	// it's by day
	int rFreq;
	int rDay;

	tmpStr = nYearsEntry->text();
	rFreq = tmpStr.toInt();
        if (rFreq < 1) rFreq = 1;
	
	//tmpStr = Recurrence->yearDayLineEdit->text();
	rDay = event->dtStart().date().dayOfYear();

	if (rDuration != 0)
	  event->recurrence()->setYearly(Recurrence::rYearlyDay, rFreq, rDuration);
	else
	  event->recurrence()->setYearly(Recurrence::rYearlyDay, rFreq, rEndDate);
	event->recurrence()->addYearlyNum(rDay);
      }
    } // yearly
  } else
    event->recurrence()->unsetRecurs();

  event->setExDates(mExceptionDates);
}

// obsolete
#if 0
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
#endif

void KOEditorRecurrence::setDateTimeStr(const QString &str)
{
  dateTimeLabel->setText(str);
}

bool KOEditorRecurrence::validateInput()
{
  // Check input here

  return true;
}
