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
#include "koglobals.h"
#ifndef KORG_NOPLUGINS
#include "kocore.h"
#include "kodecorationlabel.h"
#endif
#include "koprefs.h"
#include "koagenda.h"
#include "koagendaitem.h"
#include "kogroupware.h"
#include "kodialogmanager.h"
#include "koeventpopupmenu.h"
#include "koalternatelabel.h"
#include "timelabelszone.h"

#include <kcal/calendar.h>
#include <kcal/icaldrag.h>
#include <kcal/dndfactory.h>
#include <kcal/calfilter.h>
#include <kcal/incidence.h>
#include <kcal/incidenceformatter.h>

#include <kapplication.h>
#include <kcalendarsystem.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kholidays.h>
#include <kvbox.h>
#include <ksystemtimezone.h>
#include <kpushbutton.h>
#include <kcombobox.h>
#include <kwordwrap.h>

#include <QLabel>
#include <QFrame>
#include <QLayout>
#include <QSplitter>
#include <QFont>
#include <QFontMetrics>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QToolButton>
#include <QCursor>
#include <QBitArray>
#include <QPaintEvent>
#include <QGridLayout>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QListWidget>

using namespace KOrg;

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

KOAgendaView::KOAgendaView( Calendar *cal, QWidget *parent, bool isSideBySide ) :
  KOrg::AgendaView( cal, parent ),
  mTimeLabelsZone( 0 ),
  mAllowAgendaUpdate( true ),
  mUpdateItem( 0 ),
  mResource( 0 ),
  mIsSideBySide( isSideBySide ),
  mPendingChanges( true )
{
  mSelectedDates.append( QDate::currentDate() );

  mLayoutDayLabels = 0;
  mDayLabelsFrame = 0;
  mDayLabels = 0;
  mLayoutBottomDayLabels = 0;
  mBottomDayLabelsFrame = 0;
  mBottomDayLabels = 0;

  mTopLayout = new QGridLayout( this );
  mTopLayout->setMargin( 0 );

  /* Create agenda splitter */
  mSplitterAgenda = new QSplitter( Qt::Vertical, this );
  mTopLayout->addWidget( mSplitterAgenda, 1, 0 );
  mSplitterAgenda->setOpaqueResize( KGlobalSettings::opaqueResize() );

  /* Create day name labels for agenda columns */
  mDayLabelsFrame = new KHBox( mSplitterAgenda );
  mDayLabelsFrame->setSpacing( 2 );

  /* Create all-day agenda widget */
  mAllDayFrame = new KHBox( mSplitterAgenda );
  mAllDayFrame->setSpacing( 2 );

  // Alignment and description widgets
  mTimeBarHeaderFrame = new KHBox( mAllDayFrame );

  // The widget itself
  QWidget *dummyAllDayLeft = new QWidget( mAllDayFrame );
  mAllDayAgenda = new KOAgenda( 1, mAllDayFrame );
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
  mAgenda = new KOAgenda( 1, 96, KOPrefs::instance()->mHourSize, agendaFrame );
  mAgendaLayout->addWidget( mAgenda, 1, 1, 1, 2 );
  mAgendaLayout->setColumnStretch( 1, 1 );

  // Create time labels
  mTimeLabelsZone = new TimeLabelsZone( this, mAgenda );
  mAgendaLayout->addWidget( mTimeLabelsZone, 1, 0 );

  // Create event context menu for agenda
  mAgendaPopup = eventPopup();

  // Scrolling
  connect( mAgenda, SIGNAL(zoomView(const int,const QPoint &,const Qt::Orientation)),
           SLOT(zoomView(const int,const QPoint &,const Qt::Orientation)) );

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

  if ( cal ) {
    cal->registerObserver( this );
  }
}

KOAgendaView::~KOAgendaView()
{
  if ( calendar() ) {
    calendar()->unregisterObserver( this );
  }

  delete mAgendaPopup;
  delete mAllDayAgendaPopup;
}

