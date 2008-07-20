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

#include "monthitem.h"
#include "monthgraphicsitems.h"
#include "monthscene.h"
#include "monthview.h"
#include "kodialogmanager.h"
#include "korganizer/incidencechangerbase.h"

#include "koprefs.h"
#include "kohelper.h"
#include "koglobals.h"

#include <kcal/todo.h>
#include <kcal/journal.h>
#include <kcal/event.h>
#include <kcal/incidence.h>
#include <kcal/incidenceformatter.h>

#include <KDebug>

#include <QDate>
#include <QColor>
#include <QPixmap>
#include <QObject>

using namespace KOrg;

//-------------------------------------------------------------
MonthItem::MonthItem( MonthScene *monthScene )
  : mMonthScene( monthScene ),
    mSelected( false ),
    mMoving( false ),
    mResizing( false )
{
}

MonthItem::~MonthItem()
{
  deleteAll();
}

void MonthItem::deleteAll()
{
  qDeleteAll( mMonthGraphicsItemList );
  mMonthGraphicsItemList.clear();
}

void MonthItem::updateMonthGraphicsItems()
{
  // Remove all items
  qDeleteAll( mMonthGraphicsItemList );
  mMonthGraphicsItemList.clear();

  // For each row of the month view, create an item to build the whole
  // MonthItem's MonthGraphicsItems.
  for ( QDate d = mMonthScene->mMonthView->startDate();
        d < mMonthScene->mMonthView->endDate(); d = d.addDays( 7 ) ) {
    QDate end = d.addDays( 6 );

    int span;
    QDate start;
    if ( startDate() <= d && endDate() >= end ) { // MonthItem takes the whole line
      span = 6;
      start = d;
    } else if ( startDate() >= d && endDate() <= end ) { // starts and ends on this line
      start = startDate();
      span = daySpan();
    } else if ( d <= endDate() && endDate() <= end ) { // MonthItem ends on this line
      span = mMonthScene->getLeftSpan( endDate() );
      start = d;
    } else if ( d <= startDate() && startDate() <= end ) { // MonthItem begins on this line
      span = mMonthScene->getRightSpan( startDate() );
      start = startDate();
    } else { // MonthItem is not on the line
      continue;
    }

    // A new item needs to be created
    MonthGraphicsItem *newItem = new MonthGraphicsItem( this );
    mMonthGraphicsItemList << newItem;
    newItem->setStartDate( start );
    newItem->setDaySpan( span );
  }

  if ( isMoving() || isResizing() ) {
    setZValue( 100 );
  } else {
    setZValue( 0 );
  }
}

void MonthItem::beginResize()
{
  mOverrideDaySpan = daySpan();
  mOverrideStartDate = startDate();
  mResizing = true;
  setZValue( 100 );
}

void MonthItem::endResize()
{
  setZValue( 0 );
  mResizing = false; // startDate() and daySpan() return real values again

  if ( mOverrideStartDate != startDate() || mOverrideDaySpan != daySpan() ) {
    finalizeResize( mOverrideStartDate, mOverrideStartDate.addDays( mOverrideDaySpan ) );
  }
}

void MonthItem::beginMove()
{
  mOverrideDaySpan = daySpan();
  mOverrideStartDate = startDate();
  mMoving = true;
  setZValue( 100 );
}

void MonthItem::endMove()
{
  setZValue( 0 );
  mMoving = false; // startDate() and daySpan() return real values again

  if ( mOverrideStartDate != startDate() ) {
    finalizeMove( mOverrideStartDate );
  }
}

bool MonthItem::resizeBy( int offsetToPreviousDate )
{
  bool ret = false;
  if ( mMonthScene->resizeType() == MonthScene::ResizeLeft ) {
    if ( mOverrideDaySpan - offsetToPreviousDate >= 0 ) {
      mOverrideStartDate = mOverrideStartDate.addDays( offsetToPreviousDate );
      mOverrideDaySpan = mOverrideDaySpan - offsetToPreviousDate;
      ret = true;
    }
  } else if ( mMonthScene->resizeType() == MonthScene::ResizeRight ) {
    if ( mOverrideDaySpan + offsetToPreviousDate >= 0 ) {
      mOverrideDaySpan = mOverrideDaySpan + offsetToPreviousDate;
      ret = true;
    }
  }

  if ( ret ) {
    updateMonthGraphicsItems();
  }
  return ret;
}

