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
#include "koagenda.h"
#include "koagendaitem.h"
#include "koalternatelabel.h"
#ifndef KORG_NODECOS
#include "kocore.h"
#include "kodecorationlabel.h"
#endif
#include "kodialogmanager.h"
#include "koeventpopupmenu.h"
#include "koglobals.h"
#include "koprefs.h"
#include "timelabelszone.h"
#include "akonadicollectionview.h"

#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/collectionselection.h>
#include <akonadi/kcal/utils.h>

#include <KCal/CalFilter>

#include <KCalendarSystem>
#include <KGlobalSettings>
#include <KHBox>
#include <KVBox>

#include <QDrag>
#include <QGridLayout>
#include <QPainter>
#include <QSplitter>
#include <KWordWrap>

using namespace KOrg;
using namespace Akonadi;

EventIndicator::EventIndicator( Location loc, QWidget *parent )
  : QFrame( parent )
{
  mColumns = 1;
  mEnabled.resize( mColumns );
  mLocation = loc;

  if ( mLocation == Top ) {
    mPixmap = KOGlobals::self()->smallIcon( "arrow-up-double" );
  } else {
    mPixmap = KOGlobals::self()->smallIcon( "arrow-down-double" );
  }

  setMinimumHeight( mPixmap.height() );
}

EventIndicator::~EventIndicator()
{
}

void EventIndicator::paintEvent( QPaintEvent *event )
{
  QFrame::paintEvent( event );

  QPainter painter( this );

  int i;
  for ( i=0; i<mColumns; ++i ) {
    if ( mEnabled[i] ) {
      int cellWidth = contentsRect().right() / mColumns;
      int xOffset = KOGlobals::self()->reverseLayout() ?
                    ( mColumns - 1 - i ) * cellWidth + cellWidth / 2 - mPixmap.width() / 2 :
                    i * cellWidth + cellWidth / 2 - mPixmap.width() / 2;
      painter.drawPixmap( QPoint( xOffset, 0 ), mPixmap );
    }
  }
}

void EventIndicator::changeColumns( int columns )
{
  mColumns = columns;
  mEnabled.resize( mColumns );

  update();
}

