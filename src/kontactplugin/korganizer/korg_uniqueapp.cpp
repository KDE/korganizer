/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2003 David Faure <faure@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "korg_uniqueapp.h"
#include "../../../src/korganizer_options.h"

#include <KontactInterface/Core>

#include <KStartupInfo>
#include <KWindowSystem>
#include <config-korganizer.h>

#include "kwindowsystem_version.h"
#if KWINDOWSYSTEM_VERSION >= QT_VERSION_CHECK(5, 101, 0)
#include <KX11Extras>
#endif

#include <QDBusConnection>
#include <QDBusMessage>

void KOrganizerUniqueAppHandler::loadCommandLineOptions(QCommandLineParser *parser)
{
    korganizer_options(parser);
}

int KOrganizerUniqueAppHandler::activate(const QStringList &args, const QString &workingDir)
{
    Q_UNUSED(workingDir)

    // Ensure part is loaded
    (void)plugin()->part();

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.korganizer"),
                                                          QStringLiteral("/Korganizer"),
                                                          QStringLiteral("org.kde.korganizer.Korganizer"),
                                                          QStringLiteral("handleCommandLine"));
    message.setArguments(QList<QVariant>() << (args));
    QDBusConnection::sessionBus().send(message);

    // Bring korganizer's plugin to front
    // This bit is duplicated from KUniqueApplication::newInstance()
    QWidget *mWidget = mainWidget();
    if (mWidget) {
        mWidget->show();
#if KWINDOWSYSTEM_VERSION < QT_VERSION_CHECK(5, 101, 0)
        KWindowSystem::forceActiveWindow(mWidget->winId());
#else
#if KDEPIM_HAVE_X11
        KX11Extras::forceActiveWindow(mWidget->winId());
#endif
#endif
        KStartupInfo::appStarted();
    }

    // Then ensure the part appears in kontact.
    // ALWAYS use the korganizer plugin; i.e. never show the todo nor journal
    // plugins when creating a new instance via the command line, even if
    // the command line options are empty; else we'd need to examine the
    // options and then figure out which plugin we should show.
    // kolab/issue3971
    plugin()->core()->selectPlugin(QStringLiteral("kontact_korganizerplugin"));
    return 0;
}
