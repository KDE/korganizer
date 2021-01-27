/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2002 Adriaan de Groot <groot@kde.org>
  SPDX-FileCopyrightText: 2004 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef KORG_DATECHECKER_H
#define KORG_DATECHECKER_H

#include <QDate>
#include <QObject>

class QTimer;

class DateChecker : public QObject
{
    Q_OBJECT
public:
    explicit DateChecker(QObject *parent = nullptr);
    ~DateChecker() override;

    /**
      The DateChecker automatically checks for
      the passage of midnight. If rollover type is
      set to None, no signals are emitted and no
      processing is done. With rollover set to
      FollowDay, the day highlighter changes at
      midnight and dayPassed() is emitted.
      With FollowMonth, it has the same effect
      as FollowDay but also adjusts the month that is
      visible and emits monthPassed() when the month changes.
    */
    enum RolloverType { None, FollowDay, FollowMonth };
    void enableRollover(RolloverType);

Q_SIGNALS:
    // Signals emitted at midnight carrying the new date.
    void dayPassed(const QDate &);
    void monthPassed(const QDate &);

protected Q_SLOTS:
    /**
      Called regularly to see if we need to update the view
      wrt. the today box and the month box. Only important
      if you leave KOrganizer idle for long periods of time.

      Until we have a reliable way of setting QTimers to go
      off at a particular wall-clock time, we need this,
      which calls passedMidnight() at the right moments.
    */
    void possiblyPastMidnight();

    /**
      Handles updating the view when midnight has come by due to idle time.
    */
    void passedMidnight();

private:
    QTimer *mUpdateTimer = nullptr;
    QDate mLastDayChecked;
    RolloverType mUpdateRollover;
};

#endif