void EventIndicator::enableColumn( int column, bool enable )
{
  mEnabled[column] = enable;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

KOAgendaView::KOAgendaView( QWidget *parent, bool isSideBySide ) :
  KOrg::AgendaView( parent ),
  mTimeLabelsZone( 0 ),
  mAllowAgendaUpdate( true ),
  mUpdateItem( 0 ),
  mCollectionId( -1 ),
  mIsSideBySide( isSideBySide ),
  mPendingChanges( true )
{
  mSelectedDates.append( QDate::currentDate() );

  mLayoutTopDayLabels = 0;
  mTopDayLabelsFrame = 0;
  mTopDayLabels = 0;
  mLayoutBottomDayLabels = 0;
  mBottomDayLabelsFrame = 0;
  mBottomDayLabels = 0;

  mGridLayout = new QGridLayout( this );
  mGridLayout->setMargin( 0 );

  /* Create agenda splitter */
  mSplitterAgenda = new QSplitter( Qt::Vertical, this );
  mGridLayout->addWidget( mSplitterAgenda, 1, 0 );
  mSplitterAgenda->setOpaqueResize( KGlobalSettings::opaqueResize() );

  /* Create day name labels for agenda columns */
  mTopDayLabelsFrame = new KHBox( mSplitterAgenda );
  mTopDayLabelsFrame->setSpacing( 2 );

  /* Create all-day agenda widget */
  mAllDayFrame = new KHBox( mSplitterAgenda );
  mAllDayFrame->setSpacing( 2 );

  // Alignment and description widgets
  mTimeBarHeaderFrame = new KHBox( mAllDayFrame );

  // The widget itself
  QWidget *dummyAllDayLeft = new QWidget( mAllDayFrame );
  mAllDayAgenda = new KOAgenda( this, 1, mAllDayFrame );
  QWidget *dummyAllDayRight = new QWidget( mAllDayFrame );

  // Create the event context menu for the all-day agenda
  mAllDayAgendaPopup = eventPopup();

  /* Create the main agenda widget and the related widgets */
  QWidget *agendaFrame = new QWidget( mSplitterAgenda );
  mAgendaLayout = new QGridLayout( agendaFrame );
  mAgendaLayout->setMargin( 0 );
  mAgendaLayout->setHorizontalSpacing( 2 );
  mAgendaLayout->setVerticalSpacing( 0 );
  if ( isSideBySide ) {
    mTimeBarHeaderFrame->hide();
  }

  // Create event indicator bars
  mEventIndicatorTop = new EventIndicator( EventIndicator::Top, agendaFrame );
  mAgendaLayout->addWidget( mEventIndicatorTop, 0, 1 );
  mEventIndicatorBottom = new EventIndicator( EventIndicator::Bottom, agendaFrame );
  mAgendaLayout->addWidget( mEventIndicatorBottom, 2, 1 );

  // Alignment and description widgets
  QWidget *dummyAgendaRight = new QWidget( agendaFrame );
  mAgendaLayout->addWidget( dummyAgendaRight, 0, 2 );

  // Create agenda
  mAgenda = new KOAgenda( this, 1, 96, KOPrefs::instance()->mHourSize, agendaFrame );
  mAgendaLayout->addWidget( mAgenda, 1, 1, 1, 2 );
  mAgendaLayout->setColumnStretch( 1, 1 );

  // Create time labels
  mTimeLabelsZone = new TimeLabelsZone( this, mAgenda );
  mAgendaLayout->addWidget( mTimeLabelsZone, 1, 0 );

  // Create event context menu for agenda
  mAgendaPopup = eventPopup();

  // Scrolling
  connect( mAgenda, SIGNAL(zoomView(const int,QPoint,const Qt::Orientation)),
           SLOT(zoomView(const int,QPoint,const Qt::Orientation)) );

  // Event indicator updates
  connect( mAgenda, SIGNAL(lowerYChanged(int)),
           SLOT(updateEventIndicatorTop(int)) );
  connect( mAgenda, SIGNAL(upperYChanged(int)),
           SLOT(updateEventIndicatorBottom(int)) );

  if ( isSideBySide ) {
    mTimeLabelsZone->hide();
  }

  /* Create a frame at the bottom which may be used by decorations */
  mBottomDayLabelsFrame = new KHBox( mSplitterAgenda );
  mBottomDayLabelsFrame->setSpacing( 2 );

  if ( !isSideBySide ) {
    /* Make the all-day and normal agendas line up with each other */
    dummyAllDayRight->setFixedWidth( mAgenda->verticalScrollBar()->width() -
                                     mAgendaLayout->horizontalSpacing() );
    dummyAgendaRight->setFixedWidth( mAgenda->verticalScrollBar()->width() );
  }

  updateTimeBarWidth();
  // resize dummy widget so the allday agenda lines up with the hourly agenda
  dummyAllDayLeft->setFixedWidth( mTimeLabelsZone->width() - mTimeBarHeaderFrame->width() );

  /* Update widgets to reflect user preferences */
//  updateConfig();
  createDayLabels();

  /* Connect the agendas */
  connectAgenda( mAgenda, mAgendaPopup, mAllDayAgenda );
  connectAgenda( mAllDayAgenda, mAllDayAgendaPopup, mAgenda );

  connect( mAgenda,
           SIGNAL(newTimeSpanSignal(const QPoint &,const QPoint &)),
           SLOT(newTimeSpanSelected(const QPoint &,const QPoint &)) );
  connect( mAllDayAgenda,
           SIGNAL(newTimeSpanSignal(const QPoint &,const QPoint &)),
           SLOT(newTimeSpanSelectedAllDay(const QPoint &,const QPoint &)) );
}

KOAgendaView::~KOAgendaView()
{
  if ( calendar() ) {
    calendar()->unregisterObserver( this );
  }

  delete mAgendaPopup;
  delete mAllDayAgendaPopup;
}

void KOAgendaView::setCalendar( Akonadi::Calendar *cal )
{
  if( calendar() ) {
    calendar()->unregisterObserver( this );
  }
  Q_ASSERT( cal );
  KOrg::AgendaView::setCalendar(cal);
  calendar()->registerObserver( this );
}

void KOAgendaView::connectAgenda( KOAgenda *agenda, QMenu *popup,
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
  connect( agenda, SIGNAL(droppedToDos(QList<KCal::Todo::Ptr>,const QPoint &,bool)),
           SLOT(slotTodosDropped(QList<KCal::Todo::Ptr>,const QPoint &,bool)) );
  connect( agenda, SIGNAL(droppedToDos(QList<KUrl>,const QPoint &,bool)),
           SLOT(slotTodosDropped(QList<KUrl>,const QPoint &,bool)) );

}

void KOAgendaView::zoomInVertically( )
{
  if ( !mIsSideBySide ) {
    KOPrefs::instance()->mHourSize++;
  }
  mAgenda->updateConfig();
  mAgenda->checkScrollBoundaries();

  mTimeLabelsZone->updateAll();

  updateView();

}

void KOAgendaView::zoomOutVertically( )
{

  if ( KOPrefs::instance()->mHourSize > 4 || mIsSideBySide ) {
    if ( !mIsSideBySide ) {
      KOPrefs::instance()->mHourSize--;
    }
    mAgenda->updateConfig();
    mAgenda->checkScrollBoundaries();

    mTimeLabelsZone->updateAll();
    updateView();
  }
}

void KOAgendaView::zoomInHorizontally( const QDate &date )
{
  QDate begin;
  QDate newBegin;
  QDate dateToZoom = date;
  int ndays, count;

  begin = mSelectedDates.first();
  ndays = begin.daysTo( mSelectedDates.last() );

  // zoom with Action and are there a selected Incidence?, Yes, I zoom in to it.
  if ( ! dateToZoom.isValid () ) {
    dateToZoom = mAgenda->selectedIncidenceDate();
  }

  if( !dateToZoom.isValid() ) {
    if ( ndays > 1 ) {
      newBegin = begin.addDays(1);
      count = ndays - 1;
      emit zoomViewHorizontally ( newBegin, count );
    }
  } else {
    if ( ndays <= 2 ) {
      newBegin = dateToZoom;
      count = 1;
    } else {
      newBegin = dateToZoom.addDays( -ndays / 2 + 1 );
      count = ndays -1 ;
    }
    emit zoomViewHorizontally ( newBegin, count );
  }
}

void KOAgendaView::zoomOutHorizontally( const QDate &date )
{
  QDate begin;
  QDate newBegin;
  QDate dateToZoom = date;
  int ndays, count;

  begin = mSelectedDates.first();
  ndays = begin.daysTo( mSelectedDates.last() );

  // zoom with Action and are there a selected Incidence?, Yes, I zoom out to it.
  if ( ! dateToZoom.isValid () ) {
    dateToZoom = mAgenda->selectedIncidenceDate();
  }

  if ( !dateToZoom.isValid() ) {
    newBegin = begin.addDays( -1 );
    count = ndays + 3 ;
  } else {
    newBegin = dateToZoom.addDays( -ndays / 2 - 1 );
    count = ndays + 3;
  }

  if ( abs( count ) >= 31 ) {
    kDebug() << "change to the month view?";
  } else {
    //We want to center the date
    emit zoomViewHorizontally( newBegin, count );
  }
}

void KOAgendaView::zoomView( const int delta, const QPoint &pos, const Qt::Orientation orient )
{
  static QDate zoomDate;
  static QTimer *t = new QTimer( this );

  //Zoom to the selected incidence, on the other way
  // zoom to the date on screen after the first mousewheel move.
  if ( orient == Qt::Horizontal ) {
    QDate date=mAgenda->selectedIncidenceDate();
    if ( date.isValid() ) {
      zoomDate=date;
    } else {
      if ( !t->isActive() ) {
        zoomDate= mSelectedDates[pos.x()];
      }
      t->setSingleShot( true );
      t->start ( 1000 );
    }
    if ( delta > 0 ) {
      zoomOutHorizontally( zoomDate );
    } else {
      zoomInHorizontally( zoomDate );
    }
  } else {
    // Vertical zoom
    QPoint posConstentsOld = mAgenda->gridToContents(pos);
    if ( delta > 0 ) {
      zoomOutVertically();
    } else {
      zoomInVertically();
    }
    QPoint posConstentsNew = mAgenda->gridToContents(pos);
    mAgenda->scrollBy( 0, posConstentsNew.y() - posConstentsOld.y() );
  }
}

#ifndef KORG_NODECOS

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

#endif // KORG_NODECOS

void KOAgendaView::createDayLabels()
{
  // ### Before deleting and recreating we could check if mSelectedDates changed...
  // It would remove some flickering and gain speed (since this is called by
  // each updateView() call)
  delete mTopDayLabels;
  delete mBottomDayLabels;

  QFontMetrics fm = fontMetrics();

  mTopDayLabels = new QFrame ( mTopDayLabelsFrame );
  mTopDayLabelsFrame->setStretchFactor( mTopDayLabels, 1 );
  mLayoutTopDayLabels = new QHBoxLayout( mTopDayLabels );
  mLayoutTopDayLabels->setMargin( 0 );
  // this spacer moves the day labels over to line up with the day columns
  QSpacerItem *spacer =
    new QSpacerItem( mTimeLabelsZone->timeLabelsWidth(), 1, QSizePolicy::Fixed );
  mLayoutTopDayLabels->addSpacerItem( spacer );
  KVBox *topWeekLabelBox = new KVBox( mTopDayLabels );
  mLayoutTopDayLabels->addWidget( topWeekLabelBox );
  if ( mIsSideBySide ) {
    topWeekLabelBox->hide();
  }

  mBottomDayLabels = new QFrame( mBottomDayLabelsFrame );
  mBottomDayLabelsFrame->setStretchFactor( mBottomDayLabels, 1 );
  mLayoutBottomDayLabels = new QHBoxLayout( mBottomDayLabels );
  mLayoutBottomDayLabels->setMargin( 0 );
  KVBox *bottomWeekLabelBox = new KVBox( mBottomDayLabels );
  mLayoutBottomDayLabels->addWidget( bottomWeekLabelBox );

  const KCalendarSystem *calsys = KOGlobals::self()->calendarSystem();

#ifndef KORG_NODECOS
  QList<CalendarDecoration::Decoration *> topDecos;
  QStringList topStrDecos = KOPrefs::instance()->decorationsAtAgendaViewTop();
  placeDecorationsFrame( mTopDayLabelsFrame, loadDecorations( topStrDecos, topDecos ), true );

  QList<CalendarDecoration::Decoration *> botDecos;
  QStringList botStrDecos = KOPrefs::instance()->decorationsAtAgendaViewBottom();
  placeDecorationsFrame( mBottomDayLabelsFrame, loadDecorations( botStrDecos, botDecos ), false );
#endif

  DateList::ConstIterator dit;
  for ( dit = mSelectedDates.constBegin(); dit != mSelectedDates.constEnd(); ++dit ) {
    QDate date = *dit;
    KVBox *topDayLabelBox = new KVBox( mTopDayLabels );
    mLayoutTopDayLabels->addWidget( topDayLabelBox );
    KVBox *bottomDayLabelBox = new KVBox( mBottomDayLabels );
    mLayoutBottomDayLabels->addWidget( bottomDayLabelBox );

    int dW = calsys->dayOfWeek( date );
    QString veryLongStr = KGlobal::locale()->formatDate( date );
    QString longstr = i18nc( "short_weekday date (e.g. Mon 13)","%1 %2",
                             calsys->weekDayName( dW, KCalendarSystem::ShortDayName ),
                             calsys->day( date ) );
    QString shortstr = QString::number( calsys->day( date ) );

    KOAlternateLabel *dayLabel =
      new KOAlternateLabel( shortstr, longstr, veryLongStr, topDayLabelBox );
    dayLabel->setMinimumWidth( 1 );
    dayLabel->setAlignment( Qt::AlignHCenter );
    if ( date == QDate::currentDate() ) {
      QFont font = dayLabel->font();
      font.setBold( true );
      dayLabel->setFont( font );
    }

    // if a holiday region is selected, show the holiday name
    QStringList texts = KOGlobals::self()->holiday( date );
    QStringList::ConstIterator textit = texts.constBegin();
    for ( ; textit != texts.constEnd(); ++textit ) {
      // Compute a small version of the holiday string for KOAlternateLabel
      const KWordWrap *ww = KWordWrap::formatText( fm, topDayLabelBox->rect(), 0, (*textit), -1 );
      KOAlternateLabel *label =
        new KOAlternateLabel( ww->truncatedString(), (*textit), (*textit), topDayLabelBox );
      label->setMinimumWidth( 1 );
      label->setAlignment( Qt::AlignCenter );
      delete ww;
    }

#ifndef KORG_NODECOS
    // Day decoration labels
    placeDecorations( topDecos, date, topDayLabelBox, false );
    placeDecorations( botDecos, date, bottomDayLabelBox, false );
#endif
  }

#ifndef KORG_NODECOS
  // Week decoration labels
  placeDecorations( topDecos, mSelectedDates.first(), topWeekLabelBox, true );
  placeDecorations( botDecos, mSelectedDates.first(), bottomWeekLabelBox, true );
#endif

  if ( !mIsSideBySide ) {
    mLayoutTopDayLabels->addSpacing( mAgenda->verticalScrollBar()->width() );
    mLayoutBottomDayLabels->addSpacing( mAgenda->verticalScrollBar()->width() );
  }
  mTopDayLabels->show();
  mBottomDayLabels->show();
}

void KOAgendaView::enableAgendaUpdate( bool enable )
{
  mAllowAgendaUpdate = enable;
}

int KOAgendaView::maxDatesHint()
{
  // Not sure about the max number of events, so return 0 for now.
  return 0;
}

int KOAgendaView::currentDateCount()
{
  return mSelectedDates.count();
}

Akonadi::Item::List KOAgendaView::selectedIncidences()
{
  Akonadi::Item::List selected;

  Akonadi::Item agendaitem = mAgenda->selectedIncidence();
  if ( agendaitem.isValid() ) {
    selected.append( agendaitem );
  }

  Akonadi::Item dayitem = mAllDayAgenda->selectedIncidence();
  if ( dayitem.isValid() ) {
    selected.append( dayitem );
  }

  return selected;
}

DateList KOAgendaView::selectedDates()
{
  DateList selected;
  QDate qd;

  qd = mAgenda->selectedIncidenceDate();
  if ( qd.isValid() ) {
    selected.append( qd );
  }

  qd = mAllDayAgenda->selectedIncidenceDate();
  if ( qd.isValid() ) {
    selected.append( qd );
  }

  return selected;
}

bool KOAgendaView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  if ( selectionStart().isValid() ) {
    QDateTime start = selectionStart();
    QDateTime end = selectionEnd();

    if ( start.secsTo( end ) == 15 * 60 ) {
      // One cell in the agenda view selected, e.g.
      // because of a double-click, => Use the default duration
      QTime defaultDuration( KOPrefs::instance()->mDefaultDuration.time() );
      int addSecs = ( defaultDuration.hour() * 3600 ) + ( defaultDuration.minute() * 60 );
      end = start.addSecs( addSecs );
    }

    startDt = start;
    endDt = end;
    allDay = selectedIsAllDay();
    return true;
  }
  return false;
}

