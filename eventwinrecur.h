// 	$Id$	

#ifndef _EVENTWINRECUR_H
#define _EVENTWINRECUR_H

#include <qframe.h>
#include <qlabel.h>
#include <qbttngrp.h>
#include <qchkbox.h>
#include <qpushbt.h>
#include <qgrpbox.h>
#include <qlined.h>
#include <qradiobt.h>
#include <qcombo.h>
#include <qframe.h>
#include <qlistbox.h>
#include <qbitarray.h>
#include <kapp.h>
#include <qwidgetstack.h>

#include "ktimed.h"
#include "kdated.h"

class EventWin;
class EditEventWin;
class TodoEventWin;

class EventWinRecurrence : public QFrame
{
    Q_OBJECT

    friend EventWin;
    friend EditEventWin;
    friend TodoEventWin;
    

public:

    EventWinRecurrence(QWidget* parent = 0, 
		       const char* name = 0, bool todo=FALSE);
    virtual ~EventWinRecurrence();

public slots:
  virtual void setEnabled(bool);
  void setModified();

signals:
  void nullSignal(QWidget *);
  void modifiedEvent();

protected slots:
  void showDaily(bool);
  void showWeekly(bool);
  void showMonthly(bool);
  void showYearly(bool);
  void disableRange(bool);
  void enableDurationRange(bool);
  void enableDateRange(bool);
  void addException();
  void changeException();
  void deleteException();
  void timeStuffDisable(bool);

protected:
  bool isTodo;
  void unsetAllCheckboxes();
  void CheckDay(int day);
  void getCheckedDays(QBitArray &rDays);
  void setCheckedDays(QBitArray &rDays);

  void initMain();
  void initDaily();
  void initWeekly();
  void initMonthly();
  void initYearly();
  void initExceptions();

  void initLayout();

  /* stuff to hold the appointment time setting widgets. */
  QGroupBox* timeGroupBox;
  QLabel* startLabel;
  QLabel* endLabel;
  KTimeEdit* startTimeEdit;
  KTimeEdit* endTimeEdit;
  QLabel *durationLabel;

  /* main rule box and choices. */
  QGroupBox*    ruleGroupBox;
  QFrame*       ruleFrame;
  QWidgetStack* ruleStack;
  QButtonGroup* ruleButtonGroup;
  QRadioButton* dailyButton;
  QRadioButton* weeklyButton;
  QRadioButton* monthlyButton;
  QRadioButton* yearlyButton;
  
  QFrame* ruleSepFrame;
  
  /* daily rule choices */
  QFrame*       dailyFrame;
  QLabel*       everyNDays;
  QLineEdit*    nDaysEntry;
  QLabel*       nDaysLabel;

  /* weekly rule choices */
  QFrame*       weeklyFrame;
  QLabel*       everyNWeeks;
  QLineEdit*    nWeeksEntry;
  QLabel*       nWeeksLabel;
  QCheckBox*    sundayBox;
  QCheckBox*    mondayBox;
  QCheckBox*    tuesdayBox;
  QCheckBox*    wednesdayBox;
  QCheckBox*    thursdayBox;
  QCheckBox*    fridayBox;
  QCheckBox*    saturdayBox;

  /* monthly rule choices */
  QFrame*       monthlyFrame;
  QButtonGroup* monthlyButtonGroup;
  QRadioButton* onNthDay;
  QComboBox*    nthDayEntry;
  QLabel*       nthDayLabel;
  QRadioButton* onNthTypeOfDay;
  QComboBox*    nthNumberEntry;
  QComboBox*    nthTypeOfDayEntry;
  QLabel*       monthCommonLabel;
  QLineEdit*    nMonthsEntry;
  QLabel*       nMonthsLabel;
  
  /* yearly rule choices */
  QFrame*       yearlyFrame;
  QLabel       *yearCommonLabel;
  QButtonGroup *yearlyButtonGroup;
  QRadioButton *yearMonthButton;
  QRadioButton *yearDayButton;
  QComboBox    *yearMonthComboBox;
  QLineEdit    *yearDayLineEdit;
  QLineEdit    *nYearsEntry;
  QLabel       *yearsLabel;

  /* advanced rule choices */
  QCheckBox* advancedRuleButton;
  QLineEdit* advancedRuleEdit;

  /* range stuff */
  QGroupBox*    rangeGroupBox;
  QButtonGroup* rangeButtonGroup;
  QLabel*       startDateLabel;
  KDateEdit*    startDateEdit;
  QRadioButton* noEndDateButton;
  QRadioButton* endDurationButton;
  QLineEdit*    endDurationEdit;
  QLabel*       endDurationLabel;
  QRadioButton* endDateButton;
  KDateEdit*    endDateEdit;

  /* exception stuff */
  QGroupBox* exceptionGroupBox;
  KDateEdit *exceptionDateEdit;
  QPushButton* addExceptionButton;
  QPushButton* changeExceptionButton;
  QPushButton* deleteExceptionButton;
  QPushButton* exceptionDateButton;
  QListBox *exceptionList;

};

#endif 
