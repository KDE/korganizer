/*
  This file is part of Kontact.

  SPDX-FileCopyrightText: 2004 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2005-2006, 2008-2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kcmtodosummary.h"

#include <KAcceleratorManager>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(KCMTodoSummary, "kcmtodosummary.json")

KCMTodoSummary::KCMTodoSummary(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    setupUi(widget());

    customDaysChanged(7);

    connect(mDateTodayButton, &QRadioButton::clicked, this, &KCMTodoSummary::modified);
    connect(mDateMonthButton, &QRadioButton::clicked, this, &KCMTodoSummary::modified);
    connect(mDateRangeButton, &QRadioButton::clicked, this, &KCMTodoSummary::modified);

    connect(mHideCompletedBox, &QCheckBox::stateChanged, this, &KCMTodoSummary::modified);
    connect(mHideOpenEndedBox, &QCheckBox::stateChanged, this, &KCMTodoSummary::modified);
    connect(mHideUnstartedBox, &QCheckBox::stateChanged, this, &KCMTodoSummary::modified);
    connect(mHideInProgressBox, &QCheckBox::stateChanged, this, &KCMTodoSummary::modified);
    connect(mHideOverdueBox, &QCheckBox::stateChanged, this, &KCMTodoSummary::modified);

    connect(mCustomDays, &QSpinBox::valueChanged, this, &KCMTodoSummary::modified);
    connect(mCustomDays, &QSpinBox::valueChanged, this, &KCMTodoSummary::customDaysChanged);

    connect(mShowMineOnly, &QCheckBox::stateChanged, this, &KCMTodoSummary::modified);

    KAcceleratorManager::manage(widget());

    load();
}

KCMTodoSummary::~KCMTodoSummary() = default;

void KCMTodoSummary::modified()
{
    markAsChanged();
}

void KCMTodoSummary::customDaysChanged(int value)
{
    mCustomDays->setSuffix(i18np(" day", " days", value));
}

void KCMTodoSummary::load()
{
    // Note: match default entry values with those in defaults() and
    // TodoSummaryWidget::updateView().
    KConfig config(QStringLiteral("kcmtodosummaryrc"));
    KConfigGroup group = config.group(QStringLiteral("Days"));

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

    group = config.group(QStringLiteral("Hide"));
    mHideInProgressBox->setChecked(group.readEntry("InProgress", false));
    mHideOverdueBox->setChecked(group.readEntry("Overdue", false));
    mHideCompletedBox->setChecked(group.readEntry("Completed", true));
    mHideOpenEndedBox->setChecked(group.readEntry("OpenEnded", true));
    mHideUnstartedBox->setChecked(group.readEntry("NotStarted", false));

    group = config.group(QStringLiteral("Groupware"));
    mShowMineOnly->setChecked(group.readEntry("ShowMineOnly", false));

    setNeedsSave(false);
}

void KCMTodoSummary::save()
{
    KConfig config(QStringLiteral("kcmtodosummaryrc"));
    KConfigGroup group = config.group(QStringLiteral("Days"));

    int days;
    if (mDateTodayButton->isChecked()) {
        days = 1;
    } else if (mDateMonthButton->isChecked()) {
        days = 31;
    } else {
        days = mCustomDays->value();
    }
    group.writeEntry("DaysToShow", days);

    group = config.group(QStringLiteral("Hide"));
    group.writeEntry("InProgress", mHideInProgressBox->isChecked());
    group.writeEntry("Overdue", mHideOverdueBox->isChecked());
    group.writeEntry("Completed", mHideCompletedBox->isChecked());
    group.writeEntry("OpenEnded", mHideOpenEndedBox->isChecked());
    group.writeEntry("NotStarted", mHideUnstartedBox->isChecked());

    group = config.group(QStringLiteral("Groupware"));
    group.writeEntry("ShowMineOnly", mShowMineOnly->isChecked());

    config.sync();
    setNeedsSave(false);
}

void KCMTodoSummary::defaults()
{
    mDateRangeButton->setChecked(true);
    mCustomDays->setValue(7);
    mCustomDays->setEnabled(true);

    mHideInProgressBox->setChecked(false);
    mHideOverdueBox->setChecked(false);
    mHideCompletedBox->setChecked(true);
    mHideOpenEndedBox->setChecked(true);
    mHideUnstartedBox->setChecked(false);

    mShowMineOnly->setChecked(false);

    markAsChanged();
}

#include "kcmtodosummary.moc"

#include "moc_kcmtodosummary.cpp"
