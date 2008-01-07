/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>
#include <qheader.h>
#include <qcursor.h>
#include <qlabel.h>
#include <qtimer.h>

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
#include <libkcal/calfilter.h>
#include <libkcal/incidenceformatter.h>

#include <libkdepim/clicklineedit.h>
#include <libkdepim/kdatepickerpopup.h>

#include <libemailfunctions/email.h>

#include "docprefs.h"

#include "koincidencetooltip.h"
#include "kodialogmanager.h"
#include "kotodoview.h"
#include "koprefs.h"
#include "koglobals.h"
using namespace KOrg;
#include "kotodoviewitem.h"
#include "kotodoview.moc"
#ifndef KORG_NOPRINTER
#include "kocorehelper.h"
#include "calprinter.h"
#endif

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
    QString tipText( IncidenceFormatter::toolTipString( i->todo() ) );;
    if ( !tipText.isEmpty() ) {
      tip(r, tipText);
    }
  }

}



KOTodoListView::KOTodoListView( QWidget *parent, const char *name )
  : KListView( parent, name ), mCalendar( 0 ), mChanger( 0 )
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
  kdDebug(5850) << "KOTodoListView::contentsDropEvent" << endl;

  if ( !mCalendar || !mChanger ||
       ( !ICalDrag::canDecode( e ) && !VCalDrag::canDecode( e ) &&
         !QTextDrag::canDecode( e ) ) ) {
    e->ignore();
    return;
  }

  DndFactory factory( mCalendar );
  Todo *todo = factory.createDropTodo(e);

  if ( todo ) {
    e->acceptAction();

    KOTodoViewItem *destination =
        (KOTodoViewItem *)itemAt(contentsToViewport(e->pos()));
    Todo *destinationEvent = 0;
    if (destination) destinationEvent = destination->todo();

    Todo *existingTodo = mCalendar->todo(todo->uid());

    if( existingTodo ) {
       kdDebug(5850) << "Drop existing Todo " << existingTodo << " onto " << destinationEvent << endl;
      Incidence *to = destinationEvent;
      while(to) {
        if (to->uid() == todo->uid()) {
          KMessageBox::information(this,
              i18n("Cannot move to-do to itself or a child of itself."),
              i18n("Drop To-do"), "NoDropTodoOntoItself" );
          delete todo;
          return;
        }
        to = to->relatedTo();
      }
      Todo*oldTodo = existingTodo->clone();
      if ( mChanger->beginChange( existingTodo ) ) {
        existingTodo->setRelatedTo( destinationEvent );
        mChanger->changeIncidence( oldTodo, existingTodo, KOGlobals::RELATION_MODIFIED );
        mChanger->endChange( existingTodo );
      } else {
        KMessageBox::sorry( this, i18n("Unable to change to-do's parent, "
                            "because the to-do cannot be locked.") );
      }
      delete oldTodo;
      delete todo;
    } else {
//      kdDebug(5850) << "Drop new Todo" << endl;
      todo->setRelatedTo(destinationEvent);
      if ( !mChanger->addIncidence( todo, this ) ) {
        KODialogManager::errorSaveIncidence( this, todo );
        delete todo;
        return;
      }
    }
  }
  else {
    QString text;
    KOTodoViewItem *todoi = dynamic_cast<KOTodoViewItem *>(itemAt( contentsToViewport(e->pos()) ));
    if ( ! todoi ) {
      // Not dropped on a todo item:
      e->ignore();
      kdDebug( 5850 ) << "KOTodoListView::contentsDropEvent(): Not dropped on a todo item" << endl;
      kdDebug( 5850 ) << "TODO: Create a new todo with the given data" << endl;
      // FIXME: Create a new todo with the given text/contact/whatever
    } else if ( QTextDrag::decode(e, text) ) {
      //QListViewItem *qlvi = itemAt( contentsToViewport(e->pos()) );
      kdDebug(5850) << "Dropped : " << text << endl;
      Todo*todo = todoi->todo();
      if( mChanger->beginChange( todo ) ) {
        Todo*oldtodo = todo->clone();

        if( text.startsWith( "file:" ) ) {
          todo->addAttachment( new Attachment( text ) );
        } else {
          QStringList emails = KPIM::splitEmailAddrList( text );
          for(QStringList::ConstIterator it = emails.begin();it!=emails.end();++it) {
            kdDebug(5850) << " Email: " << (*it) << endl;
            int pos = (*it).find("<");
            QString name = (*it).left(pos);
            QString email = (*it).mid(pos);
            if (!email.isEmpty() && todoi) {
              todo->addAttendee( new Attendee( name, email ) );
            }
          }
        }
        mChanger->changeIncidence( oldtodo, todo );
        mChanger->endChange( todo );
      } else {
        KMessageBox::sorry( this, i18n("Unable to add attendees to the to-do, "
            "because the to-do cannot be locked.") );
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

KOTodoView::KOTodoView( Calendar *calendar, QWidget *parent, const char* name)
  : KOrg::BaseView( calendar, parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  QLabel *title = new QLabel( i18n("To-dos:"), this );
  title->setFrameStyle( QFrame::Panel | QFrame::Raised );
  topLayout->addWidget( title );

  mQuickAdd = new KPIM::ClickLineEdit( this, i18n( "Click to add a new to-do" ) );
  mQuickAdd->setAcceptDrops( false );
  topLayout->addWidget( mQuickAdd );

  if ( !KOPrefs::instance()->mEnableQuickTodo ) mQuickAdd->hide();

  mTodoListView = new KOTodoListView( this );
  topLayout->addWidget( mTodoListView );

  mTodoListView->setRootIsDecorated( true );
  mTodoListView->setAllColumnsShowFocus( true );

  mTodoListView->setShowSortIndicator( true );

  mTodoListView->addColumn( i18n("Summary") );
  mTodoListView->addColumn( i18n("Recurs") );
  mTodoListView->addColumn( i18n("Priority") );
  mTodoListView->setColumnAlignment( ePriorityColumn, AlignHCenter );
  mTodoListView->addColumn( i18n("Complete") );
  mTodoListView->setColumnAlignment( ePercentColumn, AlignRight );
  mTodoListView->addColumn( i18n("Due Date/Time") );
  mTodoListView->setColumnAlignment( eDueDateColumn, AlignLeft );
  mTodoListView->addColumn( i18n("Categories") );
#if 0
  mTodoListView->addColumn( i18n("Sort Id") );
  mTodoListView->setColumnAlignment( 4, AlignHCenter );
#endif

  mTodoListView->setMinimumHeight( 60 );
  mTodoListView->setItemsRenameable( true );
  mTodoListView->setRenameable( 0 );

  mTodoListView->setColumnWidthMode( eSummaryColumn, QListView::Manual );
  mTodoListView->setColumnWidthMode( eRecurColumn, QListView::Manual );
  mTodoListView->setColumnWidthMode( ePriorityColumn, QListView::Manual );
  mTodoListView->setColumnWidthMode( ePercentColumn, QListView::Manual );
  mTodoListView->setColumnWidthMode( eDueDateColumn, QListView::Manual );
  mTodoListView->setColumnWidthMode( eCategoriesColumn, QListView::Manual );
#if 0
  mTodoListView->setColumnWidthMode( eDescriptionColumn, QListView::Manual );
#endif

  mPriorityPopupMenu = new QPopupMenu( this );
  mPriority[ mPriorityPopupMenu->insertItem( i18n("Unspecified priority", "unspecified") ) ] = 0;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "1 (highest)") ) ] = 1;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "2" ) ) ] = 2;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "3" ) ) ] = 3;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "4" ) ) ] = 4;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "5 (medium)" ) ) ] = 5;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "6" ) ) ] = 6;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "7" ) ) ] = 7;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "8" ) ) ] = 8;
  mPriority[ mPriorityPopupMenu->insertItem( i18n( "9 (lowest)" ) ) ] = 9;
  connect( mPriorityPopupMenu, SIGNAL( activated( int ) ),
           SLOT( setNewPriority( int ) ));

  mPercentageCompletedPopupMenu = new QPopupMenu(this);
  for (int i = 0; i <= 100; i+=10) {
    QString label = QString ("%1 %").arg (i);
    mPercentage[mPercentageCompletedPopupMenu->insertItem (label)] = i;
  }
  connect( mPercentageCompletedPopupMenu, SIGNAL( activated( int ) ),
           SLOT( setNewPercentage( int ) ) );

  mMovePopupMenu = new KDatePickerPopup(
                             KDatePickerPopup::NoDate |
                             KDatePickerPopup::DatePicker |
                             KDatePickerPopup::Words );
  mCopyPopupMenu = new KDatePickerPopup(
                             KDatePickerPopup::NoDate |
                             KDatePickerPopup::DatePicker |
                             KDatePickerPopup::Words );


  connect( mMovePopupMenu, SIGNAL( dateChanged( QDate )),
           SLOT( setNewDate( QDate ) ) );
  connect( mCopyPopupMenu, SIGNAL( dateChanged( QDate )),
           SLOT( copyTodoToDate( QDate ) ) );

  mItemPopupMenu = new QPopupMenu(this);
  mItemPopupMenu->insertItem(i18n("&Show"), this,
                             SLOT (showTodo()));
  mItemPopupMenu->insertItem(i18n("&Edit..."), this,
                             SLOT (editTodo()), 0, ePopupEdit );
