#include <qpainter.h>

#include "lineview.h"

LineView::LineView( QWidget *parent, const char *name ) :
  QScrollView( parent, name )
{
  mLines.setAutoDelete( true );
}

LineView::~LineView()
{
}

void LineView::addLine( int line, int start, int end )
{
  mLines.append( new Line( line, start, end ) );
}

void LineView::drawContents(QPainter* p, int cx, int cy, int cw, int ch)
{
  int mGridSpacingX = 10;
  int mGridSpacingY = 10;

  // Draw horizontal lines of grid
  //  kdDebug() << "drawContents cx: " << cx << " cy: " << cy << " cw: " << cw << " ch: " << ch << endl;
  int x = ((int)(cx/mGridSpacingX))*mGridSpacingX;
  while (x < cx + cw) {
    p->drawLine(x,cy,x,cy+ch);
    x+=mGridSpacingX;
  }

  // Draw vertical lines of grid
  int y = ((int)(cy/mGridSpacingY))*mGridSpacingY;
  while (y < cy + ch) {
//    kdDebug() << " y: " << y << endl;
    p->drawLine(cx,y,cx+cw,y);
    y+=mGridSpacingY;
  }
  
  Line *line;
  for( line = mLines.first(); line; line = mLines.next() ) {
    int c = line->column * 20 + 10;
    int s = line->start * 5;
    int e = line->end * 5;
    if ( c >= cy && c <= (cy+ch) ) {
      int ss = ( s > cx ) ? s : cx;
      int ee = ( e < (cx+cw) ) ? e : (cx+cw);
      int ctop = ( (c-5) > cy ) ? (c-5) : cy;
      int cbottom = ( (c+5) < (cy+ch) ) ? (c+5) : (cy+ch);
      p->fillRect( ss, ctop, ee - ss, cbottom - ctop, QBrush("red") );
    }
  }
}
