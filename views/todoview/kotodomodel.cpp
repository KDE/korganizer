/*
  This file is part of KOrganizer.

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

#include "kotodomodel.h"
#include "koglobals.h"
#include "kohelper.h"
#include "koprefs.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/dndfactory.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <KCalCore/Attachment>
#include <KCalCore/Attendee>

#include <KCalUtils/ICalDrag>
#include <KCalUtils/IncidenceFormatter>
#include <KCalUtils/VCalDrag>

#include <KPIMUtils/Email>

#include <KMessageBox>

#include <QIcon>
#include <QMimeData>

/** This class represents a node in the todo-tree. */
struct KOTodoModel::TodoTreeNode : QObject
{
  TodoTreeNode( const Akonadi::Item &todo_, TodoTreeNode *parent, KOTodoModel *model )
    : mTodo( todo_ ), mParent( parent ), mParentListPos( 0 ),
      mToDelete( false ), mModel( model )
  {
    if ( CalendarSupport::hasTodo( mTodo ) ) {
      mItemId = mTodo.id();
      mModel->mTodoUidHash.insert( CalendarSupport::todo( mTodo )->uid(), this );
    }
  }

  /** Recursively delete all TodoTreeNodes which are children of this one. */
  ~TodoTreeNode()
  {
    if ( CalendarSupport::hasTodo( mTodo ) ) {
      mModel->mTodoUidHash.remove( CalendarSupport::todo( mTodo )->uid() );
    } else {
      // root node gets deleted, clear the whole hash
      mModel->mTodoUidHash.clear();
    }
    qDeleteAll( mChildren );
  }

  /** Checks if it's save to access the todo-pointer of this node. */
  bool isValid() const
  {
    return CalendarSupport::hasTodo( mTodo ) && !mToDelete;
  }

  /** Recursively set mToDelete to true for all todos.
   *
   *  Used in reloadTodos to mark all todos for possible deletion.
   */
  void setToDelete()
  {
    Q_FOREACH ( TodoTreeNode *node, mChildren ) {
      node->setToDelete();
    }
    mToDelete = true;
  }

  /** Recursively delete all nodes which are marked for deletion. */
  void deleteMarked()
  {
    if ( mToDelete ) {
      /*
        I commented the following check because the item get be deleted outside of korg.

#ifndef NDEBUG

      // all sub-todos should be marked for delete too, otherwise there is
      // something wrong
      QList<TodoTreeNode*> toCheck;
      toCheck << mChildren;
      while ( !toCheck.isEmpty() ) {
        TodoTreeNode *node = toCheck.takeFirst();
        Q_ASSERT ( node->mToDelete || CalendarSupport::todo( node->mTodo )->relatedTo().isEmpty() );
        toCheck << node->mChildren;
      }

#endif
      */

      QModelIndex tmp = mModel->getModelIndex( this );
      mModel->beginRemoveRows( mModel->getModelIndex( mParent ),
                               tmp.row(), tmp.row() );
      mParent->removeChild( tmp.row() );
      mModel->endRemoveRows();

      deleteLater();
    } else {
      Q_FOREACH ( TodoTreeNode *node, mChildren ) {
        node->deleteMarked();
      }
    }
  }

  /** Point to the todo this TodoTreeNode node represents.
   *  Note: public field for performance reason, and because this
   *  class is only used internally from KOTodoModel
   */
  Akonadi::Item mTodo;
  /** Pointer to the parent TodoTreeNode. Only the root element has no parent. */
  TodoTreeNode *mParent;
  /** Position of this TodoTreeNode in it's parents children list */
  int mParentListPos;

  /** Used during reloadTodos to indicate if the todo should be deleted later */
  bool mToDelete;

  /** Add a new child to this TodoTreeNode */
  void addChild( TodoTreeNode *node )
  {
    node->mParentListPos = childrenCount();
    mChildren.append( node );
  }

  /** Remove the child at the specified position */
  void removeChild( int pos )
  {
    Q_ASSERT( pos >= 0 && pos < childrenCount() );
    for ( int i = pos+1; i < childrenCount(); ++i ) {
      childAt( i )->mParentListPos--;
    }
    mChildren.removeAt( pos );
  }

  /** Returns true if this TodoTreeNode has children */
  bool hasChildren() const { return !mChildren.isEmpty(); }

