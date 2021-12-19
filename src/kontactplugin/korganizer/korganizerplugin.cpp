/*
  This file is part of Kontact.

  SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "korganizerplugin.h"
#include "apptsummarywidget.h"
#include "calendarinterface.h"
#include "korg_uniqueapp.h"

#include <KContacts/VCardDrag>

#include <KCalendarCore/Incidence>
#include <KCalendarCore/MemoryCalendar>

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <KCalUtils/ICalDrag>
#include <KMime/Message>

#include <KontactInterface/Core>

#include "korganizerplugin_debug.h"
#include <KActionCollection>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QAction>
#include <QIcon>

#include <QDropEvent>
#include <QStandardPaths>

EXPORT_KONTACT_PLUGIN_WITH_JSON(KOrganizerPlugin, "korganizerplugin.json")

KOrganizerPlugin::KOrganizerPlugin(KontactInterface::Core *core, const QVariantList &)
    : KontactInterface::Plugin(core, core, "korganizer", "calendar")
{
    setComponentName(QStringLiteral("korganizer"), i18n("KOrganizer"));

    auto action = new QAction(QIcon::fromTheme(QStringLiteral("appointment-new")), i18nc("@action:inmenu", "New Event..."), this);
    actionCollection()->addAction(QStringLiteral("new_event"), action);
    actionCollection()->setDefaultShortcut(action, QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_E));
    const QString str = i18nc("@info:status", "Create a new event");
    action->setStatusTip(str);
    action->setToolTip(str);

    action->setWhatsThis(i18nc("@info:whatsthis", "You will be presented with a dialog where you can create a new event item."));
    connect(action, &QAction::triggered, this, &KOrganizerPlugin::slotNewEvent);
    insertNewAction(action);

    mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(new KontactInterface::UniqueAppHandlerFactory<KOrganizerUniqueAppHandler>(), this);

    // information for the reminder daemon
    KConfig cfg(QStringLiteral("defaultcalendarrc"));
    KConfigGroup grp(&cfg, QStringLiteral("General"));
    grp.writeEntry(QStringLiteral("ApplicationId"), QStringLiteral("org.kde.kontact"));
    grp.writeEntry(QStringLiteral("KontactPlugin"), QStringLiteral("korganizer"));
}

KOrganizerPlugin::~KOrganizerPlugin() = default;

KontactInterface::Summary *KOrganizerPlugin::createSummaryWidget(QWidget *parent)
{
    return new ApptSummaryWidget(this, parent);
}

KParts::Part *KOrganizerPlugin::createPart()
{
    KParts::Part *part = loadPart();

    if (!part) {
        return nullptr;
    }

    mIface = new OrgKdeKorganizerCalendarInterface(QStringLiteral("org.kde.korganizer"), QStringLiteral("/Calendar"), QDBusConnection::sessionBus(), this);

    return part;
}

QStringList KOrganizerPlugin::invisibleToolbarActions() const
{
    QStringList invisible;
    invisible += QStringLiteral("new_event");
    invisible += QStringLiteral("new_todo");
    invisible += QStringLiteral("new_journal");

    invisible += QStringLiteral("view_todo");
    invisible += QStringLiteral("view_journal");
    return invisible;
}

void KOrganizerPlugin::select()
{
    interface()->showEventView();
}

OrgKdeKorganizerCalendarInterface *KOrganizerPlugin::interface()
{
    if (!mIface) {
        (void) part();
    }
    Q_ASSERT(mIface);
    return mIface;
}

void KOrganizerPlugin::slotNewEvent()
{
    interface()->openEventEditor(QString());
}

bool KOrganizerPlugin::isRunningStandalone() const
{
    return mUniqueAppWatcher->isRunningStandalone();
}

bool KOrganizerPlugin::canDecodeMimeData(const QMimeData *mimeData) const
{
    return mimeData->hasText() || KContacts::VCardDrag::canDecode(mimeData);
}

void KOrganizerPlugin::processDropEvent(QDropEvent *event)
{
    const QMimeData *md = event->mimeData();
    if (KContacts::VCardDrag::canDecode(md)) {
        KContacts::Addressee::List contacts;

        KContacts::VCardDrag::fromMimeData(md, contacts);

        KContacts::Addressee::List::ConstIterator it;

        KContacts::Addressee::List::ConstIterator end(contacts.constEnd());
        QStringList attendees;
        for (it = contacts.constBegin(); it != end; ++it) {
            QString email = (*it).fullEmail();
            if (email.isEmpty()) {
                attendees.append((*it).realName() + QStringLiteral("<>"));
            } else {
                attendees.append(email);
            }
        }

        interface()->openEventEditor(i18nc("@item", "Meeting"), QString(), QStringList(), attendees);
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
                interface()->openEventEditor(summary, i->description(), QStringList());
                return;
            }
            // else fall through to text decoding
        }
    }

    if (md->hasUrls()) {
        for (const auto &url : md->urls()) {
            if (url.scheme() == QStringLiteral("akonadi") && url.hasQuery()) {
                const QUrlQuery query(url.query());
                if (!query.queryItemValue(QStringLiteral("item")).isEmpty()
                    && query.queryItemValue(QStringLiteral("type")) == QStringLiteral("message/rfc822")) {
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
                            if (item.mimeType() == QStringLiteral("message/rfc822")) {
                                auto mail = item.payload<KMime::Message::Ptr>();
                                interface()->openEventEditor(i18nc("Event from email summary", "Mail: %1", mail->subject()->asUnicodeString()),
                                                             i18nc("Event from email content",
                                                                   "<b>From:</b> %1<br /><b>To:</b> %2<br /><b>Subject:</b> %3",
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
        qCDebug(KORGANIZERPLUGIN_LOG) << "DROP:" << text;
        interface()->openEventEditor(text);
        return;
    }

    qCWarning(KORGANIZERPLUGIN_LOG) << QStringLiteral("Cannot handle drop events of type '%1'.").arg(event->mimeData()->formats().join(QLatin1Char(';')));
}

#include "korganizerplugin.moc"
