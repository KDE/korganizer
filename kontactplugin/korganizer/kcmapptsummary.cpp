/*
  This file is part of Kontact.
  Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2005-2006,2008-2009 Allen Winter <winter@kde.org>

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

#include "kcmapptsummary.h"

#include <KAboutData>
#include <KAcceleratorManager>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KConfig>

KCModule *create_apptsummary(QWidget *parent, const char *)
{
    return new KCMApptSummary(parent);
}

KCMApptSummary::KCMApptSummary(QWidget *parent)
    : KCModule(parent)
{
    setupUi(this);

    mDaysButtonGroup = new QButtonGroup(this);   //krazy:exclude=tipsandthis
    mDaysButtonGroup->addButton(mDateTodayButton, 0);
    mDaysButtonGroup->addButton(mDateMonthButton, 1);
    mDaysButtonGroup->addButton(mDateRangeButton, 2);

    mShowButtonGroup = new QButtonGroup(this);   //krazy:exclude=tipsandthis
    mShowButtonGroup->setExclusive(false);
    mShowButtonGroup->addButton(mShowBirthdaysFromCal);
    mShowButtonGroup->addButton(mShowAnniversariesFromCal);

    mGroupwareButtonGroup = new QButtonGroup(this);   //krazy:exclude=tipsandthis
    mGroupwareButtonGroup->setExclusive(false);
    mGroupwareButtonGroup->addButton(mShowMineOnly);

    customDaysChanged(7);

    connect(mDaysButtonGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &KCMApptSummary::modified);
    connect(mDaysButtonGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &KCMApptSummary::buttonClicked);
    connect(mShowButtonGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &KCMApptSummary::modified);
    connect(mGroupwareButtonGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &KCMApptSummary::modified);

    connect(mCustomDays, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &KCMApptSummary::modified);
    connect(mCustomDays, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &KCMApptSummary::customDaysChanged);

    KAcceleratorManager::manage(this);

    load();
}

void KCMApptSummary::modified()
{
    emit changed(true);
}

void KCMApptSummary::buttonClicked(int id)
{
    mCustomDays->setEnabled(id == 2);
}

void KCMApptSummary::customDaysChanged(int value)
{
    mCustomDays->setSuffix(i18np(" day", " days", value));
}

void KCMApptSummary::load()
{
    KConfig config(QStringLiteral("kcmapptsummaryrc"));
    KConfigGroup group = config.group("Days");

    int days = group.readEntry("DaysToShow", 7);
    if (days == 1) {
        mDateTodayButton->setChecked(true);
    } else if (days == 31) {
        mDateMonthButton->setChecked(true);
    } else {
        mDateRangeButton->setChecked(true);
        mCustomDays->setValue(days);
        mCustomDays->setEnabled(true);
    }

    group = config.group("Show");

    mShowBirthdaysFromCal->setChecked(group.readEntry("BirthdaysFromCalendar", true));
    mShowAnniversariesFromCal->setChecked(group.readEntry("AnniversariesFromCalendar", true));

    group = config.group("Groupware");
    mShowMineOnly->setChecked(group.readEntry("ShowMineOnly", false));

    emit changed(false);
}

void KCMApptSummary::save()
{
    KConfig config(QStringLiteral("kcmapptsummaryrc"));
    KConfigGroup group = config.group("Days");

    int days;
    switch (mDaysButtonGroup->checkedId()) {
    case 0:
        days = 1;
        break;
    case 1:
        days = 31;
        break;
    case 2:
    default:
        days = mCustomDays->value();
        break;
    }

    group.writeEntry("DaysToShow", days);

    group = config.group("Show");
    group.writeEntry("BirthdaysFromCalendar", mShowBirthdaysFromCal->isChecked());
    group.writeEntry("AnniversariesFromCalendar", mShowAnniversariesFromCal->isChecked());

    group = config.group("Groupware");
    group.writeEntry("ShowMineOnly", mShowMineOnly->isChecked());

    config.sync();
    emit changed(false);
}

void KCMApptSummary::defaults()
{
    mDateRangeButton->setChecked(true);
    mCustomDays->setValue(7);
    mCustomDays->setEnabled(true);

    mShowBirthdaysFromCal->setChecked(true);
    mShowAnniversariesFromCal->setChecked(true);

    mShowMineOnly->setChecked(false);

    emit changed(true);
}

const KAboutData *KCMApptSummary::aboutData() const
{
    KAboutData *about = new KAboutData(
        QStringLiteral("kcmapptsummary"),
        i18n("Upcoming Events Configuration Dialog"),
        QString(), QString(), KAboutLicense::GPL,
        i18n("Copyright © 2003–2004 Tobias Koenig\n"
             "Copyright © 2005–2010 Allen Winter"));

    about->addAuthor(i18n("Tobias Koenig"), QString(), QStringLiteral("tokoe@kde.org"));
    about->addAuthor(i18n("Allen Winter"), QString(), QStringLiteral("winter@kde.org"));

    return about;
}

