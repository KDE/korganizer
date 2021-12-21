/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1997-1999 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2001, 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "aboutdata.h"
#include "koapp.h"
#include "korganizer.h"
#include "korganizer_debug.h"
#include "korganizer_options.h"
#include <kcoreaddons_version.h>
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "korgmigrateapplication.h"
#endif
#include <KCrash>
#include <KLocalizedString>

#ifdef WITH_KUSERFEEDBACK
#include "userfeedback/korganizeruserfeedbackprovider.h"
#endif
int main(int argc, char **argv)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
#endif
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    KOrganizerApp app(argc, &argv);
    KCrash::initialize();
    KLocalizedString::setApplicationDomain("korganizer");
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(6, 0, 0)
    KOrgMigrateApplication migrate;
    migrate.migrate();
#endif

    KOrg::AboutData aboutData;
    app.setAboutData(aboutData);

    QCommandLineParser *cmdArgs = app.cmdArgs();
    korganizer_options(cmdArgs);

    const QStringList args = QApplication::arguments();
    cmdArgs->process(args);
    aboutData.processCommandLine(cmdArgs);

#ifdef WITH_KUSERFEEDBACK
    if (cmdArgs->isSet(QStringLiteral("feedback"))) {
        auto userFeedBackProvider = new KOrganizerUserFeedbackProvider(nullptr);
        QTextStream(stdout) << userFeedBackProvider->describeDataSources() << '\n';
        return 0;
    }
#endif
    if (!KOrganizerApp::start(args)) {
        qCDebug(KORGANIZER_LOG) << "korganizer already running, exiting";
        return 0;
    }

    if (app.isSessionRestored()) {
        kRestoreMainWindows<KOrganizer>();
    }

    return app.exec();
}
