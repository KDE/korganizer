/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
*/
#ifndef _KOEDITORGENERALEVENT_H
#define _KOEDITORGENERALEVENT_H
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

#include <krestrictedline.h>

#include "ktimeedit.h"
#include "kdateedit.h"

using namespace KCal;

class KOEditorGeneralEvent : public QWidget
{
    Q_OBJECT
  public:
    KOEditorGeneralEvent (int spacing=8,QWidget* parent=0,const char* name=0);
    virtual ~KOEditorGeneralEvent();

    /** Set widgets to default values */
    void setDefaults(QDateTime from,QDateTime to,bool allDay);
    /** Read event object and setup widgets accordingly */
    void readEvent(Event *);
    /** Write event settings to event object */
    void writeEvent(Event *);

    /** Check if the input is valid. */
    bool validateInput();

    /** Set spacing for layouts */
    void setSpacing(int);

  public slots:
    void setDateTimes(QDateTime start, QDateTime end);
    void setCategories(const QString &);
    void setDuration();

  protected slots:

    void timeStuffDisable(bool disable);
    void alarmStuffEnable(bool enable);
    void alarmStuffDisable(bool disable);
    void dontAssociateTime(bool noTime);
    void pickAlarmSound();
    void pickAlarmProgram();

    void startTimeChanged(QTime);
    void startDateChanged(QDate);
    void endTimeChanged(QTime);
    void endDateChanged(QDate);

    void emitDateTimeStr();

  signals:
    void dateTimesChanged(QDateTime start,QDateTime end);
    void allDayChanged(bool);
    void recursChanged(bool);
    void openCategoryDialog();
    void dateTimeStrChanged(const QString &);

  protected:
    void initMisc();
    void initTimeBox();
    void initAlarmBox();

    void initLayout();

  private:
    QGroupBox               *timeGroupBox;
    QLabel                  *summaryLabel;
    QLineEdit               *summaryEdit;
    QLabel                  *startDateLabel;
    QLabel                  *endDateLabel;
    QLabel                  *startTimeLabel;
    QLabel                  *endTimeLabel;
    KDateEdit               *startDateEdit;
    KDateEdit               *endDateEdit;
    KTimeEdit               *startTimeEdit;
    KTimeEdit               *endTimeEdit;
    QLabel                  *durationLabel;
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
    QLabel *mSecrecyLabel;
    QComboBox *mSecrecyCombo;
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

#endif
