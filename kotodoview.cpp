/*
    This file is part of KOrganizer.
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// $Id$

#include <qlayout.h>
#include <qheader.h>
#include <qcursor.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <libkcal/vcaldrag.h>
#include <libkcal/dndfactory.h>

#ifndef KORG_NOPRINTER
#include "calprinter.h"
#endif
#include "docprefs.h"

#include "kotodoview.h"
#include "kotodoview.moc"

KOTodoListView::KOTodoListView(Calendar *calendar,QWidget *parent,
                               const char *name) :
  QListView(parent,name)
{
  mCalendar = calendar;

  mOldCurrent = 0;
  mMousePressed = false;

  setAcceptDrops(true);
  viewport()->setAcceptDrops(true);
}

void KOTodoListView::contentsDragEnterEvent(QDragEnterEvent *e)
{
#ifndef KORG_NODND
//  kdDebug() << "KOTodoListView::contentsDragEnterEvent" << endl;
  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  mOldCurrent = currentItem();
#endif
}


void KOTodoListView::contentsDragMoveEvent(QDragMoveEvent *e)
{
#ifndef KORG_NODND
//  kdDebug() << "KOTodoListView::contentsDragMoveEvent" << endl;

  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  e->accept();
#endif
}

void KOTodoListView::contentsDragLeaveEvent(QDragLeaveEvent *)
{
#ifndef KORG_NODND
//  kdDebug() << "KOTodoListView::contentsDragLeaveEvent" << endl;

  setCurrentItem(mOldCurrent);
  setSelected(mOldCurrent,true);
#endif
}

void KOTodoListView::contentsDropEvent(QDropEvent *e)
{
#ifndef KORG_NODND
//  kdDebug() << "KOTodoListView::contentsDropEvent" << endl;

  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  DndFactory factory( mCalendar );
  Todo *todo = factory.createDropTodo(e);

  if (todo) {
    e->acceptAction();

    KOTodoViewItem *destination =
        (KOTodoViewItem *)itemAt(contentsToViewport(e->pos()));
    Todo *destinationEvent = 0;
    if (destination) destinationEvent = destination->event();
    
    Todo *existingTodo = mCalendar->getTodo(todo->VUID());
      
    if(existingTodo) {
//      kdDebug() << "Drop existing Todo" << endl;
      Incidence *to = destinationEvent;
      while(to) {
        if (to->VUID() == todo->VUID()) {
          KMessageBox::sorry(this,
              i18n("Cannot move Todo to itself or a child of itself"),
              i18n("Drop Todo"));
          delete todo;
          return;
        }
        to = to->relatedTo();
      }
      existingTodo->setRelatedTo(destinationEvent);
      emit todoDropped(todo);
      delete todo;
    } else {
//      kdDebug() << "Drop new Todo" << endl;
      todo->setRelatedTo(destinationEvent);
      mCalendar->addTodo(todo);
      emit todoDropped(todo);
    }
  } else {
    kdDebug() << "KOTodoListView::contentsDropEvent(): Todo from drop not decodable" << endl;
    e->ignore();
  }
#endif
}

void KOTodoListView::contentsMousePressEvent(QMouseEvent* e)
{
  QListView::contentsMousePressEvent(e);
  QPoint p(contentsToViewport(e->pos()));
  QListViewItem *i = itemAt(p);
  if (i) {
    // if the user clicked into the root decoration of the item, don't
    // try to start a drag!
    if (p.x() > header()->sectionPos(header()->mapToIndex(0)) +
        treeStepSize() * (i->depth() + (rootIsDecorated() ? 1 : 0)) +
        itemMargin() ||
        p.x() < header()->sectionPos(header()->mapToIndex(0))) {
      mPressPos = e->pos();
      mMousePressed = true;
    }
  }
}

void KOTodoListView::contentsMouseMoveEvent(QMouseEvent* e)
{
#ifndef KORG_NODND
  QListView::contentsMouseMoveEvent(e);
  if (mMousePressed && (mPressPos - e->pos()).manhattanLength() >
      QApplication::startDragDistance()) {
    mMousePressed = false;
    QListViewItem *item = itemAt(contentsToViewport(mPressPos));
    if (item) {
//      kdDebug() << "Start Drag for item " << item->text(0) << endl;
      DndFactory factory( mCalendar );
      VCalDrag *vd = factory.createDragTodo(
                          ((KOTodoViewItem *)item)->event(),viewport());
      if (vd->drag()) {
        kdDebug() << "KOTodoListView::contentsMouseMoveEvent(): Delete drag source" << endl;
      }
/*
      QString source = fullPath(item);
      if ( QFile::exists(source) ) {
        QUriDrag* ud = new QUriDrag(viewport());
        ud->setFilenames( source );
        if ( ud->drag() )
          QMessageBox::information( this, "Drag source",
				    QString("Delete ")+source, "Not implemented" );
*/
    }
  }
