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

// Dialog for selecting and configuring the Holiday plugin

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <klocale.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include "holidaySettings.h"

#include "configdialog.h"
#include "configdialog.moc"

ConfigDialog::ConfigDialog(QWidget *parent)
  : KDialogBase(Plain,i18n("Configure Holidays"),Ok|Cancel,Ok,parent)
{
  QFrame *topFrame = plainPage();
  QVBoxLayout *topLayout = new QVBoxLayout(topFrame,0,spacingHint());

  QLabel *label = new QLabel(i18n("Please select a holiday set:"),topFrame);
  topLayout->addWidget(label);

  mHolidayCombo = new QComboBox(topFrame);
  topLayout->addWidget(mHolidayCombo);

  load();
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::load()
{
  HolidaySettings::self()->readConfig();
	QString currentHoliday( HolidaySettings::holidays() );

  QString currentHolidayName;

  QStringList holidayList;
  //TODO: create a new KHolidays method that returns the countryList
  QStringList countryList = KGlobal::dirs()->findAllResources("data",
      "libkholidays/holiday_*", false, true);
  QStringList::ConstIterator it;
  for ( it = countryList.begin(); it != countryList.end(); ++it ) {
    QString country = (*it).mid((*it).findRev('_') + 1);
    QString countryFile = locate("locale","l10n/" + country + "/entry.desktop");
    QString countryName;
    if (!countryFile.isEmpty()) {
      KSimpleConfig cfg(countryFile);
      cfg.setGroup("KCM Locale");
      countryName = cfg.readEntry("Name");
    }
    if (countryName.isEmpty()) countryName = country;

    mCountryMap[countryName] = country;
    holidayList << countryName;

    if (country == currentHoliday) currentHolidayName = countryName;
  }
  holidayList.sort();

  mHolidayCombo->insertStringList(holidayList);

  for(int i=0;i<mHolidayCombo->count();++i) {
    if(mHolidayCombo->text(i) == currentHolidayName) {
      mHolidayCombo->setCurrentItem(i);
      break;
    }
  }
}

void ConfigDialog::save()
{
  QString currentHoliday = mCountryMap[mHolidayCombo->currentText()];
	HolidaySettings::setHolidays( currentHoliday );
	HolidaySettings::self()->writeConfig();
}

void ConfigDialog::slotOk()
{
  save();

  accept();
}
