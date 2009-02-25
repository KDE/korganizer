/*
  Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include "agendaview.h"
#include <kcal/calendarresources.h>

using namespace KOrg;

AgendaView::AgendaView( Calendar *cal, QWidget *parent )
  : KOEventView( cal, parent )
{
  KCal::CalendarResources *calres = dynamic_cast<KCal::CalendarResources *>( cal );
  if ( calres ) {
    connect( calres, SIGNAL(signalResourceAdded(ResourceCalendar *)),
             SLOT(setUpdateNeeded()) );
    connect( calres, SIGNAL(signalResourceModified( ResourceCalendar *)),
             SLOT(setUpdateNeeded()) );
    connect( calres, SIGNAL(signalResourceDeleted(ResourceCalendar *)),
             SLOT(setUpdateNeeded()) );
  }
}

#include "agendaview.moc"
