/*
   Copyright (C) 2015-2018 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
    migrate.setConfigFiles(QStringList() << QStringLiteral("korganizerrc")
                                         << QStringLiteral("freebusyurls"));
    migrate.setUiFiles(QStringList() << QStringLiteral("korganizer_part.rc")
                                     << QStringLiteral("korganizerui.rc"));
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

    //TODO add folder to migrate
    // If you add new MigrateFileInfo we need to increase "currentVersion"  and initialVersion
}
