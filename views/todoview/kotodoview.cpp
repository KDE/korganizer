/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kotodoview.h"
#include "calprinter.h"
#include "kocorehelper.h"
#include "koglobals.h"
#include "kohelper.h"
#include "koprefs.h"
#include "kotododelegates.h"
#include "kotodomodel.h"
#include "kotodoviewsortfilterproxymodel.h"
#include "kotodoviewquickaddline.h"
#include "kotodoviewquicksearch.h"
#include "kotodoviewview.h"

#include <akonadi/calendar/etmcalendar.h>
#include <calendarsupport/categoryconfig.h>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <libkdepim/kdatepickerpopup.h>

#include <KCalCore/CalFormat>

#include <QCheckBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QMenu>
#include <QTimer>

// Share the model with the sidepanel KOTodoView
K_GLOBAL_STATIC( KOTodoModel, sModel )

KOTodoView::KOTodoView( bool sidebarView, QWidget *parent )
  : BaseView( parent )
{
  connect( sModel, SIGNAL(expandIndex(QModelIndex)),
           this, SLOT(expandIndex(QModelIndex)) );
  mProxyModel = new KOTodoViewSortFilterProxyModel( this );
  mProxyModel->setSourceModel( sModel );
  mProxyModel->setDynamicSortFilter( true );
  mProxyModel->setFilterKeyColumn( KOTodoModel::SummaryColumn );
  mProxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyModel->setSortRole( Qt::EditRole );

  // This disconnect is a workaround against QTBUG-22667
  disconnect( sModel, SIGNAL(destroyed()), mProxyModel, 0 );

  mSidebarView = sidebarView;
  if ( !mSidebarView ) {
    mQuickSearch = new KOTodoViewQuickSearch( calendar(), this );
    mQuickSearch->setVisible( KOPrefs::instance()->enableTodoQuickSearch() );
    connect( mQuickSearch, SIGNAL(searchTextChanged(QString)),
             mProxyModel, SLOT(setFilterRegExp(QString)) );
    connect( mQuickSearch, SIGNAL(searchTextChanged(QString)),
             this, SLOT(expandTree()) );
    connect( mQuickSearch, SIGNAL(filterCategoryChanged(QStringList)),
             mProxyModel, SLOT(setCategoryFilter(QStringList)) );
    connect( mQuickSearch, SIGNAL(filterCategoryChanged(QStringList)),
             this, SLOT(expandTree()) );
    connect( mQuickSearch, SIGNAL(filterPriorityChanged(QStringList)),
             mProxyModel, SLOT(setPriorityFilter(QStringList)) );
    connect( mQuickSearch, SIGNAL(filterPriorityChanged(QStringList)),
             this, SLOT(expandTree()) );
  }

  mView = new KOTodoViewView( this );
  mView->setModel( mProxyModel );

  mView->setContextMenuPolicy( Qt::CustomContextMenu );

  mView->setSortingEnabled( true );

  mView->setAutoExpandDelay( 250 );
  mView->setDragDropMode( QAbstractItemView::DragDrop );

  mView->setExpandsOnDoubleClick( false );
  mView->setEditTriggers( QAbstractItemView::SelectedClicked |
                          QAbstractItemView::EditKeyPressed );

  KOTodoRichTextDelegate *richTextDelegate = new KOTodoRichTextDelegate( mView );
  mView->setItemDelegateForColumn( KOTodoModel::SummaryColumn, richTextDelegate );
  mView->setItemDelegateForColumn( KOTodoModel::DescriptionColumn, richTextDelegate );

  KOTodoPriorityDelegate *priorityDelegate = new KOTodoPriorityDelegate( mView );
  mView->setItemDelegateForColumn( KOTodoModel::PriorityColumn, priorityDelegate );

  KOTodoDueDateDelegate *dueDateDelegate = new KOTodoDueDateDelegate( mView );
  mView->setItemDelegateForColumn( KOTodoModel::DueDateColumn, dueDateDelegate );

  KOTodoCompleteDelegate *completeDelegate = new KOTodoCompleteDelegate( mView );
  mView->setItemDelegateForColumn( KOTodoModel::PercentColumn, completeDelegate );

  mCategoriesDelegate = new KOTodoCategoriesDelegate( mView );
  mView->setItemDelegateForColumn( KOTodoModel::CategoriesColumn, mCategoriesDelegate );

  connect( mView, SIGNAL(customContextMenuRequested(QPoint)),
           this, SLOT(contextMenu(QPoint)) );
  connect( mView, SIGNAL(doubleClicked(QModelIndex)),
           this, SLOT(itemDoubleClicked(QModelIndex)) );
  connect( mView->selectionModel(),
           SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
           this,
           SLOT(selectionChanged(QItemSelection,QItemSelection)) );

  mQuickAdd = new KOTodoViewQuickAddLine( this );
  mQuickAdd->setClearButtonShown( true );
  mQuickAdd->setVisible( KOPrefs::instance()->enableQuickTodo() );
  connect( mQuickAdd, SIGNAL(returnPressed(Qt::KeyboardModifiers)),
           this, SLOT(addQuickTodo(Qt::KeyboardModifiers)) );

  mFullView = 0;
  if ( !mSidebarView ) {
    mFullView = new QCheckBox( i18nc( "Checkbox to display this view into the full window",
                                      "Full Window" ), this );
    mFullView->setToolTip(
      i18nc( "@info:tooltip",
             "Display to-do list in a full window" ) );
    mFullView->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Checking this option will cause the to-do view to use the full window." ) );
    connect( mFullView, SIGNAL(toggled(bool)),
             sModel, SLOT(setFullView(bool)) );
    connect( sModel,SIGNAL(fullViewChanged(bool)),
             SLOT(setFullView(bool)) );
  }

  mFlatView = new QCheckBox( i18nc( "Checkbox to display todos not hierarchical",
                                    "Flat View" ), this );
  mFlatView->setToolTip(
    i18nc( "@info:tooltip",
           "Display to-dos in flat list instead of a tree" ) );
  mFlatView->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Checking this option will cause the to-dos to be displayed as a "
           "flat list instead of a hierarchical tree; the parental "
           "relationships are removed in the display." ) );
  connect( mFlatView, SIGNAL(toggled(bool)),
           sModel, SLOT(setFlatView(bool)) );
  connect( sModel,SIGNAL(flatViewChanged(bool)),
           SLOT(setFlatView(bool)) );

  QGridLayout *layout = new QGridLayout( this );
  layout->setMargin( 0 );
  if ( !mSidebarView ) {
    layout->addWidget( mQuickSearch, 0, 0, 1, 2 );
  }
  layout->addWidget( mView, 1, 0, 1, 2 );
  layout->setRowStretch( 1, 1 );
  layout->addWidget( mQuickAdd, 2, 0 );

  // Dummy layout just to add a few px of right margin so the checkbox is aligned
  // with the QAbstractItemView's viewport.
  QHBoxLayout *dummyLayout = new QHBoxLayout();
  dummyLayout->setContentsMargins( 0, 0, mView->frameWidth()/*right*/, 0 );
  if ( !mSidebarView ) {
    dummyLayout->addWidget( mFullView );
  }
  dummyLayout->addWidget( mFlatView );

  layout->addLayout( dummyLayout, 2, 1 );

  setLayout( layout );

  // ---------------- POPUP-MENUS -----------------------
  mItemPopupMenu = new QMenu( this );

  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    i18n( "&Show" ), this, SLOT(showTodo()) );

  QAction *a = mItemPopupMenu->addAction(
    i18n( "&Edit..." ), this, SLOT(editTodo()) );
  mItemPopupMenuReadWriteEntries << a;
  mItemPopupMenuItemOnlyEntries << a;

  mItemPopupMenu->addSeparator();
  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    KOGlobals::self()->smallIcon( "document-print" ),
    i18n( "&Print..." ), this, SLOT(printTodo()) );

  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    KOGlobals::self()->smallIcon( "document-print-preview" ),
    i18n( "Print Previe&w..." ), this, SLOT(printPreviewTodo()) );

  mItemPopupMenu->addSeparator();
  a = mItemPopupMenu->addAction(
    KIconLoader::global()->loadIcon( "edit-delete", KIconLoader::NoGroup, KIconLoader::SizeSmall ),
    i18n( "&Delete" ), this, SLOT(deleteTodo()) );
  mItemPopupMenuReadWriteEntries << a;
  mItemPopupMenuItemOnlyEntries << a;

  mItemPopupMenu->addSeparator();

  mItemPopupMenu->addAction(
    KIconLoader::global()->loadIcon(
      "view-calendar-tasks", KIconLoader::NoGroup, KIconLoader::SizeSmall ),
    i18n( "New &To-do..." ), this, SLOT(newTodo()) );

  a = mItemPopupMenu->addAction(
    i18n( "New Su&b-to-do..." ), this, SLOT(newSubTodo()) );
  mItemPopupMenuReadWriteEntries << a;
  mItemPopupMenuItemOnlyEntries << a;

  mMakeTodoIndependent = mItemPopupMenu->addAction( i18n( "&Make this To-do Independent" ),
                                                    this, SIGNAL(unSubTodoSignal()) );

  mMakeSubtodosIndependent = mItemPopupMenu->addAction( i18n( "Make all Sub-to-dos &Independent" ),
                                                        this, SIGNAL(unAllSubTodoSignal()) );

  mItemPopupMenuItemOnlyEntries << mMakeTodoIndependent;
  mItemPopupMenuItemOnlyEntries << mMakeSubtodosIndependent;

  mItemPopupMenuReadWriteEntries << mMakeTodoIndependent;
  mItemPopupMenuReadWriteEntries << mMakeSubtodosIndependent;

  mItemPopupMenu->addSeparator();

  mCopyPopupMenu =
    new KPIM::KDatePickerPopup( KPIM::KDatePickerPopup::NoDate |
                                KPIM::KDatePickerPopup::DatePicker |
                                KPIM::KDatePickerPopup::Words,
                                QDate::currentDate(), this );
  mCopyPopupMenu->setTitle( i18n( "&Copy To" ) );

  connect( mCopyPopupMenu, SIGNAL(dateChanged(QDate)),
           SLOT(copyTodoToDate(QDate)) );

  connect( mCopyPopupMenu, SIGNAL(dateChanged(QDate)),
           mItemPopupMenu, SLOT(hide()) );

  mMovePopupMenu =
    new KPIM:: KDatePickerPopup( KPIM::KDatePickerPopup::NoDate |
                                 KPIM::KDatePickerPopup::DatePicker |
                                 KPIM::KDatePickerPopup::Words,
                                 QDate::currentDate(), this );
  mMovePopupMenu->setTitle( i18n( "&Move To" ) );

  connect( mMovePopupMenu, SIGNAL(dateChanged(QDate)),
           SLOT(setNewDate(QDate)) );

  connect( mMovePopupMenu, SIGNAL(dateChanged(QDate)),
           mItemPopupMenu, SLOT(hide()) );

  mItemPopupMenu->insertMenu( 0, mCopyPopupMenu );
  mItemPopupMenu->insertMenu( 0, mMovePopupMenu );

  mItemPopupMenu->addSeparator();
  mItemPopupMenu->addAction( i18nc( "delete completed to-dos", "Pur&ge Completed" ),
                             this, SIGNAL(purgeCompletedSignal()) );

  mPriorityPopupMenu = new QMenu( this );
  mPriority[ mPriorityPopupMenu->addAction( i18nc( "unspecified priority", "unspecified" ) ) ] = 0;
  mPriority[ mPriorityPopupMenu->addAction( i18nc( "highest priority", "1 (highest)" ) ) ] = 1;
  mPriority[ mPriorityPopupMenu->addAction( i18n( "2" ) ) ] = 2;
  mPriority[ mPriorityPopupMenu->addAction( i18n( "3" ) ) ] = 3;
  mPriority[ mPriorityPopupMenu->addAction( i18n( "4" ) ) ] = 4;
  mPriority[ mPriorityPopupMenu->addAction( i18nc( "medium priority", "5 (medium)" ) ) ] = 5;
  mPriority[ mPriorityPopupMenu->addAction( i18n( "6" ) ) ] = 6;
  mPriority[ mPriorityPopupMenu->addAction( i18n( "7" ) ) ] = 7;
  mPriority[ mPriorityPopupMenu->addAction( i18n( "8" ) ) ] = 8;
  mPriority[ mPriorityPopupMenu->addAction( i18nc( "lowest priority", "9 (lowest)" ) ) ] = 9;
  connect( mPriorityPopupMenu, SIGNAL(triggered(QAction*)),
           SLOT(setNewPriority(QAction*)) );

  mPercentageCompletedPopupMenu = new QMenu(this);
  for ( int i = 0; i <= 100; i+=10 ) {
    QString label = QString( "%1 %" ).arg( i );
    mPercentage[mPercentageCompletedPopupMenu->addAction( label )] = i;
  }
  connect( mPercentageCompletedPopupMenu, SIGNAL(triggered(QAction*)),
           SLOT(setNewPercentage(QAction*)) );

  setMinimumHeight( 50 );
}

