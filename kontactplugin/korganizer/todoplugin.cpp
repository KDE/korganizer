/*
  This file is part of Kontact.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "todoplugin.h"
#include "calendarinterface.h"
#include "korg_uniqueapp.h"
#include "todosummarywidget.h"

#include <libkdepim/misc/maillistdrag.h>

#include <KContacts/VCardDrag>

#include <KCalCore/MemoryCalendar>

#include <KCalUtils/ICalDrag>

#include <KontactInterface/Core>

#include <QAction>
#include <KActionCollection>
#include "korganizerplugin_debug.h"
#include <QIcon>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSystemTimeZone>
#include <QTemporaryFile>

#include <QDropEvent>

EXPORT_KONTACT_PLUGIN(TodoPlugin, todo)

TodoPlugin::TodoPlugin(KontactInterface::Core *core, const QVariantList &)
    : KontactInterface::Plugin(core, core, "korganizer", "todo"), mIface(Q_NULLPTR)
{
    setComponentName(QStringLiteral("korganizer"), QStringLiteral("korganizer"));
    KIconLoader::global()->addAppDir(QStringLiteral("korganizer"));
    KIconLoader::global()->addAppDir(QStringLiteral("kdepim"));

    QAction *action =
        new QAction(QIcon::fromTheme(QStringLiteral("task-new")),
                    i18nc("@action:inmenu", "New To-do..."), this);
    actionCollection()->addAction(QStringLiteral("new_todo"), action);
    actionCollection()->setDefaultShortcut(action, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_T));
    QString str = i18nc("@info:status", "Create a new to-do");
    action->setStatusTip(str);
    action->setToolTip(str);

    action->setWhatsThis(
        i18nc("@info:whatsthis",
              "You will be presented with a dialog where you can create a new to-do item."));
    connect(action, &QAction::triggered, this, &TodoPlugin::slotNewTodo);
    insertNewAction(action);

    QAction *syncAction =
        new QAction(QIcon::fromTheme(QStringLiteral("view-refresh")),
                    i18nc("@action:inmenu", "Sync To-do List"), this);
    str = i18nc("@info:status", "Synchronize groupware to-do list");
    syncAction->setStatusTip(str);
    syncAction->setToolTip(str);
    syncAction->setWhatsThis(
        i18nc("@info:whatsthis",
              "Choose this option to synchronize your groupware to-do list."));
    connect(syncAction, &QAction::triggered, this, &TodoPlugin::slotSyncTodos);
    insertSyncAction(syncAction);

    mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(
        new KontactInterface::UniqueAppHandlerFactory<KOrganizerUniqueAppHandler>(), this);
}

TodoPlugin::~TodoPlugin()
{
}

KontactInterface::Summary *TodoPlugin::createSummaryWidget(QWidget *parent)
{
    return new TodoSummaryWidget(this, parent);
}

KParts::ReadOnlyPart *TodoPlugin::createPart()
{
    KParts::ReadOnlyPart *part = loadPart();

    if (!part) {
        return Q_NULLPTR;
    }

    mIface = new OrgKdeKorganizerCalendarInterface(
        QStringLiteral("org.kde.korganizer"), QStringLiteral("/Calendar"), QDBusConnection::sessionBus(), this);

    return part;
}

void TodoPlugin::select()
{
    interface()->showTodoView();
}

QStringList TodoPlugin::invisibleToolbarActions() const
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

OrgKdeKorganizerCalendarInterface *TodoPlugin::interface()
{
    if (!mIface) {
        part();
    }
    Q_ASSERT(mIface);
    return mIface;
}

void TodoPlugin::slotNewTodo()
{
    interface()->openTodoEditor(QString());
}

void TodoPlugin::slotSyncTodos()
{
#if 0
    QDBusMessage message =
        QDBusMessage::createMethodCall("org.kde.kmail", "/Groupware",
                                       "org.kde.kmail.groupware",
                                       "triggerSync");
    message << QString("Todo");
    QDBusConnection::sessionBus().send(message);
#else
    qCWarning(KORGANIZERPLUGIN_LOG) << "TodoPlugin::slotSyncTodos : need to port to Akonadi";
#endif
}

bool TodoPlugin::createDBUSInterface(const QString &serviceType)
{
    if (serviceType == QLatin1String("DBUS/Organizer") || serviceType == QLatin1String("DBUS/Calendar")) {
        if (part()) {
            return true;
        }
    }
    return false;
}

bool TodoPlugin::canDecodeMimeData(const QMimeData *mimeData) const
{
    return
        mimeData->hasText() ||
        KPIM::MailList::canDecode(mimeData) ||
        KContacts::VCardDrag::canDecode(mimeData) ||
        KCalUtils::ICalDrag::canDecode(mimeData);
}

bool TodoPlugin::isRunningStandalone() const
{
    return mUniqueAppWatcher->isRunningStandalone();
}

void TodoPlugin::processDropEvent(QDropEvent *event)
{
    const QMimeData *md = event->mimeData();

    if (KContacts::VCardDrag::canDecode(md)) {
        KContacts::Addressee::List contacts;

        KContacts::VCardDrag::fromMimeData(md, contacts);

        KContacts::Addressee::List::ConstIterator it;

        QStringList attendees;
        KContacts::Addressee::List::ConstIterator end(contacts.constEnd());
        for (it = contacts.constBegin(); it != end; ++it) {
            const QString email = (*it).fullEmail();
            if (email.isEmpty()) {
                attendees.append((*it).realName() + QStringLiteral("<>"));
            } else {
                attendees.append(email);
            }
        }

        interface()->openTodoEditor(i18nc("@item", "Meeting"),
                                    QString(), QStringList(), attendees);
        return;
    }

    if (KCalUtils::ICalDrag::canDecode(event->mimeData())) {
        KCalCore::MemoryCalendar::Ptr cal(new KCalCore::MemoryCalendar(KSystemTimeZones::local()));
        if (KCalUtils::ICalDrag::fromMimeData(event->mimeData(), cal)) {
            KCalCore::Incidence::List incidences = cal->incidences();
            Q_ASSERT(incidences.count());
            if (!incidences.isEmpty()) {
                event->accept();
                KCalCore::Incidence::Ptr i = incidences.first();
                QString summary;
                if (i->type() == KCalCore::Incidence::TypeJournal) {
                    summary = i18nc("@item", "Note: %1", i->summary());
                } else {
                    summary = i->summary();
                }
                interface()->openTodoEditor(summary, i->description(), QStringList());
                return;
            }
            // else fall through to text decoding
        }
    }

    if (md->hasText()) {
        const QString text = md->text();
        interface()->openTodoEditor(text);
        return;
    }

    if (KPIM::MailList::canDecode(md)) {
        KPIM::MailList mails = KPIM::MailList::fromMimeData(md);
        event->accept();
        if (mails.count() != 1) {
            KMessageBox::sorry(
                core(),
                i18nc("@info", "Dropping multiple mails is not supported."));
        } else {
            KPIM::MailSummary mail = mails.first();
            QString txt = i18nc("@item", "From: %1\nTo: %2\nSubject: %3",
                                mail.from(), mail.to(), mail.subject());
            QString uri = QStringLiteral("kmail:") +
                          QString::number(mail.serialNumber()) + QLatin1Char('/') +
                          mail.messageId();
            QTemporaryFile tf;
            tf.setAutoRemove(true);

            tf.write(event->mimeData()->data(QStringLiteral("message/rfc822")));
            interface()->openTodoEditor(
                i18nc("@item", "Mail: %1", mail.subject()),
                txt, uri, tf.fileName(), QStringList(), QStringLiteral("message/rfc822"));
            tf.close();
        }
        return;
    }
    qCWarning(KORGANIZERPLUGIN_LOG) << QStringLiteral("Cannot handle drop events of type '%1'.").arg(event->mimeData()->formats().join(QLatin1Char(';')));
}
#include "todoplugin.moc"
