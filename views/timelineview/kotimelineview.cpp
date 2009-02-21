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

#include <kcal/calendar.h>
#include <kcal/calendarresources.h>

#include <qlayout.h>

#include <kdgantt1/KDGanttViewTaskItem.h>
#include <kdgantt1/KDGanttViewSubwidgets.h>

#include "koeventpopupmenu.h"
#include "koglobals.h"
#include "koprefs.h"
#include "timelineitem.h"

using namespace KOrg;
using namespace KCal;

KOTimelineView::KOTimelineView( Calendar *calendar, QWidget *parent )
  : KOEventView( calendar, parent ), mEventPopup( 0 )
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
KCal::ListBase<KCal::Incidence> KOTimelineView::selectedIncidences()
{
  return KCal::ListBase<KCal::Incidence>();
}

/*virtual*/
KCal::DateList KOTimelineView::selectedDates()
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
  CalendarResources *calres = dynamic_cast<CalendarResources *>( calendar() );
  if ( !calres ) {
    item = new TimelineItem( i18n( "Calendar" ), mGantt );
    mCalendarItemMap[0][QString()] = item;
  } else {
    CalendarResourceManager *manager = calres->resourceManager();
    for ( CalendarResourceManager::ActiveIterator it = manager->activeBegin();
          it != manager->activeEnd(); ++it ) {
      QColor resourceColor = KOPrefs::instance()->resourceColor( (*it)->identifier() );
      if ( (*it)->canHaveSubresources() ) {
        QStringList subResources = (*it)->subresources();
        for ( QStringList::ConstIterator subit = subResources.constBegin();
              subit != subResources.constEnd(); ++subit ) {
          QString type = (*it)->subresourceType( *subit );
          if ( !(*it)->subresourceActive( *subit ) ||
               ( !type.isEmpty() && type != "event" ) ) {
            continue;
          }
          item = new TimelineItem( (*it)->labelForSubresource( *subit ), mGantt );
          resourceColor = KOPrefs::instance()->resourceColor( (*it)->identifier() );
          QColor subrescol = KOPrefs::instance()->resourceColor( *subit );
          if ( subrescol.isValid() ) {
            resourceColor = subrescol;
          }
          if ( resourceColor.isValid() ) {
            item->setColors( resourceColor, resourceColor, resourceColor );
          }
          mCalendarItemMap[*it][*subit] = item;
        }
      } else {
        item = new TimelineItem( (*it)->resourceName(), mGantt );
        if ( resourceColor.isValid() ) {
          item->setColors( resourceColor, resourceColor, resourceColor );
        }
        mCalendarItemMap[*it][QString()] = item;
      }
    }
  }

  // add incidences
  Event::List events;
  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
  for ( QDate day = start; day <= end; day = day.addDays( 1 ) ) {
    events = calendar()->events( day, timeSpec, EventSortStartDate, SortDirectionAscending );
    for ( Event::List::ConstIterator it = events.constBegin(); it != events.constEnd(); ++it ) {
      insertIncidence( *it, day );
    }
  }

  mGantt->setUpdateEnabled( true );
}

/*virtual*/
void KOTimelineView::showIncidences( const KCal::ListBase<KCal::Incidence>& )
{
}

/*virtual*/
void KOTimelineView::updateView()
{
  if ( mStartDate.isValid() && mEndDate.isValid() ) {
    showDates( mStartDate, mEndDate );
  }
}

/*virtual*/
void KOTimelineView::changeIncidenceDisplay( KCal::Incidence *incidence, int mode )
{
  switch ( mode ) {
  case KOGlobals::INCIDENCEADDED:
    insertIncidence( incidence );
    break;
  case KOGlobals::INCIDENCEEDITED:
    removeIncidence( incidence );
    insertIncidence( incidence );
    break;
  case KOGlobals::INCIDENCEDELETED:
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
    emit incidenceSelected( tlitem->incidence() );
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
    return;
  }
  if ( !mEventPopup ) {
    mEventPopup = eventPopup();
  }
  mEventPopup->showIncidencePopup(
    calendar(), tlitem->incidence(), tlitem->incidence()->dtStart().date() );
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
  emit newEventSignal( dt );
}

TimelineItem *KOTimelineView::calendarItemForIncidence( KCal::Incidence *incidence )
{
  CalendarResources *calres = dynamic_cast<CalendarResources *>( calendar() );
  TimelineItem *item = 0;
  if ( !calres ) {
    item = mCalendarItemMap[0][QString()];
  } else {
    ResourceCalendar *res = calres->resource( incidence );
    if ( !res ) {
      return 0;
    }
    if ( res->canHaveSubresources() ) {
      QString subRes = res->subresourceIdentifier( incidence );
      item = mCalendarItemMap[res][subRes];
    } else {
      item = mCalendarItemMap[res][QString()];
    }
  }
  return item;
}