KOTodoView::~KOTodoView()
{
}

void KOTodoView::expandIndex( const QModelIndex &index )
{
  QModelIndex realIndex = mProxyModel->mapFromSource( index );
  while ( realIndex.isValid() ) {
    mView->expand( realIndex );
    realIndex = mProxyModel->parent( realIndex );
  }
}

void KOTodoView::expandTree()
{
  mView->expandAll();
}

void KOTodoView::setCalendar( const Akonadi::ETMCalendar::Ptr &cal )
{
  BaseView::setCalendar( cal );
  if ( !mSidebarView ) {
    mQuickSearch->setCalendar( cal );
  }
  mCategoriesDelegate->setCalendar( cal );
  sModel->setCalendar( cal );
}

Akonadi::Item::List KOTodoView::selectedIncidences()
{
  Akonadi::Item::List ret;
  const QModelIndexList selection = mView->selectionModel()->selectedRows();
  Q_FOREACH ( const QModelIndex &mi, selection ) {
    ret << mi.data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  }
  return ret;
}

DateList KOTodoView::selectedIncidenceDates()
{
  // The todo view only lists todo's. It's probably not a good idea to
  // return something about the selected todo here, because it has got
  // a couple of dates (creation, due date, completion date), and the
  // caller could not figure out what he gets. So just return an empty list.
  return DateList();
}

