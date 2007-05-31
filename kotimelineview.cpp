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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/


#include <libkcal/calendar.h>
#include <libkcal/calendarresources.h>

#include <qlayout.h>

#include <kdgantt/KDGanttView.h>
#include <kdgantt/KDGanttViewTaskItem.h>
#include <kdgantt/KDGanttViewSubwidgets.h>

#include "koglobals.h"
#include "koprefs.h"
#include "timelineitem.h"

#include "kotimelineview.h"

using namespace KOrg;
using namespace KCal;

KOTimelineView::KOTimelineView(Calendar *calendar, QWidget *parent,
                                 const char *name)
  : KOrg::BaseView(calendar, parent, name)
{
    QVBoxLayout* vbox = new QVBoxLayout(this);
    mGantt = new KDGanttView(this);
    mGantt->setCalendarMode( true );
    mGantt->setShowLegendButton( false );
    mGantt->setScale( KDGanttView::Hour );
    mGantt->removeColumn( 0 );
    mGantt->addColumn( i18n("Calendar") );
    mGantt->setHeaderVisible( true );

    vbox->addWidget( mGantt );
}

KOTimelineView::~KOTimelineView()
{
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
void KOTimelineView::showDates(const QDate& start, const QDate& end)
{
  mGantt->setHorizonStart( QDateTime(start) );
  mGantt->setHorizonEnd( QDateTime(end) );
  mGantt->zoomToFit();

  mGantt->clear();

  // item for every calendar
  TimelineItem *item = 0;
  CalendarResources *calres = dynamic_cast<CalendarResources*>( calendar() );
  if ( !calres ) {
    item = new TimelineItem( i18n("Calendar"), mGantt );
    mCalendarItemMap[0][QString()] = item;
  } else {
    CalendarResourceManager *manager = calres->resourceManager();
    for ( CalendarResourceManager::ActiveIterator it = manager->activeBegin(); it != manager->activeEnd(); ++it ) {
      QColor resourceColor = *KOPrefs::instance()->resourceColor( (*it)->identifier() );
      if ( (*it)->canHaveSubresources() ) {
        QStringList subResources = (*it)->subresources();
        for ( QStringList::ConstIterator subit = subResources.constBegin(); subit != subResources.constEnd(); ++subit ) {
          QString type = (*it)->subresourceType( *subit );
          if ( !(*it)->subresourceActive( *subit ) || (!type.isEmpty() && type != "event") )
            continue;
          item = new TimelineItem( (*it)->labelForSubresource( *subit ), mGantt );
          resourceColor = *KOPrefs::instance()->resourceColor( (*it)->identifier() );
          QColor subrescol = *KOPrefs::instance()->resourceColor( *subit );
          if ( subrescol.isValid() )
            resourceColor = subrescol;
          if ( resourceColor.isValid() )
            item->setColors( resourceColor, resourceColor, resourceColor );
          mCalendarItemMap[*it][*subit] = item;
        }
      } else {
        item = new TimelineItem( (*it)->resourceName(), mGantt );
        if ( resourceColor.isValid() )
          item->setColors( resourceColor, resourceColor, resourceColor );
        mCalendarItemMap[*it][QString()] = item;
      }
    }
  }

  // add incidences
  Event::List events;
  for ( QDate day = start; day <= end; day = day.addDays( 1 ) ) {
    events = calendar()->events( day, EventSortStartDate, SortDirectionAscending );
    for ( Event::List::ConstIterator it = events.constBegin(); it != events.constEnd(); ++it ) {
      TimelineItem *item = 0;
      if ( !calres ) {
        item = mCalendarItemMap[0][QString()];
      } else {
        ResourceCalendar *res = calres->resource( *it );
        if ( res->canHaveSubresources() ) {
          QString subRes = res->subresourceIdentifier( *it );
          item = mCalendarItemMap[res][subRes];
        } else {
          item = mCalendarItemMap[res][QString()];
        }
      }
      if ( !item ) {
        kdWarning() << k_funcinfo << "Help! Something is really wrong here!" << endl;
        continue;
      }

      if ( (*it)->doesRecur() ) {
        QValueList<QDateTime> l = (*it)->startDateTimesForDate( day );
        if ( l.isEmpty() ) {
          // strange, but seems to happen for some recurring events...
          item->insertIncidence( *it, QDateTime( day, (*it)->dtStart().time() ),
                                  QDateTime( day, (*it)->dtEnd().time() ) );
        } else {
          for ( QValueList<QDateTime>::ConstIterator it2 = l.constBegin();
                it2 != l.constEnd(); ++it2 ) {
            item->insertIncidence( *it, *it2, (*it)->endDateForStart( *it2 ) );
          }
        }
      } else {
        item->insertIncidence( *it );
      }
    }
  }
}

/*virtual*/
void KOTimelineView::showIncidences(const KCal::ListBase<KCal::Incidence>&)
{
}

/*virtual*/
void KOTimelineView::updateView()
{
}

/*virtual*/
void KOTimelineView::changeIncidenceDisplay(KCal::Incidence*, int)
{
}

#include "kotimelineview.moc"
