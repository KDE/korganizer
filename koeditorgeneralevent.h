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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _KOEDITORGENERALEVENT_H
#define _KOEDITORGENERALEVENT_H

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

#include "koeditorgeneral.h"

#include "ktimeedit.h"

class KDateEdit;

using namespace KCal;

class KOEditorGeneralEvent : public KOEditorGeneral
{
    Q_OBJECT
  public:
    KOEditorGeneralEvent (QObject* parent=0,const char* name=0);
    virtual ~KOEditorGeneralEvent();

    void initTime(QWidget *,QBoxLayout *);
    void initClass(QWidget *,QBoxLayout *);

    void finishSetup();

    /** Set widgets to default values */
    void setDefaults(QDateTime from,QDateTime to,bool allDay);
    /**
      Read event object and setup widgets accordingly. If templ is true, the
      event is read as template, i.e. the time and date information isn't set.
    */
    void readEvent( Event *, bool tmpl = false );
    /** Write event settings to event object */
    void writeEvent(Event *);

    /** Check if the input is valid. */
    bool validateInput();

  public slots:
    void setDateTimes(QDateTime start, QDateTime end);
    void setDuration();

  protected slots:
    void timeStuffDisable(bool disable);
    void dontAssociateTime(bool noTime);

    void startTimeChanged(QTime);
    void startDateChanged(QDate);
    void endTimeChanged(QTime);
    void endDateChanged(QDate);

    void emitDateTimeStr();

  signals:
    void dateTimesChanged(QDateTime start,QDateTime end);
    void allDayChanged(bool);
    void dateTimeStrChanged(const QString &);

  private:
    QLabel                  *mStartDateLabel;
    QLabel                  *mEndDateLabel;
    KDateEdit               *mStartDateEdit;
    KDateEdit               *mEndDateEdit;
    KOTimeEdit              *mStartTimeEdit;
    KOTimeEdit              *mEndTimeEdit;
    QLabel                  *mDurationLabel;
    QCheckBox               *mNoTimeButton;
    QComboBox               *mFreeTimeCombo;

    // current start and end date and time
    QDateTime mCurrStartDateTime;
    QDateTime mCurrEndDateTime;
};

#endif
