#include <qpainter.h>

#include "koprefs.h"

#include "lineview.h"
#include "lineview.moc"

LineView::LineView( QWidget *parent, const char *name ) :
  QScrollView( parent, name )
{
  int mPixelWidth = 1000;

  mLines.setAutoDelete( true );

  resizeContents( mPixelWidth, contentsHeight() );

  viewport()->setBackgroundColor(KOPrefs::instance()->mAgendaBgColor);
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
  int mGridSpacingY = 20;

#if 0
  // Draw vertical lines of grid
  //  kdDebug() << "drawContents cx: " << cx << " cy: " << cy << " cw: " << cw << " ch: " << ch << endl;
  int x = ((int)(cx/mGridSpacingX))*mGridSpacingX;
  while (x < cx + cw) {
    p->drawLine(x,cy,x,cy+ch);
    x+=mGridSpacingX;
  }
#endif

  // Draw horizontal lines of grid
  int y = ((int)(cy/mGridSpacingY))*mGridSpacingY + 10;
  while (y < cy + ch) {
//    kdDebug() << " y: " << y << endl;
    p->drawLine(cx,y,cx+cw,y);
    y+=mGridSpacingY;
  }
  
  Line *line;
  for( line = mLines.first(); line; line = mLines.next() ) {
    int ctop = line->column * 20 + 10 - 5;
    int cbottom = line->column * 20 + 10 + 5;
    int s = line->start * 5;
    int e = line->end * 5;
    if ( ctop <= (cy+ch) && cbottom >= cy &&
         s <= (cx+cw) && e >= cx ) {
      if ( s < cx ) s = cx;
      if ( e > (cx+cw) ) e = cx+cw;
      if ( ctop < cy ) ctop = cy;
      if ( cbottom > (cy+ch) ) cbottom = cy+ch;
      p->fillRect( s, ctop, e - s + 1, cbottom - ctop + 1, QBrush("red") );
    }
  }
}
