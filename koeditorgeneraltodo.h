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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KOEDITORGENERALTODO_H
#define KOEDITORGENERALTODO_H

#include "koeditorgeneral.h"

#include <QDateTime>

class KComboBox;
class QBoxLayout;
class QLabel;

namespace KPIM {
  class KDateEdit;
  class KTimeEdit;
  class KTimeZoneComboBox;
}

namespace KCal {
  class Calendar;
  class Todo;
}
using namespace KCal;

class KOEditorGeneralTodo : public KOEditorGeneral
{
  Q_OBJECT
  public:
    explicit KOEditorGeneralTodo( Calendar *calendar, QObject *parent = 0 );
    virtual ~KOEditorGeneralTodo();

    void initTime( QWidget *, QBoxLayout * );
    void initStatus( QWidget *, QBoxLayout * );
    void initCompletion( QWidget *, QBoxLayout * );
    void initPriority( QWidget *, QBoxLayout * );

    void finishSetup();

    /** Set widgets to default values */
    void setDefaults( const QDateTime &due, bool allDay );

    /** Read todo object and setup widgets accordingly */
    void readTodo( Todo *todo, bool tmpl = false );

    /** Write todo settings to event object */
    void fillTodo( Todo * );

    /** Check if the input is valid. */
    bool validateInput();

    /** The todo has been modified externally */
    void modified ( Todo *, int );

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
    virtual bool setAlarmOffset( Alarm *alarm, int value ) const;

  private:
    KPIM::KTimeZoneComboBox *mTimeZoneComboStart;
    KPIM::KTimeZoneComboBox *mTimeZoneComboDue;
    KDateTime::Spec         mStartSpec;
    KDateTime::Spec         mDueSpec;

    bool                    mAlreadyComplete;
    bool                    mStartDateModified;

    KPIM::KDateEdit         *mStartDateEdit;
    KPIM::KTimeEdit         *mStartTimeEdit;
    QCheckBox               *mTimeButton;
    QCheckBox               *mDueCheck;
    KPIM::KDateEdit         *mDueDateEdit;
    KPIM::KTimeEdit         *mDueTimeEdit;
    KComboBox               *mCompletedCombo;
    QLabel                  *mCompletedLabel;
    QLabel                  *mPriorityLabel;
    KComboBox               *mPriorityCombo;

    KPIM::KDateEdit         *mCompletionDateEdit;
    KPIM::KTimeEdit         *mCompletionTimeEdit;

    QCheckBox               *mStartCheck;

    QDateTime               mCompleted;
};

#endif
