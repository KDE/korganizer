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
#include <calendarviews/agenda/agendaview.h>


class KOAgendaView::Private
{
  public:
    Private( bool isSideBySide, QWidget *parent )
    {
      mAgendaView = new EventViews::AgendaView( parent, isSideBySide );
    }

  EventViews::AgendaView *mAgendaView;

};

KOAgendaView::KOAgendaView( QWidget *parent, bool isSideBySide ) :
  KOEventView( parent ), d( new Private( isSideBySide, parent ) )
{
  d->mAgendaView->show();

// TODO_EVENTVIEWS: the popup
//  connectAgenda( mAgenda, mAgendaPopup, mAllDayAgenda );
//  connectAgenda( mAllDayAgenda, mAllDayAgendaPopup, mAgenda );
}

KOAgendaView::~KOAgendaView()
{
//TODO_EVENTVIEWS: popups
//  delete mAgendaPopup;
//  delete mAllDayAgendaPopup;
}

void KOAgendaView::setCalendar( CalendarSupport::Calendar *cal )
{
  d->mAgendaView->setCalendar( cal );
}

//TODO_EVENTVIEW: POPUPS
/*
void KOAgendaView::connectAgenda( KOAgenda *agenda, KOEventPopupMenu *popup,
                                  KOAgenda *otherAgenda )
{
  connect( agenda, SIGNAL(showIncidencePopupSignal(Akonadi::Item,QDate)),
           popup, SLOT(showIncidencePopup(Akonadi::Item,QDate)) );

  connect( agenda, SIGNAL(showNewEventPopupSignal()),
           SLOT(showNewEventPopup()) );
  agenda->setCalendar( calendar() );

  // Create/Show/Edit/Delete Event
  // Is the newEventSignal even emitted? It doesn't seem to reach handleNewEventRequest()
  // at least.
  connect( agenda, SIGNAL(newEventSignal()), SLOT(handleNewEventRequest()) );

  connect( agenda, SIGNAL(newStartSelectSignal()),
           otherAgenda, SLOT(clearSelection()) );
  connect( agenda, SIGNAL(newStartSelectSignal()),
           SIGNAL(timeSpanSelectionChanged()) );

  connect( agenda, SIGNAL(editIncidenceSignal(Akonadi::Item)),
                   SIGNAL(editIncidenceSignal(Akonadi::Item)) );
  connect( agenda, SIGNAL(showIncidenceSignal(Akonadi::Item)),
                   SIGNAL(showIncidenceSignal(Akonadi::Item)) );
  connect( agenda, SIGNAL(deleteIncidenceSignal(Akonadi::Item)),
                   SIGNAL(deleteIncidenceSignal(Akonadi::Item)) );

  connect( agenda, SIGNAL(startMultiModify(const QString &)),
                   SIGNAL(startMultiModify(const QString &)) );
  connect( agenda, SIGNAL(endMultiModify()),
                   SIGNAL(endMultiModify()) );

  connect( agenda, SIGNAL(itemModified(KOAgendaItem *)),
                   SLOT(updateEventDates(KOAgendaItem *)) );
  connect( agenda, SIGNAL(enableAgendaUpdate(bool)),
                   SLOT(enableAgendaUpdate(bool)) );

  // drag signals
  connect( agenda, SIGNAL(startDragSignal(Akonadi::Item)),
           SLOT(startDrag(Akonadi::Item)) );

  // synchronize selections
  connect( agenda, SIGNAL(incidenceSelected(const Akonadi::Item &, const QDate &)),
           otherAgenda, SLOT(deselectItem()) );
  connect( agenda, SIGNAL(incidenceSelected(const Akonadi::Item &, const QDate &)),
           SIGNAL(incidenceSelected(const Akonadi::Item &, const QDate &)) );

  // rescheduling of todos by d'n'd
  connect( agenda, SIGNAL(droppedToDos(QList<KCalCore::Todo::Ptr>,const QPoint &,bool)),
           SLOT(slotTodosDropped(QList<KCalCore::Todo::Ptr>,const QPoint &,bool)) );
  connect( agenda, SIGNAL(droppedToDos(QList<KUrl>,const QPoint &,bool)),
           SLOT(slotTodosDropped(QList<KUrl>,const QPoint &,bool)) );

}
*/

void KOAgendaView::zoomInVertically()
{
  d->mAgendaView->zoomInVertically();
}

void KOAgendaView::zoomOutVertically()
{
  return d->mAgendaView->zoomOutVertically();
}

void KOAgendaView::zoomInHorizontally( const QDate &date )
{
  return d->mAgendaView->zoomInHorizontally( date );
}

void KOAgendaView::zoomOutHorizontally( const QDate &date )
{
  return d->mAgendaView->zoomOutHorizontally( date );
}

void KOAgendaView::zoomView( const int delta, const QPoint &pos, const Qt::Orientation orient )
{
  return d->mAgendaView->zoomView( delta, pos, orient );
}

