// 	$Id$	

#include <qlabel.h>
#include <qbttngrp.h>
#include <qlistbox.h>
#include <qlined.h>
#include <qframe.h>
#include <qlayout.h>
#include <qwidgetstack.h>
#include <qvbox.h>

#include <klocale.h>

#include "eventwinrecur.moc"

EventWinRecurrence::EventWinRecurrence(QWidget* parent, const char* name, bool todo)
  : QFrame( parent, name, 0 )
{
  isTodo = todo;
//  resize( 600,400 );

  initMain();
  initExceptions();

  initLayout();
}

void EventWinRecurrence::initLayout()
{
  QGridLayout *layoutTop = new QGridLayout(this,1,1,5);
  
  layoutTop->addMultiCellWidget(timeGroupBox,0,0,0,1);
  layoutTop->addMultiCellWidget(ruleGroupBox,1,1,0,1);
  layoutTop->addWidget(rangeGroupBox,2,0);
  layoutTop->addWidget(exceptionGroupBox,2,1);
}

void EventWinRecurrence::initMain()
{
  // Create the appointement time group box, which is 
  timeGroupBox = new QGroupBox( 1,QGroupBox::Horizontal,
                                i18n("Appointment Time "),this, "User_2" );

  QFrame *timeFrame = new QFrame(timeGroupBox,"timeFrame");
  QBoxLayout *layoutTimeFrame = new QHBoxLayout(timeFrame);
  
  startLabel    = new QLabel(i18n("Start:  "), timeFrame);
  endLabel      = new QLabel(i18n("End:  "), timeFrame);

  // preston to ian -- not sure that currentTime is the best choice here,
  // so I changed it for now...
  startTimeEdit = new KTimeEdit(timeFrame);
  endTimeEdit   = new KTimeEdit(timeFrame);
  durationLabel = new QLabel(timeFrame);

  layoutTimeFrame->addWidget(startLabel);
  layoutTimeFrame->addWidget(startTimeEdit);
  layoutTimeFrame->addSpacing(10);
  layoutTimeFrame->addWidget(endLabel);
  layoutTimeFrame->addWidget(endTimeEdit);
  layoutTimeFrame->addWidget(durationLabel);
  layoutTimeFrame->addStretch(1);

/*
//  timeFrame->move(15,5);
  startLabel->move(15,30);
  startLabel->adjustSize();
  startTimeEdit->move(startLabel->geometry().topRight().x(),
		      startLabel->geometry().topRight().y()-5);
  endLabel->move(startTimeEdit->geometry().topRight().x()+15,
		 startLabel->geometry().topRight().y());
  endLabel->adjustSize();
  endTimeEdit->move(endLabel->geometry().topRight().x(),
		    endLabel->geometry().topRight().y()-5);
  durationLabel->move(endTimeEdit->geometry().topRight().x()+15,
		      startLabel->geometry().topRight().y());
*/
//  timeGroupBox->adjustSize();
//  timeGroupBox->setFixedWidth(width()-30);
//  timeGroupBox->setFixedHeight(timeGroupBox->height()-15);



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

//  ruleGroupBox->move(timeGroupBox->geometry().topLeft().x(),
//		     timeGroupBox->geometry().bottomLeft().y());

  ruleButtonGroup = new QButtonGroup(1,Horizontal,ruleFrame);
  ruleButtonGroup->setFrameStyle(NoFrame);
  dailyButton     = new QRadioButton(i18n("Daily"), ruleButtonGroup);
  weeklyButton    = new QRadioButton(i18n("Weekly"), ruleButtonGroup);
  monthlyButton   = new QRadioButton(i18n("Monthly"), ruleButtonGroup);
  yearlyButton    = new QRadioButton(i18n("Yearly"), ruleButtonGroup);

/*
  ruleButtonGroup->move(15,25);
  dailyButton->move(0,0);
  dailyButton->adjustSize();
  weeklyButton->move(dailyButton->geometry().bottomLeft().x(),
		     dailyButton->geometry().bottomLeft().y()+5);
  weeklyButton->adjustSize();
  monthlyButton->move(weeklyButton->geometry().bottomLeft().x(),
		      weeklyButton->geometry().bottomLeft().y()+5);
  monthlyButton->adjustSize();
  yearlyButton->move(monthlyButton->geometry().bottomLeft().x(),
		     monthlyButton->geometry().bottomLeft().y()+5);
  yearlyButton->adjustSize();

  ruleButtonGroup->adjustSize();
*/

  ruleSepFrame = new QFrame(ruleFrame);
  ruleSepFrame->setFrameStyle(VLine|Sunken);
//  ruleSepFrame->setFixedWidth(5);
//  ruleSepFrame->setFixedHeight(ruleButtonGroup->height());
//  ruleSepFrame->move(ruleButtonGroup->geometry().topRight().x()+15,
//		     ruleButtonGroup->geometry().topRight().y());

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
  advancedRuleButton->setEnabled(FALSE);

  advancedRuleEdit = new QLineEdit(ruleFrame);
  advancedRuleEdit->setText( "" );
  advancedRuleEdit->setMaxLength( 32767 );
  advancedRuleEdit->setEnabled(FALSE);

  QBoxLayout *layoutAdvancedRule = new QHBoxLayout;
  layoutRuleFrame->addLayout(layoutAdvancedRule);
  layoutAdvancedRule->addWidget(advancedRuleButton);
  layoutAdvancedRule->addWidget(advancedRuleEdit);

/*
  advancedRuleButton->move(ruleButtonGroup->geometry().bottomLeft().x(),
			   ruleButtonGroup->geometry().bottomLeft().y()+20);
  advancedRuleButton->adjustSize();
  advancedRuleEdit->move(advancedRuleButton->geometry().topRight().x()+5,
			 advancedRuleButton->geometry().topRight().y()-5);
  advancedRuleEdit->adjustSize();
*/

  connect(dailyButton, SIGNAL(toggled(bool)), 
	  this, SLOT(showDaily(bool)));
  connect(weeklyButton, SIGNAL(toggled(bool)), 
	  this, SLOT(showWeekly(bool)));
  connect(monthlyButton, SIGNAL(toggled(bool)), 
	  this, SLOT(showMonthly(bool)));
  connect(yearlyButton, SIGNAL(toggled(bool)),
	  this, SLOT(showYearly(bool)));

  

//  ruleGroupBox->adjustSize();
//  ruleGroupBox->setFixedWidth(width()-30);
//  ruleGroupBox->setFixedHeight(ruleGroupBox->height()-10);
//  advancedRuleEdit->setFixedWidth(ruleGroupBox->width() -
//				  advancedRuleEdit->geometry().topLeft().x()-10);




  // Create the recursion range Group Box which contains the controls
  // specific to determining how long the recursion is to last.
  rangeGroupBox = new QGroupBox( 1,QGroupBox::Horizontal,
                                 i18n("Recurrence Range"),this,
				 "rangeGroupBox" );

  rangeButtonGroup = new QButtonGroup( rangeGroupBox,"rangeButtonGroup");
  rangeButtonGroup->setFrameStyle(NoFrame);
//  rangeButtonGroup->setExclusive(true);

//  QFrame *rangeFrame = new QFrame(rangeButtonGroup);
//  rangeButtonGroup = new QButtonGroup(this);
//  rangeButtonGroup->setTitle(i18n("Recurrence Range"));
//  rangeButtonGroup->move(ruleGroupBox->geometry().topLeft().x(),
//		      ruleGroupBox->geometry().bottomLeft().y());

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
  
/*
  startDateLabel->move(15,25);
  startDateLabel->adjustSize();
  
  startDateEdit->move(startDateLabel->geometry().topRight().x()+10,
		      startDateLabel->geometry().topLeft().y()-3);
  startDateEdit->adjustSize();

  noEndDateButton->move(startDateLabel->geometry().topLeft().x(),
			startDateLabel->geometry().bottomLeft().y()+10);
  noEndDateButton->adjustSize();
  endDurationButton->move(noEndDateButton->geometry().topLeft().x(),
			  noEndDateButton->geometry().bottomLeft().y()+10);
  endDurationButton->adjustSize();
  endDurationEdit->move(endDurationButton->geometry().topRight().x()+10,
			endDurationButton->geometry().topRight().y()-3);
  endDurationEdit->setFixedWidth(30);
  endDurationEdit->adjustSize();
  endDurationLabel->move(endDurationEdit->geometry().topRight().x()+10,
			 endDurationEdit->geometry().topRight().y()-3); 
  endDateButton->move(endDurationButton->geometry().topLeft().x(),
		      endDurationButton->geometry().bottomLeft().y()+10);
  endDateButton->adjustSize();
  endDateEdit->move(endDateButton->geometry().topRight().x()+10,
		    endDateButton->geometry().topRight().y()-3);
  endDateEdit->adjustSize();
*/
  connect(noEndDateButton, SIGNAL(toggled(bool)),
	  this, SLOT(disableRange(bool)));
  connect(endDurationButton, SIGNAL(toggled(bool)),
	  this, SLOT(enableDurationRange(bool)));
  connect(endDateButton, SIGNAL(toggled(bool)),
	  this, SLOT(enableDateRange(bool)));

//  rangeButtonGroup->adjustSize();
//  rangeButtonGroup->setFixedWidth((width()/2)-10);
//  rangeButtonGroup->setFixedHeight(rangeButtonGroup->height()-10);

}

