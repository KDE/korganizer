/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2007 Loïc Corbasson <loic.corbasson@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "configdialog.h"

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
    , mAspectRatioGroup(new QButtonGroup(this))
{
    setWindowTitle(i18nc("@title:window", "Configure Picture of the Day"));
    auto mainLayout = new QVBoxLayout(this);

    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ConfigDialog::reject);
    okButton->setDefault(true);
    setModal(true);
    auto topFrame = new QFrame(this);
    mainLayout->addWidget(topFrame);
    auto topLayout = new QVBoxLayout(topFrame);
    topLayout->setContentsMargins({});

    auto aspectRatioBox = new QGroupBox(i18n("Thumbnail Aspect Ratio Mode"), topFrame);
    topLayout->addWidget(aspectRatioBox);
    auto groupLayout = new QVBoxLayout(aspectRatioBox);

    auto btn = new QRadioButton(i18n("Ignore aspect ratio"), aspectRatioBox);
    btn->setWhatsThis(
        i18n("The thumbnail will be scaled freely. "
             "The aspect ratio will not be preserved."));
    mAspectRatioGroup->addButton(btn, int(Qt::IgnoreAspectRatio));
    groupLayout->addWidget(btn);
    btn = new QRadioButton(i18n("Keep aspect ratio"), aspectRatioBox);
    btn->setWhatsThis(
        i18n("The thumbnail will be scaled to a rectangle "
             "as large as possible inside a given rectangle, "
             "preserving the aspect ratio."));
    mAspectRatioGroup->addButton(btn, int(Qt::KeepAspectRatio));
    groupLayout->addWidget(btn);
    btn = new QRadioButton(i18n("Keep aspect ratio by expanding"), aspectRatioBox);
    btn->setWhatsThis(
        i18n("The thumbnail will be scaled to a rectangle "
             "as small as possible outside a given rectangle, "
             "preserving the aspect ratio."));
    mAspectRatioGroup->addButton(btn, int(Qt::KeepAspectRatioByExpanding));
    groupLayout->addWidget(btn);

    connect(okButton, &QPushButton::clicked, this, &ConfigDialog::slotOk);
    mainLayout->addStretch(1);
    mainLayout->addWidget(buttonBox);
    load();
}

ConfigDialog::~ConfigDialog() = default;

void ConfigDialog::load()
{
    KConfig _config(QStringLiteral("korganizerrc"), KConfig::NoGlobals);
    KConfigGroup config(&_config, QStringLiteral("Calendar/Picoftheday Plugin"));
    int datenum = config.readEntry("AspectRatioMode", 0);
    QAbstractButton *btn = mAspectRatioGroup->button(datenum);
    if (!btn) {
        btn = mAspectRatioGroup->button(0);
    }
    btn->setChecked(true);
}

void ConfigDialog::save()
{
    KConfig _config(QStringLiteral("korganizerrc"), KConfig::NoGlobals);
    KConfigGroup config(&_config, QStringLiteral("Calendar/Picoftheday Plugin"));
    config.writeEntry("AspectRatioMode", mAspectRatioGroup->checkedId());
    config.sync();
}

void ConfigDialog::slotOk()
{
    save();
    accept();
}

#include "moc_configdialog.cpp"