void KOAgendaView::connectAgenda( KOAgenda *agenda, QMenu *popup,
                                  KOAgenda *otherAgenda )
{
  connect( agenda, SIGNAL(showIncidencePopupSignal(Calendar *,Incidence *,const QDate &)),
           popup, SLOT(showIncidencePopup(Calendar *,Incidence *,const QDate &)) );

  connect( agenda, SIGNAL(showNewEventPopupSignal()),
           SLOT(showNewEventPopup()) );

  agenda->setCalendar( calendar() );

  // Create/Show/Edit/Delete Event
  connect( agenda, SIGNAL(newEventSignal()), SIGNAL(newEventSignal()) );

  connect( agenda, SIGNAL(newStartSelectSignal()),
           otherAgenda, SLOT(clearSelection()) );
  connect( agenda, SIGNAL(newStartSelectSignal()),
           SIGNAL(timeSpanSelectionChanged()) );

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

  connect( agenda, SIGNAL(itemModified(KOAgendaItem *)),
                   SLOT(updateEventDates(KOAgendaItem *)) );
  connect( agenda, SIGNAL(enableAgendaUpdate(bool)),
                   SLOT(enableAgendaUpdate(bool)) );

  // drag signals
  connect( agenda, SIGNAL(startDragSignal(Incidence *)),
           SLOT(startDrag(Incidence *)) );

  // synchronize selections
  connect( agenda, SIGNAL(incidenceSelected(Incidence *)),
           otherAgenda, SLOT(deselectItem()) );
  connect( agenda, SIGNAL(incidenceSelected(Incidence *)),
           SIGNAL(incidenceSelected(Incidence *)) );

  // rescheduling of todos by d'n'd
  connect( agenda, SIGNAL(droppedToDo(Todo *,const QPoint &,bool)),
           SLOT(slotTodoDropped(Todo *,const QPoint &,bool)) );

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

void KOAgendaView::createDayLabels()
{
  // ### Before deleting and recreating we could check if mSelectedDates changed...
  // It would remove some flickering and gain speed (since this is called by
  // each updateView() call)
  delete mDayLabels;
  delete mBottomDayLabels;

  QFontMetrics fm = fontMetrics();

  mDayLabels = new QFrame ( mDayLabelsFrame );
  mDayLabelsFrame->setStretchFactor( mDayLabels, 1 );
  mLayoutDayLabels = new QHBoxLayout( mDayLabels );
  mLayoutDayLabels->setMargin( 0 );
  // this spacer moves the day labels over to line up with the day columns
  QSpacerItem *spacer =
    new QSpacerItem( mTimeLabelsZone->timeLabelsWidth(), 1, QSizePolicy::Fixed );
  mLayoutDayLabels->addSpacerItem( spacer );
  KVBox *weekLabelBox = new KVBox( mDayLabels );
  mLayoutDayLabels->addWidget( weekLabelBox );
  if ( mIsSideBySide ) {
    weekLabelBox->hide();
  }

  mBottomDayLabels = new QFrame( mBottomDayLabelsFrame );
  mBottomDayLabelsFrame->setStretchFactor( mBottomDayLabels, 1 );
  mLayoutBottomDayLabels = new QHBoxLayout( mBottomDayLabels );
  mLayoutBottomDayLabels->setMargin( 0 );
  KVBox *bottomWeekLabelBox = new KVBox( mBottomDayLabels );
  mLayoutBottomDayLabels->addWidget( bottomWeekLabelBox );

  const KCalendarSystem *calsys = KOGlobals::self()->calendarSystem();

#ifndef KORG_NOPLUGINS
  QList<CalendarDecoration::Decoration *> topDecos;
  if ( KOPrefs::instance()->decorationsAtAgendaViewTop().count() > 0 ) {
    mDayLabelsFrame->setParent( mSplitterAgenda );
    foreach ( const QString &decoName, KOPrefs::instance()->decorationsAtAgendaViewTop() ) {
      if ( KOPrefs::instance()->selectedPlugins().contains( decoName ) ) {
        topDecos << KOCore::self()->loadCalendarDecoration( decoName );
      }
    }
  } else {
    mDayLabelsFrame->setParent( this );
    mTopLayout->addWidget( mDayLabelsFrame, 0, 0 );
  }

  QList<CalendarDecoration::Decoration *> botDecos;
  if ( KOPrefs::instance()->decorationsAtAgendaViewBottom().count() > 0 ) {
    mBottomDayLabelsFrame->setParent( mSplitterAgenda );
    foreach ( const QString &decoName, KOPrefs::instance()->decorationsAtAgendaViewBottom() ) {
      if ( KOPrefs::instance()->selectedPlugins().contains( decoName ) ) {
        botDecos << KOCore::self()->loadCalendarDecoration( decoName );
      }
    }
  } else {
    mBottomDayLabelsFrame->setParent( this );
    mTopLayout->addWidget( mBottomDayLabelsFrame, 0, 0 );
  }
#endif

  DateList::ConstIterator dit;
  for ( dit = mSelectedDates.constBegin(); dit != mSelectedDates.constEnd(); ++dit ) {
    QDate date = *dit;
    KVBox *dayLabelBox = new KVBox( mDayLabels );
    mLayoutDayLabels->addWidget( dayLabelBox );
    KVBox *bottomDayLabelBox = new KVBox( mBottomDayLabels );
    mLayoutBottomDayLabels->addWidget( bottomDayLabelBox );

    int dW = calsys->dayOfWeek( date );
    QString veryLongStr = KGlobal::locale()->formatDate( date );
    QString longstr = i18nc( "short_weekday date (e.g. Mon 13)","%1 %2",
                             calsys->weekDayName( dW, KCalendarSystem::ShortDayName ),
                             calsys->day( date ) );
    QString shortstr = QString::number( calsys->day( date ) );

    KOAlternateLabel *dayLabel =
      new KOAlternateLabel( shortstr, longstr, veryLongStr, dayLabelBox );
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
      KWordWrap *ww = KWordWrap::formatText( fm, dayLabelBox->rect(), 0, (*textit), -1 );
      KOAlternateLabel *label =
        new KOAlternateLabel( ww->truncatedString(), (*textit), (*textit), dayLabelBox );
      label->setMinimumWidth( 1 );
      label->setAlignment( Qt::AlignCenter );
    }

#ifndef KORG_NOPLUGINS
    foreach ( CalendarDecoration::Decoration *deco, topDecos ) {
      CalendarDecoration::Element::List elements;
      elements = deco->dayElements( date );
      if ( elements.count() > 0 ) {
        KHBox *decoHBox = new KHBox( dayLabelBox );
        decoHBox->setFrameShape( QFrame::StyledPanel );
        decoHBox->setMinimumWidth( 1 );

        foreach ( CalendarDecoration::Element *it, elements ) {
          KODecorationLabel *label = new KODecorationLabel( it, decoHBox );
          label->setAlignment( Qt::AlignBottom );
          label->setMinimumWidth( 1 );
        }
      }
    }

    foreach ( CalendarDecoration::Decoration *deco, botDecos ) {
      CalendarDecoration::Element::List elements;
      elements = deco->dayElements( date );
      if ( elements.count() > 0 ) {
        KHBox *decoHBox = new KHBox( bottomDayLabelBox );
        decoHBox->setFrameShape( QFrame::StyledPanel );
        decoHBox->setMinimumWidth( 1 );

        foreach ( CalendarDecoration::Element *it, elements ) {
          KODecorationLabel *label = new KODecorationLabel( it, decoHBox );
          label->setAlignment( Qt::AlignBottom );
          label->setMinimumWidth( 1 );
        }
      }
    }
#endif
  }

#ifndef KORG_NOPLUGINS
  // Week decoration labels
  foreach ( CalendarDecoration::Decoration *deco, topDecos ) {
    CalendarDecoration::Element::List elements;
    elements = deco->weekElements( mSelectedDates.first() );
    if ( elements.count() > 0 ) {
      KHBox *decoHBox = new KHBox( weekLabelBox );
      decoHBox->setFrameShape( QFrame::StyledPanel );
      decoHBox->setMinimumWidth( 1 );

      foreach ( CalendarDecoration::Element *it, elements ) {
        KODecorationLabel *label = new KODecorationLabel( it, decoHBox );
        label->setAlignment( Qt::AlignBottom );
        label->setMinimumWidth( 1 );
      }
    }
  }

  foreach ( CalendarDecoration::Decoration *deco, botDecos ) {
    CalendarDecoration::Element::List elements;
    elements = deco->weekElements( mSelectedDates.first() );
    if ( elements.count() > 0 ) {
      KHBox *decoHBox = new KHBox( bottomWeekLabelBox );
      decoHBox->setFrameShape( QFrame::StyledPanel );
      decoHBox->setMinimumWidth( 1 );

      foreach ( CalendarDecoration::Element *it, elements ) {
        KODecorationLabel *label = new KODecorationLabel( it, decoHBox );
        label->setAlignment( Qt::AlignBottom );
        label->setMinimumWidth( 1 );
      }
    }
  }
#endif

  if ( !mIsSideBySide ) {
    mLayoutDayLabels->addSpacing( mAgenda->verticalScrollBar()->width() );
    mLayoutBottomDayLabels->addSpacing( mAgenda->verticalScrollBar()->width() );
  }
  mDayLabels->show();
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

Incidence::List KOAgendaView::selectedIncidences()
{
  Incidence::List selected;
  Incidence *incidence;

  incidence = mAgenda->selectedIncidence();
  if ( incidence ) {
    selected.append( incidence );
  }

  incidence = mAllDayAgenda->selectedIncidence();
  if ( incidence ) {
    selected.append( incidence );
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
    QLabel *label = new QLabel( i18n( "All Day" ), mTimeBarHeaderFrame );
    label->setText( timeLabel->header() );
    label->setAlignment( Qt::AlignBottom | Qt::AlignHCenter );
    label->setWordWrap( true );
    label->setToolTip( timeLabel->headerToolTip() );
    mTimeBarHeaders.append( label );
  }
}

void KOAgendaView::updateTimeBarWidth()
{
  createTimeBarHeaders();

  int width = mTimeLabelsZone->timeLabelsWidth();

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

  Incidence *incidence = item->incidence();
  if ( !incidence ) {
    return;
  }
  if ( !mChanger || !mChanger->beginChange( incidence ) ) {
    return;
  }
  Incidence *oldIncidence = incidence->clone();

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
  if ( incidence->type() == "Event" ) {
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
    Event*ev = static_cast<Event*>( incidence );
    if ( incidence->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ) == startDt &&
         ev->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ) == endDt ) {
      // No change
      delete oldIncidence;
      return;
    }
  } else if ( incidence->type() == "Todo" ) {
    Todo *td = static_cast<Todo*>( incidence );

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
      delete oldIncidence;
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
  if ( incidence->type() == "Event" ) {
    /* setDtEnd() must be called before setDtStart(), otherwise, when moving events,
     * CalendarLocal::incidenceUpdated() will not remove the old hash and that causes
     * the event to be shown in the old date also (bug #179157).
     *
     * TODO: We need a better hashing mechanism for CalendarLocal.
     */
    ( static_cast<Event*>( incidence ) )->setDtEnd(
      endDt.toTimeSpec( incidence->dtEnd().timeSpec() ) );
    incidence->setDtStart( startDt.toTimeSpec( incidence->dtStart().timeSpec() ) );
  } else if ( incidence->type() == "Todo" ) {
    Todo *td = static_cast<Todo*>( incidence );
    if ( td->hasStartDate() ) {
      td->setDtStart( startDt.toTimeSpec( incidence->dtStart().timeSpec() ) );
    }
    td->setDtDue( endDt.toTimeSpec( td->dtDue().timeSpec() ) );
  }
  item->setItemDate( startDt.toTimeSpec( KOPrefs::instance()->timeSpec() ).date() );

  const bool result = mChanger->changeIncidence( oldIncidence, incidence );
  mChanger->endChange(incidence);
  delete oldIncidence;

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
    mUpdateItem = incidence;
    QTimer::singleShot( 0, this, SLOT(doUpdateItem()) );
  }

  enableAgendaUpdate( true );
}

