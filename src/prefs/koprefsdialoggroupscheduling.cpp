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
#include <MailTransport/TransportManagementWidgetNg>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <akonadi/calendarsettings.h> //krazy:exclude=camelcase this is a generated file

K_PLUGIN_CLASS_WITH_JSON(KOPrefsDialogGroupScheduling, "korganizer_configgroupscheduling.json")

KOPrefsDialogGroupScheduling::KOPrefsDialogGroupScheduling(QObject *parent, const KPluginMetaData &data)
    : Korganizer::KPrefsModule(KOPrefs::instance(), parent, data)
{
    auto topTopLayout = new QVBoxLayout(widget());
    auto topFrame = new QWidget(widget());
    topTopLayout->addWidget(topFrame);

    auto topLayout = new QGridLayout(topFrame);
    topLayout->setContentsMargins({});

    Korganizer::KPrefsWidBool *useGroupwareBool = addWidBool(CalendarSupport::KCalPrefs::instance()->useGroupwareCommunicationItem(), topFrame);
    topLayout->addWidget(useGroupwareBool->checkBox(), 0, 0, 1, 2);

    Korganizer::KPrefsWidBool *bcc = addWidBool(Akonadi::CalendarSettings::self()->bccItem(), topFrame);
    topLayout->addWidget(bcc->checkBox(), 1, 0, 1, 2);

    Korganizer::KPrefsWidBool *hideDeclined = addWidBool(Akonadi::CalendarSettings::self()->hideDeclinedInvitationsItem(), topFrame);
    topLayout->addWidget(hideDeclined->checkBox(), 2, 0, 1, 2);

    auto aTransportLabel = new QLabel(i18nc("@label", "Mail transport:"), topFrame);
    topLayout->addWidget(aTransportLabel, 3, 0, 1, 2);

    auto tmw = new MailTransport::TransportManagementWidgetNg(topFrame);
    tmw->layout()->setContentsMargins({});
    topLayout->addWidget(tmw, 4, 0, 1, 2);
    load();
}

void KOPrefsDialogGroupScheduling::usrReadConfig()
{
}

void KOPrefsDialogGroupScheduling::usrWriteConfig()
{
}

#include "koprefsdialoggroupscheduling.moc"

#include "moc_koprefsdialoggroupscheduling.cpp"
