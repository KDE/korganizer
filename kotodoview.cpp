// $Id$

#include <qlayout.h>
#include <qheader.h>

#include <kapp.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "vcaldrag.h"
#include "calprinter.h"

#include "kotodoview.h"
#include "kotodoview.moc"

KOTodoViewItem::KOTodoViewItem(QListView *parent, Todo *ev)
  : QCheckListItem(parent,"",CheckBox), mEvent(ev)
{
  construct();
}

KOTodoViewItem::KOTodoViewItem(KOTodoViewItem *parent, Todo *ev)
  : QCheckListItem(parent,"",CheckBox), mEvent(ev)
{
  construct();
}

void KOTodoViewItem::paintBranches(QPainter *p,const QColorGroup & cg,int w,
                                   int y,int h,GUIStyle s)
{
  QListViewItem::paintBranches(p,cg,w,y,h,s);
}

void KOTodoViewItem::construct()
{
  setOn(mEvent->isCompleted());
  setText(0, mEvent->summary());
  setText(1, QString::number(mEvent->priority()));
  if (mEvent->hasDueDate()) {
    setText(2, mEvent->dtDueDateStr());
    if (mEvent->doesFloat()) setText(3,"");
    else setText(3,mEvent->dtDueTimeStr());
  } else {
    setText(2,"");
    setText(2,"");
  }
  // Find sort id in description. It's the text behind the last '#' character
  // found in the description. White spaces are removed from beginning and end
  // of sort id.
  int pos = mEvent->description().findRev('#');
  if (pos < 0) {
    setText(4,"");
  } else {
    QString str = mEvent->description().mid(pos+1);
    str.stripWhiteSpace();
    setText(4,str);
  }
}

/////////////////////////////////////////////////////////////////////////////

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
//  kdDebug() << "KOTodoListView::contentsDragEnterEvent" << endl;
  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  mOldCurrent = currentItem();
}


void KOTodoListView::contentsDragMoveEvent(QDragMoveEvent *e)
{
//  kdDebug() << "KOTodoListView::contentsDragMoveEvent" << endl;

  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  e->accept();
}

void KOTodoListView::contentsDragLeaveEvent(QDragLeaveEvent *)
{
//  kdDebug() << "KOTodoListView::contentsDragLeaveEvent" << endl;

  setCurrentItem(mOldCurrent);
  setSelected(mOldCurrent,true);
}

void KOTodoListView::contentsDropEvent(QDropEvent *e)
{
//  kdDebug() << "KOTodoListView::contentsDropEvent" << endl;

  if (!VCalDrag::canDecode(e)) {
    e->ignore();
    return;
  }

  Todo *todo = mCalendar->createDropTodo(e);

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
//      kdDebug() << "Start Drag for item " << item->text(0) << endl;
      VCalDrag *vd = mCalendar->createDragTodo(
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
  KOBaseView(calendar,parent,name)
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
  mItemPopupMenu->insertItem(i18n("Purge Completed"), this,
                             SLOT(purgeCompleted()));
                       
  mPopupMenu = new QPopupMenu;
  mPopupMenu->insertItem(SmallIconSet("todo"), i18n("New To-Do"), this,
                         SLOT (newTodo()));
  mPopupMenu->insertItem(i18n("Purge Completed"), this,
                         SLOT(purgeCompleted()));
  
  // Double clicking conflicts with opening/closing the subtree                   
  QObject::connect(mTodoListView,SIGNAL(doubleClicked(QListViewItem *)),
                   this,SLOT(showItem(QListViewItem *)));
  QObject::connect(mTodoListView,SIGNAL(rightButtonClicked ( QListViewItem *,
                   const QPoint &, int )),
                   this,SLOT(popupMenu(QListViewItem *,const QPoint &,int)));
  QObject::connect(mTodoListView,SIGNAL(clicked(QListViewItem *)),
                   this,SLOT(itemClicked(QListViewItem *)));
  connect(mTodoListView,SIGNAL(todoDropped(Todo *)),SLOT(updateView()));
}

void KOTodoView::updateView()
{
//  kdDebug() << "KOTodoView::updateView()" << endl;
  mTodoListView->clear();

  QList<Todo> todoList = mCalendar->getTodoList();

/*
  kdDebug() << "KOTodoView::updateView(): Todo List:" << endl;
  Event *t;
  for(t = todoList.first(); t; t = todoList.next()) {
    kdDebug() << "  " << t->getSummary() << endl;

    if (t->getRelatedTo()) {
      kdDebug() << "      (related to " << t->getRelatedTo()->getSummary() << ")" << endl;
    }

    QList<Event> l = t->getRelations();
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
}

QMap<Todo *,KOTodoViewItem *>::ConstIterator
  KOTodoView::insertTodoItem(Todo *todo)
{
//  kdDebug() << "KOTodoView::insertTodoItem(): " << todo->getSummary() << endl;
  // TODO: Check, if dynmaic cast is necessary
  Todo *relatedTodo = dynamic_cast<Todo *>(todo->relatedTo());

  if (relatedTodo) {
//    kdDebug() << "  has Related" << endl;
    QMap<Todo *,KOTodoViewItem *>::ConstIterator itemIterator;
    itemIterator = mTodoMap.find(relatedTodo);
    if (itemIterator == mTodoMap.end()) {
//      kdDebug() << "    related not yet in list" << endl;
      itemIterator = insertTodoItem (relatedTodo);
    }
    KOTodoViewItem *todoItem = new KOTodoViewItem(*itemIterator,todo);
    todoItem->setOpen(true);
    return mTodoMap.insert(todo,todoItem);
  } else {
//    kdDebug() << "  no Related" << endl;
    KOTodoViewItem *todoItem = new KOTodoViewItem(mTodoListView,todo);
    todoItem->setOpen(true);
    return mTodoMap.insert(todo,todoItem);
  }
}


void KOTodoView::updateConfig()
{
  // to be implemented.
}

QList<Incidence> KOTodoView::getSelected()
{
  QList<Incidence> selected;

  KOTodoViewItem *item = (KOTodoViewItem *)(mTodoListView->selectedItem());
  if (item) selected.append(item->event());

  return selected;
}

QList<Todo> KOTodoView::selectedTodos()
{
  QList<Todo> selected;

  KOTodoViewItem *item = (KOTodoViewItem *)(mTodoListView->selectedItem());
  if (item) selected.append(item->event());

  return selected;
}

void KOTodoView::changeEventDisplay(Event *, int)
{
  updateView();
}

void KOTodoView::selectDates(const QDateList)
{
}
 
void KOTodoView::selectEvents(QList<Event>)
{
  kdDebug() << "KOTodoView::selectEvents(): not yet implemented" << endl;
}

void KOTodoView::printPreview(CalPrinter *calPrinter, const QDate &fd,
                              const QDate &td)
{
  calPrinter->preview(CalPrinter::Todolist, fd, td);
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
    QList<Todo> todoCal = mCalendar->getTodoList();

    Todo *aTodo;
    for (aTodo = todoCal.first(); aTodo; aTodo = todoCal.next()) {
    if (aTodo->isCompleted())
      mCalendar->deleteTodo(aTodo);
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
