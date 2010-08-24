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

#include "multiagendaview.h"

#include <calendarviews/agenda/multiagenda/multiagendaview.h>

#include <KCalCore/IncidenceBase> // DateList typedef

using namespace KOrg;

class MultiAgendaView::Private {
  public:
    Private( QWidget *parent = 0 )
    {
      mMultiAgendaView = new EventViews::MultiAgendaView( parent );
    }

    EventViews::MultiAgendaView *mMultiAgendaView;
};


MultiAgendaView::MultiAgendaView( QWidget *parent )
  : KOEventView( parent ), d( new Private( parent ) )
{
}

void MultiAgendaView::setCalendar( CalendarSupport::Calendar *cal )
{
  d->mMultiAgendaView->setCalendar( cal );
}

MultiAgendaView::~MultiAgendaView()
{
}

Akonadi::Item::List MultiAgendaView::selectedIncidences()
{
  return d->mMultiAgendaView->selectedIncidences();
}

KCalCore::DateList MultiAgendaView::selectedIncidenceDates()
{
  return d->mMultiAgendaView->selectedIncidenceDates();
}

int MultiAgendaView::currentDateCount() const
{
  return d->mMultiAgendaView->currentDateCount();
}

void MultiAgendaView::showDates( const QDate &start, const QDate &end )
{
  d->mMultiAgendaView->showDates( start, end );
}

void MultiAgendaView::showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date )
{
  d->mMultiAgendaView->showIncidences( incidenceList, date );
}

void MultiAgendaView::updateView()
{
  d->mMultiAgendaView->updateView();
}

void MultiAgendaView::changeIncidenceDisplay( const Akonadi::Item &incidence, int mode )
{
  d->mMultiAgendaView->changeIncidenceDisplay( incidence, mode );
}

int MultiAgendaView::maxDatesHint() const
{
  // TODO: remove these maxDatesHint functions, they aren't used
  return 0;
}

bool MultiAgendaView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  return d->mMultiAgendaView->eventDurationHint( startDt, endDt, allDay );
}

void MultiAgendaView::setIncidenceChanger( CalendarSupport::IncidenceChanger *changer )
{
  d->mMultiAgendaView->setIncidenceChanger( changer );
}

void MultiAgendaView::updateConfig()
{
  d->mMultiAgendaView->updateConfig();
}

void MultiAgendaView::setUpdateNeeded( bool needed )
{
  d->mMultiAgendaView->setUpdateNeeded( needed );
}

bool MultiAgendaView::hasConfigurationDialog() const
{
  return d->mMultiAgendaView->hasConfigurationDialog();
}

void MultiAgendaView::showConfigurationDialog( QWidget *parent )
{
  d->mMultiAgendaView->showConfigurationDialog( parent );
}

#include "multiagendaview.moc"
