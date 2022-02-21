/*
  This file is part of the KDE reminder agent.

  SPDX-FileCopyrightText: 2002, 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#pragma once

#include <Akonadi/ETMCalendar>

#include <QDateTime>
#include <QSessionManager>
#include <QTimer>
class AlarmDialog;
class AlarmDockWindow;

namespace Akonadi
{
class Item;
class EntityTreeModel;
}

class KOAlarmClient : public QObject
{
    Q_OBJECT
public:
    explicit KOAlarmClient(QObject *parent = nullptr);
    ~KOAlarmClient() override;

    // DBUS interface
    void quit();
    void hide();
    void show();
    void forceAlarmCheck();
    Q_REQUIRED_RESULT QString dumpDebug() const;
    Q_REQUIRED_RESULT QStringList dumpAlarms() const;

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
    Q_REQUIRED_RESULT bool dockerEnabled();
    Q_REQUIRED_RESULT bool collectionsAvailable() const;
    void createReminder(const Akonadi::Item &incidence, const QDateTime &dt, const QString &displayText);
    void saveLastCheckTime();
    void createDialog();

    AlarmDockWindow *mDocker = nullptr; // the panel icon
    Akonadi::ETMCalendar::Ptr mCalendar;
    Akonadi::EntityTreeModel *mETM = nullptr;

    QDateTime mLastChecked;
    QTimer mCheckTimer;

    AlarmDialog *mDialog = nullptr;
};

