/*
  This file is part of KOrganizer.

  Copyright (c) 2001,2002,2003 Cornelius Schumacher <schumacher@kde.org>
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

#include "kdatenavigator.h"
#include "navigatorbar.h"
#include "koglobals.h"
#include "koprefs.h"
#include "kodaymatrix.h"

#include <kdebug.h>
#include <kcalendarsystem.h>
#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>

#include <QString>
#include <qnamespace.h>
#include <QLayout>
#include <QTimer>
#include <QFrame>
#include <QLabel>
#include <QWheelEvent>
#include <QGridLayout>
#include <QEvent>

KDateNavigator::KDateNavigator( QWidget *parent )
  : QFrame( parent ), mBaseDate( 1970, 1, 1 )
{
  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setMargin( 0 );
  topLayout->setSpacing( 0 );

  mNavigatorBar = new NavigatorBar( this );
  topLayout->addWidget( mNavigatorBar, 0, 0, 1, 8 );

  connect( mNavigatorBar, SIGNAL(goPrevYear()), SIGNAL(goPrevYear()) );
  connect( mNavigatorBar, SIGNAL(goPrevMonth()), SIGNAL(goPrevMonth()) );
  connect( mNavigatorBar, SIGNAL(goNextMonth()), SIGNAL(goNextMonth()) );
  connect( mNavigatorBar, SIGNAL(goNextYear()), SIGNAL(goNextYear()) );
  connect( mNavigatorBar, SIGNAL(goMonth(int)), SIGNAL(goMonth(int)) );
  connect( mNavigatorBar, SIGNAL(goYear(int)), SIGNAL(goYear(int)) );

  QString generalFont = KGlobalSettings::generalFont().family();

  // Set up the heading fields.
  for ( int i=0; i < 7; i++ ) {
    mHeadings[i] = new QLabel( this );
    mHeadings[i]->setFont( QFont( generalFont, 10, QFont::Bold ) );
    mHeadings[i]->setAlignment( Qt::AlignCenter );

    topLayout->addWidget( mHeadings[i], 1, i + 1 );
  }

  // Create the weeknumber labels
  for ( int i=0; i < 6; i++ ) {
    mWeeknos[i] = new QLabel( this );
    mWeeknos[i]->setAlignment( Qt::AlignCenter );
    mWeeknos[i]->setFont( QFont( generalFont, 10 ) );
    mWeeknos[i]->installEventFilter( this );

    topLayout->addWidget( mWeeknos[i], i + 2, 0 );
  }

  mDayMatrix = new KODayMatrix( this );
  mDayMatrix->setObjectName( "KDateNavigator::dayMatrix" );

  connect( mDayMatrix, SIGNAL(selected(const KCal::DateList &)),
           SIGNAL(datesSelected(const KCal::DateList &)) );

  connect( mDayMatrix, SIGNAL(incidenceDropped(Incidence *,const QDate &)),
           SIGNAL(incidenceDropped(Incidence *,const QDate &)) );
  connect( mDayMatrix, SIGNAL(incidenceDroppedMove(Incidence *,const QDate &)),
           SIGNAL(incidenceDroppedMove(Incidence *,const QDate &)) );

  connect( mDayMatrix, SIGNAL(newEventSignal(const QDate &)),
           SIGNAL(newEventSignal(const QDate &)) );
  connect( mDayMatrix, SIGNAL(newTodoSignal(const QDate &)),
           SIGNAL(newTodoSignal(const QDate &)) );
  connect( mDayMatrix, SIGNAL(newJournalSignal(const QDate &)),
           SIGNAL(newJournalSignal(const QDate &)) );

  topLayout->addWidget( mDayMatrix, 2, 1, 6, 7 );

  // read settings from configuration file.
  updateConfig();
}

KDateNavigator::~KDateNavigator()
{
}

void KDateNavigator::setCalendar( Calendar *cal )
{
  mDayMatrix->setCalendar( cal );
}

void KDateNavigator::setBaseDate( const QDate &date )
{
  if ( date != mBaseDate ) {
    mBaseDate = date;

    updateDates();
    updateView();

    // Use the base date to show the monthname and year in the header
    KCal::DateList dates;
    dates.append( date );
    mNavigatorBar->selectDates( dates );

    repaint();
    mDayMatrix->repaint();
  }
}

QSizePolicy KDateNavigator::sizePolicy () const
{
  return QSizePolicy( QSizePolicy::MinimumExpanding,
                      QSizePolicy::MinimumExpanding );
}

void KDateNavigator::updateToday()
{
  mDayMatrix->recalculateToday();
  mDayMatrix->repaint();
}

QDate KDateNavigator::startDate() const
{
  // Find the first day of the week of the current month.
  QDate dayone( mBaseDate.year(), mBaseDate.month(), mBaseDate.day() );
  int d2 = KOGlobals::self()->calendarSystem()->day( dayone );
  dayone = dayone.addDays( -d2 + 1 );

  const KCalendarSystem *calsys = KOGlobals::self()->calendarSystem();
  int m_fstDayOfWkCalsys = calsys->dayOfWeek( dayone );
  int weekstart = KGlobal::locale()->weekStartDay();

  // If month begins on Monday and Monday is first day of week,
  // month should begin on second line. Sunday doesn't have this problem.
  int nextLine = m_fstDayOfWkCalsys <= weekstart ? 7 : 0;

  // update the matrix dates
  int index = weekstart - m_fstDayOfWkCalsys - nextLine;

  dayone = dayone.addDays( index );

  return dayone;
}

QDate KDateNavigator::endDate() const
{
  return startDate().addDays( 6 * 7 );
}

void KDateNavigator::updateDates()
{
  QDate dayone = startDate();

  mDayMatrix->updateView( dayone );

  const KCalendarSystem *calsys = KOGlobals::self()->calendarSystem();

  // set the week numbers.
  for ( int i=0; i < 6; i++ ) {
    // Use QDate's weekNumber method to determine the week number!
    QDate dtStart = mDayMatrix->getDate( i * 7 );
    QDate dtEnd = mDayMatrix->getDate( ( i + 1 ) * 7 - 1 );
    int weeknumstart = calsys->weekNumber( dtStart );
    int weeknumend = calsys->weekNumber( dtEnd );
    QString weeknum;

    if ( weeknumstart != weeknumend ) {
      weeknum = i18nc( "start/end week number of line in date picker", "%1/%2",
                       weeknumstart, weeknumend );
    } else {
      weeknum.setNum( weeknumstart );
    }
    mWeeknos[i]->setText( weeknum );
    mWeeknos[i]->setToolTip( i18n( "Scroll to week number %1", weeknum ) );
    mWeeknos[i]->setWhatsThis(
      i18n( "Click here to scroll the display to week number %1 "
            "of the currently displayed year.", weeknum ) );
  }

// each updateDates is followed by an updateView -> repaint is issued there !
//  mDayMatrix->repaint();
}

void KDateNavigator::updateDayMatrix()
{
  mDayMatrix->updateView();
  mDayMatrix->repaint();
}

void KDateNavigator::updateView()
{
  updateDayMatrix();
  repaint();
}

void KDateNavigator::updateConfig()
{
  int day;
  int weekstart = KGlobal::locale()->weekStartDay();
  for ( int i=0; i < 7; i++ ) {
    day = weekstart + i <= 7 ? weekstart + i : ( weekstart + i ) % 7;
    QString dayName =
      KOGlobals::self()->calendarSystem()->weekDayName( day, KCalendarSystem::ShortDayName );
    if ( KOPrefs::instance()->mCompactDialogs ) {
      dayName = dayName.left( 1 );
    }
    QString longDayName =
      KOGlobals::self()->calendarSystem()->weekDayName( day, KCalendarSystem::LongDayName );
    mHeadings[i]->setText( dayName );
    mHeadings[i]->setToolTip( i18n( "%1", longDayName ) );
    mHeadings[i]->setWhatsThis(
      i18n( "A column header of the %1 dates in the month.", longDayName ) );
  }

  // FIXME: Use actual config setting here
//  setShowWeekNums( true );
}

void KDateNavigator::setShowWeekNums( bool enabled )
{
  for ( int i=0; i < 6; i++ ) {
    if( enabled ) {
      mWeeknos[i]->show();
    } else {
      mWeeknos[i]->hide();
    }
  }
}

void KDateNavigator::selectMonthHelper( int monthDifference )
{
  QDate baseDateNextMonth = KOGlobals::self()->calendarSystem()->addMonths(
                                            mBaseDate, monthDifference );

  DateList newSelection = mSelectedDates;
  for ( int i=0; i < mSelectedDates.count(); i++ ) {
    newSelection[i] =
      KOGlobals::self()->calendarSystem()->addMonths( newSelection[i], monthDifference );
  }

  setBaseDate( baseDateNextMonth );
  mSelectedDates = newSelection;
  mDayMatrix->setSelectedDaysFrom( *( newSelection.begin() ),
                                   *( --newSelection.end() ) );
  updateView();
}

void KDateNavigator::selectNextMonth()
{
  selectMonthHelper( 1 );
}

void KDateNavigator::selectPreviousMonth()
{
  selectMonthHelper( -1 );
}

void KDateNavigator::selectDates( const DateList &dateList )
{
  if ( dateList.count() > 0 ) {
    mSelectedDates = dateList;

    updateDates();

    mDayMatrix->setSelectedDaysFrom( *( dateList.begin() ),
                                     *( --dateList.end() ) );

    updateView();
  }
}

void KDateNavigator::wheelEvent ( QWheelEvent *e )
{
  if ( e->delta() > 0 ) {
    emit goPrevious();
  } else {
    emit goNext();
  }
  e->accept();
}

bool KDateNavigator::eventFilter ( QObject *o, QEvent *e )
{
  if ( e->type() == QEvent::MouseButtonPress ) {
    int i;
    for ( i=0; i < 6; ++i ) {
      if ( o == mWeeknos[ i ] ) {
        QDate weekstart = mDayMatrix->getDate( i * 7 );
        emit weekClicked( weekstart );
        break;
      }
    }
    return true;
  } else {
    return false;
  }
}

#include "kdatenavigator.moc"