/** returns if only a single cell is selected, or a range of cells */
bool KOAgendaView::selectedIsSingleCell()
{
  if ( !selectionStart().isValid() || !selectionEnd().isValid() ) {
    return false;
  }

  if ( selectedIsAllDay() ) {
    int days = selectionStart().daysTo( selectionEnd() );
    return ( days < 1 );
  } else {
    int secs = selectionStart().secsTo( selectionEnd() );
    return ( secs <= 24 * 60 * 60 / mAgenda->rows() );
  }
}

void KOAgendaView::updateView()
{
  fillAgenda();
}

/*
  Update configuration settings for the agenda view. This method is not
  complete.
*/
void KOAgendaView::updateConfig()
{
  mAgenda->updateConfig();
  mAllDayAgenda->updateConfig();

  mTimeLabelsZone->updateAll();

  updateTimeBarWidth();

  setHolidayMasks();

  createDayLabels();

  updateView();
}

void KOAgendaView::createTimeBarHeaders()
{
  qDeleteAll( mTimeBarHeaders );
  mTimeBarHeaders.clear();

  foreach ( TimeLabels *timeLabel, mTimeLabelsZone->timeLabels() ) {
    QLabel *label = new QLabel( timeLabel->header().replace( '/', "/ " ),
                                mTimeBarHeaderFrame );
    label->setAlignment( Qt::AlignBottom | Qt::AlignLeft );
    label->setMargin( 2 );
    label->setWordWrap( true );
    label->setToolTip( timeLabel->headerToolTip() );
    mTimeBarHeaders.append( label );
  }
}

