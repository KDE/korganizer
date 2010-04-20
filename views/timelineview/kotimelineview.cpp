/*
  This file is part of KOrganizer.

  Copyright (c) 2007 Till Adam <adam@kde.org>

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

#include "kotimelineview.h"
#include "koeventpopupmenu.h"
#include "koglobals.h"
#include "koprefs.h"
#include "timelineitem.h"
#include "kohelper.h"

#include <kdgantt1/KDGanttViewTaskItem.h>
#include <kdgantt1/KDGanttViewSubwidgets.h>

#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/collectionselection.h>
#include <akonadi/kcal/utils.h>

#include <QLayout>

using namespace Akonadi;
using namespace KOrg;
using namespace KCal;

KOTimelineView::KOTimelineView( QWidget *parent )
  : KOEventView( parent ), mEventPopup( 0 )
{
  QVBoxLayout *vbox = new QVBoxLayout( this );
  mGantt = new KDGanttView( this );
  mGantt->setCalendarMode( true );
  mGantt->setShowLegendButton( false );
  mGantt->setFixedHorizon( true );
  mGantt->removeColumn( 0 );
  mGantt->addColumn( i18n( "Calendar" ) );
  mGantt->setHeaderVisible( true );
  if ( KGlobal::locale()->use12Clock() ) {
    mGantt->setHourFormat( KDGanttView::Hour_12 );
  } else {
    mGantt->setHourFormat( KDGanttView::Hour_24_FourDigit );
  }

  vbox->addWidget( mGantt );

  connect( mGantt, SIGNAL(gvCurrentChanged(KDGanttViewItem *)),
           SLOT(itemSelected(KDGanttViewItem *)) );
  connect( mGantt, SIGNAL(itemDoubleClicked(KDGanttViewItem *)),
           SLOT(itemDoubleClicked(KDGanttViewItem *)) );
  connect( mGantt, SIGNAL(itemRightClicked(KDGanttViewItem *)),
           SLOT(itemRightClicked(KDGanttViewItem *)) );
  connect( mGantt, SIGNAL(gvItemMoved(KDGanttViewItem *)),
           SLOT(itemMoved(KDGanttViewItem *)) );
  connect( mGantt, SIGNAL(rescaling(KDGanttView::Scale)),
           SLOT(overscale(KDGanttView::Scale)) );
  connect( mGantt, SIGNAL(dateTimeDoubleClicked(const QDateTime &)),
           SLOT(newEventWithHint(const QDateTime &)) );
}

KOTimelineView::~KOTimelineView()
{
  delete mEventPopup;
}

/*virtual*/
Akonadi::Item::List KOTimelineView::selectedIncidences()
{
  return mSelectedItemList;
}

/*virtual*/
KCal::DateList KOTimelineView::selectedIncidenceDates()
{
  return KCal::DateList();
}

/*virtual*/
int KOTimelineView::currentDateCount()
{
  return 0;
}

/*virtual*/
void KOTimelineView::showDates( const QDate &start, const QDate &end )
{
  kDebug() << "start=" << start << "end=" << end;

  mStartDate = start;
  mEndDate = end;
  mHintDate = QDateTime();
  mGantt->setHorizonStart( QDateTime( start ) );
  mGantt->setHorizonEnd( QDateTime( end.addDays( 1 ) ) );
  mGantt->setMinorScaleCount( 1 );
  mGantt->setScale( KDGanttView::Hour );
  mGantt->setMinimumScale( KDGanttView::Hour );
  mGantt->setMaximumScale( KDGanttView::Hour );
  mGantt->zoomToFit();

  mGantt->setUpdateEnabled( false );
  mGantt->clear();

  // item for every calendar
  TimelineItem *item = 0;
  Akonadi::Calendar *calres = calendar();
  if ( !calres ) {
    item = new TimelineItem( i18n( "Calendar" ), calendar(), mGantt );
    mCalendarItemMap.insert( -1, item );
  } else {
    const CollectionSelection *colSel = collectionSelection();
    const Collection::List collections = colSel->selectedCollections();

    Q_FOREACH ( const Collection &collection, collections ) {
      item = new TimelineItem( Akonadi::displayName( collection ), calendar(), mGantt );
      const QColor resourceColor = KOHelper::resourceColor( collection );
      if ( resourceColor.isValid() ) {
        item->setColors( resourceColor, resourceColor, resourceColor );
      }
      mCalendarItemMap.insert( collection.id(), item );
    }
  }

  // add incidences
  Item::List events;
  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
  for ( QDate day = start; day <= end; day = day.addDays( 1 ) ) {
    events = calendar()->events( day, timeSpec, EventSortStartDate, SortDirectionAscending );
    Q_FOREACH ( const Item &i, events ) {
      insertIncidence( i, day );
    }
  }

  mGantt->setUpdateEnabled( true );
}

