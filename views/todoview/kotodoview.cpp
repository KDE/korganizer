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
#include <kcalprefs.h>
#include "calprinter.h"
#include "kocorehelper.h"
#include "koglobals.h"
#include "koprefs.h"
#include "kohelper.h"
#include "kodialogmanager.h"

#include "kotododelegates.h"
#include "kotodomodel.h"
#include "kotodoviewquickaddline.h"
#include "kotodoviewquicksearch.h"
#include "kotodoviewsortfilterproxymodel.h"
#include "kotodoviewview.h"

#include <calendarsupport/categoryconfig.h>
#include <calendarsupport/utils.h>
#include <calendarsupport/calendar.h>

#include <libkdepim/kdatepickerpopup.h>

#include <kcalcore/calformat.h>
#include <kcalcore/incidence.h>
#include <kcalcore/todo.h>

#include <QCheckBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QMenu>
#include <QTimer>

using namespace KPIM;
using namespace CalendarSupport;

KOTodoView::KOTodoView( QWidget *parent )
  : BaseView( parent )
{
  mModel = new KOTodoModel( this );
  connect( mModel, SIGNAL( expandIndex( const QModelIndex& ) ),
           this, SLOT( expandIndex( const QModelIndex& ) ) );
  mProxyModel = new KOTodoViewSortFilterProxyModel( this );
  mProxyModel->setSourceModel( mModel );
  mProxyModel->setDynamicSortFilter( true );
  mProxyModel->setFilterKeyColumn( KOTodoModel::SummaryColumn );
  mProxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyModel->setSortRole( Qt::EditRole );

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

  mCategoriesDelegate = new KOTodoCategoriesDelegate( mView );
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
  mQuickAdd->setClearButtonShown( true );
  mQuickAdd->setVisible( KOPrefs::instance()->enableQuickTodo() );
  connect( mQuickAdd, SIGNAL(returnPressed(Qt::KeyboardModifiers)),
           this, SLOT(addQuickTodo(Qt::KeyboardModifiers)) );

  mFlatView = new QCheckBox( i18nc( "Checkbox to display todos not hirarchical",
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
           this, SLOT(setFlatView(bool)) );

  QGridLayout *layout = new QGridLayout( this );
  layout->setMargin( 0 );
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

  mCopyPopupMenu = new KDatePickerPopup( KDatePickerPopup::NoDate |
                                         KDatePickerPopup::DatePicker |
                                         KDatePickerPopup::Words,
                                         QDate::currentDate(), this );
  mCopyPopupMenu->setTitle( i18n( "&Copy To" ) );

  connect( mCopyPopupMenu, SIGNAL(dateChanged(const QDate &)),
           SLOT(copyTodoToDate(const QDate&)) );

  connect( mCopyPopupMenu, SIGNAL(dateChanged(QDate)),
           mItemPopupMenu, SLOT(hide()) );

  mMovePopupMenu = new KDatePickerPopup( KDatePickerPopup::NoDate |
                                         KDatePickerPopup::DatePicker |
                                         KDatePickerPopup::Words,
                                         QDate::currentDate(), this );
  mMovePopupMenu->setTitle( i18n( "&Move To" ) );

  connect( mMovePopupMenu, SIGNAL(dateChanged(const QDate &)),
           SLOT(setNewDate(const QDate&)) );

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
  connect( mPriorityPopupMenu, SIGNAL(triggered(QAction *)),
           SLOT(setNewPriority(QAction *)) );

  mPercentageCompletedPopupMenu = new QMenu(this);
  for ( int i = 0; i <= 100; i+=10 ) {
    QString label = QString( "%1 %" ).arg( i );
    mPercentage[mPercentageCompletedPopupMenu->addAction( label )] = i;
  }
  connect( mPercentageCompletedPopupMenu, SIGNAL(triggered(QAction *)),
           SLOT(setNewPercentage(QAction *)) );

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

void KOTodoView::setCalendar( CalendarSupport::Calendar *cal )
{
  BaseView::setCalendar( cal );
  mQuickSearch->setCalendar( cal );
  mCategoriesDelegate->setCalendar( cal );
  mModel->setCalendar( cal );
}

Akonadi::Item::List KOTodoView::selectedIncidences()
{
  Akonadi::Item::List ret;
  const QModelIndexList selection = mView->selectionModel()->selectedRows();
  Q_FOREACH( const QModelIndex &mi, selection ) {
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

  cfgGroup.writeEntry( "FlatView", mFlatView->isChecked() );
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
}

void KOTodoView::setIncidenceChanger( CalendarSupport::IncidenceChanger *changer )
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

void KOTodoView::showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date )
{
  Q_UNUSED( incidenceList );
  Q_UNUSED( date );
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

void KOTodoView::changeIncidenceDisplay( const Akonadi::Item &incidence, int action )
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

void KOTodoView::addTodo( const QString &summary,
                          const Todo::Ptr &parent,
                          const QStringList &categories )
{
  if ( !mChanger || summary.trimmed().isEmpty() ) {
    return;
  }

  Todo::Ptr todo( new Todo );
  todo->setSummary( summary.trimmed() );
  todo->setOrganizer( Person::Ptr( new Person( CalendarSupport::KCalPrefs::instance()->fullName(),
                                               CalendarSupport::KCalPrefs::instance()->email() ) ) );

  todo->setCategories( categories );

/*  if ( parent ) {
    todo->setRelatedTo( parent );
  }
KDAB_TODO: review
*/

  Akonadi::Collection selectedCollection;
  int dialogCode = 0;
  if ( !mChanger->addIncidence( todo, this, selectedCollection, dialogCode ) ) {
    if ( dialogCode != QDialog::Rejected ) {
      KOHelper::showSaveIncidenceErrorMsg( this, todo );
    }
  }
}

void KOTodoView::addQuickTodo( Qt::KeyboardModifiers modifiers )
{
  if ( modifiers == Qt::NoModifier ) {
    /*const QModelIndex index = */ addTodo( mQuickAdd->text(), KCalCore::Todo::Ptr(), mProxyModel->categories() );

#ifdef AKONADI_PORT_DISABLED // the todo is added asynchronously now, so we have to wait until the new item is actually added before selecting the item

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
    const Akonadi::Item parent = mModel->todoForIndex( idx );
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

  Q_FOREACH( QAction *entry, mItemPopupMenuItemOnlyEntries ) {
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

        const bool incidenceIsRO = !calendar()->hasChangeRights( item );

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
        mMakeSubtodosIndependent->setEnabled( !calendar()->findChildren( incidencePtr ).isEmpty() );
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
  Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );

  KOCoreHelper helper;
  CalPrinter printer( this, BaseView::calendar(), &helper, true );
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
    const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
    if ( mChanger->isNotDeleted( todoItem.id() ) ) {
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
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();

  emit newSubTodoSignal( todoItem );
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

  const Akonadi::Item origItem = mModel->todoForIndex( origIndex );
  const Todo::Ptr orig = CalendarSupport::todo( origItem );
  if ( !orig ) {
    return;
  }

  Todo::Ptr todo( orig->clone() );

  todo->setUid( CalFormat::createUniqueId() );

  KDateTime due = todo->dtDue();
  due.setDate( date );
  todo->setDtDue( due );

  Akonadi::Collection selectedCollection;
  int dialogCode = 0;
  if ( !mChanger->addIncidence( todo, this, selectedCollection, dialogCode ) ) {
    if ( dialogCode != QDialog::Rejected ) {
      KOHelper::showSaveIncidenceErrorMsg( this, todo );
    }
  }
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
  Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );

  QStringList checkedCategories = todo->categories();

  QStringList::Iterator it;
  CategoryConfig cc( KOPrefs::instance() );
  Q_FOREACH ( const QString &i, cc.customCategories() ) {
    QAction *action = tempMenu->addAction( i );
    action->setCheckable( true );
    mCategory[ action ] = i;
    if ( checkedCategories.contains( i ) ) {
      action->setChecked( true );
    }
  }

  connect( tempMenu, SIGNAL(triggered(QAction *)),
           SLOT(changedCategories(QAction *)) );
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
  Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );

  if ( calendar()->hasChangeRights( todoItem ) ) {
    Todo::Ptr oldTodo( todo->clone() );

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

    mChanger->changeIncidence( oldTodo, todoItem, CalendarSupport::IncidenceChanger::COMPLETION_MODIFIED, this );
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
  Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );

  if ( calendar()->hasChangeRights( todoItem ) ) {
    Todo::Ptr oldTodo( todo->clone() );

    int percentage = mPercentage.value( action );
    if ( percentage == 100 ) {
      todo->setCompleted( KDateTime::currentLocalDateTime() );
      todo->setPercentComplete( 100 );
    } else {
      todo->setPercentComplete( percentage );
    }
    if ( todo->recurs() && percentage == 100 ) {
      mChanger->changeIncidence( oldTodo, todoItem,
                                 CalendarSupport::IncidenceChanger::COMPLETION_MODIFIED_WITH_RECURRENCE, this );
    } else {
      mChanger->changeIncidence( oldTodo, todoItem,
                                 CalendarSupport::IncidenceChanger::COMPLETION_MODIFIED, this );
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
  Todo::Ptr todo = CalendarSupport::todo( todoItem );
  if ( calendar()->hasChangeRights( todoItem ) ) {
    Todo::Ptr oldTodo( todo->clone() );
    todo->setPriority( mPriority[action] );

    mChanger->changeIncidence( oldTodo, todoItem, CalendarSupport::IncidenceChanger::PRIORITY_MODIFIED, this );
  }
}

void KOTodoView::changedCategories( QAction *action )
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( KOTodoModel::TodoRole ).value<Akonadi::Item>();
  Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );
  if ( calendar()->hasChangeRights( todoItem ) ) {
    Todo::Ptr oldTodo( todo->clone() );

    QStringList categories = todo->categories();
    if ( categories.contains( mCategory[action] ) ) {
      categories.removeAll( mCategory[action] );
    } else {
      categories.append( mCategory[action] );
    }
    categories.sort();
    todo->setCategories( categories );
    mChanger->changeIncidence( oldTodo, todoItem, CalendarSupport::IncidenceChanger::CATEGORY_MODIFIED, this );
  } else {
    kDebug() << "No active item, active item is read-only, or locking failed";
  }
}

void KOTodoView::setFlatView( bool flatView )
{
  mView->setRootIsDecorated( !flatView );
  mModel->setFlatView( flatView );

  if ( flatView ) {
    // In flatview dropping confuses users and it's very easy to drop into a child item
    mView->setDragDropMode( QAbstractItemView::DragOnly );
  } else {
    mView->setDragDropMode( QAbstractItemView::DragDrop );
  }
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
