/*
   SPDX-FileCopyrightText: 2015-2026 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "korganizerplugininterface.h"

KOrganizerPluginInterface::KOrganizerPluginInterface(QObject *parent)
    : PimCommon::PluginInterface(parent)
{
    setPluginName(QStringLiteral("korganizer"));
    setPluginDirectory(QStringLiteral("korganizer/mainview"));
}

KOrganizerPluginInterface::~KOrganizerPluginInterface() = default;

KOrganizerPluginInterface *KOrganizerPluginInterface::self()
{
    static KOrganizerPluginInterface s_self;
    return &s_self;
}

#include "moc_korganizerplugininterface.cpp"
