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
#include "koprefs.h"
#include "koglobals.h"
#include "kodialogmanager.h"
#include "korganizer/incidencechangerbase.h"

#include <kcal/calendar.h>
#include <kcal/calformat.h>
#include <kcal/incidence.h>
#include <kcal/incidenceformatter.h>
#include <kcal/todo.h>

#ifndef KORG_NODND
  #include <kcal/icaldrag.h>
  #include <kcal/vcaldrag.h>
  #include <kcal/dndfactory.h>
#endif

#include <kpimutils/email.h>

#include <KMessageBox>
#include <KDebug>

#include <QString>
#include <QIcon>
#include <QHash>
#include <QList>
#include <QtAlgorithms>

#ifndef KORG_NODND
  #include <QDrag>
  #include <QMimeData>
#endif

/** This class represents a node in the todo-tree. */
struct KOTodoModel::TodoTreeNode : QObject
{
  TodoTreeNode( Todo *todo, TodoTreeNode *parent, KOTodoModel *model )
    : mTodo( todo ), mParent( parent ), mParentListPos( 0 ),
      mToDelete( false ), mModel( model )
  {
    if ( mTodo ) {
      mUid = mTodo->uid();
      mModel->mTodoHash[ mUid ] = this;
    }
  }

  /** Recursively delete all TodoTreeNodes which are children of this one. */
  ~TodoTreeNode()
  {
    if ( mTodo ) {
      mModel->mTodoHash.remove( mUid );
    } else {
      // root node gets deleted, clear the whole hash
      mModel->mTodoHash.clear();
    }
    qDeleteAll( mChildren );
  }

