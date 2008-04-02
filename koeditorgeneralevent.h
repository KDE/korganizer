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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _KOEDITORGENERALEVENT_H
#define _KOEDITORGENERALEVENT_H

#include "koeditorgeneral.h"
#include <qdatetime.h>

class QLabel;
class KDateEdit;
class KTimeEdit;
class QCheckBox;
class QComboBox;
class QBoxLayout;

namespace KCal {
class Event;
}
using namespace KCal;

class KOEditorGeneralEvent : public KOEditorGeneral
{
    Q_OBJECT
  public:
    KOEditorGeneralEvent (QObject* parent=0,const char* name=0);
    virtual ~KOEditorGeneralEvent();

    void initTime(QWidget *,QBoxLayout *);
    void initClass(QWidget *,QBoxLayout *);
    void initInvitationBar( QWidget* parent, QBoxLayout *layout );

    void finishSetup();

    /** Set widgets to default values */
    void setDefaults( const QDateTime &from, const QDateTime &to, bool allDay );
    /**
      Read event object and setup widgets accordingly. If templ is true, the
      event is read as template, i.e. the time and date information isn't set.
    */
    void readEvent( Event *event, Calendar *calendar, bool tmpl = false );
    /** Write event settings to event object */
    void writeEvent( Event * );

    /** Check if the input is valid. */
    bool validateInput();

    void updateRecurrenceSummary( const QString &summary );

    QFrame* invitationBar() const { return mInvitationBar; }

  public slots:
    void setDateTimes( const QDateTime &start, const QDateTime &end );
    void setDuration();

  protected slots:
    void timeStuffDisable( bool disable );
    void associateTime( bool time );

    void startTimeChanged( QTime );
    void startDateChanged( const QDate& );
    void endTimeChanged( QTime );
    void endDateChanged( const QDate& );

    void emitDateTimeStr();

  signals:
    void allDayChanged(bool);
    void dateTimeStrChanged( const QString & );
    void dateTimesChanged( const QDateTime &start, const QDateTime &end );
    void editRecurrence();
    void acceptInvitation();
    void declineInvitation();

  private:
    QLabel                  *mStartDateLabel;
    QLabel                  *mEndDateLabel;
    KDateEdit               *mStartDateEdit;
    KDateEdit               *mEndDateEdit;
    KTimeEdit               *mStartTimeEdit;
    KTimeEdit               *mEndTimeEdit;
    QLabel                  *mDurationLabel;
    QCheckBox               *mAlldayEventCheckbox;
    QComboBox               *mFreeTimeCombo;
    QLabel                  *mRecurrenceSummary;
    QFrame                  *mInvitationBar;

    // current start and end date and time
    QDateTime mCurrStartDateTime;
    QDateTime mCurrEndDateTime;
};

#endif