/*virtual*/
void KOTimelineView::showIncidences( const Item::List &incidenceList, const QDate &date )
{
  Q_UNUSED( incidenceList );
  Q_UNUSED( date );
}

/*virtual*/
void KOTimelineView::updateView()
{
  if ( mStartDate.isValid() && mEndDate.isValid() ) {
    showDates( mStartDate, mEndDate );
  }
}

/*virtual*/
void KOTimelineView::changeIncidenceDisplay( const Item &incidence, int mode )
{
  switch ( mode ) {
  case Akonadi::IncidenceChanger::INCIDENCEADDED:
    insertIncidence( incidence );
    break;
  case Akonadi::IncidenceChanger::INCIDENCEEDITED:
    removeIncidence( incidence );
    insertIncidence( incidence );
    break;
  case Akonadi::IncidenceChanger::INCIDENCEDELETED:
    removeIncidence( incidence );
    break;
  default:
    updateView();
  }
}

void KOTimelineView::itemSelected( KDGanttViewItem *item )
{
  TimelineSubItem *tlitem = dynamic_cast<TimelineSubItem *>( item );
  if ( tlitem ) {
    emit incidenceSelected( tlitem->incidence(), tlitem->originalStart().date() );
  }
}

void KOTimelineView::itemDoubleClicked( KDGanttViewItem *item )
{
  TimelineSubItem *tlitem = dynamic_cast<TimelineSubItem *>( item );
  if ( tlitem ) {
    emit editIncidenceSignal( tlitem->incidence() );
  }
}

void KOTimelineView::itemRightClicked( KDGanttViewItem *item )
{
  mHintDate = QDateTime( mGantt->getDateTimeForCoordX( QCursor::pos().x(), true ) );
  TimelineSubItem *tlitem = dynamic_cast<TimelineSubItem *>( item );
  if ( !tlitem ) {
    showNewEventPopup();
    mSelectedItemList = Akonadi::Item::List();
    return;
  }
  if ( !mEventPopup ) {
    mEventPopup = eventPopup();
  }
  mEventPopup->showIncidencePopup( tlitem->incidence(),
                                   Akonadi::incidence( tlitem->incidence() )->dtStart().date() );
  mSelectedItemList << tlitem->incidence();
}

bool KOTimelineView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  startDt = QDateTime( mHintDate );
  endDt = QDateTime( mHintDate.addSecs( 2 * 60 * 60 ) );
  allDay = false;
  return mHintDate.isValid();
}

//slot
void KOTimelineView::newEventWithHint( const QDateTime &dt )
{
  mHintDate = dt;
  emit newEventSignal( collectionSelection()->selectedCollections(), dt );
}

TimelineItem *KOTimelineView::calendarItemForIncidence( const Item &incidence )
{
  Akonadi::Calendar *calres = calendar();
  TimelineItem *item = 0;
  if ( !calres ) {
    item = mCalendarItemMap.value( -1 );
  } else {
    item = mCalendarItemMap.value( incidence.parentCollection().id() );
  }
  return item;
}