  /** Checks if it's save to access the todo-pointer of this node. */
  bool isValid()
  {
    return mTodo && !mToDelete;
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
#ifndef NDEBUG
      // all sub-todos should be marked for delete too, otherwise there is
      // something wrong
      QList<TodoTreeNode*> toCheck;
      toCheck << mChildren;
      while ( !toCheck.isEmpty() ) {
        TodoTreeNode *node = toCheck.takeFirst();
        Q_ASSERT ( node->mToDelete || !node->mTodo->relatedTo() );
        toCheck << node->mChildren;
      }
#endif

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
  Todo *mTodo;
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
    QString mUid;
};

KOTodoModel::KOTodoModel( Calendar *cal, QObject *parent )
  : QAbstractItemModel( parent ), mColumnCount( DescriptionColumn + 1 )
{
  mRootNode = new TodoTreeNode( 0, 0, this );
  mFlatView = false;
  setCalendar( cal );

#ifndef KORG_NODND
  mDndFactory = new DndFactory( cal );
#endif
}

KOTodoModel::~KOTodoModel()
{
  delete mRootNode;

#ifndef KORG_NODND
  delete mDndFactory;
#endif
}

void KOTodoModel::setCalendar( Calendar *cal )
{
  mCalendar = cal;
  // old todos might no longer be valid, so clear them
  clearTodos();
  reloadTodos();
}

void KOTodoModel::clearTodos()
{
  delete mRootNode;
  mRootNode = new TodoTreeNode( 0, 0, this );
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

  Todo::List todoList = mCalendar->todos();
  Todo::List::ConstIterator it;
  QList<TodoTreeNode*> changedNodes;
  for ( it = todoList.begin(); it != todoList.end(); ++it ) {
    TodoTreeNode *tmp = findTodo( *it );
    if ( !tmp ) {
      insertTodo( *it );
    } else {
      // update pointer to the todo
      // apparently this is necessary because undo's and redo's don't modify
      // the modified todos but replace pointers to them with others
      // TODO check if that's true, and if this is OK
      tmp->mTodo = *it;

      // the todo is still in the calendar, we don't delete it
      tmp->mToDelete = false;

      changedNodes << tmp;
    }
  }

  // delete all TodoTreeNodes which are still marked for deletion
  mRootNode->deleteMarked();

  // move todos if they changed their place in the hirarchy
  Q_FOREACH ( TodoTreeNode *tmp, changedNodes ) {
    const QModelIndex miChanged = moveIfParentChanged( tmp, tmp->mTodo, true );
    emit dataChanged( miChanged, miChanged.sibling( miChanged.row(), mColumnCount - 1 ) );
  }
}

void KOTodoModel::processChange( Incidence *incidence, int action )
{
  if ( incidence->type() != "Todo" ) {
    return;
  }

  Todo *todo = static_cast<Todo *>( incidence );

  if ( action == KOGlobals::INCIDENCEEDITED ) {
    TodoTreeNode *ttTodo = findTodo( todo );
    Q_ASSERT( ttTodo );

    QModelIndex miChanged = moveIfParentChanged( ttTodo, todo, false );

    // force the view to redraw the element even if the parent relationship
    // changed, because we can't be sure that only the relationship changed
    emit dataChanged( miChanged,
                      miChanged.sibling( miChanged.row(), mColumnCount - 1 ) );
  } else if ( action == KOGlobals::INCIDENCEADDED ) {
    // the todo should not be in our tree...
    Q_ASSERT( !findTodo( todo ) );

    insertTodo( todo );
  } else if ( action == KOGlobals::INCIDENCEDELETED ) {
    TodoTreeNode *ttTodo = findTodo( todo );
    // we can only delete todo's which are in the tree
    Q_ASSERT( ttTodo );
    // somebody should assure that all todo's which relate to this one
    // are un-linked before deleting this one
    Q_ASSERT( !ttTodo->hasChildren() );

    // find the model index of the deleted incidence
    QModelIndex miDeleted = getModelIndex( ttTodo );

    beginRemoveRows( miDeleted.parent(), miDeleted.row(), miDeleted.row() );
    ttTodo->mParent->removeChild( miDeleted.row() );
    delete ttTodo;
    endRemoveRows();
  } else {
    kDebug() << "Not edited, added or deleted, so what is this all about? We just reload...";
    reloadTodos();
  }
}

QModelIndex KOTodoModel::addTodo( const QString &summary,
                                  const QModelIndex &parent )
{
  if ( !mChanger ) {
    return QModelIndex();
  }

  if ( !summary.trimmed().isEmpty() ) {
    Todo *todo = new Todo();
    todo->setSummary( summary.trimmed() );
    todo->setOrganizer( Person( KOPrefs::instance()->fullName(),
                                KOPrefs::instance()->email() ) );
    if ( parent.isValid() ) {
      todo->setRelatedTo(
              static_cast<TodoTreeNode *>( parent.internalPointer() )->mTodo );
    }

    if ( !mChanger->addIncidence( todo ) ) {
      KODialogManager::errorSaveIncidence( 0, todo );
      delete todo;
      return QModelIndex();
    }

    return getModelIndex( findTodo( todo ) );
  }

  return QModelIndex();
}

void KOTodoModel::copyTodo( const QModelIndex &index, const QDate &date )
{
  if ( !mChanger || !index.isValid() ) {
    return;
  }

  TodoTreeNode *node = static_cast<TodoTreeNode *>( index.internalPointer() );
  Todo *todo = node->mTodo->clone();

  todo->setUid( CalFormat::createUniqueId() );

  KDateTime due = todo->dtDue();
  due.setDate( date );
  todo->setDtDue( due );

  if ( !mChanger->addIncidence( todo ) ) {
    KODialogManager::errorSaveIncidence( 0, todo );
    delete todo;
  }
}

QModelIndex KOTodoModel::getModelIndex( TodoTreeNode *node ) const
{
  Q_ASSERT( node );
  if ( node == mRootNode ) {
    return QModelIndex();
  }

  return createIndex( node->mParentListPos, 0, node );
}

QModelIndex KOTodoModel::moveIfParentChanged( TodoTreeNode *curNode, Todo *todo,
                                              bool addParentIfMissing )
{
  // find the model index of the changed incidence
  QModelIndex miChanged = getModelIndex( curNode );

  // get the current parent
  TodoTreeNode *ttOldParent = curNode->mParent;

  // get the new parent
  Todo *newParent = 0;

  if ( !mFlatView ) {
    // in flat view, no todo has a parent

    if ( !isInHierarchyLoop( todo ) ) {
      Incidence *inc = todo->relatedTo();
      if ( inc && inc->type() == "Todo" ) {
        newParent = static_cast<Todo *>( inc );
      }
    }
  }

  // check if the relation to the parent has changed
  if ( ( newParent == 0 && ttOldParent->mTodo != 0 ) ||
       ( newParent != 0 && ttOldParent->mTodo == 0 ) ||
       ( newParent != 0 && ttOldParent->mTodo != 0 &&
         newParent->uid() != ttOldParent->mTodo->uid() ) ) {
    // find the node and model index of the new parent
    TodoTreeNode *ttNewParent = 0;
    if ( newParent ) {
      ttNewParent = findTodo( newParent );
      if ( !ttNewParent && addParentIfMissing ) {
        ttNewParent = insertTodo( newParent );
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

KOTodoModel::TodoTreeNode *KOTodoModel::findTodo( const Todo *todo )
{
  Q_ASSERT( todo );
  return mTodoHash.value( todo->uid() );
}

bool KOTodoModel::isInHierarchyLoop( const Todo *todo ) const {

  if ( !todo ) {
    return false;
  }

  Incidence *i = todo->relatedTo();
  QList<Incidence *> processedParents;

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
        i = i->relatedTo();
      } else {
        // There's a loop but this to-do isn't in it
        // the loop is at a higher level, e.g:
        // t1->t2->t3->t4->t3->t4->t3->t4->t3..
        // t1 and t2 can be added as sons of t3 without
        // danger of inifinit looping
        return false;
      }
    }
  }

  return false;
}

KOTodoModel::TodoTreeNode *KOTodoModel::insertTodo( Todo *todo,
                                                    bool checkRelated )
{
  Incidence *incidence = todo->relatedTo();
  if ( !mFlatView && checkRelated &&
       incidence && incidence->type() == "Todo" ) {
    // Use static_cast, checked for type already
    Todo *relatedTodo = static_cast<Todo *>(incidence);

    // check if there are recursively linked todos

    if ( isInHierarchyLoop( todo ) ) {
      // recursion detected, break recursion
      return insertTodo( todo, false );
    }

    // if the parent is not already in the tree, we have to insert it first.
    // necessary because we can't rely on todos coming in a defined order.
    TodoTreeNode *parent = findTodo( relatedTodo );
    if ( !parent ) {
      parent = insertTodo( relatedTodo, checkRelated );
    }

    beginInsertRows( getModelIndex( parent ), parent->childrenCount(),
                                              parent->childrenCount() );

    // add the todo under it's parent
    TodoTreeNode *ret = new TodoTreeNode( todo, parent, this );
    parent->addChild( ret );

    endInsertRows();

    return ret;
  } else {
    beginInsertRows( getModelIndex( mRootNode ), mRootNode->childrenCount(),
                                                 mRootNode->childrenCount() );

    // add the todo as root item
    TodoTreeNode *ret = new TodoTreeNode( todo, mRootNode, this );
    mRootNode->addChild( ret );

    endInsertRows();
    return ret;
  }
}

void KOTodoModel::setFlatView( bool flatView )
{
  if ( mFlatView == flatView ) {
    return;
  }

  mFlatView = flatView;
  reloadTodos();
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

  Todo *todo = node->mTodo;

  if ( !todo->isReadOnly() ) {
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
    return createIndex( row, column, node->childAt( row ) );
  }
}

QModelIndex KOTodoModel::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() ) {
    return QModelIndex();
  }

