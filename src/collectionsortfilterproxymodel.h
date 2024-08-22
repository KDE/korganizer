/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2024 Laurent Montel <montel@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <QSortFilterProxyModel>

class CollectionSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit CollectionSortFilterProxyModel(QObject *parent = nullptr);
    ~CollectionSortFilterProxyModel() override;
};
