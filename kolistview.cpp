/*
 * KOListView provides a view of events in a list.
 * This file is part of the KOrganizer project.
 * (c) 1999 Preston Brown <pbrown@kde.org>
 */

#include <qlistview.h>
#include <qlayout.h>
#include <qpopupmenu.h>

#include <klocale.h>
#include <kiconloader.h>

#include "calobject.h"
#include "calprinter.h"
#include "calendarview.h"

#include "kolistview.h"
#include "kolistview.moc"


KOListViewItem::KOListViewItem(QListView *parent, KOEvent *ev)
  : QListViewItem(parent), mEvent(ev)
{
  setText(0, mEvent->getSummary());
  setText(1, mEvent->getDtStart().date().toString());
  setText(2, mEvent->getDtStart().time().toString());
  setText(3, mEvent->getDtEnd().date().toString());
  setText(4, mEvent->getDtEnd().time().toString());
  setText(5, mEvent->getAlarmRepeatCount() ? "Yes" : "No");
  setText(6, mEvent->doesRecur() ? "Yes" : "No");
}


KOListView::KOListView(CalObject *calendar, QWidget *parent,
		       const char *name)
  : KOBaseView(calendar, parent, name)
{
  mActiveItem = 0;

  mListView = new QListView(this);
  mListView->addColumn("Summary");
  mListView->addColumn("Start Date");
  mListView->addColumn("Start Time");
  mListView->addColumn("End Date");
  mListView->addColumn("End Time");
  mListView->addColumn("Alarm"); // alarm set?
  mListView->addColumn("Recurs"); // recurs?

  QBoxLayout *layoutTop = new QVBoxLayout(this);
  layoutTop->addWidget(mListView);

  mPopupMenu = eventPopup();
/*
  mPopupMenu = new QPopupMenu;
  mPopupMenu->insertItem(i18n("Edit Event"), this,
		     SLOT (editEvent()));
  mPopupMenu->insertItem(BarIcon("delete"), i18n("Delete Event  "), this,
		     SLOT (deleteEvent()));
  mPopupMenu->insertSeparator();
  mPopupMenu->insertItem(i18n("Show Dates"), this,
		      SLOT(showDates()));
  mPopupMenu->insertItem(i18n("Hide Dates"), this,
		      SLOT(hideDates()));
*/
  
  QObject::connect(mListView,SIGNAL(doubleClicked(QListViewItem *)),
                   this,SLOT(defaultItemAction(QListViewItem *)));
  QObject::connect(mListView,SIGNAL(rightButtonClicked ( QListViewItem *,
                     const QPoint &, int )),
                   this,SLOT(popupMenu(QListViewItem *,const QPoint &,int)));
}

KOListView::~KOListView()
{
  delete mPopupMenu;
}

int KOListView::maxDatesHint()
{
  return 1;
}

QList<KOEvent> KOListView::getSelected()
{
  QList<KOEvent> eventList;
  return eventList;
}

void KOListView::showDates(bool show)
{
  // Shouldn't we set it to a value greater 0? When showDates is called with
  // show == true at first, then the columnwidths are set to zero.
  static int oldColWidth1 = 0; 
  static int oldColWidth3 = 0;

  if (!show) {
    oldColWidth1 = mListView->columnWidth(1);
    oldColWidth3 = mListView->columnWidth(3);
    mListView->setColumnWidth(1, 0);
    mListView->setColumnWidth(3, 0);
  } else {
    mListView->setColumnWidth(1, oldColWidth1);
    mListView->setColumnWidth(3, oldColWidth3);
  } 
  mListView->repaint();
}

void KOListView::printPreview(CalPrinter *calPrinter, const QDate &fd,
                               const QDate &td)
{
  calPrinter->preview(CalPrinter::Day, fd, td);
}

inline void KOListView::showDates()
{
  showDates(true);
}

inline void KOListView::hideDates()
{
  showDates(false);
}

void KOListView::updateView()
{
  qDebug("KOListView::updateView() does nothing");
}

void KOListView::selectDates(const QDateList dateList)
{
  QDateList tmpList(FALSE);
  tmpList = dateList;

  mListView->clear();

  QDate *date;
  for(date = tmpList.first(); date; date = tmpList.next()) {
    addEvents(mCalendar->getEventsForDate(*date));
  }
}

void KOListView::addEvents(QList<KOEvent> eventList)
{
  KOEvent *ev;
  for(ev = eventList.first(); ev; ev = eventList.next()) {
    new KOListViewItem(mListView,ev);
  }
}

void KOListView::selectEvents(QList<KOEvent> eventList)
{
  mListView->clear();
  addEvents(eventList);
}

void KOListView::changeEventDisplay(KOEvent *event, int action)
{
  KOListViewItem *item;
  
  switch(action) {
    case CalendarView::EVENTADDED:
      new KOListViewItem(mListView,event);
      break;
    case CalendarView::EVENTEDITED:
      item = getItemForEvent(event);
      if (item) {
        delete item;
        new KOListViewItem(mListView,event);
      }
      break;
    case CalendarView::EVENTDELETED:
      item = getItemForEvent(event);
      if (item) {
        delete item;
      }
      break;
    default:
      qDebug("KOListView::changeEventDisplay(): Illegal action %d",action);
  }
}

KOListViewItem *KOListView::getItemForEvent(KOEvent *event)
{
  KOListViewItem *item = (KOListViewItem *)mListView->firstChild();
  while (item) {
    qDebug ("Item %s found",item->text(0).latin1());
    if (item->event() == event) return item;
    item = (KOListViewItem *)item->nextSibling();
  }  
  return 0;
}

void KOListView::defaultItemAction(QListViewItem *item)
{
  defaultEventAction(((KOListViewItem *)item)->event());
}

void KOListView::popupMenu(QListViewItem *item,const QPoint &,int)
{
  mActiveItem = (KOListViewItem *)item;
  if (mActiveItem) showEventPopup(mPopupMenu,mActiveItem->event());
}
