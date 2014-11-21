/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "korgstartup.h"

#include <kdelibs4configmigrator.h>
#include <QString>
void KOrgStartup::migrateConfig()
{
    Kdelibs4ConfigMigrator migrate(QLatin1String("korganizer"));
    migrate.setConfigFiles(QStringList() << QLatin1String("korganizer_htmlexportrc") << QLatin1String("korganizerrc"));
    migrate.setUiFiles(QStringList() << QLatin1String("korganizer_part.rc") << QLatin1String("korganizerui.rc"));
    migrate.migrate();
}
