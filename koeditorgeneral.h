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
#ifndef _KOEDITORGENERA_H
#define _KOEDITORGENERA_H
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

#include <libkcal/incidence.h>

#include "ktimeedit.h"
#include "kdateedit.h"

using namespace KCal;

class KOEditorGeneral : public QObject
{
    Q_OBJECT
  public:
    KOEditorGeneral (QObject* parent=0,const char* name=0);
    virtual ~KOEditorGeneral();

    void initHeader(QWidget *,QBoxLayout *);
    void initDescription(QWidget *,QBoxLayout *);
    void initSecrecy(QWidget *,QBoxLayout *);
    void initCategories(QWidget *,QBoxLayout *);    
    void initAlarm(QWidget *,QBoxLayout *);

    /** Set widgets to default values */
    void setDefaults(bool allDay);
    /** Read event object and setup widgets accordingly */
    void readIncidence(Incidence *);
    /** Write event settings to event object */
    void writeIncidence(Incidence *);

    /** Check if the input is valid. */
    bool validateInput() { return true; }

  public slots:
    void setCategories(const QString &);

  protected slots:
    void enableAlarmEdit( bool enable );
    void disableAlarmEdit( bool disable );
    void alarmDisable( bool disable );
    void pickAlarmSound();
    void pickAlarmProgram();

  signals:
    void openCategoryDialog();

  protected:
    QLineEdit               *mSummaryEdit;
    QLabel                  *mAlarmBell;
    QCheckBox               *mAlarmButton;
    KRestrictedLine         *mAlarmTimeEdit;
    QPushButton             *mAlarmSoundButton;
    QPushButton             *mAlarmProgramButton;
    QComboBox               *mAlarmIncrCombo;
    QMultiLineEdit          *mDescriptionEdit;
    QLabel                  *mOwnerLabel;
    QComboBox               *mSecrecyCombo;
    QPushButton             *mCategoriesButton;
    QLabel                  *mCategoriesLabel;
     
  private:
    QString mAlarmSound;
    QString mAlarmProgram;
};

#endif