  /** Returns the count of children of this TodoTreeNode */
  int childrenCount() const { return mChildren.count(); }

  /** Returns the child at the given position */
  TodoTreeNode *childAt( int pos ) const { return mChildren[ pos ]; }

  private:
    /** List of pointer to the child nodes. */
    QList<TodoTreeNode*> mChildren;
    /** Pointer to the KOTodoModel owning this object */
    KOTodoModel *mModel;
    /** The uid of the Todo object, needed in the dtor where mTodo might be invalid already. */
    Akonadi::Item::Id mItemId;
};

KOTodoModel::KOTodoModel( QObject *parent )
  : QAbstractItemModel( parent ), mColumnCount( ColumnCount ), mCalendar( 0 ),
    mChanger( 0 )
{
  mRootNode = new TodoTreeNode( Akonadi::Item(), 0, this );
  mFlatView = false;
}

static bool isDueToday( const KCalCore::Todo::Ptr &todo )
{
  return !todo->isCompleted() && todo->dtDue().date() == QDate::currentDate();
}

KOTodoModel::~KOTodoModel()
{
  delete mRootNode;
}

void KOTodoModel::setCalendar( CalendarSupport::Calendar *cal )
{
  if ( cal != mCalendar ) {
    mCalendar = cal;
    // old todos might no longer be valid, so clear them
    clearTodos();
    reloadTodos();
  }
}

void KOTodoModel::clearTodos()
{
  delete mRootNode;
  mRootNode = new TodoTreeNode( Akonadi::Item(), 0, this );
  // inform all views that we cleared our internal list
  reset();
}

void KOTodoModel::reloadTodos()
{
  // don't remove and reload all todos, this would invalidate all
  // model indexes, and the view could not maintain their status
  // (expanded / collapsed)

  // mark all current TodoTreeNodes as canditates for deletion
  mRootNode->setToDelete();
  // never delete the root node
  mRootNode->mToDelete = false;

  Akonadi::Item::List todoList = mCalendar->todos();
  Akonadi::Item::List::ConstIterator it;
  QList<TodoTreeNode*> changedNodes;
  for ( it = todoList.constBegin(); it != todoList.constEnd(); ++it ) {
    TodoTreeNode *tmp = findTodo( CalendarSupport::incidence( *it )->uid() );
    if ( !tmp ) {
      // kDebug() << "Inserting " << CalendarSupport::todo(*it)->summary()
      //          << CalendarSupport::todo(*it)->relatedTo();
      insertTodo( *it );
    } else {
      // update pointer to the todo
      // apparently this is necessary because undo's and redo's don't modify
      // the modified todos but replace pointers to them with others
      tmp->mTodo = *it;

      // the todo is still in the calendar, we don't delete it
      tmp->mToDelete = false;

      changedNodes << tmp;
    }
  }

  // delete all TodoTreeNodes which are still marked for deletion
  mRootNode->deleteMarked();

  // move todos if they changed their place in the hierarchy
  Q_FOREACH ( TodoTreeNode *tmp, changedNodes ) {
    const QModelIndex miChanged = moveIfParentChanged( tmp, tmp->mTodo, true );
    emit dataChanged( miChanged, miChanged.sibling( miChanged.row(), mColumnCount - 1 ) );
  }
}

void KOTodoModel::processChange( const Akonadi::Item &aitem,
                                 Akonadi::IncidenceChanger::ChangeType changeType )
{
  if ( !CalendarSupport::hasTodo( aitem ) ) {
    return;
  }

  if ( changeType == Akonadi::IncidenceChanger::ChangeTypeModify ) {
    TodoTreeNode *ttTodo = findTodo( CalendarSupport::incidence ( aitem )->uid() );
    if ( !ttTodo || !ttTodo->isValid() ) {
      return;
    }

    QModelIndex miChanged = moveIfParentChanged( ttTodo, aitem, false );

    // force the view to redraw the element even if the parent relationship
    // changed, because we can't be sure that only the relationship changed
    emit dataChanged( miChanged,
                      miChanged.sibling( miChanged.row(), mColumnCount - 1 ) );
  } else if ( changeType == Akonadi::IncidenceChanger::ChangeTypeCreate ) {
    const bool found = findTodo( CalendarSupport::incidence ( aitem )->uid() );

    // "found" can be true, with akonadi calendarview listens to ETM model signals
    // and calls updateview, which calls reloadTodos().  I think we have lots of room
    // for optimization and refactoring in this area. Calling reloadTodos() for one incidence
    // doesn't justify it.
    if ( !found ) {
      insertTodo( aitem );
    }
  } else if ( changeType == Akonadi::IncidenceChanger::ChangeTypeDelete ) {
    TodoTreeNode *ttTodo = findTodo( CalendarSupport::incidence ( aitem )->uid() );
    if ( !ttTodo || !ttTodo->isValid() ) {
      return;
    }

    // find the model index of the deleted incidence
    QModelIndex miDeleted = getModelIndex( ttTodo );

    beginRemoveRows( miDeleted.parent(), miDeleted.row(), miDeleted.row() );
    ttTodo->mParent->removeChild( miDeleted.row() );

    if ( ttTodo->hasChildren() ) {
      moveIfParentChanged( ttTodo, aitem, true );
    }

    delete ttTodo;
    endRemoveRows();
  } else {
    kDebug() << "Not edited, added or deleted, so what is this all about? We just reload...";
    reloadTodos();
  }
}

