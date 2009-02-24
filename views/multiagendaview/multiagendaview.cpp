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
#include "koprefs.h"
#include "timelabelszone.h"
#include "views/agendaview/koagenda.h"
#include "views/agendaview/koagendaview.h"

#include <kcal/calendarresources.h>

#include <KGlobalSettings>
#include <KHBox>
#include <KVBox>

#include <Q3ScrollView>
#include <QHBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QSplitter>

using namespace KOrg;

MultiAgendaView::MultiAgendaView( Calendar *cal, QWidget *parent )
  : AgendaView( cal, parent ),
    mUpdateOnShow( true ),
    mPendingChanges( true )
{
  QHBoxLayout *topLevelLayout = new QHBoxLayout( this );
  topLevelLayout->setSpacing( 0 );
  topLevelLayout->setMargin( 0 );

  QFontMetrics fm( font() );
  int topLabelHeight = 2 * fm.lineSpacing() + 6; // 3 * 2: spacing in the KOAgendaView

  KVBox *topSideBox = new KVBox( this );
  QWidget *topSideSpacer = new QWidget( topSideBox );
  topSideSpacer->setFixedHeight( topLabelHeight );
  mLeftSplitter = new QSplitter( Qt::Vertical, topSideBox );
  mLeftSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );
  QLabel *label = new QLabel( i18n( "All Day" ), mLeftSplitter );
  label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
  label->setWordWrap( true );
  KVBox *sideBox = new KVBox( mLeftSplitter );
  EventIndicator *eiSpacer = new EventIndicator( EventIndicator::Top, sideBox );
  eiSpacer->changeColumns( 0 );
  QWidget *timeLabelTopAlignmentSpacer = new QWidget( sideBox ); // compensate for the frame the agenda views but not the timelabels have
  mTimeLabelsZone = new TimeLabelsZone( sideBox );
  QWidget *timeLabelBotAlignmentSpacer = new QWidget( sideBox );
  eiSpacer = new EventIndicator( EventIndicator::Bottom, sideBox );
  eiSpacer->changeColumns( 0 );
  mLeftBottomSpacer = new QWidget( topSideBox );
  topLevelLayout->addWidget( topSideBox );

  mScrollView = new Q3ScrollView( this );
  mScrollView->setResizePolicy( Q3ScrollView::Manual );
  mScrollView->setVScrollBarMode( Q3ScrollView::AlwaysOff );
  timeLabelTopAlignmentSpacer->setFixedHeight( mScrollView->frameWidth() - 1 ); // asymetric since the timelabels
  timeLabelBotAlignmentSpacer->setFixedHeight( mScrollView->frameWidth() - 2 ); // have 25 horizontal lines
  mScrollView->setFrameShape( QFrame::NoFrame );
  topLevelLayout->addWidget( mScrollView, 100 );
  mTopBox = new KHBox( mScrollView->viewport() );
  mScrollView->addChild( mTopBox );

  topSideBox = new KVBox( this );
  topSideSpacer = new QWidget( topSideBox );
  topSideSpacer->setFixedHeight( topLabelHeight );
  mRightSplitter = new QSplitter( Qt::Vertical, topSideBox );
  mRightSplitter->setOpaqueResize( KGlobalSettings::opaqueResize() );
  new QWidget( mRightSplitter );
  sideBox = new KVBox( mRightSplitter );
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
  if ( !mPendingChanges ) {
    return;
  }
  mPendingChanges = false;

  deleteViews();

  CalendarResources *calres = dynamic_cast<CalendarResources*>( calendar() );
  if ( !calres ) {
    // fallback to single-agenda
    KOAgendaView *av = new KOAgendaView( calendar(), mTopBox );
    mAgendaViews.append( av );
    mAgendaWidgets.append( av );
    av->show();
  } else {
    CalendarResourceManager *manager = calres->resourceManager();
    for ( CalendarResourceManager::ActiveIterator it = manager->activeBegin();
          it != manager->activeEnd(); ++it ) {
      if ( (*it)->canHaveSubresources() ) {
        QStringList subResources = (*it)->subresources();
        for ( QStringList::ConstIterator subit = subResources.constBegin();
              subit != subResources.constEnd(); ++subit ) {
          QString type = (*it)->subresourceType( *subit );
          if ( !(*it)->subresourceActive( *subit ) ||
               ( !type.isEmpty() && type != "event" ) ) {
            continue;
          }
          addView( (*it)->labelForSubresource( *subit ), *it, *subit );
        }
      } else {
        addView( (*it)->resourceName(), *it );
      }
    }
  }

  // no resources activated, so stop here to avoid crashing somewhere down the line
  // TODO: show a nice message instead
  if ( mAgendaViews.isEmpty() ) {
    return;
  }

  setupViews();
  QTimer::singleShot( 0, this, SLOT(slotResizeScrollView()) );
  mTimeLabelsZone->updateAll();

  TimeLabels *timeLabel = mTimeLabelsZone->timeLabels().first();
  connect( timeLabel->verticalScrollBar(), SIGNAL(valueChanged(int)),
           mScrollBar, SLOT(setValue(int)) );
  connect( mScrollBar, SIGNAL(valueChanged(int)),
           timeLabel, SLOT(positionChanged(int)) );

  connect( mLeftSplitter, SIGNAL(splitterMoved(int,int)), SLOT(resizeSplitters()) );
  connect( mRightSplitter, SIGNAL(splitterMoved(int,int)), SLOT(resizeSplitters()) );
  QTimer::singleShot( 0, this, SLOT(resizeSplitters()) );
  QTimer::singleShot( 0, this, SLOT(setupScrollBar()) );
  foreach ( TimeLabels *label, mTimeLabelsZone->timeLabels() )
    label->positionChanged();
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
    connect( agenda, SIGNAL(newEventSignal()),
             SIGNAL(newEventSignal()) );
    connect( agenda, SIGNAL(editIncidenceSignal(Incidence *)),
             SIGNAL(editIncidenceSignal(Incidence *)) );
    connect( agenda, SIGNAL(showIncidenceSignal(Incidence *)),
             SIGNAL(showIncidenceSignal(Incidence *)) );
    connect( agenda, SIGNAL(deleteIncidenceSignal(Incidence *)),
             SIGNAL(deleteIncidenceSignal(Incidence *)) );
    connect( agenda, SIGNAL(startMultiModify(const QString &)),
             SIGNAL(startMultiModify(const QString &)) );
    connect( agenda, SIGNAL(endMultiModify()),
             SIGNAL(endMultiModify()) );

    connect( agenda, SIGNAL(incidenceSelected(Incidence *)),
             SIGNAL(incidenceSelected(Incidence *)) );

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

    disconnect( agenda->agenda(),
                SIGNAL(zoomView(const int,const QPoint&,const Qt::Orientation)),
                agenda, 0 );
    connect( agenda->agenda(),
             SIGNAL(zoomView(const int,const QPoint&,const Qt::Orientation)),
             SLOT(zoomView(const int,const QPoint&,const Qt::Orientation)) );
  }

  KOAgendaView *lastView = mAgendaViews.last();
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    if ( agenda != lastView ) {
      connect( agenda->agenda()->verticalScrollBar(), SIGNAL(valueChanged(int)),
               lastView->agenda()->verticalScrollBar(), SLOT(setValue(int)) );
    }
  }

  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    agenda->readSettings();
  }

  int minWidth = 0;
  foreach ( QWidget *widget, mAgendaWidgets ) {
    minWidth = qMax( minWidth, widget->minimumSizeHint().width() );
  }
  foreach ( QWidget *widget, mAgendaWidgets ) {
    widget->setMinimumWidth( minWidth );
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
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    return agendaView->currentDateCount();
  }
  return 0;
}

