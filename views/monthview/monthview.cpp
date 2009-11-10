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
#include "akonadicalendar.h"
#include "monthscene.h"
#include "monthitem.h"
#include "monthgraphicsitems.h"
#include "koglobals.h"
#include "koprefs.h"
#include "koeventpopupmenu.h"

#include <akonadi/kcal/calendarsearch.h>
#include <akonadi/kcal/utils.h>

#include <KCal/Incidence>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QDate>
#include <QTimer>

using namespace Akonadi;
using namespace KOrg;

MonthView::MonthView( QWidget *parent )
  : KOEventView( parent )
{
  QHBoxLayout *topLayout = new QHBoxLayout( this );

  mView = new MonthGraphicsView( this );

  mScene = new MonthScene( this );
  mView->setScene( mScene );
  topLayout->addWidget( mView );

  QVBoxLayout *rightLayout = new QVBoxLayout( );
  rightLayout->setSpacing( 0 );
  rightLayout->setMargin( 0 );

  // push buttons to the bottom
  rightLayout->addStretch( 1 );

  QToolButton *minusMonth = new QToolButton( this );
  minusMonth->setIcon( KIcon( "arrow-up-double" ) );
  minusMonth->setToolTip( i18n( "Go back one month" ) );
  minusMonth->setAutoRaise( true );
  connect( minusMonth, SIGNAL(clicked()),
           this, SLOT(moveBackMonth()) );

  QToolButton *minusWeek = new QToolButton( this );
  minusWeek->setIcon( KIcon( "arrow-up" ) );
  minusWeek->setToolTip( i18n( "Go back one week" ) );
  minusWeek->setAutoRaise( true );
  connect( minusWeek, SIGNAL(clicked()),
           this, SLOT(moveBackWeek()) );

  QToolButton *plusWeek = new QToolButton( this );
  plusWeek->setIcon( KIcon( "arrow-down" ) );
  plusWeek->setToolTip( i18n( "Go forward one week" ) );
  plusWeek->setAutoRaise( true );
  connect( plusWeek, SIGNAL(clicked()),
           this, SLOT(moveFwdWeek()) );

  QToolButton *plusMonth = new QToolButton( this );
  plusMonth->setIcon( KIcon( "arrow-down-double" ) );
  plusMonth->setToolTip( i18n( "Go forward one month" ) );
  plusMonth->setAutoRaise( true );
  connect( plusMonth, SIGNAL(clicked()),
           this, SLOT(moveFwdMonth()) );

  rightLayout->addWidget( minusMonth );
  rightLayout->addWidget( minusWeek );
  rightLayout->addWidget( plusWeek );
  rightLayout->addWidget( plusMonth );

  topLayout->addLayout( rightLayout );

  mViewPopup = eventPopup();

  connect( mScene, SIGNAL(showIncidencePopupSignal(Akonadi::Item, QDate)),
           mViewPopup, SLOT(showIncidencePopup(Akonadi::Item, QDate)) );

  connect( mScene, SIGNAL(showNewEventPopupSignal()),
           SLOT(showNewEventPopup()) );

  connect( mScene, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           this, SIGNAL(incidenceSelected(Akonadi::Item,QDate)) );

  connect( mScene, SIGNAL(newEventSignal()),
           this, SIGNAL(newEventSignal()) );
  updateConfig();
}

void MonthView::updateConfig() {
  CalendarSearch::IncidenceTypes types;
  if ( KOPrefs::instance()->showTodosMonthView() )
    types |= CalendarSearch::Todos;
  if ( KOPrefs::instance()->showJournalsMonthView() )
    types |= CalendarSearch::Journals;
  types |= CalendarSearch::Events;
  calendarSearch()->setIncidenceTypes( types );
}

MonthView::~MonthView()
{
}

int MonthView::currentDateCount()
{
  return mStartDate.daysTo( mEndDate );
}

int MonthView::maxDatesHint()
{
  return 6 * 7;
}

DateList MonthView::selectedDates()
{
  DateList list;

  if ( mScene->selectedItem() ) {
    IncidenceMonthItem *tmp = qobject_cast<IncidenceMonthItem *>( mScene->selectedItem() );
    if ( tmp ) {
      QDate selectedItemDate = tmp->realStartDate();
      if ( selectedItemDate.isValid() ) {
        list << selectedItemDate;
      }
    }
  }

  return list;
}

QDateTime MonthView::selectionStart()
{
  if ( mScene->selectedCell() ) {
    return QDateTime( mScene->selectedCell()->date() );
  } else {
    return QDateTime();
  }
}

QDateTime MonthView::selectionEnd()
{
  // Only one cell can be selected (for now)
  return selectionStart();
}

bool MonthView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay )
{
  if ( mScene->selectedCell() ) {
    startDt.setDate( mScene->selectedCell()->date() );
    endDt.setDate( mScene->selectedCell()->date() );
    allDay = true;
    return true;
  }

  return false;
}

