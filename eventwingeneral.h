// 	$Id$	

#ifndef _EVENTWINGENERAL_H
#define _EVENTWINGENERAL_H

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

#include "wingeneral.h"
#include "ktimed.h"
#include "kdated.h"

class EventWin;
//class TodoEventWin;
class EditEventWin;

class EventWinGeneral : public WinGeneral
{
    Q_OBJECT

    friend EventWin;
    friend EditEventWin;

public:

  EventWinGeneral(QWidget* parent = 0, const char* name = 0);
  virtual ~EventWinGeneral();

public slots:


protected slots:

  virtual void setEnabled(bool);
  void timeStuffDisable(bool disable);
  void alarmStuffEnable(bool enable);
  void alarmStuffDisable(bool disable);
  void pickAlarmSound();
  void pickAlarmProgram();

signals:
  void nullSignal(QWidget *);

protected:
  void initMisc();
  void initTimeBox();
  void initAlarmBox();

  void initLayout();

  /*
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
  QLabel                  *completedLabel;
  QCheckBox               *completedButton;
  QLabel                  *priorityLabel;
  QComboBox               *priorityCombo;
  QComboBox               *completedCombo;

  // variables to hold stuff temporarily.
  QString alarmSound;
  QString alarmProgram;
  */
};

#endif
