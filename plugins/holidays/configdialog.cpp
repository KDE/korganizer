// $Id$
// Dialog for selecting and configuring the Holiday plugin
// (c) 2001 Cornelius Schumacher

#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kapp.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstddirs.h>
#include <ksimpleconfig.h>

#include "configdialog.h"
#include "configdialog.moc"

ConfigDialog::ConfigDialog(QWidget *parent)
  : KDialogBase(Plain,i18n("Configure Holidays"),Ok,Ok,parent)
{
  QFrame *topFrame = plainPage();
  QVBoxLayout *topLayout = new QVBoxLayout(topFrame,0,spacingHint());

  QLabel *label = new QLabel(i18n("Please select a holiday set."),topFrame);
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
  kapp->config()->setGroup("Calendar/Holiday Plugin");
  QString currentHoliday = kapp->config()->readEntry("Holidays");
  QString currentHolidayName;

  QStringList holidayList;
  QStringList countryList = KGlobal::dirs()->findAllResources("data",
      "korganizer/holiday_*", false, true);
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

  kapp->config()->setGroup("Calendar/Holiday Plugin");
  kapp->config()->writeEntry("Holidays",currentHoliday);
  kapp->config()->sync();
}

void ConfigDialog::slotOk()
{
  save();

  accept();
}
