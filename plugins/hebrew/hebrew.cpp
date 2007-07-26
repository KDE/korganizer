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

#include "hebrew.h"

#include <KCalendarSystem>
#include <KConfig>
#include <KGlobal>
#include <KStandardDirs>

#include "configdialog.h"
#include "converter.h"
#include "holiday.h"
#include "parsha.h"

using namespace KOrg::CalendarDecoration;

class HebrewFactory : public DecorationFactory {
  public:
    Decoration *create() { return new Hebrew; }
};

K_EXPORT_COMPONENT_FACTORY( libkorg_hebrew, HebrewFactory )


Hebrew::Hebrew()
{
  KConfig config( "korganizerrc", KConfig::NoGlobals );

  KConfigGroup group( &config, "Calendar/Hebrew Calendar Plugin" );
  areWeInIsrael = group.readEntry( "UseIsraelSettings",
                                   ( KGlobal::locale()->country()
                                          == QLatin1String(".il") ) );
  showParsha = group.readEntry( "ShowParsha", true );
  showChol = group.readEntry( "ShowChol_HaMoed", true );
  showOmer = group.readEntry( "ShowOmer", true );
}

Hebrew::~Hebrew()
{
}

void Hebrew::configure( QWidget *parent )
{
  ConfigDialog dlg( parent );
}

Element::List Hebrew::createDayElements( const QDate &date )
{
  Element::List el;

  QString text;

  HebrewDate hd = HebrewDate::fromSecular( date.year(), date.month(),
                                           date.day() );

  QStringList holidays = Holiday::findHoliday( hd, areWeInIsrael, showParsha,
                                               showChol, showOmer );

  KCalendarSystem *cal = KCalendarSystem::create("hebrew");
  text = cal->dayString( date ) + ' ' + cal->monthName( date );

  foreach( QString holiday, holidays ) {
    text += "<br/>\n" + holiday;
  }

  text = i18nc("Change the next two strings if emphasis is done differently in "
               "your language.", "<qt><p align=\"center\"><i>\n")
         + text + i18n("\n</i></p></qt>");
  el.append( new StoredElement( text ) );

  return el;
}

QString Hebrew::info()
{
  return i18n("This plugin provides the date in the Jewish calendar.");
}