void EventWinRecurrence::showDaily(bool on)
{
  if(on) {
/*    weeklyFrame->hide();
    monthlyFrame->hide();
    yearlyFrame->hide();
    dailyFrame->show();
    
    ruleFrame->update();
*/
    ruleStack->raiseWidget(dailyFrame);
    return;
  }
}

void EventWinRecurrence::showWeekly(bool on)
{
  if(on) {
/*
    dailyFrame->hide();
    monthlyFrame->hide();
    yearlyFrame->hide();
    weeklyFrame->show();

    ruleFrame->update();
*/
    ruleStack->raiseWidget(weeklyFrame);

    return;
  }
}

void EventWinRecurrence::showMonthly(bool on)
{
  if(on) {
/*
    dailyFrame->hide();
    weeklyFrame->hide();
    monthlyFrame->show();
    yearlyFrame->hide();
*/
    ruleStack->raiseWidget(monthlyFrame);

    return;
  }
}

void EventWinRecurrence::showYearly(bool on)
{
  if(on) {
/*  
    dailyFrame->hide();
    weeklyFrame->hide();
    monthlyFrame->hide();
    yearlyFrame->show();
*/
    ruleStack->raiseWidget(yearlyFrame);

    return;
  }
}

void EventWinRecurrence::disableRange(bool on)
{
  if (on) {
    endDateEdit->setEnabled(FALSE);
    endDurationEdit->setEnabled(FALSE);
    endDurationLabel->setEnabled(FALSE);
    return;
  }
}