Akonadi::Item KOTodoModel::todoForIndex( const QModelIndex &idx ) const
{
  if ( !idx.isValid() ) {
    return Akonadi::Item();
  }

  TodoTreeNode *node = static_cast<TodoTreeNode *>( idx.internalPointer() );
  Q_ASSERT( node );
  return node->isValid() ? node->mTodo : Akonadi::Item();
}

QModelIndex KOTodoModel::getModelIndex( TodoTreeNode *node ) const
{
  Q_ASSERT( node );
  if ( node == mRootNode ) {
    return QModelIndex();
  }

  return createIndex( node->mParentListPos, 0, node );
}

QModelIndex KOTodoModel::moveIfParentChanged( TodoTreeNode *curNode, const Akonadi::Item &aitem,
                                              bool addParentIfMissing )
{
  const KCalCore::Todo::Ptr todo = CalendarSupport::todo( aitem );
  // find the model index of the changed incidence
  QModelIndex miChanged = getModelIndex( curNode );

  // get the current parent
  TodoTreeNode *ttOldParent = curNode->mParent;

  // get the new parent
  KCalCore::Todo::Ptr newParent;

  if ( !mFlatView ) {
    // in flat view, no todo has a parent

    if ( !isInHierarchyLoop( todo ) ) {
      const QString parentUid = todo->relatedTo();
      if ( !parentUid.isEmpty() ) {
        Akonadi::Item parentItem = mCalendar->itemForIncidenceUid( parentUid );
        KCalCore::Incidence::Ptr inc = CalendarSupport::incidence( parentItem );
        if ( inc && inc->type() == KCalCore::Incidence::TypeTodo ) {
          newParent = KCalCore::Todo::Ptr( static_cast<KCalCore::Todo *>( inc ->clone() ) );
        }
      }
    }
  }

  // check if the relation to the parent has changed
  if ( ( newParent == 0 && CalendarSupport::hasTodo( ttOldParent->mTodo ) ) ||  // became parentless
       ( newParent != 0 && !CalendarSupport::hasTodo( ttOldParent->mTodo ) ) ||  // gained a parent
       ( newParent != 0 && CalendarSupport::hasTodo( ttOldParent->mTodo ) &&     // changed parent
         newParent->uid() != CalendarSupport::todo( ttOldParent->mTodo )->uid() ) ) {

    kDebug() << "parent changed";

    // find the node and model index of the new parent
    TodoTreeNode *ttNewParent = 0;
    if ( newParent ) {
      Akonadi::Item newParentItem;
      newParentItem.setPayload<KCalCore::Todo::Ptr>(newParent);
      ttNewParent = findTodo( CalendarSupport::incidence ( newParentItem )->uid() );
      if ( !ttNewParent && addParentIfMissing ) {
        ttNewParent = insertTodo( newParentItem );
      }
    } else {
      ttNewParent = mRootNode;
    }
    Q_ASSERT( ttNewParent );
    QModelIndex miNewParent = getModelIndex( ttNewParent );

    emit layoutAboutToBeChanged();
    // create a list of all model indexes which will be changed
    QModelIndexList indexListFrom, indexListTo;
    for ( int r = miChanged.row(); r < ttOldParent->childrenCount(); ++r ) {
      for ( int c = 0; c < mColumnCount; ++c ) {
        indexListFrom << createIndex( r, c, ttOldParent->childAt( r ) );
      }
    }

    ttOldParent->removeChild( miChanged.row() );

    // insert the changed todo
    ttNewParent->addChild( curNode );
    curNode->mParent = ttNewParent;

    QModelIndex miMoved = getModelIndex( curNode );

    // create a list of all changed model indexes
    for ( int c = 0; c < mColumnCount; ++c ) {
      indexListTo << createIndex( miMoved.row(), c, curNode );
    }
    for ( int r = miChanged.row(); r < ttOldParent->childrenCount(); ++r ) {
      for ( int c = 0; c < mColumnCount; ++c ) {
        indexListTo << createIndex( r, c, ttOldParent->childAt( r ) );
      }
    }
      // update the persistend model indexes
    changePersistentIndexList( indexListFrom, indexListTo );

    emit layoutChanged();

    miChanged = miMoved;
  }

  return miChanged;
}