void KOTimelineView::insertIncidence( const Item &aitem, const QDate &day )
{
  const Incidence::Ptr incidence = Akonadi::incidence( aitem );
  TimelineItem *item = calendarItemForIncidence( aitem );
  if ( !item ) {
    kWarning() << "Help! Something is really wrong here!";
    return;
  }

  if ( incidence->recurs() ) {
    QList<KDateTime> l = incidence->startDateTimesForDate( day );
    if ( l.isEmpty() ) {
      // strange, but seems to happen for some recurring events...
      item->insertIncidence( aitem, KDateTime( day, incidence->dtStart().time() ),
                              KDateTime( day, incidence->dtEnd().time() ) );
    } else {
      for ( QList<KDateTime>::ConstIterator it = l.constBegin(); it != l.constEnd(); ++it ) {
        item->insertIncidence( aitem, *it, incidence->endDateForStart( *it ) );
      }
    }
  } else {
    if ( incidence->dtStart().date() == day ||
         incidence->dtStart().date() < mStartDate ) {
      item->insertIncidence( aitem );
    }
  }
}

void KOTimelineView::insertIncidence( const Item &incidence )
{
  const Event::Ptr event = Akonadi::event( incidence );
  if ( !event ) {
    return;
  }

  if ( event->recurs() ) {
    insertIncidence( incidence, QDate() );
  }

  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
  for ( QDate day = mStartDate; day <= mEndDate; day = day.addDays( 1 ) ) {
    Item::List events = calendar()->events(
      day, timeSpec, EventSortStartDate, SortDirectionAscending );
    if ( events.contains( incidence ) ) //PENDING(AKONADI_PORT) check if correct. also check the original if, was inside the for loop (unnecessarily)
      for ( Item::List::ConstIterator it = events.constBegin(); it != events.constEnd(); ++it ) {
        insertIncidence( *it, day );
      }
  }
}

void KOTimelineView::removeIncidence( const Item &incidence )
{
  TimelineItem *item = calendarItemForIncidence( incidence );
  if ( item ) {
    item->removeIncidence( incidence );
  } else {
#if 0 //AKONADI_PORT_DISABLED
    // try harder, the incidence might already be removed from the resource
    typedef QMap<QString, KOrg::TimelineItem *> M2_t;
    typedef QMap<KCal::ResourceCalendar *, M2_t> M1_t;
    for ( M1_t::ConstIterator it1 = mCalendarItemMap.constBegin();
          it1 != mCalendarItemMap.constEnd(); ++it1 ) {
      for ( M2_t::ConstIterator it2 = it1.value().constBegin();
            it2 != it1.value().constEnd(); ++it2 ) {
        if ( it2.value() ) {
          it2.value()->removeIncidence( incidence );
        }
      }
    }
#endif
  }
}

void KOTimelineView::itemMoved( KDGanttViewItem *item )
{
  TimelineSubItem *tlit = dynamic_cast<TimelineSubItem *>( item );
  if ( !tlit ) {
    return;
  }

  const Item i = tlit->incidence();
  const Incidence::Ptr inc = Akonadi::incidence( i );
  mChanger->beginChange( i );

  KDateTime newStart( tlit->startTime() );
  if ( inc->allDay() ) {
    newStart = KDateTime( newStart.date() );
  }

  int delta = tlit->originalStart().secsTo( newStart );
  inc->setDtStart( inc->dtStart().addSecs( delta ) );
  int duration = tlit->startTime().secsTo( tlit->endTime() );
  int allDayOffset = 0;
  if ( inc->allDay() ) {
    int secsPerDay = 60 * 60 * 24;
    duration /= secsPerDay;
    duration *= secsPerDay;
    allDayOffset = secsPerDay;
    duration -= allDayOffset;
    if ( duration < 0 ) {
      duration = 0;
    }
  }
  inc->setDuration( duration );
  TimelineItem *parent = static_cast<TimelineItem *>( tlit->parent() );
  parent->moveItems( i, tlit->originalStart().secsTo( newStart ), duration + allDayOffset );
  mChanger->endChange( i );
}

void KOTimelineView::overscale( KDGanttView::Scale scale )
{
  Q_UNUSED( scale );
  /* Disabled, looks *really* bogus:
     this triggers and endless rescaling loop; we want to set
     a fixed scale, the Gantt view doesn't like it and rescales
     (emitting a rescaling signal that leads here) and so on...
  //set a relative zoom factor of 1 (?!)
  mGantt->setZoomFactor( 1, false );
  mGantt->setScale( KDGanttView::Hour );
  mGantt->setMinorScaleCount( 12 );
  */
}

#include "kotimelineview.moc"
