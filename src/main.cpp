/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1997-1999 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2001, 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/

#include "aboutdata.h"
#include "koapp.h"
#include "korganizer.h"
#include "korganizer_debug.h"
#include "korganizer_options.h"
#include <KConfig>
#include <KConfigGroup>
#include <KCrash>
#include <KLocalizedString>

#if KORGANIZER_WITH_KUSERFEEDBACK
#include "userfeedback/korganizeruserfeedbackprovider.h"
#endif

#include <KIconTheme>

#include <KStyleManager>

int main(int argc, char **argv)
{
    KIconTheme::initTheme();
    KOrganizerApp app(argc, &argv);
    KStyleManager::initStyle();
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("korganizer"));

    KOrg::AboutData aboutData;
    app.setAboutData(aboutData);
    KCrash::initialize();

    QCommandLineParser *cmdArgs = app.cmdArgs();
    korganizer_options(cmdArgs);

    const QStringList args = QApplication::arguments();
    cmdArgs->process(args);
    aboutData.processCommandLine(cmdArgs);
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("korganizer")));

#if KORGANIZER_WITH_KUSERFEEDBACK
    if (cmdArgs->isSet(QStringLiteral("feedback"))) {
        auto userFeedBackProvider = new KOrganizerUserFeedbackProvider(nullptr);
        QTextStream(stdout) << userFeedBackProvider->describeDataSources() << '\n';
        delete userFeedBackProvider;
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

    // information for the reminder daemon
    KConfig cfg(QStringLiteral("defaultcalendarrc"));
    KConfigGroup grp(&cfg, QStringLiteral("General"));
    grp.writeEntry(QStringLiteral("ApplicationId"), QStringLiteral("org.kde.korganizer"));

    return app.exec();
}