void KOTodoView::saveLayout( KConfig *config, const QString &group ) const
{
  KConfigGroup cfgGroup = config->group( group );
  QHeaderView *header = mView->header();

  QVariantList columnVisibility;
  QVariantList columnOrder;
  QVariantList columnWidths;
  for ( int i = 0; i < header->count(); i++ ) {
    columnVisibility << QVariant( !mView->isColumnHidden( i ) );
    columnWidths << QVariant( header->sectionSize( i ) );
    columnOrder << QVariant( header->visualIndex( i ) );
  }
  cfgGroup.writeEntry( "ColumnVisibility", columnVisibility );
  cfgGroup.writeEntry( "ColumnOrder", columnOrder );
  cfgGroup.writeEntry( "ColumnWidths", columnWidths );

  cfgGroup.writeEntry( "SortAscending", (int)header->sortIndicatorOrder() );
  if ( header->isSortIndicatorShown() ) {
    cfgGroup.writeEntry( "SortColumn", header->sortIndicatorSection() );
  } else {
    cfgGroup.writeEntry( "SortColumn", -1 );
  }

  if ( !mSidebarView ) {
    KOPrefs::instance()->setFullViewTodo( mFullView->isChecked() );
  }
  KOPrefs::instance()->setFlatListTodo( mFlatView->isChecked() );
}

