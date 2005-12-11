/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, 
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>
#include <q3header.h>
#include <qcursor.h>
#include <qlabel.h>
#include <qtimer.h>

#include <QStackedWidget>
//Added by qt3to4:
#include <QBoxLayout>
#include <Q3PopupMenu>
#include <QMouseEvent>
#include <QFrame>
#include <QEvent>
#include <QDragMoveEvent>
#include <Q3CString>
#include <QDragLeaveEvent>
#include <QVBoxLayout>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QSplitter>
#include <QApplication>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kactioncollection.h>
#ifndef KORG_NODND
#include <k3multipledrag.h>
#endif
#include <ktoolbar.h>

#include <libkcal/icaldrag.h>
#include <libkcal/vcaldrag.h>
#include <libkcal/dndfactory.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/calfilter.h>
#include <libkcal/incidenceformatter.h>
#include <libkcal/journal.h>

#include <libkdepim/clicklineedit.h>
#include <libkdepim/kdatepickerpopup.h>

#include <libemailfunctions/email.h>

#include "docprefs.h"

#include "kodialogmanager.h"
#include "kotodoview.h"
#include "koprefs.h"
#include "koglobals.h"
using namespace KOrg;
#include "kotodoviewitem.h"
#include "kotodoviewquicksearch.h"
#include <korganizer/mainwindow.h>
#include <kvbox.h>

#include "kotodoview.moc"


KOTodoListViewToolTip::KOTodoListViewToolTip (QWidget* parent,
                                              KOTodoListView* lv )
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
#warning port QToolTip usage
      // tip(r, tipText);
    }
  }

}



