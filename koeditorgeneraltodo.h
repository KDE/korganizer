#ifndef _KOEDITORGENERALTODO_H
#define _KOEDITORGENERALTODO_H
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

class KOEditorGeneralTodo : public QWidget
{
    Q_OBJECT
  public:
    KOEditorGeneralTodo (int spacing=8,QWidget* parent=0,const char* name=0);
    virtual ~KOEditorGeneralTodo();

    /** Set widgets to default values */
    void setDefaults(QDateTime due,bool allDay);
    /** Read todo object and setup widgets accordingly */
    void readTodo(Todo *);
    /** Write todo settings to event object */
    void writeTodo(Todo *);

    /** Check if the input is valid. */
    bool validateInput();

  public slots:
    void setCategories(QString);

  signals:
    void openCategoryDialog();

  protected slots:
    void timeStuffDisable(bool disable);
    void dueStuffDisable(bool disable);
    void startStuffDisable(bool disable);
    void completedClicked();

  protected:
    void initMisc();
    void initLayout();
    void initTimeBox();
    
    void setCompletedDate();

  private:
    QGroupBox               *timeGroupBox;
    QLabel                  *summaryLabel;
    QLineEdit               *summaryEdit;
    QLabel                  *mStartLabel;
    QLabel                  *endLabel;
    KDateEdit               *mStartDateEdit;
    KDateEdit               *endDateEdit;
    KTimeEdit               *mStartTimeEdit;
    KTimeEdit               *endTimeEdit;
    QCheckBox               *noTimeButton;
    QLabel                  *freeTimeLabel;
    QMultiLineEdit          *descriptionEdit;
    QComboBox               *freeTimeCombo;
    QLabel                  *ownerLabel;
    QLabel *mSecrecyLabel;
    QComboBox *mSecrecyCombo;

    QPushButton             *categoriesButton;
    QLabel                  *categoriesLabel;

    // variables for the todo stuff
    QCheckBox               *mNoDueCheck;
    QLabel                  *mDueLabel;
    KDateEdit               *mDueDateEdit;
    KTimeEdit               *mDueTimeEdit;
    
    QCheckBox               *completedButton;
    QLabel                  *priorityLabel;
    QComboBox               *priorityCombo;

    QCheckBox               *mNoStartCheck;
  

    // variables to hold stuff temporarily.
    QString alarmSound;
    QString alarmProgram;
    QDateTime mCompleted;

    int mSpacing;
};


#endif
