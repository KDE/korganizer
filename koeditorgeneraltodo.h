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
    void readTodo(KOEvent *);
    /** Write todo settings to event object */
    void writeTodo(KOEvent *);

    /** Check if the input is valid. */
    bool validateInput();

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
    QLabel                  *freeTimeLabel;
    QMultiLineEdit          *descriptionEdit;
    QComboBox               *freeTimeCombo;
    QLabel                  *ownerLabel;
    QCheckBox               *privateButton;
    QPushButton             *categoriesButton;
    QLabel                  *categoriesLabel;
  
    // variables for the todo stuff
    QCheckBox               *noDueButton;
    QCheckBox               *completedButton;
    QLabel                  *priorityLabel;
    QComboBox               *priorityCombo;
  
    // variables to hold stuff temporarily.
    QString alarmSound;
    QString alarmProgram;

    int mSpacing;
};


#endif