void MonthView::showIncidences( const Item::List &incidenceList, const QDate &date )
{
  Q_UNUSED( incidenceList );
  Q_UNUSED( date );
}

void MonthView::changeIncidenceDisplay( const Item &incidence, int action )
{
  Q_UNUSED( incidence );
  Q_UNUSED( action );

  //TODO: add some more intelligence here...

  // don't call reloadIncidences() directly. It would delete all
  // MonthItems, but this changeIncidenceDisplay()-method was probably
  // called by one of the MonthItem objects. So only schedule a reload
  // as event
  QTimer::singleShot( 0, this, SLOT(reloadIncidences()) );
}

void MonthView::addIncidence( const Item &incidence )
{
  Q_UNUSED( incidence );

  //TODO: add some more intelligence here...
  reloadIncidences();
}

void MonthView::updateView()
{
  mView->update();
}

void MonthView::wheelEvent( QWheelEvent *event )
{
  // invert direction to get scroll-like behaviour
  if ( event->delta() > 0 ) {
    moveStartDate( -1, 0 );
  } else if ( event->delta() < 0 ) {
    moveStartDate( 1, 0 );
  }

  // call accept in every case, we do not want anybody else to react
  event->accept();
}

void MonthView::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_PageUp ) {
    moveStartDate( 0, -1 );
    event->accept();
  } else if ( event->key() == Qt::Key_PageDown ) {
    moveStartDate( 0, 1 );
    event->accept();
  } else if ( processKeyEvent( event ) ) {
    event->accept();
  } else {
    event->ignore();
  }
}

void MonthView::keyReleaseEvent( QKeyEvent *event )
{
  if ( processKeyEvent( event ) ) {
    event->accept();
  } else {
    event->ignore();
  }
}

void MonthView::moveBackMonth()
{
  moveStartDate( 0, -1 );
}

void MonthView::moveBackWeek()
{
  moveStartDate( -1, 0 );
}

void MonthView::moveFwdWeek()
{
  moveStartDate( 1, 0 );
}

void MonthView::moveFwdMonth()
{
  moveStartDate( 0, 1 );
}

void MonthView::moveStartDate( int weeks, int months )
{
  QDate start = startDate();
  QDate end = endDate();
  start = start.addDays( weeks * 7 );
  end = end.addDays( weeks * 7 );
  start = start.addMonths( months );
  end = end.addMonths( months );
  setDateRange( start, end );
}

void MonthView::showDates( const QDate &start, const QDate &end )
{
  Q_UNUSED( end );

  QDate dayOne( start );
  dayOne.setDate( start.year(), start.month(), 1 );

  setStartDate( dayOne );
}

QPair<QDate,QDate> MonthView::actualDateRange( const QDate& start, const QDate& ) const {
  QDate dayOne( start );
  dayOne.setDate( start.year(), start.month(), 1 );
  const int weekdayCol = ( dayOne.dayOfWeek() + 7 - KGlobal::locale()->weekStartDay() ) % 7;
  const QDate actualStart = dayOne.addDays( -weekdayCol );
  const QDate actualEnd = actualStart.addDays( 6 * 7 - 1 );
  return qMakePair( actualStart, actualEnd );
}

void MonthView::setStartDate( const QDate &start )
{
  int weekdayCol = ( start.dayOfWeek() + 7 - KGlobal::locale()->weekStartDay() ) % 7;
  mStartDate = start.addDays( -weekdayCol );

  mEndDate = mStartDate.addDays( 6 * 7 - 1 );

  // take "middle" day's month as current month
  mCurrentMonth = mStartDate.addDays( ( 6 * 7 ) / 2 ).month();

  reloadIncidences();
}

Akonadi::Item::List MonthView::selectedIncidences()
{
  Akonadi::Item::List selected;
  if ( mScene->selectedItem() ) {
    IncidenceMonthItem *tmp = qobject_cast<IncidenceMonthItem *>( mScene->selectedItem() );
    if ( tmp ) {
      Akonadi::Item incidenceSelected = tmp->incidence();
      if ( incidenceSelected.isValid() ) {
        selected.append( incidenceSelected );
      }
    }
  }
  return selected;
}