void EventWinRecurrence::enableDurationRange(bool on)
{
  if (on) {
    endDurationEdit->setEnabled(TRUE);
    endDurationLabel->setEnabled(TRUE);
    endDateEdit->setEnabled(FALSE);
    return;
  }
}

void EventWinRecurrence::enableDateRange(bool on)
{
  if (on) {
    endDateEdit->setEnabled(TRUE);
    endDurationEdit->setEnabled(FALSE);
    endDurationLabel->setEnabled(FALSE);
    return;
  }
}

void EventWinRecurrence::initDaily()
{
  dailyFrame = new QFrame(ruleFrame);
  dailyFrame->setFrameStyle(NoFrame);

  everyNDays = new QLabel(i18n("Recur every"), dailyFrame);

  nDaysEntry = new QLineEdit(dailyFrame);
  nDaysEntry->setText( "" );
  nDaysEntry->setMaxLength( 3 );

  nDaysLabel = new QLabel(i18n("day(s)"), dailyFrame);

  QBoxLayout *layoutDaily = new QHBoxLayout(dailyFrame,10);
  layoutDaily->addWidget(everyNDays);
  layoutDaily->addWidget(nDaysEntry);
  layoutDaily->addWidget(nDaysLabel);
  
/*
  dailyFrame->move(ruleSepFrame->geometry().topRight().x()+10,
		   ruleSepFrame->geometry().topRight().y());
  
  everyNDays->move(10,15);
  everyNDays->adjustSize();
  nDaysEntry->move(everyNDays->geometry().topRight().x()+5,
		   everyNDays->geometry().topRight().y()-5);
  nDaysEntry->adjustSize();
  nDaysEntry->setFixedWidth(30);
  nDaysLabel->move(nDaysEntry->geometry().topRight().x()+5,
		   everyNDays->geometry().topRight().y()+2);
  nDaysLabel->adjustSize();
  
  dailyFrame->adjustSize();
*/

//  dailyFrame->hide();
}

