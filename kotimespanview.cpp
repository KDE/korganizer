#include <qlayout.h>

#include <kglobal.h>
#include <kconfig.h>

#include "timespanview.h"

#include "kotimespanview.h"
#include "kotimespanview.moc"

KOTimeSpanView::KOTimeSpanView(Calendar *calendar, QWidget *parent, 
               const char *name) :
  KOEventView( calendar, parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  
  mTimeSpanView = new TimeSpanView( this );
  topLayout->addWidget( mTimeSpanView );

  // Debug: Set some values manually

#if 0
  mTimeSpanView->setDateRange( QDateTime::currentDateTime(),
                               QDateTime::currentDateTime().addDays( 6 ) );
#else
  mTimeSpanView->setDateRange( QDateTime( QDate( 2001, 11, 23 ),
                                          QTime( 18, 0 ) ),
                               QDateTime( QDate( 2001, 11, 29 ),
                                          QTime( 6, 0 ) ) );
#endif

  Event *event = new Event;
  event->setSummary( "Hallo" );
  event->setDtStart( QDateTime::currentDateTime().addDays( 1 ) );
  event->setDtEnd( QDateTime::currentDateTime().addDays( 2 ) );

  mTimeSpanView->addItem( event );

  event = new Event;
  event->setSummary( "Dudelidei" );
  event->setDtStart( QDateTime::currentDateTime().addDays( 1 ) );
  event->setDtEnd( QDateTime::currentDateTime().addDays( 3 ) );

  mTimeSpanView->addItem( event );

  event = new Event;
  event->setSummary( "Trumtum" );
  event->setDtStart( QDateTime::currentDateTime().addDays( 2 ) );
  event->setDtEnd( QDateTime::currentDateTime().addDays( 3 ) );

  mTimeSpanView->addItem( event );
  
  event = new Event;
  event->setSummary( "Exactly" );
  event->setDtStart( QDateTime( QDate( 2001, 11, 24 ), QTime( 0, 0 ) ) );
  event->setDtEnd( QDateTime( QDate( 2001, 11, 27 ), QTime( 12, 0 ) ) );

  mTimeSpanView->addItem( event );
}

KOTimeSpanView::~KOTimeSpanView()
{
}

void KOTimeSpanView::readSettings()
{
  readSettings(KGlobal::config());
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
}

void KOTimeSpanView::showDates(const QDate &start, const QDate &end)
{
}

void KOTimeSpanView::showEvents(QPtrList<Event> eventList)
{
}

void KOTimeSpanView::changeEventDisplay(Event *, int)
{
}