void KOTimelineView::insertIncidence( KCal::Incidence *incidence, const QDate &day )
{
  TimelineItem *item = calendarItemForIncidence( incidence );
  if ( !item ) {
    kWarning() << "Help! Something is really wrong here!";
    return;
  }

  if ( incidence->recurs() ) {
    QList<KDateTime> l = incidence->startDateTimesForDate( day );
    if ( l.isEmpty() ) {
      // strange, but seems to happen for some recurring events...
      item->insertIncidence( incidence, KDateTime( day, incidence->dtStart().time() ),
                              KDateTime( day, incidence->dtEnd().time() ) );
    } else {
      for ( QList<KDateTime>::ConstIterator it = l.constBegin(); it != l.constEnd(); ++it ) {
        item->insertIncidence( incidence, *it, incidence->endDateForStart( *it ) );
      }
    }
  } else {
    if ( incidence->dtStart().date() == day ||
         incidence->dtStart().date() < mStartDate ) {
      item->insertIncidence( incidence );
    }
  }
}

void KOTimelineView::insertIncidence( KCal::Incidence *incidence )
{
  KCal::Event *event = dynamic_cast<KCal::Event *>( incidence );
  if ( !event ) {
    return;

  }

  if ( incidence->recurs() ) {
    insertIncidence( incidence, QDate() );
  }

  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
  for ( QDate day = mStartDate; day <= mEndDate; day = day.addDays( 1 ) ) {
    Event::List events = calendar()->events(
      day, timeSpec, EventSortStartDate, SortDirectionAscending );
    for ( Event::List::ConstIterator it = events.constBegin(); it != events.constEnd(); ++it ) {
      if ( events.contains( event ) ) {
        insertIncidence( *it, day );
      }
    }
  }
}

void KOTimelineView::removeIncidence( KCal::Incidence *incidence )
{
  TimelineItem *item = calendarItemForIncidence( incidence );
  if ( item ) {
    item->removeIncidence( incidence );
  } else {
    // try harder, the incidence might already be removed from the resource
    typedef QMap<QString, KOrg::TimelineItem *> M2_t;
    typedef QMap<KCal::ResourceCalendar *, M2_t> M1_t;
    for ( M1_t::ConstIterator it1 = mCalendarItemMap.constBegin();
          it1 != mCalendarItemMap.constEnd(); ++it1 ) {
      for ( M2_t::ConstIterator it2 = it1.data().constBegin();
            it2 != it1.data().constEnd(); ++it2 ) {
        if ( it2.data() ) {
          it2.data()->removeIncidence( incidence );
        }
      }
    }
  }
}

void KOTimelineView::itemMoved( KDGanttViewItem *item )
{
  TimelineSubItem *tlit = dynamic_cast<TimelineSubItem *>( item );
  if ( !tlit ) {
    return;
  }

  Incidence *i = tlit->incidence();
  mChanger->beginChange( i );

  KDateTime newStart( tlit->startTime() );
  if ( i->allDay() ) {
    newStart = KDateTime( newStart.date() );
  }

  int delta = tlit->originalStart().secsTo( newStart );
  i->setDtStart( i->dtStart().addSecs( delta ) );
  int duration = tlit->startTime().secsTo( tlit->endTime() );
  int allDayOffset = 0;
  if ( i->allDay() ) {
    int secsPerDay = 60 * 60 * 24;
    duration /= secsPerDay;
    duration *= secsPerDay;
    allDayOffset = secsPerDay;
    duration -= allDayOffset;
    if ( duration < 0 ) {
      duration = 0;
    }
  }
  i->setDuration( duration );
  TimelineItem *parent = static_cast<TimelineItem *>( tlit->parent() );
  parent->moveItems( i, tlit->originalStart().secsTo( newStart ), duration + allDayOffset );
  mChanger->endChange( i );
}

void KOTimelineView::overscale( KDGanttView::Scale scale )
{
  /* Disabled, looks *really* bogus:
     this triggers and endless rescaling loop; we want to set
     a fixed scale, the Gantt view doesn't like it and rescales
     (emitting a rescaling signal that leads here) and so on...
  Q_UNUSED( scale );
  //set a relative zoom factor of 1 (?!)
  mGantt->setZoomFactor( 1, false );
  mGantt->setScale( KDGanttView::Hour );
  mGantt->setMinorScaleCount( 12 );
  */
}

#include "kotimelineview.moc"
