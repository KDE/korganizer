/*
   SPDX-FileCopyrightText: 2015-2021 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "korgmigrateapplication.h"

#include <Kdelibs4ConfigMigrator>

KOrgMigrateApplication::KOrgMigrateApplication()
{
    initializeMigrator();
}

void KOrgMigrateApplication::migrate()
{
    // Migrate to xdg
    Kdelibs4ConfigMigrator migrate(QStringLiteral("korganizer"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("korganizerrc") << QStringLiteral("freebusyurls"));
    migrate.setUiFiles(QStringList() << QStringLiteral("korganizer_part.rc") << QStringLiteral("korganizerui.rc"));
    migrate.migrate();

    // Migrate folders and files.
    if (mMigrator.checkIfNecessary()) {
        (void) mMigrator.start();
    }
}

void KOrgMigrateApplication::initializeMigrator()
{
    const int currentVersion = 2;
    mMigrator.setApplicationName(QStringLiteral("korganizer"));
    mMigrator.setConfigFileName(QStringLiteral("korganizerrc"));
    mMigrator.setCurrentConfigVersion(currentVersion);

    // To migrate we need a version < currentVersion
    const int initialVersion = currentVersion + 1;

    // Templates
    PimCommon::MigrateFileInfo migrateInfoTemplates;
    migrateInfoTemplates.setFolder(true);
    migrateInfoTemplates.setType(QStringLiteral("data"));
    migrateInfoTemplates.setPath(QStringLiteral("korganizer/templates/"));
    migrateInfoTemplates.setVersion(initialVersion);
    mMigrator.insertMigrateInfo(migrateInfoTemplates);

    // Designer
    PimCommon::MigrateFileInfo migrateInfoDesigner;
    migrateInfoDesigner.setFolder(true);
    migrateInfoDesigner.setType(QStringLiteral("data"));
    migrateInfoDesigner.setPath(QStringLiteral("korganizer/designer/"));
    migrateInfoDesigner.setVersion(initialVersion);
    mMigrator.insertMigrateInfo(migrateInfoDesigner);

    // TODO add folder to migrate
    // If you add new MigrateFileInfo we need to increase "currentVersion"  and initialVersion
}
