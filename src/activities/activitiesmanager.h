/*
  SPDX-FileCopyrightText: 2024-2025 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-only
*/

#pragma once
#include "korganizerprivate_export.h"
#include <PimCommonActivities/ActivitiesBaseManager>
namespace KActivities
{
class Consumer;
}
class TransportActivities;
class AccountActivities;
class KORGANIZERPRIVATE_EXPORT ActivitiesManager : public PimCommonActivities::ActivitiesBaseManager
{
    Q_OBJECT
public:
    static ActivitiesManager *self();

    explicit ActivitiesManager(QObject *parent = nullptr);
    ~ActivitiesManager() override;

    [[nodiscard]] bool enabled() const override;
    void setEnabled(bool newEnabled);

    [[nodiscard]] TransportActivities *transportActivities() const;
    [[nodiscard]] AccountActivities *accountActivities() const;

private:
    KActivities::Consumer *const mActivitiesConsumer;
    AccountActivities *const mAccountActivities;
    TransportActivities *const mTransportActivities;
    bool mEnabled = false;
};
