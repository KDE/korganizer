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
#include "koagenda.h"
#include "koprefs.h"
#include "timelabels.h"

#include <libkcal/calendarresources.h>

#include <kglobalsettings.h>

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
    AgendaView( cal, parent, name ),
    mLastMovedSplitter( 0 ),
    mUpdateOnShow( false ),
    mPendingChanges( true )
{
  QBoxLayout *topLevelLayout = new QHBoxLayout( this );

  QFontMetrics fm( font() );
  int topLabelHeight = 2 * fm.height();

  QVBox *topSideBox = new QVBox( this );
  QWidget *topSideSpacer = new QWidget( topSideBox );
  topSideSpacer->setFixedHeight( topLabelHeight );
  mLeftSplitter = new QSplitter( Qt::Vertical, topSideBox );
  mLeftSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );
  QLabel *label = new QLabel( i18n("All Day"), mLeftSplitter );
  label->setAlignment( Qt::AlignRight | Qt::AlignVCenter | Qt::WordBreak );
  QVBox *sideBox = new QVBox( mLeftSplitter );
  EventIndicator *eiSpacer = new EventIndicator( EventIndicator::Top, sideBox );
  eiSpacer->changeColumns( 0 );
  mTimeLabels = new TimeLabels( 24, sideBox );
  eiSpacer = new EventIndicator( EventIndicator::Bottom, sideBox );
  eiSpacer->changeColumns( 0 );
  mLeftBottomSpacer = new QWidget( topSideBox );
  topLevelLayout->addWidget( topSideBox );

  mScrollView = new QScrollView( this );
  mScrollView->setResizePolicy( QScrollView::Manual );
  mScrollView->setVScrollBarMode( QScrollView::AlwaysOff );
  mScrollView->setFrameShape( QFrame::NoFrame );
  topLevelLayout->addWidget( mScrollView, 100 );
  mTopBox = new QHBox( mScrollView->viewport() );
  mScrollView->addChild( mTopBox );

  topSideBox = new QVBox( this );
  topSideSpacer = new QWidget( topSideBox );
  topSideSpacer->setFixedHeight( topLabelHeight );
  mRightSplitter = new QSplitter( Qt::Vertical, topSideBox );
  mRightSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );
  new QWidget( mRightSplitter );
  sideBox = new QVBox( mRightSplitter );
  eiSpacer = new EventIndicator( EventIndicator::Top, sideBox );
  eiSpacer->setFixedHeight( eiSpacer->minimumHeight() );
  eiSpacer->changeColumns( 0 );
  mScrollBar = new QScrollBar( Qt::Vertical, sideBox );
  eiSpacer = new EventIndicator( EventIndicator::Bottom, sideBox );
  eiSpacer->setFixedHeight( eiSpacer->minimumHeight() );
  eiSpacer->changeColumns( 0 );
  mRightBottomSpacer = new QWidget( topSideBox );
  topLevelLayout->addWidget( topSideBox );

  recreateViews();
}

void MultiAgendaView::recreateViews()
{
  if ( !mPendingChanges )
    return;
  mPendingChanges = false;

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

  // no resources activated, so stop here to avoid crashing somewhere down the line, TODO: show a nice message instead
  if ( mAgendaViews.isEmpty() )
    return;

  setupViews();
  QTimer::singleShot( 0, this, SLOT(slotResizeScrollView()) );
  mTimeLabels->updateConfig();

  QScrollBar *scrollBar = mAgendaViews.first()->agenda()->verticalScrollBar();
  mScrollBar->setMinValue( scrollBar->minValue() );
  mScrollBar->setMaxValue( scrollBar->maxValue() );
  mScrollBar->setLineStep( scrollBar->lineStep() );
  mScrollBar->setPageStep( scrollBar->pageStep() );
  mScrollBar->setValue( scrollBar->value() );
  connect( mTimeLabels->verticalScrollBar(), SIGNAL(valueChanged(int)),
           mScrollBar, SLOT(setValue(int)) );
  connect( mScrollBar, SIGNAL(valueChanged(int)),
           mTimeLabels, SLOT(positionChanged(int)) );

  installSplitterEventFilter( mLeftSplitter );
  installSplitterEventFilter( mRightSplitter );
  resizeSplitters();
}

void MultiAgendaView::deleteViews()
{
  for ( QValueList<QWidget*>::ConstIterator it = mAgendaWidgets.constBegin();
        it != mAgendaWidgets.constEnd(); ++it ) {
    delete *it;
  }
  mAgendaViews.clear();
  mAgendaWidgets.clear();
  mLastMovedSplitter = 0;
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
    connect( agenda, SIGNAL(pasteIncidenceSignal()),
             SIGNAL(pasteIncidenceSignal()) );
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

    disconnect( agenda->agenda(), SIGNAL(zoomView(const int,const QPoint&,const Qt::Orientation)), agenda, 0 );
    connect( agenda->agenda(), SIGNAL(zoomView(const int,const QPoint&,const Qt::Orientation)),
             SLOT(zoomView(const int,const QPoint&,const Qt::Orientation)) );
  }

  FOREACH_VIEW( agenda ) {
    agenda->readSettings();
  }

  int minWidth = 0;
  for ( QValueList<QWidget*>::ConstIterator it = mAgendaWidgets.constBegin(); it != mAgendaWidgets.constEnd(); ++it )
    minWidth = QMAX( minWidth, (*it)->minimumSizeHint().width() );
  for ( QValueList<QWidget*>::ConstIterator it = mAgendaWidgets.constBegin(); it != mAgendaWidgets.constEnd(); ++it )
    (*it)->setMinimumWidth( minWidth );
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
  mStartDate = start;
  mEndDate = end;
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
  KOAgendaView* av = new KOAgendaView( calendar(), box, 0, true );
  av->setResource( res, subRes );
  av->setIncidenceChanger( mChanger );
  av->agenda()->setVScrollBarMode( QScrollView::AlwaysOff );
  mAgendaViews.append( av );
  mAgendaWidgets.append( box );
  box->show();
  mTimeLabels->setAgenda( av->agenda() );

  connect( av->agenda()->verticalScrollBar(), SIGNAL(valueChanged(int)),
           mTimeLabels, SLOT(positionChanged(int)) );
  connect( mTimeLabels->verticalScrollBar(), SIGNAL(valueChanged(int)),
           av, SLOT(setContentsPos(int)) );

  installSplitterEventFilter( av->splitter() );
}

