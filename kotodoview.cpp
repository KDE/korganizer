/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <qlayout.h>
#include <qheader.h>
#include <qcursor.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include <libkcal/icaldrag.h>
#include <libkcal/vcaldrag.h>
#include <libkcal/dndfactory.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcecalendar.h>

#ifndef KORG_NOPRINTER
#include "calprinter.h"
#endif
#include "docprefs.h"

#include "koincidencetooltip.h"
#include "kodialogmanager.h"
#include "kotodoview.h"
#include "koglobals.h"
using namespace KOrg;
#include "kotodoview.moc"

const int KOTodoView::POPUP_UNSUBTODO=1234;

KOTodoListViewToolTip::KOTodoListViewToolTip (QWidget* parent,
                                              KOTodoListView* lv )
  :QToolTip(parent)
{
  todolist=lv;
}

void KOTodoListViewToolTip::maybeTip( const QPoint & pos)
{
  QRect r;
  int headerPos;
  int col=todolist->header()->sectionAt(todolist->contentsX() + pos.x());
  KOTodoViewItem *i=(KOTodoViewItem *)todolist->itemAt(pos);

  /* Check wether a tooltip is necessary. */
  if( i && KOPrefs::instance()->mEnableToolTips )
  {

    /* Calculate the rectangle. */
    r=todolist->itemRect(i);
    headerPos = todolist->header()->sectionPos(col)-todolist->contentsX();
    r.setLeft( (headerPos < 0 ? 0 : headerPos) );
    r.setRight(headerPos + todolist->header()->sectionSize(col));

    /* Show the tip */
    QString tipText;
    ToolTipVisitor v;
    if (v.act(i->todo(), &tipText, true)) {
      tip(r, tipText);
    }
  }

}



KOTodoListView::KOTodoListView( QWidget *parent, const char *name )
  : KListView( parent, name ), mCalendar( 0 )
{
  mOldCurrent = 0;
  mMousePressed = false;

  /* Create a Tooltip */
  tooltip = new KOTodoListViewToolTip( viewport(), this );
}

KOTodoListView::~KOTodoListView()
{
  delete tooltip;
}

void KOTodoListView::setCalendar( Calendar *cal )
{
  mCalendar = cal;
  setAcceptDrops( mCalendar );
  viewport()->setAcceptDrops( mCalendar );
}

bool KOTodoListView::event(QEvent *e)
{
  int tmp=0;
  KOTodoViewItem *i;

  /* Checks for an ApplicationPaletteChange event and updates
   * the small Progress bars to make therm have the right colors. */
  if(e->type()==QEvent::ApplicationPaletteChange)
  {

    KListView::event(e);
    i=(KOTodoViewItem *)itemAtIndex(tmp);

    while(i!=0)
    {
      i->construct();
      tmp++;
      i=(KOTodoViewItem *)itemAtIndex(tmp);
    }

  }

  return (KListView::event(e) || e->type()==QEvent::ApplicationPaletteChange);
}

void KOTodoListView::contentsDragEnterEvent(QDragEnterEvent *e)
{
#ifndef KORG_NODND
//  kdDebug(5850) << "KOTodoListView::contentsDragEnterEvent" << endl;
  if ( !ICalDrag::canDecode( e ) && !VCalDrag::canDecode( e ) &&
       !QTextDrag::canDecode( e ) ) {
    e->ignore();
    return;
  }

  mOldCurrent = currentItem();
#endif
}


void KOTodoListView::contentsDragMoveEvent(QDragMoveEvent *e)
{
#ifndef KORG_NODND
//  kdDebug(5850) << "KOTodoListView::contentsDragMoveEvent" << endl;

  if ( !ICalDrag::canDecode( e ) && !VCalDrag::canDecode( e ) &&
       !QTextDrag::canDecode( e ) ) {
    e->ignore();
    return;
  }

  e->accept();
#endif
}

