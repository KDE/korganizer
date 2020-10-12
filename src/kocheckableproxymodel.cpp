/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2012 Sergio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kocheckableproxymodel.h"

KOCheckableProxyModel::KOCheckableProxyModel(QObject *parent) : KCheckableProxyModel(parent)
{
}

bool KOCheckableProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    const Qt::CheckState newState = static_cast<Qt::CheckState>(value.toInt());
    if (role == Qt::CheckStateRole && index.column() == 0) {
        Q_EMIT aboutToToggle(newState);
    }

    const bool result = KCheckableProxyModel::setData(index, value, role);

    if (result) {
        Q_EMIT toggled(newState);
    }
    return result;
}