void EventWinRecurrence::initWeekly()
{
  weeklyFrame = new QFrame(ruleFrame);
  weeklyFrame->setFrameStyle(NoFrame);

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

/*
  weeklyFrame->move(ruleSepFrame->geometry().topRight().x()+10,
		    ruleSepFrame->geometry().topRight().y());
  
  everyNWeeks->move(10,15);
  everyNWeeks->adjustSize();
  nWeeksEntry->move(everyNWeeks->geometry().topRight().x()+5,
		    everyNWeeks->geometry().topRight().y()-7);
  nWeeksEntry->adjustSize();
  nWeeksEntry->setFixedWidth(25);
  nWeeksLabel->move(nWeeksEntry->geometry().topRight().x()+7,
		    everyNWeeks->geometry().topRight().y());
  nWeeksLabel->adjustSize();

  sundayBox->move(everyNWeeks->geometry().bottomLeft().x(),
		  everyNWeeks->geometry().bottomLeft().y()+25);
  sundayBox->adjustSize();
  mondayBox->move(sundayBox->geometry().topRight().x()+5,
		  sundayBox->geometry().topRight().y());
  mondayBox->adjustSize();
  tuesdayBox->move(mondayBox->geometry().topRight().x()+5,
		   mondayBox->geometry().topRight().y());
  tuesdayBox->adjustSize();
  wednesdayBox->move(tuesdayBox->geometry().topRight().x()+5,
		     tuesdayBox->geometry().topRight().y());
  wednesdayBox->adjustSize();
  thursdayBox->move(wednesdayBox->geometry().topRight().x()+5,
		    wednesdayBox->geometry().topRight().y());
  thursdayBox->adjustSize();
  fridayBox->move(thursdayBox->geometry().topRight().x()+5,
		  thursdayBox->geometry().topRight().y());
  fridayBox->adjustSize();
  saturdayBox->move(fridayBox->geometry().topRight().x()+5,
		    fridayBox->geometry().topRight().y());
  saturdayBox->adjustSize();
  
  weeklyFrame->adjustSize();
  weeklyFrame->hide();
*/
}

void EventWinRecurrence::initMonthly()
{
  monthlyFrame = new QVBox(ruleFrame);
  monthlyFrame->setFrameStyle(NoFrame);
//  monthlyFrame->hide();

  monthlyButtonGroup = new QButtonGroup(monthlyFrame);
  monthlyButtonGroup->setFrameStyle(NoFrame);

  onNthDay          = new QRadioButton(i18n("Recur on the"), monthlyButtonGroup);
  nthDayEntry       = new QComboBox(FALSE, monthlyButtonGroup);
  nthDayLabel       = new QLabel(i18n("day"), monthlyButtonGroup);

  onNthTypeOfDay    = new QRadioButton(i18n("Recur on the"), monthlyButtonGroup);
  nthNumberEntry    = new QComboBox(FALSE, monthlyButtonGroup);
  nthTypeOfDayEntry = new QComboBox(FALSE, monthlyButtonGroup);

  monthCommonLabel  = new QLabel(i18n("every"), monthlyButtonGroup);
  nMonthsEntry      = new QLineEdit(monthlyButtonGroup);
  nMonthsLabel      = new QLabel(i18n("month(s)"), monthlyButtonGroup);

/*
  monthlyFrame->move(ruleSepFrame->geometry().topRight().x()+10,
		     ruleSepFrame->geometry().topRight().y());
  
  monthlyButtonGroup->move(10,5);

  onNthDay->move(0,10);
  onNthDay->adjustSize();
  nthDayEntry->move(onNthDay->geometry().topRight().x()+5,
		    onNthDay->geometry().topRight().y()-7);
*/
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

/*
  nthDayEntry->adjustSize();
  nthDayLabel->move(nthDayEntry->geometry().topRight().x()+35,
		    onNthDay->geometry().topRight().y());
  nthDayLabel->adjustSize();

  onNthTypeOfDay->move(onNthDay->geometry().bottomLeft().x(),
		       onNthDay->geometry().bottomLeft().y()+15);
  onNthTypeOfDay->adjustSize();
  nthNumberEntry->move(onNthTypeOfDay->geometry().topRight().x()+5,
		       onNthTypeOfDay->geometry().topRight().y()-7);
*/
  nthNumberEntry->insertItem( i18n("1st") ); 
  nthNumberEntry->insertItem( i18n("2nd") );
  nthNumberEntry->insertItem( i18n("3rd") ); 
  nthNumberEntry->insertItem( i18n("4th") );
  nthNumberEntry->insertItem( i18n("5th") ); 

/*
  nthNumberEntry->setFixedSize(nthDayEntry->size());
  nthTypeOfDayEntry->move(nthNumberEntry->geometry().topRight().x()+5,
			  nthNumberEntry->geometry().topRight().y());
*/
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

/*  
  monthCommonLabel->move(nthTypeOfDayEntry->geometry().topRight().x()+10,
			 nthTypeOfDayEntry->geometry().topRight().y()-15);
  monthCommonLabel->adjustSize();
  nMonthsEntry->move(monthCommonLabel->geometry().topRight().x()+10,
		     monthCommonLabel->geometry().topRight().y()-5);
  nMonthsEntry->setMaxLength(3);
  nMonthsEntry->setFixedWidth(30);
  nMonthsEntry->adjustSize();
  nMonthsLabel->move(nMonthsEntry->geometry().topRight().x()+5,
		     monthCommonLabel->geometry().topRight().y());
  nMonthsLabel->adjustSize();

  monthlyButtonGroup->resize(QSize(monthlyButtonGroup->childrenRect().width(),
				   monthlyButtonGroup->childrenRect().height()+10));
  monthlyFrame->adjustSize();
  monthlyFrame->hide();
*/
}