KOTodoListView::KOTodoListView( QWidget *parent )
  : KListView( parent ), mCalendar( 0 ), mChanger( 0 )
{
  mOldCurrent = 0;
  mMousePressed = false;

  setRootIsDecorated( true );
  setAllColumnsShowFocus( true );

  setShowSortIndicator( true );

  addColumn( i18n("Summary") );
  addColumn( i18n("Recurs") );
  addColumn( i18n("Priority") );
  setColumnAlignment( KOTodoView::ePriorityColumn, Qt::AlignHCenter );
  addColumn( i18n("Complete") );
  setColumnAlignment( KOTodoView::ePercentColumn, Qt::AlignRight );
  addColumn( i18n("Due Date/Time") );
  setColumnAlignment( KOTodoView::eDueDateColumn, Qt::AlignLeft );
  addColumn( i18n("Categories") );
#if 0
  addColumn( i18n("Sort Id") );
  setColumnAlignment( 4, Qt::AlignHCenter );
#endif

  setMinimumHeight( 60 );
  setItemsRenameable( true );
  setRenameable( 0 );

  setColumnWidthMode( KOTodoView::eSummaryColumn, Q3ListView::Manual );
  setColumnWidthMode( KOTodoView::eRecurColumn, Q3ListView::Manual );
  setColumnWidthMode( KOTodoView::ePriorityColumn, Q3ListView::Manual );
  setColumnWidthMode( KOTodoView::ePercentColumn, Q3ListView::Manual );
  setColumnWidthMode( KOTodoView::eDueDateColumn, Q3ListView::Manual );
  setColumnWidthMode( KOTodoView::eCategoriesColumn, Q3ListView::Manual );
#if 0
  setColumnWidthMode( KOTodoView::eDescriptionColumn, Q3ListView::Manual );
#endif

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
       !Q3TextDrag::canDecode( e ) ) {
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
       !Q3TextDrag::canDecode( e ) ) {
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
         !Q3TextDrag::canDecode( e ) ) ) {
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
      if ( !mChanger->addIncidence( todo ) ) {
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
    } else if ( Q3TextDrag::decode(e, text) ) {
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
  Q3ListView::contentsMousePressEvent(e);
  QPoint p(contentsToViewport(e->pos()));
  Q3ListViewItem *i = itemAt(p);
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
  Q3ListView::contentsMouseMoveEvent(e);
  if (mMousePressed && (mPressPos - e->pos()).manhattanLength() >
      QApplication::startDragDistance()) {
    mMousePressed = false;
    Q3ListViewItem *item = itemAt(contentsToViewport(mPressPos));
    if ( item && mCalendar ) {
//      kdDebug(5850) << "Start Drag for item " << item->text(0) << endl;
      DndFactory factory( mCalendar );
      Q3DragObject *vd = factory.createDrag(
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
  Q3ListView::contentsMouseReleaseEvent(e);
  mMousePressed = false;
}

void KOTodoListView::contentsMouseDoubleClickEvent(QMouseEvent *e)
{
  if (!e) return;

  QPoint vp = contentsToViewport(e->pos());

  Q3ListViewItem *item = itemAt(vp);

  if (!item) return;

  emit doubleClicked(item,vp,0);
}

/////////////////////////////////////////////////////////////////////////////

KOTodoView::KOTodoView( Calendar *calendar, QWidget *parent)
  : KOrg::BaseView( calendar, parent),
    mWidgetStack( 0 ),
    mSplitter( 0 ),
    mMyTodoListView( 0 ),
    mOneTodoListView( 0 ),
    mYourTodoListView( 0 ),
    mOtherTodoListView( 0 )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  // find the main window (for the action collection)
  KActionCollection *collection = 0;
  for ( QWidget *curWidget = parentWidget(); curWidget; 
        curWidget = curWidget->parentWidget() ) {
    KOrg::MainWindow *mainWidget = dynamic_cast<KOrg::MainWindow *>( curWidget );
    if ( mainWidget )
      collection = mainWidget->getActionCollection();
  }
  
  setupListViews();
  QList<KListView *> list;
  list.append( mMyTodoListView );
  list.append( mOneTodoListView );
  list.append( mYourTodoListView );
  list.append( mOtherTodoListView );
  KOTodoListViewQuickSearchContainer *container =
          new KOTodoListViewQuickSearchContainer( this, list, 
                                                  collection, calendar);
  container->setObjectName("todo quick search");
  mSearchToolBar = container->quickSearch();
  
  if ( !KOPrefs::instance()->mEnableTodoQuickSearch ) container->hide();
  topLayout->addWidget( container );

  QLabel *title = new QLabel( i18n("To-dos:"), this );
  title->setFrameStyle( QFrame::Panel | Q3Frame::Raised );
  topLayout->addWidget( title );

  mQuickAdd = new KPIM::ClickLineEdit( this, i18n( "Click to add a new to-do" ) );
  mQuickAdd->setAcceptDrops( false );
  topLayout->addWidget( mQuickAdd );

  if ( !KOPrefs::instance()->mEnableQuickTodo ) mQuickAdd->hide();

//  topLayout->addWidget( mTodoListView );
  topLayout->addWidget( mWidgetStack );

  mPriorityPopupMenu = new Q3PopupMenu( this );
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

  mPercentageCompletedPopupMenu = new Q3PopupMenu(this);
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


  connect( mMovePopupMenu, SIGNAL( dateChanged( const QDate& )),
           SLOT( setNewDate( const QDate& ) ) );
  connect( mCopyPopupMenu, SIGNAL( dateChanged( const QDate& )),
           SLOT( copyTodoToDate( const QDate& ) ) );

  mItemPopupMenu = new Q3PopupMenu(this);
  mItemPopupMenu->insertItem(i18n("&Show"), this,
                             SLOT (showTodo()));
  mItemPopupMenu->insertItem(i18n("&Edit..."), this,
                             SLOT (editTodo()), 0, ePopupEdit );
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

  mPopupMenu = new Q3PopupMenu(this);
  mPopupMenu->insertItem(KOGlobals::self()->smallIconSet("todo"), i18n("&New To-do..."), this,
                         SLOT (newTodo()));
  mPopupMenu->insertItem(i18n("delete completed to-dos","&Purge Completed"),
                         this, SLOT(purgeCompleted()));

  mDocPrefs = new DocPrefs( objectName() );
  connect( mQuickAdd, SIGNAL( returnPressed () ),
           SLOT( addQuickTodo() ) );
}

KOTodoView::~KOTodoView()
{
  saveListViewState( mMyTodoListView );
  saveListViewState( mOneTodoListView );
  saveListViewState( mYourTodoListView );
  saveListViewState( mOtherTodoListView );
  delete mDocPrefs;
}


void KOTodoView::setupListViews()
{
  mWidgetStack = new QStackedWidget( this );
  mWidgetStack->setSizePolicy( QSizePolicy( QSizePolicy::Expanding,
                               QSizePolicy::Expanding ) );

  /* Set up split list views:
   * Three list views - the first one contains the tasks _I_ need to
   * work on, the second one contains the tasks _I_ want _somebody
   * else_ to work on, and the third one contains everything else. But
   * only do this if the "split listview" configuration option is on.
   */
  mSplitter = new QSplitter( Qt::Vertical, this );
  mWidgetStack->insertWidget( eSplitListViews,mSplitter );

  KVBox* myVBox = new KVBox( mSplitter );
  new QLabel( i18n( "<qt><b>Tasks I have to work on:</b></qt>" ), myVBox );
  mMyTodoListView = new KOTodoListView( myVBox );
  mMyTodoListView->setObjectName( "my todos" );

  KVBox* yourVBox = new KVBox( mSplitter );
  new QLabel( i18n( "<qt><b>Tasks I want others to work on:</b></qt>" ),
              yourVBox );
  mYourTodoListView = new KOTodoListView( yourVBox );
  mYourTodoListView->setObjectName( "your todos" );

  KVBox* otherVBox = new KVBox( mSplitter );
  new QLabel( i18n( "<qt><b>Other tasks I am watching:</b></qt>" ), otherVBox );
  mOtherTodoListView = new KOTodoListView( otherVBox );
  mOtherTodoListView->setObjectName( "other todos" );

  /* Set up the single list view */
  mOneTodoListView = new KOTodoListView( this );
  mOneTodoListView->setObjectName( "all todos" );
  mWidgetStack->insertWidget( eOneListView, mOneTodoListView );

  /* Now show the right widget stack page depending on KOPrefs */
  if( KOPrefs::instance()->mUseSplitListViews )
    mWidgetStack->setCurrentIndex( eSplitListViews );
  else
    mWidgetStack->setCurrentIndex( eOneListView );

  // Double clicking conflicts with opening/closing the subtree
  connect( mMyTodoListView, SIGNAL( doubleClicked( Q3ListViewItem *,
                                                   const QPoint &, int ) ),
           SLOT( editItem( Q3ListViewItem *, const QPoint &, int ) ) );
  connect( mOneTodoListView, SIGNAL( doubleClicked( Q3ListViewItem *,
                                                   const QPoint &, int ) ),
           SLOT( editItem( Q3ListViewItem *, const QPoint &, int ) ) );
  connect( mYourTodoListView, SIGNAL( doubleClicked( Q3ListViewItem *,
                                                     const QPoint &, int ) ),
           SLOT( editItem( Q3ListViewItem *, const QPoint &, int ) ) );
  connect( mOtherTodoListView, SIGNAL( doubleClicked( Q3ListViewItem *,
                                                      const QPoint &, int ) ),
           SLOT( editItem( Q3ListViewItem *, const QPoint &, int ) ) );

  connect( mMyTodoListView, SIGNAL( returnPressed( Q3ListViewItem * ) ),
           SLOT( editItem( Q3ListViewItem * ) ) );
  connect( mOneTodoListView, SIGNAL( returnPressed( Q3ListViewItem * ) ),
           SLOT( editItem( Q3ListViewItem * ) ) );
  connect( mYourTodoListView, SIGNAL( returnPressed( Q3ListViewItem * ) ),
           SLOT( editItem( Q3ListViewItem * ) ) );
  connect( mOtherTodoListView, SIGNAL( returnPressed( Q3ListViewItem * ) ),
           SLOT( editItem( Q3ListViewItem * ) ) );

  connect( mMyTodoListView, SIGNAL( contextMenuRequested( Q3ListViewItem *,
                                                          const QPoint &, int ) ),
           SLOT( popupMenu( Q3ListViewItem *, const QPoint &, int ) ) );
  connect( mOneTodoListView, SIGNAL( contextMenuRequested( Q3ListViewItem *,
                                                          const QPoint &, int ) ),
           SLOT( popupMenu( Q3ListViewItem *, const QPoint &, int ) ) );
  connect( mYourTodoListView, SIGNAL( contextMenuRequested( Q3ListViewItem *,
                                                            const QPoint &, int ) ),
           SLOT( popupMenu( Q3ListViewItem *, const QPoint &, int ) ) );
  connect( mOtherTodoListView, SIGNAL( contextMenuRequested( Q3ListViewItem *,
                                                             const QPoint &, int ) ),
             SLOT( popupMenu( Q3ListViewItem *, const QPoint &, int ) ) );

  connect( mMyTodoListView, SIGNAL( expanded( Q3ListViewItem * ) ),
           SLOT( itemStateChanged( Q3ListViewItem * ) ) );
  connect( mOneTodoListView, SIGNAL( expanded( Q3ListViewItem * ) ),
           SLOT( itemStateChanged( Q3ListViewItem * ) ) );

  connect( mYourTodoListView, SIGNAL( expanded( Q3ListViewItem * ) ),
           SLOT( itemStateChanged( Q3ListViewItem * ) ) );
  connect( mOtherTodoListView, SIGNAL( expanded( Q3ListViewItem * ) ),
           SLOT( itemStateChanged( Q3ListViewItem * ) ) );

  connect( mMyTodoListView, SIGNAL( collapsed( Q3ListViewItem * ) ),
           SLOT( itemStateChanged( Q3ListViewItem * ) ) );
  connect( mOneTodoListView, SIGNAL( collapsed( Q3ListViewItem * ) ),
           SLOT( itemStateChanged( Q3ListViewItem * ) ) );
  connect( mYourTodoListView, SIGNAL( collapsed( Q3ListViewItem * ) ),
           SLOT( itemStateChanged( Q3ListViewItem * ) ) );
  connect( mOtherTodoListView, SIGNAL( collapsed( Q3ListViewItem * ) ),
           SLOT( itemStateChanged( Q3ListViewItem * ) ) );

#if 0
  connect(mMyTodoListView,SIGNAL(selectionChanged(Q3ListViewItem *)),
          SLOT(selectionChanged(Q3ListViewItem *)));
  connect(mOneTodoListView,SIGNAL(selectionChanged(Q3ListViewItem *)),
          SLOT(selectionChanged(Q3ListViewItem *)));
  connect(mYourTodoListView,SIGNAL(selectionChanged(Q3ListViewItem *)),
          SLOT(selectionChanged(Q3ListViewItem *)));
  connect(mOtherTodoListView,SIGNAL(selectionChanged(Q3ListViewItem *)),
          SLOT(selectionChanged(Q3ListViewItem *)));

  connect(mMyTodoListView,SIGNAL(clicked(Q3ListViewItem *)),
          SLOT(selectionChanged(Q3ListViewItem *)));
  connect(mOneTodoListView,SIGNAL(clicked(Q3ListViewItem *)),
          SLOT(selectionChanged(Q3ListViewItem *)));
  connect(mYourTodoListView,SIGNAL(clicked(Q3ListViewItem *)),
          SLOT(selectionChanged(Q3ListViewItem *)));
  connect(mOtherTodoListView,SIGNAL(clicked(Q3ListViewItem *)),
          SLOT(selectionChanged(Q3ListViewItem *)));

  connect(mMyTodoListView,SIGNAL(pressed(Q3ListViewItem *)),
          SLOT(selectionChanged(Q3ListViewItem *)));
  connect(mOneTodoListView,SIGNAL(pressed(Q3ListViewItem *)),
          SLOT(selectionChanged(Q3ListViewItem *)));
  connect(mYourTodoListView,SIGNAL(pressed(Q3ListViewItem *)),
          SLOT(selectionChanged(Q3ListViewItem *)));
  connect(mOtherTodoListView,SIGNAL(pressed(Q3ListViewItem *)),
          SLOT(selectionChanged(Q3ListViewItem *)));
#endif
  connect( mMyTodoListView, SIGNAL(selectionChanged() ),
           SLOT( processSelectionChange() ) );
  connect( mOneTodoListView, SIGNAL(selectionChanged() ),
           SLOT( processSelectionChange() ) );
  connect( mYourTodoListView, SIGNAL(selectionChanged() ),
           SLOT( processSelectionChange() ) );
  connect( mOtherTodoListView, SIGNAL(selectionChanged() ),
           SLOT( processSelectionChange() ) );

}

// PENDING(kalle) Use a different distribution of splitter space


void KOTodoView::setCalendar( Calendar *cal )
{
  BaseView::setCalendar( cal );
  mMyTodoListView->setCalendar( cal );
  mOneTodoListView->setCalendar( cal );
  mYourTodoListView->setCalendar( cal );
  mOtherTodoListView->setCalendar( cal );
  mSearchToolBar->setCalendar( cal );
}

void KOTodoView::updateView()
{
  mItemsToDelete.clear();
  // Cache the list of all email addresses, as it is very expensive to
  // query. It might change, though, (e.g. the user changing
  // addressbook entries), so we cannot just cache it once in the
  // KOTodoView ctor.
  mAllEmailAddrs = KOPrefs::instance()->allEmails();
  saveListViewState( mMyTodoListView );
  mMyTodoListView->clear();
  saveListViewState( mOneTodoListView );
  mOneTodoListView->clear();
  saveListViewState( mYourTodoListView );
  mYourTodoListView->clear();
  saveListViewState( mOtherTodoListView );
  mOtherTodoListView->clear();

  fillViews();

  restoreListViewState( mMyTodoListView );
  restoreListViewState( mOneTodoListView );
  restoreListViewState( mYourTodoListView );
  restoreListViewState( mOtherTodoListView );

  processSelectionChange();  
}


void KOTodoView::fillViews()
{
//  kdDebug(5850) << "KOTodoView::updateView()" << endl;
  Todo::List todoList = calendar()->todos();
  
  // Put for each Event a KOTodoViewItem in the list view. Don't rely on a
  // specific order of events. That means that we have to generate parent items
  // recursively for proper hierarchical display of Todos.
  mTodoMap.clear();
  // PENDING(kalle) Keep mTodoMap separate for the three listviews?
  Todo::List::ConstIterator it;
  for( it = todoList.begin(); it != todoList.end(); ++it ) {
    if ( !mTodoMap.contains( *it ) ) {
      insertTodoItem( *it );
    }
  }

  mSearchToolBar->fillCategories();
}

void KOTodoView::restoreListViewState( Q3ListView *listView )
{
  if ( mDocPrefs ) {
    listView->blockSignals( true );
    for ( Q3ListViewItemIterator it( listView ); it.current(); ++it )
      if ( KOTodoViewItem *todoItem
          = dynamic_cast<KOTodoViewItem *>( it.current() ) )
        todoItem->setOpen( mDocPrefs->readBoolEntry( todoItem->todo()->uid() ) );
    listView->setContentsPos( 0, mDocPrefs
        ->readNumEntry( Q3CString( listView->name() ) + " pos" ) );
    listView->blockSignals( false );
  } else
    kdError( 5850 ) << k_funcinfo << " mDocPrefs doesn't exist" << endl;
}

void KOTodoView::saveListViewState( Q3ListView *listView )
{
  if ( mDocPrefs ) {
    mDocPrefs->writeBoolEntry( Q3CString( listView->name() ) + " pos",
                           listView->contentsY() );

    for ( Q3ListViewItemIterator it( listView ); it.current(); ++it )
      if ( KOTodoViewItem *todoItem
          = dynamic_cast<KOTodoViewItem *>( it.current() ) )
        mDocPrefs->writeNumEntry( todoItem->todo()->uid(), todoItem->isOpen() );
  } else
    kdError( 5850 ) << k_funcinfo << " mDocPrefs doesn't exist" << endl;
}

// PENDING(kalle) Don't use split listview when in sidebar.

QMap<Todo *,KOTodoViewItem *>::ConstIterator
  KOTodoView::insertTodoItem( Todo *todo)
{
//  kdDebug(5850) << "KOTodoView::insertTodoItem(): " << todo->getSummary() << endl;
  Incidence *incidence = todo->relatedTo();
  if (incidence && incidence->type() == "Todo") {
    // Use dynamic_cast, because in the future the related item might also be an event
    Todo *relatedTodo = dynamic_cast<Todo *>(incidence);

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

    // This is where all the logic is about which item goes into which
    // list view. All this stuff should really done via CalFilter, but
    // as long as the filters work on the models instead of the view,
    // that is not possible. Something to be done for KDE 3.6 or
    // 4.0. For now, this is copied from libkcal/calfilter.cpp. But we
    // only do it anyway if this was configured to be done.
    KOTodoViewItem *todoItem = 0;
    if( KOPrefs::instance()->mUseSplitListViews  ) {
      bool iAmOneOfTheAttendees = false;
      const Attendee::List &attendees = todo->attendees();
      if ( !todo->attendees().isEmpty() ) {
        for( Attendee::List::ConstIterator it = attendees.begin();
             it != attendees.end(); ++it ) {
          if ( mAllEmailAddrs.find( (*it)->email() ) != mAllEmailAddrs.end() ) {
            iAmOneOfTheAttendees = true;
            break;
          }
        }
      } else {
        // no attendees, must be me only
        iAmOneOfTheAttendees = true;
      }

      // If I am one of the attendes, create the item and be happy about
      // it, no need to go further.
      if( iAmOneOfTheAttendees )
        todoItem = new KOTodoViewItem( mMyTodoListView, todo, this );
      else {
        bool iAmTheOrganizer =
          KOPrefs::instance()->thatIsMe( todo->organizer().email() );

        if( iAmTheOrganizer /* and not one of the attendees */ )
          todoItem = new KOTodoViewItem( mYourTodoListView, todo, this );
        else
          /* I am neither one of the attendees nor the organizer */
          todoItem = new KOTodoViewItem( mOtherTodoListView, todo, this );
      }
    } else {
      // no split list views, just put into the single list view
      todoItem = new KOTodoViewItem( mOneTodoListView, todo, this );
    }
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
  /* Now show the right widget stack page depending on KOPrefs */
  if( KOPrefs::instance()->mUseSplitListViews )
    mWidgetStack->setCurrentIndex( eSplitListViews );
  else
    mWidgetStack->setCurrentIndex( eOneListView );

  mMyTodoListView->repaintContents();
  mOneTodoListView->repaintContents();
  mYourTodoListView->repaintContents();
  mOtherTodoListView->repaintContents();
}

Incidence::List KOTodoView::selectedIncidences()
{
  // PENDING(kalle) This needs review. Where is this invoked; does it
  // always apply to one particular listview?

  Incidence::List selected;

  KOTodoViewItem *item = (KOTodoViewItem *)(mMyTodoListView->selectedItem());
  if (item) selected.append(item->todo());
  item = (KOTodoViewItem *)(mOneTodoListView->selectedItem());
  if (item) selected.append(item->todo());
  item = (KOTodoViewItem*)(mYourTodoListView->selectedItem());
  if (item) selected.append(item->todo());
  item = (KOTodoViewItem*)(mOtherTodoListView->selectedItem());
  if (item) selected.append(item->todo());

  return selected;
}

Todo::List KOTodoView::selectedTodos()
{
  // PENDING(kalle) This needs review. Where is this invoked; does it
  // always apply to one particular listview?
  Todo::List selected;

  KOTodoViewItem *item = (KOTodoViewItem *)(mMyTodoListView->selectedItem());
  if (item) selected.append(item->todo());
  item = (KOTodoViewItem *)(mOneTodoListView->selectedItem());
  if (item) selected.append(item->todo());
  item = (KOTodoViewItem*)(mYourTodoListView->selectedItem());
  if (item) selected.append(item->todo());
  item = (KOTodoViewItem*)(mOtherTodoListView->selectedItem());
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
                // PENDING(kalle) This is definitely wrong. We need to
                // determine where this go here. Can an item ever
                // change listviews? I guess it can, if assignments
                // are changed.
                mMyTodoListView->insertItem( todoItem );
              }
            }
            todoItem->construct();
          }
        } else {
          if ( !isFiltered ) {
            insertTodoItem( todo );
          }
        }
        mMyTodoListView->sort();
        mOneTodoListView->sort();
        mYourTodoListView->sort();
        mOtherTodoListView->sort();
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

void KOTodoView::showIncidences( const Incidence::List &incidences )
{
// FIXME after merging Kalle's branch with qsearch
#if 0 
  // we must check if they are not filtered; if they are, remove the filter
  CalFilter *filter = calendar()->filter();
  bool wehaveall = true;
  if ( filter )
    for ( Incidence::List::ConstIterator it = incidences.constBegin();
        it != incidences.constEnd(); ++it )
      if ( !( wehaveall = filter->filterIncidence( *it ) ) )
        break;
  
  if ( !wehaveall )
    calendar()->setFilter( 0 );
  
  // calculate the rectangle we must have
  uint begin = mTodoListView->contentsHeight(), end = 0;
  KOTodoViewItem *first = 0, *last;
  for ( Q3ListViewItemIterator it( mTodoListView ); it.current(); ++it )
    if ( incidences.contains( static_cast<KOTodoViewItem *>( it.current() 
                                                           )->todo() ) ) {
      if ( !first ) first = static_cast<KOTodoViewItem *>( it.current() );
      last = static_cast<KOTodoViewItem *>( it.current() );
      uint pos = it.current()->itemPos();
      begin = qMin( begin, pos );
      end = qMax( end, pos + it.current()->height() );
    }
  
  if ( end < begin )
    // nothing to do
    return;
  if ( end - begin > (uint) mTodoListView->visibleHeight() )
    // we can't show them all anyway, mise show the first on top
    mTodoListView->setContentsPos( 0, first->itemPos() );
  else  // center it
    mTodoListView->center( 0, (begin + end) / 2 );
  
  // the final touch (make the user notice)
  first->setSelected( true );
#endif
}

CalPrinter::PrintType KOTodoView::printType()
{
  return CalPrinter::Todolist;
}

void KOTodoView::editItem( Q3ListViewItem *item )
{
  if (item)
    emit editIncidenceSignal( static_cast<KOTodoViewItem *>( item )->todo() );
}

void KOTodoView::editItem( Q3ListViewItem *item, const QPoint &, int )
{
  editItem( item );
}

void KOTodoView::showItem( Q3ListViewItem *item )
{
  if (item)
    emit showIncidenceSignal( static_cast<KOTodoViewItem *>( item )->todo() );
}

void KOTodoView::showItem( Q3ListViewItem *item, const QPoint &, int )
{
  showItem( item );
}

void KOTodoView::popupMenu( Q3ListViewItem *item, const QPoint &, int column )
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
      if ( !todo->isCompleted() ) item->setState( Q3CheckListItem::Off );
      else todo->setPercentComplete( percentage );
    } else {
      todo->setCompleted( false );
      todo->setPercentComplete( percentage );
    }
    item->construct();
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