void KOAgendaView::updateTimeBarWidth()
{
  createTimeBarHeaders();

  QFontMetrics fm( font() );

  int num = 0;
  int width = mTimeLabelsZone->timeLabelsWidth();
  foreach( QLabel *l, mTimeBarHeaders ) {
    num++;
    foreach( const QString &word, l->text().split( ' ' ) ) {
      width = qMax( width, fm.width( word ) );
    }
  }

  if ( num > 0 ) {
    width += ( num * 2 ) + 2;
    if ( num > 1 ) {
      width += ( num * fm.averageCharWidth() );
    }
  }

  mTimeBarHeaderFrame->setFixedWidth( width );
  mTimeLabelsZone->setTimeLabelsWidth( width );
}

void KOAgendaView::updateEventDates( KOAgendaItem *item )
{
  kDebug() << item->text();

  KDateTime startDt, endDt;

  // Start date of this incidence, calculate the offset from it
  // (so recurring and non-recurring items can be treated exactly the same,
  // we never need to check for recurs(), because we only move the start day
  // by the number of days the agenda item was really moved. Smart, isn't it?)
  QDate thisDate;
  if ( item->cellXLeft() < 0 ) {
    thisDate = ( mSelectedDates.first() ).addDays( item->cellXLeft() );
  } else {
    thisDate = mSelectedDates[ item->cellXLeft() ];
  }
  QDate oldThisDate( item->itemDate() );
  int daysOffset = 0;

  // daysOffset should only be calculated if item->cellXLeft() is positive which doesn't happen
  // if the event's start isn't visible.
  if ( item->cellXLeft() >= 0 ) {
    daysOffset = oldThisDate.daysTo( thisDate );
  }

  int daysLength = 0;
//  startDt.setDate( startDate );

  const Item aitem = item->incidence();
  Incidence::Ptr incidence = Akonadi::incidence( aitem );
  if ( !incidence ) {
    return;
  }
  if ( !mChanger || !mChanger->beginChange( aitem ) ) {
    return;
  }
  Incidence::Ptr oldIncidence( incidence->clone() );

  QTime startTime( 0, 0, 0 ), endTime( 0, 0, 0 );
  if ( incidence->allDay() ) {
    daysLength = item->cellWidth() - 1;
  } else {
    startTime = mAgenda->gyToTime( item->cellYTop() );
    if ( item->lastMultiItem() ) {
      endTime = mAgenda->gyToTime( item->lastMultiItem()->cellYBottom() + 1 );
      daysLength = item->lastMultiItem()->cellXLeft() - item->cellXLeft();
    } else {
      endTime = mAgenda->gyToTime( item->cellYBottom() + 1 );
    }
  }

  // FIXME: use a visitor here
  if ( const Event::Ptr ev = Akonadi::event( aitem ) ) {
    startDt = incidence->dtStart();
    // convert to calendar timespec because we then manipulate it
    // with time coming from the calendar
    startDt = startDt.toTimeSpec( KOPrefs::instance()->timeSpec() );
    startDt = startDt.addDays( daysOffset );
    if ( !startDt.isDateOnly() ) {
      startDt.setTime( startTime );
    }
    endDt = startDt.addDays( daysLength );
    if ( !endDt.isDateOnly() ) {
      endDt.setTime( endTime );
    }
    if ( incidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ) == startDt &&
         ev->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ) == endDt ) {
      // No change
      mChanger->endChange( aitem );
      QTimer::singleShot( 0, this, SLOT(updateView()) );
      return;
    }
  } else if ( const Todo::Ptr td = Akonadi::todo( aitem ) ) {
    startDt = td->hasStartDate() ? td->dtStart() : td->dtDue();
    // convert to calendar timespec because we then manipulate it with time coming from
    // the calendar
    startDt = startDt.toTimeSpec( KOPrefs::instance()->timeSpec() );
    startDt.setDate( thisDate.addDays( td->dtDue().daysTo( startDt ) ) );
    if ( !startDt.isDateOnly() ) {
      startDt.setTime( startTime );
    }

    endDt = startDt;
    endDt.setDate( thisDate );
    if ( !endDt.isDateOnly() ) {
      endDt.setTime( endTime );
    }

    if( td->dtDue().toTimeSpec( KOPrefs::instance()->timeSpec() )  == endDt ) {
      // No change
      mChanger->endChange( aitem );
      QTimer::singleShot( 0, this, SLOT(updateView()) );
      return;
    }
  }
  // FIXME: Adjusting the recurrence should really go to CalendarView so this
  // functionality will also be available in other views!
  // TODO_Recurrence: This does not belong here, and I'm not really sure
  // how it's supposed to work anyway.