void MonthView::reloadIncidences()
{
  // keep selection if it exists
  Akonadi::Item incidenceSelected;

  MonthItem *itemToReselect = 0;

  if ( IncidenceMonthItem *tmp = qobject_cast<IncidenceMonthItem *>( mScene->selectedItem() ) ) {
    mSelectedItemId = tmp->incidence().id();
    mSelectedItemDate = tmp->realStartDate();
    if ( !mSelectedItemDate.isValid() ) {
      return;
    }
  }

  mScene->resetAll();

  // build monthcells hash
  int i = 0;
  for ( QDate d = mStartDate; d <= mEndDate; d = d.addDays( 1 ) ) {
    mScene->mMonthCellMap[ d ] = new MonthCell( i, d, mScene );
    i ++;
  }

//#ifdef AKONADI_PORT_DISABLED
  // build global event list
  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
  const Item::List incidences = calendar()->incidences();

  foreach ( const Item &aitem, incidences ) {
    if ( Akonadi::hasTodo( aitem ) && !KOPrefs::instance()->showTodosMonthView() ) {
      continue;
    }
    if ( Akonadi::hasJournal( aitem ) && !KOPrefs::instance()->showJournalsMonthView() ) {
      continue;
    }

    const Incidence::Ptr incidence = Akonadi::incidence( aitem );
    KDateTime incDtStart = incidence->dtStart().toTimeSpec( timeSpec );
    KDateTime incDtEnd   = incidence->dtEnd().toTimeSpec( timeSpec );

    // An event could start before the currently displayed date, so we
    // have to check at least those dates before the start date, which would
    // cause the event to span into the displayed date range.
    int offset = 0;
    if ( Akonadi::hasEvent( aitem ) ) {
      offset = incDtStart.daysTo( incDtEnd );
    }

    KDateTime startDateTime( mStartDate.addDays( - offset ), QTime( 0, 0 ), timeSpec );
    KDateTime endDateTime( mEndDate, QTime( 23, 59 ), timeSpec );

    DateTimeList dateTimeList;

    if ( incidence->recurs() ) {
      // Get a list of all dates that the recurring event will happen
      dateTimeList = incidence->recurrence()->timesInInterval( startDateTime, endDateTime );
    } else {
      KDateTime dateToAdd;

      if ( Todo::Ptr todo = Akonadi::todo( aitem ) ) {
        if ( todo->hasDueDate() ) {
          dateToAdd = todo->dtDue();
        }
      } else {
        dateToAdd = incDtStart;
      }

      if ( dateToAdd >= startDateTime && dateToAdd <= endDateTime ) {
        dateTimeList += dateToAdd;
      }
    }
    DateTimeList::iterator t;
    for ( t = dateTimeList.begin(); t != dateTimeList.end(); ++t ) {
      MonthItem *manager = new IncidenceMonthItem( mScene,
                                                   aitem,
                                                   t->toTimeSpec( timeSpec ).date() );
      mScene->mManagerList << manager;
      if ( mSelectedItemId == aitem.id() &&
           manager->realStartDate() == mSelectedItemDate ) {
        // only select it outside the loop because we are still creating items
        itemToReselect = manager;
      }
    }
  }

  if ( itemToReselect ) {
    mScene->selectItem( itemToReselect );
  }

  // add holidays
  for ( QDate d = mStartDate; d <= mEndDate; d = d.addDays( 1 ) ) {
    QStringList holidays( KOGlobals::self()->holiday( d ) );
    if ( !holidays.isEmpty() ) {
      MonthItem *holidayItem =
        new HolidayMonthItem(
          mScene, d,
          holidays.join( i18nc( "delimiter for joining holiday names", "," ) ) );
      mScene->mManagerList << holidayItem;
    }
  }

  // sort it
  qSort( mScene->mManagerList.begin(),
         mScene->mManagerList.end(),
         MonthItem::greaterThan );

  // build each month's cell event list
  foreach ( MonthItem *manager, mScene->mManagerList ) {
    for ( QDate d = manager->startDate(); d <= manager->endDate(); d = d.addDays( 1 ) ) {
      MonthCell *cell = mScene->mMonthCellMap.value( d );
      if ( cell ) {
        cell->mMonthItemList << manager;
      }
    }
  }

//#endif // AKONADI_PORT_DISABLED

  foreach ( MonthItem *manager, mScene->mManagerList ) {
    manager->updateMonthGraphicsItems();
    manager->updatePosition();
  }

  foreach ( MonthItem *manager, mScene->mManagerList ) {
    manager->updateGeometry();
  }

  mScene->setInitialized( true );
  mView->update();
  mScene->update();
}

void MonthView::calendarReset()
{
  kDebug();
}

void MonthView::incidencesAdded( const Item::List& incidences )
{
  Q_FOREACH( const Item& i, incidences )
      kDebug() << "item added: " << Akonadi::incidence( i )->summary();
}

void MonthView::incidencesAboutToBeRemoved( const Item::List& incidences )
{
  Q_FOREACH( const Item& i, incidences )
      kDebug() << "item removed: " << Akonadi::incidence( i )->summary();
}

void MonthView::incidencesChanged( const Item::List& incidences )
{
  Q_FOREACH( const Item& i, incidences )
      kDebug() << "item changed: " << Akonadi::incidence( i )->summary();
}


QDate MonthView::averageDate() const
{
  return startDate().addDays( startDate().daysTo( endDate() ) / 2 );
}

bool MonthView::usesFullWindow()
{
  return KOPrefs::instance()->mFullViewMonth;
}
