// 	$Id$	

#ifndef _WINGENERAL_H
#define _WINGENERAL_H

#include <qframe.h>
#include <qlabel.h>
#include <qchkbox.h>
#include <qpushbt.h>
#include <qgrpbox.h>
#include <qlined.h>
#include <qcombo.h>
#include <qmlined.h>
#include <kapp.h>
#include <krestrictedline.h>

#include "ktimed.h"
#include "kdated.h"


class EventWin;
class TodoEventWin;
class EditEventWin;

class WinGeneral : public QFrame
{
    Q_OBJECT

    friend EventWin;
    friend TodoEventWin;
    friend EditEventWin;

public:

  WinGeneral(QWidget* parent = 0, const char* name = 0);
  virtual ~WinGeneral();

public slots:


protected slots:

  virtual void setEnabled(bool);

  virtual void timeStuffDisable(bool disable);
  virtual void alarmStuffEnable(bool enable);
  virtual void alarmStuffDisable(bool disable);
  virtual void pickAlarmSound();
  virtual void pickAlarmProgram();
  void setModified();

signals:
  void nullSignal(QWidget *);
  void modifiedEvent();

protected:
  void initMisc();
  void initTimeBox();
  void initAlarmBox();
//  void initTodoSpecific();

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
};

#endif