void MultiAgendaView::showDates( const QDate &start, const QDate &end )
{
  mStartDate = start;
  mEndDate = end;
  recreateViews();
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    agendaView->showDates( start, end );
  }
}

void MultiAgendaView::showIncidences( const Incidence::List &incidenceList )
{
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    agendaView->showIncidences( incidenceList );
  }
}

void MultiAgendaView::updateView()
{
  recreateViews();
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    agendaView->updateView();
  }
}

void MultiAgendaView::changeIncidenceDisplay( Incidence *incidence, int mode )
{
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    agendaView->changeIncidenceDisplay( incidence, mode );
  }
}

int MultiAgendaView::maxDatesHint()
{
  foreach ( KOAgendaView *agendaView, mAgendaViews ) {
    return agendaView->maxDatesHint();
  }
  return 0;
}

void MultiAgendaView::slotSelectionChanged()
{
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    if ( agenda != sender() ) {
      agenda->clearSelection();
    }
  }
}

bool MultiAgendaView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    bool valid = agenda->eventDurationHint( startDt, endDt, allDay );
    if ( valid ) {
      return true;
    }
  }
  return false;
}

void MultiAgendaView::slotClearTimeSpanSelection()
{
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    if ( agenda != sender() ) {
      agenda->clearTimeSpanSelection();
    }
  }
}

