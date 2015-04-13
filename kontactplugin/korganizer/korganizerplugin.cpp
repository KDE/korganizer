/*
  This file is part of Kontact.

  Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>

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

#include "korganizerplugin.h"
#include "apptsummarywidget.h"
#include "calendarinterface.h"
#include "korg_uniqueapp.h"

#include <libkdepim/misc/maillistdrag.h>

#include <KContacts/VCardDrag>

#include <KCalCore/Incidence>
#include <KCalCore/MemoryCalendar>

#include <KCalUtils/ICalDrag>

#include <KontactInterface/Core>

#include <QAction>
#include <KActionCollection>
#include <QDebug>
#include <QIcon>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSystemTimeZone>
#include <QTemporaryFile>

#include <QDropEvent>
#include <QStandardPaths>

EXPORT_KONTACT_PLUGIN(KOrganizerPlugin, korganizer)

KOrganizerPlugin::KOrganizerPlugin(KontactInterface::Core *core, const QVariantList &)
    : KontactInterface::Plugin(core, core, "korganizer", "calendar"), mIface(Q_NULLPTR)
{
#pragma message("port QT5")
    //QT5 setComponentData( KontactPluginFactory::componentData() );
    KIconLoader::global()->addAppDir(QStringLiteral("korganizer"));
    KIconLoader::global()->addAppDir(QStringLiteral("kdepim"));

    QAction *action  =
        new QAction(QIcon::fromTheme(QStringLiteral("appointment-new")),
                    i18nc("@action:inmenu", "New Event..."), this);
    actionCollection()->addAction(QStringLiteral("new_event"), action);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_E));
    QString str = i18nc("@info:status", "Create a new event");
    action->setStatusTip(str);
    action->setToolTip(str);

    action->setWhatsThis(
        i18nc("@info:whatsthis",
              "You will be presented with a dialog where you can create a new event item."));
    connect(action, &QAction::triggered, this, &KOrganizerPlugin::slotNewEvent);
    insertNewAction(action);

    QAction *syncAction =
        new QAction(QIcon::fromTheme(QStringLiteral("view-refresh")),
                    i18nc("@action:inmenu", "Sync Calendar"), this);
    actionCollection()->addAction(QStringLiteral("korganizer_sync"), syncAction);
    str = i18nc("@info:status", "Synchronize groupware calendar");
    syncAction->setStatusTip(str);
    syncAction->setToolTip(str);

    syncAction->setWhatsThis(
        i18nc("@info:whatsthis",
              "Choose this option to synchronize your groupware events."));
    connect(syncAction, &QAction::triggered, this, &KOrganizerPlugin::slotSyncEvents);
    insertSyncAction(syncAction);

    mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(
        new KontactInterface::UniqueAppHandlerFactory<KOrganizerUniqueAppHandler>(), this);
}

KOrganizerPlugin::~KOrganizerPlugin()
{
}

KontactInterface::Summary *KOrganizerPlugin::createSummaryWidget(QWidget *parent)
{
    return new ApptSummaryWidget(this, parent);
}

KParts::ReadOnlyPart *KOrganizerPlugin::createPart()
{
    KParts::ReadOnlyPart *part = loadPart();

    if (!part) {
        return Q_NULLPTR;
    }

    mIface = new OrgKdeKorganizerCalendarInterface(
        QStringLiteral("org.kde.korganizer"), QStringLiteral("/Calendar"), QDBusConnection::sessionBus(), this);

    return part;
}

QString KOrganizerPlugin::tipFile() const
{
    QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("korganizer/tips"));
    return file;
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
        part();
    }
    Q_ASSERT(mIface);
    return mIface;
}

void KOrganizerPlugin::slotNewEvent()
{
    interface()->openEventEditor(QString());
}

void KOrganizerPlugin::slotSyncEvents()
{
#if 0
    QDBusMessage message =
        QDBusMessage::createMethodCall("org.kde.kmail", "/Groupware",
                                       "org.kde.kmail.groupware",
                                       "triggerSync");
    message << QString("Calendar");
    QDBusConnection::sessionBus().send(message);
#else
    qWarning() << " KOrganizerPlugin::slotSyncEvents : need to port to Akonadi";
#endif
}

bool KOrganizerPlugin::createDBUSInterface(const QString &serviceType)
{
    if (serviceType == QStringLiteral("DBUS/Organizer") || serviceType == QStringLiteral("DBUS/Calendar")) {
        if (part()) {
            return true;
        }
    }
    return false;
}

bool KOrganizerPlugin::isRunningStandalone() const
{
    return mUniqueAppWatcher->isRunningStandalone();
}

bool KOrganizerPlugin::canDecodeMimeData(const QMimeData *mimeData) const
{
    return mimeData->hasText() ||
           KPIM::MailList::canDecode(mimeData) ||
           KContacts::VCardDrag::canDecode(mimeData);
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

        interface()->openEventEditor(i18nc("@item", "Meeting"),
                                     QString(), QStringList(), attendees);
        return;
    }

    if (KCalUtils::ICalDrag::canDecode(event->mimeData())) {
        KCalCore::MemoryCalendar::Ptr cal(
            new KCalCore::MemoryCalendar(KSystemTimeZones::local()));
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
                interface()->openEventEditor(summary, i->description(), QStringList());
                return;
            }
            // else fall through to text decoding
        }
    }

    if (md->hasText()) {
        const QString text = md->text();
        qDebug() << "DROP:" << text;
        interface()->openEventEditor(text);
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

            QTemporaryFile tf;
            tf.setAutoRemove(true);
            tf.open();
            QString uri = QStringLiteral("kmail:") + QString::number(mail.serialNumber());
            tf.write(event->mimeData()->data(QStringLiteral("message/rfc822")));
            interface()->openEventEditor(
                i18nc("@item", "Mail: %1", mail.subject()), txt,
                uri, tf.fileName(), QStringList(), QStringLiteral("message/rfc822"));
            tf.close();
        }
        return;
    }
    qWarning() << QStringLiteral("Cannot handle drop events of type '%1'.").arg(event->mimeData()->formats().join(QLatin1Char(';')));
}
#include "korganizerplugin.moc"
