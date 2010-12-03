/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  Author: Sergio Martins <sergio@kdab.com>

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

#include "datenavigatorcontainer.h"
#include "koglobals.h"
#include "navigatorbar.h"
#include "kdatenavigator.h"
#include "kodaymatrix.h"

#include <kdebug.h>
#include <klocale.h>
#include <kcalendarsystem.h>
#include <kdialog.h>

#include <QTimer>
#include <QFrame>
#include <QResizeEvent>

DateNavigatorContainer::DateNavigatorContainer( QWidget *parent )
  : QFrame( parent ), mCalendar( 0 ),
    mHorizontalCount( 1 ), mVerticalCount( 1 ),
    mIgnoreNavigatorUpdates( false )
{
  mNavigatorView = new KDateNavigator( this );
  mNavigatorView->setWhatsThis(
                   i18n( "<qt><p>Select the dates you want to "
                         "display in KOrganizer's main view here. Hold the "
                         "mouse button to select more than one day.</p>"
                         "<p>Press the top buttons to browse to the next "
                         "/ previous months or years.</p>"
                         "<p>Each line shows a week. The number in the left "
                         "column is the number of the week in the year. "
                         "Press it to select the whole week.</p>"
                         "</qt>" ) );

  connectNavigatorView( mNavigatorView );
}

DateNavigatorContainer::~DateNavigatorContainer()
{
  qDeleteAll( mExtraViews );
}

void DateNavigatorContainer::connectNavigatorView( KDateNavigator *v )
{
  connect( v, SIGNAL(datesSelected(const KCalCore::DateList &)),
           SIGNAL(datesSelected(const KCalCore::DateList &)) );

  connect( v, SIGNAL(incidenceDropped(Akonadi::Item,QDate)),
           SIGNAL(incidenceDropped(Akonadi::Item,QDate)) );
  connect( v, SIGNAL(incidenceDroppedMove(Akonadi::Item,QDate)),
           SIGNAL(incidenceDroppedMove(Akonadi::Item,QDate)) );

  connect( v, SIGNAL(newEventSignal(const QDate &)),
           SIGNAL(newEventSignal(const QDate &)) );
  connect( v, SIGNAL(newTodoSignal(const QDate &)),
           SIGNAL(newTodoSignal(const QDate &)) );
  connect( v, SIGNAL(newJournalSignal(const QDate &)),
           SIGNAL(newJournalSignal(const QDate &)) );

  connect( v, SIGNAL(weekClicked(const QDate &)),
           SIGNAL(weekClicked(const QDate &)) );

  connect( v, SIGNAL(goPrevious()), SIGNAL(goPrevious()) );
  connect( v, SIGNAL(goNext()), SIGNAL(goNext()) );

  connect( v, SIGNAL(nextYearClicked()), SIGNAL(nextYearClicked()) );
  connect( v, SIGNAL(prevYearClicked()), SIGNAL(prevYearClicked()) );

  connect( v, SIGNAL(prevMonthClicked()), this, SLOT(goPrevMonth()) );
  connect( v, SIGNAL(nextMonthClicked()), this, SLOT(goNextMonth()) );

  connect( v, SIGNAL(monthSelected(int)), SIGNAL(monthSelected(int) ) );
  connect( v, SIGNAL(yearSelected(int)), SIGNAL(yearSelected(int)) );

}

void DateNavigatorContainer::setCalendar( CalendarSupport::Calendar *cal )
{
  mCalendar = cal;
  mNavigatorView->setCalendar( cal );
  foreach ( KDateNavigator *n, mExtraViews ) {
    if ( n ) {
      n->setCalendar( cal );
    }
  }
}

// TODO_Recurrence: let the navigators update just once, and tell them that
// if data has changed or just the selection (because then the list of dayss
// with events doesn't have to be updated if the month stayed the same
void DateNavigatorContainer::updateDayMatrix()
{
  mNavigatorView->updateDayMatrix();
  foreach ( KDateNavigator *n, mExtraViews ) {
    if ( n ) {
      n->updateDayMatrix();
    }
  }
}

