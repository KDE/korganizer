#include <qsplitter.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qheader.h>
#include <qpushbutton.h>

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

  QBoxLayout *buttonLayout = new QHBoxLayout( rightPaneLayout );
  
  QPushButton *zoomInButton = new QPushButton( i18n("Zoom In"), rightPane );
  connect( zoomInButton, SIGNAL( clicked() ), SLOT( zoomIn() ) );
  buttonLayout->addWidget( zoomInButton );
  
  QPushButton *zoomOutButton = new QPushButton( i18n("Zoom Out"), rightPane );
  connect( zoomOutButton, SIGNAL( clicked() ), SLOT( zoomOut() ) );
  buttonLayout->addWidget( zoomOutButton );
  
  QPushButton *centerButton = new QPushButton( i18n("Center View"), rightPane );
  connect( centerButton, SIGNAL( clicked() ), SLOT( centerView() ) );
  buttonLayout->addWidget( centerButton );

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

//  kdDebug(5850) << "TimeSpanView::addItem(): start: " << startDt.toString()
//            << "  end: " << endDt.toString() << endl;

  int startSecs = mStartDate.secsTo( startDt );
  int durationSecs = startDt.secsTo( endDt );
  
//  kdDebug(5850) << "--- startSecs: " << startSecs << "  dur: " << durationSecs << endl;

  int startX = mStartDate.secsTo( startDt ) / mSecsPerPixel;
  int endX = startX + startDt.secsTo( endDt ) / mSecsPerPixel;
  
//  kdDebug(5850) << "TimeSpanView::addItem(): s: " << startX << "  e: " << endX << endl;
  
  mLineView->addLine( startX, endX );
}

void TimeSpanView::clear()
{
  mList->clear();
  mLineView->clear();
}

void TimeSpanView::updateView()
{
#if QT_VERSION >= 300
  mLineView->updateContents();
  mTimeLine->updateContents();
#else
#endif
}

void TimeSpanView::setDateRange( const QDateTime &start, const QDateTime &end )
{
  mStartDate = start;
  mEndDate = end;
  
  mTimeLine->setDateRange( start, end );

  mSecsPerPixel = mStartDate.secsTo( mEndDate ) / mLineView->pixelWidth();
}

QDateTime TimeSpanView::startDateTime()
{
  return mStartDate;
}

QDateTime TimeSpanView::endDateTime()
{
  return mEndDate;
}

void TimeSpanView::zoomIn()
{
  int span = mStartDate.daysTo( mEndDate );
  setDateRange( mStartDate.addDays( span / 4 ), mEndDate.addDays( span / -4 ) );

  emit dateRangeChanged();
}

void TimeSpanView::zoomOut()
{
  int span = mStartDate.daysTo( mEndDate );
  setDateRange( mStartDate.addDays( span / -4 ), mEndDate.addDays( span / 4 ) );

  emit dateRangeChanged();
}

void TimeSpanView::centerView()
{
  QScrollBar *scrollBar = mLineView->horizontalScrollBar();
  int min = scrollBar->minValue();
  int max = scrollBar->maxValue();
  scrollBar->setValue( min + (max-min) / 2 );
}
