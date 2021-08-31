/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koprefsdialoggroupscheduling.h"
#include "prefs/koprefs.h"
#include <CalendarSupport/KCalPrefs>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <TransportManagementWidget>
#include <akonadi/calendar/calendarsettings.h> //krazy:exclude=camelcase this is a generated file

K_PLUGIN_CLASS_WITH_JSON(KOPrefsDialogGroupScheduling, "korganizer_configgroupscheduling.json")

KOPrefsDialogGroupScheduling::KOPrefsDialogGroupScheduling(QWidget *parent, const QVariantList &args)
    : Korganizer::KPrefsModule(KOPrefs::instance(), parent, args)
{
    auto topTopLayout = new QVBoxLayout(this);

    auto topFrame = new QWidget(this);
    topTopLayout->addWidget(topFrame);

    auto topLayout = new QGridLayout(topFrame);
    topLayout->setContentsMargins({});

    Korganizer::KPrefsWidBool *useGroupwareBool = addWidBool(CalendarSupport::KCalPrefs::instance()->useGroupwareCommunicationItem(), topFrame);
    topLayout->addWidget(useGroupwareBool->checkBox(), 0, 0, 1, 2);

    Korganizer::KPrefsWidBool *bcc = addWidBool(Akonadi::CalendarSettings::self()->bccItem(), topFrame);
    topLayout->addWidget(bcc->checkBox(), 1, 0, 1, 2);

    auto aTransportLabel = new QLabel(i18nc("@label", "Mail transport:"), topFrame);
    topLayout->addWidget(aTransportLabel, 2, 0, 1, 2);

    auto tmw = new MailTransport::TransportManagementWidget(topFrame);
    tmw->layout()->setContentsMargins({});
    topLayout->addWidget(tmw, 3, 0, 1, 2);
    load();
}

void KOPrefsDialogGroupScheduling::usrReadConfig()
{
}

void KOPrefsDialogGroupScheduling::usrWriteConfig()
{
}

#include "koprefsdialoggroupscheduling.moc"