void MultiAgendaView::resizeEvent(QResizeEvent * ev)
{
  resizeScrollView( ev->size() );
  AgendaView::resizeEvent( ev );
}

void MultiAgendaView::resizeScrollView(const QSize & size)
{
  const int widgetWidth = size.width() - mTimeLabels->width() - mScrollBar->width();
  int width = QMAX( mTopBox->sizeHint().width(), widgetWidth );
  int height = size.height();
  if ( width > widgetWidth ) {
    const int sbHeight = mScrollView->horizontalScrollBar()->height();
    height -= sbHeight;
    mLeftBottomSpacer->setFixedHeight( sbHeight );
    mRightBottomSpacer->setFixedHeight( sbHeight );
  } else {
    mLeftBottomSpacer->setFixedHeight( 0 );
    mRightBottomSpacer->setFixedHeight( 0 );
  }
  mScrollView->resizeContents( width, height );
  mTopBox->resize( width, height );
}

void MultiAgendaView::setIncidenceChanger(IncidenceChangerBase * changer)
{
  AgendaView::setIncidenceChanger( changer );
  FOREACH_VIEW( agenda )
    agenda->setIncidenceChanger( changer );
}

void MultiAgendaView::updateConfig()
{
  AgendaView::updateConfig();
  mTimeLabels->updateConfig();
  FOREACH_VIEW( agenda )
    agenda->updateConfig();
}

// KDE4: not needed anymore, QSplitter has a moved signal there
bool MultiAgendaView::eventFilter(QObject * obj, QEvent * event)
{
  if ( obj->className() == QCString("QSplitterHandle") ) {
    if ( (event->type() == QEvent::MouseMove && KGlobalSettings::opaqueResize())
           || event->type() == QEvent::MouseButtonRelease ) {
      FOREACH_VIEW( agenda ) {
        if ( agenda->splitter() == obj->parent() )
          mLastMovedSplitter = agenda->splitter();
      }
      if ( mLeftSplitter == obj->parent() )
        mLastMovedSplitter = mLeftSplitter;
      else if ( mRightSplitter == obj->parent() )
        mLastMovedSplitter = mRightSplitter;
      QTimer::singleShot( 0, this, SLOT(resizeSplitters()) );
    }
  }
  return AgendaView::eventFilter( obj, event );
}

void MultiAgendaView::resizeSplitters()
{
  if ( !mLastMovedSplitter )
    mLastMovedSplitter = mAgendaViews.first()->splitter();
  FOREACH_VIEW( agenda ) {
    if ( agenda->splitter() == mLastMovedSplitter )
      continue;
    agenda->splitter()->setSizes( mLastMovedSplitter->sizes() );
  }
  if ( mLastMovedSplitter != mLeftSplitter )
    mLeftSplitter->setSizes( mLastMovedSplitter->sizes() );
  if ( mLastMovedSplitter != mRightSplitter )
    mRightSplitter->setSizes( mLastMovedSplitter->sizes() );
}

void MultiAgendaView::zoomView( const int delta, const QPoint & pos, const Qt::Orientation ori )
{
  if ( ori == Qt::Vertical ) {
    if ( delta > 0 ) {
      if ( KOPrefs::instance()->mHourSize > 4 )
        KOPrefs::instance()->mHourSize--;
    } else {
      KOPrefs::instance()->mHourSize++;
    }
  }

  FOREACH_VIEW( agenda )
    agenda->zoomView( delta, pos, ori );

  mTimeLabels->updateConfig();
  mTimeLabels->positionChanged();
  mTimeLabels->repaint();
}

// KDE4: not needed, use existing QSplitter signals instead
void MultiAgendaView::installSplitterEventFilter(QSplitter * splitter)
{
  QObjectList *objlist = splitter->queryList( "QSplitterHandle" );
  // HACK: when not being visible, the splitter handle is sometimes not found
  // for unknown reasons, so trigger an update when we are shown again
  if ( objlist->count() == 0 && !isVisible() )
    mUpdateOnShow = true;
  QObjectListIt it( *objlist );
  QObject *obj;
  while ( (obj = it.current()) != 0 ) {
    obj->removeEventFilter( this );
    obj->installEventFilter( this );
    ++it;
  }
  delete objlist;
}

void MultiAgendaView::slotResizeScrollView()
{
  resizeScrollView( size() );
}

void MultiAgendaView::show()
{
  AgendaView::show();
  if ( mUpdateOnShow ) {
    mUpdateOnShow = false;
    mPendingChanges = true; // force a full view recreation
    showDates( mStartDate, mEndDate );
  }
}

void MultiAgendaView::resourcesChanged()
{
  mPendingChanges = true;
  FOREACH_VIEW( agenda )
    agenda->resourcesChanged();
}

#include "multiagendaview.moc"
