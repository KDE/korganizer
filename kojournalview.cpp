// $Id$
//
// View of Journal entries

#include <qlayout.h>
#include <qpopupmenu.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include "calobject.h"
#include "journalentry.h"

#include "kojournalview.h"
#include "kojournalview.moc"


KOJournalView::KOJournalView(CalObject *calendar, QWidget *parent,
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

QList<Incidence> KOJournalView::getSelected()
{
  QList<Incidence> eventList;

  return eventList;
}

void KOJournalView::updateView()
{
  kdDebug() << "KOJournalView::updateView() does nothing" << endl;
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

void KOJournalView::selectEvents(QList<Event> eventList)
{
  // After new creation of list view no events are selected.
//  emit eventsSelected(false);
}

void KOJournalView::changeEventDisplay(Event *event, int action)
{
  updateView();
}
