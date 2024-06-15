/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "configdialog.h"
#include "datenums.h"

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
    , mDayNumGroup(new QButtonGroup(this))
{
    setWindowTitle(i18nc("@title:window", "Configure Day Numbers"));
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

    auto dayNumBox = new QGroupBox(i18n("Show Date Number"), topFrame);
    dayNumBox->setFlat(true);
    topLayout->addWidget(dayNumBox);
    auto groupLayout = new QVBoxLayout(dayNumBox);

    auto btn = new QRadioButton(i18n("Show day number"), dayNumBox);
    mDayNumGroup->addButton(btn, int(Datenums::DayOfYear));
    groupLayout->addWidget(btn);
    btn = new QRadioButton(i18n("Show days to end of year"), dayNumBox);
    mDayNumGroup->addButton(btn, int(Datenums::DaysRemaining));
    groupLayout->addWidget(btn);
    btn = new QRadioButton(i18n("Show both"), dayNumBox);
    mDayNumGroup->addButton(btn, int(Datenums::DayOfYear | Datenums::DaysRemaining));
    groupLayout->addWidget(btn);

    connect(okButton, &QPushButton::clicked, this, &ConfigDialog::slotOk);

    load();
}

ConfigDialog::~ConfigDialog() = default;

void ConfigDialog::load()
{
    KConfig _config(QStringLiteral("korganizerrc"), KConfig::NoGlobals);
    KConfigGroup config(&_config, QStringLiteral("Calendar/Datenums Plugin"));
    int datenum = config.readEntry("ShowDayNumbers", int(Datenums::DayOfYear | Datenums::DaysRemaining));
    QAbstractButton *btn = mDayNumGroup->button(datenum);
    if (!btn) {
        btn = mDayNumGroup->button(int(Datenums::DayOfYear | Datenums::DaysRemaining));
    }
    btn->setChecked(true);
}

void ConfigDialog::save()
{
    KConfig _config(QStringLiteral("korganizerrc"), KConfig::NoGlobals);
    KConfigGroup config(&_config, QStringLiteral("Calendar/Datenums Plugin"));
    config.writeEntry("ShowDayNumbers", mDayNumGroup->checkedId());
    config.sync();
}

void ConfigDialog::slotOk()
{
    save();
    accept();
}

#include "moc_configdialog.cpp"
