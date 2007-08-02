/*
  This file is part of KOrganizer.

  Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KOGLOBALS_H
#define KOGLOBALS_H

#include "korganizer_export.h"
#include <kcomponentdata.h>
#include <QPixmap>

class QPixmap;
class QIcon;
class KCalendarSystem;
class AlarmClient;

class KConfig;
namespace LibKHolidays {
  class KHolidays;
}

class KORGANIZER_EXPORT KOGlobals
{
  public:
    static KOGlobals *self();

    enum { INCIDENCEADDED, INCIDENCEEDITED, INCIDENCEDELETED };
    enum { PRIORITY_MODIFIED, COMPLETION_MODIFIED, CATEGORY_MODIFIED,
           DATE_MODIFIED, RELATION_MODIFIED, ALARM_MODIFIED,
           DESCRIPTION_MODIFIED, SUMMARY_MODIFIED,
           COMPLETION_MODIFIED_WITH_RECURRENCE, UNKNOWN_MODIFIED };

    static void fitDialogToScreen( QWidget *widget, bool force=false );
    KConfig *config() const;

    static bool reverseLayout();

    const KCalendarSystem *calendarSystem() const;

    AlarmClient *alarmClient() const;

    ~KOGlobals();

    QPixmap smallIcon( const QString &name ) const;
    QIcon smallIconSet( const QString &name, int size = 0 ) const;

    QStringList holiday( const QDate &qd ) const;
    bool isWorkDay( const QDate &qd ) const;
    int getWorkWeekMask();

    /**
       Set which holidays the user wants to use.
       @param h a KHolidays object initialized with the desired locale.
       We capture this object, so you must not delete it.
    */
    void setHolidays( LibKHolidays::KHolidays *h );

    /** return the KHolidays object or 0 if none has been defined
    */
    LibKHolidays::KHolidays *holidays() const;

    const KComponentData &componentData() const { return mOwnInstance; }

  protected:
    KOGlobals();

  private:
    static KOGlobals *mSelf;

    KComponentData mOwnInstance;

    AlarmClient *mAlarmClient;

    LibKHolidays::KHolidays *mHolidays;
};

#endif
