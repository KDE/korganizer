#include <qlayout.h>

#include <kconfig.h>

#include "timespanview.h"
#include "koglobals.h"

#include "kotimespanview.h"
#include "kotimespanview.moc"

KOTimeSpanView::KOTimeSpanView(Calendar *calendar, QWidget *parent, 
               const char *name) :
  KOEventView( calendar, parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  
  mTimeSpanView = new TimeSpanView( this );
  topLayout->addWidget( mTimeSpanView );

  connect( mTimeSpanView, SIGNAL( dateRangeChanged() ), SLOT( updateView() ) );
}

KOTimeSpanView::~KOTimeSpanView()
{
}

void KOTimeSpanView::readSettings()
{
  readSettings(KOGlobals::config());
}

void KOTimeSpanView::readSettings(KConfig *config)
{
//  kdDebug() << "KOTimeSpanView::readSettings()" << endl;

  config->setGroup("Views");

  QValueList<int> sizes = config->readIntListEntry("Separator TimeSpanView");
  if (sizes.count() == 2) {
    mTimeSpanView->setSplitterSizes(sizes);
  }
}

void KOTimeSpanView::writeSettings(KConfig *config)
{
//  kdDebug() << "KOTimeSpanView::writeSettings()" << endl;

  config->setGroup("Views");

  QValueList<int> list = mTimeSpanView->splitterSizes();
  config->writeEntry("Separator TimeSpanView",list);
}

int KOTimeSpanView::maxDatesHint()
{
  return 0;
}

int KOTimeSpanView::currentDateCount()
{
  return 0;
}

QPtrList<Incidence> KOTimeSpanView::selectedIncidences()
{
  QPtrList<Incidence> selected;
  
  return selected;
}

void KOTimeSpanView::updateView()
{
  insertItems( mTimeSpanView->startDateTime().date(),
               mTimeSpanView->endDateTime().date() );
}

void KOTimeSpanView::showDates(const QDate &start, const QDate &end)
{
  QDate s = start.addDays( -2 );
  QDate e = end.addDays( 2 );

  insertItems( s, e );
}

void KOTimeSpanView::insertItems(const QDate &start, const QDate &end)
{
  mTimeSpanView->clear();
  mTimeSpanView->setDateRange( start, end );

  QPtrList<Event> events = calendar()->events( start, end );
  Event *event = events.first();
  while( event ) {
//    kdDebug() << "KOTimeSpanView::showDates() add event: " << event->summary()
//              << endl;

    mTimeSpanView->addItem( event );          

    event = events.next();
  }
  
  mTimeSpanView->updateView();
}

void KOTimeSpanView::showEvents(QPtrList<Event> eventList)
{
}

void KOTimeSpanView::changeEventDisplay(Event *, int)
{
}