void KOTodoView::restoreLayout( KConfig *config, const QString &group, bool minimalDefaults )
{
  KConfigGroup cfgGroup = config->group( group );
  QHeaderView *header = mView->header();

  QVariantList columnVisibility = cfgGroup.readEntry( "ColumnVisibility", QVariantList() );
  QVariantList columnOrder = cfgGroup.readEntry( "ColumnOrder", QVariantList() );
  QVariantList columnWidths = cfgGroup.readEntry( "ColumnWidths", QVariantList() );

  if ( columnVisibility.isEmpty() ) {
    // if config is empty then use default settings
    mView->hideColumn( eRecurColumn );

    if ( minimalDefaults ) {
      mView->hideColumn( ePriorityColumn );
      mView->hideColumn( ePercentColumn );
      mView->hideColumn( eDescriptionColumn );
    }

    // We don't have any incidences (content) yet, so we delay resizing
    QTimer::singleShot( 0, this, SLOT(resizeColumnsToContent()) );

  } else {
      for ( int i = 0;
            i < header->count() &&
            i < columnOrder.size() &&
            i < columnWidths.size() &&
            i < columnVisibility.size();
            i++ ) {
      bool visible = columnVisibility[i].toBool();
      int width = columnWidths[i].toInt();
      int order = columnOrder[i].toInt();

      header->resizeSection( i, width );
      header->moveSection( header->visualIndex( i ), order );
      if ( i != 0 && !visible ) {
        mView->hideColumn( i );
      }
    }
  }

  int sortOrder = cfgGroup.readEntry( "SortAscending", (int)Qt::AscendingOrder );
  int sortColumn = cfgGroup.readEntry( "SortColumn", -1 );
  if ( sortColumn >= 0 ) {
    mView->sortByColumn( sortColumn, (Qt::SortOrder)sortOrder );
  }

  mFlatView->setChecked( cfgGroup.readEntry( "FlatView", false ) );
  mView->expandAll();
}