#endif
}

void KOTodoListView::contentsMouseReleaseEvent(QMouseEvent *e)
{
  QListView::contentsMouseReleaseEvent(e);
  mMousePressed = false;
}

void KOTodoListView::contentsMouseDoubleClickEvent(QMouseEvent *e)
{
  if (!e) return;

  QPoint vp = contentsToViewport(e->pos());

  QListViewItem *item = itemAt(vp);

  if (!item) return;

  emit doubleClicked(item);
}

/////////////////////////////////////////////////////////////////////////////

KOTodoView::KOTodoView(Calendar *calendar,QWidget* parent,const char* name) :
  KOrg::BaseView(calendar,parent,name)
{
  QBoxLayout *topLayout = new QVBoxLayout(this);

  QLabel *title = new QLabel(i18n("To-Do Items"),this);
  title->setFrameStyle(QFrame::Panel|QFrame::Raised);
  topLayout->addWidget(title);

  mTodoListView = new KOTodoListView(calendar,this);
  topLayout->addWidget(mTodoListView);

  mTodoListView->setRootIsDecorated(true);
  mTodoListView->setAllColumnsShowFocus(true);

  mTodoListView->addColumn(i18n("Summary"));
  mTodoListView->addColumn(i18n("Priority"));
  mTodoListView->setColumnAlignment(1,AlignHCenter);
  mTodoListView->addColumn(i18n("Complete"));
  mTodoListView->setColumnAlignment(2,AlignRight);
  mTodoListView->addColumn(i18n("Due Date"));
  mTodoListView->addColumn(i18n("Due Time"));
  mTodoListView->addColumn(i18n("Categories"));
  mTodoListView->addColumn(i18n("Sort Id"));
  mTodoListView->setColumnAlignment(4,AlignHCenter);

  mItemPopupMenu = new QPopupMenu;
  mItemPopupMenu->insertItem(i18n("Show..."), this,
                             SLOT (showTodo()));
  mItemPopupMenu->insertItem(i18n("Edit..."), this,
                             SLOT (editTodo()));
  mItemPopupMenu->insertItem(SmallIconSet("editdelete"), i18n("Delete"), this,
                             SLOT (deleteTodo()));
  mItemPopupMenu->insertSeparator();
  mItemPopupMenu->insertItem(SmallIconSet("todo"), i18n("New To-Do..."), this,
                             SLOT (newTodo()));
  mItemPopupMenu->insertItem(i18n("New Sub-To-Do..."), this,
                             SLOT (newSubTodo()));
  mItemPopupMenu->insertSeparator();
  mItemPopupMenu->insertItem(i18n("delete completed todos","Purge Completed"),
                             this,SLOT(purgeCompleted()));
                       
  mPopupMenu = new QPopupMenu;
  mPopupMenu->insertItem(SmallIconSet("todo"), i18n("New To-Do"), this,
                         SLOT (newTodo()));
  mPopupMenu->insertItem(i18n("Purge Completed"), this,
                         SLOT(purgeCompleted()));
  
  mDocPrefs = new DocPrefs( name );
  
  // Double clicking conflicts with opening/closing the subtree                   
  QObject::connect(mTodoListView,SIGNAL(doubleClicked(QListViewItem *)),
                   this,SLOT(showItem(QListViewItem *)));
  QObject::connect(mTodoListView,SIGNAL(rightButtonClicked ( QListViewItem *,
                   const QPoint &, int )),
                   this,SLOT(popupMenu(QListViewItem *,const QPoint &,int)));
  QObject::connect(mTodoListView,SIGNAL(clicked(QListViewItem *)),
                   this,SLOT(itemClicked(QListViewItem *)));
  connect(mTodoListView,SIGNAL(todoDropped(Todo *)),SLOT(updateView()));
  connect(mTodoListView,SIGNAL(expanded(QListViewItem *)),
          SLOT(itemStateChanged(QListViewItem *)));
  connect(mTodoListView,SIGNAL(collapsed(QListViewItem *)),
          SLOT(itemStateChanged(QListViewItem *)));
}

