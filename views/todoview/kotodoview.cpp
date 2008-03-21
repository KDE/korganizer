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
#include "kotodomodel.h"
#include "kotododelegates.h"
#include "koprefs.h"
#include "koglobals.h"

#include <kcal/todo.h>
#include <kcal/incidence.h>

#include <klineedit.h>
#include <kdatepickerpopup.h>
#include <kconfig.h>
#include <kconfiggroup.h>

#include <QVBoxLayout>
#include <QMenu>
#include <QContextMenuEvent>
#include <QTreeView>
#include <QHeaderView>
#include <QSortFilterProxyModel>

using namespace KCal;
using namespace KOrg;
using namespace KPIM;

KOTodoView::KOTodoView( Calendar *cal, QWidget *parent )
  : BaseView( cal, parent )
{
  mQuickSearch = new KOTodoViewQuickSearch( calendar(), this );

  mModel = new KOTodoModel( calendar(), this );
  mProxyModel = new QSortFilterProxyModel( this );
  mProxyModel->setSourceModel( mModel );
  mProxyModel->setDynamicSortFilter( true );
  mProxyModel->setFilterKeyColumn( KOTodoModel::SummaryColumn );
  mProxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );

  connect( mQuickSearch, SIGNAL(searchTextChanged(const QString &)),
           mProxyModel, SLOT(setFilterRegExp(const QString &)) );

  mView = new QTreeView( this );
  mView->setModel( mProxyModel );

  mView->setSortingEnabled( true );

  mView->setDragDropMode( QAbstractItemView::DragDrop );

  mView->setEditTriggers( QAbstractItemView::SelectedClicked |
                          QAbstractItemView::EditKeyPressed );

  KOTodoPriorityDelegate *priorityDelegate = new KOTodoPriorityDelegate( mView );
  mView->setItemDelegateForColumn( KOTodoModel::PriorityColumn, priorityDelegate );
  KOTodoCompleteDelegate *completeDelegate = new KOTodoCompleteDelegate( mView );
  mView->setItemDelegateForColumn( KOTodoModel::PercentColumn, completeDelegate );

  mView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mView, SIGNAL(customContextMenuRequested(const QPoint &)),
           this, SLOT(contextMenu(const QPoint &)) );

  mQuickAdd = new KLineEdit( this );
  mQuickAdd->setClickMessage( i18n( "Click to add a new to-do" ) );
  mQuickAdd->setClearButtonShown( true );
  if ( !KOPrefs::instance()->mEnableQuickTodo ) {
    mQuickAdd->hide();
  }
  connect( mQuickAdd, SIGNAL(returnPressed()), SLOT(addQuickTodo()) );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( mQuickSearch );
  layout->addWidget( mView, 1 );
  layout->addWidget( mQuickAdd );

  setLayout( layout );

  // ---------------- POPUP-MENUS -----------------------
  mItemPopupMenu = new QMenu( this );
  mItemPopupMenu->addAction( i18n( "&Show" ), this, SLOT(showTodo()) );
  mItemPopupMenu->addAction( i18n( "&Edit..." ), this, SLOT(editTodo()) );
#ifndef KORG_NOPRINTER
  mItemPopupMenu->addAction( KOGlobals::self()->smallIcon( "document-print" ),
                             i18n( "&Print..." ), this, SLOT(printTodo()) );
#endif
  mItemPopupMenu->addAction( KOGlobals::self()->smallIconSet( "edit-delete" ),
                             i18n( "&Delete" ), this, SLOT(deleteTodo()) );

  mItemPopupMenu->addSeparator();

  mItemPopupMenu->addAction( KOGlobals::self()->smallIconSet( "view-calendar-tasks" ),
                             i18n( "New &To-do..." ), this, SLOT(newTodo()) );
  mItemPopupMenu->addAction( i18n( "New Su&b-to-do..." ), this, SLOT(newSubTodo()) );

  mItemPopupMenu->addAction( i18n( "&Make this To-do Independent" ), this,
                             SIGNAL(unSubTodoSignal()) );
  mItemPopupMenu->addAction( i18n( "Make all Sub-to-dos &Independent" ), this,
                             SIGNAL(unAllSubTodoSignal()) );

  mItemPopupMenu->addSeparator();

  KDatePickerPopup *mCopyPopupMenu = new KDatePickerPopup(
                                          KDatePickerPopup::NoDate |
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

  mPopupMenu = new QMenu( this );
  mPopupMenu->addAction( KOGlobals::self()->smallIconSet( "view-calendar-tasks" ),
                         i18n( "&New To-do..." ), this, SLOT(newTodo()) );
  mPopupMenu->addAction( i18nc( "delete completed to-dos", "Pur&ge Completed" ),
                         this, SIGNAL(purgeCompletedSignal()) );
}

KOTodoView::~KOTodoView()
{
}

void KOTodoView::setCalendar( Calendar *cal )
{
  BaseView::setCalendar( cal );
  mQuickSearch->setCalendar( cal );
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

  QStringList columnOrder;
  QStringList columnWidths;
  for ( int i = 0; i < header->count(); i++ ) {
    columnWidths << QString::number( header->sectionSize( i ) );
    columnOrder << QString::number( header->visualIndex( i ) );
  }
  cfgGroup.writeEntry( "ColumnOrder", columnOrder );
  cfgGroup.writeEntry( "ColumnWidths", columnWidths );

  cfgGroup.writeEntry( "SortAscending", (int)header->sortIndicatorOrder() );
  cfgGroup.writeEntry( "SortColumn", header->sortIndicatorSection() );

  cfgGroup.sync();
}

void KOTodoView::restoreLayout( KConfig *config, const QString &group )
{
  KConfigGroup cfgGroup = config->group( group );
  QHeaderView *header = mView->header();

  QStringList columnOrder = cfgGroup.readEntry( "ColumnOrder", QStringList() );
  QStringList columnWidths = cfgGroup.readEntry( "ColumnWidths", QStringList() );
  for ( int i = 0;
        i < header->count() && i < columnOrder.size() && i < columnWidths.size();
        i++ ) {
    int width = columnWidths[i].toInt();
    int order = columnOrder[i].toInt();
    header->resizeSection( i, width );
    header->moveSection( header->visualIndex( i ), order );
  }
  int sortOrder = cfgGroup.readEntry( "SortAscending", (int)Qt::AscendingOrder );
  int sortColumn = cfgGroup.readEntry( "SortColumn", 0 );
  header->setSortIndicator( sortColumn, (Qt::SortOrder)sortOrder );
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
  // TODO: hmm, not sure how to do this... there is nothing configurable here yet...
  // maybe just call updateView, to be sure??
  kDebug() << "this is a stub";
}

void KOTodoView::clearSelection()
{
  mView->selectionModel()->clearSelection();
}

void KOTodoView::addQuickTodo()
{
  mModel->addTodo( mQuickAdd->text() );
  mQuickAdd->setText( QString() );
}

void KOTodoView::contextMenu( const QPoint &pos )
{
  QModelIndex index = mView->indexAt( pos );
  if ( index.isValid() ) {
    mItemPopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
  } else {
    mPopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
  }
}

void KOTodoView::selectionChanged( const QItemSelection &selected,
                                   const QItemSelection &deselected )
{
  Q_UNUSED( deselected );
  QModelIndexList selection = selected.indexes();
  if ( selection.isEmpty() ) {
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

#include "kotodoview.moc"
