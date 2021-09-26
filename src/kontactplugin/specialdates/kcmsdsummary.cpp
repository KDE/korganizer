/*
  This file is part of Kontact.

  SPDX-FileCopyrightText: 2004 Tobias Koenig <tokoe@kde.org>
  SPDX-FileCopyrightText: 2004-2006, 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "kcmsdsummary.h"

#include <KAboutData>
#include <KAcceleratorManager>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(KCMSDSummary, "kcmsdsummary.json")

KCMSDSummary::KCMSDSummary(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    setupUi(this);

    customDaysChanged(7);

    connect(mDateTodayButton, &QRadioButton::clicked, this, &KCMSDSummary::modified);
    connect(mDateMonthButton, &QRadioButton::clicked, this, &KCMSDSummary::modified);
    connect(mDateRangeButton, &QRadioButton::clicked, this, &KCMSDSummary::modified);

    connect(mCustomDays, &QSpinBox::valueChanged, this, &KCMSDSummary::modified);
    connect(mCustomDays, &QSpinBox::valueChanged, this, &KCMSDSummary::customDaysChanged);

    connect(mShowBirthdaysFromCalBox, &QCheckBox::stateChanged, this, &KCMSDSummary::modified);
    connect(mShowAnniversariesFromCalBox, &QCheckBox::stateChanged, this, &KCMSDSummary::modified);
    connect(mShowHolidaysFromCalBox, &QCheckBox::stateChanged, this, &KCMSDSummary::modified);
    connect(mShowSpecialsFromCalBox, &QCheckBox::stateChanged, this, &KCMSDSummary::modified);

    connect(mShowBirthdaysFromKABBox, &QCheckBox::stateChanged, this, &KCMSDSummary::modified);
    connect(mShowAnniversariesFromKABBox, &QCheckBox::stateChanged, this, &KCMSDSummary::modified);

    connect(mShowMineOnly, &QCheckBox::stateChanged, this, &KCMSDSummary::modified);

    KAcceleratorManager::manage(this);

    load();
}

void KCMSDSummary::modified()
{
    Q_EMIT changed(true);
}

void KCMSDSummary::buttonClicked(int id)
{
    mCustomDays->setEnabled(id == 2);
}

void KCMSDSummary::customDaysChanged(int value)
{
    mCustomDays->setSuffix(i18np(" day", " days", value));
}

void KCMSDSummary::load()
{
    KConfig config(QStringLiteral("kcmsdsummaryrc"));

    KConfigGroup group = config.group("Days");
    const int days = group.readEntry("DaysToShow", 7);
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

    mShowBirthdaysFromKABBox->setChecked(group.readEntry("BirthdaysFromContacts", true));
    mShowBirthdaysFromCalBox->setChecked(group.readEntry("BirthdaysFromCalendar", true));

    mShowAnniversariesFromKABBox->setChecked(group.readEntry("AnniversariesFromContacts", true));
    mShowAnniversariesFromCalBox->setChecked(group.readEntry("AnniversariesFromCalendar", true));

    mShowHolidaysFromCalBox->setChecked(group.readEntry("HolidaysFromCalendar", true));

    mShowSpecialsFromCalBox->setChecked(group.readEntry("SpecialsFromCalendar", true));

    group = config.group("Groupware");
    mShowMineOnly->setChecked(group.readEntry("ShowMineOnly", false));

    Q_EMIT changed(false);
}

void KCMSDSummary::save()
{
    KConfig config(QStringLiteral("kcmsdsummaryrc"));

    KConfigGroup group = config.group("Days");

    int days;
    if (mDateTodayButton->isChecked()) {
        days = 1;
    } else if (mDateMonthButton->isChecked()) {
        days = 31;
    } else {
        days = mCustomDays->value();
    }
    group.writeEntry("DaysToShow", days);

    group = config.group("Show");

    group.writeEntry("BirthdaysFromContacts", mShowBirthdaysFromKABBox->isChecked());
    group.writeEntry("BirthdaysFromCalendar", mShowBirthdaysFromCalBox->isChecked());

    group.writeEntry("AnniversariesFromContacts", mShowAnniversariesFromKABBox->isChecked());
    group.writeEntry("AnniversariesFromCalendar", mShowAnniversariesFromCalBox->isChecked());

    group.writeEntry("HolidaysFromCalendar", mShowHolidaysFromCalBox->isChecked());

    group.writeEntry("SpecialsFromCalendar", mShowSpecialsFromCalBox->isChecked());

    group = config.group("Groupware");
    group.writeEntry("ShowMineOnly", mShowMineOnly->isChecked());

    group.sync();
    Q_EMIT changed(false);
}

void KCMSDSummary::defaults()
{
    mDateRangeButton->setChecked(true);
    mCustomDays->setValue(7);
    mCustomDays->setEnabled(true);

    mShowBirthdaysFromKABBox->setChecked(true);
    mShowBirthdaysFromCalBox->setChecked(true);
    mShowAnniversariesFromKABBox->setChecked(true);
    mShowAnniversariesFromCalBox->setChecked(true);
    mShowHolidaysFromCalBox->setChecked(true);
    mShowSpecialsFromCalBox->setChecked(true);

    mShowMineOnly->setChecked(false);

    Q_EMIT changed(true);
}

const KAboutData *KCMSDSummary::aboutData() const
{
    auto about = new KAboutData(QStringLiteral("kcmsdsummary"),
                                i18n("Upcoming Special Dates Configuration Dialog"),
                                QString(),
                                QString(),
                                KAboutLicense::GPL,
                                i18n("Copyright © 2004 Tobias Koenig\n"
                                     "Copyright © 2004–2010 Allen Winter"));

    about->addAuthor(i18n("Tobias Koenig"), QString(), QStringLiteral("tokoe@kde.org"));
    about->addAuthor(i18n("Allen Winter"), QString(), QStringLiteral("winter@kde.org"));

    return about;
}

#include "kcmsdsummary.moc"
