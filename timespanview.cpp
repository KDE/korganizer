#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qheader.h>

#include <klocale.h>
#include <kdebug.h>

#include "lineview.h"
#include "timeline.h"

#include "timespanview.h"
#include "timespanview.moc"

TimeSpanView::TimeSpanView( QWidget *parent, const char *name ) :
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

  connect(mLineView->horizontalScrollBar(),SIGNAL(valueChanged(int)),
          mTimeLine,SLOT(setContentsPos(int)));
}

TimeSpanView::~TimeSpanView()
{
}

QValueList<int> TimeSpanView::splitterSizes()
{
  return mSplitter->sizes();
}

void TimeSpanView::setSplitterSizes( QValueList<int> sizes )
{
  mSplitter->setSizes( sizes );
}

void TimeSpanView::addItem( KCal::Event *event )
{
  new QListViewItem( mList, event->summary() );
  
  QDateTime startDt = event->dtStart();
  QDateTime endDt = event->dtEnd();

  kdDebug() << "TimeSpanView::addItem(): start: " << startDt.toString()
            << "  end: " << endDt.toString() << endl;

  int startSecs = mStartDate.secsTo( startDt );
  int durationSecs = startDt.secsTo( endDt );
  
  kdDebug() << "--- startSecs: " << startSecs << "  dur: " << durationSecs << endl;

  int startX = mStartDate.secsTo( startDt ) / mSecsPerPixel;
  int endX = startX + startDt.secsTo( endDt ) / mSecsPerPixel;
  
  kdDebug() << "TimeSpanView::addItem(): s: " << startX << "  e: " << endX << endl;
  
  mLineView->addLine( startX, endX );
}

void TimeSpanView::setDateRange( const QDateTime &start, const QDateTime &end )
{
  mStartDate = start;
  mEndDate = end;
  
  mTimeLine->setDateRange( start, end );

  mSecsPerPixel = mStartDate.secsTo( mEndDate ) / mLineView->pixelWidth();
}
