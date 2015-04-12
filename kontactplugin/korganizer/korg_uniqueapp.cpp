/*
  This file is part of KDE Kontact.

  Copyright (c) 2003 David Faure <faure@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "korg_uniqueapp.h"
#include "korganizer/korganizer_options.h"

#include <KontactInterface/Core>

#include <KStartupInfo>
#include <KWindowSystem>

#include <QDBusMessage>
#include <QDBusConnection>

void KOrganizerUniqueAppHandler::loadCommandLineOptions()
{
    KCmdLineArgs::addCmdLineOptions(korganizer_options());
}

int KOrganizerUniqueAppHandler::newInstance()
{
    // Ensure part is loaded
    (void)plugin()->part();

    QDBusMessage message = QDBusMessage::createMethodCall(QLatin1String("org.kde.korganizer"),
                           QLatin1String("/Korganizer"),
                           QLatin1String("org.kde.korganizer.Korganizer"),
                           QLatin1String("handleCommandLine"));
    QDBusConnection::sessionBus().send(message);

    // Bring korganizer's plugin to front
    // This bit is duplicated from KUniqueApplication::newInstance()
    QWidget *mWidget = mainWidget();
    if (mWidget) {
        mWidget->show();
        KWindowSystem::forceActiveWindow(mWidget->winId());
        KStartupInfo::appStarted();
    }

    // Then ensure the part appears in kontact.
    // ALWAYS use the korganizer plugin; i.e. never show the todo nor journal
    // plugins when creating a new instance via the command line, even if
    // the command line options are empty; else we'd need to examine the
    // options and then figure out which plugin we should show.
    // kolab/issue3971
    plugin()->core()->selectPlugin(QLatin1String("kontact_korganizerplugin"));
    return 0;
}