#ifndef KORG_NODECOS
/*
  TODO_EVENTVIEW: decorations
bool KOAgendaView::loadDecorations( const QStringList &decorations, DecorationList &decoList )
{
  foreach ( const QString &decoName, decorations ) {
    if ( KOPrefs::instance()->selectedPlugins().contains( decoName ) ) {
      decoList << KOCore::self()->loadCalendarDecoration( decoName );
    }
  }
  return ( decorations.count() > 0 );
}

void KOAgendaView::placeDecorationsFrame( KHBox *frame, bool decorationsFound, bool isTop )
{
  if ( decorationsFound ) {

    if ( isTop ) {
      // inserts in the first position
      mSplitterAgenda->insertWidget( 0, frame );
    } else {
      // inserts in the last position
      frame->setParent( mSplitterAgenda );
    }
  } else {
    frame->setParent( this );
    mGridLayout->addWidget( frame, 0, 0 );
  }
}

void KOAgendaView::placeDecorations( DecorationList &decoList, const QDate &date,
                                     KHBox *labelBox, bool forWeek )
{
  foreach ( CalendarDecoration::Decoration *deco, decoList ) {
    CalendarDecoration::Element::List elements;
    elements = forWeek ? deco->weekElements( date ) : deco->dayElements( date );
    if ( elements.count() > 0 ) {
      KHBox *decoHBox = new KHBox( labelBox );
      decoHBox->setFrameShape( QFrame::StyledPanel );
      decoHBox->setMinimumWidth( 1 );

      foreach ( CalendarDecoration::Element *it, elements ) {
        KODecorationLabel *label = new KODecorationLabel( it, decoHBox );
        label->setAlignment( Qt::AlignBottom );
        label->setMinimumWidth( 1 );
      }
    }
  }
}
*/
#endif // KORG_NODECOS

void KOAgendaView::createDayLabels()
{
  //TODO_EVENTVIEW: this function had decoration code
  d->mAgendaView->createDayLabels( false );
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

DateList KOAgendaView::selectedIncidenceDates()
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
  return d->mAgendaView->updateView();
}

void KOAgendaView::updateConfig()
{
  return d->mAgendaView->updateConfig();
}

void KOAgendaView::createTimeBarHeaders()
{
  return d->mAgendaView->createTimeBarHeaders();
}

void KOAgendaView::updateTimeBarWidth()
{
  return d->mAgendaView->updateTimeBarWidth();
}

void KOAgendaView::showDates( const QDate &start, const QDate &end )
{
  return d->mAgendaView->showDates( start, end );
}

void KOAgendaView::showIncidences( const Akonadi::Item::List &incidences, const QDate &date )
{
  return d->mAgendaView->showIncidences( incidences, date );
}

void KOAgendaView::insertIncidence( const Akonadi::Item &aitem, const QDate &curDate )
{
  return d->mAgendaView->insertIncidence( aitem, curDate );
}

void KOAgendaView::changeIncidenceDisplayAdded( const Akonadi::Item &aitem )
{
  return d->mAgendaView->changeIncidenceDisplayAdded( aitem );
}

void KOAgendaView::changeIncidenceDisplay( const Akonadi::Item &aitem, int mode )
{
  return d->mAgendaView->changeIncidenceDisplay( aitem, mode );
}

void KOAgendaView::clearView()
{
  return d->mAgendaView->clearView();
}

CalPrinter::PrintType KOAgendaView::printType()
{
  // If up to three days are selected, use day style, otherwise week
  if ( currentDateCount() <= 3 ) {
    return CalPrinter::Day;
  } else {
    return CalPrinter::Week;
  }
}

void KOAgendaView::slotTodosDropped( const QList<KUrl> &items, const QPoint &gpos, bool allDay )
{
  return d->mAgendaView->slotTodosDropped( items, gpos, allDay );
}

void KOAgendaView::slotTodosDropped( const QList<Todo::Ptr> &items, const QPoint &gpos, bool allDay )
{
  return d->mAgendaView->slotTodosDropped( items, gpos, allDay );
}
void KOAgendaView::startDrag( const Akonadi::Item &incidence )
{
  return d->mAgendaView->startDrag( incidence );
}

void KOAgendaView::readSettings()
{
  return d->mAgendaView->readSettings();
}

void KOAgendaView::readSettings( KConfig *config )
{
  return d->mAgendaView->readSettings( config );
}

void KOAgendaView::writeSettings( KConfig *config )
{
  return d->mAgendaView->writeSettings( config );
}

void KOAgendaView::setContentsPos( int y )
{
  return d->mAgendaView->setContentsPos( y );
}

void KOAgendaView::clearSelection()
{
  return d->mAgendaView->clearSelection();
}

void KOAgendaView::deleteSelectedDateTime()
{
  return d->mAgendaView->deleteSelectedDateTime();
}

void KOAgendaView::setIncidenceChanger( CalendarSupport::IncidenceChanger *changer )
{
  return d->mAgendaView->setIncidenceChanger( changer );
}

void KOAgendaView::clearTimeSpanSelection()
{
  return d->mAgendaView->clearTimeSpanSelection();
}

void KOAgendaView::setCollection( Akonadi::Collection::Id coll )
{
  return d->mAgendaView->setCollection( coll );
}

Akonadi::Collection::Id KOAgendaView::collection() const
{
  return d->mAgendaView->collection();
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

/*
KOAgenda * KOAgendaView::agenda() const
{
//TODO_EVENTVIEW, why do we need this here?
  return 0; // review what this function is fore
  }*/

QSplitter * KOAgendaView::splitter() const
{
//TODO_EVENTVIEW, why do we need this here?
  return 0; // review what this function is fore
}

#include "koagendaview.moc"