/*
  Recurrence *recur = incidence->recurrence();
  if ( recur->recurs() && daysOffset != 0 ) {
    switch ( recur->recurrenceType() ) {
    case Recurrence::rYearlyPos:
    {
      int freq = recur->frequency();
      int duration = recur->duration();
      QDate endDt( recur->endDate() );
      bool negative = false;

      QPtrList<Recurrence::rMonthPos> monthPos( recur->yearMonthPositions() );
      if ( monthPos.first() ) {
        negative = monthPos.first()->negative;
      }
      QBitArray days( 7 );
      int pos = 0;
      days.fill( false );
      days.setBit( thisDate.dayOfWeek() - 1 );
      if ( negative ) {
        pos =  - ( thisDate.daysInMonth() - thisDate.day() - 1 ) / 7 - 1;
      } else {
        pos =  ( thisDate.day()-1 ) / 7 + 1;
      }
      // Terrible hack: to change the month days,
      // I have to unset the recurrence, and set all days manually again
      recur->unsetRecurs();
      if ( duration != 0 ) {
        recur->setYearly( Recurrence::rYearlyPos, freq, duration );
      } else {
        recur->setYearly( Recurrence::rYearlyPos, freq, endDt );
      }
      recur->addYearlyMonthPos( pos, days );
      recur->addYearlyNum( thisDate.month() );

      break;
    }
    case Recurrence::rYearlyDay:
    {
      int freq = recur->frequency();
      int duration = recur->duration();
      QDate endDt( recur->endDate() );
      // Terrible hack: to change the month days,
      // I have to unset the recurrence, and set all days manually again
      recur->unsetRecurs();
      if ( duration == 0 ) { // end by date
        recur->setYearly( Recurrence::rYearlyDay, freq, endDt );
      } else {
        recur->setYearly( Recurrence::rYearlyDay, freq, duration );
      }
      recur->addYearlyNum( thisDate.dayOfYear() );
      break;
    }
    case Recurrence::rYearlyMonth:
    {
      int freq = recur->frequency();
      int duration = recur->duration();
      QDate endDt( recur->endDate() );
      // Terrible hack: to change the month days,
      // I have to unset the recurrence, and set all days manually again
      recur->unsetRecurs();
      if ( duration != 0 ) {
        recur->setYearlyByDate( thisDate.day(), recur->feb29YearlyType(), freq, duration );
      } else {
        recur->setYearlyByDate( thisDate.day(), recur->feb29YearlyType(), freq, endDt );
      }
      recur->addYearlyNum( thisDate.month() );
      break; }
    case Recurrence::rMonthlyPos:
    {
      int freq = recur->frequency();
      int duration = recur->duration();
      QDate endDt( recur->endDate() );
      QPtrList<Recurrence::rMonthPos> monthPos( recur->monthPositions() );
      if ( !monthPos.isEmpty() ) {
        // FIXME: How shall I adapt the day x of week Y if we move the
        // date across month borders??? for now, just use the date of
        // the moved item and assume the recurrence only occurs on that day.
        // That's fine for korganizer, but might mess up other organizers.
        QBitArray rDays( 7 );
        rDays = monthPos.first()->rDays;
        bool negative = monthPos.first()->negative;
        int newPos;
        rDays.fill( false );
        rDays.setBit( thisDate.dayOfWeek() - 1 );
        if ( negative ) {
          newPos =  - ( thisDate.daysInMonth() - thisDate.day() - 1 ) / 7 - 1;
        } else {
          newPos =  ( thisDate.day()-1 ) / 7 + 1;
        }

        // Terrible hack: to change the month days,
        // I have to unset the recurrence, and set all days manually again
        recur->unsetRecurs();
        if ( duration == 0 ) { // end by date
          recur->setMonthly( Recurrence::rMonthlyPos, freq, endDt );
        } else {
          recur->setMonthly( Recurrence::rMonthlyPos, freq, duration );
        }
        recur->addMonthlyPos( newPos, rDays );
      }
      break;
    }
    case Recurrence::rMonthlyDay:
    {
      int freq = recur->frequency();
      int duration = recur->duration();
      QDate endDt( recur->endDate() );
      QPtrList<int> monthDays( recur->monthDays() );
      // Terrible hack: to change the month days,
      // I have to unset the recurrence, and set all days manually again
      recur->unsetRecurs();
      if ( duration == 0 ) { // end by date
        recur->setMonthly( Recurrence::rMonthlyDay, freq, endDt );
      } else {
        recur->setMonthly( Recurrence::rMonthlyDay, freq, duration );
      }
      // FIXME: How shall I adapt the n-th day if we move the date across
      // month borders??? for now, just use the date of the moved item and
      // assume the recurrence only occurs on that day.
      // That's fine for korganizer, but might mess up other organizers.
      recur->addMonthlyDay( thisDate.day() );

      break;
    }
    case Recurrence::rWeekly:
    {
      QBitArray days(7), oldDays( recur->days() );
      int offset = daysOffset % 7;
      if ( offset < 0 ) {
        offset = ( offset + 7 ) % 7;
      }
      // rotate the days
      for ( int d=0; d<7; d++ ) {
        days.setBit( ( d + offset ) % 7, oldDays.at( d ) );
      }
      if ( recur->duration() == 0 ) { // end by date
        recur->setWeekly( recur->frequency(), days, recur->endDate(), recur->weekStart() );
      } else { // duration or no end
        recur->setWeekly( recur->frequency(), days, recur->duration(), recur->weekStart() );
      }
      break;
    }
      // nothing to be done for the following:
    case Recurrence::rDaily:
    case Recurrence::rHourly:
    case Recurrence::rMinutely:
    case Recurrence::rNone:
    default:
      break;
    }
    if ( recur->duration() == 0 ) { // end by date
      recur->setEndDate( recur->endDate().addDays( daysOffset ) );
    }
    KMessageBox::information( this,
                              i18n( "A recurring calendar item was moved to a "
                                    "different day. The recurrence settings "
                                    "have been updated with that move. Please "
                                    "check them in the editor." ),
                              i18n( "Recurrence Moved" ),
                              "RecurrenceMoveInAgendaWarning" );
  }
*/

  // FIXME: use a visitor here
  if ( const Event::Ptr ev = Akonadi::event( aitem ) ) {
    /* setDtEnd() must be called before setDtStart(), otherwise, when moving
     * events, CalendarLocal::incidenceUpdated() will not remove the old hash
     * and that causes the event to be shown in the old date also (bug #179157).
     *
     * TODO: We need a better hashing mechanism for CalendarLocal.
     */
    ev->setDtEnd(
      endDt.toTimeSpec( incidence->dtEnd().timeSpec() ) );
    incidence->setDtStart( startDt.toTimeSpec( incidence->dtStart().timeSpec() ) );
  } else if ( const Todo::Ptr td = Akonadi::todo( aitem ) ) {
    if ( td->hasStartDate() ) {
      td->setDtStart( startDt.toTimeSpec( incidence->dtStart().timeSpec() ) );
    }
    td->setDtDue( endDt.toTimeSpec( td->dtDue().timeSpec() ) );
  }
  item->setItemDate( startDt.toTimeSpec( KOPrefs::instance()->timeSpec() ).date() );

  const bool result = mChanger->changeIncidence( oldIncidence, aitem,
                                                 KOGlobals::DATE_MODIFIED, this );
  mChanger->endChange( aitem );

  // Update the view correctly if an agenda item move was aborted by
  // cancelling one of the subsequent dialogs.
  if ( !result ) {
    mPendingChanges = true;
    QTimer::singleShot( 0, this, SLOT(updateView()) );
    return;
  }

  // don't update the agenda as the item already has the correct coordinates.
  // an update would delete the current item and recreate it, but we are still
  // using a pointer to that item! => CRASH
  enableAgendaUpdate( false );
  // We need to do this in a timer to make sure we are not deleting the item
  // we are currently working on, which would lead to crashes
  // Only the actually moved agenda item is already at the correct position and mustn't be
  // recreated. All others have to!!!
  if ( incidence->recurs() ) {
    mUpdateItem = aitem;
    QTimer::singleShot( 0, this, SLOT(doUpdateItem()) );
  }

  enableAgendaUpdate( true );
}

void KOAgendaView::doUpdateItem()
{
  if ( Akonadi::hasIncidence( mUpdateItem ) ) {
    changeIncidenceDisplay( mUpdateItem, KOGlobals::INCIDENCEEDITED );
    mUpdateItem = Item();
  }
}

void KOAgendaView::showDates( const QDate &start, const QDate &end )
{
  if ( !mSelectedDates.isEmpty() &&
       mSelectedDates.first() == start &&
       mSelectedDates.last() == end &&
       !mPendingChanges ) {
    return;
  }

  mSelectedDates.clear();

  QDate d = start;
  while ( d <= end ) {
    mSelectedDates.append( d );
    d = d.addDays( 1 );
  }

  // and update the view
  fillAgenda();
}

void KOAgendaView::showIncidences( const Item::List &incidences, const QDate &date )
{
  Q_UNUSED( date );

  // we must check if they are not filtered; if they are, remove the filter
  CalFilter *filter = calendar()->filter();
  bool wehaveall = true;
  if ( filter ) {
    Q_FOREACH( const Item& aitem, incidences ){
      if ( !( wehaveall = filter->filterIncidence( Akonadi::incidence( aitem ).get() ) ) ) {
        break;
      }
    }
  }

  if ( !wehaveall ) {
    calendar()->setFilter( 0 );
  }

  KDateTime start = Akonadi::incidence( incidences.first() )->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ),
            end = Akonadi::incidence( incidences.first() )->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() );
  Item first = incidences.first();
  Q_FOREACH( const Item &aitem, incidences ) {
    if ( Akonadi::incidence( aitem )->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ) < start ) {
      first = aitem;
    }
    start = qMin( start, Akonadi::incidence( aitem )->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ) );
    end = qMax( start, Akonadi::incidence( aitem )->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ) );
  }

  end.toTimeSpec( start );    // allow direct comparison of dates
  if ( start.date().daysTo( end.date() ) + 1 <= currentDateCount() ) {
    showDates( start.date(), end.date() );
  } else {
    showDates( start.date(), start.date().addDays( currentDateCount() - 1 ) );
  }

  mAgenda->selectItem( first );
}

