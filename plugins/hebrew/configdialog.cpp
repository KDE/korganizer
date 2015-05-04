/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Jonathan Singer <jsinger@leeta.net>
  Copyright (C) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

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
*/

#include "configdialog.h"

#include <KConfig>
#include <KLocalizedString>
#include <KLocale>

#include <QCheckBox>
#include <QFrame>
#include <QVBoxLayout>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    QFrame *topFrame = new QFrame(this);

    setWindowTitle(i18n("Configure Holidays"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(topFrame);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ConfigDialog::reject);
    mainLayout->addWidget(buttonBox);
    okButton->setDefault(true);
    setModal(true);
    QVBoxLayout *topLayout = new QVBoxLayout(topFrame);
    topLayout->setMargin(0);

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
    connect(okButton, &QPushButton::clicked, this, &ConfigDialog::slotOk);
    load();
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::load()
{
    KConfig config(QStringLiteral("korganizerrc"));

    KConfigGroup group(&config, "Hebrew Calendar Plugin");
    mIsraelBox->setChecked(
        group.readEntry("UseIsraelSettings",
                        (KLocale::global()->country() == QLatin1String(".il"))));
    mParshaBox->setChecked(group.readEntry("ShowParsha", true));
    mCholBox->setChecked(group.readEntry("ShowChol_HaMoed", true));
    mOmerBox->setChecked(group.readEntry("ShowOmer", true));
}

void ConfigDialog::save()
{
    KConfig config(QStringLiteral("korganizerrc"));
    KConfigGroup group(&config, "Hebrew Calendar Plugin");
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