#ifndef KORG_NOPRINTER
  mItemPopupMenu->insertItem(KOGlobals::self()->smallIcon("printer1"), i18n("&Print..."), this, SLOT( printTodo() ) );
#endif
  mItemPopupMenu->insertItem(KOGlobals::self()->smallIconSet("editdelete"), i18n("&Delete"), this,
                             SLOT (deleteTodo()), 0, ePopupDelete );
  mItemPopupMenu->insertSeparator();
  mItemPopupMenu->insertItem(KOGlobals::self()->smallIconSet("todo"), i18n("New &To-do..."), this,
                             SLOT (newTodo()));
  mItemPopupMenu->insertItem(i18n("New Su&b-to-do..."), this,
                             SLOT (newSubTodo()));
  mItemPopupMenu->insertItem( i18n("&Make this To-do Independent"), this,
      SIGNAL( unSubTodoSignal() ), 0, ePopupUnSubTodo );
  mItemPopupMenu->insertItem( i18n("Make all Sub-to-dos &Independent"), this,
      SIGNAL( unAllSubTodoSignal() ), 0, ePopupUnAllSubTodo );
  mItemPopupMenu->insertSeparator();
  mItemPopupMenu->insertItem( i18n("&Copy To"), mCopyPopupMenu, ePopupCopyTo );
  mItemPopupMenu->insertItem(i18n("&Move To"), mMovePopupMenu, ePopupMoveTo );
  mItemPopupMenu->insertSeparator();
  mItemPopupMenu->insertItem(i18n("delete completed to-dos","Pur&ge Completed"),
                             this, SLOT( purgeCompleted() ) );

  connect( mMovePopupMenu, SIGNAL( dateChanged( QDate ) ),
           mItemPopupMenu, SLOT( hide() ) );
  connect( mCopyPopupMenu, SIGNAL( dateChanged( QDate ) ),
           mItemPopupMenu, SLOT( hide() ) );

  mPopupMenu = new QPopupMenu(this);
  mPopupMenu->insertItem(KOGlobals::self()->smallIconSet("todo"), i18n("&New To-do..."), this,
                         SLOT (newTodo()));
  mPopupMenu->insertItem(i18n("delete completed to-dos","&Purge Completed"),
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
  int oldPos = mTodoListView->contentsY();
  mItemsToDelete.clear();
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

  mTodoListView->setContentsPos( 0, oldPos );

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
  Incidence *incidence = todo->relatedTo();
  if (incidence && incidence->type() == "Todo") {
    // Use dynamic_cast, because in the future the related item might also be an event
    Todo *relatedTodo = dynamic_cast<Todo *>(incidence);

    // just make sure we know we have this item already to avoid endless recursion (Bug 101696)
    mTodoMap.insert(todo,0);

//    kdDebug(5850) << "  has Related" << endl;
    QMap<Todo *,KOTodoViewItem *>::ConstIterator itemIterator;
    itemIterator = mTodoMap.find(relatedTodo);
    if (itemIterator == mTodoMap.end()) {
//      kdDebug(5850) << "    related not yet in list" << endl;
      itemIterator = insertTodoItem (relatedTodo);
    }
    // isn't this pretty stupid? We give one Todo  to the KOTodoViewItem
    // and one into the map. Sure finding is more easy but why? -zecke
    KOTodoViewItem *todoItem;

    // in case we found a related parent, which has no KOTodoViewItem yet, this must
    // be the case where 2 items refer to each other, therefore simply create item as root item
    if ( *itemIterator == 0 ) {
      todo->setRelatedTo(0);  // break the recursion, else we will have troubles later
      todoItem = new KOTodoViewItem(mTodoListView,todo,this);
    }
    else
      todoItem = new KOTodoViewItem(*itemIterator,todo,this);

    return mTodoMap.insert(todo,todoItem);
  } else {
//    kdDebug(5850) << "  no Related" << endl;
      // see above -zecke
    KOTodoViewItem *todoItem = new KOTodoViewItem(mTodoListView,todo,this);
    return mTodoMap.insert(todo,todoItem);
  }
}

