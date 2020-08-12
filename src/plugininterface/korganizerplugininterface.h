/*
   SPDX-FileCopyrightText: 2015-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KORGANIZERPLUGININTERFACE_H
#define KORGANIZERPLUGININTERFACE_H

#include <QObject>
#include <PimCommonAkonadi/PluginInterface>
class KOrganizerPluginInterface : public PimCommon::PluginInterface
{
    Q_OBJECT
public:
    explicit KOrganizerPluginInterface(QObject *parent = nullptr);
    ~KOrganizerPluginInterface();
    static KOrganizerPluginInterface *self();
};

#endif // KORGANIZERPLUGININTERFACE_H
