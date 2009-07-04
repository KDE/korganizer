/*
  This file is part of KOrganizer.

  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kohelper.h"
#include "koprefs.h"

#include <KCal/CalendarResources>

QColor KOHelper::getTextColor( const QColor &c )
{
  float luminance = ( c.red() * 0.299 ) + ( c.green() * 0.587 ) + ( c.blue() * 0.114 );
  return ( luminance > 128.0 ) ? QColor( 0, 0, 0 ) : QColor( 255, 255, 255 );
}

QColor KOHelper::resourceColor( KCal::Calendar *calendar,
                                KCal::Incidence *incidence )
{
  QColor resourceColor = QColor(); //Default invalid color
  //FIXME: dynamic_cast are dirty, Better We implements interface to get
  // the color from the calendar
  KCal::CalendarResources *calendarResource =
    dynamic_cast<KCal::CalendarResources*>( calendar );

  if ( calendarResource ) {
    KCal::ResourceCalendar *resourceCalendar =
      calendarResource->resource( incidence );

    if ( resourceCalendar ) {
      QString identifier = resourceCalendar->identifier();
      resourceColor = KOPrefs::instance()->resourceColor( identifier );

      if ( !resourceCalendar->subresources().isEmpty() ) {
        identifier = resourceCalendar->subresourceIdentifier( incidence );
        if ( identifier.isEmpty() ) {
          identifier = resourceCalendar->identifier();
        }
        QColor subrescolor( KOPrefs::instance()->resourceColor( identifier ) );
        if ( subrescolor.isValid() ) {
          resourceColor = subrescolor;
        }
      }
    }
  }
  return resourceColor;
}

QString KOHelper::resourceLabel( KCal::Calendar *calendar, KCal::Incidence *incidence )
{
  KCal::CalendarResources *calendarResource = dynamic_cast<KCal::CalendarResources*>( calendar );
  if ( !calendarResource || ! incidence ) {
    return QString();
  }

  KCal::ResourceCalendar *resourceCalendar = calendarResource->resource( incidence );
  if ( resourceCalendar ) {
    if ( !resourceCalendar->subresources().isEmpty() ) {
      QString subRes = resourceCalendar->subresourceIdentifier( incidence );
      if ( subRes.isEmpty() ) {
        return resourceCalendar->resourceName();
      } else {
        return resourceCalendar->labelForSubresource( subRes );
      }
    }
    return resourceCalendar->resourceName();
  }

  return QString();
}

