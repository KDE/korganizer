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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KORG_GLOBALS_H
#define KORG_GLOBALS_H

class QPixmap;
class QIconSet;
class KCalendarSystem;
class AlarmClient;

class KConfig;
class KInstance;

class KOGlobals
{
  public:
    static KOGlobals *self();

    enum { INCIDENCEADDED, INCIDENCEEDITED, INCIDENCEDELETED };  
    enum { PRIORITY_MODIFIED, COMPLETION_MODIFIED, CATEGORY_MODIFIED, 
           DATE_MODIFIED, RELATION_MODIFIED, ALARM_MODIFIED,
           DESCRIPTION_MODIFIED, SUMMARY_MODIFIED,
           UNKNOWN_MODIFIED };

    static void fitDialogToScreen( QWidget *widget, bool force=false );
    KConfig *config() const;

    static bool reverseLayout();

    const KCalendarSystem *calendarSystem() const;

    AlarmClient *alarmClient() const;

    ~KOGlobals();

    QPixmap smallIcon(const QString& name);
    QIconSet smallIconSet(const QString& name, int size = 0);
    
  protected:
    KOGlobals();

  private:
    static KOGlobals *mSelf;

    KInstance *mOwnInstance;

    AlarmClient *mAlarmClient;
};

#endif
