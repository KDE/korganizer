/*
   SPDX-FileCopyrightText: 2015-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KORGMIGRATEAPPLICATION_H
#define KORGMIGRATEAPPLICATION_H

#include <PimCommon/MigrateApplicationFiles>
#include "korganizerprivate_export.h"

class KORGANIZERPRIVATE_EXPORT KOrgMigrateApplication
{
public:
    KOrgMigrateApplication();

    void migrate();
private:
    void initializeMigrator();
    PimCommon::MigrateApplicationFiles mMigrator;
};

#endif // KORGMIGRATEAPPLICATION_H
