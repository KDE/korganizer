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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qheader.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <kdebug.h>

#include <libkcal/event.h>

#include "lineview.h"
#include "timeline.h"

#include "timespanwidget.h"
#include "timespanwidget.moc"

TimeSpanWidget::TimeSpanWidget( QWidget *parent, const char *name ) :
  QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  mSplitter = new QSplitter( this );
  topLayout->addWidget( mSplitter );

  mList = new QListView( mSplitter );
  mList->addColumn( i18n("Summary") );
  
  QWidget *rightPane = new QWidget( mSplitter );
  QBoxLayout *rightPaneLayout = new QVBoxLayout( rightPane );

  mTimeLine = new TimeLine( rightPane );
  mTimeLine->setFixedHeight( mList->header()->height() );
  rightPaneLayout->addWidget( mTimeLine );
  
  mLineView = new LineView( rightPane );
  rightPaneLayout->addWidget( mLineView );

  QBoxLayout *buttonLayout = new QHBoxLayout( rightPaneLayout );
  
  QPushButton *zoomInButton = new QPushButton( i18n("Zoom In"), rightPane );
  connect( zoomInButton, SIGNAL( clicked() ), SLOT( zoomIn() ) );
  buttonLayout->addWidget( zoomInButton );
  
  QPushButton *zoomOutButton = new QPushButton( i18n("Zoom Out"), rightPane );
  connect( zoomOutButton, SIGNAL( clicked() ), SLOT( zoomOut() ) );
  buttonLayout->addWidget( zoomOutButton );
  
  QPushButton *centerButton = new QPushButton( i18n("Center View"), rightPane );
  connect( centerButton, SIGNAL( clicked() ), SLOT( centerView() ) );
  buttonLayout->addWidget( centerButton );

  connect(mLineView->horizontalScrollBar(),SIGNAL(valueChanged(int)),
          mTimeLine,SLOT(setContentsPos(int)));
}

TimeSpanWidget::~TimeSpanWidget()
{
}

QValueList<int> TimeSpanWidget::splitterSizes()
{
  return mSplitter->sizes();
}

void TimeSpanWidget::setSplitterSizes( QValueList<int> sizes )
{
  mSplitter->setSizes( sizes );
}

void TimeSpanWidget::addItem( KCal::Event *event )
{
  new QListViewItem( mList, event->summary() );
  
  QDateTime startDt = event->dtStart();
  QDateTime endDt = event->dtEnd();

//  kdDebug(5850) << "TimeSpanWidget::addItem(): start: " << startDt.toString()
//            << "  end: " << endDt.toString() << endl;

//  int startSecs = mStartDate.secsTo( startDt );
//  int durationSecs = startDt.secsTo( endDt );
  
//  kdDebug(5850) << "--- startSecs: " << startSecs << "  dur: " << durationSecs << endl;

  int startX = mStartDate.secsTo( startDt ) / mSecsPerPixel;
  int endX = startX + startDt.secsTo( endDt ) / mSecsPerPixel;
  
//  kdDebug(5850) << "TimeSpanWidget::addItem(): s: " << startX << "  e: " << endX << endl;
  
  mLineView->addLine( startX, endX );
}

void TimeSpanWidget::clear()
{
  mList->clear();
  mLineView->clear();
}

void TimeSpanWidget::updateView()
{
#if QT_VERSION >= 300
  mLineView->updateContents();
  mTimeLine->updateContents();
#else
#endif
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