void KOAgendaView::insertIncidence( const Item &aitem, const QDate &curDate )
{
  if ( !filterByCollectionSelection( aitem ) ) {
    return;
  }

  // FIXME: Use a visitor here, or some other method to get rid of the dynamic_cast's
  Incidence::Ptr incidence = Akonadi::incidence( aitem );
  Event::Ptr event = Akonadi::event( aitem );
  Todo::Ptr todo = Akonadi::todo( aitem );

  int curCol = mSelectedDates.first().daysTo( curDate );

  // In case incidence->dtStart() isn't visible (crosses bounderies)
  if ( curCol < 0 ) {
    curCol = 0;
  }

  // The date for the event is not displayed, just ignore it
  if ( curCol >= mSelectedDates.count() ) {
    return;
  }

  // Default values, which can never be reached
  mMinY[curCol] = mAgenda->timeToY( QTime( 23, 59 ) ) + 1;
  mMaxY[curCol] = mAgenda->timeToY( QTime( 0, 0 ) ) - 1;

  int beginX;
  int endX;
  QDate columnDate;
  if ( event ) {
    QDate firstVisibleDate = mSelectedDates.first();
    // its crossing bounderies, lets calculate beginX and endX
    if ( curDate < firstVisibleDate ) {
      beginX = curCol + firstVisibleDate.daysTo( curDate );
      endX   = beginX + event->dtStart().daysTo( event->dtEnd() );
      columnDate = firstVisibleDate;
    } else {
      beginX = curCol;
      endX   = beginX + event->dtStart().daysTo( event->dtEnd() );
      columnDate = curDate;
    }
  } else if ( todo ) {
    if ( !todo->hasDueDate() ) {
      return;  // todo shall not be displayed if it has no date
    }
    columnDate = curDate;
    beginX = endX = curCol;

  } else {
    return;
  }
  if ( todo && todo->isOverdue() ) {
    mAllDayAgenda->insertAllDayItem( aitem, columnDate, curCol, curCol );
  } else if ( incidence->allDay() ) {
      mAllDayAgenda->insertAllDayItem( aitem, columnDate, beginX, endX );
  } else if ( event && event->isMultiDay( KOPrefs::instance()->timeSpec() ) ) {
    int startY = mAgenda->timeToY(
      event->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).time() );
    QTime endtime( event->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ).time() );
    if ( endtime == QTime( 0, 0, 0 ) ) {
      endtime = QTime( 23, 59, 59 );
    }
    int endY = mAgenda->timeToY( endtime ) - 1;
    if ( ( beginX <= 0 && curCol == 0 ) || beginX == curCol ) {
      mAgenda->insertMultiItem( aitem, columnDate, beginX, endX, startY, endY );

    }
    if ( beginX == curCol ) {
      mMaxY[curCol] = mAgenda->timeToY( QTime( 23, 59 ) );
      if ( startY < mMinY[curCol] ) {
        mMinY[curCol] = startY;
      }
    } else if ( endX == curCol ) {
      mMinY[curCol] = mAgenda->timeToY( QTime( 0, 0 ) );
      if ( endY > mMaxY[curCol] ) {
        mMaxY[curCol] = endY;
      }
    } else {
      mMinY[curCol] = mAgenda->timeToY( QTime( 0, 0 ) );
      mMaxY[curCol] = mAgenda->timeToY( QTime( 23, 59 ) );
    }
  } else {
    int startY = 0, endY = 0;
    if ( event ) {
      startY = mAgenda->timeToY(
        incidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).time() );
      QTime endtime( event->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ).time() );
      if ( endtime == QTime( 0, 0, 0 ) ) {
        endtime = QTime( 23, 59, 59 );
      }
      endY = mAgenda->timeToY( endtime ) - 1;
    }
    if ( todo ) {
      QTime t = todo->dtDue().toTimeSpec( KOPrefs::instance()->timeSpec() ).time();

      int halfHour = 1800;
      if ( t.addSecs( -halfHour ) < t ) {
        startY = mAgenda->timeToY( t.addSecs( -halfHour ) );
        endY   = mAgenda->timeToY( t ) - 1;
      } else {
        startY = 0;
        endY   = mAgenda->timeToY( t.addSecs( halfHour ) ) - 1;
      }
    }
    if ( endY < startY ) {
      endY = startY;
    }
    mAgenda->insertItem( aitem, columnDate, curCol, startY, endY );
    if ( startY < mMinY[curCol] ) {
      mMinY[curCol] = startY;
    }
    if ( endY > mMaxY[curCol] ) {
      mMaxY[curCol] = endY;
    }
  }
}

void KOAgendaView::changeIncidenceDisplayAdded( const Item &aitem )
{
  Todo::Ptr todo = Akonadi::todo( aitem );
  CalFilter *filter = calendar()->filter();
  if ( ( filter && !filter->filterIncidence( Akonadi::incidence( aitem ).get() ) ) ||
       ( ( todo && !KOPrefs::instance()->showTodosAgendaView() ) ) ) {
    return;
  }

  displayIncidence( aitem );
}

void KOAgendaView::changeIncidenceDisplay( const Item &aitem, int mode )
{
  switch ( mode ) {
    case KOGlobals::INCIDENCEADDED:
    {
      // Add an event. No need to recreate the whole view!
      // recreating everything even causes troubles: dropping to the
      // day matrix recreates the agenda items, but the evaluation is
      // still in an agendaItems' code, which was deleted in the mean time.
      // Thus KOrg crashes...
      changeIncidenceDisplayAdded( aitem );
      updateEventIndicators();
      break;
    }
    case KOGlobals::INCIDENCEEDITED:
    {
      if ( mAllowAgendaUpdate ) {
        //PENDING(AKONADI_PORT) try harder not to recreate the items here, this causes flicker with the delayed notification from Akonadi, after a dnd operation
        removeIncidence( aitem );
        changeIncidenceDisplayAdded( aitem );
      }
      updateEventIndicators();
      break;
    }
    case KOGlobals::INCIDENCEDELETED:
    {
      mAgenda->removeIncidence( aitem );
      mAllDayAgenda->removeIncidence( aitem );
      updateEventIndicators();
      break;
    }
    default:
      updateView();
  }
}

void KOAgendaView::fillAgenda( const QDate & )
{
  fillAgenda();
}

