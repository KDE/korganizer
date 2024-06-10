/*
  SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-only
*/

#pragma once
#include "korganizerprivate_export.h"
#include <QObject>
namespace KActivities
{
class Consumer;
}
class AccountActivities;
class KORGANIZERPRIVATE_EXPORT ActivitiesManager : public QObject
{
    Q_OBJECT
public:
    static ActivitiesManager *self();

    explicit ActivitiesManager(QObject *parent = nullptr);
    ~ActivitiesManager() override;

    [[nodiscard]] bool enabled() const;
    void setEnabled(bool newEnabled);

    [[nodiscard]] bool isInCurrentActivity(const QStringList &lst) const;
    [[nodiscard]] QString currentActivity() const;

    AccountActivities *accountActivities() const;

Q_SIGNALS:
    void activitiesChanged();

private:
    KActivities::Consumer *const mActivitiesConsumer;
    AccountActivities *const mAccountActivities;
    bool mEnabled = false;
};
