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

NavigatorBar::NavigatorBar( const QDate & date, QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QBoxLayout *topLayout = new QHBoxLayout( this );

  // Set up the control buttons and date label
  mCtrlFrame = new QFrame( this );
  mCtrlFrame->setFrameStyle(QFrame::Panel|QFrame::Raised);
  mCtrlFrame->setLineWidth(1);

  topLayout->addWidget( mCtrlFrame );

  QFont tfont = font();
  tfont.setPointSize(10);
  tfont.setBold(FALSE);

  bool isRTL = KOGlobals::self()->reverseLayout();

  // Create backward navigation buttons
  mPrevYear = new QPushButton( mCtrlFrame );
  mPrevYear->setPixmap( SmallIcon( isRTL ? "2rightarrow" : "2leftarrow" ) );
  QToolTip::add( mPrevYear, i18n("Previous Year") );

  mPrevMonth = new QPushButton( mCtrlFrame );
  mPrevMonth->setPixmap( SmallIcon( isRTL ? "1rightarrow" : "1leftarrow") );
  QToolTip::add( mPrevMonth, i18n("Previous Month") );

  // Create forward navigation buttons
  mNextMonth = new QPushButton( mCtrlFrame );
  mNextMonth->setPixmap( SmallIcon( isRTL ? "1leftarrow" : "1rightarrow") );
  QToolTip::add( mNextMonth, i18n("Next Month") );

  mNextYear = new QPushButton( mCtrlFrame );
  mNextYear->setPixmap( SmallIcon( isRTL ? "2leftarrow" : "2rightarrow") );
  QToolTip::add( mNextYear, i18n("Next Year") );

  // Create month name label
  mDateLabel = new QLabel( mCtrlFrame );
  mDateLabel->setFont( tfont );
  mDateLabel->setAlignment( AlignCenter );

  // Set minimum width to width of widest month name label
  int i;
  int maxwidth = 0;
  QFontMetrics fm = mDateLabel->fontMetrics();

  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();
  for( i = 1; i <= calSys->monthsInYear(date); ++i ) {
    QString dtstr(i18n("monthname year", "%1 %2"));
    dtstr=dtstr.arg( calSys->monthName( i, calSys->year(date) ) )
          .arg( calSys->year( date ) );
    int width = fm.width( dtstr );
    if ( width > maxwidth ) maxwidth = width;
  }
  mDateLabel->setMinimumWidth( maxwidth );

  // set up control frame layout
  QBoxLayout *ctrlLayout = new QHBoxLayout( mCtrlFrame, 1 );
  ctrlLayout->addWidget( mPrevYear, 3 );
  ctrlLayout->addWidget( mPrevMonth, 3 );
  ctrlLayout->addStretch( 1 );
  ctrlLayout->addSpacing( 2 );
  ctrlLayout->addWidget( mDateLabel );
  ctrlLayout->addSpacing( 2 );
  ctrlLayout->addStretch( 1 );
  ctrlLayout->addWidget( mNextMonth, 3 );
  ctrlLayout->addWidget( mNextYear, 3 );

  connect( mPrevYear, SIGNAL( clicked() ), SIGNAL( goPrevYear() ) );
  connect( mPrevMonth, SIGNAL( clicked() ), SIGNAL( goPrevMonth() ) );
  connect( mNextMonth, SIGNAL( clicked() ), SIGNAL( goNextMonth() ) );
  connect( mNextYear, SIGNAL( clicked() ), SIGNAL( goNextYear() ) );
}

NavigatorBar::~NavigatorBar()
{
}

void NavigatorBar::selectDates( const KCal::DateList &dateList )
{
  if (dateList.count() > 0) {
    QDate date = dateList.first();

    const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

    // compute the label at the top of the navigator
    QString dtstr(i18n("monthname year", "%1 %2"));
    dtstr=dtstr.arg( calSys->monthName( date ) )
          .arg( calSys->year( date ) );

    mDateLabel->setText( dtstr );
  }
}

#include "navigatorbar.moc"
