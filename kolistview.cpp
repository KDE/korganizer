/*
    This file is part of KOrganizer.
    Copyright (c) 1999 Preston Brown
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlistview.h>
#include <qlayout.h>
#include <qpopupmenu.h>

#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kglobal.h>

#include <libkcal/calendar.h>

#ifndef KORG_NOPRINTER
#include "calprinter.h"
#endif
#include "koglobals.h"
#include "koincidencetooltip.h"

#include "kolistview.h"
#include "kolistview.moc"


KOListViewToolTip::KOListViewToolTip( QWidget* parent,
                                      KListView* lv )
  :QToolTip(parent)
{
  eventlist=lv;
}

void KOListViewToolTip::maybeTip( const QPoint & pos)
{
  QRect r;
  QListViewItem *it = eventlist->itemAt(pos);
  KOListViewItem *i = static_cast<KOListViewItem*>(it);

  if( i && KOPrefs::instance()->mEnableToolTips ) {
    /* Calculate the rectangle. */
    r=eventlist->itemRect( it );
    /* Show the tip */
    QString tipText;
    ToolTipVisitor v;
    if (v.act(i->data(), &tipText, true)) {
      tip(r, tipText);
    }
  }

}

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
  mItem->setText(5,e->isAlarmEnabled() ? i18n("Yes") : i18n("No"));
  mItem->setText(6,e->doesRecur() ? i18n("Yes") : i18n("No"));
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
  mItem->setText(0,i18n("To-Do: %1").arg(t->summary()));
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

KOListView::KOListView( Calendar *calendar, QWidget *parent,
                        const char *name)
  : KOEventView(calendar, parent, name)
{
  mActiveItem = 0;

  mListView = new KListView(this);
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
  mPopupMenu->insertItem(SmallIcon("delete"), i18n("Delete Event"), this,
                     SLOT (deleteEvent()));
  mPopupMenu->insertSeparator();
  mPopupMenu->insertItem(i18n("Show Dates"), this,
                      SLOT(showDates()));
  mPopupMenu->insertItem(i18n("Hide Dates"), this,
                      SLOT(hideDates()));
*/

  QObject::connect( mListView, SIGNAL( doubleClicked( QListViewItem * ) ),
                    SLOT( defaultItemAction( QListViewItem * ) ) );
  QObject::connect( mListView, SIGNAL( returnPressed( QListViewItem * ) ),
                    SLOT( defaultItemAction( QListViewItem * ) ) );
  QObject::connect( mListView, SIGNAL( rightButtonClicked ( QListViewItem *,
                                                            const QPoint &,
                                                            int ) ),
                    SLOT( popupMenu( QListViewItem *, const QPoint &, int ) ) );
  QObject::connect( mListView, SIGNAL( selectionChanged() ),
                    SLOT( processSelectionChange() ) );

//  setMinimumSize(100,100);
  mListView->restoreLayout(KOGlobals::config(),"KOListView Layout");

  new KOListViewToolTip( mListView->viewport(), mListView );
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

Incidence::List KOListView::selectedIncidences()
{
  Incidence::List eventList;

  QListViewItem *item = mListView->selectedItem();
  if (item) eventList.append(((KOListViewItem *)item)->data());

  return eventList;
}

DateList KOListView::selectedDates()
{
  DateList eventList;
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
#ifndef KORG_NOPRINTER
  calPrinter->preview(CalPrinter::Day, fd, td);
#endif
}

void KOListView::showDates()
{
  showDates(true);
}

void KOListView::hideDates()
{
  showDates(false);
}

void KOListView::updateView()
{
  kdDebug(5850) << "KOListView::updateView() does nothing" << endl;
}

void KOListView::showDates(const QDate &start, const QDate &end)
{
  clear();

  QDate date = start;
  while( date <= end ) {
    addEvents(calendar()->events(date));
    addTodos(calendar()->todos(date));
    date = date.addDays( 1 );
  }

  emit incidenceSelected( 0 );
}

void KOListView::addEvents( const Event::List &eventList )
{
  Event::List::ConstIterator it;
  for( it = eventList.begin(); it != eventList.end(); ++it ) {
    addIncidence( *it );
  }
}

void KOListView::addTodos( const Todo::List &eventList )
{
  Todo::List::ConstIterator it;
  for( it = eventList.begin(); it != eventList.end(); ++it ) {
    addIncidence( *it );
  }
}

void KOListView::addIncidence(Incidence *incidence)
{
  if ( mUidDict.find( incidence->uid() ) ) return;

  mUidDict.insert( incidence->uid(), incidence );

  KOListViewItem *item = new KOListViewItem( incidence, mListView );
  ListItemVisitor v(item);
  if (incidence->accept(v)) return;
  else delete item;
}

void KOListView::showEvents( const Event::List &eventList )
{
  clear();

  addEvents( eventList );

  // After new creation of list view no events are selected.
  emit incidenceSelected( 0 );
}

void KOListView::changeEventDisplay(Event *event, int action)
{
  KOListViewItem *item;

  switch(action) {
    case KOGlobals::EVENTADDED:
      addIncidence( event );
      break;
    case KOGlobals::EVENTEDITED:
      item = getItemForEvent(event);
      if (item) {
        delete item;
        mUidDict.remove( event->uid() );
        addIncidence( event );
      }
      break;
    case KOGlobals::EVENTDELETED:
      item = getItemForEvent(event);
      if (item) {
        delete item;
      }
      break;
    default:
      kdDebug(5850) << "KOListView::changeEventDisplay(): Illegal action " << action << endl;
  }
}

KOListViewItem *KOListView::getItemForEvent(Event *event)
{
  KOListViewItem *item = (KOListViewItem *)mListView->firstChild();
  while (item) {
//    kdDebug(5850) << "Item " << item->text(0) << " found" << endl;
    if (item->data() == event) return item;
    item = (KOListViewItem *)item->nextSibling();
  }
  return 0;
}

void KOListView::defaultItemAction(QListViewItem *i)
{
  KOListViewItem *item = static_cast<KOListViewItem *>( i );
  if ( item ) defaultAction( item->data() );
}

void KOListView::popupMenu(QListViewItem *item,const QPoint &,int)
{
  mActiveItem = (KOListViewItem *)item;
  if (mActiveItem) {
    Incidence *incidence = mActiveItem->data();
    mPopupMenu->showIncidencePopup(incidence);

    /*
    if ( incidence && incidence->type() == "Event" ) {
      Event *event = static_cast<Event *>( incidence );
      mPopupMenu->showEventPopup(event);
    }
    */
  }
}

void KOListView::readSettings(KConfig *config)
{
  mListView->restoreLayout(config,"KOListView Layout");
}

void KOListView::writeSettings(KConfig *config)
{
  mListView->saveLayout(config,"KOListView Layout");
}

void KOListView::processSelectionChange()
{
  kdDebug(5850) << "KOListView::processSelectionChange()" << endl;

  KOListViewItem *item =
    static_cast<KOListViewItem *>( mListView->selectedItem() );

  if ( !item ) {
    emit incidenceSelected( 0 );
  } else {
    emit incidenceSelected( item->data() );
  }
}

void KOListView::clearSelection()
{
  mListView->selectAll( false );
}

void KOListView::clear()
{
  mListView->clear();
  mUidDict.clear();
}
