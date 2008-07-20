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
#include "kotodoviewquicksearch.h"
#include "kotodoviewquickaddline.h"
#include "kotodomodel.h"
#include "kotodoviewsortfilterproxymodel.h"
#include "kotodoviewview.h"
#include "kotododelegates.h"
#include "koprefs.h"
#include "koglobals.h"

#include <kcal/todo.h>
#include <kcal/incidence.h>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdatepickerpopup.h>
#include <kiconloader.h>
#include <klineedit.h>

#include <QGridLayout>
#include <QCheckBox>
#include <QMenu>
#include <QList>
#include <QContextMenuEvent>
#include <QModelIndex>
#include <QHeaderView>

using namespace KCal;
using namespace KOrg;
using namespace KPIM;

KOTodoView::KOTodoView( Calendar *cal, QWidget *parent )
  : BaseView( cal, parent )
{
  mModel = new KOTodoModel( calendar(), this );
  mProxyModel = new KOTodoViewSortFilterProxyModel( this );
  mProxyModel->setSourceModel( mModel );
  mProxyModel->setDynamicSortFilter( true );
  mProxyModel->setFilterKeyColumn( KOTodoModel::SummaryColumn );
  mProxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );

  mQuickSearch = new KOTodoViewQuickSearch( calendar(), this );
  mQuickSearch->setVisible( KOPrefs::instance()->enableTodoQuickSearch() );
  connect( mQuickSearch, SIGNAL(searchTextChanged(const QString &)),
           mProxyModel, SLOT(setFilterRegExp(const QString &)) );
  connect( mQuickSearch, SIGNAL(searchCategoryChanged(const QStringList &)),
           mProxyModel, SLOT(setCategoryFilter(const QStringList &)) );

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

  mCategoriesDelegate = new KOTodoCategoriesDelegate( cal, mView );
  mView->setItemDelegateForColumn( KOTodoModel::CategoriesColumn, mCategoriesDelegate );

  connect( mView, SIGNAL(customContextMenuRequested(const QPoint &)),
           this, SLOT(contextMenu(const QPoint &)) );
  connect( mView, SIGNAL(doubleClicked(const QModelIndex &)),
           this, SLOT(itemDoubleClicked(const QModelIndex &)) );
  connect( mView->selectionModel(),
           SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
           this,
           SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)) );

  mQuickAdd = new KOTodoViewQuickAddLine( this );
  mQuickAdd->setClickMessage( i18n( "Click to add a new to-do" ) );
  mQuickAdd->setClearButtonShown( true );
  mQuickAdd->setVisible( KOPrefs::instance()->enableQuickTodo() );
  connect( mQuickAdd, SIGNAL(returnPressed(Qt::KeyboardModifiers)),
           this, SLOT(addQuickTodo(Qt::KeyboardModifiers)) );

  mFlatView = new QCheckBox( i18nc( "Checkbox to display todos not hirarchical",
                                    "Flat View" ), this );
  mFlatView->setToolTip( i18n( "Display to-dos as list rather than as tree\n"
                               "(i.e. without parental relationship displayed)" ) );
  connect( mFlatView, SIGNAL(toggled(bool)),
           mModel, SLOT(setFlatView(bool)) );

  QGridLayout *layout = new QGridLayout( this );
  layout->addWidget( mQuickSearch, 0, 0, 1, 2 );
  layout->addWidget( mView, 1, 0, 1, 2 );
  layout->setRowStretch( 1, 1 );
  layout->addWidget( mQuickAdd, 2, 0 );
  layout->addWidget( mFlatView, 2, 1 );

  setLayout( layout );

  // ---------------- POPUP-MENUS -----------------------
  mItemPopupMenu = new QMenu( this );

  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    i18n( "&Show" ), this, SLOT(showTodo()) );

  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    i18n( "&Edit..." ), this, SLOT(editTodo()) );

#ifndef KORG_NOPRINTER
  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    KOGlobals::self()->smallIcon( "document-print" ),
    i18n( "&Print..." ), this, SLOT(printTodo()) );