KOTodoModel::TodoTreeNode *KOTodoModel::findTodo( const QString &uid ) const
{
  return mTodoUidHash.value( uid );
}

void KOTodoModel::expandTodoIfNeeded( const Akonadi::Item &todoItem )
{
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  if ( !todo ) {
    return;
  }

  if ( todo->isOverdue() || isDueToday( todo ) ) {
    QModelIndex index = getModelIndex( findTodo( CalendarSupport::incidence( todoItem )->uid() ) );
    emit expandIndex( index );
  }
}

bool KOTodoModel::isInHierarchyLoop( const KCalCore::Todo::Ptr &todo ) const
{
  if ( !todo ) {
    return false;
  }

  QString parentUid = todo->relatedTo();

  if ( !parentUid.isEmpty() ) {
    KCalCore::Incidence::Ptr i =
      CalendarSupport::incidence( mCalendar->itemForIncidenceUid( parentUid ) );
    QList<KCalCore::Incidence::Ptr > processedParents;

    // Lets iterate through all parents, if we find one with the same
    // uid then there's a loop.
    while ( i ) {
      if ( i->uid() == todo->uid() ) {
        // loop detected!
        return true;
      } else {
        if ( !processedParents.contains( i ) ) {
          processedParents.append( i );
          // Next parent
          parentUid = todo->relatedTo();
          i = CalendarSupport::incidence( mCalendar->itemForIncidenceUid( parentUid ) );
        } else {
          // There's a loop but this to-do isn't in it
          // the loop is at a higher level, e.g:
          // t1->t2->t3->t4->t3->t4->t3->t4->t3..
          // t1 and t2 can be added as sons of t3 without
          // danger of infinit looping
          return false;
        }
      }
    }
  }

  return false;
}

KOTodoModel::TodoTreeNode *KOTodoModel::insertTodo( const Akonadi::Item &todoItem,
                                                    bool checkRelated )
{
  const KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  if ( !mFlatView && checkRelated && todo && !todo->relatedTo().isEmpty() ) {
    const QString parentUid = todo->relatedTo();
    Akonadi::Item parentItem = mCalendar->itemForIncidenceUid( parentUid );
    KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( parentItem );
    KCalCore::Todo::Ptr relatedTodo = incidence.dynamicCast<KCalCore::Todo>();

    // check if there are recursively linked todos, or if we got broken input data
    if ( isInHierarchyLoop( todo ) || !relatedTodo ) {
      // recursion detected, break recursion
      return insertTodo( todoItem, false );
    }

    // FIXME(AKONADI_PORT) how to handle the related incidences where we don't know the item?
    Akonadi::Item relatedTodoItem;
    relatedTodoItem.setPayload( KCalCore::Todo::Ptr( relatedTodo->clone() ) );

    // if the parent is not already in the tree, we have to insert it first.
    // necessary because we can't rely on todos coming in a defined order.
    TodoTreeNode *parent = findTodo( relatedTodo->uid() );

    if ( !parent ) {
      parent = insertTodo( relatedTodoItem, checkRelated );
    }

    beginInsertRows( getModelIndex( parent ), parent->childrenCount(),
                                              parent->childrenCount() );

    // add the todo under it's parent
    TodoTreeNode *ret = new TodoTreeNode( todoItem, parent, this );
    parent->addChild( ret );

    endInsertRows();
    expandTodoIfNeeded( todoItem );
    return ret;
  } else {
    beginInsertRows( getModelIndex( mRootNode ), mRootNode->childrenCount(),
                                                 mRootNode->childrenCount() );

    // add the todo as root item
    TodoTreeNode *ret = new TodoTreeNode( todoItem, mRootNode, this );
    mRootNode->addChild( ret );

    endInsertRows();
    expandTodoIfNeeded( todoItem );
    return ret;
  }
}

