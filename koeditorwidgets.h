#ifndef _KOEDITORWIDGETS_H
#define _KOEDITORWIDGETS_H
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

#include "ktimed.h"
#include "kdated.h"

class Attendee;


class KOEditorGeneralEvent : public QWidget
{
    Q_OBJECT
  public:
    KOEditorGeneralEvent (int spacing=8,QWidget* parent=0,const char* name=0);
    virtual ~KOEditorGeneralEvent();

    /** Set widgets to default values */
    void setDefaults(QDateTime from,QDateTime to,bool allDay);
    /** Read event object and setup widgets accordingly */
    void readEvent(KOEvent *);
    /** Write event settings to event object */
    void writeEvent(KOEvent *);

    /** Set spacing for layouts */
    void setSpacing(int);

  public slots:
    void setDateTimes(QDateTime start, QDateTime end);
    void setCategories(QString);

  protected slots:

    virtual void setEnabled(bool);
    void timeStuffDisable(bool disable);
    void alarmStuffEnable(bool enable);
    void alarmStuffDisable(bool disable);
    void recurStuffEnable(bool enable);
    void pickAlarmSound();
    void pickAlarmProgram();

    void startTimeChanged(QTime,int);
    void startDateChanged(QDate);
    void endTimeChanged(QTime,int);
    void endDateChanged(QDate);

  signals:
    void dateTimesChanged(QDateTime start,QDateTime end);
    void allDayChanged(bool);
    void recursChanged(bool);
    void openCategoryDialog();

  protected:
    void initMisc();
    void initTimeBox();
    void initAlarmBox();

    void initLayout();

  private:
    QGroupBox               *timeGroupBox;
    QLabel                  *summaryLabel;
    QLineEdit               *summaryEdit;
    QLabel                  *startLabel;
    QLabel                  *endLabel;
    KDateEdit               *startDateEdit;
    KDateEdit               *endDateEdit;
    KTimeEdit               *startTimeEdit;
    KTimeEdit               *endTimeEdit;
    QCheckBox               *noTimeButton;
    QCheckBox               *recursButton;
    QLabel                  *alarmBell;
    QCheckBox               *alarmButton;
    KRestrictedLine         *alarmTimeEdit;
    QPushButton             *alarmSoundButton;
    QPushButton             *alarmProgramButton;
    QLabel                  *freeTimeLabel;
    QMultiLineEdit          *descriptionEdit;
    QComboBox               *freeTimeCombo;
    QLabel                  *ownerLabel;
    QCheckBox               *privateButton;
    QPushButton             *categoriesButton;
    QLabel                  *categoriesLabel;
    QComboBox               *alarmIncrCombo;

    QString alarmSound;
    QString alarmProgram;
    
    // current start and end date and time
    QDateTime currStartDateTime;
    QDateTime currEndDateTime;

    int mSpacing;
};


class AttendeeListItem : public QListViewItem
{
  public:
    AttendeeListItem(Attendee *a, QListView *parent);
    ~AttendeeListItem();

    Attendee *attendee() { return mAttendee; }
    void updateItem();
    
  private:
    Attendee *mAttendee;
};

class KOEditorDetails : public QWidget
{
    Q_OBJECT
  public:
    KOEditorDetails (int spacing = 8,QWidget* parent = 0, const char* name = 0);
    virtual ~KOEditorDetails();

    /** Set widgets to default values */
    void setDefaults();
    /** Read event object and setup widgets accordingly */
    void readEvent(KOEvent *);
    /** Write event settings to event object */
    void writeEvent(KOEvent *);

    /** Set spacing for layouts */
    void setSpacing(int);

  public slots:
    virtual void setEnabled(bool);
    void insertAttendee(Attendee *);
    void setCategories(QString);

  protected slots:
    void addNewAttendee(); 
    void updateAttendee();
    void removeAttendee();
    void attendeeListHilite(QListViewItem *);
    void attendeeListAction(QListViewItem *);
    void openAddressBook();

  signals:
    void openCategoryDialog();
    
  protected:
    void initAttendee();
    void initAttach();
    void initMisc();