void KOTodoListView::contentsDragLeaveEvent( QDragLeaveEvent * )
{
#ifndef KORG_NODND
//  kdDebug(5850) << "KOTodoListView::contentsDragLeaveEvent" << endl;

  setCurrentItem(mOldCurrent);
  setSelected(mOldCurrent,true);
#endif
}

void KOTodoListView::contentsDropEvent( QDropEvent *e )
{
#ifndef KORG_NODND
//  kdDebug(5850) << "KOTodoListView::contentsDropEvent" << endl;

  if ( !mCalendar ||
       ( !ICalDrag::canDecode( e ) && !VCalDrag::canDecode( e ) &&
         !QTextDrag::canDecode( e ) ) ) {
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
    if (destination) destinationEvent = destination->todo();

    Todo *existingTodo = mCalendar->todo(todo->uid());

    if(existingTodo) {
//      kdDebug(5850) << "Drop existing Todo" << endl;
      Incidence *to = destinationEvent;
      while(to) {
        if (to->uid() == todo->uid()) {
          KMessageBox::sorry(this,
              i18n("Cannot move To-Do to itself or a child of itself"),
              i18n("Drop To-Do"));
          delete todo;
          return;
        }
        to = to->relatedTo();
      }
      Todo*oldTodo = existingTodo->clone();
      existingTodo->setRelatedTo(destinationEvent);

      emit todoDropped( todo );
      emit todoChanged( oldTodo, todo );
      delete oldTodo;
      delete todo;
    } else {
//      kdDebug(5850) << "Drop new Todo" << endl;
      todo->setRelatedTo(destinationEvent);
      if ( !mCalendar->addTodo( todo ) ) {
        KODialogManager::errorSaveTodo( this );
        return;
      }

      emit todoDropped(todo);
      emit todoAdded( todo );
    }
  }
  else {
    QString text;
    if (QTextDrag::decode(e,text)) {
      //QListViewItem *qlvi = itemAt( contentsToViewport(e->pos()) );
      KOTodoViewItem *todoi = static_cast<KOTodoViewItem *>(itemAt( contentsToViewport(e->pos()) ));
      kdDebug(5850) << "Dropped : " << text << endl;
      QStringList emails = QStringList::split(",",text);
      for(QStringList::ConstIterator it = emails.begin();it!=emails.end();++it) {
        kdDebug(5850) << " Email: " << (*it) << endl;
        int pos = (*it).find("<");
        QString name = (*it).left(pos);
        QString email = (*it).mid(pos);
        if (!email.isEmpty() && todoi) {
          todoi->todo()->addAttendee(new Attendee(name,email));
        }
      }
    }
    else {
      kdDebug(5850) << "KOTodoListView::contentsDropEvent(): Todo from drop not decodable" << endl;
      e->ignore();
    }
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
      if (e->button()==Qt::LeftButton) {
        mPressPos = e->pos();
        mMousePressed = true;
      }
    }
  }
}

