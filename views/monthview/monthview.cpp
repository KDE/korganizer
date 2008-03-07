/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Bruno Virlet <bruno.virlet@gmail.com>

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

#include "monthview.h"
#include "monthscene.h"

#include <QVBoxLayout>
#include <QDate>

#include <kcal/incidence.h>
#include <kcal/calendar.h>

#include "koglobals.h"
#include "koprefs.h"
#include "koeventpopupmenu.h"
#include "kohelper.h"

using namespace KOrg;

KONewMonthView::KONewMonthView( Calendar *calendar, QWidget *parent )
  : KOEventView( calendar, parent)
{
  mView = new MonthGraphicsView( this, calendar );
  mScene = new MonthScene( this, calendar );
  mView->setScene( mScene );
  mView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->addWidget( mView );

  mViewPopup = eventPopup();

  connect( mScene, SIGNAL( showIncidencePopupSignal( Incidence *, const QDate & ) ),
           mViewPopup, SLOT( showIncidencePopup( Incidence *, const QDate & ) ) );

  connect( mScene, SIGNAL( showNewEventPopupSignal() ),
           SLOT( showNewEventPopup() ) );

}

KONewMonthView::~KONewMonthView()
{
}

int KONewMonthView::currentDateCount()
{
  return mStartDate.daysTo( mEndDate );
}

int KONewMonthView::maxDatesHint()
{
  return 6*7;
}

DateList KONewMonthView::selectedDates()
{
  DateList list;

   if ( mScene->selectedCell() )
     list << mScene->selectedCell()->date();

  return list;
}


bool KONewMonthView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  if ( mScene->selectedCell() ) {
    startDt.setDate( mScene->selectedCell()->date() );
    endDt.setDate( mScene->selectedCell()->date() );
    allDay = true;
    return true;
  }

  return false;
}


void KONewMonthView::showIncidences( const Incidence::List & )
{
}

void KONewMonthView::changeIncidenceDisplay( Incidence *incidence, int action )
{
  switch(action) {
    case KOGlobals::INCIDENCEADDED:
      //     addIncidence( incidence );
    case KOGlobals::INCIDENCEEDITED:
    case KOGlobals::INCIDENCEDELETED:
      reloadIncidences();
      break;
    default:
      break;
  }
}

void KONewMonthView::addIncidence( Incidence* incidence )
{
  reloadIncidences();
}

void KONewMonthView::updateView()
{
  mView->update();
}

void KONewMonthView::showDates( const QDate &start, const QDate &end )
{
  mCurrentMonth = start.month();

  QDate firstOfMonth = QDate(start.year(), start.month(), 1 );

  mStartDate = firstOfMonth.addDays( - ( firstOfMonth.dayOfWeek() - 1 ) );
  mEndDate = mStartDate.addDays( 6 * 7 - 1 );

  // TODO check that dates different from old ones.

  reloadIncidences();
}

void KONewMonthView::reloadIncidences()
{
  // keep selection if it exists
  Incidence *incidenceSelected = 0;
  if ( mScene->selectedItem() ) {
    incidenceSelected = mScene->selectedItem()->incidence();
  }

  mScene->resetAll();

  // build monthcells hash
  int i = 0;
  for ( QDate d = mStartDate; d <= mEndDate; d = d.addDays( 1 ) ) {
    mScene->mMonthCellMap[ d ] = new MonthCell( i, d );
    i ++;
  }

  // build global event list
  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
  Incidence::List incidences = calendar()->incidences();

  // remove incidences which are not in the good timespan
  foreach( Incidence *incidence, incidences ) {
    if ( incidence->type() == "Event" ) {
      Event *event = static_cast<Event*>( incidence );
      if ( mEndDate < KOHelper::toTimeSpec( event->dtStart() ).date()
           || KOHelper::toTimeSpec( event->dtEnd() ).date() < mStartDate ) {
        incidences.removeAll( incidence );
      }
    }
    else if ( incidence->type() == "Todo" ) {
      Todo *todo = dynamic_cast<Todo*>( incidence );
      if ( KOHelper::toTimeSpec( todo->dtDue() ).date() < mStartDate
           || KOHelper::toTimeSpec( todo->dtDue() ).date() > mEndDate )
      {
        incidences.removeAll( incidence );
      }

    }
  }

  foreach( Incidence *incidence, incidences ) {
    MonthItem *manager = new MonthItem( mScene, incidence );
    mScene->mManagerList << manager;
  }
  // sort it
  qSort( mScene->mManagerList.begin(),
         mScene->mManagerList.end(),
         MonthItem::greaterThan );

  // build each month's cell event list
  foreach( MonthItem *manager, mScene->mManagerList ) {
    for ( QDate d = manager->startDate(); d <= manager->endDate(); d = d.addDays( 1 ) ) {
      MonthCell *cell = mScene->mMonthCellMap.value( d );
      if ( cell )
	cell->mMonthItemList << manager;
    }
  }

  foreach( MonthItem *manager, mScene->mManagerList ) {
    manager->updateMonthGraphicsItems();
    manager->updateHeight();
  }

  foreach( MonthItem *manager, mScene->mManagerList ) {
    manager->updateGeometry();
  }

  // If there was an item selected before, reselect it.
  if ( incidenceSelected ) {
    foreach( MonthItem *manager, mScene->mManagerList ) {
      if ( manager->incidence() == incidenceSelected ) {
        mScene->selectItem( manager );
        break;
      }
    }
  }

  mScene->setInitialized( true );
  mView->update();
  mScene->update();
}