void KOTodoModel::setFlatView( bool flatView )
{
  if ( mFlatView != flatView ) {
    mFlatView = flatView;
    reloadTodos();
    // Tell both views, so they can disable drag n drop if needed
    emit flatViewChanged( flatView );
  }
}

void KOTodoModel::setFullView( bool fullView )
{
  if ( mFullView != fullView ) {
    mFullView = fullView;
    emit fullViewChanged( fullView );
  }
}

Qt::ItemFlags KOTodoModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags ret = QAbstractItemModel::flags( index );

  if ( !index.isValid() ) {
    return ret | Qt::ItemIsDropEnabled;
  }

  TodoTreeNode *node = static_cast<TodoTreeNode *>( index.internalPointer() );
  if ( !node->isValid() ) {
    return ret;
  }

  ret |= Qt::ItemIsDragEnabled;

  const KCalCore::Todo::Ptr todo = CalendarSupport::todo( node->mTodo );

  if ( mCalendar->hasChangeRights( node->mTodo ) ) {
    // the following columns are editable:
    switch ( index.column() ) {
    case SummaryColumn:
    case PriorityColumn:
    case PercentColumn:
    case DueDateColumn:
    case CategoriesColumn:
      ret |= Qt::ItemIsEditable;
      break;
    case DescriptionColumn:
      if ( !todo->descriptionIsRich() ) {
        ret |= Qt::ItemIsEditable;
      }
      break;
    }
  }

  if ( index.column() == 0 ) {
    // whole rows should have checkboxes, so append the flag for the
    // first item of every row only. Also, only the first item of every
    // row should be used as a target for a drag and drop operation.
    ret |= Qt::ItemIsUserCheckable |
           Qt::ItemIsDropEnabled;
  }
  return ret;
}

QModelIndex KOTodoModel::index( int row, int column,
                                const QModelIndex &parent ) const
{
  // This method creates model indexes.
  // NOTE: we use a pointer to the correponding TodoTreeNode node as
  // internal data for each model index.

  if ( row < 0 || row >= rowCount( parent ) ||
       column < 0 || column >= mColumnCount ) {
    return QModelIndex();
  }

  if ( !parent.isValid() ) {
    return createIndex( row, column, mRootNode->childAt( row ) );
  } else {
    TodoTreeNode *node = static_cast<TodoTreeNode *>( parent.internalPointer() );
    if ( node->isValid() ) {
      return createIndex( row, column, node->childAt( row ) );
    } else {
      return QModelIndex();
    }
  }
}

QModelIndex KOTodoModel::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() || !child.internalPointer() ) {
    return QModelIndex();
  }

  TodoTreeNode *node = static_cast<TodoTreeNode *>( child.internalPointer() );
  if ( node->isValid() ) {
    return getModelIndex( node->mParent );
  } else {
    return QModelIndex();
  }
}

int KOTodoModel::columnCount( const QModelIndex &/*parent*/ ) const
{
  return mColumnCount;
}

int KOTodoModel::rowCount( const QModelIndex &parent ) const
{
  if ( !parent.isValid() ) {
    return mRootNode->childrenCount();
  }

  if ( parent.column() == 0 ) {
    // only items in the first column have children
    TodoTreeNode *node = static_cast<TodoTreeNode *>( parent.internalPointer() );
    if ( node->isValid() ) {
      return node->childrenCount();
    }
  }
  return 0;
}

