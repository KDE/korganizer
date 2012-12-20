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

#if !defined(Q_WS_WINCE)
# include <KSessionManager>
#endif

#include <QTimer>
#include <QDateTime>

class AlarmDialog;
class AlarmDockWindow;

namespace Akonadi {
  class Item;
  class EntityTreeModel;
}

#if !defined(Q_WS_WINCE)
class KOAlarmClient : public QObject, public KSessionManager
#else
class KOAlarmClient : public QObject
#endif
{
  Q_OBJECT
  public:
    explicit KOAlarmClient( QObject *parent = 0 );
    ~KOAlarmClient();

#if !defined(Q_WS_WINCE)
    bool commitData( QSessionManager & );
#endif

    // DBUS interface
    void quit();
    void hide();
    void show();
    void forceAlarmCheck();
    void dumpDebug();
    QStringList dumpAlarms();

    void debugShowDialog();

  public slots:
    void slotQuit();

  protected slots:
    void deferredInit();
    void checkAlarms();

  signals:
    void reminderCount( int );
    void saveAllSignal();

  private:
    bool dockerEnabled();
    bool collectionsAvailable() const;
    void createReminder( const Akonadi::ETMCalendar::Ptr &calendar,
                         const Akonadi::Item &incidence,
                         const QDateTime &dt, const QString &displayText );
    void saveLastCheckTime();

    AlarmDockWindow *mDocker;  // the panel icon
    Akonadi::ETMCalendar::Ptr mCalendar;
    Akonadi::EntityTreeModel *mETM;

    QDateTime mLastChecked;
    QTimer mCheckTimer;

    AlarmDialog *mDialog;
};

#endif
