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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KORG_GLOBALS_H
#define KORG_GLOBALS_H

class KCalendarSystem;
class AlarmClient;

class KConfig;

class KOGlobals
{
  public:
    static KOGlobals *self();
  
    enum { EVENTADDED, EVENTEDITED, EVENTDELETED };  
    enum { PRIORITY_MODIFIED, COMPLETION_MODIFIED, CATEGORY_MODIFIED, UNKNOWN_MODIFIED };

    static void fitDialogToScreen( QWidget *widget, bool force=false );
    static KConfig *config();

    static bool reverseLayout();

    KCalendarSystem *calendarSystem();

    AlarmClient *alarmClient();

  protected:
    KOGlobals();
    ~KOGlobals();
    
  private:
    static KOGlobals *mSelf;
    
    KCalendarSystem *mCalendarSystem;    
    AlarmClient *mAlarmClient;
};

#endif
