/*
  This file is part of Kontact.

  Copyright (c) 2004,2009 Allen Winter <winter@kde.org>

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

#include "journalplugin.h"
#include "calendarinterface.h"
#include "korg_uniqueapp.h"

#include <KontactInterface/Core>

#include <KActionCollection>
#include <KIconLoader>
#include <KLocalizedString>
#include <QDebug>
#include <QtDBus/QtDBus>
#include <QAction>
#include <QIcon>

EXPORT_KONTACT_PLUGIN(JournalPlugin, journal)

JournalPlugin::JournalPlugin(KontactInterface::Core *core, const QVariantList &)
    : KontactInterface::Plugin(core, core, "korganizer", "journal"), mIface(Q_NULLPTR)
{
    setComponentName(QStringLiteral("korganizer"), QStringLiteral("korganizer"));

    KIconLoader::global()->addAppDir(QStringLiteral("korganizer"));
    KIconLoader::global()->addAppDir(QStringLiteral("kdepim"));

    QAction *action =
        new QAction(QIcon::fromTheme(QStringLiteral("journal-new")),
                    i18nc("@action:inmenu", "New Journal..."), this);
    actionCollection()->addAction(QStringLiteral("new_journal"), action);
    actionCollection()->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_J));
    QString str = i18nc("@info:status", "Create a new journal");
    action->setStatusTip(str);
    action->setToolTip(str);

    action->setWhatsThis(
        i18nc("@info:whatsthis",
              "You will be presented with a dialog where you can create "
              "a new journal entry."));
    connect(action, &QAction::triggered, this, &JournalPlugin::slotNewJournal);
    insertNewAction(action);

    QAction *syncAction =
        new QAction(QIcon::fromTheme(QStringLiteral("view-refresh")),
                    i18nc("@action:inmenu", "Sync Journal"), this);
    actionCollection()->addAction(QStringLiteral("journal_sync"), syncAction);
    str = i18nc("@info:status", "Synchronize groupware journal");
    syncAction->setStatusTip(str);
    syncAction->setToolTip(str);

    syncAction->setWhatsThis(
        i18nc("@info:whatsthis",
              "Choose this option to synchronize your groupware journal entries."));
    connect(syncAction, &QAction::triggered, this, &JournalPlugin::slotSyncJournal);
    insertSyncAction(syncAction);

    mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(
        new KontactInterface::UniqueAppHandlerFactory<KOrganizerUniqueAppHandler>(), this);
}

JournalPlugin::~JournalPlugin()
{
}

KParts::ReadOnlyPart *JournalPlugin::createPart()
{
    KParts::ReadOnlyPart *part = loadPart();

    if (!part) {
        return Q_NULLPTR;
    }

    mIface = new OrgKdeKorganizerCalendarInterface(
        QStringLiteral("org.kde.korganizer"), QStringLiteral("/Calendar"), QDBusConnection::sessionBus(), this);

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
    invisible += QStringLiteral("view_timespent");

    return invisible;
}

OrgKdeKorganizerCalendarInterface *JournalPlugin::interface()
{
    if (!mIface) {
        part();
    }
    Q_ASSERT(mIface);
    return mIface;
}

void JournalPlugin::slotNewJournal()
{
    interface()->openJournalEditor(QString(), QDate());
}

void JournalPlugin::slotSyncJournal()
{
#if 0 //TODO porting !!!!
    QDBusMessage message =
        QDBusMessage::createMethodCall("org.kde.kmail", "/Groupware",
                                       "org.kde.kmail.groupware",
                                       "triggerSync");
    message << QString("Journal");
    QDBusConnection::sessionBus().send(message);
#else
    qWarning() << " JournalPlugin::slotSyncJournal : need to port to Akonadi";
#endif
}

bool JournalPlugin::createDBUSInterface(const QString &serviceType)
{
    if (serviceType == QStringLiteral("DBUS/Organizer") || serviceType == QStringLiteral("DBUS/Calendar")) {
        if (part()) {
            return true;
        }
    }
    return false;
}

bool JournalPlugin::isRunningStandalone() const
{
    return mUniqueAppWatcher->isRunningStandalone();
}
#include "journalplugin.moc"
