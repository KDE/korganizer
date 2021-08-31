/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2000, 2001, 2002, 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "koprefsdialoggroupwarescheduling.h"
#include "ui_kogroupwareprefspage.h"
#include <CalendarSupport/KCalPrefs>
#include <KPluginFactory>
#include <QIcon>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <akonadi/calendar/calendarsettings.h> //krazy:exclude=camelcase this is a generated file

K_PLUGIN_CLASS_WITH_JSON(KOPrefsDialogGroupwareScheduling, "korganizer_configfreebusy.json")

KOPrefsDialogGroupwareScheduling::KOPrefsDialogGroupwareScheduling(QWidget *parent, const QVariantList &args)
    : KPrefsModule(CalendarSupport::KCalPrefs::instance(), parent, args)
    , mGroupwarePage(new Ui::KOGroupwarePrefsPage())
{
    auto widget = new QWidget(this);
    widget->setObjectName(QStringLiteral("KOGrouparePrefsPage"));

    mGroupwarePage->setupUi(widget);

    mGroupwarePage->groupwareTab->setTabIcon(0, QIcon::fromTheme(QStringLiteral("go-up")));
    mGroupwarePage->groupwareTab->setTabIcon(1, QIcon::fromTheme(QStringLiteral("go-down")));

    connect(mGroupwarePage->publishDays, qOverload<int>(&QSpinBox::valueChanged), this, &KOPrefsDialogGroupwareScheduling::slotWidChanged);
    connect(mGroupwarePage->publishUrl, &QLineEdit::textChanged, this, &KOPrefsDialogGroupwareScheduling::slotWidChanged);
    connect(mGroupwarePage->publishUser, &QLineEdit::textChanged, this, &KOPrefsDialogGroupwareScheduling::slotWidChanged);
    connect(mGroupwarePage->publishPassword, &QLineEdit::textChanged, this, &KOPrefsDialogGroupwareScheduling::slotWidChanged);
    connect(mGroupwarePage->publishSavePassword, &QCheckBox::toggled, this, &KOPrefsDialogGroupwareScheduling::slotWidChanged);
    connect(mGroupwarePage->retrieveEnable, &QCheckBox::toggled, this, &KOPrefsDialogGroupwareScheduling::slotWidChanged);
    connect(mGroupwarePage->retrieveUser, &QLineEdit::textChanged, this, &KOPrefsDialogGroupwareScheduling::slotWidChanged);
    connect(mGroupwarePage->retrievePassword, &QLineEdit::textChanged, this, &KOPrefsDialogGroupwareScheduling::slotWidChanged);
    connect(mGroupwarePage->retrieveSavePassword, &QCheckBox::toggled, this, &KOPrefsDialogGroupwareScheduling::slotWidChanged);
    connect(mGroupwarePage->retrieveUrl, &QLineEdit::textChanged, this, &KOPrefsDialogGroupwareScheduling::slotWidChanged);
    connect(mGroupwarePage->publishDelay, qOverload<int>(&QSpinBox::valueChanged), this, &KOPrefsDialogGroupwareScheduling::slotWidChanged);
    connect(mGroupwarePage->fullDomainRetrieval, &QCheckBox::toggled, this, &KOPrefsDialogGroupwareScheduling::slotWidChanged);
    connect(mGroupwarePage->publishEnable, &QCheckBox::toggled, this, &KOPrefsDialogGroupwareScheduling::slotWidChanged);

    (new QVBoxLayout(this))->addWidget(widget);

    load();
}

KOPrefsDialogGroupwareScheduling::~KOPrefsDialogGroupwareScheduling()
{
    delete mGroupwarePage;
}

void KOPrefsDialogGroupwareScheduling::usrReadConfig()
{
    mGroupwarePage->publishEnable->setChecked(Akonadi::CalendarSettings::self()->freeBusyPublishAuto());
    mGroupwarePage->publishDelay->setValue(Akonadi::CalendarSettings::self()->freeBusyPublishDelay());
    mGroupwarePage->publishDays->setValue(Akonadi::CalendarSettings::self()->freeBusyPublishDays());
    mGroupwarePage->publishUrl->setText(Akonadi::CalendarSettings::self()->freeBusyPublishUrl());
    mGroupwarePage->publishUser->setText(Akonadi::CalendarSettings::self()->freeBusyPublishUser());
    mGroupwarePage->publishPassword->setText(Akonadi::CalendarSettings::self()->freeBusyPublishPassword());
    mGroupwarePage->publishSavePassword->setChecked(Akonadi::CalendarSettings::self()->freeBusyPublishSavePassword());

    mGroupwarePage->retrieveEnable->setChecked(Akonadi::CalendarSettings::self()->freeBusyRetrieveAuto());
    mGroupwarePage->fullDomainRetrieval->setChecked(Akonadi::CalendarSettings::self()->freeBusyFullDomainRetrieval());
    mGroupwarePage->retrieveUrl->setText(Akonadi::CalendarSettings::self()->freeBusyRetrieveUrl());
    mGroupwarePage->retrieveUser->setText(Akonadi::CalendarSettings::self()->freeBusyRetrieveUser());
    mGroupwarePage->retrievePassword->setText(Akonadi::CalendarSettings::self()->freeBusyRetrievePassword());
    mGroupwarePage->retrieveSavePassword->setChecked(Akonadi::CalendarSettings::self()->freeBusyRetrieveSavePassword());
}

void KOPrefsDialogGroupwareScheduling::usrWriteConfig()
{
    Akonadi::CalendarSettings::self()->setFreeBusyPublishAuto(mGroupwarePage->publishEnable->isChecked());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishDelay(mGroupwarePage->publishDelay->value());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishDays(mGroupwarePage->publishDays->value());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishUrl(mGroupwarePage->publishUrl->text());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishUser(mGroupwarePage->publishUser->text());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishPassword(mGroupwarePage->publishPassword->text());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishSavePassword(mGroupwarePage->publishSavePassword->isChecked());

    Akonadi::CalendarSettings::self()->setFreeBusyRetrieveAuto(mGroupwarePage->retrieveEnable->isChecked());
    Akonadi::CalendarSettings::self()->setFreeBusyFullDomainRetrieval(mGroupwarePage->fullDomainRetrieval->isChecked());
    Akonadi::CalendarSettings::self()->setFreeBusyRetrieveUrl(mGroupwarePage->retrieveUrl->text());
    Akonadi::CalendarSettings::self()->setFreeBusyRetrieveUser(mGroupwarePage->retrieveUser->text());
    Akonadi::CalendarSettings::self()->setFreeBusyRetrievePassword(mGroupwarePage->retrievePassword->text());
    Akonadi::CalendarSettings::self()->setFreeBusyRetrieveSavePassword(mGroupwarePage->retrieveSavePassword->isChecked());

    // clear the url cache for our user
    const QString configFile = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/korganizer/freebusyurls");
    KConfig cfg(configFile);
    cfg.deleteGroup(CalendarSupport::KCalPrefs::instance()->email());

    Akonadi::CalendarSettings::self()->save();
}

#include "koprefsdialoggroupwarescheduling.moc"
