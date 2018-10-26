/*
  This file is part of the KDE reminder agent.

  Copyright (c) 2002,2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KORGAC_KOALARMCLIENT_H
#define KORGAC_KOALARMCLIENT_H

#include <Akonadi/Calendar/ETMCalendar>

#include <QTimer>
#include <QDateTime>
#include <QSessionManager>
class AlarmDialog;
class AlarmDockWindow;

namespace Akonadi {
class Item;
class EntityTreeModel;
}

class KOAlarmClient : public QObject
{
    Q_OBJECT
public:
    explicit KOAlarmClient(QObject *parent = nullptr);
    ~KOAlarmClient();

    // DBUS interface
    void quit();
    void hide();
    void show();
    void forceAlarmCheck();
    QString dumpDebug() const;
    QStringList dumpAlarms() const;

public Q_SLOTS:
    void slotQuit();

Q_SIGNALS:
    void reminderCount(int);
    void saveAllSignal();

private:
    void deferredInit();
    void checkAlarms();
    void setupAkonadi();
    void slotCommitData(QSessionManager &);
    void showReminder();
    bool dockerEnabled();
    bool collectionsAvailable() const;
    void createReminder(const Akonadi::Item &incidence,
                        const QDateTime &dt, const QString &displayText);
    void saveLastCheckTime();
    void createDialog();

    AlarmDockWindow *mDocker = nullptr;  // the panel icon
    Akonadi::ETMCalendar::Ptr mCalendar;
    Akonadi::EntityTreeModel *mETM = nullptr;

    QDateTime mLastChecked;
    QTimer mCheckTimer;

    AlarmDialog *mDialog = nullptr;
};

#endif