void KOTodoView::removeTodoItems()
{
  KOTodoViewItem *item;
  for ( item = mItemsToDelete.first(); item; item = mItemsToDelete.next() ) {
    Todo *todo = item->todo();
    if ( todo && mTodoMap.contains( todo ) ) {
      mTodoMap.remove( todo );
    }
    delete item;
  }
  mItemsToDelete.clear();
}


bool KOTodoView::scheduleRemoveTodoItem( KOTodoViewItem *todoItem )
{
  if ( todoItem ) {
    mItemsToDelete.append( todoItem );
    QTimer::singleShot( 0, this, SLOT( removeTodoItems() ) );
    return true;
  } else
    return false;
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

void KOTodoView::changeIncidenceDisplay(Incidence *incidence, int action)
{
  // The todo view only displays todos, so exit on all other incidences
  if ( incidence->type() != "Todo" )
    return;
  CalFilter *filter = calendar()->filter();
  bool isFiltered = filter && !filter->filterIncidence( incidence );
  Todo *todo = static_cast<Todo *>(incidence);
  if ( todo ) {
    KOTodoViewItem *todoItem = 0;
    if ( mTodoMap.contains( todo ) ) {
      todoItem = mTodoMap[todo];
    }
    switch ( action ) {
      case KOGlobals::INCIDENCEADDED:
      case KOGlobals::INCIDENCEEDITED:
        // If it's already there, edit it, otherwise just add
        if ( todoItem ) {
          if ( isFiltered ) {
            scheduleRemoveTodoItem( todoItem );
          } else {
            // correctly update changes in relations
            Todo*parent = dynamic_cast<Todo*>( todo->relatedTo() );
            KOTodoViewItem*parentItem = 0;
            if ( parent && mTodoMap.contains(parent) ) {
              parentItem = mTodoMap[ parent ];
            }
            if ( todoItem->parent() != parentItem ) {
              // The relations changed
              if ( parentItem ) {
                parentItem->insertItem( todoItem );
              } else {
                mTodoListView->insertItem( todoItem );
              }
            }
            todoItem->construct();
          }
        } else {
          if ( !isFiltered ) {
            insertTodoItem( todo );
          }
        }
        mTodoListView->sort();
        break;
      case KOGlobals::INCIDENCEDELETED:
        if ( todoItem ) {
          scheduleRemoveTodoItem( todoItem );
        }
        break;
      default:
        QTimer::singleShot( 0, this, SLOT( updateView() ) );
    }
  } else {
    // use a QTimer here, because when marking todos finished using
    // the checkbox, this slot gets called, but we cannot update the views
    // because we're still inside KOTodoViewItem::stateChange
    QTimer::singleShot(0,this,SLOT(updateView()));
  }
}

void KOTodoView::showDates(const QDate &, const QDate &)
{
}

void KOTodoView::showIncidences( const Incidence::List & )
{
  kdDebug(5850) << "KOTodoView::showIncidences( const Incidence::List & ): not yet implemented" << endl;
}

CalPrinterBase::PrintType KOTodoView::printType()
{
  if ( mTodoListView->selectedItem() ) {
    return CalPrinterBase::Incidence;
  } else {
    return CalPrinterBase::Todolist;
  }
}

void KOTodoView::editItem( QListViewItem *item )
{
  if (item)
    emit editIncidenceSignal( static_cast<KOTodoViewItem *>( item )->todo() );
}

void KOTodoView::editItem( QListViewItem *item, const QPoint &, int )
{
  editItem( item );
}

void KOTodoView::showItem( QListViewItem *item )
{
  if (item)
    emit showIncidenceSignal( static_cast<KOTodoViewItem *>( item )->todo() );
}

void KOTodoView::showItem( QListViewItem *item, const QPoint &, int )
{
  showItem( item );
}

void KOTodoView::popupMenu( QListViewItem *item, const QPoint &, int column )
{
  mActiveItem = static_cast<KOTodoViewItem *>( item );
  if ( mActiveItem && mActiveItem->todo() &&
       !mActiveItem->todo()->isReadOnly() ) {
    bool editable = !mActiveItem->todo()->isReadOnly();
    mItemPopupMenu->setItemEnabled( ePopupEdit, editable );
    mItemPopupMenu->setItemEnabled( ePopupDelete, editable );
    mItemPopupMenu->setItemEnabled( ePopupMoveTo, editable );
    mItemPopupMenu->setItemEnabled( ePopupCopyTo, editable );
    mItemPopupMenu->setItemEnabled( ePopupUnSubTodo, editable );
    mItemPopupMenu->setItemEnabled( ePopupUnAllSubTodo, editable );

    if ( editable ) {
      QDate date = mActiveItem->todo()->dtDue().date();
      if ( mActiveItem->todo()->hasDueDate () ) {
        mMovePopupMenu->datePicker()->setDate( date );
      } else {
        mMovePopupMenu->datePicker()->setDate( QDate::currentDate() );
      }
      switch ( column ) {
        case ePriorityColumn:
          mPriorityPopupMenu->popup( QCursor::pos() );
          break;
        case ePercentColumn: {
          mPercentageCompletedPopupMenu->popup( QCursor::pos() );
          break;
        }
        case eDueDateColumn:
          mMovePopupMenu->popup( QCursor::pos() );
          break;
        case eCategoriesColumn:
          getCategoryPopupMenu( mActiveItem )->popup( QCursor::pos() );
          break;
        default:
          mCopyPopupMenu->datePicker()->setDate( date );
          mCopyPopupMenu->datePicker()->setDate( QDate::currentDate() );
          mItemPopupMenu->setItemEnabled( ePopupUnSubTodo,
                                          mActiveItem->todo()->relatedTo() );
          mItemPopupMenu->setItemEnabled( ePopupUnAllSubTodo,
                                          !mActiveItem->todo()->relations().isEmpty() );
          mItemPopupMenu->popup( QCursor::pos() );
      }
    } else {
      mItemPopupMenu->popup( QCursor::pos() );
    }
  } else mPopupMenu->popup( QCursor::pos() );
}

void KOTodoView::newTodo()
{
  kdDebug() << k_funcinfo << endl;
  emit newTodoSignal( QDate::currentDate().addDays(7) );
}

void KOTodoView::newSubTodo()
{
  if (mActiveItem) {
    emit newSubTodoSignal(mActiveItem->todo());
  }
}

void KOTodoView::editTodo()
{
  editItem( mActiveItem );
}

void KOTodoView::showTodo()
{
  showItem( mActiveItem );
}

void KOTodoView::printTodo()
{
#ifndef KORG_NOPRINTER
  KOCoreHelper helper;
  CalPrinter printer( this, BaseView::calendar(), &helper );
  connect( this, SIGNAL(configChanged()), &printer, SLOT(updateConfig()) );

  Incidence::List selectedIncidences;
  selectedIncidences.append( mActiveItem->todo() );

  printer.print( KOrg::CalPrinterBase::Incidence,
                 QDate(), QDate(), selectedIncidences );
#endif
}

void KOTodoView::deleteTodo()
{
  if (mActiveItem) {
    emit deleteIncidenceSignal( mActiveItem->todo() );
  }
}

void KOTodoView::setNewPriority(int index)
{
  if ( !mActiveItem || !mChanger ) return;
  Todo *todo = mActiveItem->todo();
  if ( !todo->isReadOnly () &&
       mChanger->beginChange( todo ) ) {
    Todo *oldTodo = todo->clone();
    todo->setPriority(mPriority[index]);
    mActiveItem->construct();

    mChanger->changeIncidence( oldTodo, todo, KOGlobals::PRIORITY_MODIFIED );
    mChanger->endChange( todo );
    delete oldTodo;
  }
}

void KOTodoView::setNewPercentage( KOTodoViewItem *item, int percentage )
{
  kdDebug(5850) << "KOTodoView::setNewPercentage( " << percentage << "), item = " << item << endl;
  if ( !item || !mChanger  ) return;
  Todo *todo = item->todo();
  if ( !todo ) return;

  if ( !todo->isReadOnly () && mChanger->beginChange( todo ) ) {
    Todo *oldTodo = todo->clone();

/*  Old code to make sub-items's percentage related to this one's:
    QListViewItem *myChild = firstChild();
    KOTodoViewItem *item;
    while( myChild ) {
      item = static_cast<KOTodoViewItem*>(myChild);
      item->stateChange(state);
      myChild = myChild->nextSibling();
    }*/
    if ( percentage == 100 ) {
      todo->setCompleted( QDateTime::currentDateTime() );
      // If the todo does recur, it doesn't get set as completed. However, the
      // item is still checked. Uncheck it again.
      if ( !todo->isCompleted() ) item->setState( QCheckListItem::Off );
      else todo->setPercentComplete( percentage );
    } else {
      todo->setCompleted( false );
      todo->setPercentComplete( percentage );
    }
    item->construct();
    if ( todo->doesRecur() && percentage == 100 )
      mChanger->changeIncidence( oldTodo, todo, KOGlobals::COMPLETION_MODIFIED_WITH_RECURRENCE );
    else
      mChanger->changeIncidence( oldTodo, todo, KOGlobals::COMPLETION_MODIFIED );
    mChanger->endChange( todo );
    delete oldTodo;
  } else {
    item->construct();
    kdDebug(5850) << "No active item, active item is read-only, or locking failed" << endl;
  }
}

void KOTodoView::setNewPercentage( int index )
{
  setNewPercentage( mActiveItem, mPercentage[index] );
}

void KOTodoView::setNewDate( QDate date )
{
  if ( !mActiveItem || !mChanger ) return;
  Todo *todo = mActiveItem->todo();
  if ( !todo ) return;

  if ( !todo->isReadOnly() && mChanger->beginChange( todo ) ) {
    Todo *oldTodo = todo->clone();

    QDateTime dt;
    dt.setDate( date );

    if ( !todo->doesFloat() )
      dt.setTime( todo->dtDue().time() );

    if ( date.isNull() )
      todo->setHasDueDate( false );
    else if ( !todo->hasDueDate() )
      todo->setHasDueDate( true );
    todo->setDtDue( dt );

    mActiveItem->construct();
    mChanger->changeIncidence( oldTodo, todo, KOGlobals::COMPLETION_MODIFIED );
    mChanger->endChange( todo );
    delete oldTodo;
  } else {
    kdDebug(5850) << "No active item, active item is read-only, or locking failed" << endl;
  }
}

void KOTodoView::copyTodoToDate( QDate date )
{
  QDateTime dt( date );

  if ( mActiveItem && mChanger ) {
    Todo *newTodo = mActiveItem->todo()->clone();
    newTodo->recreate();

   newTodo->setHasDueDate( !date.isNull() );
   newTodo->setDtDue( dt );
   newTodo->setPercentComplete( 0 );

   // avoid forking
   if ( newTodo->doesRecur() )
     newTodo->recurrence()->unsetRecurs();

   mChanger->addIncidence( newTodo, this );
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
  if ( !mActiveItem || !mChanger ) return;
  Todo *todo = mActiveItem->todo();
  if ( !todo ) return;

  if ( !todo->isReadOnly() && mChanger->beginChange( todo ) ) {
    Todo *oldTodo = todo->clone();

    QStringList categories = todo->categories ();
    if ( categories.find( mCategory[index] ) != categories.end() )
      categories.remove( mCategory[index] );
    else
      categories.insert( categories.end(), mCategory[index] );
    categories.sort();
    todo->setCategories( categories );
    mActiveItem->construct();
    mChanger->changeIncidence( oldTodo, todo, KOGlobals::CATEGORY_MODIFIED );
    mChanger->endChange( todo );
    delete oldTodo;
  } else {
    kdDebug(5850) << "No active item, active item is read-only, or locking failed" << endl;
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

void KOTodoView::setNewPercentageDelayed( KOTodoViewItem *item, int percentage )
{
  mPercentChangedMap.append( qMakePair( item, percentage ) );

  QTimer::singleShot( 0, this, SLOT( processDelayedNewPercentage() ) );
}

void KOTodoView::processDelayedNewPercentage()
{
  QValueList< QPair< KOTodoViewItem *, int> >::Iterator it;
  for ( it = mPercentChangedMap.begin(); it != mPercentChangedMap.end(); ++it )
    setNewPercentage( (*it).first, (*it).second );

  mPercentChangedMap.clear();
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
  if ( ! mQuickAdd->text().stripWhiteSpace().isEmpty() ) {
    Todo *todo = new Todo();
    todo->setSummary( mQuickAdd->text() );
    todo->setOrganizer( Person( KOPrefs::instance()->fullName(),
                        KOPrefs::instance()->email() ) );
    if ( !mChanger->addIncidence( todo, this ) ) {
      KODialogManager::errorSaveIncidence( this, todo );
      delete todo;
      return;
    }
    mQuickAdd->setText( QString::null );
  }
}

void KOTodoView::setIncidenceChanger( IncidenceChangerBase *changer )
{
  mChanger = changer;
  mTodoListView->setIncidenceChanger( changer );
}
