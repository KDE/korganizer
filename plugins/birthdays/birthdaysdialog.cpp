/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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


#include <qframe.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <krestrictedline.h>

#include <klocale.h>

#include "birthdaysdialog.h"
#include "birthdaysdialog.moc"

BirthdaysDialog::BirthdaysDialog(QWidget *parent) :
  KDialogBase(Plain,i18n("Import Birthdays From KAddressBook"),User1|Cancel,
              User1,parent,"bimport",true,true,i18n("Import"))
{
  setMaximumSize(350,200);
  QFrame *topFrame = plainPage();
  QGridLayout *topLayout = new QGridLayout(topFrame,2,1);
  mAlarm = new QCheckBox(i18n("Set alarm"),topFrame);
  topLayout->addWidget(mAlarm,0,0);

  QBoxLayout *alarmLayout = new QHBoxLayout(topLayout);
  topLayout->addLayout(alarmLayout,1,0);

  mALabel = new QLabel(i18n("Alarm before (in days):"),topFrame);
  alarmLayout->addWidget(mALabel);
  mAlarmTimeEdit = new KRestrictedLine(topFrame, "alarmTimeEdit","1234567890");
  mAlarmTimeEdit->setText("0");
  alarmLayout->addWidget(mAlarmTimeEdit);

  mAlarmTimeEdit->setDisabled(true);
  mALabel->setDisabled(true);

  connect(mAlarm,SIGNAL(clicked()),SLOT(alarmClicked()));
}

BirthdaysDialog::~BirthdaysDialog()
{
}

void BirthdaysDialog::slotUser1()
{
 accept();
}

void BirthdaysDialog::alarmClicked()
{
  mAlarmTimeEdit->setDisabled(!mAlarm->isChecked());
  mALabel->setDisabled(!mAlarm->isChecked());
}


