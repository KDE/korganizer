/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2024-2025 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "collectionsortfilterproxymodel.h"
#include <Akonadi/AccountActivitiesAbstract>
#include <Akonadi/AgentInstance>
#include <Akonadi/AgentManager>
#include <Akonadi/EntityTreeModel>

CollectionSortFilterProxyModel::CollectionSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{
}

CollectionSortFilterProxyModel::~CollectionSortFilterProxyModel() = default;

bool CollectionSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (mAccountActivities) {
        const QModelIndex modelIndex = sourceModel()->index(source_row, 0, source_parent);
        Q_ASSERT(modelIndex.isValid());

        const auto collection = sourceModel()->data(modelIndex, Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
        const Akonadi::AgentInstance instance = Akonadi::AgentManager::self()->instance(collection.resource());
        if (instance.activitiesEnabled()) {
            return mAccountActivities->filterAcceptsRow(instance.activities());
        }
    }
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

void CollectionSortFilterProxyModel::setAccountActivities(Akonadi::AccountActivitiesAbstract *accountActivities)
{
    if (mAccountActivities) {
        disconnect(mAccountActivities, &Akonadi::AccountActivitiesAbstract::activitiesChanged, this, &CollectionSortFilterProxyModel::slotInvalidateFilter);
    }
    mAccountActivities = accountActivities;
    if (mAccountActivities) {
        connect(mAccountActivities, &Akonadi::AccountActivitiesAbstract::activitiesChanged, this, &CollectionSortFilterProxyModel::slotInvalidateFilter);
    }
}

void CollectionSortFilterProxyModel::slotInvalidateFilter()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    beginFilterChange();
    endFilterChange(QSortFilterProxyModel::Direction::Rows);
#else
    invalidateFilter();
#endif
}

#include "moc_collectionsortfilterproxymodel.cpp"
