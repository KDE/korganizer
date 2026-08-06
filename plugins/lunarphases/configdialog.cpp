/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: Allen Winter <winter@kde.org>
  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "configdialog.h"
#include "lunarphases.h"

#include <KConfig>
#include <KLocalizedString>

#include <KConfigGroup>
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent)
    , mLunarPhaseGroup(new QButtonGroup(this))
{
    setWindowTitle(i18nc("@title:window", "Configure Lunar Phases"));
    auto mainLayout = new QVBoxLayout(this);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ConfigDialog::reject);
    okButton->setDefault(true);
    setModal(true);
    auto topFrame = new QFrame(this);
    mainLayout->addWidget(topFrame);
    mainLayout->addStretch(1);
    mainLayout->addWidget(buttonBox);
    auto topLayout = new QVBoxLayout(topFrame);
    topLayout->setContentsMargins({});

    auto lunarPhaseBox = new QGroupBox(i18nc("@title:group", "Choose your hemisphere"), topFrame);
    lunarPhaseBox->setFlat(true);
    topLayout->addWidget(lunarPhaseBox);
    auto groupLayout = new QVBoxLayout(lunarPhaseBox);

    auto btn = new QRadioButton(i18nc("@option:radio", "Northern Hemisphere"), lunarPhaseBox);
    btn->setToolTip(i18nc("@info:tooltip", "Show the lunar phases for the northern hemisphere"));
    btn->setWhatsThis(i18nc("@info:whatsthis", "Select this option to show the moon phases if you live in the northern hemisphere."));
    mLunarPhaseGroup->addButton(btn, int(Lunarphases::NorthernHemisphere));
    groupLayout->addWidget(btn);
    btn = new QRadioButton(i18nc("@option:radio", "Southern Hemisphere"), lunarPhaseBox);
    btn->setToolTip(i18nc("@info:tooltip", "Show the lunar phases for the southern hemisphere"));
    btn->setWhatsThis(i18nc("@info:whatsthis", "Select this option to show the moon phases if you live in the southern hemisphere."));
    mLunarPhaseGroup->addButton(btn, int(Lunarphases::SouthernHemisphere));
    groupLayout->addWidget(btn);

    connect(okButton, &QPushButton::clicked, this, &ConfigDialog::slotOk);

    load();
}

ConfigDialog::~ConfigDialog() = default;

void ConfigDialog::load()
{
    KConfig _config(QStringLiteral("korganizerrc"), KConfig::NoGlobals);
    KConfigGroup const config(&_config, QStringLiteral("Calendar/Lunar Phases Plugin"));
    int const hemisphere = config.readEntry("Hemisphere", int(Lunarphases::NorthernHemisphere));
    QAbstractButton *btn = mLunarPhaseGroup->button(hemisphere);
    if (!btn) {
        btn = mLunarPhaseGroup->button(int(Lunarphases::NorthernHemisphere));
    }
    btn->setChecked(true);
}

void ConfigDialog::save()
{
    KConfig _config(QStringLiteral("korganizerrc"), KConfig::NoGlobals);
    KConfigGroup config(&_config, QStringLiteral("Calendar/Lunar Phases Plugin"));
    config.writeEntry("Hemisphere", mLunarPhaseGroup->checkedId());
    config.sync();
}

void ConfigDialog::slotOk()
{
    save();
    accept();
}

#include "moc_configdialog.cpp"
