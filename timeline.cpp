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

  // Draw vertical lines of grid
//  kdDebug() << "drawContents cx: " << cx << " cy: " << cy << " cw: " << cw << " ch: " << ch << endl;
  int cell = ((int)(cx/spacingX));
  int x = cell*spacingX;
//  kdDebug() << "  x: " << x << endl;
  while (x < cx + cw) {
//    kdDebug() << "    x: " << x << endl;
    p->drawLine(x,cy,x,cy+ch);    
    p->drawText( x + 5, 15, QString::number( mStartDate.addDays(cell).date().day() ) );

    x+=spacingX;
    cell++;
  }
}

void TimeLine::setDateRange( const QDateTime &start, const QDateTime &end )
{
  mStartDate = start;
  mEndDate = end;
  
  mDaySpacing = mPixelWidth / start.daysTo( end );
}

void TimeLine::setContentsPos( int pos )
{
  QScrollView::setContentsPos ( pos, 0 );
}
