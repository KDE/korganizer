/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  based on code from Sune Vuorela <sune@vuorela.dk> (Rawatar source code)

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "korgmigrateapplication.h"

#include <Kdelibs4ConfigMigrator>

KOrgMigrateApplication::KOrgMigrateApplication()
{
    initializeMigrator();
}

void KOrgMigrateApplication::migrate()
{
    //Migrate to xdg
    Kdelibs4ConfigMigrator migrate(QStringLiteral("korganizer"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("korganizer_htmlexportrc") << QStringLiteral("korganizerrc") << QStringLiteral("freebusyurls"));
    migrate.setUiFiles(QStringList() << QStringLiteral("korganizer_part.rc") << QStringLiteral("korganizerui.rc"));
    migrate.migrate();

    // Migrate folders and files.
    if (mMigrator.checkIfNecessary()) {
        mMigrator.start();
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
    migrateInfoTemplates.setType(QStringLiteral("apps"));
    migrateInfoTemplates.setPath(QStringLiteral("korganizer/templates/"));
    migrateInfoTemplates.setVersion(initialVersion);
    mMigrator.insertMigrateInfo(migrateInfoTemplates);

    // Designer
    PimCommon::MigrateFileInfo migrateInfoDesigner;
    migrateInfoDesigner.setFolder(true);
    migrateInfoDesigner.setType(QStringLiteral("apps"));
    migrateInfoDesigner.setPath(QStringLiteral("korganizer/designer/"));
    migrateInfoDesigner.setVersion(initialVersion);
    mMigrator.insertMigrateInfo(migrateInfoDesigner);

    //TODO add folder to migrate
    // If you add new MigrateFileInfo we need to increase "currentVersion"  and initialVersion
}

