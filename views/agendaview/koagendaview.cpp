/*
  This file is part of KOrganizer.
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "koagendaview.h"
#include "koeventpopupmenu.h"
#include "koprefs.h"

#include <calendarviews/agenda/agendaview.h>

#include <QHBoxLayout>

class KOAgendaView::Private
{
  public:
    Private( bool isSideBySide, KOAgendaView *parent ) : q( parent )
    {
      mAgendaView = new EventViews::AgendaView( KOPrefs::instance()->eventViewsPreferences(),
                                                QDate::currentDate(),
                                                QDate::currentDate(),
                                                true,
                                                isSideBySide,
                                                parent );
      mPopup = q->eventPopup();
    }

    ~Private()
    {
      delete mAgendaView;
      delete mPopup;
    }

    EventViews::AgendaView *mAgendaView;
    KOEventPopupMenu *mPopup;

  private:
    KOAgendaView * const q;
};

KOAgendaView::KOAgendaView( QWidget *parent, bool isSideBySide ) :
  KOEventView( parent ), d( new Private( isSideBySide, this ) )
{
  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setMargin( 0 );
  layout->addWidget( d->mAgendaView );

  connect( d->mAgendaView, SIGNAL(zoomViewHorizontally(QDate,int)),
           SIGNAL(zoomViewHorizontally(QDate,int)) );

  connect( d->mAgendaView, SIGNAL(timeSpanSelectionChanged()),
           SIGNAL(timeSpanSelectionChanged()) );

  connect( d->mAgendaView, SIGNAL(showIncidencePopupSignal(Akonadi::Item,QDate)),
           d->mPopup, SLOT(showIncidencePopup(Akonadi::Item,QDate)) );

  connect( d->mAgendaView, SIGNAL(showNewEventPopupSignal()),
           SLOT(showNewEventPopup()) );

  connect( d->mAgendaView, SIGNAL(datesSelected(KCalCore::DateList)),
           SIGNAL(datesSelected(KCalCore::DateList)) );

  connect( d->mAgendaView, SIGNAL(shiftedEvent(QDate,QDate)),
           SIGNAL(shiftedEvent(QDate,QDate)) );

  connect( d->mAgendaView, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           SIGNAL(incidenceSelected(Akonadi::Item,QDate)) );

  connect( d->mAgendaView, SIGNAL(showIncidenceSignal(Akonadi::Item)),
           SIGNAL(showIncidenceSignal(Akonadi::Item)) );

  connect( d->mAgendaView, SIGNAL(editIncidenceSignal(Akonadi::Item)),
           SIGNAL(editIncidenceSignal(Akonadi::Item)) );

  connect( d->mAgendaView, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
           SIGNAL(deleteIncidenceSignal(Akonadi::Item)) );

  connect( d->mAgendaView, SIGNAL(cutIncidenceSignal(Akonadi::Item)),
           SIGNAL(cutIncidenceSignal(Akonadi::Item)) );

  connect( d->mAgendaView, SIGNAL(copyIncidenceSignal(Akonadi::Item)),
           SIGNAL(copyIncidenceSignal(Akonadi::Item)) );

  connect( d->mAgendaView, SIGNAL(pasteIncidenceSignal()),
           SIGNAL(pasteIncidenceSignal()) );

  connect( d->mAgendaView, SIGNAL(toggleAlarmSignal(Akonadi::Item)),
           SIGNAL(toggleAlarmSignal(Akonadi::Item)) );

  connect( d->mAgendaView, SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)),
           SIGNAL(toggleTodoCompletedSignal(Akonadi::Item)) );

  connect( d->mAgendaView, SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,QString)),
           SIGNAL(copyIncidenceToResourceSignal(Akonadi::Item,QString)) );

  connect( d->mAgendaView, SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,QString)),
           SIGNAL(moveIncidenceToResourceSignal(Akonadi::Item,QString)) );

  connect( d->mAgendaView, SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)),
           SIGNAL(dissociateOccurrencesSignal(Akonadi::Item,QDate)) );

  connect( d->mAgendaView, SIGNAL(newEventSignal()),
           SIGNAL(newEventSignal()) );

  connect( d->mAgendaView, SIGNAL(newEventSignal(QDate)),
           SIGNAL(newEventSignal(QDate)) );

  connect( d->mAgendaView, SIGNAL(newEventSignal(QDateTime)),
           SIGNAL(newEventSignal(QDateTime)) );

  connect( d->mAgendaView, SIGNAL(newEventSignal(QDateTime,QDateTime)),
           SIGNAL(newEventSignal(QDateTime,QDateTime)) );

  connect( d->mAgendaView, SIGNAL(newTodoSignal(QDate)),
           SIGNAL(newTodoSignal(QDate)) );

  connect( d->mAgendaView, SIGNAL(newSubTodoSignal(Akonadi::Item)),
           SIGNAL(newSubTodoSignal(Akonadi::Item)) );

  connect( d->mAgendaView, SIGNAL(newJournalSignal(QDate)),
           SIGNAL(newJournalSignal(QDate)) );

  d->mAgendaView->show();
}

KOAgendaView::~KOAgendaView()
{
  delete d;
}

void KOAgendaView::setCalendar( const Akonadi::ETMCalendar::Ptr &cal )
{
  KOEventView::setCalendar( cal );
  d->mPopup->setCalendar( cal );
  d->mAgendaView->setCalendar( cal );
}

void KOAgendaView::zoomInVertically()
{
  d->mAgendaView->zoomInVertically();
}

void KOAgendaView::zoomOutVertically()
{
  d->mAgendaView->zoomOutVertically();
}

void KOAgendaView::zoomInHorizontally( const QDate &date )
{
  d->mAgendaView->zoomInHorizontally( date );
}

void KOAgendaView::zoomOutHorizontally( const QDate &date )
{
  d->mAgendaView->zoomOutHorizontally( date );
}

void KOAgendaView::zoomView( const int delta, const QPoint &pos, const Qt::Orientation orient )
{
  d->mAgendaView->zoomView( delta, pos, orient );
}

void KOAgendaView::enableAgendaUpdate( bool enable )
{
  d->mAgendaView->enableAgendaUpdate( enable );
}

int KOAgendaView::maxDatesHint() const
{
  // Not sure about the max number of events, so return 0 for now.
  return 0;
}

int KOAgendaView::currentDateCount() const
{
  return d->mAgendaView->currentDateCount();
}

Akonadi::Item::List KOAgendaView::selectedIncidences()
{
  return d->mAgendaView->selectedIncidences();
}

KCalCore::DateList KOAgendaView::selectedIncidenceDates()
{
  return d->mAgendaView->selectedIncidenceDates();
}

bool KOAgendaView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  return d->mAgendaView->eventDurationHint( startDt, endDt, allDay );
}

/** returns if only a single cell is selected, or a range of cells */
bool KOAgendaView::selectedIsSingleCell()
{
  return d->mAgendaView->selectedIsSingleCell();
}

