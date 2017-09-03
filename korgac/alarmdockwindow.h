/*
  This file is part of the KDE reminder agent.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2008-2009 Allen Winter <winter@kde.org>

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
#ifndef KORGAC_ALARMDOCKWINDOW_H
#define KORGAC_ALARMDOCKWINDOW_H

#include <KStatusNotifierItem>

#include <QAction>
#include <QIcon>

class AlarmDockWindow : public KStatusNotifierItem
{
    Q_OBJECT
public:
    AlarmDockWindow();
    virtual ~AlarmDockWindow();

    void enableAutostart(bool enabled);

public Q_SLOTS:
    void toggleAlarmsEnabled(bool checked);
    void toggleAutostart(bool checked);
    void slotUpdate(int reminders);

Q_SIGNALS:
    void quitSignal();
    void suspendAllSignal();
    void dismissAllSignal();

protected Q_SLOTS:
    void activate(const QPoint &pos) override;
    void slotQuit();
    void slotSuspendAll();
    void slotDismissAll();

private:
    void changeSystrayIcon(bool alarmsEnabled);

    QIcon mIconDisabled;
    QString mName;

    QAction *mAlarmsEnabled = nullptr;
    QAction *mAutostart = nullptr;
    QAction *mSuspendAll = nullptr;
    QAction *mDismissAll = nullptr;

    bool mAutostartSet = false;
};

#endif
