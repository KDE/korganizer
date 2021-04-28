/*
  This file is part of KOrganizer.
  This file is part of the KDE reminder agent.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2008-2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "alarmdockwindow.h"

#include "koalarmclient_debug.h"
#include <KConfigGroup>
#include <KIconEffect>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QMenu>

AlarmDockWindow::AlarmDockWindow()
    : KStatusNotifierItem(nullptr)
{
    // Read the autostart status from the config file
    KConfigGroup config(KSharedConfig::openConfig(), "General");
    const bool autostartSet = config.hasKey("Autostart");
    const bool autostart = config.readEntry("Autostart", true);
    const bool grabFocus = config.readEntry("GrabFocus", false);
    const bool alarmsEnabled = config.readEntry("Enabled", true);

    // Don't mention Daemon here since it's a technical
    // term the user doesn't care about
    mName = i18nc("@title:window", "KOrganizer Reminders");
    setToolTipTitle(mName);
    setToolTipIconByName(QStringLiteral("korgac"));
    setTitle(mName);

    // Set up icons
    const QIcon iconEnabled = QIcon::fromTheme(QStringLiteral("korgac"));

    KIconLoader loader;
    QImage iconDisabled = iconEnabled.pixmap(loader.currentSize(KIconLoader::Panel)).toImage();
    KIconEffect::toGray(iconDisabled, 1.0);
    mIconDisabled = QIcon(QPixmap::fromImage(iconDisabled));

    changeSystrayIcon(alarmsEnabled);

    // Set up the context menu
    mSuspendAll = contextMenu()->addAction(i18nc("@action:inmenu", "&Suspend All Reminders"), this, &AlarmDockWindow::slotSuspendAll);
    mDismissAll = contextMenu()->addAction(i18nc("@action:inmenu", "&Dismiss All Reminders"), this, &AlarmDockWindow::slotDismissAll);
    // leave mShow always enabled that way you can get to alarms that are
    // suspended and inactive to dismiss them before they go off again
    // (as opposed to the other two that are initially disabled)
    mShow = contextMenu()->addAction(i18nc("@action:inmenu", "Show &Reminders"), this, &AlarmDockWindow::showReminderSignal);
    mSuspendAll->setEnabled(false);
    mDismissAll->setEnabled(false);

    contextMenu()->addSeparator();
    mAlarmsEnabled = contextMenu()->addAction(i18nc("@action:inmenu", "Enable Reminders"));
    connect(mAlarmsEnabled, &QAction::toggled, this, &AlarmDockWindow::toggleAlarmsEnabled);
    mAlarmsEnabled->setCheckable(true);

    mAutostart = contextMenu()->addAction(i18nc("@action:inmenu", "Start Reminder Daemon at Login"));
    connect(mAutostart, &QAction::toggled, this, &AlarmDockWindow::toggleAutostart);
    mAutostart->setCheckable(true);

    mAlarmsEnabled->setChecked(alarmsEnabled);
    mAutostart->setChecked(autostart);

    mGrabFocus = contextMenu()->addAction(i18nc("@action:inmenu", "Reminder Requests Focus"));
    mGrabFocus->setToolTip(i18nc("@info:tooltip",
                                 "When this option is enabled the reminder dialog will "
                                 "automatically receive keyboard focus when it opens."));
    connect(mGrabFocus, &QAction::toggled, this, &AlarmDockWindow::toggleGrabFocus);
    // ToolTips aren't enabled for menus by default.
    contextMenu()->setToolTipsVisible(true);
    mGrabFocus->setCheckable(true);
    mGrabFocus->setChecked(grabFocus);

    // Disable standard quit behaviour. We have to intercept the quit even,
    // if the main window is hidden.
    QAction *act = action(QStringLiteral("quit"));
    if (act) {
        disconnect(act, SIGNAL(triggered(bool)), this, SLOT(maybeQuit()));
        connect(act, &QAction::triggered, this, &AlarmDockWindow::slotQuit);
    } else {
        qCDebug(KOALARMCLIENT_LOG) << "No Quit standard action.";
    }
    mAutostartSet = autostartSet;
}

AlarmDockWindow::~AlarmDockWindow()
{
}

void AlarmDockWindow::slotUpdate(int reminders)
{
    const bool actif = (reminders > 0);
    mSuspendAll->setEnabled(actif);
    mDismissAll->setEnabled(actif);
    if (actif) {
        setToolTip(QStringLiteral("korgac"), mName, i18ncp("@info:status", "There is 1 active reminder.", "There are %1 active reminders.", reminders));
    } else {
        setToolTip(QStringLiteral("korgac"), mName, i18nc("@info:status", "No active reminders."));
    }
}

void AlarmDockWindow::toggleAlarmsEnabled(bool checked)
{
    changeSystrayIcon(checked);

    KConfigGroup config(KSharedConfig::openConfig(), "General");
    config.writeEntry("Enabled", checked);
    config.sync();
}

void AlarmDockWindow::toggleAutostart(bool checked)
{
    qCDebug(KOALARMCLIENT_LOG);
    mAutostartSet = true;
    enableAutostart(checked);
}

void AlarmDockWindow::toggleGrabFocus(bool checked)
{
    KConfigGroup config(KSharedConfig::openConfig(), "General");
    config.writeEntry("GrabFocus", checked);
}

void AlarmDockWindow::slotSuspendAll()
{
    Q_EMIT suspendAllSignal();
}

void AlarmDockWindow::slotDismissAll()
{
    Q_EMIT dismissAllSignal();
}

void AlarmDockWindow::enableAutostart(bool enable)
{
    KConfigGroup config(KSharedConfig::openConfig(), "General");
    config.writeEntry("Autostart", enable);
    config.sync();
}

void AlarmDockWindow::activate(const QPoint &pos)
{
    Q_UNUSED(pos)
    QDBusConnection::sessionBus().interface()->startService(QStringLiteral("org.kde.korganizer"));
}

void AlarmDockWindow::slotQuit()
{
    if (mAutostartSet == true) {
        const int result = KMessageBox::warningContinueCancel(associatedWidget(),
                                                              xi18nc("@info",
                                                                     "Do you want to quit the KOrganizer reminder daemon?<nl/>"
                                                                     "<note> you will not get calendar reminders unless the daemon is running.</note>"),
                                                              i18nc("@title:window", "Close KOrganizer Reminder Daemon"),
                                                              KStandardGuiItem::quit());

        if (result == KMessageBox::Continue) {
            Q_EMIT quitSignal();
        }
    } else {
        const int result = KMessageBox::questionYesNoCancel(associatedWidget(),
                                                            xi18nc("@info",
                                                                   "Do you want to start the KOrganizer reminder daemon at login?<nl/>"
                                                                   "<note> you will not get calendar reminders unless the daemon is running.</note>"),
                                                            i18nc("@title:window", "Close KOrganizer Reminder Daemon"),
                                                            KGuiItem(i18nc("@action:button start the reminder daemon", "Start")),
                                                            KGuiItem(i18nc("@action:button do not start the reminder daemon", "Do Not Start")),
                                                            KStandardGuiItem::cancel(),
                                                            QStringLiteral("AskForStartAtLogin"));

        bool autostart = true;
        if (result == KMessageBox::No) {
            autostart = false;
        }
        enableAutostart(autostart);

        if (result != KMessageBox::Cancel) {
            Q_EMIT quitSignal();
        }
    }
}

void AlarmDockWindow::changeSystrayIcon(bool alarmsEnabled)
{
    if (alarmsEnabled) {
        setIconByName(QStringLiteral("korgac"));
    } else {
        setIconByPixmap(mIconDisabled.pixmap(22, 22));
    }
}