void KOTodoView::setIncidenceChanger( Akonadi::IncidenceChanger *changer )
{
  BaseView::setIncidenceChanger( changer );
  sModel->setIncidenceChanger( changer );
}

void KOTodoView::showDates( const QDate &start, const QDate &end, const QDate & )
{
  // There is nothing to do here for the Todo View
  Q_UNUSED( start );
  Q_UNUSED( end );
}

void KOTodoView::showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date )
{
  Q_UNUSED( incidenceList );
  Q_UNUSED( date );
}

void KOTodoView::updateView()
{
  sModel->reloadTodos();
  mView->expandAll();
}

void KOTodoView::updateCategories()
{
  if ( !mSidebarView ) {
    mQuickSearch->updateCategories();
  }
  // TODO check if we have to do something with the category delegate
}

void KOTodoView::changeIncidenceDisplay( const Akonadi::Item &incidence,
                                         Akonadi::IncidenceChanger::ChangeType changeType )
{
  sModel->processChange( incidence, changeType );
}

void KOTodoView::updateConfig()
{
  if ( !mSidebarView ) {
    mQuickSearch->setVisible( KOPrefs::instance()->enableTodoQuickSearch() );
  }
  mQuickAdd->setVisible( KOPrefs::instance()->enableQuickTodo() );

  updateView();
}

void KOTodoView::clearSelection()
{
  mView->selectionModel()->clearSelection();
}

void KOTodoView::addTodo( const QString &summary,
                          const KCalCore::Todo::Ptr &parent,
                          const QStringList &categories )
{
  if ( !mChanger || summary.trimmed().isEmpty() ) {
    return;
  }

  KCalCore::Todo::Ptr todo( new KCalCore::Todo );
  todo->setSummary( summary.trimmed() );
  todo->setOrganizer(
    Person::Ptr( new Person( CalendarSupport::KCalPrefs::instance()->fullName(),
                             CalendarSupport::KCalPrefs::instance()->email() ) ) );

  todo->setCategories( categories );

  Q_UNUSED( parent );
  /*  if ( parent ) {
    todo->setRelatedTo( parent );
  }
  TODO: review
  */

  bool result = false;
  CalendarSupport::CollectionSelection *selection =
    EventViews::EventView::globalCollectionSelection();

  Akonadi::Collection collection;
  // If we only have one collection, don't ask in which collection to save the to-do.
  if ( selection && selection->model()->model()->rowCount() == 1 ) {
    QModelIndex index = selection->model()->model()->index( 0, 0 );
    if ( index.isValid() ) {
      collection = CalendarSupport::collectionFromIndex( index );
    }
  }

  result = -1 != mChanger->createIncidence( todo, collection, this );

  if ( !result ) {
    KOHelper::showSaveIncidenceErrorMsg( this, todo );
  }
}

