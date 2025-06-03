/*
  SPDX-FileCopyrightText: 2024-2025 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once
#include "korganizerprivate_export.h"
#include <PimCommonActivities/ActivitiesBaseManager>
class TransportActivities;
class AccountActivities;
class KORGANIZERPRIVATE_EXPORT ActivitiesManager : public PimCommonActivities::ActivitiesBaseManager
{
    Q_OBJECT
public:
    static ActivitiesManager *self();

    explicit ActivitiesManager(QObject *parent = nullptr);
    ~ActivitiesManager() override;

    [[nodiscard]] TransportActivities *transportActivities() const;
    [[nodiscard]] AccountActivities *accountActivities() const;

private:
    AccountActivities *const mAccountActivities;
    TransportActivities *const mTransportActivities;
};