#endif

  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    KIconLoader::global()->loadIcon( "edit-delete", KIconLoader::NoGroup, KIconLoader::SizeSmall ),
    i18n( "&Delete" ), this, SLOT(deleteTodo()) );

  mItemPopupMenu->addSeparator();

  mItemPopupMenu->addAction(
    KIconLoader::global()->loadIcon(
      "view-calendar-tasks", KIconLoader::NoGroup, KIconLoader::SizeSmall ),
    i18n( "New &To-do..." ), this, SLOT(newTodo()) );

  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    i18n( "New Su&b-to-do..." ), this, SLOT(newSubTodo()) );

  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    i18n( "&Make this To-do Independent" ), this, SIGNAL(unSubTodoSignal()) );

  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    i18n( "Make all Sub-to-dos &Independent" ), this, SIGNAL(unAllSubTodoSignal()) );

  mItemPopupMenu->addSeparator();

  mCopyPopupMenu = new KDatePickerPopup( KDatePickerPopup::NoDate |
                                         KDatePickerPopup::DatePicker |
                                         KDatePickerPopup::Words,
                                         QDate::currentDate(), this );
  mCopyPopupMenu->setTitle( i18n( "&Copy To" ) );

  connect( mCopyPopupMenu, SIGNAL(dateChanged(const QDate &)),
           SLOT(copyTodoToDate(const QDate&)) );

  connect( mCopyPopupMenu, SIGNAL(dateChanged(QDate)),
           mItemPopupMenu, SLOT(hide()) );

  mItemPopupMenu->insertMenu( 0, mCopyPopupMenu );

  mItemPopupMenu->addSeparator();

  mItemPopupMenu->addAction( i18nc( "delete completed to-dos", "Pur&ge Completed" ),
                             this, SIGNAL(purgeCompletedSignal()) );
}

KOTodoView::~KOTodoView()
{
}

void KOTodoView::setCalendar( Calendar *cal )
{
  BaseView::setCalendar( cal );
  mQuickSearch->setCalendar( cal );
  mCategoriesDelegate->setCalendar( cal );
  mModel->setCalendar( cal );
}

Incidence::List KOTodoView::selectedIncidences()
{
  Incidence::List ret;

  QModelIndexList selection = mView->selectionModel()->selectedRows();

  Q_FOREACH( const QModelIndex &mi, selection ) {
    Todo *todo = static_cast<Todo *>( mi.data( KOTodoModel::TodoRole ).
                                            value<void *>() );
    ret << todo;
  }

  return ret;
}

DateList KOTodoView::selectedDates()
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

  cfgGroup.writeEntry( "FlatView", mFlatView->isChecked() );
}

void KOTodoView::restoreLayout( KConfig *config, const QString &group )
{
  KConfigGroup cfgGroup = config->group( group );
  QHeaderView *header = mView->header();

  QVariantList columnVisibility = cfgGroup.readEntry( "ColumnVisibility", QVariantList() );
  QVariantList columnOrder = cfgGroup.readEntry( "ColumnOrder", QVariantList() );
  QVariantList columnWidths = cfgGroup.readEntry( "ColumnWidths", QVariantList() );
  for ( int i = 0;
        i < header->count() && i < columnOrder.size() &&
            i < columnWidths.size() && i < columnVisibility.size();
        i++ ) {
    bool visible = columnVisibility[i].toBool();
    int width = columnWidths[i].toInt();
    int order = columnOrder[i].toInt();

    header->resizeSection( i, width );
    header->moveSection( header->visualIndex( i ), order );
    if ( !visible ) {
      mView->hideColumn( i );
    }
  }
  int sortOrder = cfgGroup.readEntry( "SortAscending", (int)Qt::AscendingOrder );
  int sortColumn = cfgGroup.readEntry( "SortColumn", -1 );
  if ( sortColumn >= 0 ) {
    mView->sortByColumn( sortColumn, (Qt::SortOrder)sortOrder );
  }

  mFlatView->setChecked( cfgGroup.readEntry( "FlatView", false ) );
}

void KOTodoView::setIncidenceChanger( IncidenceChangerBase *changer )
{
  BaseView::setIncidenceChanger( changer );
  mModel->setIncidenceChanger( changer );
}