void KOTodoView::addQuickTodo( Qt::KeyboardModifiers modifiers )
{
  if ( modifiers == Qt::NoModifier ) {
    /*const QModelIndex index = */ addTodo( mQuickAdd->text(), KCalCore::Todo::Ptr(),
                                            mProxyModel->categories() );

#ifdef AKONADI_PORT_DISABLED
    // the todo is added asynchronously now, so we have to wait until
    // the new item is actually added before selecting the item

    QModelIndexList selection = mView->selectionModel()->selectedRows();
    if ( selection.size() <= 1 ) {
      // don't destroy complex selections, not applicable now (only single
      // selection allowed), but for the future...
      mView->selectionModel()->select( mProxyModel->mapFromSource( index ),
                                       QItemSelectionModel::ClearAndSelect |
                                       QItemSelectionModel::Rows );
    }
#else
    kDebug() << "AKONADI PORT: Disabled code in  " << Q_FUNC_INFO;
#endif
  } else if ( modifiers == Qt::ControlModifier ) {
    QModelIndexList selection = mView->selectionModel()->selectedRows();
    if ( selection.size() != 1 ) {
      return;
    }
    const QModelIndex idx = mProxyModel->mapToSource( selection[0] );
    const Akonadi::Item parent = sModel->todoForIndex( idx );
    addTodo( mQuickAdd->text(), CalendarSupport::todo( parent ), mProxyModel->categories() );
  } else {
    return;
  }
  mQuickAdd->setText( QString() );
}

void KOTodoView::contextMenu( const QPoint &pos )
{
  const bool hasItem = mView->indexAt( pos ).isValid();
  Incidence::Ptr incidencePtr;

  Q_FOREACH ( QAction *entry, mItemPopupMenuItemOnlyEntries ) {
    bool enable;

    if ( hasItem ) {
      const Akonadi::Item::List incidences = selectedIncidences();

      if ( incidences.isEmpty() ) {
        enable = false;
      } else {
        Akonadi::Item item = incidences.first();
        incidencePtr = CalendarSupport::incidence( item );

        // Action isn't RO, it can change the incidence, "Edit" for example.
        const bool actionIsRw = mItemPopupMenuReadWriteEntries.contains( entry );

        const bool incidenceIsRO = !calendar()->hasRight( item, Akonadi::Collection::CanChangeItem );

        enable = hasItem && ( !actionIsRw ||
                              ( actionIsRw && !incidenceIsRO ) );

      }
    } else {
      enable = false;
    }

    entry->setEnabled( enable );
  }
  mCopyPopupMenu->setEnabled( hasItem );
  mMovePopupMenu->setEnabled( hasItem );

  if ( hasItem ) {

    if ( !incidencePtr ) {

      if ( calendar() ) {
        mMakeSubtodosIndependent->setEnabled( !calendar()->childItems( incidencePtr->uid() ).isEmpty() );
        mMakeTodoIndependent->setEnabled( !incidencePtr->relatedTo().isEmpty() );
      }
    }

    switch ( mView->indexAt( pos ).column() ) {
    case ePriorityColumn:
      mPriorityPopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
      break;
    case ePercentColumn:
      mPercentageCompletedPopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
      break;
    case eDueDateColumn:
      mMovePopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
      break;
    case eCategoriesColumn:
      createCategoryPopupMenu()->popup( mView->viewport()->mapToGlobal( pos ) );
      break;
    default:
      mItemPopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
      break;
    }
  } else {
    mItemPopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
  }
}

void KOTodoView::selectionChanged( const QItemSelection &selected,
                                   const QItemSelection &deselected )
{
  Q_UNUSED( deselected );
  QModelIndexList selection = selected.indexes();
  if ( selection.isEmpty() || !selection[0].isValid() ) {
    emit incidenceSelected( Akonadi::Item(), QDate() );
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();

  if ( selectedIncidenceDates().isEmpty() ) {
    emit incidenceSelected( todoItem, QDate() );
  } else {
    emit incidenceSelected( todoItem, selectedIncidenceDates().first() );
  }
}

void KOTodoView::showTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();

  emit showIncidenceSignal( todoItem );
}

