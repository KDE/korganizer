// $Id$
//
// View of Journal entries

#include <qlayout.h>
#include <qpopupmenu.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include <libkcal/calendar.h>

#include "journalentry.h"

#include "kojournalview.h"
#include "kojournalview.moc"


KOJournalView::KOJournalView(Calendar *calendar, QWidget *parent,
		       const char *name)
  : KOBaseView(calendar, parent, name)
{
  mEntry = new JournalEntry(calendar,this);
  
  QBoxLayout *topLayout = new QVBoxLayout(this);
  topLayout->addWidget(mEntry);
}

KOJournalView::~KOJournalView()
{
}

int KOJournalView::currentDateCount()
{
  return 0;
}

QPtrList<Incidence> KOJournalView::getSelected()
{
  QPtrList<Incidence> eventList;

  return eventList;
}

void KOJournalView::updateView()
{
  kdDebug() << "KOJournalView::updateView() does nothing" << endl;
}

void KOJournalView::flushView()
{
  mEntry->flushEntry();
}

void KOJournalView::selectDates(const QDateList dateList)
{
//  kdDebug() << "KOJournalView::selectDates()" << endl;

  QDateList dates = dateList;

  if (dateList.count() == 0) {
    kdDebug() << "KOJournalView::selectDates() called with empty list." << endl;
    return;
  }
  
  QDate date = *dates.first();

  mEntry->setDate(date);

  Journal *j = mCalendar->journal(date);
  if (j) mEntry->setJournal(j);
  else mEntry->clear();
  
//  emit eventsSelected(false);
}

void KOJournalView::selectEvents(QPtrList<Event> eventList)
{
  // After new creation of list view no events are selected.
//  emit eventsSelected(false);
}

void KOJournalView::changeEventDisplay(Event *event, int action)
{
  updateView();
}
