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

#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>

#include <libkcal/calendar.h>
#include <libkcal/event.h>

#include "calendarview.h"

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

  new KAction(i18n("Import birthdays"), 0, this, SLOT(importBirthdays()),
              actionCollection(), "import_birthdays");
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
  QDate now_date = QDate::currentDate();
  QDate birthd;

  KABC::AddressBook *add_book = KABC::StdAddressBook::self();
  KABC::AddressBook::Iterator it;
  for ( it = add_book->begin(); it != add_book->end(); ++it ) {
    if ( (*it).birthday().date().isValid() ) {
      kdDebug() << "found a birthday " << (*it).birthday().toString() << endl;
      Event *ev = new Event();
      birthd = (*it).birthday().date();
      birthd.setYMD(now_date.year(),birthd.month(),birthd.day());
      if ( birthd < now_date ) {
        birthd.setYMD(now_date.year()+1,birthd.month(),birthd.day());
      }
      birthdate = (*it).birthday();
      birthdate.setDate(birthd);
      ev->setDtStart(birthdate);
      ev->setDtEnd(birthdate);
      int old = birthd.year() - (*it).birthday().date().year();
      QString summary;
      summary.setNum(old);
      summary  = (*it).formattedName() + " " + summary + "s birthday";
      ev->setSummary(summary);

      bool insert = true;
      QPtrList<Event> events = cal->getAllEvents();
      Event *e;
      for ( e = events.first(); e; e = events.next() ) {
        if ( e->summary()==summary ) {
          insert = false;
          e = events.last();
        }
      }
      if (insert) {
        cal->addEvent(ev);
        kdDebug() << "imported " << birthdate.toString() << endl;
      }
    }
  }
#endif

}