void DateNavigatorContainer::updateToday()
{
  mNavigatorView->updateToday();
  foreach ( KDateNavigator *n, mExtraViews ) {
    if ( n ) {
      n->updateToday();
    }
  }
}

void DateNavigatorContainer::setUpdateNeeded()
{
  mNavigatorView->setUpdateNeeded();
  foreach ( KDateNavigator *n, mExtraViews ) {
    if ( n ) {
      n->setUpdateNeeded();
    }
  }
}

void DateNavigatorContainer::updateView()
{
  mNavigatorView->updateView();
  foreach ( KDateNavigator *n, mExtraViews ) {
    if ( n ) {
      n->setUpdateNeeded();
    }
  }
}

void DateNavigatorContainer::updateConfig()
{
  mNavigatorView->updateConfig();
  foreach ( KDateNavigator *n, mExtraViews ) {
    if ( n ) {
      n->updateConfig();
    }
  }
}

void DateNavigatorContainer::selectDates( const DateList &dateList, const QDate &preferredMonth )
{
  if ( !dateList.isEmpty() ) {
    QDate start( dateList.first() );
    QDate end( dateList.last() );
    QDate navfirst( mNavigatorView->startDate() );
    QDate navsecond; // start of the second shown month if existent
    QDate navlast;
    if ( !mExtraViews.isEmpty() ) {
      navlast = mExtraViews.last()->endDate();
      navsecond = mExtraViews.first()->startDate();
    } else {
      navlast = mNavigatorView->endDate();
      navsecond = navfirst;
    }

    const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

    // If the datelist crosses months we won't know which month to show
    // so we read what's in preferredMonth
    const bool changingMonth = ( preferredMonth.isValid()  &&
                                 calSys->month( mNavigatorView->month() ) != calSys->month( preferredMonth ) );

    if ( start < navfirst  // <- start should always be visible
         // end is not visible and we have a spare month at the beginning:
         || ( end > navlast && start >= navsecond )
         || changingMonth ) {

      if ( preferredMonth.isValid() ) {
        setBaseDates( preferredMonth );
      } else {
        setBaseDates( start );
      }
    }

    if ( !mIgnoreNavigatorUpdates ) {
      mNavigatorView->selectDates( dateList );
      foreach ( KDateNavigator *n, mExtraViews ) {
        if ( n ) {
          n->selectDates( dateList );
        }
      }
    }
  }
}

void DateNavigatorContainer::setBaseDates( const QDate &start )
{
  QDate baseDate = start;
  if ( !mIgnoreNavigatorUpdates ) {
    mNavigatorView->setBaseDate( baseDate );
  }

  foreach ( KDateNavigator *n, mExtraViews ) {
    baseDate = KOGlobals::self()->calendarSystem()->addMonths( baseDate, 1 );
    if ( !mIgnoreNavigatorUpdates ) {
      n->setBaseDate( baseDate );
    }
  }
}

void DateNavigatorContainer::resizeEvent( QResizeEvent * )
{
#if 0
  kDebug() << "DateNavigatorContainer::resizeEvent()";
  kDebug() << "  CURRENT SIZE:" << size();
  kDebug() << "  MINIMUM SIZEHINT:" << minimumSizeHint();
  kDebug() << "  SIZEHINT:" << sizeHint();
  kDebug() << "  MINIMUM SIZE:" << minimumSize();
#endif
  QTimer::singleShot( 0, this, SLOT(resizeAllContents()) );
}

