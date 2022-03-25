/*
   SPDX-FileCopyrightText: 2015-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <PimCommonAkonadi/PluginInterface>
#include <QObject>
class KOrganizerPluginInterface : public PimCommon::PluginInterface
{
    Q_OBJECT
public:
    explicit KOrganizerPluginInterface(QObject *parent = nullptr);
    ~KOrganizerPluginInterface() override;
    static KOrganizerPluginInterface *self();
};
