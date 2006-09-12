/*
    This file is part of KOrganizer.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KOEDITORALARMS_H
#define KOEDITORALARMS_H

#include <kdialogbase.h>
#include <libkcal/alarm.h>

class KOEditorAlarms_base;

class AlarmListViewItem;

class KOEditorAlarms : public KDialogBase
{
    Q_OBJECT
  public:
    KOEditorAlarms( KCal::Alarm::List *alarms, QWidget *parent = 0,
                    const char *name = 0 );
    ~KOEditorAlarms();

  protected slots:
    void slotOk();
    void slotAdd();
    void slotDuplicate();
    void slotRemove();
    void changed();
    void selectionChanged( QListViewItem *listviewitem );
  protected:
    void init();
    void readAlarm( KCal::Alarm *alarm );
    void writeAlarm( KCal::Alarm *alarm );
  private:
    KCal::Alarm::List *mAlarms;
    KOEditorAlarms_base *mWidget;
    bool mInitializing;
    AlarmListViewItem *mCurrentItem;
};

#endif
