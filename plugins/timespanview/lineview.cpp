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

#include "lineview.h"
#include "koprefs.h"

#include <kdebug.h>

#include <QPainter>

#include "lineview.moc"

LineView::LineView( QWidget *parent ) : Q3ScrollView( parent )
{
  mPixelWidth = 1000;

  mLines.setAutoDelete( true );

  resizeContents( mPixelWidth, contentsHeight() );

  viewport()->setBackgroundColor( KOPrefs::instance()->mAgendaBgColor );
}

LineView::~LineView()
{
}

int LineView::pixelWidth()
{
  return mPixelWidth;
}

void LineView::addLine( int start, int end )
{
  int count = mLines.count();

  if( start < 0 ) {
    start = 0;
  }
  if ( end > mPixelWidth ) {
    end = mPixelWidth;
  }

  kDebug() << "col:" << count << "start:" << start << "end:" << end;

  mLines.append( new Line( count, start, end ) );
}

void LineView::clear()
{
  mLines.clear();
  update();
}

void LineView::drawContents( QPainter *p, int cx, int cy, int cw, int ch )
{
//  kDebug();

//  int mGridSpacingX = 10;
  int mGridSpacingY = 20;

#if 0
  // Draw vertical lines of grid
  //  kDebug() << "cx:" << cx <<" cy:" << cy <<" cw:" << cw <<" ch:" << ch;
  int x = ( (int)( cx / mGridSpacingX ) ) * mGridSpacingX;
  while ( x < cx + cw ) {
    p->drawLine( x, cy, x, cy + ch );
    x += mGridSpacingX;
  }
#endif

  // Draw horizontal lines of grid
  int y = ( (int)( cy / mGridSpacingY ) ) * mGridSpacingY + 10;
  while ( y < cy + ch ) {
//    kDebug() << " y:" << y;
    p->drawLine( cx, y, cx + cw, y );
    y += mGridSpacingY;
  }

  Line *line;
  for ( line = mLines.first(); line; line = mLines.next() ) {
    int ctop = line->column * 20 + 10 - 5;
    int cbottom = line->column * 20 + 10 + 5;
    int s = line->start;
    int e = line->end;
//    kDebug() << "ctop:" << ctop << " cbottom:" << cbottom
//             << " s:" << s << " e:" << e;
    if ( ctop <= ( cy + ch ) &&
         cbottom >= cy &&
         s <= ( cx + cw ) &&
         e >= cx ) {
      if ( s < cx ) {
        s = cx;
      }
      if ( e > ( cx + cw ) ) {
        e = cx + cw;
      }
      if ( ctop < cy ) {
        ctop = cy;
      }
      if ( cbottom > ( cy + ch ) ) {
        cbottom = cy + ch;
      }
//      kDebug() <<"            ctop:" << ctop <<" cbottom:"
//                << cbottom << " s:" << s << " e:" << e;
      p->fillRect( s, ctop, e - s + 1, cbottom - ctop + 1, QBrush( "red" ) );
    }
  }
}