void MonthItem::moveBy( int offsetToPreviousDate )
{
  mOverrideStartDate = mOverrideStartDate.addDays( offsetToPreviousDate );
  updateMonthGraphicsItems();
}

void MonthItem::updateGeometry()
{
  foreach ( MonthGraphicsItem *item, mMonthGraphicsItemList ) {
      item->updateGeometry();
  }
}

void MonthItem::setZValue( qreal z )
{
  foreach ( MonthGraphicsItem *item, mMonthGraphicsItemList ) {
    item->setZValue( z );
  }
}

QDate MonthItem::startDate() const
{
  if ( isMoving() || isResizing() ) {
    return mOverrideStartDate;
  }

  return realStartDate();
}

QDate MonthItem::endDate() const
{
  if ( isMoving() || isResizing() ) {
    return mOverrideStartDate.addDays( mOverrideDaySpan );
  }

  return realEndDate();
}

int MonthItem::daySpan() const
{
  if ( isMoving() || isResizing() ) {
    return mOverrideDaySpan;
  }

  QDateTime start( startDate() );
  QDateTime end( endDate() );

  if ( end.isValid() ) {
    return start.daysTo( end );
  }

  return 0;
}

bool MonthItem::greaterThan( const MonthItem *e1, const MonthItem *e2 )
{
  if ( e1->startDate() == e2->startDate() ) {
    if ( e1->daySpan() == e2->daySpan() ) {
      if ( e1->allDay() ) {
        return true;
      }
      if ( e2->allDay() ) {
        return false;
      }
      return e1->greaterThanFallback( e2 );
    } else {
      return e1->daySpan() >  e2->daySpan();
    }
  }

  return e1->startDate() < e2->startDate();
}

bool MonthItem::greaterThanFallback( const MonthItem *other ) const
{
  // Yeah, pointer comparison if there is nothing else to compare...
  return this < other;
}

void MonthItem::updatePosition()
{
  int firstFreeSpace = 0;
  for ( QDate d = startDate(); d <= endDate(); d = d.addDays( 1 ) ) {
    MonthCell *cell = mMonthScene->mMonthCellMap.value( d );
    if ( !cell ) {
      continue; // cell can be null if the item begins outside the month
    }
    int firstFreeSpaceTmp = cell->firstFreeSpace();
    if ( firstFreeSpaceTmp > firstFreeSpace ) {
      firstFreeSpace = firstFreeSpaceTmp;
    }
  }

  for ( QDate d = startDate(); d <= endDate(); d = d.addDays( 1 ) ) {
    MonthCell *cell = mMonthScene->mMonthCellMap.value( d );
    if ( !cell ) {
      continue;
    }
    cell->addMonthItem( this, firstFreeSpace );
  }

  mPosition = firstFreeSpace;
}

//-----------------------------------------------------------------
// INCIDENCEMONTHITEM
IncidenceMonthItem::IncidenceMonthItem( MonthScene *monthScene,
                                        Incidence *incidence,
                                        const QDate &recurStartDate )
  : MonthItem( monthScene ), mIncidence( incidence )
{
  mIsEvent = mIncidence->type() == "Event";
  mIsJournal = mIncidence->type() == "Journal";
  mIsTodo = mIncidence->type() == "Todo";

  connect( monthScene, SIGNAL(incidenceSelected(Incidence* )),
           this, SLOT(updateSelection(Incidence* )) );

  // first set to 0, because it's used in startDate()
  mRecurDayOffset = 0;
  if ( recurStartDate.isValid() ) {
    mRecurDayOffset = startDate().daysTo( recurStartDate );
  }
}

IncidenceMonthItem::~IncidenceMonthItem()
{
}

bool IncidenceMonthItem::greaterThanFallback( const MonthItem *other ) const
{
  const IncidenceMonthItem *o = dynamic_cast<const IncidenceMonthItem *>( other );
  if ( !o ) {
    return MonthItem::greaterThanFallback( other );
  }

  if ( allDay() != o->allDay() ) {
    return allDay();
  }
  if ( mIncidence->dtStart().time() != o->mIncidence->dtStart().time() ) {
    return mIncidence->dtStart().time() < o->mIncidence->dtStart().time();
  }

  // as a last resort, compare the uid's
  return mIncidence->uid() < o->mIncidence->uid();
}

