#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>

#include <klocale.h>

#include "lineview.h"

#include "timespanview.h"

TimeSpanView::TimeSpanView( QWidget *parent, const char *name ) :
  QWidget( parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  mSplitter = new QSplitter( this );
  topLayout->addWidget( mSplitter );
  
  mList = new QListView( mSplitter );
  mList->addColumn( i18n("Summary") );

  mLineView = new LineView( mSplitter );

  mLineView->addLine( 0, 10, 20 );
  mLineView->addLine( 1, 10, 30 );
  mLineView->addLine( 2, 20, 25 );
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