void DateNavigatorContainer::resizeAllContents()
{
  QSize minSize = mNavigatorView->minimumSizeHint();

//  kDebug() << "  NAVIGATORVIEW minimumSizeHint:" << minSize;

  int verticalCount = size().height() / minSize.height();
  int horizontalCount = size().width() / minSize.width();

  if ( horizontalCount != mHorizontalCount || verticalCount != mVerticalCount ) {
    int count = horizontalCount * verticalCount;
    if ( count == 0 ) {
      return;
    }

    while ( count > ( mExtraViews.count() + 1 ) ) {
      KDateNavigator *n = new KDateNavigator( this );
      mExtraViews.append( n );
      n->setCalendar( mCalendar );
      connectNavigatorView( n );
    }

    while ( count < ( mExtraViews.count() + 1 ) ) {
      delete ( mExtraViews.last() );
      mExtraViews.removeLast();
    }

    mHorizontalCount = horizontalCount;
    mVerticalCount = verticalCount;
    setBaseDates( mNavigatorView->selectedDates().first() );
    selectDates( mNavigatorView->selectedDates() );
    foreach ( KDateNavigator *n, mExtraViews ) {
      if ( n ) {
        n->show();
      }
    }
  }

  int height = size().height() / verticalCount;
  int width = size().width() / horizontalCount;

  NavigatorBar *bar = mNavigatorView->navigatorBar();
  if ( horizontalCount > 1 ) {
    bar->showButtons( true, false );
  } else {
    bar->showButtons( true, true );
  }

  mNavigatorView->setGeometry( ( ( ( KOGlobals::self()->reverseLayout() ) ?
                                   ( horizontalCount - 1 ) : 0 ) * width ),
                               0, width, height );
  for ( int i = 0; i < mExtraViews.count(); ++i ) {
    int x = ( i + 1 ) % horizontalCount;
    int y = ( i + 1 ) / horizontalCount;

    KDateNavigator *view = mExtraViews.at( i );
    bar = view->navigatorBar();
    if ( y > 0 ) {
      bar->showButtons( false, false );
    } else {
      if ( x + 1 == horizontalCount ) {
        bar->showButtons( false, true );
      } else {
        bar->showButtons( false, false );
      }
    }
    view->setGeometry( ( ( ( KOGlobals::self()->reverseLayout() ) ?
                           ( horizontalCount - 1 - x ) : x ) * width ),
                       y * height, width, height );

  }
}

QSize DateNavigatorContainer::minimumSizeHint() const
{
  return mNavigatorView->minimumSizeHint();
}

QSize DateNavigatorContainer::sizeHint() const
{
  return mNavigatorView->sizeHint();
}

void DateNavigatorContainer::setHighlightMode( bool highlightEvents,
                                               bool highlightTodos,
                                               bool highlightJournals ) const {

  mNavigatorView->setHighlightMode( highlightEvents, highlightTodos, highlightJournals );

  foreach ( KDateNavigator *n, mExtraViews ) {
    if ( n ) {
      n->setHighlightMode( highlightEvents, highlightTodos, highlightJournals );
    }
  }

}

void DateNavigatorContainer::goNextMonth()
{
  const QPair<QDate,QDate> p = dateLimits( 1 );

  emit nextMonthClicked( mNavigatorView->month(),
                         p.first,
                         p.second);
}

void DateNavigatorContainer::goPrevMonth()
{
  const QPair<QDate,QDate> p = dateLimits( -1 );

  emit prevMonthClicked( mNavigatorView->month(),
                         p.first,
                         p.second );
}

QPair<QDate,QDate> DateNavigatorContainer::dateLimits( int offset )
{
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
  QDate firstMonth, lastMonth;
  if ( mExtraViews.isEmpty() ) {
    lastMonth = mNavigatorView->month();
  } else {
    lastMonth = mExtraViews.last()->month();
  }

  firstMonth = calSys->addMonths( mNavigatorView->month(), offset );
  lastMonth = calSys->addMonths( lastMonth, offset );

  QPair<QDate,QDate> firstMonthBoundary = KODayMatrix::matrixLimits( firstMonth );
  QPair<QDate,QDate> lastMonthBoundary = KODayMatrix::matrixLimits( lastMonth );

  return qMakePair( firstMonthBoundary.first, lastMonthBoundary.second );
}

#include "datenavigatorcontainer.moc"