QDate IncidenceMonthItem::realStartDate() const
{
  KDateTime dt;
  if ( mIsEvent || mIsJournal ) {
    dt = mIncidence->dtStart();
  } else if ( mIsTodo ) {
    dt = static_cast<Todo *>( mIncidence )->dtDue();
  }

  QDate start;
  if ( dt.isDateOnly() ) {
    start = dt.date();
  } else {
    start = dt.toTimeSpec( KOPrefs::instance()->timeSpec() ).date();
  }

  return start.addDays( mRecurDayOffset );
}
QDate IncidenceMonthItem::realEndDate() const
{
  KDateTime dt;
  if ( mIsEvent || mIsJournal ) {
    dt = mIncidence->dtEnd();
  } else if ( mIsTodo ) {
    dt = static_cast<Todo *>( mIncidence )->dtDue();
  }

  QDate end;
  if ( dt.isDateOnly() ) {
    end = dt.date();
  } else {
    end = dt.toTimeSpec( KOPrefs::instance()->timeSpec() ).date();
  }

  return end.addDays( mRecurDayOffset );
}
bool IncidenceMonthItem::allDay() const
{
  return mIncidence->allDay();
}

bool IncidenceMonthItem::isMoveable() const
{
  return !mIncidence->isReadOnly();
}
bool IncidenceMonthItem::isResizable() const
{
  return mIsEvent && !mIncidence->isReadOnly();
}

void IncidenceMonthItem::finalizeMove( const QDate &newStartDate )
{
  Q_ASSERT( isMoveable() );
  IncidenceChangerBase *changer = monthScene()->incidenceChanger();

  if ( !changer || !changer->beginChange( mIncidence ) ) {
    KODialogManager::errorSaveIncidence( 0, mIncidence );
    return;
  }

  Incidence *oldInc = mIncidence->clone();

  int offset = startDate().daysTo( newStartDate );

  if ( mIsTodo ) {
    Todo *todo = static_cast<Todo *>( mIncidence );
    todo->setDtDue( todo->dtDue().addDays( offset ) );
  } else {
    mIncidence->setDtStart( mIncidence->dtStart().addDays( offset ) );
    if ( mIsEvent ) {
      Event *event = static_cast<Event *>( mIncidence );
      event->setDtEnd( event->dtEnd().addDays( offset ) );
    }
  }

  changer->changeIncidence( oldInc, mIncidence, KOGlobals::DATE_MODIFIED );
  changer->endChange( mIncidence );

  delete oldInc;
}
void IncidenceMonthItem::finalizeResize( const QDate &newStartDate,
                                         const QDate &newEndDate )
{
  Q_ASSERT( isResizable() );
  IncidenceChangerBase *changer = monthScene()->incidenceChanger();

  if ( !changer || !changer->beginChange( mIncidence ) ) {
    KODialogManager::errorSaveIncidence( 0, mIncidence );
    return;
  }

  Incidence *oldInc = mIncidence->clone();

  Event *event = static_cast<Event *>( mIncidence );

  int offset = startDate().daysTo( newStartDate );
  event->setDtStart( event->dtStart().addDays( offset ) );

  offset = endDate().daysTo( newEndDate );
  event->setDtEnd( event->dtEnd().addDays( offset ) );

  changer->changeIncidence( oldInc, mIncidence, KOGlobals::DATE_MODIFIED );
  changer->endChange( mIncidence );

  delete oldInc;
}

void IncidenceMonthItem::updateSelection( Incidence *incidence )
{
  setSelected( incidence == mIncidence );
}

QString IncidenceMonthItem::text( bool end ) const
{
  QString ret = mIncidence->summary();
  if ( !allDay() ) { // Prepend the time str to the text
    QTime time;
    if ( mIsTodo ) {
      Todo *todo = static_cast<Todo*>( mIncidence );
      time = todo->dtDue().time();
    } else {
      if ( !end ) {
        time = mIncidence->dtStart().time();
      } else {
        time = mIncidence->dtEnd().time();
      }
    }
    if ( time.isValid() ) {
      if ( !end ) {
        ret = KGlobal::locale()->formatTime( time ) + ' ' + ret;
      } else {
        ret = ret + ' ' + KGlobal::locale()->formatTime( time );
      }
    }
  }

  return ret;
}

QString IncidenceMonthItem::toolTipText() const
{
  return IncidenceFormatter::toolTipString( mIncidence );
}

