/*
   SPDX-FileCopyrightText: 2015-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "korganizerprivate_export.h"
#include <PimCommon/MigrateApplicationFiles>

class KORGANIZERPRIVATE_EXPORT KOrgMigrateApplication
{
public:
    KOrgMigrateApplication();

    void migrate();

private:
    void initializeMigrator();
    PimCommon::MigrateApplicationFiles mMigrator;
};

