/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koprefsdialoggroupscheduling.h"
#include <akonadi/calendar/calendarsettings.h>  //krazy:exclude=camelcase this is a generated file
#include <CalendarSupport/KCalPrefs>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <KLocalizedString>
#include <TransportManagementWidget>

KOPrefsDialogGroupScheduling::KOPrefsDialogGroupScheduling(QWidget *parent)
    : KCModule(parent)
{
    QBoxLayout *topTopLayout = new QVBoxLayout(this);

    QWidget *topFrame = new QWidget(this);
    topTopLayout->addWidget(topFrame);

    QGridLayout *topLayout = new QGridLayout(topFrame);
    topLayout->setContentsMargins(0, 0, 0, 0);

    mUseGroupwareCommunicationCheckBox = new QCheckBox(CalendarSupport::KCalPrefs::instance()->useGroupwareCommunicationItem()->label(), this);
    topLayout->addWidget(mUseGroupwareCommunicationCheckBox, 0, 0, 1, 2);
    connect(mUseGroupwareCommunicationCheckBox, &QCheckBox::toggled,
            this, &KOPrefsDialogGroupScheduling::slotConfigChanged);

    mBccBox = new QCheckBox(Akonadi::CalendarSettings::self()->bccItem()->label(), this);
    topLayout->addWidget(mBccBox, 1, 0, 1, 2);
    connect(mBccBox, &QCheckBox::toggled,
            this, &KOPrefsDialogGroupScheduling::slotConfigChanged);

    QLabel *aTransportLabel = new QLabel(
        i18nc("@label", "Mail transport:"), topFrame);
    topLayout->addWidget(aTransportLabel, 2, 0, 1, 2);

    MailTransport::TransportManagementWidget *tmw
        = new MailTransport::TransportManagementWidget(topFrame);
    tmw->layout()->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(tmw, 3, 0, 1, 2);

    load();
}

void KOPrefsDialogGroupScheduling::slotConfigChanged()
{
    Q_EMIT markAsChanged();
}

void KOPrefsDialogGroupScheduling::save()
{
    CalendarSupport::KCalPrefs::instance()->setUseGroupwareCommunication(mUseGroupwareCommunicationCheckBox->isChecked());
    Akonadi::CalendarSettings::self()->setBcc(mBccBox->isChecked());
}

void KOPrefsDialogGroupScheduling::load()
{
    mUseGroupwareCommunicationCheckBox->setChecked(CalendarSupport::KCalPrefs::instance()->useGroupwareCommunication());
    mBccBox->setChecked(Akonadi::CalendarSettings::self()->bcc());
}

extern "C"
{
Q_DECL_EXPORT KCModule *create_korganizerconfiggroupscheduling(QWidget *parent, const char *)
{
    return new KOPrefsDialogGroupScheduling(parent);
}
}