void KOAgendaView::fillAgenda()
{
  mPendingChanges = false;

  /* Remember the item Ids of the selected items. In case one of the
   * items was deleted and re-added, we want to reselect it. */
  const Item::Id selectedAgendaId = mAgenda->lastSelectedItemId();
  const Item::Id selectedAllDayAgendaId = mAllDayAgenda->lastSelectedItemId();

  enableAgendaUpdate( true );
  clearView();

  mAllDayAgenda->changeColumns( mSelectedDates.count() );
  mAgenda->changeColumns( mSelectedDates.count() );
  mEventIndicatorTop->changeColumns( mSelectedDates.count() );
  mEventIndicatorBottom->changeColumns( mSelectedDates.count() );

  createDayLabels();
  setHolidayMasks();

  mMinY.resize( mSelectedDates.count() );
  mMaxY.resize( mSelectedDates.count() );

  mAgenda->setDateList( mSelectedDates );

  bool somethingReselected = false;
  const Item::List incidences = calendar()->incidences();

  foreach ( const Item& aitem, incidences ) {
    displayIncidence( aitem );
    if( aitem.id() == selectedAgendaId ) {
      mAgenda->selectItem( aitem );
      somethingReselected = true;
    }

    if( aitem.id() == selectedAllDayAgendaId ) {
      mAllDayAgenda->selectItem( aitem );
      somethingReselected = true;
    }
  }

  mAgenda->checkScrollBoundaries();
  updateEventIndicators();

  //  mAgenda->viewport()->update();
  //  mAllDayAgenda->viewport()->update();

  // make invalid
  deleteSelectedDateTime();

  if( !somethingReselected ) {
    emit incidenceSelected( Item(), QDate() );
  }
}

void KOAgendaView::displayIncidence( const Item &aitem )
{
  QDate today = QDate::currentDate();
  DateTimeList::iterator t;

  // FIXME: use a visitor here
  Incidence::Ptr incidence = Akonadi::incidence( aitem );
  Todo::Ptr todo = Akonadi::todo( aitem );
  Event::Ptr event = Akonadi::event( aitem );

  KDateTime firstVisibleDateTime( mSelectedDates.first(), KOPrefs::instance()->timeSpec() );
  KDateTime lastVisibleDateTime( mSelectedDates.last(), KOPrefs::instance()->timeSpec() );

  lastVisibleDateTime.setTime( QTime( 23, 59 ) );
  firstVisibleDateTime.setTime( QTime( 0, 0 ) );
  DateTimeList dateTimeList;

  KDateTime incDtStart = incidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() );
  KDateTime incDtEnd   = incidence->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() );

  if ( todo &&
       ( !KOPrefs::instance()->showTodosAgendaView() || !todo->hasDueDate() ) ) {
    return;
  }

  if ( incidence->recurs() ) {
    int eventDuration = incDtStart.daysTo( incDtEnd );

    // if there's a multiday event that starts before firstVisibleDateTime but ends after
    // lets include it. timesInInterval() ignores incidences that aren't totaly inside
    // the range
    KDateTime startDateTimeWithOffset = firstVisibleDateTime.addDays( -eventDuration );
    dateTimeList =
      incidence->recurrence()->timesInInterval( startDateTimeWithOffset,
                                                lastVisibleDateTime );
  } else {
    KDateTime dateToAdd; // date to add to our date list
    KDateTime incidenceStart;
    KDateTime incidenceEnd;

    if ( todo && todo->hasDueDate() && !todo->isOverdue() ) {
      // If it's not overdue it will be shown at the original date (not today)
      dateToAdd = todo->dtDue().toTimeSpec( KOPrefs::instance()->timeSpec() );
      incidenceEnd = dateToAdd;
    } else if ( event ) {
      dateToAdd = incDtStart;
      incidenceEnd = incDtEnd;

      if ( dateToAdd.isDateOnly() ) {
        // so comparisons with < > actually work
        dateToAdd.setTime( QTime( 0, 0 ) );
      }
    }

    if  ( dateToAdd <= lastVisibleDateTime && incidenceEnd >= firstVisibleDateTime ) {
      dateTimeList += dateToAdd;
    }
  }

  // ToDo items shall be displayed today if they are already overdude
  KDateTime dateTimeToday = KDateTime( today, KOPrefs::instance()->timeSpec() );
  if ( todo &&
       todo->isOverdue() &&
       dateTimeToday >= firstVisibleDateTime &&
       dateTimeToday <= lastVisibleDateTime ) {

    bool doAdd = true;

    if ( todo->recurs() ) {
      /* If there's a recurring instance showing up today don't add "today" again
       * we don't want the event to appear duplicated */
      for ( t = dateTimeList.begin(); t != dateTimeList.end(); ++t ) {
        if ( t->toTimeSpec( KOPrefs::instance()->timeSpec() ).date() == today ) {
          doAdd = false;
          break;
        }
      }
    }

    if ( doAdd ) {
      dateTimeList += dateTimeToday;
    }
  }

  for ( t = dateTimeList.begin(); t != dateTimeList.end(); ++t ) {
    insertIncidence( aitem, t->toTimeSpec( KOPrefs::instance()->timeSpec() ).date() );
  }
}

void KOAgendaView::clearView()
{
  mAllDayAgenda->clear();
  mAgenda->clear();
}

CalPrinter::PrintType KOAgendaView::printType()
{
  if ( currentDateCount() == 1 ) {
    return CalPrinter::Day;
  } else {
    return CalPrinter::Week;
  }
}

void KOAgendaView::updateEventIndicatorTop( int newY )
{
  for ( int i = 0; i < mMinY.size(); ++i ) {
    mEventIndicatorTop->enableColumn( i, newY > mMinY[i] );
  }
  mEventIndicatorTop->update();
}

void KOAgendaView::updateEventIndicatorBottom( int newY )
{
  for ( int i = 0; i < mMaxY.size(); ++i ) {
    mEventIndicatorBottom->enableColumn( i, newY <= mMaxY[i] );
  }
  mEventIndicatorBottom->update();
}

void KOAgendaView::slotTodosDropped( const QList<KUrl> &items, const QPoint &gpos, bool allDay ) {
#ifdef AKONADI_PORT_DISABLED // one item -> multiple items, Incidence* -> akonadi item url (we might have to fetch the items here first!)
  if ( gpos.x() < 0 || gpos.y() < 0 ) {
    return;
  }

  QDate day = mSelectedDates[gpos.x()];
  QTime time = mAgenda->gyToTime( gpos.y() );
  KDateTime newTime( day, time, KOPrefs::instance()->timeSpec() );
  newTime.setDateOnly( allDay );

  Todo::Ptr todo = Akonadi::todo( todoItem );
  if ( todo &&  dynamic_cast<Akonadi::Calendar*>( calendar() ) ) {
    const Item existingTodoItem = dynamic_cast<Akonadi::Calendar*>( calendar() )->itemForIncidence( calendar()->todo( todo->uid() ) );
    if ( Todo::Ptr existingTodo = Akonadi::todo( existingTodoItem ) ) {
      kDebug() << "Drop existing Todo";
      Todo::Ptr oldTodo( existingTodo->clone() );
      if ( mChanger && mChanger->beginChange( existingTodoItem ) ) {
        existingTodo->setDtDue( newTime );
        existingTodo->setAllDay( allDay );
        existingTodo->setHasDueDate( true );
        mChanger->changeIncidence( oldTodo, existingTodoItem, KOGlobals::DATE_MODIFIED, this );
        mChanger->endChange( existingTodoItem );
      } else {
        KMessageBox::sorry( this, i18n( "Unable to modify this to-do, "
                                        "because it cannot be locked." ) );
      }
    } else {
      kDebug() << "Drop new Todo";
      todo->setDtDue( newTime );
      todo->setAllDay( allDay );
      todo->setHasDueDate( true );
      if ( !mChanger->addIncidence( todo, this ) ) {
        KODialogManager::errorSaveIncidence( this, todo );
      }
    }
  }
#else
  kWarning()<<"TODO";
#endif
}

