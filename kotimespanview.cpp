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

  Event *event = new Event;
  event->setSummary( "Hallo" );
  event->setDtStart( QDate::currentDate() );
  event->setDtEnd( QDate::currentDate().addDays( 1 ) );

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

QPtrList<Incidence> KOTimeSpanView::getSelected()
{
  QPtrList<Incidence> selected;
  
  return selected;
}

void KOTimeSpanView::updateView()
{
}

void KOTimeSpanView::selectDates(const QDateList dateList)
{
}

void KOTimeSpanView::selectEvents(QPtrList<Event> eventList)
{
}

void KOTimeSpanView::changeEventDisplay(Event *, int)
{
}
