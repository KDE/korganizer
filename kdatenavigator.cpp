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
#include "kodaymatrix.h"
#include "koglobals.h"
#include "navigatorbar.h"

#include <KCalendarSystem>
#include <KGlobal>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QWheelEvent>
#include <KLocalizedString>
#include <QFontDatabase>
#include <QLocale>

KDateNavigator::KDateNavigator( QWidget *parent )
  : QFrame( parent ), mBaseDate( 1970, 1, 1 )
{
  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setMargin( 0 );
  topLayout->setSpacing( 0 );

  mNavigatorBar = new NavigatorBar( this );
  topLayout->addWidget( mNavigatorBar, 0, 0, 1, 8 );

  connect( mNavigatorBar, SIGNAL(prevYearClicked()), SIGNAL(prevYearClicked()) );
  connect( mNavigatorBar, SIGNAL(prevMonthClicked()), SIGNAL(prevMonthClicked()) );
  connect( mNavigatorBar, SIGNAL(nextMonthClicked()), SIGNAL(nextMonthClicked()) );
  connect( mNavigatorBar, SIGNAL(nextYearClicked()), SIGNAL(nextYearClicked()) );
  connect( mNavigatorBar, SIGNAL(monthSelected(int)), SIGNAL(monthSelected(int)) );
  connect( mNavigatorBar, SIGNAL(yearSelected(int)), SIGNAL(yearSelected(int)));

  QString generalFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();

  // Set up the heading fields.
  for ( int i = 0; i < 7; ++i ) {
    mHeadings[i] = new QLabel( this );
    mHeadings[i]->setFont( QFont( generalFont, 10, QFont::Bold ) );
    mHeadings[i]->setAlignment( Qt::AlignCenter );

    topLayout->addWidget( mHeadings[i], 1, i + 1 );
  }

  // Create the weeknumber labels
  for ( int i = 0; i < 6; ++i ) {
    mWeeknos[i] = new QLabel( this );
    mWeeknos[i]->setAlignment( Qt::AlignCenter );
    mWeeknos[i]->setFont( QFont( generalFont, 10 ) );
    mWeeknos[i]->installEventFilter( this );

    topLayout->addWidget( mWeeknos[i], i + 2, 0 );
  }

  mDayMatrix = new KODayMatrix( this );
  mDayMatrix->setObjectName( QLatin1String("KDateNavigator::dayMatrix") );

  connect( mDayMatrix, SIGNAL(selected(KCalCore::DateList)),
           SIGNAL(datesSelected(KCalCore::DateList)) );

  connect( mDayMatrix, SIGNAL(incidenceDropped(Akonadi::Item,QDate)),
           SIGNAL(incidenceDropped(Akonadi::Item,QDate)) );
  connect( mDayMatrix, SIGNAL(incidenceDroppedMove(Akonadi::Item,QDate)),
           SIGNAL(incidenceDroppedMove(Akonadi::Item,QDate)) );

  connect( mDayMatrix, SIGNAL(newEventSignal(QDate)),
           SIGNAL(newEventSignal(QDate)) );
  connect( mDayMatrix, SIGNAL(newTodoSignal(QDate)),
           SIGNAL(newTodoSignal(QDate)) );
  connect( mDayMatrix, SIGNAL(newJournalSignal(QDate)),
           SIGNAL(newJournalSignal(QDate)) );

  topLayout->addWidget( mDayMatrix, 2, 1, 6, 7 );

  // read settings from configuration file.
  updateConfig();
}

KDateNavigator::~KDateNavigator()
{
}

void KDateNavigator::setCalendar( const Akonadi::ETMCalendar::Ptr &calendar )
{
    if (mCalendar)
        disconnect(mCalendar.data(), 0, this, 0);

    mCalendar = calendar;

    if (mCalendar)
        connect(mCalendar.data(), SIGNAL(calendarChanged()), SLOT(setUpdateNeeded()));

    mDayMatrix->setCalendar( calendar );
}