void KOAgendaView::updateView()
{
  d->mAgendaView->updateView();
}

void KOAgendaView::updateConfig()
{
  d->mAgendaView->updateConfig();
}

void KOAgendaView::showDates( const QDate &start, const QDate &end, const QDate & )
{
  d->mAgendaView->showDates( start, end );
}

void KOAgendaView::showIncidences( const Akonadi::Item::List &incidences, const QDate &date )
{
  d->mAgendaView->showIncidences( incidences, date );
}

void KOAgendaView::changeIncidenceDisplayAdded( const Akonadi::Item & )
{
  // Do nothing, EventViews::AgendaView knows when items change
}

void KOAgendaView::changeIncidenceDisplay( const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType )
{
  // Do nothing, EventViews::AgendaView knows when items change
}

CalendarSupport::CalPrinter::PrintType KOAgendaView::printType() const
{
  // If up to three days are selected, use day style, otherwise week
  if ( currentDateCount() <= 3 ) {
    return CalendarSupport::CalPrinter::Day;
  } else {
    return CalendarSupport::CalPrinter::Week;
  }
}

void KOAgendaView::readSettings()
{
  d->mAgendaView->readSettings();
}

void KOAgendaView::readSettings( KConfig *config )
{
  d->mAgendaView->readSettings( config );
}

void KOAgendaView::writeSettings( KConfig *config )
{
  d->mAgendaView->writeSettings( config );
}

void KOAgendaView::clearSelection()
{
  d->mAgendaView->clearSelection();
}

void KOAgendaView::deleteSelectedDateTime()
{
  d->mAgendaView->deleteSelectedDateTime();
}

void KOAgendaView::setIncidenceChanger( Akonadi::IncidenceChanger *changer )
{
  d->mAgendaView->setIncidenceChanger( changer );
}

QDateTime KOAgendaView::selectionStart()
{
  return d->mAgendaView->selectionStart();
}

QDateTime KOAgendaView::selectionEnd()
{
  return d->mAgendaView->selectionEnd();
}

bool KOAgendaView::selectedIsAllDay()
{
  return d->mAgendaView->selectedIsAllDay();
}

void KOAgendaView::setTypeAheadReceiver( QObject *o )
{
  d->mAgendaView->setTypeAheadReceiver( o );
}

void KOAgendaView::setChanges( EventViews::EventView::Changes changes )
{
  // Only ConfigChanged and FilterChanged should go from korg->AgendaView
  // All other values are already detected inside AgendaView.
  // We could just pass "changes", but korganizer does a very bad job at
  // determining what changed, for example if you move an incidence
  // the BaseView::setDateRange(...) is called causing DatesChanged
  // flag to be on, when no dates changed.
  EventViews::EventView::Changes c;
  if ( changes.testFlag( EventViews::EventView::ConfigChanged ) ) {
    c = EventViews::EventView::ConfigChanged;
  }

  if ( changes.testFlag( EventViews::EventView::FilterChanged ) ) {
    c |= EventViews::EventView::FilterChanged;
  }

  d->mAgendaView->setChanges( c | d->mAgendaView->changes() );
}

void KOAgendaView::setDateRange( const KDateTime &start, const KDateTime &end, const QDate & )
{
  d->mAgendaView->setDateRange( start, end );
}

