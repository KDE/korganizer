#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qheader.h>

#include <klocale.h>

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

  mLineView->addLine( 0, 10, 20 );
  mLineView->addLine( 1, 10, 30 );
  mLineView->addLine( 2, 20, 25 );

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
}

void TimeSpanView::setDateRange( const QDateTime &start, const QDateTime &end )
{
  mStartDate = start;
  mEndDate = end;
  
  mTimeLine->setDateRange( start, end );
}
