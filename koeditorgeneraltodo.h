/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _KOEDITORGENERALTODO_H
#define _KOEDITORGENERALTODO_H

#include "koeditorgeneral.h"
#include <qdatetime.h>

class KRestrictedLine;

class KDateEdit;
class KTimeEdit;

namespace KCal {
class Todo;
}
using namespace KCal;

class KOEditorGeneralTodo : public KOEditorGeneral
{
    Q_OBJECT
  public:
    KOEditorGeneralTodo (QObject* parent=0,const char* name=0);
    virtual ~KOEditorGeneralTodo();

    void initTime(QWidget *, QBoxLayout *);
    void initStatus(QWidget *, QBoxLayout *);
    void initCompletion(QWidget *, QBoxLayout *);
    void initPriority(QWidget *, QBoxLayout *);

    void finishSetup();

    /** Set widgets to default values */
    void setDefaults( const QDateTime &due, bool allDay );
    /** Read todo object and setup widgets accordingly */
    void readTodo(Todo *);
    /** Write todo settings to event object */
    void writeTodo(Todo *);

    /** Check if the input is valid. */
    bool validateInput();

    /** The todo has been modified externally */
    void modified (Todo*, int);

  signals:
    void dueDateEditToggle( bool );
    void dateTimeStrChanged( const QString & );
    void signalDateTimeChanged( const QDateTime &, const QDateTime & );

  protected slots:
    void completedChanged(int);
    void dateChanged();
    void startDateModified();

    void enableDueEdit( bool enable );
    void enableStartEdit( bool enable );
    void enableTimeEdits( bool enable );
    void showAlarm();

  protected:
    void setCompletedDate();

 private:
    bool                    mAlreadyComplete;
    bool                    mStartDateModified;

    KDateEdit               *mStartDateEdit;
    KTimeEdit               *mStartTimeEdit;
    QCheckBox               *mTimeButton;
    QCheckBox               *mDueCheck;
    KDateEdit               *mDueDateEdit;
    KTimeEdit               *mDueTimeEdit;
    QComboBox               *mCompletedCombo;
    QLabel                  *mCompletedLabel;
    QLabel                  *mPriorityLabel;
    QComboBox               *mPriorityCombo;
    
    KDateEdit               *mCompletionDateEdit;
    KTimeEdit               *mCompletionTimeEdit;

    QCheckBox               *mStartCheck;

    QDateTime mCompleted;
};


#endif