KOTodoView::~KOTodoView()
{
  delete mDocPrefs;
}

void KOTodoView::updateView()
{
//  kdDebug() << "KOTodoView::updateView()" << endl;

  mTodoListView->clear();

  QPtrList<Todo> todoList = calendar()->getFilteredTodoList();

/*
  kdDebug() << "KOTodoView::updateView(): Todo List:" << endl;
  Event *t;
  for(t = todoList.first(); t; t = todoList.next()) {
    kdDebug() << "  " << t->getSummary() << endl;

    if (t->getRelatedTo()) {
      kdDebug() << "      (related to " << t->getRelatedTo()->getSummary() << ")" << endl;
    }

    QPtrList<Event> l = t->getRelations();
    Event *c;
    for(c=l.first();c;c=l.next()) {
      kdDebug() << "    - relation: " << c->getSummary() << endl;
    }
  }
*/

  // Put for each Event a KOTodoViewItem in the list view. Don't rely on a
  // specific order of events. That means that we have to generate parent items
  // recursively for proper hierarchical display of Todos.
  mTodoMap.clear();
  Todo *todo;
  for(todo = todoList.first(); todo; todo = todoList.next()) {
    if (!mTodoMap.contains(todo)) {
      insertTodoItem(todo);
    }
  }
  
  // Restore opened/closed state
  mTodoListView->blockSignals( true );
  if( mDocPrefs ) restoreItemState( mTodoListView->firstChild() );
  mTodoListView->blockSignals( false );
}

void KOTodoView::restoreItemState( QListViewItem *item )
{
  while( item ) {
    KOTodoViewItem *todoItem = (KOTodoViewItem *)item;
    todoItem->setOpen( mDocPrefs->readBoolEntry( todoItem->event()->VUID() ) );
    if( item->childCount() > 0 ) restoreItemState( item->firstChild() );
    item = item->nextSibling();
  }
}


QMap<Todo *,KOTodoViewItem *>::ConstIterator
  KOTodoView::insertTodoItem(Todo *todo)
{
//  kdDebug() << "KOTodoView::insertTodoItem(): " << todo->getSummary() << endl;
  // TODO: Check, if dynmaic cast is necessary
  Incidence *incidence = todo->relatedTo();
  if (incidence && incidence->type() == "Todo") {
    Todo *relatedTodo = static_cast<Todo *>(incidence);

//    kdDebug() << "  has Related" << endl;
    QMap<Todo *,KOTodoViewItem *>::ConstIterator itemIterator;
    itemIterator = mTodoMap.find(relatedTodo);
    if (itemIterator == mTodoMap.end()) {
//      kdDebug() << "    related not yet in list" << endl;
      itemIterator = insertTodoItem (relatedTodo);
    }
    KOTodoViewItem *todoItem = new KOTodoViewItem(*itemIterator,todo);
    return mTodoMap.insert(todo,todoItem);
  } else {
//    kdDebug() << "  no Related" << endl;
    KOTodoViewItem *todoItem = new KOTodoViewItem(mTodoListView,todo);
    return mTodoMap.insert(todo,todoItem);
  }
}


void KOTodoView::updateConfig()
{
  // to be implemented.
}

