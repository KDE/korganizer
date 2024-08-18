/*
  SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-only
*/

#include "activitiesmanager.h"
#include "accountactivities.h"

#include <PlasmaActivities/Consumer>

ActivitiesManager::ActivitiesManager(QObject *parent)
    : PimCommonActivities::ActivitiesBaseManager{parent}
    , mActivitiesConsumer(new KActivities::Consumer(this))
    , mAccountActivities(new AccountActivities(this))
{
    connect(this, &ActivitiesManager::activitiesChanged, this, [this]() {
        Q_EMIT mAccountActivities->activitiesChanged();
    });
}

ActivitiesManager::~ActivitiesManager() = default;

ActivitiesManager *ActivitiesManager::self()
{
    static ActivitiesManager s_self;
    return &s_self;
}

bool ActivitiesManager::enabled() const
{
    return mEnabled;
}

void ActivitiesManager::setEnabled(bool newEnabled)
{
    if (mEnabled != newEnabled) {
        mEnabled = newEnabled;
        Q_EMIT activitiesChanged();
    }
}

AccountActivities *ActivitiesManager::accountActivities() const
{
    return mAccountActivities;
}

#include "moc_activitiesmanager.cpp"
