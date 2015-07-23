/*
  This file is part of KOrganizer.

  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "koapp.h"
#include "actionmanager.h"
#include "calendarview.h"
#include "korganizer.h"
#include "reminderclient.h"
#include "kdepim-version.h"
#include <KCalCore/CalFormat>
#include <KStartupInfo>

#include "korganizer_debug.h"
#include "korganizer_options.h"
#include <KStartupInfo>
#include <QCommandLineParser>

KOrganizerApp::KOrganizerApp(int &argc, char **argv[])
    : KontactInterface::PimUniqueApplication(argc, argv)
{
    QString prodId = QStringLiteral("-//K Desktop Environment//NONSGML KOrganizer %1//EN");
    KCalCore::CalFormat::setApplication(QStringLiteral("KOrganizer"), prodId.arg(QStringLiteral(KDEPIM_VERSION)));
}

KOrganizerApp::~KOrganizerApp()
{
}

int KOrganizerApp::activate(const QStringList &args)
{
    qCDebug(KORGANIZER_LOG);
    static bool first = true;
    if (isSessionRestored() && first) {
        KOrg::MainWindow *korg = ActionManager::findInstance(QUrl());
        if (korg) {
            korg->view()->updateView();
        }
        first = false;
        return 0;
    }
    first = false;

    QCommandLineParser parser;
    korganizer_options(&parser);
    parser.process(args);

    KPIM::ReminderClient::startDaemon();

    // No filenames given => all other args are meaningless, show main Window
    if (parser.positionalArguments().isEmpty()) {
        processCalendar(QUrl());
        return 0;
    }

    // If filenames were given as arguments, load them as calendars, one per window.
    // Import, merge, or ask => we need the resource calendar window anyway.
    processCalendar(QUrl());
    KOrg::MainWindow *korg = ActionManager::findInstance(QUrl());
    if (!korg) {
        qCCritical(KORGANIZER_LOG) << "Unable to find default calendar resources view.";
        return -1;
    }
    // Check for import, merge or ask
    if (parser.isSet(QStringLiteral("import"))) {
        for (const QString &url : parser.positionalArguments()) {
            korg->actionManager()->importURL(QUrl::fromUserInput(url), false);
        }
    } else if (parser.isSet(QStringLiteral("merge"))) {
        for (const QString &url : parser.positionalArguments()) {
            korg->actionManager()->importURL(QUrl::fromUserInput(url), true);
        }
    } else {
        for (const QString &url : parser.positionalArguments()) {
            korg->actionManager()->importCalendar(QUrl::fromUserInput(url));
        }
    }

    return 0;
}

void KOrganizerApp::processCalendar(const QUrl &url)
{
    KOrg::MainWindow *korg = ActionManager::findInstance(url);
    if (!korg) {
        bool hasDocument = !url.isEmpty();
        korg = new KOrganizer();
        korg->init(hasDocument);
        korg->topLevelWidget()->show();

        qCDebug(KORGANIZER_LOG) << url.url();

        if (hasDocument) {
            korg->openURL(url);
        } else {
            //      korg->view()->updateView();
        }
    } else {
        korg->topLevelWidget()->show();
    }

    // Handle window activation
#if defined Q_OS_X11 && ! defined K_WS_QTONLY
    KStartupInfo::setNewStartupId(korg->topLevelWidget(), startupId());
#endif
}

