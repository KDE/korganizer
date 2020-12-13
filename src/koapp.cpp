/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2000, 2001, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koapp.h"
#include "actionmanager.h"
#include "calendarview.h"
#include "korganizer.h"
#include "korganizer-version.h"

#include <KCalendarCore/CalFormat>

#include "korganizer_debug.h"
#include "korganizer_options.h"

#include <QCommandLineParser>
#include <QDBusConnectionInterface>

#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>

#include <KMessageBox>

KOrganizerApp::KOrganizerApp(int &argc, char **argv[])
    : KontactInterface::PimUniqueApplication(argc, argv)
{
    const QString prodId = QStringLiteral("-//K Desktop Environment//NONSGML KOrganizer %1//EN");
    KCalendarCore::CalFormat::setApplication(QStringLiteral("KOrganizer"),
                                             prodId.arg(QStringLiteral(KORGANIZER_VERSION)));
}

KOrganizerApp::~KOrganizerApp()
{
}

int KOrganizerApp::activate(const QStringList &args, const QString &workingDir)
{
    Q_UNUSED(workingDir)
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

    QDBusConnection::sessionBus().interface()->startService(QStringLiteral("org.kde.korgac"));

    QCommandLineParser parser;
    korganizer_options(&parser);
    parser.process(args);

    if (parser.isSet(QStringLiteral("view"))) {
        processCalendar(QUrl(), false);
        const auto url = QUrl{parser.value(QStringLiteral("view"))};
        auto fetchJob = new Akonadi::ItemFetchJob(Akonadi::Item::fromUrl(url), this);
        fetchJob->fetchScope().fetchFullPayload();
        connect(fetchJob, &Akonadi::ItemFetchJob::result,
                this, [](KJob *job) {
            if (job->error()) {
                KMessageBox::detailedSorry(nullptr, i18n("Failed to retrieve incidence from Akonadi"), job->errorText());
                return;
            }
            auto fetchJob = static_cast<Akonadi::ItemFetchJob *>(job);
            if (fetchJob->count() != 1) {
                KMessageBox::sorry(nullptr, i18n("Failed to retrieve incidence from Akonadi: requested incidence doesn't exist."));
                return;
            }
            KOrg::MainWindow *korg = ActionManager::findInstance(QUrl());
            korg->actionManager()->view()->showIncidence(fetchJob->items().first());
        });
        return 0;
    }

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
        const auto lst = parser.positionalArguments();
        for (const QString &url : lst) {
            korg->actionManager()->importURL(QUrl::fromUserInput(url), false);
        }
    } else if (parser.isSet(QStringLiteral("merge"))) {
        const auto lst = parser.positionalArguments();
        for (const QString &url : lst) {
            korg->actionManager()->importURL(QUrl::fromUserInput(url), true);
        }
    } else {
        const auto lst = parser.positionalArguments();
        for (const QString &url : lst) {
            korg->actionManager()->importCalendar(QUrl::fromUserInput(url));
        }
    }

    return 0;
}

void KOrganizerApp::processCalendar(const QUrl &url, bool show)
{
    KOrg::MainWindow *korg = ActionManager::findInstance(url);
    if (!korg) {
        bool hasDocument = !url.isEmpty();
        korg = new KOrganizer();
        korg->init(hasDocument);
        if (show) {
            korg->topLevelWidget()->show();
        }

        qCDebug(KORGANIZER_LOG) << url.url();

        if (hasDocument) {
            korg->openURL(url);
        } else {
            //      korg->view()->updateView();
        }
    } else if (show) {
        korg->topLevelWidget()->show();
    }
}
