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

#include "ktimeedit.h"
#include "kdateedit.h"
#include "koeditorgeneral.h"

class KRestrictedLine;

using namespace KCal;

class KOEditorGeneralTodo : public KOEditorGeneral
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

  signals:
    void openCategoryDialog();

  protected slots:
    void timeStuffDisable(bool disable);
    void dueStuffDisable(bool disable);
    void startStuffDisable(bool disable);
    void completedChanged(int);

  protected:
    void initTime(QBoxLayout *);
    void initStatus(QBoxLayout *);
  
    void setCompletedDate();

 private:
    QLabel                  *mStartLabel;
    KDateEdit               *mStartDateEdit;
    KTimeEdit               *mStartTimeEdit;
    QCheckBox               *mNoTimeButton;
    QCheckBox               *mNoDueCheck;
    QLabel                  *mDueLabel;
    KDateEdit               *mDueDateEdit;
    KTimeEdit               *mDueTimeEdit;
    QComboBox               *mCompletedCombo;
    QLabel                  *mCompletedLabel;
    QLabel                  *mPriorityLabel;
    QComboBox               *mPriorityCombo;

    QCheckBox               *mNoStartCheck;
  
    QDateTime mCompleted;

    int mSpacing;
};


#endif
