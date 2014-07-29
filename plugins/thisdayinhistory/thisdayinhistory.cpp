/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2007 Loïc Corbasson <loic.corbasson@gmail.com>

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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "thisdayinhistory.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

ThisDayInHistory::ThisDayInHistory()
{
  KConfig _config( QLatin1String("korganizerrc") );
  KConfigGroup config( &_config, "This Day in History Plugin" );
}

QString ThisDayInHistory::info() const
{
  return i18n( "This plugin provides links to Wikipedia's "
               "'This Day in History' pages." );
}

Element::List ThisDayInHistory::createDayElements( const QDate &date )
{
  Element::List elements;

  StoredElement *element =
    new StoredElement( QLatin1String("Wikipedia link"), i18n( "This day in history" ) );

  element->setUrl(
    QString( i18nc( "Localized Wikipedia website", "http://en.wikipedia.org/wiki/" ) +
             date.toString( i18nc( "Qt date format used by the localized Wikipedia",
                                   "MMMM_d" ) ) ) );

  elements.append( element );

  return elements;
}

Element::List ThisDayInHistory::createMonthElements( const QDate &date )
{
  Element::List elements;

  StoredElement *element =
    new StoredElement( QLatin1String("Wikipedia link"), i18n( "This month in history" ) );

  element->setUrl(
    QString( i18nc( "Localized Wikipedia website", "http://en.wikipedia.org/wiki/" ) +
             date.toString( i18nc( "Qt date format used by the localized Wikipedia",
                                   "MMMM_yyyy" ) ) ) );

  elements.append( element );

  return elements;
}
