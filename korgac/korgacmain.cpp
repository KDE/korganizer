/*
  This file is part of the KDE reminder agent.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koalarmclient.h"
#include "korganizer-version.h"

#include <KAboutData>
#include <KDBusService>
#include <KLocalizedString>

#include <kcoreaddons_version.h>
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Kdelibs4ConfigMigrator>
#endif
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Kdelibs4ConfigMigrator migrate(QStringLiteral("korgac"));

    migrate.setConfigFiles(QStringList() << QStringLiteral("korgacrc"));
    migrate.migrate();
#endif

    KAboutData aboutData(QStringLiteral("korgac"),
                         i18n("KOrganizer Reminder Daemon"),
                         QStringLiteral(KORGANIZER_VERSION),
                         i18n("KOrganizer Reminder Daemon"),
                         KAboutLicense::GPL,
                         i18n("(c) 2003 Cornelius Schumacher"),
                         QString(),
                         QStringLiteral("https://community.kde.org/KDE_PIM/"));
    aboutData.addAuthor(i18n("Cornelius Schumacher"), i18n("Former Maintainer"), QStringLiteral("schumacher@kde.org"));
    aboutData.addAuthor(i18n("Reinhold Kainhofer"), i18n("Former Maintainer"), QStringLiteral("kainhofer@kde.org"));
    aboutData.addAuthor(i18n("Allen Winter"), i18n("Janitorial Staff"), QStringLiteral("winter@kde.org"));

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KDBusService service(KDBusService::Unique);
    KOAlarmClient client;

    return app.exec();
}
