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