void KOTodoView::setNewDate( const QDate &date )
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

void KOTodoView::copyTodoToDate( const QDate &date )
{
  QDateTime dt;
  dt.setDate( date );

  if ( mActiveItem && mChanger ) {
    Todo *newTodo = mActiveItem->todo()->clone();
    newTodo->recreate();

   if ( date.isNull() )
     newTodo->setHasDueDate( false );
   newTodo->setDtDue( dt );
   newTodo->setPercentComplete( 0 );

   // avoid forking
   if ( newTodo->doesRecur() )
     newTodo->recurrence()->unsetRecurs();

   mChanger->addIncidence( newTodo );
 }
}

Q3PopupMenu *KOTodoView::getCategoryPopupMenu( KOTodoViewItem *todoItem )
{
  Q3PopupMenu *tempMenu = new Q3PopupMenu( this );
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

void KOTodoView::itemStateChanged( Q3ListViewItem *item )
{
  if (!item) return;

  KOTodoViewItem *todoItem = (KOTodoViewItem *)item;

//  kdDebug(5850) << "KOTodoView::itemStateChanged(): " << todoItem->todo()->summary() << endl;
}

void KOTodoView::setNewPercentageDelayed( KOTodoViewItem *item, int percentage )
{
  mPercentChangedMap.append( qMakePair( item, percentage ) );

  QTimer::singleShot( 0, this, SLOT( processDelayedNewPercentage() ) );
}

void KOTodoView::processDelayedNewPercentage()
{
  QList< QPair< KOTodoViewItem *, int> >::Iterator it;
  for ( it = mPercentChangedMap.begin(); it != mPercentChangedMap.end(); ++it )
    setNewPercentage( (*it).first, (*it).second );

  mPercentChangedMap.clear();
}

void KOTodoView::saveLayout(KConfig *config, const QString &group) const
{
  mMyTodoListView->saveLayout(config,group);
  mOneTodoListView->saveLayout(config,group);
  mYourTodoListView->saveLayout(config,group);
  mOtherTodoListView->saveLayout(config,group);
}

void KOTodoView::restoreLayout(KConfig *config, const QString &group)
{
  mMyTodoListView->restoreLayout(config,group);
  mOneTodoListView->restoreLayout(config,group);
  mYourTodoListView->restoreLayout(config,group);
  mOtherTodoListView->restoreLayout(config,group);
}

void KOTodoView::processSelectionChange()
{
//  kdDebug(5850) << "KOTodoView::processSelectionChange()" << endl;

  // PENDING(kalle) This is definitely wrong. We need to think about
  // whose selection is changing here.
  KOTodoViewItem *item =
    static_cast<KOTodoViewItem *>( mOneTodoListView->selectedItem() );

  if ( !item ) {
    emit incidenceSelected( 0 );
  } else {
    emit incidenceSelected( item->todo() );
  }
}

void KOTodoView::clearSelection()
{
  mMyTodoListView->selectAll( false );
  if( mYourTodoListView )
    mYourTodoListView->selectAll( false );
  if( mOtherTodoListView )
    mOtherTodoListView->selectAll( false );
}

void KOTodoView::purgeCompleted()
{
  emit purgeCompletedSignal();
}

void KOTodoView::addQuickTodo()
{
  if ( ! mQuickAdd->text().trimmed().isEmpty() ) {
    Todo *todo = new Todo();
    todo->setSummary( mQuickAdd->text() );
    todo->setOrganizer( Person( KOPrefs::instance()->fullName(),
                        KOPrefs::instance()->email() ) );
    if ( !mChanger->addIncidence( todo ) ) {
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
  mMyTodoListView->setIncidenceChanger( changer );
  mOneTodoListView->setIncidenceChanger( changer );
  mYourTodoListView->setIncidenceChanger( changer );
  mOtherTodoListView->setIncidenceChanger( changer );
}

void KOTodoView::updateCategories()
{
  mSearchToolBar->fillCategories();
}