QList<QPixmap *> IncidenceMonthItem::icons() const
{
  QList<QPixmap *> ret;

  if ( mIsEvent ) {
    ret << monthScene()->eventPixmap();
  } else if ( mIsTodo ) {
    if ( static_cast<Todo *>( mIncidence )->isCompleted() ) {
      ret << monthScene()->todoDonePixmap();
    } else {
      ret << monthScene()->todoPixmap();
    }
  } else if ( mIsJournal ) {
    ret << monthScene()->journalPixmap();
  }
  if ( mIncidence->isReadOnly() ) {
    ret << monthScene()->readonlyPixmap();
  }
#if 0
  /* sorry, this looks too cluttered. disable until we can
     make something prettier; no idea at this time -- allen */
  if ( mIncidence->isAlarmEnabled() ) {
    ret << monthScene()->alarmPixmap();
  }
  if ( mIncidence->recurs() ) {
    ret << monthScene()->recurPixmap();
  }
  //TODO: check what to do with Reply
#endif
  return ret;
}

QColor IncidenceMonthItem::bgColor() const
{
  QColor ret = QColor();
  if ( mIsTodo && !KOPrefs::instance()->todosUseCategoryColors() ) {
    if ( static_cast<Todo*>( mIncidence )->isOverdue() ) {
      ret = KOPrefs::instance()->agendaCalendarItemsToDosOverdueBackgroundColor();
    } else if ( static_cast<Todo*>( mIncidence )->dtDue().date() == QDate::currentDate() ) {
      ret = KOPrefs::instance()->agendaCalendarItemsToDosDueTodayBackgroundColor();
    }
  }

  if ( !ret.isValid() ) {
    QStringList categories = mIncidence->categories();
    QString cat;
    if ( !categories.isEmpty() ) {
      cat = categories.first();
    }
    if ( cat.isEmpty() ) {
      ret = KOPrefs::instance()->monthCalendarItemsEventsBackgroundColor();
    } else {
      ret = KOPrefs::instance()->categoryColor( cat );
    }
  }

  return ret;
}

QColor IncidenceMonthItem::frameColor( const QColor &bgColor ) const
{
  QColor resourceColor = KOHelper::resourceColor( monthScene()->calendar(),
                                                  mIncidence );
  QColor ret = Qt::black;
  if ( resourceColor.isValid() ) {
    ret = selected() ? QColor( 85 + resourceColor.red() * 2 / 3,
                               85 + resourceColor.green() * 2 / 3,
                               85 + resourceColor.blue() * 2 / 3 )
                      : resourceColor;
  } else {
    ret = selected() ? QColor( 85 + bgColor.red() * 2 / 3,
                               85 + bgColor.green() * 2 / 3,
                               85 + bgColor.blue() * 2 / 3 )
                      : bgColor.dark(115);
  }

  return ret;
}

//-----------------------------------------------------------------
// HOLIDAYMONTHITEM
HolidayMonthItem::HolidayMonthItem( MonthScene *monthScene, const QDate &date,
                                    const QString &name )
  : MonthItem( monthScene ), mDate( date ), mName( name )
{
}

HolidayMonthItem::~HolidayMonthItem()
{
}

bool HolidayMonthItem::greaterThanFallback( const MonthItem *other ) const
{
  const HolidayMonthItem *o = dynamic_cast<const HolidayMonthItem *>( other );
  if ( o ) {
    return MonthItem::greaterThanFallback( other );
  }

  // always put holidays on top
  return false;
}

void HolidayMonthItem::finalizeMove( const QDate &newStartDate )
{
  Q_UNUSED( newStartDate );
  Q_ASSERT( false );
}
void HolidayMonthItem::finalizeResize( const QDate &newStartDate,
                                       const QDate &newEndDate )
{
  Q_UNUSED( newStartDate );
  Q_UNUSED( newEndDate );
  Q_ASSERT( false );
}

QList<QPixmap *> HolidayMonthItem::icons() const
{
  QList<QPixmap *> ret;
  ret << monthScene()->holidayPixmap();

  return ret;
}

QColor HolidayMonthItem::bgColor() const
{
  // FIXME: Currently, only this value is settable in the options.
  // There is a monthHolidaysBackgroundColor() option too. Maybe it would be
  // wise to merge those two.
  return KOPrefs::instance()->agendaHolidaysBackgroundColor();
}

QColor HolidayMonthItem::frameColor( const QColor &bgColor ) const
{
  Q_UNUSED( bgColor );
  return Qt::black;
}
