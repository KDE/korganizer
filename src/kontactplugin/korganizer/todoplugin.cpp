/*
  This file is part of Kontact.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "todoplugin.h"
#include "calendarinterface.h"
#include "korg_uniqueapp.h"
#include "todosummarywidget.h"

#include <KContacts/VCardDrag>

#include <KCalendarCore/MemoryCalendar>

#include <KCalUtils/ICalDrag>

#include <KontactInterface/Core>

#include <QAction>
#include <KActionCollection>
#include "korganizerplugin_debug.h"
#include <QIcon>
#include <KLocalizedString>
#include <KMessageBox>
#include <QTemporaryFile>

#include <QDropEvent>

EXPORT_KONTACT_PLUGIN_WITH_JSON(TodoPlugin, "todoplugin.json")

TodoPlugin::TodoPlugin(KontactInterface::Core *core, const QVariantList &)
    : KontactInterface::Plugin(core, core, "korganizer", "todo")
    , mIface(nullptr)
{
    setComponentName(QStringLiteral("korganizer"), i18n("KOrganizer"));

    QAction *action
        = new QAction(QIcon::fromTheme(QStringLiteral("task-new")),
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

KParts::Part *TodoPlugin::createPart()
{
    KParts::Part *part = loadPart();

    if (!part) {
        return nullptr;
    }

    mIface = new OrgKdeKorganizerCalendarInterface(
        QStringLiteral("org.kde.korganizer"), QStringLiteral(
            "/Calendar"), QDBusConnection::sessionBus(), this);

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

bool TodoPlugin::canDecodeMimeData(const QMimeData *mimeData) const
{
    return
        mimeData->hasText()
        || KContacts::VCardDrag::canDecode(mimeData)
        || KCalUtils::ICalDrag::canDecode(mimeData);
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
        KCalendarCore::MemoryCalendar::Ptr cal(new KCalendarCore::MemoryCalendar(QTimeZone::systemTimeZone()));
        if (KCalUtils::ICalDrag::fromMimeData(event->mimeData(), cal)) {
            KCalendarCore::Incidence::List incidences = cal->incidences();
            Q_ASSERT(incidences.count());
            if (!incidences.isEmpty()) {
                event->accept();
                KCalendarCore::Incidence::Ptr i = incidences.first();
                QString summary;
                if (i->type() == KCalendarCore::Incidence::TypeJournal) {
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

    qCWarning(KORGANIZERPLUGIN_LOG)
        << QStringLiteral("Cannot handle drop events of type '%1'.").arg(
        event->mimeData()->formats().join(QLatin1Char(';')));
}

#include "todoplugin.moc"
