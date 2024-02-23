/*
   SPDX-FileCopyrightText: 2015-2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <PimCommonAkonadi/PluginInterface>
#include <QObject>
class KOrganizerPluginInterface : public PimCommon::PluginInterface
{
    Q_OBJECT
public:
    ~KOrganizerPluginInterface() override;
    static KOrganizerPluginInterface *self();

private:
    explicit KOrganizerPluginInterface(QObject *parent = nullptr);
};
