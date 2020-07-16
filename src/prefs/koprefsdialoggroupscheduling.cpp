/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
