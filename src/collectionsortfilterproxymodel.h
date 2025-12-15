/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2024-2025 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#pragma once

#include <QSortFilterProxyModel>
namespace Akonadi
{
class AccountActivitiesAbstract;
}
class CollectionSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit CollectionSortFilterProxyModel(QObject *parent = nullptr);
    ~CollectionSortFilterProxyModel() override;

    void setAccountActivities(Akonadi::AccountActivitiesAbstract *accountActivities);

protected:
    [[nodiscard]] bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    void slotInvalidateFilter();
    Akonadi::AccountActivitiesAbstract *mAccountActivities = nullptr;
};
