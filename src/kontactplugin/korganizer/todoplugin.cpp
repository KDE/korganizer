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

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <KCalUtils/ICalDrag>
#include <KMime/Message>

#include <KontactInterface/Core>

#include "korganizerplugin_debug.h"
#include <KActionCollection>
#include <KLocalizedString>
#include <QAction>
#include <QIcon>

#include <QDropEvent>

EXPORT_KONTACT_PLUGIN_WITH_JSON(TodoPlugin, "todoplugin.json")

TodoPlugin::TodoPlugin(KontactInterface::Core *core, const KPluginMetaData &data, const QVariantList &)
    : KontactInterface::Plugin(core, core, data, "korganizer", "todo")
{
    setComponentName(QStringLiteral("korganizer"), i18nc("@info/plain", "KOrganizer"));

    auto action = new QAction(QIcon::fromTheme(QStringLiteral("task-new")), i18nc("@action:inmenu", "New To-do…"), this);
    actionCollection()->addAction(QStringLiteral("new_todo"), action);
    actionCollection()->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T));
    QString const str = i18nc("@info:status", "Create a new to-do");
    action->setStatusTip(str);
    action->setToolTip(str);

    action->setWhatsThis(i18nc("@info:whatsthis", "You will be presented with a dialog where you can create a new to-do item."));
    connect(action, &QAction::triggered, this, &TodoPlugin::slotNewTodo);
    insertNewAction(action);
    mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(new KontactInterface::UniqueAppHandlerFactory<KOrganizerUniqueAppHandler>(), this);
}

TodoPlugin::~TodoPlugin() = default;

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

    mIface = new OrgKdeKorganizerCalendarInterface(QStringLiteral("org.kde.korganizer"), QStringLiteral("/Calendar"), QDBusConnection::sessionBus(), this);

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
        (void)part();
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
    return mimeData->hasText() || KContacts::VCardDrag::canDecode(mimeData) || KCalUtils::ICalDrag::canDecode(mimeData);
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
        KContacts::Addressee::List::ConstIterator const end(contacts.constEnd());
        for (it = contacts.constBegin(); it != end; ++it) {
            const QString email = (*it).fullEmail();
            if (email.isEmpty()) {
                attendees.append((*it).realName() + QStringLiteral("<>"));
            } else {
                attendees.append(email);
            }
        }

        interface()->openTodoEditor(i18nc("@item", "Meeting"), QString(), QStringList(), attendees);
        return;
    }

    if (KCalUtils::ICalDrag::canDecode(event->mimeData())) {
        KCalendarCore::MemoryCalendar::Ptr const cal(new KCalendarCore::MemoryCalendar(QTimeZone::systemTimeZone()));
        if (KCalUtils::ICalDrag::fromMimeData(event->mimeData(), cal)) {
            KCalendarCore::Incidence::List incidences = cal->incidences();
            Q_ASSERT(incidences.count());
            if (!incidences.isEmpty()) {
                event->accept();
                KCalendarCore::Incidence::Ptr const &i = incidences.first();
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

    if (md->hasUrls()) {
        const auto urls = md->urls();
        for (const auto &url : urls) {
            if (url.scheme() == QLatin1StringView("akonadi") && url.hasQuery()) {
                const QUrlQuery query(url.query());
                if (!query.queryItemValue(QStringLiteral("item")).isEmpty()
                    && query.queryItemValue(QStringLiteral("type")) == QLatin1StringView("message/rfc822")) {
                    auto job = new Akonadi::ItemFetchJob(Akonadi::Item(static_cast<qint64>(query.queryItemValue(QStringLiteral("item")).toLongLong())));
                    job->fetchScope().fetchAllAttributes();
                    job->fetchScope().fetchFullPayload(true);
                    connect(job, &KJob::result, this, [this, url](KJob *job) {
                        if (job->error()) {
                            return;
                        }
                        auto fetchJob = qobject_cast<Akonadi::ItemFetchJob *>(job);
                        const Akonadi::Item::List items = fetchJob->items();
                        for (const Akonadi::Item &item : items) {
                            if (item.mimeType() == QLatin1StringView("message/rfc822")) {
                                auto mail = item.payload<KMime::Message::Ptr>();
                                interface()->openTodoEditor(
                                    i18nc("@info/plain to-do summary from email subjuect", "Mail: %1", mail->subject()->asUnicodeString()),
                                    xi18nc("@info to-do description from email content",
                                           "<emphasis>From:</emphasis> %1<nl/>"
                                           "<emphasis>To:</emphasis> %2<nl/>"
                                           "<emphasis>Subject:</emphasis> %3",
                                           mail->from()->displayString(),
                                           mail->to()->displayString(),
                                           mail->subject()->asUnicodeString()),
                                    url.toDisplayString(),
                                    QString(),
                                    QStringList(),
                                    QStringLiteral("message/rfc822"));
                            }
                        }
                    });
                }
                return;
            }
        }
    }

    if (md->hasText()) {
        const QString text = md->text();
        interface()->openTodoEditor(text);
        return;
    }

    qCWarning(KORGANIZERPLUGIN_LOG) << QStringLiteral("Cannot handle drop events of type '%1'.").arg(event->mimeData()->formats().join(u';'));
}

#include "todoplugin.moc"

#include "moc_todoplugin.cpp"
