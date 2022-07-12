/*
  This file is part of Kontact.
  SPDX-FileCopyrightText: 2004 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2005-2006, 2008-2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kcmapptsummary.h"

#include <KAcceleratorManager>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

#include <QButtonGroup>

K_PLUGIN_CLASS_WITH_JSON(KCMApptSummary, "kcmapptsummary.json")

KCMApptSummary::KCMApptSummary(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
    , mDaysButtonGroup(new QButtonGroup(this)) // krazy:exclude=tipsandthis
    , mShowButtonGroup(new QButtonGroup(this)) // krazy:exclude=tipsandthis
    , mGroupwareButtonGroup(new QButtonGroup(this)) // krazy:exclude=tipsandthis
{
    setupUi(this);

    mDaysButtonGroup->addButton(mDateTodayButton, 0);
    mDaysButtonGroup->addButton(mDateMonthButton, 1);
    mDaysButtonGroup->addButton(mDateRangeButton, 2);

    mShowButtonGroup->setExclusive(false);
    mShowButtonGroup->addButton(mShowBirthdaysFromCal);
    mShowButtonGroup->addButton(mShowAnniversariesFromCal);

    mGroupwareButtonGroup->setExclusive(false);
    mGroupwareButtonGroup->addButton(mShowMineOnly);

    customDaysChanged(7);

    // Remove QOverload<QAbstractButton *> when we switch on qt6. For the moment it avoids to add an #ifdef
    connect(mDaysButtonGroup, &QButtonGroup::buttonClicked, this, &KCMApptSummary::modified);
    connect(mShowButtonGroup, &QButtonGroup::buttonClicked, this, &KCMApptSummary::modified);
    connect(mGroupwareButtonGroup, &QButtonGroup::buttonClicked, this, &KCMApptSummary::modified);

    connect(mDaysButtonGroup, &QButtonGroup::buttonClicked, this, &KCMApptSummary::buttonClicked);

    connect(mCustomDays, &QSpinBox::valueChanged, this, &KCMApptSummary::modified);
    connect(mCustomDays, &QSpinBox::valueChanged, this, &KCMApptSummary::customDaysChanged);

    KAcceleratorManager::manage(this);

    load();
}

void KCMApptSummary::modified()
{
    Q_EMIT changed(true);
}

void KCMApptSummary::buttonClicked(QAbstractButton *button)
{
    if (button) {
        mCustomDays->setEnabled(mDaysButtonGroup->id(button) == 2);
    }
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

    Q_EMIT changed(false);
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
    Q_EMIT changed(false);
}

void KCMApptSummary::defaults()
{
    mDateRangeButton->setChecked(true);
    mCustomDays->setValue(7);
    mCustomDays->setEnabled(true);

    mShowBirthdaysFromCal->setChecked(true);
    mShowAnniversariesFromCal->setChecked(true);

    mShowMineOnly->setChecked(false);

    Q_EMIT changed(true);
}

#include "kcmapptsummary.moc"