QVariant KOTodoModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() ) {
    return QVariant();
  }

  TodoTreeNode *node = static_cast<TodoTreeNode *>( index.internalPointer() );
  if ( !node->isValid() ) {
    return QVariant();
  }

  const KCalCore::Todo::Ptr todo = CalendarSupport::todo( node->mTodo );

  if ( role == Qt::DisplayRole ) {
    switch ( index.column() ) {
    case SummaryColumn:
      return QVariant( todo->summary() );
    case RecurColumn:
      return QVariant( todo->recurs() ?
                       i18nc( "yes, recurring to-do", "Yes" ) :
                       i18nc( "no, not a recurring to-do", "No" ) );
    case PriorityColumn:
      if ( todo->priority() == 0 ) {
        return QVariant( QString::fromAscii( "--" ) );
      }
      return QVariant( todo->priority() );
    case PercentColumn:
      return QVariant( todo->percentComplete() );
    case DueDateColumn:
      if ( todo->hasDueDate() && todo->dtDue().date().isValid() ) {
        return QVariant(
          KCalUtils::IncidenceFormatter::dateToString( todo->dtDue() ) );
      } else {
        return QVariant( QString() );
      }
    case CategoriesColumn:
    {
      QString categories = todo->categories().join(
        i18nc( "delimiter for joining category names", "," ) );
      return QVariant( categories );
    }
    case DescriptionColumn:
      return QVariant( todo->description() );
    case CalendarColumn:
      return QVariant( CalendarSupport::displayName( mCalendar, node->mTodo.parentCollection() ) );
    }
    return QVariant();
  }

  if ( role == Qt::EditRole ) {
    switch ( index.column() ) {
    case SummaryColumn:
      return QVariant( todo->summary() );
    case RecurColumn:
      return QVariant( todo->recurs() );
    case PriorityColumn:
      return QVariant( todo->priority() );
    case PercentColumn:
      return QVariant( todo->percentComplete() );
    case DueDateColumn:
      return QVariant( todo->dtDue().date() );
    case CategoriesColumn:
      return QVariant( todo->categories() );
    case DescriptionColumn:
      return QVariant( todo->description() );
    case CalendarColumn:
      return QVariant( CalendarSupport::displayName( mCalendar, node->mTodo.parentCollection() ) );
    }
    return QVariant();
  }

  // set the tooltip for every item
  if ( role == Qt::ToolTipRole ) {
    if ( KOPrefs::instance()->enableToolTips() ) {
      return QVariant(
        KCalUtils::IncidenceFormatter::toolTipStr(
          CalendarSupport::displayName( mCalendar, node->mTodo.parentCollection() ),
          todo, QDate(), true,
          CalendarSupport::KCalPrefs::instance()->timeSpec() ) );
    } else {
      return QVariant();
    }
  }

  // background colour for todos due today or overdue todos
  if ( role == Qt::BackgroundRole ) {
    if ( todo->isOverdue() ) {
      return QVariant(
        QBrush( KOPrefs::instance()->todoOverdueColor() ) );
    } else if ( isDueToday( todo ) ) {
      return QVariant(
        QBrush( KOPrefs::instance()->todoDueTodayColor() ) );
    }
  }

  // indicate if a row is checked (=completed) only in the first column
  if ( role == Qt::CheckStateRole && index.column() == 0 ) {
    if ( todo->isCompleted() ) {
      return QVariant( Qt::Checked );
    } else {
      return QVariant( Qt::Unchecked );
    }
  }

  // icon for recurring todos
  // It's in the summary column so you don't accidentally click
  // the checkbox ( which increments the next occurrence date ).
  if ( role == Qt::DecorationRole && index.column() == SummaryColumn ) {
    if ( todo->recurs() ) {
      return QVariant( QIcon( KOGlobals::self()->smallIcon( "task-recurring" ) ) );
    }
  }

  // category colour
  if ( role == Qt::DecorationRole && index.column() == SummaryColumn ) {
    QStringList categories = todo->categories();
    if ( !categories.isEmpty() ) {
      return
        QVariant( CalendarSupport::KCalPrefs::instance()->categoryColor( categories.first() ) );
    }
  }

  if ( role == TodoRole ) {
    QVariant ret( QMetaType::VoidStar );
    ret.setValue( node->mTodo );
    return ret;
  }

  if ( role == IsRichTextRole ) {
    if ( index.column() == SummaryColumn ) {
      return QVariant( todo->summaryIsRich() );
    } else if ( index.column() == DescriptionColumn ) {
      return QVariant( todo->descriptionIsRich() );
    } else {
      return QVariant();
    }
  }

  if ( role == Qt::TextAlignmentRole ) {
    switch ( index.column() ) {
      // If you change this, change headerData() too.
      case RecurColumn:
      case PriorityColumn:
      case PercentColumn:
      case DueDateColumn:
        return QVariant( Qt::AlignHCenter | Qt::AlignVCenter );
    }
    return QVariant( Qt::AlignLeft | Qt::AlignVCenter );
  }

  return QVariant();
}