    QVBoxLayout *topLayout;

    QGroupBox* attendeeGroupBox;
    QLabel* attendeeLabel;
    QLineEdit *attendeeEdit;
    QLineEdit *emailEdit;
    QListView *attendeeListBox;
    QPushButton* addAttendeeButton;
    QPushButton* removeAttendeeButton;
    QGroupBox* attachGroupBox;
    QPushButton* attachFileButton;
    QPushButton* removeFileButton;
    QPushButton* saveFileAsButton;
    QPushButton* addressBookButton;
    QPushButton* categoriesButton;
    QLabel* categoriesLabel;
    QLabel* attendeeRoleLabel;
    QComboBox* attendeeRoleCombo;
    QCheckBox* attendeeRSVPButton;
    QLabel* statusLabel;
    QComboBox* statusCombo;
    QLabel* locationLabel;
    QLabel* priorityLabel;
    QComboBox* priorityCombo;
    QPushButton* resourceButton;
    QLineEdit* resourcesEdit;
    QLabel* transparencyLabel;
    QLabel* transparencyAmountLabel;

    QList<AttendeeListItem> mAttendeeList; // list of attendee items

  private:
    int mSpacing;
};


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

  public slots:
    virtual void setEnabled(bool);
    void setDateTimes(QDateTime start,QDateTime end);
    void setAllDay(bool allDay);
  
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
    void timeStuffDisable(bool);

    void startTimeChanged(QTime,int);
    void startDateChanged(QDate);
    void endTimeChanged(QTime,int);
    void endDateChanged(QDate);
  
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

    void setDuration();

  private:
    QDate *dateFromText(QString text);
    
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

    bool mAllDay;

    // current start and end date and time
    QDateTime currStartDateTime;
    QDateTime currEndDateTime;

    bool mEnabled;

    int mSpacing;
};


class KOEditorGeneralTodo : public QWidget
{
    Q_OBJECT
  public:
    KOEditorGeneralTodo (int spacing=8,QWidget* parent=0,const char* name=0);
    virtual ~KOEditorGeneralTodo();

    /** Set widgets to default values */
    void setDefaults(QDateTime due,bool allDay);
    /** Read todo object and setup widgets accordingly */
    void readTodo(KOEvent *);
    /** Write todo settings to event object */
    void writeTodo(KOEvent *);

  public slots:
    void setCategories(QString);

  signals:
    void openCategoryDialog();

  protected slots:
    virtual void setEnabled(bool);
    void timeStuffDisable(bool disable);
    void dueStuffDisable(bool disable);

  protected:
    void initMisc();
    void initLayout();
    void initTimeBox();

  private:
    QGroupBox               *timeGroupBox;
    QLabel                  *summaryLabel;
    QLineEdit               *summaryEdit;
    QLabel                  *startLabel;
    QLabel                  *endLabel;
    KDateEdit               *startDateEdit;
    KDateEdit               *endDateEdit;
    KTimeEdit               *startTimeEdit;
    KTimeEdit               *endTimeEdit;
    QCheckBox               *noTimeButton;
    QCheckBox               *recursButton;
    QLabel                  *alarmBell;
    QCheckBox               *alarmButton;
    KRestrictedLine         *alarmTimeEdit;
    QPushButton             *alarmSoundButton;
    QPushButton             *alarmProgramButton;
    QLabel                  *freeTimeLabel;
    QMultiLineEdit          *descriptionEdit;
    QComboBox               *freeTimeCombo;
    QLabel                  *ownerLabel;
    QCheckBox               *privateButton;
    QPushButton             *categoriesButton;
    QLabel                  *categoriesLabel;
    QComboBox               *alarmIncrCombo;
  
    // variables for the todo stuff
    QCheckBox               *noDueButton;
    QLabel                  *completedLabel;
    QCheckBox               *completedButton;
    QLabel                  *priorityLabel;
    QComboBox               *priorityCombo;
    QComboBox               *completedCombo;
  
    // variables to hold stuff temporarily.
    QString alarmSound;
    QString alarmProgram;

    int mSpacing;
};


#endif
