#include <qpainter.h>

#include <kdebug.h>

#include "koprefs.h"

#include "lineview.h"
#include "lineview.moc"

LineView::LineView( QWidget *parent, const char *name ) :
  QScrollView( parent, name )
{
  mPixelWidth = 1000;

  mLines.setAutoDelete( true );

  resizeContents( mPixelWidth, contentsHeight() );

  viewport()->setBackgroundColor(KOPrefs::instance()->mAgendaBgColor);
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
  
  if( start < 0 ) start = 0;
  if( end > mPixelWidth) end = mPixelWidth;
  
  kdDebug(5850) << "LineView::addLine() col: " << count << "  start: " << start
            << "  end: " << end << endl;
  
  mLines.append( new Line( count, start, end ) );
}

void LineView::clear()
{
  mLines.clear();
  update();
}

void LineView::drawContents(QPainter* p, int cx, int cy, int cw, int ch)
{
//  kdDebug(5850) << "LineView::drawContents()" << endl;

//  int mGridSpacingX = 10;
  int mGridSpacingY = 20;

#if 0
  // Draw vertical lines of grid
  //  kdDebug(5850) << "drawContents cx: " << cx << " cy: " << cy << " cw: " << cw << " ch: " << ch << endl;
  int x = ((int)(cx/mGridSpacingX))*mGridSpacingX;
  while (x < cx + cw) {
    p->drawLine(x,cy,x,cy+ch);
    x+=mGridSpacingX;
  }
#endif

  // Draw horizontal lines of grid
  int y = ((int)(cy/mGridSpacingY))*mGridSpacingY + 10;
  while (y < cy + ch) {
//    kdDebug(5850) << " y: " << y << endl;
    p->drawLine(cx,y,cx+cw,y);
    y+=mGridSpacingY;
  }
  
  Line *line;
  for( line = mLines.first(); line; line = mLines.next() ) {
    int ctop = line->column * 20 + 10 - 5;
    int cbottom = line->column * 20 + 10 + 5;
    int s = line->start;
    int e = line->end;
//    kdDebug(5850) << "  LineView::drawContents(): ctop: " << ctop << "  cbottom: "
//              << cbottom << "  s: " << s << "  e: " << e << endl;
    if ( ctop <= (cy+ch) && cbottom >= cy &&
         s <= (cx+cw) && e >= cx ) {
      if ( s < cx ) s = cx;
      if ( e > (cx+cw) ) e = cx+cw;
      if ( ctop < cy ) ctop = cy;
      if ( cbottom > (cy+ch) ) cbottom = cy+ch;
//      kdDebug(5850) << "            drawContents(): ctop: " << ctop << "  cbottom: "
//                << cbottom << "  s: " << s << "  e: " << e << endl;
      p->fillRect( s, ctop, e - s + 1, cbottom - ctop + 1, QBrush("red") );
    }
  }
}
