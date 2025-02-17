/*
  SPDX-FileCopyrightText: 2024-2025 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-only
*/

#include "activitiesmanager.h"
#include "accountactivities.h"
#include "transportactivities.h"

ActivitiesManager::ActivitiesManager(QObject *parent)
    : PimCommonActivities::ActivitiesBaseManager{parent}
    , mAccountActivities(new AccountActivities(this))
    , mTransportActivities(new TransportActivities(this))
{
    connect(this, &ActivitiesManager::activitiesChanged, this, [this]() {
        Q_EMIT mAccountActivities->activitiesChanged();
        Q_EMIT mTransportActivities->activitiesChanged();
    });
}

ActivitiesManager::~ActivitiesManager() = default;

ActivitiesManager *ActivitiesManager::self()
{
    static ActivitiesManager s_self;
    return &s_self;
}

AccountActivities *ActivitiesManager::accountActivities() const
{
    return mAccountActivities;
}

TransportActivities *ActivitiesManager::transportActivities() const
{
    return mTransportActivities;
}

#include "moc_activitiesmanager.cpp"