QVariant KOTodoModel::headerData( int column,
                                  Qt::Orientation orientation,
                                  int role ) const
{
  Q_ASSERT( column >= 0 && column < mColumnCount );

  if ( orientation != Qt::Horizontal ) {
    return QVariant();
  }

  if ( role == Qt::DisplayRole ) {
    switch ( column ) {
    case SummaryColumn:
      return QVariant( i18n( "Summary" ) );
    case RecurColumn:
      return QVariant( i18n( "Recurs" ) );
    case PriorityColumn:
      return QVariant( i18n( "Priority" ) );
    case PercentColumn:
      return QVariant( i18nc( "@title:column percent complete", "Complete" ) );
    case DueDateColumn:
      return QVariant( i18n( "Due Date/Time" ) );
    case CategoriesColumn:
      return QVariant( i18n( "Categories" ) );
    case DescriptionColumn:
      return QVariant( i18n( "Description" ) );
    case CalendarColumn:
      return QVariant( i18n( "Calendar" ) );
    }
  }

  if ( role == Qt::TextAlignmentRole ) {
    switch ( column ) {
      // If you change this, change data() too.
      case RecurColumn:
      case PriorityColumn:
      case PercentColumn:
      case DueDateColumn:
        return QVariant( Qt::AlignHCenter );
    }
    return QVariant();
  }
  return QVariant();
}

bool KOTodoModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !mChanger || !index.isValid() ) {
    return false;
  }

  TodoTreeNode *node = static_cast<TodoTreeNode *>( index.internalPointer() );
  if ( !node->isValid() ) {
    return false;
  }

  const QVariant oldValue = data( index, role );

  if ( oldValue == value ) {
    // Nothing changed, the user used one of the QStyledDelegate's editors but seted the old value
    // Lets just skip this then and avoid a roundtrip to akonadi, and avoid sending invitations
    return true;
  }

  KCalCore::Todo::Ptr todo = CalendarSupport::todo( node->mTodo );
  todo->resetDirtyFields();
  if ( mCalendar->hasChangeRights( node->mTodo ) ) {
    KCalCore::Todo::Ptr oldTodo( todo->clone() );
    if ( role == Qt::CheckStateRole && index.column() == 0 ) {
      todo->setCompleted( static_cast<Qt::CheckState>( value.toInt() ) == Qt::Checked );
    }

    if ( role == Qt::EditRole ) {
      switch ( index.column() ) {
        case SummaryColumn:
          if ( !value.toString().isEmpty() ) {
            todo->setSummary( value.toString() );
          }
          break;
        case PriorityColumn:
          todo->setPriority( value.toInt() );
          break;
        case PercentColumn:
          todo->setPercentComplete( value.toInt() );
          break;
        case DueDateColumn:
          {
            KDateTime tmp = todo->dtDue();
            tmp.setDate( value.toDate() );
            todo->setDtDue( tmp );
            todo->setHasDueDate( value.toDate().isValid() );
          }
          break;
        case CategoriesColumn:
          todo->setCategories( value.toStringList() );
          break;
        case DescriptionColumn:
          todo->setDescription( value.toString() );
          break;
      }
    }

    if ( !todo->dirtyFields().isEmpty() ) {
      mChanger->modifyIncidence( node->mTodo, oldTodo );
      // modifyIncidence will eventually call the view's
      // changeIncidenceDisplay method, which in turn
      // will call processChange. processChange will then emit
      // dataChanged to the view, so we don't have to
      // do it here
    }

    return true;
  } else {
    if ( !( role == Qt::CheckStateRole && index.column() == 0 ) ) {
      KOHelper::showSaveIncidenceErrorMsg( 0, todo ); //TODO pass parent
    }
    return false;
  }
}

Qt::DropActions KOTodoModel::supportedDropActions() const
{
  // Qt::CopyAction not supported yet
  return Qt::MoveAction;
}

QStringList KOTodoModel::mimeTypes() const
{
  QStringList ret;

  ret << KCalUtils::ICalDrag::mimeType() << KCalUtils::VCalDrag::mimeType();

  return ret;
}