void KOTodoListView::contentsMouseMoveEvent(QMouseEvent* e)
{
#ifndef KORG_NODND
//  kdDebug(5850) << "KOTodoListView::contentsMouseMoveEvent()" << endl;
  QListView::contentsMouseMoveEvent(e);
  if (mMousePressed && (mPressPos - e->pos()).manhattanLength() >
      QApplication::startDragDistance()) {
    mMousePressed = false;
    QListViewItem *item = itemAt(contentsToViewport(mPressPos));
    if ( item && mCalendar ) {
//      kdDebug(5850) << "Start Drag for item " << item->text(0) << endl;
      DndFactory factory( mCalendar );
      ICalDrag *vd = factory.createDrag(
                          ((KOTodoViewItem *)item)->todo(),viewport());
      if (vd->drag()) {
        kdDebug(5850) << "KOTodoListView::contentsMouseMoveEvent(): Delete drag source" << endl;
      }
/*
      QString source = fullPath(item);
      if ( QFile::exists(source) ) {
        KURL url;
        url.setPath(source);
        KURLDrag* ud = KURLDrag::newDrag(KURL::List(url), viewport());
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

  emit doubleClicked(item,vp,0);
}

/////////////////////////////////////////////////////////////////////////////

KOQuickTodo::KOQuickTodo(QWidget *parent) :
  QLineEdit(parent)
{
  setText(i18n("Click to add a new Todo"));
  setPaletteForegroundColor(gray);
}

void KOQuickTodo::focusInEvent(QFocusEvent *ev)
{
  if ( text()==i18n("Click to add a new Todo") )
    setText(QString::null);
  setPaletteForegroundColor(parentWidget()->paletteForegroundColor());
  QLineEdit::focusInEvent(ev);
}

void KOQuickTodo::focusOutEvent(QFocusEvent *ev)
{
  setText(i18n("Click to add a new Todo"));
  setPaletteForegroundColor(gray);
  QLineEdit::focusOutEvent(ev);
}

/////////////////////////////////////////////////////////////////////////////

KOTodoView::KOTodoView( Calendar *calendar, QWidget *parent, const char* name)
  : KOrg::BaseView( calendar, parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  QLabel *title = new QLabel( i18n("To-do items:"), this );
  title->setFrameStyle( QFrame::Panel | QFrame::Raised );
  topLayout->addWidget( title );

  mQuickAdd = new KOQuickTodo( this );
  topLayout->addWidget( mQuickAdd );

  if ( !KOPrefs::instance()->mEnableQuickTodo ) mQuickAdd->hide();

  mTodoListView = new KOTodoListView( this );
  topLayout->addWidget( mTodoListView );

  mTodoListView->setRootIsDecorated( true );
  mTodoListView->setAllColumnsShowFocus( true );

  mTodoListView->setShowSortIndicator( true );

  mTodoListView->addColumn( i18n("Summary") );
  mTodoListView->addColumn( i18n("Priority") );
  mTodoListView->setColumnAlignment( 1, AlignHCenter );
  mTodoListView->addColumn( i18n("Complete") );
  mTodoListView->setColumnAlignment( 2, AlignRight );
  mTodoListView->addColumn( i18n("Due Date") );
  mTodoListView->setColumnAlignment( 3, AlignHCenter );
  mTodoListView->addColumn( i18n("Due Time") );
  mTodoListView->setColumnAlignment( 4, AlignHCenter );
  mTodoListView->addColumn( i18n("Categories") );
#if 0
  mTodoListView->addColumn( i18n("Sort Id") );
  mTodoListView->setColumnAlignment( 4, AlignHCenter );
#endif

  mTodoListView->setMinimumHeight( 60 );
  mTodoListView->setItemsRenameable( true );
  mTodoListView->setRenameable( 0 );

  mTodoListView->setColumnWidthMode( 0, QListView::Manual );
  mTodoListView->setColumnWidthMode( 1, QListView::Manual );
  mTodoListView->setColumnWidthMode( 2, QListView::Manual );
  mTodoListView->setColumnWidthMode( 3, QListView::Manual );
  mTodoListView->setColumnWidthMode( 4, QListView::Manual );
  mTodoListView->setColumnWidthMode( 5, QListView::Manual );
#if 0
  mTodoListView->setColumnWidthMode( 6, QListView::Manual );
#endif

  mPriorityPopupMenu = new QPopupMenu( this );
  for ( int i = 1; i <= 5; i++ ) {
    QString label = QString ("%1").arg( i );
    mPriority[ mPriorityPopupMenu->insertItem( label ) ] = i;
  }
  connect( mPriorityPopupMenu, SIGNAL( activated( int ) ),
           SLOT( setNewPriority( int ) ));

  mPercentageCompletedPopupMenu = new QPopupMenu(this);
  for (int i = 0; i <= 100; i+=20) {
    QString label = QString ("%1 %").arg (i);
    mPercentage[mPercentageCompletedPopupMenu->insertItem (label)] = i;
  }
  connect( mPercentageCompletedPopupMenu, SIGNAL( activated( int ) ),
           SLOT( setNewPercentage( int ) ) );

  mItemPopupMenu = new QPopupMenu(this);
  mItemPopupMenu->insertItem(i18n("Show"), this,
                             SLOT (showTodo()));
  mItemPopupMenu->insertItem(i18n("Edit..."), this,
                             SLOT (editTodo()));
  mItemPopupMenu->insertItem(KOGlobals::self()->smallIconSet("editdelete"), i18n("Delete"), this,
                             SLOT (deleteTodo()));
  mItemPopupMenu->insertSeparator();
  mItemPopupMenu->insertItem(KOGlobals::self()->smallIconSet("todo"), i18n("New To-Do..."), this,
                             SLOT (newTodo()));
  mItemPopupMenu->insertItem(i18n("New Sub-To-Do..."), this,
                             SLOT (newSubTodo()));
  mItemPopupMenu->insertItem( i18n("Make Sub-To-Do Independent"), this,
      SIGNAL( unSubTodoSignal() ), 0, POPUP_UNSUBTODO );
  mItemPopupMenu->insertSeparator();
  mItemPopupMenu->insertItem(i18n("delete completed To-Dos","Purge Completed"),
                             this, SLOT( purgeCompleted() ) );

  mPopupMenu = new QPopupMenu(this);
  mPopupMenu->insertItem(KOGlobals::self()->smallIconSet("todo"), i18n("New To-Do..."), this,
                         SLOT (newTodo()));
  mPopupMenu->insertItem(i18n("delete completed To-Dos","Purge Completed"),
                         this, SLOT(purgeCompleted()));

  mDocPrefs = new DocPrefs( name );

  // Double clicking conflicts with opening/closing the subtree
  connect( mTodoListView, SIGNAL( doubleClicked( QListViewItem *,
                                                 const QPoint &, int ) ),
           SLOT( editItem( QListViewItem *, const QPoint &, int ) ) );
  connect( mTodoListView, SIGNAL( returnPressed( QListViewItem * ) ),
           SLOT( editItem( QListViewItem * ) ) );
  connect( mTodoListView, SIGNAL( contextMenuRequested( QListViewItem *,
                                                        const QPoint &, int ) ),
           SLOT( popupMenu( QListViewItem *, const QPoint &, int ) ) );
  connect( mTodoListView, SIGNAL( clicked( QListViewItem * ) ),
           SLOT( itemClicked( QListViewItem * ) ) );
  connect( mTodoListView, SIGNAL( todoDropped( Todo * ) ),
           SLOT( updateView() ) );
  connect( mTodoListView, SIGNAL( expanded( QListViewItem * ) ),
           SLOT( itemStateChanged( QListViewItem * ) ) );
  connect( mTodoListView, SIGNAL( collapsed( QListViewItem * ) ),
           SLOT( itemStateChanged( QListViewItem * ) ) );

#if 0
  connect(mTodoListView,SIGNAL(selectionChanged(QListViewItem *)),
          SLOT(selectionChanged(QListViewItem *)));
  connect(mTodoListView,SIGNAL(clicked(QListViewItem *)),
          SLOT(selectionChanged(QListViewItem *)));
  connect(mTodoListView,SIGNAL(pressed(QListViewItem *)),
          SLOT(selectionChanged(QListViewItem *)));
#endif
  connect( mTodoListView, SIGNAL(selectionChanged() ),
           SLOT( processSelectionChange() ) );
  connect( mQuickAdd, SIGNAL( returnPressed () ),
           SLOT( addQuickTodo() ) );
  connect( mTodoListView, SIGNAL( todoChanged( Todo*, Todo* ) ),
           SIGNAL( todoChanged( Todo*, Todo* ) ) );
  connect( mTodoListView, SIGNAL( todoAdded( Todo* ) ),
           SIGNAL( todoAdded( Todo* ) ) );
}

KOTodoView::~KOTodoView()
{
  delete mDocPrefs;
}

void KOTodoView::setCalendar( Calendar *cal )
{
  BaseView::setCalendar( cal );
  mTodoListView->setCalendar( cal );
}

void KOTodoView::updateView()
{
//  kdDebug(5850) << "KOTodoView::updateView()" << endl;

  mTodoListView->clear();

  Todo::List todoList = calendar()->todos();

/*
  kdDebug(5850) << "KOTodoView::updateView(): Todo List:" << endl;
  Event *t;
  for(t = todoList.first(); t; t = todoList.next()) {
    kdDebug(5850) << "  " << t->getSummary() << endl;

    if (t->getRelatedTo()) {
      kdDebug(5850) << "      (related to " << t->getRelatedTo()->getSummary() << ")" << endl;
    }

    QPtrList<Event> l = t->getRelations();
    Event *c;
    for(c=l.first();c;c=l.next()) {
      kdDebug(5850) << "    - relation: " << c->getSummary() << endl;
    }
  }
*/

  // Put for each Event a KOTodoViewItem in the list view. Don't rely on a
  // specific order of events. That means that we have to generate parent items
  // recursively for proper hierarchical display of Todos.
  mTodoMap.clear();
  Todo::List::ConstIterator it;
  for( it = todoList.begin(); it != todoList.end(); ++it ) {
    if ( !mTodoMap.contains( *it ) ) {
      insertTodoItem( *it );
    }
  }

  // Restore opened/closed state
  mTodoListView->blockSignals( true );
  if( mDocPrefs ) restoreItemState( mTodoListView->firstChild() );
  mTodoListView->blockSignals( false );

  processSelectionChange();
}

void KOTodoView::restoreItemState( QListViewItem *item )
{
  while( item ) {
    KOTodoViewItem *todoItem = (KOTodoViewItem *)item;
    todoItem->setOpen( mDocPrefs->readBoolEntry( todoItem->todo()->uid() ) );
    if( item->childCount() > 0 ) restoreItemState( item->firstChild() );
    item = item->nextSibling();
  }
}


QMap<Todo *,KOTodoViewItem *>::ConstIterator
  KOTodoView::insertTodoItem(Todo *todo)
{
//  kdDebug(5850) << "KOTodoView::insertTodoItem(): " << todo->getSummary() << endl;
  // TODO: Check, if dynmaic cast is necessary
  Incidence *incidence = todo->relatedTo();
  if (incidence && incidence->type() == "Todo") {
    Todo *relatedTodo = static_cast<Todo *>(incidence);

//    kdDebug(5850) << "  has Related" << endl;
    QMap<Todo *,KOTodoViewItem *>::ConstIterator itemIterator;
    itemIterator = mTodoMap.find(relatedTodo);
    if (itemIterator == mTodoMap.end()) {
//      kdDebug(5850) << "    related not yet in list" << endl;
      itemIterator = insertTodoItem (relatedTodo);
    }
    // isn't this pretty stupid? We give one Todo  to the KOTodoViewItem
    // and one into the map. Sure finding is more easy but why? -zecke
    KOTodoViewItem *todoItem = new KOTodoViewItem(*itemIterator,todo,this);
    return mTodoMap.insert(todo,todoItem);
  } else {
//    kdDebug(5850) << "  no Related" << endl;
      // see above -zecke
    KOTodoViewItem *todoItem = new KOTodoViewItem(mTodoListView,todo,this);
    return mTodoMap.insert(todo,todoItem);
  }
}


void KOTodoView::updateConfig()
{
  mTodoListView->repaintContents();
}

Incidence::List KOTodoView::selectedIncidences()
{
  Incidence::List selected;

  KOTodoViewItem *item = (KOTodoViewItem *)(mTodoListView->selectedItem());
//  if (!item) item = mActiveItem;
  if (item) selected.append(item->todo());

  return selected;
}

Todo::List KOTodoView::selectedTodos()
{
  Todo::List selected;

  KOTodoViewItem *item = (KOTodoViewItem *)(mTodoListView->selectedItem());
//  if (!item) item = mActiveItem;
  if (item) selected.append(item->todo());

  return selected;
}

void KOTodoView::changeEventDisplay(Event *, int)
{
  updateView();
}

void KOTodoView::showDates(const QDate &, const QDate &)
{
}

void KOTodoView::showEvents( const Event::List & )
{
  kdDebug(5850) << "KOTodoView::selectEvents(): not yet implemented" << endl;
}

void KOTodoView::printPreview(CalPrinter *calPrinter, const QDate &fd,
                              const QDate &td)
{
#ifndef KORG_NOPRINTER
  calPrinter->preview(CalPrinter::Todolist, fd, td);
#endif
}

CalPrinter::PrintType KOTodoView::printType()
{
  return CalPrinter::Todolist;
}

void KOTodoView::editItem( QListViewItem *item )
{
  emit editTodoSignal( static_cast<KOTodoViewItem *>( item )->todo() );
}

void KOTodoView::editItem( QListViewItem *item, const QPoint &, int )
{
  editItem( item );
}

void KOTodoView::showItem( QListViewItem *item, const QPoint &, int )
{
  emit showTodoSignal( static_cast<KOTodoViewItem *>( item )->todo() );
}

void KOTodoView::popupMenu( QListViewItem *item, const QPoint &, int column )
{
  mActiveItem = static_cast<KOTodoViewItem *>( item );
  if ( item ) {
    switch ( column ) {
      case 1:
        mPriorityPopupMenu->popup( QCursor::pos () );
        break;
      case 2:
        mPercentageCompletedPopupMenu->popup( QCursor::pos () );
        break;
      case 5:
        getCategoryPopupMenu(
            static_cast<KOTodoViewItem *>( item ) )->popup( QCursor::pos () );
        break;
      default:
        mItemPopupMenu->setItemEnabled( POPUP_UNSUBTODO,
                                        mActiveItem->todo()->relatedTo() );
        mItemPopupMenu->popup( QCursor::pos() );
    }
  } else mPopupMenu->popup( QCursor::pos() );
}

void KOTodoView::newTodo()
{
  emit newTodoSignal();
}

void KOTodoView::newSubTodo()
{
  if (mActiveItem) {
    emit newSubTodoSignal(mActiveItem->todo());
  }
}

void KOTodoView::editTodo()
{
  if (mActiveItem) {
    emit editTodoSignal(mActiveItem->todo());
  }
}

void KOTodoView::showTodo()
{
  if (mActiveItem) {
    emit showTodoSignal(mActiveItem->todo());
  }
}

void KOTodoView::deleteTodo()
{
  if (mActiveItem) {
    if (mActiveItem->childCount()) {
      KMessageBox::sorry(this,i18n("Cannot delete To-Do which has children."),
                         i18n("Delete To-Do"));
    } else {
      emit deleteTodoSignal(mActiveItem->todo());
    }
  }
}

void KOTodoView::setNewPriority(int index)
{
  if (mActiveItem && !mActiveItem->todo()->isReadOnly ()) {
    Todo*todo = mActiveItem->todo();
    Todo*oldTodo = todo->clone();
    todo->setPriority(mPriority[index]);
    mActiveItem->construct();
    emit todoModifiedSignal ( todo, oldTodo, KOGlobals::PRIORITY_MODIFIED );
    delete oldTodo;
  }
}

void KOTodoView::setNewPercentage(int index)
{
  if ( mActiveItem && !mActiveItem->todo()->isReadOnly () ) {
    Todo*todo = mActiveItem->todo();
    Todo*oldTodo = todo->clone();

    if (mPercentage[index] == 100) {
      todo->setCompleted(QDateTime::currentDateTime());
    } else {
      todo->setCompleted(false);
    }
    todo->setPercentComplete(mPercentage[index]);
    mActiveItem->construct();
    emit todoModifiedSignal( todo, oldTodo, KOGlobals::COMPLETION_MODIFIED );
    delete oldTodo;
  }
}


QPopupMenu *KOTodoView::getCategoryPopupMenu( KOTodoViewItem *todoItem )
{
  QPopupMenu *tempMenu = new QPopupMenu( this );
  QStringList checkedCategories = todoItem->todo()->categories();

  tempMenu->setCheckable( true );
  QStringList::Iterator it;
  for ( it = KOPrefs::instance()->mCustomCategories.begin();
        it != KOPrefs::instance()->mCustomCategories.end();
        ++it ) {
    int index = tempMenu->insertItem( *it );
    mCategory[ index ] = *it;
    if ( checkedCategories.find( *it ) != checkedCategories.end() )
      tempMenu->setItemChecked( index, true );
  }

  connect ( tempMenu, SIGNAL( activated( int ) ),
            SLOT( changedCategories( int ) ) );
  return tempMenu;
}

void KOTodoView::changedCategories(int index)
{
  if (mActiveItem && !mActiveItem->todo()->isReadOnly ()) {
    Todo*todo = mActiveItem->todo();
    Todo*oldTodo = todo->clone();
    QStringList categories = todo->categories ();
    if (categories.find (mCategory[index]) != categories.end ())
      categories.remove (mCategory[index]);
    else
      categories.insert (categories.end(), mCategory[index]);
    categories.sort ();
    todo->setCategories (categories);
    mActiveItem->construct();
    emit todoModifiedSignal( todo, oldTodo, KOGlobals::CATEGORY_MODIFIED);
    delete oldTodo;
  }
}

void KOTodoView::itemClicked(QListViewItem *item)
{
  if (!item) return;

  KOTodoViewItem *todoItem = (KOTodoViewItem *)item;
  int completed = todoItem->todo()->isCompleted();  // Completed or not?

  if (todoItem->isOn()) {
    if (!completed) {
      todoItem->todo()->setCompleted(QDateTime::currentDateTime());
    }
  } else {
    if (completed) {
      todoItem->todo()->setCompleted(false);
    }
  }
}

void KOTodoView::setDocumentId( const QString &id )
{
  kdDebug(5850) << "KOTodoView::setDocumentId()" << endl;

  mDocPrefs->setDoc( id );
}

void KOTodoView::itemStateChanged( QListViewItem *item )
{
  if (!item) return;

  KOTodoViewItem *todoItem = (KOTodoViewItem *)item;

//  kdDebug(5850) << "KOTodoView::itemStateChanged(): " << todoItem->todo()->summary() << endl;

  if( mDocPrefs ) mDocPrefs->writeEntry( todoItem->todo()->uid(), todoItem->isOpen() );
}

void KOTodoView::saveLayout(KConfig *config, const QString &group) const
{
  mTodoListView->saveLayout(config,group);
}

void KOTodoView::restoreLayout(KConfig *config, const QString &group)
{
  mTodoListView->restoreLayout(config,group);
}

void KOTodoView::processSelectionChange()
{
//  kdDebug(5850) << "KOTodoView::processSelectionChange()" << endl;

  KOTodoViewItem *item =
    static_cast<KOTodoViewItem *>( mTodoListView->selectedItem() );

  if ( !item ) {
    emit incidenceSelected( 0 );
  } else {
    emit incidenceSelected( item->todo() );
  }
}

void KOTodoView::modified(bool b)
{
  emit isModified(b);
}

void KOTodoView::clearSelection()
{
  mTodoListView->selectAll( false );
}

void KOTodoView::purgeCompleted()
{
  emit purgeCompletedSignal();
}

void KOTodoView::addQuickTodo()
{
  Todo *todo = new Todo();
  todo->setSummary( mQuickAdd->text() );
  todo->setOrganizer( KOPrefs::instance()->email() );
  if ( !calendar()->addTodo( todo ) ) {
    KODialogManager::errorSaveTodo( this );
    return;
  }
  mQuickAdd->setText( QString::null );
  emit todoAdded( todo );
  updateView();
}