void EventWinRecurrence::initYearly()
{
  yearlyFrame = new QVBox(ruleFrame);
  yearlyFrame->setFrameStyle(NoFrame);

  yearlyButtonGroup = new QButtonGroup(yearlyFrame);
  yearlyButtonGroup->setFrameStyle(NoFrame);
  
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

/*
  yearlyFrame->move(ruleSepFrame->geometry().topRight().x()+10,
		    ruleSepFrame->geometry().topRight().y());
    
  yearlyButtonGroup->move(10,5);
  yearMonthButton->move(0,10);
  yearMonthButton->adjustSize();

  yearDayButton->move(yearMonthButton->geometry().topLeft().x(),
  		      yearMonthButton->geometry().bottomLeft().y()+15);
  yearDayButton->adjustSize();

  yearMonthComboBox->move(yearMonthButton->geometry().topRight().x()+10,
			  yearMonthButton->geometry().topRight().y()-7);
  yearMonthComboBox->adjustSize();

  //yearDayLineEdit->move(yearDayButton->geometry().topRight().x()+10,
  //			yearDayButton->geometry().topRight().y());
  //yearDayLineEdit->adjustSize();

  yearCommonLabel->move(monthCommonLabel->geometry().topLeft().x(),
			monthCommonLabel->geometry().topLeft().y());
  yearCommonLabel->adjustSize();

  nYearsEntry->move(yearCommonLabel->geometry().topRight().x()+10,
		    yearCommonLabel->geometry().topRight().y()-5);
  nYearsEntry->setFixedWidth(30);
  nYearsEntry->adjustSize();

  yearsLabel->move(nYearsEntry->geometry().topRight().x()+5,
		   yearCommonLabel->geometry().topRight().y());
  yearsLabel->adjustSize();
  
  yearlyButtonGroup->resize(QSize(yearlyButtonGroup->childrenRect().width(),
  				  yearlyButtonGroup->childrenRect().height()+10));

  yearlyFrame->adjustSize();
  yearlyFrame->hide();
*/
}