void KOAgendaView::slotTodosDropped( const QList<Todo::Ptr> &items, const QPoint &gpos, bool allDay )
{
  if ( gpos.x() < 0 || gpos.y() < 0 ) {
    return;
  }

  QDate day = mSelectedDates[gpos.x()];
  QTime time = mAgenda->gyToTime( gpos.y() );
  KDateTime newTime( day, time, KOPrefs::instance()->timeSpec() );
  newTime.setDateOnly( allDay );

  Q_FOREACH( const Todo::Ptr &todo, items ) {
    todo->setDtDue( newTime );
    todo->setAllDay( allDay );
    todo->setHasDueDate( true );
    if ( !mChanger->addIncidence( todo, this ) ) {
      KODialogManager::errorSaveIncidence( this, todo );
    }
  }
}
void KOAgendaView::startDrag( const Item &incidence )
{
  if ( QDrag *drag = Akonadi::createDrag( incidence, calendar()->timeSpec(), this ) )
    drag->exec();
}

void KOAgendaView::readSettings()
{
  readSettings( KOGlobals::self()->config() );
}

void KOAgendaView::readSettings( KConfig *config )
{
  KConfigGroup group = config->group( "Views" );

  QList<int> sizes = group.readEntry( "Separator AgendaView", QList<int>() );

  // the size depends on the number of plugins used
  // we don't want to read invalid/corrupted settings or else agenda becomes invisible
  if ( sizes.count() >= 2 && !sizes.contains( 0 ) ) {
      mSplitterAgenda->setSizes( sizes );
  }

  updateConfig();
}

void KOAgendaView::writeSettings( KConfig *config )
{
  KConfigGroup group = config->group( "Views" );

  QList<int> list = mSplitterAgenda->sizes();
  group.writeEntry( "Separator AgendaView", list );
}

void KOAgendaView::setHolidayMasks()
{
  if ( mSelectedDates.isEmpty() || !mSelectedDates[0].isValid() ) {
    return;
  }

  mHolidayMask.resize( mSelectedDates.count() + 1 );

  for ( int i = 0; i < mSelectedDates.count(); ++i ) {
    mHolidayMask[i] = !KOGlobals::self()->isWorkDay( mSelectedDates[ i ] );
  }

  // Store the information about the day before the visible area (needed for
  // overnight working hours) in the last bit of the mask:
  bool showDay = !KOGlobals::self()->isWorkDay( mSelectedDates[ 0 ].addDays( -1 ) );
  mHolidayMask[ mSelectedDates.count() ] = showDay;

  mAgenda->setHolidayMask( &mHolidayMask );
  mAllDayAgenda->setHolidayMask( &mHolidayMask );
}

void KOAgendaView::setContentsPos( int y )
{
  if ( y != mAgenda->contentsY() ) {
    mAgenda->setContentsPos( 0, y );
  }
}

void KOAgendaView::clearSelection()
{
  mAgenda->deselectItem();
  mAllDayAgenda->deselectItem();
}

void KOAgendaView::newTimeSpanSelectedAllDay( const QPoint &start, const QPoint &end )
{
  newTimeSpanSelected( start, end );
  mTimeSpanInAllDay = true;
}

void KOAgendaView::handleNewEventRequest()
{
  emit newEventSignal( Akonadi::Collection::List() << Collection( collection() ) );
}

void KOAgendaView::newTimeSpanSelected( const QPoint &start, const QPoint &end )
{
  if ( !mSelectedDates.count() ) {
    return;
  }

  mTimeSpanInAllDay = false;

  QDate dayStart = mSelectedDates[ qBound( 0, start.x(), (int)mSelectedDates.size() - 1 ) ];
  QDate dayEnd = mSelectedDates[ qBound( 0, end.x(), (int)mSelectedDates.size() - 1 ) ];

  QTime timeStart = mAgenda->gyToTime( start.y() );
  QTime timeEnd = mAgenda->gyToTime( end.y() + 1 );

  QDateTime dtStart( dayStart, timeStart );
  QDateTime dtEnd( dayEnd, timeEnd );

  mTimeSpanBegin = dtStart;
  mTimeSpanEnd = dtEnd;
}

void KOAgendaView::deleteSelectedDateTime()
{
  mTimeSpanBegin.setDate( QDate() );
  mTimeSpanEnd.setDate( QDate() );
  mTimeSpanInAllDay = false;
}

void KOAgendaView::removeIncidence( const Item &incidence )
{
  mAgenda->removeIncidence( incidence );
  mAllDayAgenda->removeIncidence( incidence );
}

void KOAgendaView::updateEventIndicators()
{
  mMinY = mAgenda->minContentsY();
  mMaxY = mAgenda->maxContentsY();

  mAgenda->checkScrollBoundaries();
  updateEventIndicatorTop( mAgenda->visibleContentsYMin() );
  updateEventIndicatorBottom( mAgenda->visibleContentsYMax() );
}

void KOAgendaView::setIncidenceChanger( IncidenceChangerBase *changer )
{
  mChanger = changer;
  mAgenda->setIncidenceChanger( changer );
  mAllDayAgenda->setIncidenceChanger( changer );
}

void KOAgendaView::clearTimeSpanSelection()
{
  mAgenda->clearSelection();
  mAllDayAgenda->clearSelection();
  deleteSelectedDateTime();
}

#if 0
void KOAgendaView::setCollectionSelection( CollectionSelection* sel )
{
  if ( mCollectionSelection == sel )
    return;
  mCollectionSelection = sel;
}
#endif

void KOAgendaView::setCollection( Collection::Id coll )
{
  if ( mCollectionId == coll )
    return;
  mCollectionId = coll;
}

Akonadi::Collection::Id KOAgendaView::collection() const
{
  return mCollectionId;
}

bool KOAgendaView::filterByCollectionSelection( const Item &incidence )
{
  if ( customCollectionSelection() ) {
    return customCollectionSelection()->contains( incidence.parentCollection().id() );
  }
  if ( mCollectionId < 0 )
    return true;
  else
    return mCollectionId == incidence.storageCollectionId();
}

void KOAgendaView::setUpdateNeeded()
{
  mPendingChanges = true;
}

void KOAgendaView::calendarIncidenceAdded( const Item &incidence )
{
  Q_UNUSED( incidence );
  mPendingChanges = true;
}

void KOAgendaView::calendarIncidenceChanged( const Item &incidence )
{
  Q_UNUSED( incidence );
  mPendingChanges = true;
}

void KOAgendaView::calendarIncidenceRemoved( const Item &incidence )
{
  Q_UNUSED( incidence );
  mPendingChanges = true;
}

#include "koagendaview.moc"
