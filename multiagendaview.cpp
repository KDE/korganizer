
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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "multiagendaview.h"

#include "koagendaview.h"

#include <kcal/calendarresources.h>

#include <qlayout.h>
#include <KHBox>
#include <KVBox>
#include <Q3ScrollView>

using namespace KOrg;

MultiAgendaView::MultiAgendaView(Calendar * cal, QWidget * parent ) :
    AgendaView( cal, parent )
{
  QBoxLayout *topLevelLayout = new QHBoxLayout( this );
  mScrollView = new Q3ScrollView( this );
  mScrollView->setResizePolicy( Q3ScrollView::Manual );
  mScrollView->setVScrollBarMode( Q3ScrollView::AlwaysOff );
  mScrollView->setFrameShape( QFrame::NoFrame );
  topLevelLayout->addWidget( mScrollView );
  mTopBox = new KHBox( mScrollView->viewport() );
  mScrollView->addChild( mTopBox );
  recreateViews();
}

void MultiAgendaView::recreateViews()
{
  deleteViews();

  CalendarResources *calres = dynamic_cast<CalendarResources*>( calendar() );
  if ( !calres ) {
    // fallback to single-agenda
    KOAgendaView* av = new KOAgendaView( calendar(), mTopBox );
    mAgendaViews.append( av );
    mAgendaWidgets.append( av );
    av->show();
  } else {
    CalendarResourceManager *manager = calres->resourceManager();
    for ( CalendarResourceManager::ActiveIterator it = manager->activeBegin(); it != manager->activeEnd(); ++it ) {
      if ( (*it)->canHaveSubresources() ) {
        QStringList subResources = (*it)->subresources();
        for ( QStringList::ConstIterator subit = subResources.constBegin(); subit != subResources.constEnd(); ++subit ) {
          QString type = (*it)->subresourceType( *subit );
          if ( !(*it)->subresourceActive( *subit ) || (!type.isEmpty() && type != "event") )
            continue;
          addView( (*it)->labelForSubresource( *subit ), *it, *subit );
        }
      } else {
        addView( (*it)->resourceName(), *it );
      }
    }
  }
  setupViews();
  resizeScrollView( size() );
}

void MultiAgendaView::deleteViews()
{
  for ( QList<QWidget*>::ConstIterator it = mAgendaWidgets.constBegin();
        it != mAgendaWidgets.constEnd(); ++it ) {
    delete *it;
  }
  mAgendaViews.clear();
  mAgendaWidgets.clear();
}

void MultiAgendaView::setupViews()
{
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    connect( agenda, SIGNAL( newEventSignal() ),
             SIGNAL( newEventSignal() ) );
    connect( agenda, SIGNAL( editIncidenceSignal( Incidence * ) ),
             SIGNAL( editIncidenceSignal( Incidence * ) ) );
    connect( agenda, SIGNAL( showIncidenceSignal( Incidence * ) ),
             SIGNAL( showIncidenceSignal( Incidence * ) ) );
    connect( agenda, SIGNAL( deleteIncidenceSignal( Incidence * ) ),
             SIGNAL( deleteIncidenceSignal( Incidence * ) ) );
    connect( agenda, SIGNAL( startMultiModify( const QString & ) ),
             SIGNAL( startMultiModify( const QString & ) ) );
    connect( agenda, SIGNAL( endMultiModify() ),
             SIGNAL( endMultiModify() ) );

    connect( agenda, SIGNAL( incidenceSelected( Incidence * ) ),
             SIGNAL( incidenceSelected( Incidence * ) ) );

    connect( agenda, SIGNAL(cutIncidenceSignal(Incidence*)),
             SIGNAL(cutIncidenceSignal(Incidence*)) );
    connect( agenda, SIGNAL(copyIncidenceSignal(Incidence*)),
             SIGNAL(copyIncidenceSignal(Incidence*)) );
    connect( agenda, SIGNAL(toggleAlarmSignal(Incidence*)),
             SIGNAL(toggleAlarmSignal(Incidence*)) );
    connect( agenda, SIGNAL(dissociateOccurrenceSignal(Incidence*, const QDate&)),
             SIGNAL(dissociateOccurrenceSignal(Incidence*, const QDate&)) );
    connect( agenda, SIGNAL(dissociateFutureOccurrenceSignal(Incidence*, const QDate&)),
             SIGNAL(dissociateFutureOccurrenceSignal(Incidence*, const QDate&)) );

    connect( agenda, SIGNAL(newEventSignal(const QDate&)),
             SIGNAL(newEventSignal(const QDate&)) );
    connect( agenda, SIGNAL(newEventSignal(const QDateTime&)),
             SIGNAL(newEventSignal(const QDateTime&)) );
    connect( agenda, SIGNAL(newEventSignal(const QDateTime&, const QDateTime&)),
             SIGNAL(newEventSignal(const QDateTime&, const QDateTime&)) );
    connect( agenda, SIGNAL(newTodoSignal(const QDate&)),
             SIGNAL(newTodoSignal(const QDate&)) );

    connect( agenda, SIGNAL(incidenceSelected(Incidence*)),
             SLOT(slotSelectionChanged()) );

    connect( agenda, SIGNAL(timeSpanSelectionChanged()),
             SLOT(slotClearTimeSpanSelection()) );

  }

  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    agenda->readSettings();
  }
}

