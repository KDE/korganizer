/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2003 Jonathan Singer <jsinger@leeta.net>
  SPDX-FileCopyrightText: 2007 Loïc Corbasson <loic.corbasson@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "configdialog.h"

#include <KConfig>
#include <KLocalizedString>
#include <QLocale>

#include <KConfigGroup>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    QFrame *topFrame = new QFrame(this);

    setWindowTitle(i18nc("@title:window", "Configure Holidays"));
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(topFrame);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ConfigDialog::reject);
    mainLayout->addWidget(buttonBox);
    okButton->setDefault(true);
    setModal(true);
    QVBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setContentsMargins(0, 0, 0, 0);

    mIsraelBox = new QCheckBox(topFrame);
    mIsraelBox->setText(i18n("Use Israeli holidays"));
    topLayout->addWidget(mIsraelBox);

    mParshaBox = new QCheckBox(topFrame);
    mParshaBox->setText(i18n("Show weekly parsha"));
    topLayout->addWidget(mParshaBox);

    mOmerBox = new QCheckBox(topFrame);
    mOmerBox->setText(i18n("Show day of Omer"));
    topLayout->addWidget(mOmerBox);

    mCholBox = new QCheckBox(topFrame);
    mCholBox->setText(i18n("Show Chol HaMoed"));
    topLayout->addWidget(mCholBox);
    topLayout->addStretch(1);
    connect(okButton, &QPushButton::clicked, this, &ConfigDialog::slotOk);
    load();
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::load()
{
    KConfig config(QStringLiteral("korganizerrc"));

    KConfigGroup group(&config, QStringLiteral("Hebrew Calendar Plugin"));
    mIsraelBox->setChecked(group.readEntry("UseIsraelSettings", QLocale::territoryToString(QLocale().territory()) == QLatin1StringView(".il")));
    mParshaBox->setChecked(group.readEntry("ShowParsha", true));
    mCholBox->setChecked(group.readEntry("ShowChol_HaMoed", true));
    mOmerBox->setChecked(group.readEntry("ShowOmer", true));
}

void ConfigDialog::save()
{
    KConfig config(QStringLiteral("korganizerrc"));
    KConfigGroup group(&config, QStringLiteral("Hebrew Calendar Plugin"));
    group.writeEntry("UseIsraelSettings", mIsraelBox->isChecked());
    group.writeEntry("ShowParsha", mParshaBox->isChecked());
    group.writeEntry("ShowChol_HaMoed", mCholBox->isChecked());
    group.writeEntry("ShowOmer", mOmerBox->isChecked());
    group.sync();
}

void ConfigDialog::slotOk()
{
    save();
    accept();
}

#include "moc_configdialog.cpp"
