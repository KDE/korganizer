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
  if (mEvent->getTodoStatus()) {
    setText(0,i18n("Todo: %1").arg(mEvent->getSummary()));
    setText(1,"---");
    setText(2,"---");
    setText(3,"---");
    setText(4,"---");
    setText(5,"---");
    setText(6,"---");
    if (mEvent->hasDueDate()) {
      setText(7,mEvent->getDtDueDateStr());
      if (mEvent->doesFloat()) {
        setText(8,"---");
      } else {
        setText(8,mEvent->getDtDueDateStr());
      }
    } else {
      setText(7,"---");
      setText(8,"---");
    }
  } else {
    setText(0, mEvent->getSummary());
    setText(1,mEvent->getDtStartDateStr());
    setText(2,mEvent->getDtStartTimeStr());
    setText(3,mEvent->getDtEndDateStr());
    setText(4,mEvent->getDtEndTimeStr());
    setText(5,mEvent->getAlarmRepeatCount() ? i18n("Yes") : i18n("No"));
    setText(6,mEvent->doesRecur() ? i18n("Yes") : i18n("No"));
    setText(7,"---");
    setText(8,"---");
  }
}


KOListView::KOListView(CalObject *calendar, QWidget *parent,
		       const char *name)
  : KOBaseView(calendar, parent, name)
{
  mActiveItem = 0;

  mListView = new QListView(this);
  mListView->addColumn(i18n("Summary"));
  mListView->addColumn(i18n("Start Date"));
  mListView->setColumnAlignment(1,AlignHCenter);
  mListView->addColumn(i18n("Start Time"));
  mListView->setColumnAlignment(2,AlignHCenter);
  mListView->addColumn(i18n("End Date"));
  mListView->setColumnAlignment(3,AlignHCenter);
  mListView->addColumn(i18n("End Time"));
  mListView->setColumnAlignment(4,AlignHCenter);
  mListView->addColumn(i18n("Alarm")); // alarm set?
  mListView->addColumn(i18n("Recurs")); // recurs?
  mListView->addColumn(i18n("Due Date"));
  mListView->setColumnAlignment(7,AlignHCenter);
  mListView->addColumn(i18n("Due Time"));
  mListView->setColumnAlignment(8,AlignHCenter);

  QBoxLayout *layoutTop = new QVBoxLayout(this);
  layoutTop->addWidget(mListView);

  mPopupMenu = eventPopup();
/*
  mPopupMenu = new QPopupMenu;
  mPopupMenu->insertItem(i18n("Edit Event"), this,
		     SLOT (editEvent()));
  mPopupMenu->insertItem(SmallIcon("delete"), i18n("Delete Event  "), this,
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
  QObject::connect(mListView,SIGNAL(selectionChanged()),
                   SLOT(processSelectionChange()));
}

KOListView::~KOListView()
{
  delete mPopupMenu;
}

int KOListView::maxDatesHint()
{
  return 0;
}

int KOListView::currentDateCount()
{
  return 0;
}

QList<KOEvent> KOListView::getSelected()
{
  QList<KOEvent> eventList;

  QListViewItem *item = mListView->selectedItem();
  if (item) eventList.append(((KOListViewItem *)item)->event());
  
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
    addEvents(mCalendar->getTodosForDate(*date));
  }
  
  emit eventsSelected(false);
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

  // After new creation of list view no events are selected.
  emit eventsSelected(false);
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

void KOListView::processSelectionChange()
{
  // If selection has changed, we know that one event is selected.
  emit eventsSelected(true);
}
