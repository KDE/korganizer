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
#include "monthitem.h"
#include "monthgraphicsitems.h"
#include "koglobals.h"
#include "koprefs.h"
#include "koeventpopupmenu.h"

#include <akonadi/kcal/calendar.h>
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

  connect( mScene, SIGNAL(newEventSignal(Akonadi::Collection::List)),
           this, SIGNAL(newEventSignal(Akonadi::Collection::List)) );
  mReloadTimer.setSingleShot( true );
  connect( &mReloadTimer, SIGNAL(timeout()), this, SLOT(reloadIncidences()) );
  mReloadTimer.start( 50 );
  updateConfig();

  mSelectedItemId = -1;
}

void MonthView::updateConfig() {
  CalendarSearch::IncidenceTypes types;
  if ( KOPrefs::instance()->showTodosMonthView() ) {
    types |= CalendarSearch::Todos;
  }

  if ( KOPrefs::instance()->showJournalsMonthView() ) {
    types |= CalendarSearch::Journals;
  }

  types |= CalendarSearch::Events;
  calendarSearch()->setIncidenceTypes( types );
}

MonthView::~MonthView()
{
}

int MonthView::currentDateCount()
{
  return actualStartDateTime().date().daysTo( actualEndDateTime().date() );
}

int MonthView::maxDatesHint()
{
  return 6 * 7;
}

DateList MonthView::selectedIncidenceDates()
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
  } else if ( mScene->selectedCell() ) {
    list << mScene->selectedCell()->date();
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
  KDateTime start = startDateTime();
  KDateTime end = endDateTime();
  start = start.addDays( weeks * 7 );
  end = end.addDays( weeks * 7 );
  start = start.addMonths( months );
  end = end.addMonths( months );
  setDateRange( start, end );
}

void MonthView::showDates( const QDate &start, const QDate &end )
{
  Q_UNUSED( start );
  Q_UNUSED( end );
  triggerDelayedReload();
}

QPair<KDateTime,KDateTime> MonthView::actualDateRange( const KDateTime &start,
                                                       const KDateTime & ) const {
  KDateTime dayOne( start );
  dayOne.setDate( QDate( start.date().year(), start.date().month(), 1 ) );
  const int weekdayCol = ( dayOne.date().dayOfWeek() + 7 - KGlobal::locale()->weekStartDay() ) % 7;
  KDateTime actualStart = dayOne.addDays( -weekdayCol );
  actualStart.setTime( QTime( 0, 0, 0, 0 ) );
  KDateTime actualEnd = actualStart.addDays( 6 * 7 - 1 );
  actualEnd.setTime( QTime( 23, 59, 59, 99 ) );
  return qMakePair( actualStart, actualEnd );
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
  for ( QDate d = actualStartDateTime().date(); d <= actualEndDateTime().date(); d = d.addDays( 1 ) ) {
    mScene->mMonthCellMap[ d ] = new MonthCell( i, d, mScene );
    i ++;
  }

  // build global event list
  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
  const Item::List incidences = Akonadi::itemsFromModel( calendarSearch()->model() );

  foreach ( const Item &aitem, incidences ) {
    const Incidence::Ptr incidence = Akonadi::incidence( aitem );

    DateTimeList dateTimeList;

    if ( incidence->recurs() ) {
      // Get a list of all dates that the recurring event will happen
      dateTimeList = incidence->recurrence()->timesInInterval(
        actualStartDateTime(), actualEndDateTime() );
    } else {
      KDateTime dateToAdd;

      if ( Todo::Ptr todo = Akonadi::todo( aitem ) ) {
        if ( todo->hasDueDate() ) {
          dateToAdd = todo->dtDue();
        }
      } else {
        dateToAdd = incidence->dtStart();
      }

      if ( dateToAdd >= actualStartDateTime() &&
           dateToAdd <= actualEndDateTime() ) {
        dateTimeList += dateToAdd;
      }

    }
    DateTimeList::const_iterator t;
    for ( t = dateTimeList.constBegin(); t != dateTimeList.constEnd(); ++t ) {
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
  for ( QDate d = actualStartDateTime().date(); d <= actualEndDateTime().date(); d = d.addDays( 1 ) ) {
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
  triggerDelayedReload();
}

void MonthView::incidencesAdded( const Item::List &incidences )
{
  KDateTime::Spec timeSpec = KOPrefs::instance()->timeSpec();
  Q_FOREACH ( const Item &i, incidences ) {
    kDebug() << "item added: " << Akonadi::incidence( i )->summary();
  }
  triggerDelayedReload();
}

void MonthView::incidencesAboutToBeRemoved( const Item::List &incidences )
{
  Q_FOREACH ( const Item &i, incidences ) {
    kDebug() << "item removed: " << Akonadi::incidence( i )->summary();
  }
  triggerDelayedReload();
}

void MonthView::incidencesChanged( const Item::List &incidences )
{
  Q_FOREACH ( const Item &i, incidences ) {
    kDebug() << "item changed: " << Akonadi::incidence( i )->summary();
  }
  triggerDelayedReload();
}

QDate MonthView::averageDate() const
{
  return actualStartDateTime().date().addDays( actualStartDateTime().date().daysTo( actualEndDateTime().date() ) / 2 );
}

int MonthView::currentMonth() const
{
  return averageDate().month();
}

bool MonthView::usesFullWindow()
{
  return KOPrefs::instance()->mFullViewMonth;
}
