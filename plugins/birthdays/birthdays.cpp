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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <qfile.h>
#include <qstring.h>
#include <qdatetime.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kaction.h>
#include <kmessagebox.h>

#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>

#include <libkcal/calendar.h>
#include <libkcal/event.h>
#include <libkcal/alarm.h>

#include "calendarview.h"
#include "koprefs.h"

#include "birthdaysdialog.h"

#include "birthdays.h"
#include "birthdays.moc"


class BirthdaysFactory : public KOrg::PartFactory {
  public:
    KOrg::Part *create(KOrg::MainWindow *parent, const char *name)
    {
      return new Birthdays(parent,name);
    }
};

extern "C" {
  void *init_libkorg_birthdays()
  {
    return (new BirthdaysFactory);
  }
}


Birthdays::Birthdays(KOrg::MainWindow *parent, const char *name) :
  KOrg::Part(parent,name)
{
  setXMLFile("plugins/birthdaysui.rc");

  parent->addPluginAction( new KAction( i18n("Import Birthdays..."), 0, this,
                                        SLOT(importBirthdays()),
				        actionCollection(),
                                        "import_birthdays") );
  mParent = parent->topLevelWidget();
}

Birthdays::~Birthdays()
{
}

QString Birthdays::info()
{
  return i18n("This plugin inserts birthdays imported from the KDE addressbook for the next one year.");
}

void Birthdays::importBirthdays()
{
//  kdDebug() << "import the birthdays from the addressbook" << endl;

#ifndef KORG_NOKABC
  Calendar *cal = mainWindow()->view()->calendar();
  QDateTime birthdate;
  QString summary;
  int inserted_birthdays = 0;

  BirthdaysDialog *bd = new BirthdaysDialog();
  if (bd->exec()!=QDialog::Accepted) return;

  KABC::AddressBook *add_book = KABC::StdAddressBook::self();
  KABC::AddressBook::Iterator it;
  for ( it = add_book->begin(); it != add_book->end(); ++it ) {
    if ( (*it).birthday().date().isValid() ) {
      kdDebug() << "found a birthday " << (*it).birthday().toString() << endl;

      QString name = (*it).nickName();
      if (name.isEmpty()) name = (*it).realName();
      summary = i18n("%1's birthday").arg( name );
      birthdate = (*it).birthday();

      Event *ev = 0;
      Event *e;
      // look if not already imported
      bool insert = true;
      QPtrList<Event> events = cal->events(birthdate);
      for ( e = events.first(); e; e = events.next() ) {
        kdDebug() << summary << " | " << e->summary() << endl;
        if ( e->summary()==summary ) {
          kdDebug() << " inserted " << e->summary() << endl;
          insert = false;
          ev = e;
          e = events.last();
        }
      }
      if (!ev) ev = new Event();


      ev->setDtStart(birthdate);
      ev->setDtEnd(birthdate);
      ev->setHasEndDate(true);

      ev->setSummary(summary);

      // Set the recurrence
      Recurrence *vRecurrence = ev->recurrence();
      vRecurrence->setRecurStart(birthdate);
      vRecurrence->setYearly(Recurrence::rYearlyMonth,1,-1);
      vRecurrence->addYearlyNum(birthdate.date().month());

      ev->clearAlarms();
      if (bd->mAlarm->isChecked()) {
        // Set the alarm
        Alarm* vAlarm = ev->newAlarm();
        vAlarm->setText(summary);
        vAlarm->setTime(birthdate);
        vAlarm->setStartOffset(-1440 * bd->mAlarmTimeEdit->text().toInt());
        vAlarm->setEnabled(true);
      }

      // insert category
      QStringList::Iterator itc;
      for (itc = KOPrefs::instance()->mCustomCategories.begin();
          itc != KOPrefs::instance()->mCustomCategories.end(); ++itc ) {
        if ((*itc)==i18n("Birthday"))
          ev->setCategories(i18n("Birthday"));
      }

      if (insert) {
        cal->addEvent(ev);
        inserted_birthdays++;
        kdDebug() << "imported " << birthdate.toString() << endl;
      }
    }
  }
  summary = i18n("Imported 1 birthday.", "Imported %n birthdays.", inserted_birthdays);
  KMessageBox::information(mParent,summary);
#endif

}
