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
#include <KConfig>
#include <KConfigGroup>
#include <KCrash>
#include <KLocalizedString>

#ifdef WITH_KUSERFEEDBACK
#include "userfeedback/korganizeruserfeedbackprovider.h"
#endif

#define HAVE_KICONTHEME __has_include(<KIconTheme>)
#if HAVE_KICONTHEME
#include <KIconTheme>
#endif

#define HAVE_STYLE_MANAGER __has_include(<KStyleManager>)
#if HAVE_STYLE_MANAGER
#include <KStyleManager>
#endif

int main(int argc, char **argv)
{
#if HAVE_KICONTHEME && (KICONTHEMES_VERSION >= QT_VERSION_CHECK(6, 3, 0))
    KIconTheme::initTheme();
#endif
    KOrganizerApp app(argc, &argv);
    KCrash::initialize();
#if HAVE_STYLE_MANAGER
    KStyleManager::initStyle();
#else // !HAVE_STYLE_MANAGER
#if defined(Q_OS_MACOS) || defined(Q_OS_WIN)
    QApplication::setStyle(QStringLiteral("breeze"));
#endif // defined(Q_OS_MACOS) || defined(Q_OS_WIN)
#endif // HAVE_STYLE_MANAGER
    KLocalizedString::setApplicationDomain(QByteArrayLiteral("korganizer"));

    KOrg::AboutData aboutData;
    app.setAboutData(aboutData);

    QCommandLineParser *cmdArgs = app.cmdArgs();
    korganizer_options(cmdArgs);

    const QStringList args = QApplication::arguments();
    cmdArgs->process(args);
    aboutData.processCommandLine(cmdArgs);
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("korganizer")));

#ifdef WITH_KUSERFEEDBACK
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
