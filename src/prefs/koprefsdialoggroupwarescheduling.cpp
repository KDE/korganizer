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

#include "koprefsdialoggroupwarescheduling.h"
#include "ui_kogroupwareprefspage.h"
#include <akonadi/calendar/calendarsettings.h>  //krazy:exclude=camelcase this is a generated file
#include <QIcon>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <CalendarSupport/KCalPrefs>

KOPrefsDialogGroupwareScheduling::KOPrefsDialogGroupwareScheduling(QWidget *parent)
    : KCModule(parent)
{
    mGroupwarePage = new Ui::KOGroupwarePrefsPage();
    QWidget *widget = new QWidget(this);
    widget->setObjectName(QStringLiteral("KOGrouparePrefsPage"));

    mGroupwarePage->setupUi(widget);

    mGroupwarePage->groupwareTab->setTabIcon(0, QIcon::fromTheme(QStringLiteral("go-up")));
    mGroupwarePage->groupwareTab->setTabIcon(1, QIcon::fromTheme(QStringLiteral("go-down")));

    // signals and slots connections

    connect(mGroupwarePage->publishDays, qOverload<int>(&QSpinBox::valueChanged),
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->publishUrl, &QLineEdit::textChanged,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->publishUser, &QLineEdit::textChanged,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->publishPassword, &QLineEdit::textChanged,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->publishSavePassword, &QCheckBox::toggled,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->retrieveEnable, &QCheckBox::toggled,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->retrieveUser, &QLineEdit::textChanged,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->retrievePassword, &QLineEdit::textChanged,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->retrieveSavePassword, &QCheckBox::toggled,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->retrieveUrl, &QLineEdit::textChanged,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->publishDelay, qOverload<int>(&QSpinBox::valueChanged),
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->fullDomainRetrieval, &QCheckBox::toggled,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);
    connect(mGroupwarePage->publishEnable, &QCheckBox::toggled,
            this, &KOPrefsDialogGroupwareScheduling::slotConfigChanged);

    (new QVBoxLayout(this))->addWidget(widget);

    load();
}

KOPrefsDialogGroupwareScheduling::~KOPrefsDialogGroupwareScheduling()
{
    delete mGroupwarePage;
}

void KOPrefsDialogGroupwareScheduling::slotConfigChanged()
{
    Q_EMIT changed(true);
}

void KOPrefsDialogGroupwareScheduling::load()
{
    mGroupwarePage->publishEnable->setChecked(
        Akonadi::CalendarSettings::self()->freeBusyPublishAuto());
    mGroupwarePage->publishDelay->setValue(
        Akonadi::CalendarSettings::self()->freeBusyPublishDelay());
    mGroupwarePage->publishDays->setValue(
        Akonadi::CalendarSettings::self()->freeBusyPublishDays());
    mGroupwarePage->publishUrl->setText(
        Akonadi::CalendarSettings::self()->freeBusyPublishUrl());
    mGroupwarePage->publishUser->setText(
        Akonadi::CalendarSettings::self()->freeBusyPublishUser());
    mGroupwarePage->publishPassword->setText(
        Akonadi::CalendarSettings::self()->freeBusyPublishPassword());
    mGroupwarePage->publishSavePassword->setChecked(
        Akonadi::CalendarSettings::self()->freeBusyPublishSavePassword());

    mGroupwarePage->retrieveEnable->setChecked(
        Akonadi::CalendarSettings::self()->freeBusyRetrieveAuto());
    mGroupwarePage->fullDomainRetrieval->setChecked(
        Akonadi::CalendarSettings::self()->freeBusyFullDomainRetrieval());
    mGroupwarePage->retrieveUrl->setText(
        Akonadi::CalendarSettings::self()->freeBusyRetrieveUrl());
    mGroupwarePage->retrieveUser->setText(
        Akonadi::CalendarSettings::self()->freeBusyRetrieveUser());
    mGroupwarePage->retrievePassword->setText(
        Akonadi::CalendarSettings::self()->freeBusyRetrievePassword());
    mGroupwarePage->retrieveSavePassword->setChecked(
        Akonadi::CalendarSettings::self()->freeBusyRetrieveSavePassword());
}

void KOPrefsDialogGroupwareScheduling::save()
{
    Akonadi::CalendarSettings::self()->setFreeBusyPublishAuto(
        mGroupwarePage->publishEnable->isChecked());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishDelay(mGroupwarePage->publishDelay->value());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishDays(
        mGroupwarePage->publishDays->value());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishUrl(
        mGroupwarePage->publishUrl->text());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishUser(
        mGroupwarePage->publishUser->text());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishPassword(
        mGroupwarePage->publishPassword->text());
    Akonadi::CalendarSettings::self()->setFreeBusyPublishSavePassword(
        mGroupwarePage->publishSavePassword->isChecked());

    Akonadi::CalendarSettings::self()->setFreeBusyRetrieveAuto(
        mGroupwarePage->retrieveEnable->isChecked());
    Akonadi::CalendarSettings::self()->setFreeBusyFullDomainRetrieval(
        mGroupwarePage->fullDomainRetrieval->isChecked());
    Akonadi::CalendarSettings::self()->setFreeBusyRetrieveUrl(
        mGroupwarePage->retrieveUrl->text());
    Akonadi::CalendarSettings::self()->setFreeBusyRetrieveUser(
        mGroupwarePage->retrieveUser->text());
    Akonadi::CalendarSettings::self()->setFreeBusyRetrievePassword(
        mGroupwarePage->retrievePassword->text());
    Akonadi::CalendarSettings::self()->setFreeBusyRetrieveSavePassword(
        mGroupwarePage->retrieveSavePassword->isChecked());

    // clear the url cache for our user
    const QString configFile
        = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String(
              "/korganizer/freebusyurls");
    KConfig cfg(configFile);
    cfg.deleteGroup(CalendarSupport::KCalPrefs::instance()->email());

    Akonadi::CalendarSettings::self()->save();
}

extern "C"
{
Q_DECL_EXPORT KCModule *create_korganizerconfigfreebusy(QWidget *parent, const char *)
{
    return new KOPrefsDialogGroupwareScheduling(parent);
}
}
