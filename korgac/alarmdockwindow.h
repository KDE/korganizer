/*
  This file is part of the KDE reminder agent.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2008-2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
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
    ~AlarmDockWindow() override;

    void enableAutostart(bool enabled);

public Q_SLOTS:
    void toggleAlarmsEnabled(bool checked);
    void toggleAutostart(bool checked);
    void toggleGrabFocus(bool checked);
    void slotUpdate(int reminders);

Q_SIGNALS:
    void quitSignal();
    void suspendAllSignal();
    void dismissAllSignal();
    void showReminderSignal();

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
    // True/Enable if the notify daemon should grab focus (activate window) away
    // from the current application.  This makes it easy to dismiss, but if the
    // user is typing AlarmDialog now gets those keys and space or return will
    // dismiss all notifications before the user has a chance to read them.
    QAction *mGrabFocus = nullptr;
    QAction *mShow = nullptr;

    bool mAutostartSet = false;
};

#endif
