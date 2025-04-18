/*
  This file is part of Kontact.

  SPDX-FileCopyrightText: 2004, 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "journalplugin.h"
#include "calendarinterface.h"
#include "korg_uniqueapp.h"

#include <KontactInterface/Core>

#include "korganizerplugin_debug.h"
#include <KActionCollection>
#include <KLocalizedString>
#include <QAction>
#include <QIcon>

EXPORT_KONTACT_PLUGIN_WITH_JSON(JournalPlugin, "journalplugin.json")

JournalPlugin::JournalPlugin(KontactInterface::Core *core, const KPluginMetaData &data, const QVariantList &)
    : KontactInterface::Plugin(core, core, data, "korganizer", "journal")
{
    setComponentName(QStringLiteral("korganizer"), i18nc("@info/plain", "KOrganizer"));

    auto action = new QAction(QIcon::fromTheme(QStringLiteral("journal-new")), i18nc("@action:inmenu", "New Journal…"), this);
    actionCollection()->addAction(QStringLiteral("new_journal"), action);
    actionCollection()->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_J));
    QString const str = i18nc("@info:status", "Create a new journal");
    action->setStatusTip(str);
    action->setToolTip(str);

    action->setWhatsThis(i18nc("@info:whatsthis",
                               "You will be presented with a dialog where you can create "
                               "a new journal entry."));
    connect(action, &QAction::triggered, this, &JournalPlugin::slotNewJournal);
    insertNewAction(action);

    mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(new KontactInterface::UniqueAppHandlerFactory<KOrganizerUniqueAppHandler>(), this);
}

JournalPlugin::~JournalPlugin() = default;

KParts::Part *JournalPlugin::createPart()
{
    KParts::Part *part = loadPart();

    if (!part) {
        return nullptr;
    }

    mIface = new OrgKdeKorganizerCalendarInterface(QStringLiteral("org.kde.korganizer"), QStringLiteral("/Calendar"), QDBusConnection::sessionBus(), this);

    return part;
}

void JournalPlugin::select()
{
    interface()->showJournalView();
}

QStringList JournalPlugin::invisibleToolbarActions() const
{
    QStringList invisible;
    invisible += QStringLiteral("new_event");
    invisible += QStringLiteral("new_todo");
    invisible += QStringLiteral("new_journal");

    invisible += QStringLiteral("view_whatsnext");
    invisible += QStringLiteral("view_day");
    invisible += QStringLiteral("view_nextx");
    invisible += QStringLiteral("view_month");
    invisible += QStringLiteral("view_workweek");
    invisible += QStringLiteral("view_week");
    invisible += QStringLiteral("view_list");
    invisible += QStringLiteral("view_todo");
    invisible += QStringLiteral("view_journal");
    invisible += QStringLiteral("view_timeline");

    return invisible;
}

OrgKdeKorganizerCalendarInterface *JournalPlugin::interface()
{
    if (!mIface) {
        (void)part();
    }
    Q_ASSERT(mIface);
    return mIface;
}

void JournalPlugin::slotNewJournal()
{
    interface()->openJournalEditor(QString(), QDate());
}

bool JournalPlugin::isRunningStandalone() const
{
    return mUniqueAppWatcher->isRunningStandalone();
}

#include "journalplugin.moc"

#include "moc_journalplugin.cpp"
