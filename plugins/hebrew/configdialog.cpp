/*
    This file is part of KOrganizer.
    Copyright (c) 2003 Jonathan Singer <jsinger@leeta.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "configdialog.h"
#include "configdialog.moc"
#include <klocale.h>
#include <qlayout.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>

ConfigDialog::ConfigDialog(QWidget * parent):KDialogBase(Plain, i18n("Configure Holidays"), Ok, Ok,
            parent)
{
  QFrame *topFrame = plainPage();
  QVBoxLayout *topLayout =
    new QVBoxLayout(topFrame, 0, spacingHint());

  israel_box = new QCheckBox(topFrame);
  israel_box->setText(i18n("Use Israeli holidays"));
  topLayout->addWidget(israel_box);

  parsha_box = new QCheckBox(topFrame);
  parsha_box->setText(i18n("Show weekly parsha"));
  topLayout->addWidget(parsha_box);

  omer_box = new QCheckBox(topFrame);
  omer_box->setText(i18n("Show day of Omer"));
  topLayout->addWidget(omer_box);

  chol_box = new QCheckBox(topFrame);
  chol_box->setText(i18n("Show Chol HaMoed"));
  topLayout->addWidget(chol_box);

  load();
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::load()
{
  KConfig config("korganizerrc", true, false); // Open read-only, no kdeglobals

  config.setGroup("Calendar/Hebrew Calendar Plugin");
  israel_box->setChecked(config.
                         readBoolEntry("Israel",
                                       (KGlobal::locale()->
                                        country() == ".il")));
  parsha_box->setChecked(config.readBoolEntry("Parsha", true));
  chol_box->setChecked(config.readBoolEntry("Chol_HaMoed", true));
  omer_box->setChecked(config.readBoolEntry("Omer", true));

}

void ConfigDialog::save()
{
  KConfig config("korganizerrc", false, false); // Open read-write, no kdeglobals

  config.setGroup("Calendar/Hebrew Calendar Plugin");
  config.writeEntry("Israel", israel_box->isChecked());
  config.writeEntry("Parsha", parsha_box->isChecked());
  config.writeEntry("Chol_HaMoed", chol_box->isChecked());
  config.writeEntry("Omer", omer_box->isChecked());
  config.sync();
}

void ConfigDialog::slotOk()
{
  save();

  accept();
}
