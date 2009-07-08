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

#include "timeline.h"

#include <kdebug.h>

#include <QPainter>

#include "timeline.moc"

TimeLine::TimeLine( QWidget *parent ) : Q3ScrollView( parent )
{
  mPixelWidth = 1000;

  resizeContents( mPixelWidth, 20 );

  viewport()->setBackgroundMode( PaletteBackground );

  setHScrollBarMode(AlwaysOff);
  setVScrollBarMode(AlwaysOff);
}

TimeLine::~TimeLine()
{
}

void TimeLine::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
  int spacingX = mDaySpacing;
  int offsetX = mDayOffset;

  // Draw vertical lines of grid
//  kDebug() << "cx:" << cx <<" cy:" << cy <<" cw:" << cw <<" ch:" << ch;
  int cell = int( ( cx - ( spacingX - offsetX ) ) / spacingX );
  int x = cell * spacingX + ( spacingX - offsetX );
//  kDebug() << "  x:" << x;
  while ( x < cx + cw ) {
//    kDebug() << "    x:" << x;
    p->drawLine( x, cy, x, cy + ch );
    p->drawText( x + 5, 15, QString::number( mStartDate.addDays( cell + 1 ).date().day() ) );

    x += spacingX;
    cell++;
  }
}

void TimeLine::setDateRange( const QDateTime &start, const QDateTime &end )
{
  mStartDate = start;
  mEndDate = end;

  mSecsPerPixel = mStartDate.secsTo( mEndDate ) / mPixelWidth;

  mDaySpacing = 60 * 60 * 24 / mSecsPerPixel;

  mDayOffset = QDateTime( mStartDate.date() ).secsTo( mStartDate ) / mSecsPerPixel;

  kDebug() << "mDaySpacing:" << mDaySpacing
           << "mDayOffset:" << mDayOffset
           << " mSecsPerPixel: " << mSecsPerPixel;
}

void TimeLine::setContentsPos( int pos )
{
  Q3ScrollView::setContentsPos ( pos, 0 );
}