void KOTodoView::editTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  emit editIncidenceSignal( todoItem );
}

void KOTodoView::printTodo()
{
  printTodo( false );
}

void KOTodoView::printPreviewTodo()
{
  printTodo( true );
}

void KOTodoView::printTodo( bool preview )
{
   QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );

  KOCoreHelper helper;
  CalPrinter printer( this, calendar().data(), &helper, true );
  connect( this, SIGNAL(configChanged()), &printer, SLOT(updateConfig()) );

  Incidence::List selectedIncidences;
  selectedIncidences.append( todo );

  KDateTime todoDate;
  if ( todo->hasStartDate() ) {
    todoDate = todo->dtStart();
  } else {
    todoDate = todo->dtDue();
  }

  printer.print( KOrg::CalPrinterBase::Incidence,
                 todoDate.date(), todoDate.date(), selectedIncidences, preview );

}

void KOTodoView::deleteTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() == 1 ) {
    const Akonadi::Item todoItem =
      selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();

    if ( mChanger->deletedRecently( todoItem.id() ) ) {
      emit deleteIncidenceSignal( todoItem );
    }
  }
}

void KOTodoView::newTodo()
{
  emit newTodoSignal( QDate::currentDate().addDays( 7 ) );
}

void KOTodoView::newSubTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() == 1 ) {
    const Akonadi::Item todoItem =
      selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();

    emit newSubTodoSignal( todoItem );
  } else {
    // This never happens
    kWarning() << "Selection size isn't 1";
  }
}

void KOTodoView::copyTodoToDate( const QDate &date )
{
  if ( !mChanger ) {
    return;
  }

  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const QModelIndex origIndex = mProxyModel->mapToSource( selection[0] );
  Q_ASSERT( origIndex.isValid() );

  const Akonadi::Item origItem = sModel->todoForIndex( origIndex );
  const KCalCore::Todo::Ptr orig = CalendarSupport::todo( origItem );
  if ( !orig ) {
    return;
  }

  KCalCore::Todo::Ptr todo( orig->clone() );

  todo->setUid( KCalCore::CalFormat::createUniqueId() );

  KDateTime due = todo->dtDue();
  due.setDate( date );
  todo->setDtDue( due );

  mChanger->createIncidence( todo, Akonadi::Collection(), this );
}

void KOTodoView::itemDoubleClicked( const QModelIndex &index )
{
  if ( index.isValid() ) {
    QModelIndex summary = index.sibling( index.row(), KOTodoModel::SummaryColumn );
    if ( summary.flags() & Qt::ItemIsEditable ) {
      editTodo();
    } else {
      showTodo();
    }
  }
}

QMenu *KOTodoView::createCategoryPopupMenu()
{
  QMenu *tempMenu = new QMenu( this );

  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return tempMenu;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );

  QStringList checkedCategories = todo->categories();

  QStringList::Iterator it;
  CalendarSupport::CategoryConfig cc( KOPrefs::instance() );
  Q_FOREACH ( const QString &i, cc.customCategories() ) {
    QAction *action = tempMenu->addAction( i );
    action->setCheckable( true );
    mCategory[ action ] = i;
    if ( checkedCategories.contains( i ) ) {
      action->setChecked( true );
    }
  }

  connect( tempMenu, SIGNAL(triggered(QAction*)),
           SLOT(changedCategories(QAction*)) );
  connect( tempMenu, SIGNAL(aboutToHide()),
           tempMenu, SLOT(deleteLater()) );
  return tempMenu;
}

