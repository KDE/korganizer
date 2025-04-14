/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2002 Adriaan de Groot <groot@kde.org>
  SPDX-FileCopyrightText: 2004 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "datechecker.h"

#include <QTimer>

DateChecker::DateChecker(QObject *parent)
    : QObject(parent)
{
    enableRollover(FollowMonth);
}

DateChecker::~DateChecker() = default;

void DateChecker::enableRollover(RolloverType r)
{
    switch (r) {
    case None:
        if (mUpdateTimer) {
            mUpdateTimer->stop();
            delete mUpdateTimer;
            mUpdateTimer = nullptr;
        }
        break;
    case FollowDay:
    case FollowMonth:
        if (!mUpdateTimer) {
            mUpdateTimer = new QTimer(this);
            connect(mUpdateTimer, &QTimer::timeout, this, &DateChecker::possiblyPastMidnight);
        }
        mUpdateTimer->setSingleShot(true);
        mUpdateTimer->start(0);
        mLastDayChecked = QDate::currentDate();
        break;
    }
    mUpdateRollover = r;
}

void DateChecker::passedMidnight()
{
    QDate const today = QDate::currentDate();

    if (today.month() != mLastDayChecked.month()) {
        if (mUpdateRollover == FollowMonth) {
            Q_EMIT monthPassed(today);
        }
    }
    Q_EMIT dayPassed(today);
}

void DateChecker::possiblyPastMidnight()
{
    if (mLastDayChecked != QDate::currentDate()) {
        passedMidnight();
        mLastDayChecked = QDate::currentDate();
    }
    // Set the timer to go off 1 second after midnight
    // or after 8 minutes, whichever comes first.
    if (mUpdateTimer) {
        const QTime now = QTime::currentTime();
        const QTime midnight = QTime(23, 59, 59);
        const int msecsWait = qMin(480000, now.msecsTo(midnight) + 2000);

        mUpdateTimer->stop();
        mUpdateTimer->start(msecsWait);
    }
}

#include "moc_datechecker.cpp"
