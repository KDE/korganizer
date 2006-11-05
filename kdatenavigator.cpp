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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qstring.h>
#include <qkeycode.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qframe.h>
#include <qlabel.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kglobalsettings.h>

#include "koglobals.h"
#include "koprefs.h"
#include "kodaymatrix.h"

#include <kcalendarsystem.h>

#include "navigatorbar.h"

#include "kdatenavigator.h"

KDateNavigator::KDateNavigator( QWidget *parent, const char *name )
  : QFrame( parent, name ), mBaseDate( 1970, 1, 1 )
{
  QGridLayout* topLayout = new QGridLayout( this, 8, 8 );

  mNavigatorBar = new NavigatorBar( this );
  topLayout->addMultiCellWidget( mNavigatorBar, 0, 0, 0, 7 );

  connect( mNavigatorBar, SIGNAL( goPrevYear() ), SIGNAL( goPrevYear() ) );
  connect( mNavigatorBar, SIGNAL( goPrevMonth() ), SIGNAL( goPrevMonth() ) );
  connect( mNavigatorBar, SIGNAL( goNextMonth() ), SIGNAL( goNextMonth() ) );
  connect( mNavigatorBar, SIGNAL( goNextYear() ), SIGNAL( goNextYear() ) );
  connect( mNavigatorBar, SIGNAL( goMonth( int ) ), SIGNAL( goMonth( int ) ) );

  int i;
  QString generalFont = KGlobalSettings::generalFont().family();

  // Set up the heading fields.
  for( i = 0; i < 7; i++ ) {
    mHeadings[i] = new QLabel( this );
    mHeadings[i]->setFont( QFont( generalFont, 10, QFont::Bold ) );
    mHeadings[i]->setAlignment( AlignCenter );

    topLayout->addWidget( mHeadings[i], 1, i + 1 );
  }

  // Create the weeknumber labels
  for( i = 0; i < 6; i++ ) {
    mWeeknos[i] = new QLabel( this );
    mWeeknos[i]->setAlignment( AlignCenter );
    mWeeknos[i]->setFont( QFont( generalFont, 10 ) );
    mWeeknos[i]->installEventFilter( this );

    topLayout->addWidget( mWeeknos[i], i + 2, 0 );
  }

  mDayMatrix = new KODayMatrix( this, "KDateNavigator::dayMatrix" );

  connect( mDayMatrix, SIGNAL( selected( const KCal::DateList & ) ),
           SIGNAL( datesSelected( const KCal::DateList & ) ) );

  connect( mDayMatrix, SIGNAL( incidenceDropped( Incidence *, const QDate & ) ),
           SIGNAL( incidenceDropped( Incidence *, const QDate & ) ) );
  connect( mDayMatrix, SIGNAL( incidenceDroppedMove( Incidence * , const QDate & ) ),
           SIGNAL( incidenceDroppedMove( Incidence *, const QDate & ) ) );


  topLayout->addMultiCellWidget( mDayMatrix, 2, 7, 1, 7 );

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
  //int di = d1 - d2 + 1;
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
  return startDate().addDays( 6*7 );
}

void KDateNavigator::updateDates()
{
// kdDebug(5850) << "KDateNavigator::updateDates(), this=" << this << endl;
  QDate dayone = startDate();

  mDayMatrix->updateView( dayone );

  const KCalendarSystem *calsys = KOGlobals::self()->calendarSystem();

  // set the week numbers.
  for( int i = 0; i < 6; i++ ) {
    // Use QDate's weekNumber method to determine the week number!
    QDate dtStart = mDayMatrix->getDate( i * 7 );
    QDate dtEnd = mDayMatrix->getDate( ( i + 1 ) * 7 - 1 );
    int weeknumstart = calsys->weekNumber( dtStart );
    int weeknumend = calsys->weekNumber( dtEnd );
    QString weeknum;

    if ( weeknumstart != weeknumend ) {
      weeknum = i18n("start/end week number of line in date picker", "%1/%2")
                .arg( weeknumstart ).arg( weeknumend );
    } else {
      weeknum.setNum( weeknumstart );
    }
    mWeeknos[i]->setText( weeknum );
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
//   kdDebug(5850) << "KDateNavigator::updateView(), view " << this << endl;

  updateDayMatrix();
  repaint();
}

void KDateNavigator::updateConfig()
{
  int day;
  int weekstart = KGlobal::locale()->weekStartDay();
  for( int i = 0; i < 7; i++ ) {
    day = weekstart + i <= 7 ? weekstart + i : ( weekstart + i ) % 7;
    QString dayName = KOGlobals::self()->calendarSystem()->weekDayName( day,
                                                                        true );
    if ( KOPrefs::instance()->mCompactDialogs ) dayName = dayName.left( 1 );
    mHeadings[i]->setText( dayName );
  }

  // FIXME: Use actual config setting here
//  setShowWeekNums( true );
}

void KDateNavigator::setShowWeekNums( bool enabled )
{
  for( int i = 0; i < 6; i++ ) {
    if( enabled )
      mWeeknos[i]->show();
    else
      mWeeknos[i]->hide();
  }
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
  if( e->delta() > 0 ) emit goPrevious();
  else emit goNext();

  e->accept();
}

bool KDateNavigator::eventFilter ( QObject *o, QEvent *e )
{
  if ( e->type() == QEvent::MouseButtonPress ) {
    int i;
    for( i = 0; i < 6; ++i ) {
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