void MultiAgendaView::setTypeAheadReceiver( QObject *o )
{
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    agenda->setTypeAheadReceiver( o );
  }
}

void MultiAgendaView::finishTypeAhead()
{
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    agenda->finishTypeAhead();
  }
}

void MultiAgendaView::addView( const QString &label, KCal::ResourceCalendar *res,
                               const QString &subResource )
{
  KVBox *box = new KVBox( mTopBox );
  QLabel *l = new QLabel( label, box );
  l->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
  KOAgendaView *av = new KOAgendaView( calendar(), box, true );
  av->setResource( res, subResource );
  av->setIncidenceChanger( mChanger );
  av->agenda()->setVScrollBarMode( Q3ScrollView::AlwaysOff );
  mAgendaViews.append( av );
  mAgendaWidgets.append( box );
  box->show();
  mTimeLabelsZone->setAgendaView( av );

  connect( av->splitter(), SIGNAL(splitterMoved(int,int)), SLOT(resizeSplitters()) );
}

void MultiAgendaView::resizeEvent( QResizeEvent *ev )
{
  resizeScrollView( ev->size() );
  AgendaView::resizeEvent( ev );
}

void MultiAgendaView::resizeScrollView( const QSize &size )
{
  const int widgetWidth = size.width() - mTimeLabelsZone->width() - mScrollBar->width();
  const int width = qMax( mTopBox->sizeHint().width(), widgetWidth );
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

void MultiAgendaView::setIncidenceChanger( IncidenceChangerBase *changer )
{
  AgendaView::setIncidenceChanger( changer );
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    agenda->setIncidenceChanger( changer );
  }
}

void MultiAgendaView::updateConfig()
{
  AgendaView::updateConfig();
  mTimeLabelsZone->updateAll();
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    agenda->updateConfig();
  }
}

void MultiAgendaView::resizeSplitters()
{
  QSplitter *lastMovedSplitter = qobject_cast<QSplitter*>( sender() );
  if ( !lastMovedSplitter ) {
    lastMovedSplitter = mAgendaViews.first()->splitter();
  }
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    if ( agenda->splitter() == lastMovedSplitter ) {
      continue;
    }
    agenda->splitter()->setSizes( lastMovedSplitter->sizes() );
  }
  if ( lastMovedSplitter != mLeftSplitter ) {
    mLeftSplitter->setSizes( lastMovedSplitter->sizes() );
  }
  if ( lastMovedSplitter != mRightSplitter ) {
    mRightSplitter->setSizes( lastMovedSplitter->sizes() );
  }
}

void MultiAgendaView::zoomView( const int delta, const QPoint &pos, const Qt::Orientation ori )
{
  if ( ori == Qt::Vertical ) {
    if ( delta > 0 ) {
      if ( KOPrefs::instance()->mHourSize > 4 ) {
        KOPrefs::instance()->mHourSize--;
      }
    } else {
      KOPrefs::instance()->mHourSize++;
    }
  }

  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    agenda->zoomView( delta, pos, ori );
  }

  mTimeLabelsZone->updateAll();
}

void MultiAgendaView::slotResizeScrollView()
{
  resizeScrollView( size() );
}

void MultiAgendaView::showEvent( QShowEvent *event )
{
  AgendaView::showEvent( event );
  if ( mUpdateOnShow ) {
    mUpdateOnShow = false;
    mPendingChanges = true; // force a full view recreation
    showDates( mStartDate, mEndDate );
  }
}

void MultiAgendaView::setUpdateNeeded()
{
  mPendingChanges = true;
  foreach ( KOAgendaView *agenda, mAgendaViews ) {
    agenda->setUpdateNeeded();
  }
}

void MultiAgendaView::setupScrollBar()
{
  QScrollBar *scrollBar = mAgendaViews.first()->agenda()->verticalScrollBar();
  mScrollBar->setMinimum( scrollBar->minimum() );
  mScrollBar->setMaximum( scrollBar->maximum() );
  mScrollBar->setSingleStep( scrollBar->singleStep() );
  mScrollBar->setPageStep( scrollBar->pageStep() );
  mScrollBar->setValue( scrollBar->value() );
}

#include "multiagendaview.moc"
