#ifndef _KOEDITORRECURRENCE_H
#define _KOEDITORRECURRENCE_H
// $Id$

#include <qframe.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qmultilineedit.h>
#include <qlistview.h>
#include <qradiobutton.h>

#include <kapp.h>
#include <krestrictedline.h>

#include "ktimeedit.h"
#include "kdateedit.h"

class KOEditorRecurrence : public QWidget
{
    Q_OBJECT
  public:
    KOEditorRecurrence (int spacing=8,QWidget* parent=0,const char* name=0);
    virtual ~KOEditorRecurrence();

    /** Set widgets to default values */
    void setDefaults(QDateTime from,QDateTime to,bool allday);
    /** Read event object and setup widgets accordingly */
    void readEvent(KOEvent *);
    /** Write event settings to event object */
    void writeEvent(KOEvent *);

    /** Check if the input is valid. */
    bool validateInput();

  public slots:
    virtual void setEnabled(bool);
    void setDateTimes(QDateTime start,QDateTime end);
    void setDateTimeStr(const QString &);
  
  signals:
    void dateTimesChanged(QDateTime start,QDateTime end);
  
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
  
  protected:
    void unsetAllCheckboxes();
    void checkDay(int day);
    void getCheckedDays(QBitArray &rDays);
    void setCheckedDays(QBitArray &rDays);
  
    void initMain();
    void initDaily();
    void initWeekly();
    void initMonthly();
    void initYearly();
    void initExceptions();
  
    void initLayout();

  private:
    QDate *dateFromText(QString text);
    
    /* stuff to hold the appointment time setting widgets. */
    QGroupBox* timeGroupBox;
    QLabel *dateTimeLabel;
  
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

    // current start and end date and time
    QDateTime currStartDateTime;
    QDateTime currEndDateTime;

    bool mEnabled;

    int mSpacing;
};

#endif