void KOTodoView::showDates( const QDate &start, const QDate &end )
{
  // There is nothing to do here for the Todo View
  Q_UNUSED( start );
  Q_UNUSED( end );
}

void KOTodoView::showIncidences( const Incidence::List &incidenceList )
{
  Q_UNUSED( incidenceList );
  // TODO: hmm, not sure how to do this...
  kDebug() << "this is a stub";
}

void KOTodoView::updateView()
{
  mModel->reloadTodos();
}

void KOTodoView::updateCategories()
{
  mQuickSearch->updateCategories();
  // TODO check if we have to do something with the category delegate
}

void KOTodoView::changeIncidenceDisplay( Incidence *incidence, int action )
{
  mModel->processChange( incidence, action );
}

void KOTodoView::updateConfig()
{
  mQuickSearch->setVisible( KOPrefs::instance()->enableTodoQuickSearch() );
  mQuickAdd->setVisible( KOPrefs::instance()->enableQuickTodo() );

  updateView();
}

void KOTodoView::clearSelection()
{
  mView->selectionModel()->clearSelection();
}

void KOTodoView::addQuickTodo( Qt::KeyboardModifiers modifiers )
{
  if ( modifiers == Qt::NoModifier ) {
    QModelIndex index = mModel->addTodo( mQuickAdd->text() );

    QModelIndexList selection = mView->selectionModel()->selectedRows();
    if ( selection.size() <= 1 ) {
      // don't destroy complex selections, not applicable now (only single
      // selection allowed), but for the future...
      mView->selectionModel()->select( mProxyModel->mapFromSource( index ),
                                       QItemSelectionModel::ClearAndSelect |
                                       QItemSelectionModel::Rows );
    }
  } else if ( modifiers == Qt::ControlModifier ) {
    QModelIndexList selection = mView->selectionModel()->selectedRows();
    if ( selection.size() != 1 ) {
      return;
    }
    mModel->addTodo( mQuickAdd->text(), mProxyModel->mapToSource( selection[0] ) );
  } else {
    return;
  }
  mQuickAdd->setText( QString() );
}

void KOTodoView::contextMenu( const QPoint &pos )
{
  bool enable = mView->indexAt( pos ).isValid();

  Q_FOREACH( QAction *entry, mItemPopupMenuItemOnlyEntries ) {
    entry->setEnabled( enable );
  }
  mCopyPopupMenu->setEnabled( enable );

  mItemPopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
}

void KOTodoView::selectionChanged( const QItemSelection &selected,
                                   const QItemSelection &deselected )
{
  Q_UNUSED( deselected );
  QModelIndexList selection = selected.indexes();
  if ( selection.isEmpty() || !selection[0].isValid() ) {
    emit incidenceSelected( 0 );
    return;
  }

  Todo *todo = static_cast<Todo *>( selection[0].data( KOTodoModel::TodoRole ).value<void *>() );

  emit incidenceSelected( todo );
}

void KOTodoView::showTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  Todo *todo = static_cast<Todo *>( selection[0].data( KOTodoModel::TodoRole ).value<void *>() );

  emit showIncidenceSignal( todo );
}

void KOTodoView::editTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  Todo *todo = static_cast<Todo *>( selection[0].data( KOTodoModel::TodoRole ).value<void *>() );

  emit editIncidenceSignal( todo );
}

void KOTodoView::printTodo()
{
  //TODO
  kDebug() << "this is a stub";
}

void KOTodoView::deleteTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  Todo *todo = static_cast<Todo *>( selection[0].data( KOTodoModel::TodoRole ).value<void *>() );

  emit deleteIncidenceSignal( todo );
}

void KOTodoView::newTodo()
{
  emit newTodoSignal( QDate::currentDate().addDays( 7 ) );
}

void KOTodoView::newSubTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  Todo *todo = static_cast<Todo *>( selection[0].data( KOTodoModel::TodoRole ).value<void *>() );

  emit newSubTodoSignal( todo );
}

void KOTodoView::copyTodoToDate( const QDate &date )
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  mModel->copyTodo( mProxyModel->mapToSource( selection[0] ), date );
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

#include "kotodoview.moc"