void KDateNavigator::setBaseDate( const QDate &date )
{
  if ( date != mBaseDate ) {
    mBaseDate = date;

    updateDates();
    updateView();

    // Use the base date to show the monthname and year in the header
    KCalCore::DateList dates;
    dates.append( date );
    mNavigatorBar->selectDates( dates );

    update();
    mDayMatrix->update();
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
  mDayMatrix->update();
}

QDate KDateNavigator::startDate() const
{
  // Find the first day of the week of the current month.
  QDate dayone( mBaseDate.year(), mBaseDate.month(), mBaseDate.day() );
  int d2 = KOGlobals::self()->calendarSystem()->day( dayone );
  dayone = dayone.addDays( -d2 + 1 );

  const KCalendarSystem *calsys = KOGlobals::self()->calendarSystem();
  int m_fstDayOfWkCalsys = calsys->dayOfWeek( dayone );
  int weekstart = QLocale().firstDayOfWeek();

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

void KDateNavigator::setHighlightMode( bool highlightEvents,
                                       bool highlightTodos,
                                       bool highlightJournals ) const {

  mDayMatrix->setHighlightMode( highlightEvents, highlightTodos, highlightJournals );
}

void KDateNavigator::updateDates()
{
  QDate dayone = startDate();

  mDayMatrix->updateView( dayone );

  const KCalendarSystem *calsys = KOGlobals::self()->calendarSystem();

  // set the week numbers.
  for ( int i=0; i < 6; ++i ) {
    // Use QDate's weekNumber method to determine the week number!
    QDate dtStart = mDayMatrix->getDate( i * 7 );
    QDate dtEnd = mDayMatrix->getDate( ( i + 1 ) * 7 - 1 );
    const int weeknumstart = calsys->week( dtStart );
    const int weeknumend = calsys->week( dtEnd );
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
  mDayMatrix->update();
}

void KDateNavigator::setUpdateNeeded()
{
  mDayMatrix->setUpdateNeeded();
}

QDate KDateNavigator::month() const
{
  QDate firstCell = startDate();
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

  if ( calSys->day( firstCell ) == 1 ) {
    return firstCell;
  } else {
    calSys->setDate( firstCell, calSys->year( firstCell ),
                    calSys->month( firstCell ), 1 );
    return calSys->addMonths( firstCell, 1 );
  }
}

void KDateNavigator::updateView()
{
  updateDayMatrix();
  update();
}

void KDateNavigator::updateConfig()
{
  int weekstart = QLocale().firstDayOfWeek();
  for ( int i=0; i < 7; ++i ) {
    const int day = weekstart + i <= 7 ? weekstart + i : ( weekstart + i ) % 7;
    QString dayName =
      KOGlobals::self()->calendarSystem()->weekDayName( day, KCalendarSystem::ShortDayName );
    QString longDayName =
      KOGlobals::self()->calendarSystem()->weekDayName( day, KCalendarSystem::LongDayName );
    mHeadings[i]->setText( dayName );
    mHeadings[i]->setToolTip( i18n( "%1", longDayName ) );
    mHeadings[i]->setWhatsThis(
      i18n( "A column header of the %1 dates in the month.", longDayName ) );
  }
  mDayMatrix->setUpdateNeeded();
  updateDayMatrix();
  update();
  // FIXME: Use actual config setting here
//  setShowWeekNums( true );
}

void KDateNavigator::setShowWeekNums( bool enabled )
{
  for ( int i=0; i < 6; ++i ) {
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

  KCalCore::DateList newSelection = mSelectedDates;
  for ( int i=0; i < mSelectedDates.count(); ++i ) {
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

void KDateNavigator::selectDates( const KCalCore::DateList &dateList )
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
      if ( o == mWeeknos[i] ) {
        const QDate weekstart = mDayMatrix->getDate( i * 7 );
        emit weekClicked( weekstart, month() );
        break;
      }
    }
    return true;
  } else {
    return false;
  }
}