  TodoTreeNode *node = static_cast<TodoTreeNode *>( child.internalPointer() );
  // every model index given out by us has a valid internal pointer, so
  // that's save:
  TodoTreeNode *parent = node->mParent;

  return getModelIndex( parent );
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
    return node->childrenCount();
  } else {
    return 0;
  }
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

  Todo *todo = node->mTodo;

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
        return QVariant( todo->dtDueStr() );
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
    }
    return QVariant();
  }

  // set the tooltip for every item
  if ( role == Qt::ToolTipRole ) {
    if ( KOPrefs::instance()->enableToolTips() ) {
      return QVariant( IncidenceFormatter::toolTipString( todo, true ) );
    } else {
      return QVariant();
    }
  }

  // background colour for todos due today or overdue todos
  if ( role == Qt::BackgroundRole ) {
    if ( todo->isOverdue() ) {
      return QVariant(
        QBrush( KOPrefs::instance()->agendaCalendarItemsToDosOverdueBackgroundColor() ) );
    } else if ( !todo->isCompleted() && todo->dtDue().date() == QDate::currentDate() ) {
      return QVariant(
        QBrush( KOPrefs::instance()->agendaCalendarItemsToDosDueTodayBackgroundColor() ) );
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

  // item for recurring todos
  if ( role == Qt::DecorationRole && index.column() == RecurColumn ) {
    if ( todo->recurs() ) {
      return QVariant( QIcon( KOGlobals::self()->smallIcon( "task-recurring" ) ) );
    }
  }

  // category colour
  if ( role == Qt::DecorationRole && index.column() == SummaryColumn ) {
    QStringList categories = todo->categories();
    if ( !categories.isEmpty() ) {
      return QVariant( KOPrefs::instance()->categoryColor( categories.first() ) );
    }
  }

  if ( role == TodoRole ) {
    QVariant ret( QMetaType::VoidStar );
    ret.setValue( static_cast<void *>( todo ) );
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
    }
  }

  if ( role == Qt::TextAlignmentRole ) {
    switch ( column ) {
      case RecurColumn:
      case PriorityColumn:
      case PercentColumn:
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

  Todo *todo = static_cast<TodoTreeNode *>( index.internalPointer() )->mTodo;

  if ( !todo->isReadOnly() && mChanger->beginChange( todo ) ) {
    Todo *oldTodo = todo->clone();
    int modified = KOGlobals::UNKNOWN_MODIFIED;

    if ( role == Qt::CheckStateRole && index.column() == 0 ) {
      todo->setCompleted( static_cast<Qt::CheckState>( value.toInt() ) == Qt::Checked ?
                          true : false );
      if ( todo->recurs() ) {
        modified = KOGlobals::COMPLETION_MODIFIED_WITH_RECURRENCE;
      } else {
        modified = KOGlobals::COMPLETION_MODIFIED;
      }
    }

    if ( role == Qt::EditRole ) {
      switch ( index.column() ) {
        case SummaryColumn:
          if ( !value.toString().isEmpty() ) {
            todo->setSummary( value.toString() );
            modified = KOGlobals::SUMMARY_MODIFIED;
          }
          break;
        case PriorityColumn:
          todo->setPriority( value.toInt() );
          modified = KOGlobals::PRIORITY_MODIFIED;
          break;
        case PercentColumn:
          todo->setPercentComplete( value.toInt() );
          modified = KOGlobals::COMPLETION_MODIFIED;
          break;
        case DueDateColumn:
          {
            KDateTime tmp = todo->dtDue();
            tmp.setDate( value.toDate() );
            todo->setDtDue( tmp );
            todo->setHasDueDate( value.toDate().isValid() );
            modified = KOGlobals::DATE_MODIFIED;
          }
          break;
        case CategoriesColumn:
          todo->setCategories( value.toStringList() );
          modified = KOGlobals::CATEGORY_MODIFIED;
          break;
        case DescriptionColumn:
          todo->setDescription( value.toString() );
          modified = KOGlobals::DESCRIPTION_MODIFIED;
          break;
      }
    }

    if ( modified != KOGlobals::UNKNOWN_MODIFIED ) {
      mChanger->changeIncidence( oldTodo, todo, modified );
      // changeIncidence will eventually call the view's
      // changeIncidenceDisplay method, which in turn
      // will call processChange. processChange will then emit
      // dataChanged to the view, so we don't have to
      // do it here
    }
    mChanger->endChange( todo );
    delete oldTodo;

    return true;
  } else {
    KODialogManager::errorSaveIncidence( 0, todo );
    return false;
  }
}

#ifndef KORG_NODND
Qt::DropActions KOTodoModel::supportedDropActions() const
{
  // Qt::CopyAction not supported yet
  return Qt::MoveAction;
}

QStringList KOTodoModel::mimeTypes() const
{
  QStringList ret;

  ret << ICalDrag::mimeType() << VCalDrag::mimeType();

  return ret;
}

QMimeData *KOTodoModel::mimeData( const QModelIndexList &indexes ) const
{
  Q_FOREACH ( const QModelIndex &index, indexes ) {
    Todo *todo = static_cast<TodoTreeNode *>( index.internalPointer() )->mTodo;

    return mDndFactory->createMimeData( todo );

   //TODO implement dragging of more than one element
  }

  return new QMimeData();
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
       ( ICalDrag::canDecode( data ) || VCalDrag::canDecode( data ) ) ) {
    Todo *todo = mDndFactory->createDropTodo( data );
    Event *event = mDndFactory->createDropEvent( data );

    if ( todo ) {
      // we don't want to change the created todo, but the one which is already
      // stored in our calendar / tree
      TodoTreeNode *ttTodo = findTodo( todo );
      Q_ASSERT( ttTodo ); //TODO if the todo is not found, just insert a new one
      delete todo;
      todo = ttTodo->mTodo;

      Todo *destTodo = 0;
      if ( parent.isValid() ) {
        destTodo = static_cast<TodoTreeNode *>( parent.internalPointer() )->mTodo;
      }

      Incidence *tmp = destTodo;
      while ( tmp ) {
        if ( *tmp == *todo ) {
          KMessageBox::information(
            0,
            i18n( "Cannot move to-do to itself or a child of itself." ),
            i18n( "Drop To-do" ), "NoDropTodoOntoItself" );
          return false;
        }
        tmp = tmp->relatedTo();
      }

      if ( mChanger->beginChange( todo ) ) {
        Todo *oldTodo = todo->clone();
        todo->setRelatedTo( destTodo );
        mChanger->changeIncidence( oldTodo, todo, KOGlobals::RELATION_MODIFIED );
        mChanger->endChange( todo );
        // again, no need to emit dataChanged, that's done by processChange

        delete oldTodo;
        return true;
      } else {
        KODialogManager::errorSaveIncidence( 0, todo );
        return false;
      }
    } else if ( event ) {
      // TODO: Implement dropping an event onto a to-do: Generate a relationship to the event!
    } else {
      if ( !parent.isValid() ) {
        // TODO we should create a new todo with the data in the drop object
        kDebug() << "TODO: Create a new todo with the given data";
        return false;
      }

      Todo *destTodo = static_cast<TodoTreeNode *>( parent.internalPointer() )->mTodo;

      if ( data->hasText() ) {
        QString text = data->text();

        if( mChanger->beginChange( destTodo ) ) {
          Todo *oldTodo = destTodo->clone();

          if( text.startsWith( "file:" ) ) {
            destTodo->addAttachment( new Attachment( text ) );
          } else {
            QStringList emails = KPIMUtils::splitAddressList( text );
            for ( QStringList::ConstIterator it = emails.begin();
                  it != emails.end(); ++it ) {
              QString name, email, comment;
              if ( KPIMUtils::splitAddress( *it, name, email, comment ) ==
                   KPIMUtils::AddressOk ) {
                destTodo->addAttendee( new Attendee( name, email ) );
              }
            }
          }
          mChanger->changeIncidence( oldTodo, destTodo );
          mChanger->endChange( destTodo );

          delete oldTodo;
          return true;
        } else {
          KODialogManager::errorSaveIncidence( 0, destTodo );
          return false;
        }
      }
    }
  } else {
    kDebug() << "Drop was not decodable";
    return false;
  }

  return false;
}
#endif /*KORG_NODND*/

#include "kotodomodel.moc"
