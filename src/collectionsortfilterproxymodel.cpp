/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "collectionsortfilterproxymodel.h"
#include <Akonadi/AccountActivitiesAbstract>

CollectionSortFilterProxyModel::CollectionSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{
}

CollectionSortFilterProxyModel::~CollectionSortFilterProxyModel() = default;

bool CollectionSortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    // TODO
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
}

void CollectionSortFilterProxyModel::setAccountActivities(Akonadi::AccountActivitiesAbstract *accountActivities)
{
    if (mAccountActivities) {
        disconnect(mAccountActivities, &Akonadi::AccountActivitiesAbstract::activitiesChanged, this, &CollectionSortFilterProxyModel::invalidateFilter);
    }
    mAccountActivities = accountActivities;
    if (mAccountActivities) {
        connect(mAccountActivities, &Akonadi::AccountActivitiesAbstract::activitiesChanged, this, &CollectionSortFilterProxyModel::invalidateFilter);
    }
}

#include "moc_collectionsortfilterproxymodel.cpp"
