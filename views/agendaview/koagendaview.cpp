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

#include <calendarviews/agenda/agendaview.h>
#include <QHBoxLayout>

class KOAgendaView::Private
{
  public:
    Private( bool isSideBySide, KOAgendaView *parent ) : q( parent )
    {
      mAgendaView = new EventViews::AgendaView( parent, isSideBySide );
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
  layout->addWidget( d->mAgendaView );

  connect( d->mAgendaView, SIGNAL(zoomViewHorizontally(QDate,int)),
           SIGNAL(zoomViewHorizontally(QDate,int)) );
  connect( d->mAgendaView, SIGNAL(timeSpanSelectionChanged()),
           SIGNAL(timeSpanSelectionChanged()) );

  connect( d->mAgendaView, SIGNAL(showIncidencePopupSignal(Akonadi::Item,QDate)),
           d->mPopup, SLOT(showIncidencePopup(Akonadi::Item,QDate)) );

  connect( d->mAgendaView, SIGNAL(showNewEventPopupSignal()),
           SLOT(showNewEventPopup()) );

  d->mAgendaView->show();
}

KOAgendaView::~KOAgendaView()
{
  delete d;
}

void KOAgendaView::setCalendar( CalendarSupport::Calendar *cal )
{
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
  d->mAgendaView->updateView();
}

void KOAgendaView::updateConfig()
{
  d->mAgendaView->updateConfig();
}

void KOAgendaView::createTimeBarHeaders()
{
  d->mAgendaView->createTimeBarHeaders();
}

void KOAgendaView::updateTimeBarWidth()
{
  d->mAgendaView->updateTimeBarWidth();
}

void KOAgendaView::showDates( const QDate &start, const QDate &end )
{
  d->mAgendaView->showDates( start, end );
}

void KOAgendaView::showIncidences( const Akonadi::Item::List &incidences, const QDate &date )
{
  d->mAgendaView->showIncidences( incidences, date );
}

void KOAgendaView::insertIncidence( const Akonadi::Item &aitem, const QDate &curDate )
{
  d->mAgendaView->insertIncidence( aitem, curDate );
}

void KOAgendaView::changeIncidenceDisplayAdded( const Akonadi::Item &aitem )
{
  d->mAgendaView->changeIncidenceDisplayAdded( aitem );
}

void KOAgendaView::changeIncidenceDisplay( const Akonadi::Item &aitem, int mode )
{
  d->mAgendaView->changeIncidenceDisplay( aitem, mode );
}

void KOAgendaView::clearView()
{
  d->mAgendaView->clearView();
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
  d->mAgendaView->slotTodosDropped( items, gpos, allDay );
}

void KOAgendaView::slotTodosDropped( const QList<Todo::Ptr> &items, const QPoint &gpos, bool allDay )
{
  d->mAgendaView->slotTodosDropped( items, gpos, allDay );
}
void KOAgendaView::startDrag( const Akonadi::Item &incidence )
{
  d->mAgendaView->startDrag( incidence );
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

void KOAgendaView::setContentsPos( int y )
{
  d->mAgendaView->setContentsPos( y );
}

void KOAgendaView::clearSelection()
{
  d->mAgendaView->clearSelection();
}

void KOAgendaView::deleteSelectedDateTime()
{
  d->mAgendaView->deleteSelectedDateTime();
}

void KOAgendaView::setIncidenceChanger( CalendarSupport::IncidenceChanger *changer )
{
  d->mAgendaView->setIncidenceChanger( changer );
}

void KOAgendaView::clearTimeSpanSelection()
{
  d->mAgendaView->clearTimeSpanSelection();
}

void KOAgendaView::setCollection( Akonadi::Collection::Id coll )
{
  d->mAgendaView->setCollection( coll );
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

#include "koagendaview.moc"
