// $Id$

#include <qpainter.h>
#include <qkeycode.h>
#include <qprinter.h>
#include <qscrollbar.h>
#include <qcheckbox.h>
#include <qpoint.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qdrawutil.h>
#include <qframe.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qheader.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include "optionsdlg.h"
#include "vcaldrag.h"

#include "kotodoview.h"
#include "kotodoview.moc"

KOTodoViewItem::KOTodoViewItem(QListView *parent, KOEvent *ev)
  : QCheckListItem(parent,"",CheckBox), mEvent(ev)
{
  construct();
}

KOTodoViewItem::KOTodoViewItem(KOTodoViewItem *parent, KOEvent *ev)
  : QCheckListItem(parent,"",CheckBox), mEvent(ev)
{
  construct();
}

void KOTodoViewItem::construct()
{
  setOn(mEvent->getStatus() == KOEvent::NEEDS_ACTION ? false : true );
  setText(0, mEvent->getSummary());
  setText(1, QString::number(mEvent->getPriority()));
  if (mEvent->hasDueDate()) {
    setText(2, mEvent->getDtDue().date().toString());
    if (mEvent->doesFloat()) setText(3,"");
    else setText(3,mEvent->getDtDue().time().toString());
  } else {
    setText(2,"");
    setText(2,"");
  }
}

/////////////////////////////////////////////////////////////////////////////

KOTodoListView::KOTodoListView(CalObject *calendar,QWidget *parent,
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
//  qDebug("KOTodoListView::contentsDragEnterEvent");
  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  mOldCurrent = currentItem();
}


void KOTodoListView::contentsDragMoveEvent(QDragMoveEvent *e)
{
//  qDebug("KOTodoListView::contentsDragMoveEvent");

  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  e->accept();
}

void KOTodoListView::contentsDragLeaveEvent(QDragLeaveEvent *)
{
//  qDebug("KOTodoListView::contentsDragLeaveEvent");

  setCurrentItem(mOldCurrent);
  setSelected(mOldCurrent,true);
}

void KOTodoListView::contentsDropEvent(QDropEvent *e)
{
//  qDebug("KOTodoListView::contentsDropEvent");

  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  KOEvent *todo = mCalendar->createDropTodo(e);

  if (todo) {
    e->acceptAction();

    KOTodoViewItem *destination =
        (KOTodoViewItem *)itemAt(contentsToViewport(e->pos()));
    KOEvent *destinationEvent = 0;
    if (destination) destinationEvent = destination->event();
    
    KOEvent *existingTodo = mCalendar->getTodo(todo->getVUID());
      
    if(existingTodo) {
//      qDebug("Drop existing Todo");
      KOEvent *to = destinationEvent;
      while(to) {
        if (to->getVUID() == todo->getVUID()) {
          QMessageBox::warning(this,"Drop Todo","Cannot move Todo to itself"
                               " or a child of itself","Close");
          delete todo;
          return;
        }
        to = to->getRelatedTo();
      }
      existingTodo->setRelatedTo(destinationEvent);
      emit todoDropped(todo);
      delete todo;
    } else {
//      qDebug("Drop new Todo");
      todo->setRelatedTo(destinationEvent);
      mCalendar->addTodo(todo);
      emit todoDropped(todo);
    }
  } else {
    qDebug("KOTodoListView::contentsDropEvent(): Todo from drop not decodable");
    e->ignore();
  }
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
  QListView::contentsMouseMoveEvent(e);
  if (mMousePressed && (mPressPos - e->pos()).manhattanLength() >
      QApplication::startDragDistance()) {
    mMousePressed = false;
    QListViewItem *item = itemAt(contentsToViewport(mPressPos));
    if (item) {
//      qDebug("Start Drag for item %s",item->text(0).latin1());
      VCalDrag *vd = mCalendar->createDragTodo(
                          ((KOTodoViewItem *)item)->event(),viewport());
      if (vd->drag()) {
        qDebug("KOTodoListView::contentsMouseMoveEvent(): Delete drag source");
      }
/*
      QString source = fullPath(item);
      if ( QFile::exists(source) ) {
        QUriDrag* ud = new QUriDrag(viewport());
        ud->setUnicodeUris( source );
        if ( ud->drag() )
          QMessageBox::information( this, "Drag source",
				    QString("Delete ")+source, "Not implemented" );
*/
    }
  }
}

void KOTodoListView::contentsMouseReleaseEvent(QMouseEvent *e)
{
  QListView::contentsMouseReleaseEvent(e);
  mMousePressed = false;
}


/////////////////////////////////////////////////////////////////////////////

