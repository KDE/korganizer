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

#include <libkcal/calendarresources.h>

#include <qlayout.h>
#include <qvbox.h>
#include <qobjectlist.h>

#define FOREACH_VIEW(av) \
for(QValueList<KOAgendaView*>::ConstIterator it = mAgendaViews.constBegin(); \
  it != mAgendaViews.constEnd();) \
  for(KOAgendaView* av = (it != mAgendaViews.constEnd() ? (*it) : 0); \
      it != mAgendaViews.constEnd(); ++it, av = (*it)  )

using namespace KOrg;

MultiAgendaView::MultiAgendaView(Calendar * cal, QWidget * parent, const char *name ) :
    AgendaView( cal, parent, name )
{
  QBoxLayout *topLevelLayout = new QHBoxLayout( this );
  mScrollView = new QScrollView( this );
  mScrollView->setResizePolicy( QScrollView::Manual );
  mScrollView->setVScrollBarMode( QScrollView::AlwaysOff );
  mScrollView->setFrameShape( QFrame::NoFrame );
  topLevelLayout->addWidget( mScrollView );
  mTopBox = new QHBox( mScrollView->viewport() );
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
  for ( QValueList<QWidget*>::ConstIterator it = mAgendaWidgets.constBegin();
        it != mAgendaWidgets.constEnd(); ++it ) {
    delete *it;
  }
  mAgendaViews.clear();
  mAgendaWidgets.clear();
}

void MultiAgendaView::setupViews()
{
  FOREACH_VIEW( agenda ) {
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

  FOREACH_VIEW( agenda ) {
    agenda->readSettings();
  }
}

MultiAgendaView::~ MultiAgendaView()
{
}

Incidence::List MultiAgendaView::selectedIncidences()
{
  Incidence::List list;
  FOREACH_VIEW(agendaView) {
    list += agendaView->selectedIncidences();
  }
  return list;
}

DateList MultiAgendaView::selectedDates()
{
  DateList list;
  FOREACH_VIEW(agendaView) {
    list += agendaView->selectedDates();
  }
  return list;
}

int MultiAgendaView::currentDateCount()
{
  FOREACH_VIEW( agendaView )
    return agendaView->currentDateCount();
  return 0;
}

void MultiAgendaView::showDates(const QDate & start, const QDate & end)
{
  kdDebug() << k_funcinfo << endl;
  recreateViews();
  FOREACH_VIEW( agendaView )
    agendaView->showDates( start, end );
}

void MultiAgendaView::showIncidences(const Incidence::List & incidenceList)
{
  FOREACH_VIEW( agendaView )
    agendaView->showIncidences( incidenceList );
}

void MultiAgendaView::updateView()
{
  kdDebug() << k_funcinfo << endl;
  recreateViews();
  FOREACH_VIEW( agendaView )
    agendaView->updateView();
}

void MultiAgendaView::changeIncidenceDisplay(Incidence * incidence, int mode)
{
  FOREACH_VIEW( agendaView )
    agendaView->changeIncidenceDisplay( incidence, mode );
}

int MultiAgendaView::maxDatesHint()
{
  FOREACH_VIEW( agendaView )
    return agendaView->maxDatesHint();
  return 0;
}

void MultiAgendaView::slotSelectionChanged()
{
  FOREACH_VIEW( agenda ) {
    if ( agenda != sender() )
      agenda->clearSelection();
  }
}

bool MultiAgendaView::eventDurationHint(QDateTime & startDt, QDateTime & endDt, bool & allDay)
{
  FOREACH_VIEW( agenda ) {
    bool valid = agenda->eventDurationHint( startDt, endDt, allDay );
    if ( valid )
      return true;
  }
  return false;
}

void MultiAgendaView::slotClearTimeSpanSelection()
{
  FOREACH_VIEW( agenda ) {
    if ( agenda != sender() )
      agenda->clearTimeSpanSelection();
  }
}

void MultiAgendaView::setTypeAheadReceiver(QObject * o)
{
  FOREACH_VIEW( agenda )
    agenda->setTypeAheadReceiver( o );
}

void MultiAgendaView::finishTypeAhead()
{
  FOREACH_VIEW( agenda )
    agenda->finishTypeAhead();
}

void MultiAgendaView::addView( const QString &label, KCal::ResourceCalendar * res, const QString & subRes )
{
  QVBox *box = new QVBox( mTopBox );
  QLabel *l = new QLabel( label, box );
  l->setAlignment( AlignVCenter | AlignHCenter );
  KOAgendaView* av = new KOAgendaView( calendar(), box );
  av->setResource( res, subRes );
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
  int width = QMAX( mTopBox->sizeHint().width(), size.width() );
  int height = size.height();
  if ( width > size.width() )
    height -= mScrollView->horizontalScrollBar()->height();
  mScrollView->resizeContents( width, height );
  mTopBox->resize( width, height );
}

#include "multiagendaview.moc"