void EventWinRecurrence::initExceptions()
{
  // Create the exceptions group box, which holds controls for
  // specifying dates which are exceptions to the rule specified on
  // this tab.
  exceptionGroupBox = new QGroupBox(1,QGroupBox::Horizontal,i18n("Exceptions"),
                                    this,"execeptionGroupBox");

//  exceptionGroupBox->move(rangeButtonGroup->geometry().topRight().x()+5,
//			  rangeButtonGroup->geometry().topRight().y());

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

/*
  exceptionDateEdit->move(15,22);
  exceptionDateEdit->adjustSize();

  addExceptionButton->adjustSize();
  addExceptionButton->setFixedHeight(20);
  addExceptionButton->setFixedWidth(80);
  addExceptionButton->move(exceptionDateEdit->geometry().topLeft().x(),
			   exceptionDateEdit->geometry().bottomLeft().y()+10);

  changeExceptionButton->adjustSize();
  changeExceptionButton->setFixedHeight(20);
  changeExceptionButton->setFixedWidth(80);
  changeExceptionButton->move(addExceptionButton->geometry().topLeft().x(),
			      addExceptionButton->geometry().bottomLeft().y()+5);

  deleteExceptionButton->adjustSize();
  deleteExceptionButton->setFixedHeight(20);
  deleteExceptionButton->setFixedWidth(80);
  deleteExceptionButton->move(changeExceptionButton->geometry().topLeft().x(),
			      changeExceptionButton->geometry().bottomLeft().y()+5);

  exceptionList->move(exceptionDateEdit->geometry().topRight().x()+10,
		      exceptionDateEdit->geometry().topRight().y());

  exceptionList->adjustSize();
  exceptionList->setFixedWidth(110);
  exceptionList->setFixedHeight(100);

  exceptionGroupBox->adjustSize();
  exceptionGroupBox->setFixedWidth((width()/2)-25);
  exceptionGroupBox->setFixedHeight(exceptionGroupBox->height()-10);
*/

  connect(addExceptionButton, SIGNAL(clicked()),
	  this, SLOT(addException()));
  connect(changeExceptionButton, SIGNAL(clicked()),
	  this, SLOT(changeException()));
  connect(deleteExceptionButton, SIGNAL(clicked()),
	  this, SLOT(deleteException()));

}

EventWinRecurrence::~EventWinRecurrence()
{
  
}

void EventWinRecurrence::setEnabled(bool enabled)
{
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
}

void EventWinRecurrence::addException()
{
  QDate tmpDate;

  tmpDate = exceptionDateEdit->getDate();
  exceptionList->insertItem(tmpDate.toString().data());
}

void EventWinRecurrence::changeException()
{
  QDate tmpDate;
  tmpDate = exceptionDateEdit->getDate();

  exceptionList->changeItem(tmpDate.toString().data(), 
			    exceptionList->currentItem());
}

void EventWinRecurrence::deleteException()
{
  exceptionList->removeItem(exceptionList->currentItem());
}

void EventWinRecurrence::timeStuffDisable(bool disable)
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

void EventWinRecurrence::unsetAllCheckboxes() {
    dailyButton->setChecked(FALSE);
    weeklyButton->setChecked(FALSE);
    monthlyButton->setChecked(FALSE);
    yearlyButton->setChecked(FALSE);
    onNthDay->setChecked(FALSE);
    onNthTypeOfDay->setChecked(FALSE);
    yearMonthButton->setChecked(FALSE);
    yearDayButton->setChecked(FALSE);

    mondayBox->setChecked(FALSE);
    tuesdayBox->setChecked(FALSE);
    wednesdayBox->setChecked(FALSE);
    thursdayBox->setChecked(FALSE);
    fridayBox->setChecked(FALSE);
    saturdayBox->setChecked(FALSE);
    sundayBox->setChecked(FALSE);
    endDateButton->setChecked(FALSE);
    noEndDateButton->setChecked(FALSE);
    endDurationButton->setChecked(FALSE);
}


void EventWinRecurrence::CheckDay(int day) {
    switch (day) {
    case 1:
      mondayBox->setChecked(TRUE);
      break;
    case 2:
      tuesdayBox->setChecked(TRUE);
      break;
    case 3:
      wednesdayBox->setChecked(TRUE);
      break;
    case 4:
      thursdayBox->setChecked(TRUE);
      break;
    case 5:
      fridayBox->setChecked(TRUE);
      break;
    case 6:
      saturdayBox->setChecked(TRUE);
      break;
    case 7:
      sundayBox->setChecked(TRUE);
      break;
    }
}

void EventWinRecurrence::getCheckedDays(QBitArray &rDays) {
      rDays.fill(FALSE);
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

void EventWinRecurrence::setCheckedDays(QBitArray &rDays) {
    if (rDays.testBit(0))
      mondayBox->setChecked(TRUE);
    if (rDays.testBit(1))
      tuesdayBox->setChecked(TRUE);
    if (rDays.testBit(2))
      wednesdayBox->setChecked(TRUE);
    if (rDays.testBit(3))
      thursdayBox->setChecked(TRUE);
    if (rDays.testBit(4))
      fridayBox->setChecked(TRUE);
    if (rDays.testBit(5))
      saturdayBox->setChecked(TRUE);
    if (rDays.testBit(6))
      sundayBox->setChecked(TRUE);
}

void EventWinRecurrence::setModified()
{
  emit modifiedEvent();
}