MultiAgendaView::~ MultiAgendaView()
{
}

Incidence::List MultiAgendaView::selectedIncidences()
{
  Incidence::List list;
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    list += agendaView->selectedIncidences();
  }
  return list;
}

DateList MultiAgendaView::selectedDates()
{
  DateList list;
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    list += agendaView->selectedDates();
  }
  return list;
}

int MultiAgendaView::currentDateCount()
{
  foreach ( KOAgendaView *agendaView, mAgendaViews )
    return agendaView->currentDateCount();
  return 0;
}

void MultiAgendaView::showDates(const QDate & start, const QDate & end)
{
  recreateViews();
  foreach ( KOAgendaView *agendaView, mAgendaViews )
    agendaView->showDates( start, end );
}

void MultiAgendaView::showIncidences(const Incidence::List & incidenceList)
{
  foreach ( KOAgendaView *agendaView, mAgendaViews )
    agendaView->showIncidences( incidenceList );
}

void MultiAgendaView::updateView()
{
  recreateViews();
  foreach ( KOAgendaView *agendaView, mAgendaViews )
    agendaView->updateView();
}

void MultiAgendaView::changeIncidenceDisplay(Incidence * incidence, int mode)
{
  foreach ( KOAgendaView *agendaView, mAgendaViews )
    agendaView->changeIncidenceDisplay( incidence, mode );
}

int MultiAgendaView::maxDatesHint()
{
  foreach ( KOAgendaView *agendaView, mAgendaViews )
    return agendaView->maxDatesHint();
  return 0;
}

void MultiAgendaView::slotSelectionChanged()
{
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    if ( agenda != sender() )
      agenda->clearSelection();
  }
}

bool MultiAgendaView::eventDurationHint(QDateTime & startDt, QDateTime & endDt, bool & allDay)
{
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    bool valid = agenda->eventDurationHint( startDt, endDt, allDay );
    if ( valid )
      return true;
  }
  return false;
}

void MultiAgendaView::slotClearTimeSpanSelection()
{
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    if ( agenda != sender() )
      agenda->clearTimeSpanSelection();
  }
}

void MultiAgendaView::setTypeAheadReceiver(QObject * o)
{
  foreach ( KOAgendaView *agenda, mAgendaViews )
    agenda->setTypeAheadReceiver( o );
}

void MultiAgendaView::finishTypeAhead()
{
  foreach ( KOAgendaView *agenda, mAgendaViews )
    agenda->finishTypeAhead();
}

void MultiAgendaView::addView( const QString& label, KCal::ResourceCalendar * res,  const QString& subResource )
{
    KVBox *box = new KVBox( mTopBox );
    QLabel *l = new QLabel( label, box );
    l->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
    KOAgendaView *av = new KOAgendaView( calendar(), box );
    av->setResource( res, subResource );
    av->setIncidenceChanger( mChanger );
    mAgendaViews.append( av );
    mAgendaWidgets.append( box );
    box->show();
}

void MultiAgendaView::resizeEvent(QResizeEvent * ev)
{
  resizeScrollView( ev->size() );
  AgendaView::resizeEvent( ev );
}

void MultiAgendaView::resizeScrollView(const QSize & size)
{
  int width = qMax( mTopBox->sizeHint().width(), size.width() );
  int height = size.height();
  if ( width > size.width() )
    height -= mScrollView->horizontalScrollBar()->height();
  mScrollView->resizeContents( width, height );
  mTopBox->resize( width, height );
}

void MultiAgendaView::setIncidenceChanger(IncidenceChangerBase * changer)
{
  AgendaView::setIncidenceChanger( changer );
  foreach ( KOAgendaView *agenda, mAgendaViews )
    agenda->setIncidenceChanger( changer );
}

#include "multiagendaview.moc"