void KOAgendaView::doUpdateItem()
{
  if ( mUpdateItem ) {
    changeIncidenceDisplay( mUpdateItem, KOGlobals::INCIDENCEEDITED );
    mUpdateItem = 0;
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

void KOAgendaView::showIncidences( const Incidence::List &incidences )
{
  // we must check if they are not filtered; if they are, remove the filter
  CalFilter *filter = calendar()->filter();
  bool wehaveall = true;
  if ( filter ) {
    for ( Incidence::List::ConstIterator it = incidences.constBegin();
          it != incidences.constEnd(); ++it ) {
      if ( !( wehaveall = filter->filterIncidence( *it ) ) ) {
        break;
      }
    }
  }

  if ( !wehaveall ) {
    calendar()->setFilter( 0 );
  }

  KDateTime start = incidences.first()->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ),
            end = incidences.first()->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() );
  Incidence *first = incidences.first();
  for ( Incidence::List::ConstIterator it = incidences.constBegin();
        it != incidences.constEnd(); ++it ) {
    if ( ( *it )->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ) < start ) {
      first = *it;
    }
    start = qMin( start, ( *it )->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ) );
    end = qMax( start, ( *it )->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ) );
  }

  end.toTimeSpec( start );    // allow direct comparison of dates
  if ( start.date().daysTo( end.date() ) + 1 <= currentDateCount() ) {
    showDates( start.date(), end.date() );
  } else {
    showDates( start.date(), start.date().addDays( currentDateCount() - 1 ) );
  }

  mAgenda->selectItemByUID( first->uid() );
}

