/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koeventviewerdialog.h"
#include "korganizer_debug.h"
#include "korganizerinterface.h"

#include <Akonadi/Calendar/ETMCalendar>
#include <CalendarSupport/IncidenceViewer>
#include <CalendarSupport/Utils>

#include <AkonadiCore/Item>

#include <KGuiItem>
#include <KLocalizedString>
#include <QDialogButtonBox>
#include <QIcon>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>

KOEventViewerDialog::KOEventViewerDialog(Akonadi::ETMCalendar *calendar, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18nc("@title:window", "Event Viewer"));
    auto mainLayout = new QVBoxLayout(this);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    mUser1Button = new QPushButton(this);
    buttonBox->addButton(mUser1Button, QDialogButtonBox::ActionRole);
    auto user2Button = new QPushButton(this);
    buttonBox->addButton(user2Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &KOEventViewerDialog::reject);
    setModal(false);
    KGuiItem::assign(mUser1Button, KGuiItem(i18n("Edit..."), QIcon::fromTheme(QStringLiteral("document-edit"))));
    KGuiItem::assign(user2Button, KGuiItem(i18n("Show in Context")));
    mEventViewer = new CalendarSupport::IncidenceViewer(calendar, this);
    mainLayout->addWidget(mEventViewer);
    mainLayout->addWidget(buttonBox);

    resize(QSize(500, 520).expandedTo(minimumSizeHint()));

    connect(this, &KOEventViewerDialog::finished, this, &KOEventViewerDialog::delayedDestruct);
    connect(mUser1Button, &QPushButton::clicked, this, &KOEventViewerDialog::editIncidence);
    connect(user2Button, &QPushButton::clicked, this, &KOEventViewerDialog::showIncidenceContext);
}

KOEventViewerDialog::~KOEventViewerDialog() = default;

void KOEventViewerDialog::delayedDestruct()
{
    if (isVisible()) {
        hide();
    }

    deleteLater();
}

QPushButton *KOEventViewerDialog::editButton() const
{
    return mUser1Button;
}

void KOEventViewerDialog::setIncidence(const Akonadi::Item &incidence, const QDate &date)
{
    mEventViewer->setIncidence(incidence, date);
}

void KOEventViewerDialog::addText(const QString &text)
{
    mEventViewer->setHeaderText(text);
}

void KOEventViewerDialog::editIncidence()
{
    const Akonadi::Item item = mEventViewer->item();

    if (CalendarSupport::hasIncidence(item)) {
        // make sure korganizer is running or the part is shown
        const QString desktopFile = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, QStringLiteral("org.kde.korganizer.desktop"));
        QString error;
        if (!QDBusConnection::sessionBus().interface()->startService(desktopFile).isValid()) {
            OrgKdeKorganizerKorganizerInterface korganizerIface(QStringLiteral("org.kde.korganizer"),
                                                                QStringLiteral("/Korganizer"),
                                                                QDBusConnection::sessionBus());
            korganizerIface.editIncidence(QString::number(item.id()));
        } else {
            qCWarning(KORGANIZER_LOG) << "Failure starting korganizer:" << error;
        }
    }
}

void KOEventViewerDialog::showIncidenceContext()
{
    const Akonadi::Item item = mEventViewer->item();

    if (CalendarSupport::hasIncidence(item)) {
        // make sure korganizer is running or the part is shown
        const QString desktopFile = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, QStringLiteral("org.kde.korganizer.desktop"));
        QString error;
        if (!QDBusConnection::sessionBus().interface()->startService(desktopFile).isValid()) {
            OrgKdeKorganizerKorganizerInterface korganizerIface(QStringLiteral("org.kde.korganizer"),
                                                                QStringLiteral("/Korganizer"),
                                                                QDBusConnection::sessionBus());
            korganizerIface.showIncidenceContext(QString::number(item.id()));
        } else {
            qCWarning(KORGANIZER_LOG) << "Failure starting korganizer:" << error;
        }
    }
}
