/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2003 David Faure <faure@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "korg_uniqueapp.h"
#include "../../../src/korganizer_options.h"
#include "config-korganizer.h"

#include <KontactInterface/Core>

#if KDEPIM_HAVE_X11
#include <KStartupInfo>
#endif

#include <KWindowSystem>

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
        KWindowSystem::activateWindow(mWidget->windowHandle());
#if KDEPIM_HAVE_X11
        KStartupInfo::appStarted();
#endif
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

#include "moc_korg_uniqueapp.cpp"
