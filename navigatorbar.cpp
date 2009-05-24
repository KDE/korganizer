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

#include <KCalendarSystem>
#include <KIconLoader>
#include <KLocale>

#include <QHBoxLayout>
#include <QMenu>
#include <QSpacerItem>
#include <QToolButton>

ActiveLabel::ActiveLabel( QWidget *parent ) : QLabel( parent )
{
}

void ActiveLabel::enterEvent( QEvent * )
{
  setFrameStyle( QFrame::Panel | QFrame::Raised );
  setLineWidth( 1 );
}

void ActiveLabel::leaveEvent( QEvent * )
{
  setFrameStyle( QFrame::NoFrame );
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

  // Create a horizontal spacers
  QSpacerItem *frontSpacer = new QSpacerItem( 50, 1, QSizePolicy::Expanding );
  QSpacerItem *endSpacer = new QSpacerItem( 50, 1, QSizePolicy::Expanding );

  mPrevYear = createNavigationButton(
    isRTL ? "arrow-right-double" : "arrow-left-double",
    i18n( "Scroll backward to the previous year" ),
    i18n( "Click this button to scroll the display to the "
          "same approximate day of the previous year" ) );

  mPrevMonth = createNavigationButton(
    isRTL ? "arrow-right" : "arrow-left",
    i18n( "Scroll backward to the previous month" ),
    i18n( "Click this button to scroll the display to the "
          "same approximate date of the previous month" ) );

  mNextMonth = createNavigationButton(
    isRTL ? "arrow-left" : "arrow-right",
    i18n( "Scroll forward to the next month" ),
    i18n( "Click this button to scroll the display to the "
          "same approximate date of the next month" ) );

  mNextYear = createNavigationButton(
    isRTL ? "arrow-left-double" : "arrow-right-double",
    i18n( "Scroll forward to the next year" ),
    i18n( "Click this button to scroll the display to the "
          "same approximate day of the next year" ) );

  // Create month name button
  mMonth = new ActiveLabel( this );
  mMonth->setFont( tfont );
  mMonth->setAlignment( Qt::AlignCenter );
  mMonth->setMinimumHeight( mPrevYear->sizeHint().height() );
  mMonth->setToolTip( i18n( "Select a month" ) );

  // Create year button
  mYear = new ActiveLabel( this );
  mYear->setFont( tfont );
  mYear->setAlignment( Qt::AlignCenter );
  mYear->setMinimumHeight( mPrevYear->sizeHint().height() );
  mYear->setToolTip( i18n( "Select a year" ) );

  // set up control frame layout
  QHBoxLayout *ctrlLayout = new QHBoxLayout( this );
  ctrlLayout->addSpacerItem( frontSpacer );
  ctrlLayout->addWidget( mPrevYear );
  ctrlLayout->addWidget( mPrevMonth );
  ctrlLayout->addWidget( mMonth );
  ctrlLayout->addWidget( mYear );
  ctrlLayout->addWidget( mNextMonth );
  ctrlLayout->addWidget( mNextYear );
  ctrlLayout->addSpacerItem( endSpacer );

  connect( mPrevYear, SIGNAL(clicked()), SIGNAL(goPrevYear()) );
  connect( mPrevMonth, SIGNAL(clicked()), SIGNAL(goPrevMonth()) );
  connect( mNextMonth, SIGNAL(clicked()), SIGNAL(goNextMonth()) );
  connect( mNextYear, SIGNAL(clicked()), SIGNAL(goNextYear()) );
  connect( mMonth, SIGNAL(clicked()), SLOT(selectMonthFromMenu()) );
  connect( mYear, SIGNAL(clicked()), SLOT(selectYearFromMenu()) );
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
        int w = QFontMetrics( mMonth->font() ).width( QString( "%1" ).arg( m ) );
        if ( w > maxwidth ) {
          maxwidth = w;
        }
      }
      mMonth->setMinimumWidth( maxwidth );

      mHasMinWidth = true;
    }

    // set the label text at the top of the navigator
    mMonth->setText( i18nc( "monthname", "%1", calSys->monthName( mDate ) ) );
    mYear->setText( i18nc( "4 digit year", "%1", calSys->yearString( mDate ) ) );
  }
}

void NavigatorBar::selectMonthFromMenu()
{
  // every year can have different month names (in some calendar systems)
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

  int month = calSys->month( mDate );
  int year = calSys->year( mDate );
  int months = calSys->monthsInYear( mDate );

  QMenu *menu = new QMenu( mMonth );
  QList<QAction *>act;

  QAction *activateAction = 0;
  for ( int i=1; i <= months; ++i ) {
    QAction *monthAction = menu->addAction( calSys->monthName( i, year ) );
    act.append( monthAction );
    if ( i == month ) {
      activateAction = monthAction;
    }
  }
  if ( activateAction ) {
    menu->setActiveAction( activateAction );
  }
  month = 0;
  QAction *selectedAct = menu->exec( mMonth->mapToGlobal( QPoint( 0, 0 ) ) );
  if ( selectedAct && ( selectedAct != activateAction ) ) {
    for ( int i=0; i < months; i++ ) {
      if ( act[i] == selectedAct ) {
        month = i + 1;
      }
    }
  }
  qDeleteAll( act );
  act.clear();
  delete menu;

  if ( month > 0 ) {
    emit goMonth( month );
  }
}

void NavigatorBar::selectYearFromMenu()
{
  const KCalendarSystem *calSys = KOGlobals::self()->calendarSystem();

  int year = calSys->year( mDate );
  int years = 11;  // odd number (show a few years ago -> a few years from now)
  int minYear = year - ( years / 3 );

  QMenu *menu = new QMenu( mYear );
  QList<QAction *>act;

  QString yearStr;
  QAction *activateAction = 0;
  int y = minYear;
  for ( int i=0; i < years; i++ ) {
    QAction *yearAction = menu->addAction( yearStr.setNum( y ) );
    act.append( yearAction );
    if ( y == year ) {
      activateAction = yearAction;
    }
    y++;
  }
  if ( activateAction ) {
    menu->setActiveAction( activateAction );
  }
  year = 0;
  QAction *selectedAct = menu->exec( mYear->mapToGlobal( QPoint( 0, 0 ) ) );
  if ( selectedAct && ( selectedAct != activateAction ) ) {
    int y = minYear;
    for ( int i=0; i < years; i++ ) {
      if ( act[i] == selectedAct ) {
        year = y;
      }
      y++;
    }
  }
  qDeleteAll( act );
  act.clear();
  delete menu;

  if ( year > 0 ) {
    emit goYear( year );
  }
}

QToolButton *NavigatorBar::createNavigationButton( const QString &icon,
                                                   const QString &toolTip,
                                                   const QString &whatsThis )
{
  QToolButton *button = new QToolButton( this );

  button->setIcon(
    KIconLoader::global()->loadIcon( icon, KIconLoader::Desktop, KIconLoader::SizeSmall ) );
  button->setIconSize( QSize( KIconLoader::SizeSmall, KIconLoader::SizeSmall ) );
  button->setToolButtonStyle( Qt::ToolButtonIconOnly );
  button->setToolTip( toolTip );
  button->setWhatsThis( whatsThis );

  return button;
}

#include "navigatorbar.moc"