void KOTodoView::setNewDate( const QDate &date )
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );

  if ( calendar()->hasRight( todoItem, Akonadi::Collection::CanChangeItem ) ) {
    KCalCore::Todo::Ptr oldTodo( todo->clone() );

    KDateTime dt( date );

    if ( !todo->allDay() ) {
      dt.setTime( todo->dtDue().time() );
    }

    if ( date.isNull() ) {
      todo->setHasDueDate( false );
    } else if ( !todo->hasDueDate() ) {
      todo->setHasDueDate( true );
    }
    todo->setDtDue( dt );

    mChanger->modifyIncidence( todoItem, oldTodo, this );
  } else {
    kDebug() << "Item is readOnly";
  }
}

void KOTodoView::setNewPercentage( QAction *action )
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );

  if ( calendar()->hasRight( todoItem, Akonadi::Collection::CanChangeItem ) ) {
    KCalCore::Todo::Ptr oldTodo( todo->clone() );

    int percentage = mPercentage.value( action );
    if ( percentage == 100 ) {
      todo->setCompleted( KDateTime::currentLocalDateTime() );
      todo->setPercentComplete( 100 );
    } else {
      todo->setPercentComplete( percentage );
    }
    if ( todo->recurs() && percentage == 100 ) {
      mChanger->modifyIncidence( todoItem, oldTodo, this );
    } else {
      mChanger->modifyIncidence( todoItem, oldTodo, this );
    }
  } else {
    kDebug() << "Item is read only";
  }
}

void KOTodoView::setNewPriority( QAction *action )
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }
  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  if ( calendar()->hasRight( todoItem, Akonadi::Collection::CanChangeItem ) ) {
    KCalCore::Todo::Ptr oldTodo( todo->clone() );
    todo->setPriority( mPriority[action] );

    mChanger->modifyIncidence( todoItem, oldTodo, this );
  }
}

void KOTodoView::changedCategories( QAction *action )
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );
  if ( calendar()->hasRight( todoItem, Akonadi::Collection::CanChangeItem ) ) {
    KCalCore::Todo::Ptr oldTodo( todo->clone() );

    QStringList categories = todo->categories();
    if ( categories.contains( mCategory[action] ) ) {
      categories.removeAll( mCategory[action] );
    } else {
      categories.append( mCategory[action] );
    }
    categories.sort();
    todo->setCategories( categories );
    mChanger->modifyIncidence( todoItem, oldTodo, this );
  } else {
    kDebug() << "No active item, active item is read-only, or locking failed";
  }
}

void KOTodoView::setFullView( bool fullView )
{
  if ( !mFullView ) {
    return;
  }

  mFullView->blockSignals( true );
  // We block signals to avoid recursion, we have two KOTodoViews and mFullView is synchronized
  mFullView->setChecked( fullView );
  mFullView->blockSignals( false );

  KOPrefs::instance()->setFullViewTodo( fullView );
  KOPrefs::instance()->writeConfig();

  emit fullViewChanged( fullView );
}

void KOTodoView::setFlatView( bool flatView )
{
  mFlatView->blockSignals( true );
  // We block signals to avoid recursion, we have two KOTodoViews and mFlatView is synchronized
  mFlatView->setChecked( flatView );
  mFlatView->blockSignals( false );

  mView->setRootIsDecorated( !flatView );

  if ( flatView ) {
    // In flatview dropping confuses users and it's very easy to drop into a child item
    mView->setDragDropMode( QAbstractItemView::DragOnly );
  } else {
    mView->setDragDropMode( QAbstractItemView::DragDrop );
  }

  KOPrefs::instance()->setFlatListTodo( flatView );
  KOPrefs::instance()->writeConfig();
}

void KOTodoView::getHighlightMode( bool &highlightEvents,
                                   bool &highlightTodos,
                                   bool &highlightJournals )
{
  highlightTodos    = KOPrefs::instance()->mHighlightTodos;
  highlightEvents   = !highlightTodos;
  highlightJournals = false;
}

bool KOTodoView::usesFullWindow()
{
  return KOPrefs::instance()->mFullViewTodo;
}

void KOTodoView::resizeColumnsToContent()
{
  mView->resizeColumnToContents( eDueDateColumn );
  mView->resizeColumnToContents( eSummaryColumn );
}

KOrg::CalPrinterBase::PrintType KOTodoView::printType() const
{
  return KOrg::CalPrinterBase::Todolist;
}

#include "kotodoview.moc"