KOTodoView::KOTodoView(CalObject *calendar,QWidget* parent,const char* name) :
  QWidget(parent,name), mCalendar(calendar)
{
  QBoxLayout *topLayout = new QVBoxLayout(this);
  
  QLabel *title = new QLabel(i18n("To-Do Items"),this);
  title->setFrameStyle(QFrame::Panel|QFrame::Raised);
  topLayout->addWidget(title);
  
  mTodoListView = new KOTodoListView(mCalendar,this);
  topLayout->addWidget(mTodoListView);

  mTodoListView->setRootIsDecorated(true);
  mTodoListView->setAllColumnsShowFocus(true);
  
  mTodoListView->addColumn(i18n("Summary"));
  mTodoListView->addColumn(i18n("Priority"));
  mTodoListView->setColumnAlignment(1,AlignHCenter);
  mTodoListView->addColumn(i18n("Due Date"));
  mTodoListView->addColumn(i18n("Due Time"));
  
  mItemPopupMenu = new QPopupMenu;
  mItemPopupMenu->insertItem(BarIcon("todolist"), i18n("New To-Do"), this,
                             SLOT (newTodo()));
  mItemPopupMenu->insertItem(i18n("Show To-Do"), this,
                             SLOT (showTodo()));
  mItemPopupMenu->insertItem(i18n("New Sub-To-Do"), this,
                             SLOT (newSubTodo()));
  mItemPopupMenu->insertItem(i18n("Edit To-Do"), this,
                             SLOT (editTodo()));
  mItemPopupMenu->insertItem(BarIcon("delete"), i18n("Delete To-Do"), this,
                             SLOT (deleteTodo()));
  mItemPopupMenu->insertItem(i18n("Purge Completed"), this,
                             SLOT(purgeCompleted()));
                       
  mPopupMenu = new QPopupMenu;
  mPopupMenu->insertItem(BarIcon("todolist"), i18n("New To-Do"), this,
                         SLOT (newTodo()));
  mPopupMenu->insertItem(i18n("Purge Completed"), this,
                         SLOT(purgeCompleted()));
  
  // Double clicking conflicts with opening/closing the subtree                   
//  QObject::connect(mTodoListView,SIGNAL(doubleClicked(QListViewItem *)),
//                   this,SLOT(editItem(QListViewItem *)));
  QObject::connect(mTodoListView,SIGNAL(rightButtonClicked ( QListViewItem *,
                   const QPoint &, int )),
                   this,SLOT(popupMenu(QListViewItem *,const QPoint &,int)));
  QObject::connect(mTodoListView,SIGNAL(clicked(QListViewItem *)),
                   this,SLOT(itemClicked(QListViewItem *)));
  connect(mTodoListView,SIGNAL(todoDropped(KOEvent *)),SLOT(updateView()));
}

void KOTodoView::updateView()
{
//  qDebug("KOTodoView::updateView()");
  mTodoListView->clear();

  QList<KOEvent> todoList = mCalendar->getTodoList();

/*
  qDebug("KOTodoView::updateView(): Todo List:");
  KOEvent *t;
  for(t = todoList.first(); t; t = todoList.next()) {
    qDebug("  %s",t->getSummary().latin1());

    if (t->getRelatedTo()) {
      qDebug("      (related to %s)",t->getRelatedTo()->getSummary().latin1());
    }

    QList<KOEvent> l = t->getRelations();
    KOEvent *c;
    for(c=l.first();c;c=l.next()) {
      qDebug("    - relation: %s",c->getSummary().latin1());
    }
  }
*/

  // Put for each KOEvent a KOTodoViewItem in the list view. Don't rely on a
  // specific order of events. That means that we have to generate parent items
  // recursively for proper hierarchical display of Todos.
  mTodoMap.clear();
  KOEvent *todo;
  for(todo = todoList.first(); todo; todo = todoList.next()) {
    if (!mTodoMap.contains(todo)) {
      insertTodoItem(todo);
    }
  }
}

QMap<KOEvent *,KOTodoViewItem *>::ConstIterator
  KOTodoView::insertTodoItem(KOEvent *todo)
{
//  qDebug("KOTodoView::insertTodoItem(): %s",todo->getSummary().latin1());
  KOEvent *relatedTodo = todo->getRelatedTo();
  if (relatedTodo) {
//    qDebug("  has Related");
    QMap<KOEvent *,KOTodoViewItem *>::ConstIterator itemIterator;
    itemIterator = mTodoMap.find(relatedTodo);
    if (itemIterator == mTodoMap.end()) {
//      qDebug("    related not yet in list");
      itemIterator = insertTodoItem (relatedTodo);
    }
    KOTodoViewItem *todoItem = new KOTodoViewItem(*itemIterator,todo);
    todoItem->setOpen(true);
    return mTodoMap.insert(todo,todoItem);
  } else {
//    qDebug("  no Related");
    KOTodoViewItem *todoItem = new KOTodoViewItem(mTodoListView,todo);
    todoItem->setOpen(true);
    return mTodoMap.insert(todo,todoItem);
  }
}


void KOTodoView::updateConfig()
{
  // to be implemented.
}

KOEvent *KOTodoView::getSelected()
{
  KOTodoViewItem *item = (KOTodoViewItem *)(mTodoListView->selectedItem());
  if (item) return item->event();
  else return 0;
}

void KOTodoView::editItem(QListViewItem *item)
{
  emit editEventSignal(((KOTodoViewItem *)item)->event());
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
    emit editEventSignal(mActiveItem->event());
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
      QMessageBox::warning(this,"Delete To-Do",
                           "Can not delete To-Do which has children.",
                           QMessageBox::Ok,0);
    } else {
      emit deleteEventSignal(mActiveItem->event());
    }
  }
}

void KOTodoView::purgeCompleted()
{
  QList<KOEvent> todoCal = mCalendar->getTodoList();

  KOEvent *aTodo;
  for (aTodo = todoCal.first(); aTodo; aTodo = todoCal.next())
  {
    if (aTodo->getStatus() != KOEvent::NEEDS_ACTION)
      mCalendar->deleteTodo(aTodo);
  }
  updateView();
}

void KOTodoView::itemClicked(QListViewItem *item)
{
  if (!item) return;

  KOTodoViewItem *todoItem = (KOTodoViewItem *)item;
  int status = todoItem->event()->getStatus();  // Completed or not?
  
  if (todoItem->isOn()) {
    if (status != KOEvent::COMPLETED) {
      todoItem->event()->setStatus(KOEvent::COMPLETED);
    }
  } else {
    if (status != KOEvent::NEEDS_ACTION) {
      todoItem->event()->setStatus(KOEvent::NEEDS_ACTION);
    }
  }   
}