void KOAgendaView::insertIncidence( Incidence *incidence, const QDate &curDate )
{
  if ( !filterByResource( incidence ) ) {
    return;
  }

  // FIXME: Use a visitor here, or some other method to get rid of the dynamic_cast's
  Event *event = dynamic_cast<Event *>( incidence );
  Todo  *todo  = dynamic_cast<Todo  *>( incidence );

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
    mAllDayAgenda->insertAllDayItem( incidence, columnDate, curCol, curCol );
  } else if ( incidence->allDay() ) {
      mAllDayAgenda->insertAllDayItem( incidence, columnDate, beginX, endX );
  } else if ( event && event->isMultiDay( KOPrefs::instance()->timeSpec() ) ) {
    int startY = mAgenda->timeToY(
      event->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).time() );
    QTime endtime( event->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ).time() );
    if ( endtime == QTime( 0, 0, 0 ) ) {
      endtime = QTime( 23, 59, 59 );
    }
    int endY = mAgenda->timeToY( endtime ) - 1;
    if ( ( beginX <= 0 && curCol == 0 ) || beginX == curCol ) {
      mAgenda->insertMultiItem( event, columnDate, beginX, endX, startY, endY );

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
      endY = mAgenda->timeToY( t ) - 1;
      startY = mAgenda->timeToY( t.addSecs( -1800 ) );
    }
    if ( endY < startY ) {
      endY = startY;
    }
    mAgenda->insertItem( incidence, columnDate, curCol, startY, endY );
    if ( startY < mMinY[curCol] ) {
      mMinY[curCol] = startY;
    }
    if ( endY > mMaxY[curCol] ) {
      mMaxY[curCol] = endY;
    }
  }
}

