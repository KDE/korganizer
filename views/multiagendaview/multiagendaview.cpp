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

#include <QHBoxLayout>

using namespace KOrg;

class MultiAgendaView::Private {
  public:
    Private( QWidget *parent = 0 )
    {
      QHBoxLayout *layout = new QHBoxLayout( parent );
      mMultiAgendaView = new EventViews::MultiAgendaView( parent );
      layout->addWidget( mMultiAgendaView );
    }

    EventViews::MultiAgendaView *mMultiAgendaView;
};


MultiAgendaView::MultiAgendaView( QWidget *parent )
  : KOEventView( parent ), d( new Private( this ) )
{
  connect( d->mMultiAgendaView, SIGNAL(datesSelected(KCalCore::DateList)),
           SIGNAL(datesSelected(KCalCore::DateList)) );

  connect( d->mMultiAgendaView, SIGNAL(shiftedEvent(QDate,QDate)),
           SIGNAL(shiftedEvent(QDate,QDate)) );


  connect( d->mMultiAgendaView, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           SIGNAL(incidenceSelected(Akonadi::Item,QDate)) );

  connect( d->mMultiAgendaView, SIGNAL(showIncidenceSignal(Akonadi::Item)),
           SIGNAL(showIncidenceSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(editIncidenceSignal(Akonadi::Item)),
           SIGNAL(editIncidenceSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
           SIGNAL(deleteIncidenceSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(cutIncidenceSignal(Akonadi::Item)),
           SIGNAL(cutIncidenceSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(copyIncidenceSignal(Akonadi::Item)),
           SIGNAL(copyIncidenceSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(pasteIncidenceSignal()),
           SIGNAL(pasteIncidenceSignal()) );

  connect( d->mMultiAgendaView, SIGNAL(toggleAlarmSignal(Akonadi::Item)),
           SIGNAL(toggleAlarmSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)),
           SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,QString)),
           SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,QString)) );

  connect( d->mMultiAgendaView, SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,QString)),
           SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,QString)) );

  connect( d->mMultiAgendaView, SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)),
           SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)) );

  connect( d->mMultiAgendaView, SIGNAL(startMultiModify(QString)),
           SIGNAL(startMultiModify(QString)) );

  connect( d->mMultiAgendaView, SIGNAL(endMultiModify()),
           SIGNAL(endMultiModify()) );

  connect( d->mMultiAgendaView, SIGNAL(newEventSignal(Akonadi::Collection::List)),
           SIGNAL(newEventSignal(Akonadi::Collection::List)) );

  connect( d->mMultiAgendaView, SIGNAL(newEventSignal(Akonadi::Collection::List,QDate)),
           SIGNAL(newEventSignal(Akonadi::Collection::List,QDate)) );

  connect( d->mMultiAgendaView, SIGNAL(newEventSignal(Akonadi::Collection::List,QDateTime)),
           SIGNAL(newEventSignal(Akonadi::Collection::List,QDateTime)) );

  connect( d->mMultiAgendaView, SIGNAL(newEventSignal(Akonadi::Collection::List,QDateTime,QDateTime)),
           SIGNAL(newEventSignal(Akonadi::Collection::List,QDateTime,QDateTime)) );

  connect( d->mMultiAgendaView, SIGNAL(newTodoSignal(QDate)),
           SIGNAL(newTodoSignal(QDate)) );

  connect( d->mMultiAgendaView, SIGNAL(newSubTodoSignal(Akonadi::Item)),
           SIGNAL(newSubTodoSignal(Akonadi::Item)) );

  connect( d->mMultiAgendaView, SIGNAL(newJournalSignal(QDate)),
           SIGNAL(newJournalSignal(QDate)) );

}

void MultiAgendaView::setCalendar( CalendarSupport::Calendar *cal )
{
  d->mMultiAgendaView->setCalendar( cal );
}

MultiAgendaView::~MultiAgendaView()
{
  delete d;
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

void MultiAgendaView::setChanges( EventViews::EventView::Changes changes )
{
  // Only ConfigChanged and FilterChanged should go from korg->AgendaView
  // All other values are already detected inside AgendaView.
  // We could just pass "changes", but korganizer does a very bad job at
  // determining what changed, for example if you move an incidence
  // the BaseView::setDateRange(...) is called causing DatesChanged
  // flag to be on, when no dates changed.
  EventViews::EventView::Changes c;
  if ( changes.testFlag( EventViews::EventView::ConfigChanged ) )
    c = EventViews::EventView::ConfigChanged;

  if ( changes.testFlag( EventViews::EventView::FilterChanged ) )
    c |= EventViews::EventView::FilterChanged;

  d->mMultiAgendaView->setChanges( c | d->mMultiAgendaView->changes() );
}

bool MultiAgendaView::hasConfigurationDialog() const
{
  return d->mMultiAgendaView->hasConfigurationDialog();
}

void MultiAgendaView::showConfigurationDialog( QWidget *parent )
{
  d->mMultiAgendaView->showConfigurationDialog( parent );
}


CalendarSupport::CollectionSelectionProxyModel *MultiAgendaView::takeCustomCollectionSelectionProxyModel()
{
  return d->mMultiAgendaView->takeCustomCollectionSelectionProxyModel();
}

void MultiAgendaView::setCustomCollectionSelectionProxyModel( CalendarSupport::CollectionSelectionProxyModel* model )
{
  d->mMultiAgendaView->setCustomCollectionSelectionProxyModel( model );
}

#include "multiagendaview.moc"
