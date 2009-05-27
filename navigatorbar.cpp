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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "navigatorbar.h"
#include "koglobals.h"
#include "koprefs.h"

#include <kdebug.h>
#include <kcalendarsystem.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QString>
#include <QSpacerItem>
#include <QToolButton>

NavigatorBar::NavigatorBar( QWidget *parent ) : QWidget( parent )
{
  QFont tfont = font();
  tfont.setPointSize( 10 );
  tfont.setBold( false );

  bool isRTL = KOGlobals::self()->reverseLayout();

  // Create a horizontal spacers
  QSpacerItem *frontSpacer = new QSpacerItem( 50, 1, QSizePolicy::Expanding );
  QSpacerItem *endSpacer = new QSpacerItem( 50, 1, QSizePolicy::Expanding );

  mPrevYear = createNavigationButton(
    isRTL ? "arrow-right-double" : "arrow-left-double",
    i18n( "Scroll backward to the previous year" ) );

  mPrevMonth = createNavigationButton(
    isRTL ? "arrow-right" : "arrow-left",
    i18n( "Scroll backward to the previous month" ) );

  mNextMonth = createNavigationButton(
    isRTL ? "arrow-left" : "arrow-right",
    i18n( "Scroll forward to the next month" ) );

  mNextYear = createNavigationButton(
    isRTL ? "arrow-left-double" : "arrow-right-double",
    i18n( "Scroll forward to the next year" ) );

  // Create month name button
  mMonth = new QToolButton( this );
  mMonth->setPopupMode( QToolButton::InstantPopup );
  mMonth->setAutoRaise( true );
  mMonth->setFont( tfont );
  mMonth->setToolTip( i18n( "Select a month" ) );

  // set up control frame layout
  QHBoxLayout *ctrlLayout = new QHBoxLayout( this );
  ctrlLayout->addSpacerItem( frontSpacer );
  ctrlLayout->addWidget( mPrevYear );
  ctrlLayout->addWidget( mPrevMonth );
  ctrlLayout->addWidget( mMonth );
  ctrlLayout->addWidget( mNextMonth );
  ctrlLayout->addWidget( mNextYear );
  ctrlLayout->addSpacerItem( endSpacer );

  connect( mPrevYear, SIGNAL(clicked()), SIGNAL(goPrevYear()) );
  connect( mPrevMonth, SIGNAL(clicked()), SIGNAL(goPrevMonth()) );
  connect( mNextMonth, SIGNAL(clicked()), SIGNAL(goNextMonth()) );
  connect( mNextYear, SIGNAL(clicked()), SIGNAL(goNextYear()) );
  connect( mMonth, SIGNAL(clicked()), SLOT(selectMonth()) );
}

NavigatorBar::~NavigatorBar()
{
}

void NavigatorBar::showButtons( bool left, bool right )
{
  if ( left ) {
    mPrevYear->show();
    mPrevMonth->show();
  } else {
    mPrevYear->hide();
    mPrevMonth->hide();
  }

  if ( right ) {
    mNextYear->show();
    mNextMonth->show();
  } else {
    mNextYear->hide();
    mNextMonth->hide();
  }
}

void NavigatorBar::selectDates( const KCal::DateList &dateList )
{
  if ( dateList.count() > 0 ) {
    mDate = dateList.first();

    const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

    // set the label text at the top of the navigator
    mMonth->setText( i18nc( "monthname year", "%1 %2",
                            calSys->monthName( mDate ),
                            calSys->yearString( mDate ) ) );
  }
}

void NavigatorBar::selectMonth()
{
  // every year can have different month names (in some calendar systems)
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

  int i;
  int month = calSys->month( mDate );
  int year = calSys->year( mDate );
  int months = calSys->monthsInYear( mDate );

  QMenu *menu = new QMenu( mMonth );
  QList<QAction *>act;

  QAction *activateAction = 0;
  for ( i=1; i <= months; i++ ) {
    QAction *monthAction = menu->addAction( calSys->monthName( i, year ) );
    act.append( monthAction );
    if ( i == month ) {
      activateAction = monthAction;
    }
  }
  if ( activateAction ) {
    menu->setActiveAction( activateAction );
  }
  QAction *selectedAct = menu->exec( mMonth->mapToGlobal( QPoint( 0, 0 ) ) );
  if ( selectedAct && ( selectedAct != activateAction ) ) {
    for ( i=0; i < months; i++ ) {
      if ( act[i] == selectedAct ) {
        emit goMonth( i + 1 );
      }
    }
  }
  qDeleteAll( act );
  act.clear();
  delete menu;
}

QToolButton *NavigatorBar::createNavigationButton( const QString &icon,
                                                   const QString &toolTip )
{
  QToolButton *button = new QToolButton( this );

  button->setIcon(
    KIconLoader::global()->loadIcon( icon, KIconLoader::Desktop, KIconLoader::SizeSmall ) );
  button->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  button->setToolButtonStyle( Qt::ToolButtonIconOnly );
  button->setAutoRaise( true );
  button->setToolTip( toolTip );

  return button;
}

#include "navigatorbar.moc"
