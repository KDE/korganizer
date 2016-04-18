/*
  This file is part of the KDE reminder agent.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "koalarmclient.h"
#include "kdepim-version.h"

#include <kaboutdata.h>

#include <kdelibs4configmigrator.h>
#include <stdlib.h>
#include <QCommandLineParser>
#include <KDBusService>
#include <KLocalizedString>
#include <QApplication>
#ifdef SERIALIZER_PLUGIN_STATIC

Q_IMPORT_PLUGIN(akonadi_serializer_kcalcore)
#endif

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#if QT_VERSION >= 0x050600
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    Kdelibs4ConfigMigrator migrate(QStringLiteral("korgac"));

    migrate.setConfigFiles(QStringList() << QStringLiteral("korgacrc"));
    migrate.migrate();

    KAboutData aboutData(QStringLiteral("korgac"), i18n("KOrganizer Reminder Daemon"),
                         QStringLiteral(KDEPIM_VERSION), i18n("KOrganizer Reminder Daemon"),
                         KAboutLicense::GPL,
                         i18n("(c) 2003 Cornelius Schumacher"),
                         QString(), QStringLiteral("http://pim.kde.org"));
    aboutData.addAuthor(i18n("Cornelius Schumacher"), i18n("Former Maintainer"),
                        QStringLiteral("schumacher@kde.org"));
    aboutData.addAuthor(i18n("Reinhold Kainhofer"), i18n("Former Maintainer"),
                        QStringLiteral("kainhofer@kde.org"));
    aboutData.addAuthor(i18n("Allen Winter"), i18n("Janitorial Staff"),
                        QStringLiteral("winter@kde.org"));

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KDBusService service(KDBusService::Unique);
    KOAlarmClient client;
    //app.disableSessionManagement();

    return app.exec();
}
