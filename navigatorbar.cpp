/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qstring.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qframe.h>
#include <qpopupmenu.h>
#include <qlabel.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "koglobals.h"
#include "koprefs.h"
#ifndef KORG_NOPLUGINS
#include "kocore.h"
#endif

#include <kcalendarsystem.h>

#include "navigatorbar.h"

ActiveLabel::ActiveLabel( QWidget *parent, const char *name )
  : QLabel( parent, name )
{
}

void ActiveLabel::mouseReleaseEvent( QMouseEvent * )
{
  emit clicked();
}


NavigatorBar::NavigatorBar( const QDate & date, QWidget *parent, const char *name )
  : QWidget( parent, name ), mDate(date)
{
  QBoxLayout *topLayout = new QHBoxLayout( this );

  // Set up the control buttons and date label
  mCtrlFrame = new QFrame( this );
  mCtrlFrame->setFrameStyle( QFrame::Panel | QFrame::Raised );
  mCtrlFrame->setLineWidth( 1 );

  topLayout->addWidget( mCtrlFrame );

  QFont tfont = font();
  tfont.setPointSize( 10 );
  tfont.setBold( false );

  bool isRTL = KOGlobals::self()->reverseLayout();

  // Create month name button
  mMonth = new ActiveLabel( mCtrlFrame );
  mMonth->setFont( tfont );
  mMonth->setAlignment( AlignCenter );
  QToolTip::add( mMonth, i18n("Select a Month") );

  // Set minimum width to width of widest month name label
  int i;
  int maxwidth = 0;

  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
  for( i = 1; i <= calSys->monthsInYear(date); ++i ) {
    mMonth->setText( QString("%1 8888").arg(calSys->monthName( i, calSys->year(date) )) );
    if ( mMonth->width() > maxwidth ) maxwidth = mMonth->width();
  }
  mMonth->setMinimumWidth( maxwidth );

  // Create backward navigation buttons
  mPrevYear = new QPushButton( mCtrlFrame );
  mPrevYear->setPixmap( SmallIcon( isRTL ? "2rightarrow" : "2leftarrow" ) );
  mPrevYear->setMinimumHeight(mMonth->height());
  QToolTip::add( mPrevYear, i18n("Previous year") );

  mPrevMonth = new QPushButton( mCtrlFrame );
  mPrevMonth->setPixmap( SmallIcon( isRTL ? "1rightarrow" : "1leftarrow") );
  mPrevMonth->setMinimumHeight(mMonth->height());
  QToolTip::add( mPrevMonth, i18n("Previous month") );

  // Create forward navigation buttons
  mNextMonth = new QPushButton( mCtrlFrame );
  mNextMonth->setPixmap( SmallIcon( isRTL ? "1leftarrow" : "1rightarrow") );
  mNextMonth->setMinimumHeight(mMonth->height());
  QToolTip::add( mNextMonth, i18n("Next month") );

  mNextYear = new QPushButton( mCtrlFrame );
  mNextYear->setPixmap( SmallIcon( isRTL ? "2leftarrow" : "2rightarrow") );
  mNextYear->setMinimumHeight(mMonth->height());
  QToolTip::add( mNextYear, i18n("Next year") );

  // set up control frame layout
  QBoxLayout *ctrlLayout = new QHBoxLayout( mCtrlFrame, 1 );
  ctrlLayout->addWidget( mPrevYear, 3 );
  ctrlLayout->addWidget( mPrevMonth, 3 );
  ctrlLayout->addSpacing( 2 );
  ctrlLayout->addWidget( mMonth, 3 );
  ctrlLayout->addSpacing( 2 );
  ctrlLayout->addWidget( mNextMonth, 3 );
  ctrlLayout->addWidget( mNextYear, 3 );

  connect( mPrevYear, SIGNAL( clicked() ), SIGNAL( goPrevYear() ) );
  connect( mPrevMonth, SIGNAL( clicked() ), SIGNAL( goPrevMonth() ) );
  connect( mNextMonth, SIGNAL( clicked() ), SIGNAL( goNextMonth() ) );
  connect( mNextYear, SIGNAL( clicked() ), SIGNAL( goNextYear() ) );
  connect( mMonth, SIGNAL( clicked() ), SLOT( selectMonth() ) );
}

NavigatorBar::~NavigatorBar()
{
}

void NavigatorBar::selectDates( const KCal::DateList &dateList )
{
  if (dateList.count() > 0) {
    mDate = dateList.first();

    const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

    // compute the label at the top of the navigator
    mMonth->setText( QString("%1 %2").arg(calSys->monthName( mDate )).arg(calSys->year(mDate)) );
  }
}

void NavigatorBar::selectMonth()
{
  // every year can have different month names (in some calendar systems)
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

  int i, month, months = calSys->monthsInYear( mDate );

  QPopupMenu *popup = new QPopupMenu( mMonth );

  for ( i = 1; i <= months; i++ )
    popup->insertItem( calSys->monthName( i, calSys->year( mDate ) ), i );

  popup->setActiveItem( calSys->month( mDate ) - 1 );
  popup->setMinimumWidth( mMonth->width() );

  if ( ( month = popup->exec( mMonth->mapToGlobal( QPoint( 0, 0 ) ),
                              calSys->month( mDate ) - 1 ) ) == -1 ) {
    delete popup;
    return;  // canceled
  }

  emit goMonth( month );

  delete popup;
}

#include "navigatorbar.moc"
