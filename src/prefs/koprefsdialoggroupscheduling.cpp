/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koprefsdialoggroupscheduling.h"
#include "prefs/koprefs.h"
#include <akonadi/calendar/calendarsettings.h>  //krazy:exclude=camelcase this is a generated file
#include <CalendarSupport/KCalPrefs>
#include <QCheckBox>
#include <QLabel>
#include <QVBoxLayout>
#include <KLocalizedString>
#include <TransportManagementWidget>

KOPrefsDialogGroupScheduling::KOPrefsDialogGroupScheduling(QWidget *parent)
    : Korganizer::KPrefsModule(KOPrefs::instance(), parent)
{
    QBoxLayout *topTopLayout = new QVBoxLayout(this);

    QWidget *topFrame = new QWidget(this);
    topTopLayout->addWidget(topFrame);

    QGridLayout *topLayout = new QGridLayout(topFrame);
    topLayout->setContentsMargins(0, 0, 0, 0);

    Korganizer::KPrefsWidBool *useGroupwareBool
        = addWidBool(
              CalendarSupport::KCalPrefs::instance()->useGroupwareCommunicationItem(), topFrame);
    topLayout->addWidget(useGroupwareBool->checkBox(), 0, 0, 1, 2);

    Korganizer::KPrefsWidBool *bcc
        = addWidBool(Akonadi::CalendarSettings::self()->bccItem(), topFrame);
    topLayout->addWidget(bcc->checkBox(), 1, 0, 1, 2);

    QLabel *aTransportLabel = new QLabel(
        i18nc("@label", "Mail transport:"), topFrame);
    topLayout->addWidget(aTransportLabel, 2, 0, 1, 2);

    MailTransport::TransportManagementWidget *tmw
        = new MailTransport::TransportManagementWidget(topFrame);
    tmw->layout()->setContentsMargins(0, 0, 0, 0);
    topLayout->addWidget(tmw, 3, 0, 1, 2);

    //topLayout->setRowStretch( 2, 1 );

    load();
}

void KOPrefsDialogGroupScheduling::usrReadConfig()
{
}

void KOPrefsDialogGroupScheduling::usrWriteConfig()
{
}

extern "C"
{
Q_DECL_EXPORT KCModule *create_korganizerconfiggroupscheduling(QWidget *parent, const char *)
{
    return new KOPrefsDialogGroupScheduling(parent);
}
}