QMimeData *KOTodoModel::mimeData( const QModelIndexList &indexes ) const
{
  Akonadi::Item::List items;
  Q_FOREACH ( const QModelIndex &index, indexes ) {
    const TodoTreeNode * const node = static_cast<TodoTreeNode *>( index.internalPointer() );
    if ( node->isValid() && !items.contains( node->mTodo ) ) {
      items.push_back( node->mTodo );
    }
  }
  return CalendarSupport::createMimeData( items, mCalendar->timeSpec() );
}

bool KOTodoModel::dropMimeData( const QMimeData *data, Qt::DropAction action,
                                int row, int column,
                                const QModelIndex &parent )
{
  Q_UNUSED( row );
  Q_UNUSED( column );

  if ( action != Qt::MoveAction ) {
    kDebug() << "No action other than MoveAction currently supported!"; //TODO
    return false;
  }

  if ( mCalendar && mChanger &&
       ( KCalUtils::ICalDrag::canDecode( data ) || KCalUtils::VCalDrag::canDecode( data ) ) ) {
    CalendarSupport::DndFactory dndFactory (
      CalendarSupport::CalendarAdaptor::Ptr(
        new CalendarSupport::CalendarAdaptor( mCalendar, 0 ) ), true );
    KCalCore::Todo::Ptr t = dndFactory.createDropTodo( data );
    KCalCore::Event::Ptr e = dndFactory.createDropEvent( data );

    if ( t ) {
      // we don't want to change the created todo, but the one which is already
      // stored in our calendar / tree
      TodoTreeNode *ttTodo = findTodo( t->uid() );
      Q_ASSERT( ttTodo ); //TODO if the todo is not found, just insert a new one
      KCalCore::Todo::Ptr todo = CalendarSupport::todo( ttTodo->mTodo );

      KCalCore::Todo::Ptr destTodo;
      if ( parent.isValid() ) {
        TodoTreeNode *node = static_cast<TodoTreeNode *>( parent.internalPointer() );
        if ( node->isValid() ) {
          destTodo = CalendarSupport::todo( node->mTodo );
        }
      }

      KCalCore::Incidence::Ptr tmp = destTodo;
      while ( tmp ) {
        if ( tmp->uid() == todo->uid() ) {
          KMessageBox::information(
            0,
            i18n( "Cannot move to-do to itself or a child of itself." ),
            i18n( "Drop To-do" ), "NoDropTodoOntoItself" );
          return false;
        }
        const QString parentUid = tmp->relatedTo();
        tmp = CalendarSupport::incidence( mCalendar->itemForIncidenceUid( parentUid ) );
      }

      KCalCore::Todo::Ptr oldTodo = KCalCore::Todo::Ptr( todo->clone() );
      // destTodo is empty when we drag a to-do out of a relationship
      todo->setRelatedTo( destTodo ? destTodo->uid() : QString() );
      mChanger->modifyIncidence( ttTodo->mTodo, oldTodo );

      // again, no need to emit dataChanged, that's done by processChange
      return true;

    } else if ( e ) {
      // TODO: Implement dropping an event onto a to-do: Generate a relationship to the event!
    } else {
      if ( !parent.isValid() ) {
        // TODO we should create a new todo with the data in the drop object
        kDebug() << "TODO: Create a new todo with the given data";
        return false;
      }

      TodoTreeNode *node = static_cast<TodoTreeNode *>( parent.internalPointer() );
      KCalCore::Todo::Ptr destTodo = CalendarSupport::todo( node->mTodo );

      if ( data->hasText() ) {
        QString text = data->text();

        KCalCore::Todo::Ptr oldTodo = KCalCore::Todo::Ptr( destTodo->clone() );

        if( text.startsWith( QLatin1String( "file:" ) ) ) {
          destTodo->addAttachment(
            KCalCore::Attachment::Ptr( new KCalCore::Attachment( text ) ) );
        } else {
          QStringList emails = KPIMUtils::splitAddressList( text );
          for ( QStringList::ConstIterator it = emails.constBegin();
                it != emails.constEnd(); ++it ) {
            QString name, email, comment;
            if ( KPIMUtils::splitAddress( *it, name, email, comment ) ==
                 KPIMUtils::AddressOk ) {
              destTodo->addAttendee(
                KCalCore::Attendee::Ptr( new KCalCore::Attendee( name, email ) ) );
            }
          }
        }
        mChanger->modifyIncidence( node->mTodo, oldTodo );
        return true;
      }
    }
  }

  return false;
}

#include "kotodomodel.moc"