void KOAgendaView::changeIncidenceDisplayAdded( Incidence *incidence )
{
  Todo *todo = dynamic_cast<Todo *>(incidence);
  CalFilter *filter = calendar()->filter();
  if ( ( filter && !filter->filterIncidence( incidence ) ) ||
       ( ( todo && !KOPrefs::instance()->showAllDayTodo() ) ) ) {
    return;
  }

  displayIncidence( incidence );
}

void KOAgendaView::changeIncidenceDisplay( Incidence *incidence, int mode )
{
  switch ( mode ) {
    case KOGlobals::INCIDENCEADDED:
    {
      // Add an event. No need to recreate the whole view!
      // recreating everything even causes troubles: dropping to the
      // day matrix recreates the agenda items, but the evaluation is
      // still in an agendaItems' code, which was deleted in the mean time.
      // Thus KOrg crashes...
      changeIncidenceDisplayAdded( incidence );
      updateEventIndicators();
      break;
    }
    case KOGlobals::INCIDENCEEDITED:
    {
      if ( mAllowAgendaUpdate ) {
        removeIncidence( incidence );
        changeIncidenceDisplayAdded( incidence );
      }
      updateEventIndicators();
      break;
    }
    case KOGlobals::INCIDENCEDELETED:
    {
      mAgenda->removeIncidence( incidence );
      mAllDayAgenda->removeIncidence( incidence );
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

  /* Remember the uids of the selected items. In case one of the
   * items was deleted and re-added, we want to reselect it. */
  const QString &selectedAgendaUid = mAgenda->lastSelectedUid();
  const QString &selectedAllDayAgendaUid = mAllDayAgenda->lastSelectedUid();

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
  Incidence::List incidences = calendar()->incidences();

  foreach ( Incidence *incidence, incidences ) {
    displayIncidence( incidence );

    if( incidence->uid() == selectedAgendaUid && !selectedAgendaUid.isNull() ) {
      mAgenda->selectItemByUID( incidence->uid() );
      somethingReselected = true;
    }

    if( incidence->uid() == selectedAllDayAgendaUid && !selectedAllDayAgendaUid.isNull() ) {
      mAllDayAgenda->selectItemByUID( incidence->uid() );
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
    emit incidenceSelected( 0 );
  }
}

void KOAgendaView::displayIncidence( Incidence *incidence ) {

  QDate today = QDate::currentDate();
  DateTimeList::iterator t;

  // FIXME: use a visitor here
  Todo *todo = dynamic_cast<Todo *>( incidence );
  Event *event = dynamic_cast<Event *>( incidence );

  KDateTime firstVisibleDateTime( mSelectedDates.first(), KOPrefs::instance()->timeSpec() );
  KDateTime lastVisibleDateTime( mSelectedDates.last(), KOPrefs::instance()->timeSpec() );

  lastVisibleDateTime.setTime( QTime( 23, 59 ) );
  firstVisibleDateTime.setTime( QTime( 0, 0 ) );
  DateTimeList dateTimeList;

  if ( todo &&
       ( !KOPrefs::instance()->showAllDayTodo() || !todo->hasDueDate() ) ) {
    return;
  }

  if ( incidence->recurs() ) {
    int eventDuration = incidence->dtStart().daysTo( incidence->dtEnd() );

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
      dateToAdd = todo->dtDue();
      incidenceEnd = dateToAdd;
    } else if ( event ) {
      dateToAdd = incidence->dtStart();
      incidenceEnd = incidence->dtEnd();

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
        if ( t->date() == today ) {
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
    insertIncidence( incidence, t->date() );
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
    mEventIndicatorTop->enableColumn( i, newY >= mMinY[i] );
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

void KOAgendaView::slotTodoDropped( Todo *todo, const QPoint &gpos, bool allDay )
{
  if ( gpos.x() < 0 || gpos.y() < 0 ) {
    return;
  }

  QDate day = mSelectedDates[gpos.x()];
  QTime time = mAgenda->gyToTime( gpos.y() );
  KDateTime newTime( day, time, KOPrefs::instance()->timeSpec() );
  newTime.setDateOnly( allDay );

  if ( todo ) {
    Todo *existingTodo = calendar()->todo( todo->uid() );
    if ( existingTodo ) {
      kDebug() << "Drop existing Todo";
      Todo *oldTodo = existingTodo->clone();
      if ( mChanger && mChanger->beginChange( existingTodo ) ) {
        existingTodo->setDtDue( newTime );
        existingTodo->setAllDay( allDay );
        existingTodo->setHasDueDate( true );
        mChanger->changeIncidence( oldTodo, existingTodo );
        mChanger->endChange( existingTodo );
      } else {
        KMessageBox::sorry( this, i18n( "Unable to modify this to-do, "
                                        "because it cannot be locked." ) );
      }
      delete oldTodo;
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
}

void KOAgendaView::startDrag( Incidence *incidence )
{
#ifndef KORG_NODND
  DndFactory factory( calendar() );
  QDrag *drag = factory.createDrag( incidence, this );
  if ( drag->start() ) {
    kDebug() << "Delete drag source";
  }
#endif
}

void KOAgendaView::readSettings()
{
  readSettings( KOGlobals::self()->config() );
}

void KOAgendaView::readSettings( KConfig *config )
{
  KConfigGroup group = config->group( "Views" );

  QList<int> sizes = group.readEntry( "Separator AgendaView", QList<int>() );
  mSplitterAgenda->setSizes( sizes );

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

void KOAgendaView::setTypeAheadReceiver( QObject *o )
{
  mAgenda->setTypeAheadReceiver( o );
  mAllDayAgenda->setTypeAheadReceiver( o );
}

void KOAgendaView::finishTypeAhead()
{
  mAgenda->finishTypeAhead();
  mAllDayAgenda->finishTypeAhead();
}

void KOAgendaView::removeIncidence( Incidence *incidence )
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

void KOAgendaView::setResource( KCal::ResourceCalendar *res, const QString &subResource )
{
  mResource = res;
  mSubResource = subResource;
}

bool KOAgendaView::filterByResource( Incidence *incidence )
{
  if ( !mResource ) {
    return true;
  }

  CalendarResources *calRes = dynamic_cast<CalendarResources*>( calendar() );
  if ( !calRes ) {
    return true;
  }

  if ( calRes->resource( incidence ) != mResource ) {
    return false;
  }

  if ( !mSubResource.isEmpty() ) {
    if ( mResource->subresourceIdentifier( incidence ) != mSubResource ) {
      return false;
    }
  }
  return true;
}

void KOAgendaView::setUpdateNeeded()
{
  mPendingChanges = true;
}

void KOAgendaView::calendarIncidenceAdded( Incidence *incidence )
{
  Q_UNUSED( incidence );
  mPendingChanges = true;
}

void KOAgendaView::calendarIncidenceChanged( Incidence *incidence )
{
  Q_UNUSED( incidence );
  mPendingChanges = true;
}

void KOAgendaView::calendarIncidenceRemoved( Incidence *incidence )
{
  Q_UNUSED( incidence );
  mPendingChanges = true;
}

#include "koagendaview.moc"
