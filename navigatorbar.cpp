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

#include <QString>
#include <QPushButton>
#include <QLayout>
#include <QLabel>
#include <Q3PopupMenu>
#include <QMouseEvent>
#include <QPixmap>
#include <QHBoxLayout>
#include <QBoxLayout>

ActiveLabel::ActiveLabel( QWidget *parent ) : QLabel( parent )
{
}

void ActiveLabel::mouseReleaseEvent( QMouseEvent * )
{
  emit clicked();
}

NavigatorBar::NavigatorBar( QWidget *parent )
  : QWidget( parent ), mHasMinWidth( false )
{
  QFont tfont = font();
  tfont.setPointSize( 10 );
  tfont.setBold( false );

  bool isRTL = KOGlobals::self()->reverseLayout();

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
  mMonth = new ActiveLabel( this );
  mMonth->setFont( tfont );
  mMonth->setAlignment( Qt::AlignCenter );
  mMonth->setMinimumHeight( mPrevYear->sizeHint().height() );
  mMonth->setToolTip( i18n( "Select a month" ) );

  // set up control frame layout
  QBoxLayout *ctrlLayout = new QHBoxLayout( this );
  ctrlLayout->setSpacing( 4 );
  ctrlLayout->setMargin( 0 );
  ctrlLayout->addWidget( mPrevYear, 3 );
  ctrlLayout->addWidget( mPrevMonth, 3 );
  ctrlLayout->addWidget( mMonth, 3 );
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

    if ( !mHasMinWidth ) {
      // Set minimum width to width of widest month name label
      int i;
      int maxwidth = 0;

      for ( i = 1; i <= calSys->monthsInYear( mDate ); ++i ) {
        QString m = calSys->monthName( i, calSys->year( mDate ) );
        int w = QFontMetrics( mMonth->font() ).width( QString( "%1 8888" ).arg( m ) );
        if ( w > maxwidth ) {
          maxwidth = w;
        }
      }
      mMonth->setMinimumWidth( maxwidth );

      mHasMinWidth = true;
    }

    // compute the labels at the top of the navigator
    mMonth->setText( i18nc( "monthname year", "%1 %2",
                            calSys->monthName( mDate ),
                            calSys->yearString( mDate ) ) );
  }
}

void NavigatorBar::selectMonth()
{
  // every year can have different month names (in some calendar systems)
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

  int i, month, months = calSys->monthsInYear( mDate );

  Q3PopupMenu *popup = new Q3PopupMenu( mMonth );

  for ( i = 1; i <= months; i++ ) {
    popup->insertItem( calSys->monthName( i, calSys->year( mDate ) ), i );
  }
  popup->setActiveItem( calSys->month( mDate ) );

  month = popup->exec( mMonth->mapToGlobal( QPoint( 0, 0 ) ), calSys->month( mDate ) - 1 );

  delete popup;
  if ( month >= 0 ) {
    emit goMonth( month );
  }
}

QPushButton *NavigatorBar::createNavigationButton( const QString &icon,
  const QString &toolTip )
{
  QPushButton *button = new QPushButton( this );

  button->setIcon(
    KIconLoader::global()->loadIcon( icon, KIconLoader::Desktop, KIconLoader::SizeSmall ) );

  // By the default the button has a very wide minimum size (for whatever
  // reasons). Override this, so that the date navigator doesn't need to be
  // so wide anymore. The minimum size is dominated by the other elements of the
  // date navigator then.
  button->setMinimumSize( 10, 10 );

  button->setToolTip( toolTip );

  return button;
}

#include "navigatorbar.moc"
