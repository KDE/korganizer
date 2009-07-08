/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include "timespanwidget.h"
#include "lineview.h"
#include "timeline.h"

#include <kcal/event.h>

#include <klocale.h>
#include <kdebug.h>

#include <QSplitter>
#include <q3listview.h>
#include <QLayout>
#include <q3header.h>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>

#include "timespanwidget.moc"

TimeSpanWidget::TimeSpanWidget( QWidget *parent ) : QWidget( parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  mSplitter = new QSplitter( this );
  topLayout->addWidget( mSplitter );

  mList = new Q3ListView( mSplitter );
  mList->addColumn( i18n( "Summary" ) );

  QWidget *rightPane = new QWidget( mSplitter );
  QBoxLayout *rightPaneLayout = new QVBoxLayout( rightPane );

  mTimeLine = new TimeLine( rightPane );
  mTimeLine->setFixedHeight( mList->header()->height() );
  rightPaneLayout->addWidget( mTimeLine );

  mLineView = new LineView( rightPane );
  rightPaneLayout->addWidget( mLineView );

  QBoxLayout *buttonLayout = new QHBoxLayout();
  rightPaneLayout->addItem( buttonLayout );

  QPushButton *zoomInButton = new QPushButton( i18n( "Zoom In" ), rightPane );
  connect( zoomInButton, SIGNAL(clicked()), SLOT(zoomIn()) );
  buttonLayout->addWidget( zoomInButton );

  QPushButton *zoomOutButton = new QPushButton( i18n( "Zoom Out" ), rightPane );
  connect( zoomOutButton, SIGNAL(clicked()), SLOT(zoomOut()) );
  buttonLayout->addWidget( zoomOutButton );

  QPushButton *centerButton = new QPushButton( i18n( "Center View" ), rightPane );
  connect( centerButton, SIGNAL(clicked()), SLOT(centerView()) );
  buttonLayout->addWidget( centerButton );

  connect( mLineView->horizontalScrollBar(), SIGNAL(valueChanged(int)),
           mTimeLine, SLOT(setContentsPos(int)) );
}

TimeSpanWidget::~TimeSpanWidget()
{
}

QList<int> TimeSpanWidget::splitterSizes()
{
  return mSplitter->sizes();
}

void TimeSpanWidget::setSplitterSizes( QList<int> sizes )
{
  if ( sizes.count() == 2 ) {
    mSplitter->setSizes( sizes );
  }
}

void TimeSpanWidget::addItem( KCal::Event *event )
{
  new Q3ListViewItem( mList, event->summary() );

  QDateTime startDt = event->dtStart().toTimeSpec( KOPrefs::instance()->timeSpec() ).dateTime();
  QDateTime endDt = event->dtEnd().toTimeSpec( KOPrefs::instance()->timeSpec() ).dateTime();

//  kDebug(5850) <<"TimeSpanWidget::addItem(): start:" << startDt.toString()
//            << " end:" << endDt.toString();

//  int startSecs = mStartDate.secsTo( startDt );
//  int durationSecs = startDt.secsTo( endDt );

//  kDebug(5850) <<"--- startSecs:" << startSecs <<"  dur:" << durationSecs;

  int startX = mStartDate.secsTo( startDt ) / mSecsPerPixel;
  int endX = startX + startDt.secsTo( endDt ) / mSecsPerPixel;

//  kDebug(5850) <<"TimeSpanWidget::addItem(): s:" << startX <<"  e:" << endX;

  mLineView->addLine( startX, endX );
}

void TimeSpanWidget::clear()
{
  mList->clear();
  mLineView->clear();
}

void TimeSpanWidget::updateView()
{
  mLineView->updateContents();
  mTimeLine->updateContents();
}

void TimeSpanWidget::setDateRange( const QDateTime &start, const QDateTime &end )
{
  mStartDate = start;
  mEndDate = end;

  mTimeLine->setDateRange( start, end );

  mSecsPerPixel = mStartDate.secsTo( mEndDate ) / mLineView->pixelWidth();
}

QDateTime TimeSpanWidget::startDateTime()
{
  return mStartDate;
}

QDateTime TimeSpanWidget::endDateTime()
{
  return mEndDate;
}

void TimeSpanWidget::zoomIn()
{
  int span = mStartDate.daysTo( mEndDate );
  setDateRange( mStartDate.addDays( span / 4 ), mEndDate.addDays( span / -4 ) );

  emit dateRangeChanged();
}

void TimeSpanWidget::zoomOut()
{
  int span = mStartDate.daysTo( mEndDate );
  setDateRange( mStartDate.addDays( span / -4 ), mEndDate.addDays( span / 4 ) );

  emit dateRangeChanged();
}

void TimeSpanWidget::centerView()
{
  QScrollBar *scrollBar = mLineView->horizontalScrollBar();
  int min = scrollBar->minValue();
  int max = scrollBar->maxValue();
  scrollBar->setValue( min + (max-min) / 2 );
}