QPtrList<Incidence> KOTodoView::selectedIncidences()
{
  QPtrList<Incidence> selected;

  KOTodoViewItem *item = (KOTodoViewItem *)(mTodoListView->selectedItem());
  if (item) selected.append(item->event());

  return selected;
}

QPtrList<Todo> KOTodoView::selectedTodos()
{
  QPtrList<Todo> selected;

  KOTodoViewItem *item = (KOTodoViewItem *)(mTodoListView->selectedItem());
  if (item) selected.append(item->event());

  return selected;
}

void KOTodoView::changeEventDisplay(Event *, int)
{
  updateView();
}

void KOTodoView::showDates(const QDate &, const QDate &)
{
}

void KOTodoView::showEvents(QPtrList<Event>)
{
  kdDebug() << "KOTodoView::selectEvents(): not yet implemented" << endl;
}

void KOTodoView::printPreview(CalPrinter *calPrinter, const QDate &fd,
                              const QDate &td)
{
#ifndef KORG_NOPRINTER
  calPrinter->preview(CalPrinter::Todolist, fd, td);
#endif
}

void KOTodoView::editItem(QListViewItem *item)
{
  emit editTodoSignal(((KOTodoViewItem *)item)->event());
}

void KOTodoView::showItem(QListViewItem *item)
{
  emit showTodoSignal(((KOTodoViewItem *)item)->event());
}

void KOTodoView::popupMenu(QListViewItem *item,const QPoint &,int)
{
  mActiveItem = (KOTodoViewItem *)item;
  if (item) mItemPopupMenu->popup(QCursor::pos());
  else mPopupMenu->popup(QCursor::pos());
}

void KOTodoView::newTodo()
{
  emit newTodoSignal();
}

void KOTodoView::newSubTodo()
{
  if (mActiveItem) {
    emit newSubTodoSignal(mActiveItem->event());
  }
}

void KOTodoView::editTodo()
{
  if (mActiveItem) {
    emit editTodoSignal(mActiveItem->event());
  }
}

void KOTodoView::showTodo()
{
  if (mActiveItem) {
    emit showTodoSignal(mActiveItem->event());
  }
}

void KOTodoView::deleteTodo()
{
  if (mActiveItem) {
    if (mActiveItem->childCount()) {
      KMessageBox::sorry(this,i18n("Cannot delete To-Do which has children."),
                         i18n("Delete To-Do"));
    } else {
      emit deleteTodoSignal(mActiveItem->event());
    }
  }
}

void KOTodoView::purgeCompleted()
{
  int result = KMessageBox::warningContinueCancel(this,
      i18n("Delete all completed todos?"),i18n("Purge Todos"),i18n("Purge"));

  if (result == KMessageBox::Continue) {
    QPtrList<Todo> todoCal = calendar()->getTodoList();

    Todo *aTodo;
    for (aTodo = todoCal.first(); aTodo; aTodo = todoCal.next()) {
    if (aTodo->isCompleted())
      calendar()->deleteTodo(aTodo);
    }
    updateView();
  }
}

void KOTodoView::itemClicked(QListViewItem *item)
{
  if (!item) return;

  KOTodoViewItem *todoItem = (KOTodoViewItem *)item;
  int completed = todoItem->event()->isCompleted();  // Completed or not?
  
  if (todoItem->isOn()) {
    if (!completed) {
      todoItem->event()->setCompleted(QDateTime::currentDateTime());
    }
  } else {
    if (completed) {
      todoItem->event()->setCompleted(false);
    }
  }
}

void KOTodoView::setDocumentId( const QString &id )
{
  kdDebug() << "KOTodoView::setDocumentId()" << endl;

  mDocPrefs->setDoc( id );
}

void KOTodoView::itemStateChanged( QListViewItem *item )
{
  if (!item) return;

  KOTodoViewItem *todoItem = (KOTodoViewItem *)item;

//  kdDebug() << "KOTodoView::itemStateChanged(): " << todoItem->event()->summary() << endl;

  if( mDocPrefs ) mDocPrefs->writeEntry( todoItem->event()->VUID(), todoItem->isOpen() );
}
