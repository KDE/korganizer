/*
 * KOListView provides a view of events in a list.
 * This file is part of the KOrganizer project.
 * (c) 1999 Preston Brown <pbrown@kde.org>
 */

#include <qlistview.h>
#include <qlayout.h>
#include <qpopupmenu.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>

#include "calendar.h"
#include "calprinter.h"
#include "calendarview.h"

#include "kolistview.h"
#include "kolistview.moc"

ListItemVisitor::ListItemVisitor(KOListViewItem *item)
{
  mItem = item;
}

ListItemVisitor::~ListItemVisitor()
{
}

bool ListItemVisitor::visit(Event *e)
{
  mItem->setText(0,e->summary());
  mItem->setText(1,e->dtStartDateStr());
  mItem->setText(2,e->dtStartTimeStr());
  mItem->setText(3,e->dtEndDateStr());
  mItem->setText(4,e->dtEndTimeStr());
  mItem->setText(5,e->alarm()->repeatCount() ? i18n("Yes") : i18n("No"));
  mItem->setText(6,e->recurrence()->doesRecur() ? i18n("Yes") : i18n("No"));
  mItem->setText(7,"---");
  mItem->setText(8,"---");
  mItem->setText(9,e->categoriesStr());

  QString key;
  QDate d = e->dtStart().date();
  QTime t = e->doesFloat() ? QTime(0,0) : e->dtStart().time();
  key.sprintf("%04d%02d%02d%02d%02d",d.year(),d.month(),d.day(),t.hour(),t.minute());
  mItem->setSortKey(1,key);

  d = e->dtEnd().date();
  t = e->doesFloat() ? QTime(0,0) : e->dtEnd().time();
  key.sprintf("%04d%02d%02d%02d%02d",d.year(),d.month(),d.day(),t.hour(),t.minute());
  mItem->setSortKey(3,key);

  return true;
}

bool ListItemVisitor::visit(Todo *t)
{
  mItem->setText(0,i18n("Todo: %1").arg(t->summary()));
  mItem->setText(1,"---");
  mItem->setText(2,"---");
  mItem->setText(3,"---");
  mItem->setText(4,"---");
  mItem->setText(5,"---");
  mItem->setText(6,"---");
  if (t->hasDueDate()) {
    mItem->setText(7,t->dtDueDateStr());
    if (t->doesFloat()) {
      mItem->setText(8,"---");
    } else {
      mItem->setText(8,t->dtDueTimeStr());
    }
  } else {
    mItem->setText(7,"---");
    mItem->setText(8,"---");
  }
  mItem->setText(9,t->categoriesStr());

  QString key; 
  QDate d = t->dtDue().date();
  QTime tm = t->doesFloat() ? QTime(0,0) : t->dtDue().time();
  key.sprintf("%04d%02d%02d%02d%02d",d.year(),d.month(),d.day(),tm.hour(),tm.minute());
  mItem->setSortKey(7,key);
  
  return true;
}

bool ListItemVisitor::visit(Journal *)
{
  return false;
}


KOListViewItem::KOListViewItem(QListView *parent, Incidence *ev)
  : QListViewItem(parent), mEvent(ev)
{
}

QString KOListViewItem::key(int column,bool) const
{
  QMap<int,QString>::ConstIterator it = mKeyMap.find(column);
  if (it == mKeyMap.end()) {
    return text(column);
  } else {
    return *it;
  }
}

void KOListViewItem::setSortKey(int column,const QString &key)
{
  mKeyMap.insert(column,key);
}


KOListView::KOListView(Calendar *calendar, QWidget *parent,
		       const char *name)
  : KOEventView(calendar, parent, name)
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
  mListView->addColumn(i18n("Categories"));
  mListView->setColumnAlignment(9,AlignHCenter);

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

QList<Incidence> KOListView::getSelected()
{
  QList<Incidence> eventList;

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
  kdDebug() << "KOListView::updateView() does nothing" << endl;
}

void KOListView::selectDates(const QDateList dateList)
{
  QDateList tmpList(FALSE);
  tmpList = dateList;

  mListView->clear();

  QDate *date;
  for(date = tmpList.first(); date; date = tmpList.next()) {
    addEvents(mCalendar->getEventsForDate(*date));
    addTodos(mCalendar->getTodosForDate(*date));
  }
  
  emit eventsSelected(false);
}

void KOListView::addEvents(QList<Event> eventList)
{
  Event *ev;
  for(ev = eventList.first(); ev; ev = eventList.next()) {
    addIncidence(ev);
  }
}

void KOListView::addTodos(QList<Todo> eventList)
{
  Todo *ev;
  for(ev = eventList.first(); ev; ev = eventList.next()) {
    addIncidence(ev);
  }
}

void KOListView::addIncidence(Incidence *incidence)
{
  KOListViewItem *item = new KOListViewItem(mListView,incidence);
  ListItemVisitor v(item);
  if (incidence->accept(v)) return;
  else delete item;
}

void KOListView::selectEvents(QList<Event> eventList)
{
  mListView->clear();
  addEvents(eventList);

  // After new creation of list view no events are selected.
  emit eventsSelected(false);
}

void KOListView::changeEventDisplay(Event *event, int action)
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
      kdDebug() << "KOListView::changeEventDisplay(): Illegal action " << action << endl;
  }
}

KOListViewItem *KOListView::getItemForEvent(Event *event)
{
  KOListViewItem *item = (KOListViewItem *)mListView->firstChild();
  while (item) {
//    kdDebug() << "Item " << item->text(0) << " found" << endl;
    if (item->event() == event) return item;
    item = (KOListViewItem *)item->nextSibling();
  }
  return 0;
}

void KOListView::defaultItemAction(QListViewItem *item)
{
  Event *event = dynamic_cast<Event *>(((KOListViewItem *)item)->event());
  if (event) defaultEventAction(event);
}

void KOListView::popupMenu(QListViewItem *item,const QPoint &,int)
{
  mActiveItem = (KOListViewItem *)item;
  if (mActiveItem) {
    Event *event = dynamic_cast<Event *>(mActiveItem->event());
    if (event) mPopupMenu->showEventPopup(event);
  }
}

void KOListView::processSelectionChange()
{
  // If selection has changed, we know that one event is selected.
  emit eventsSelected(true);
}
