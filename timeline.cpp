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

#include <qpainter.h>

#include <kdebug.h>

#include "timeline.h"
#include "timeline.moc"

TimeLine::TimeLine( QWidget *parent, const char *name ) :
  QScrollView( parent, name )
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

void TimeLine::drawContents(QPainter* p, int cx, int cy, int cw, int ch)
{
  int spacingX = mDaySpacing;
  int offsetX = mDayOffset;

  // Draw vertical lines of grid
//  kdDebug(5850) << "drawContents cx: " << cx << " cy: " << cy << " cw: " << cw << " ch: " << ch << endl;
  int cell = int( (cx - ( spacingX - offsetX ) ) / spacingX );
  int x = cell * spacingX + ( spacingX - offsetX );
//  kdDebug(5850) << "  x: " << x << endl;
  while (x < cx + cw) {
//    kdDebug(5850) << "    x: " << x << endl;
    p->drawLine(x,cy,x,cy+ch);
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

  kdDebug(5850) << "TimeLines::setDateRange(): mDaySpacing: " << mDaySpacing << "  mDayOffset: "
            << mDayOffset << "  mSecsPerPixel: " << mSecsPerPixel << endl;
}

void TimeLine::setContentsPos( int pos )
{
  QScrollView::setContentsPos ( pos, 0 );
}
