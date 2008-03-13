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
#include <kcal/incidence.h>
#include <kcal/incidenceformatter.h>
#include <kcal/todo.h>

#ifndef KORG_NODND
  #include <kcal/icaldrag.h>
  #include <kcal/vcaldrag.h>
  #include <kcal/dndfactory.h>
#endif

#include <kpimutils/email.h>

#include <kmessagebox.h>
#include <kdebug.h>

#include <QString>
#include <QIcon>
#include <QtAlgorithms>

#ifndef KORG_NODND
  #include <QDrag>
  #include <QMimeData>
#endif

/** This class represents a node in the todo-tree. */
struct KOTodoModel::TodoTreeNode
{
  TodoTreeNode( Todo *todo, TodoTreeNode *parent )
    : mTodo( todo ), mParent( parent ) {}
  /** Recursively delete all TodoTreeNodes which are children of this one. */
  ~TodoTreeNode()
  {
    qDeleteAll( mChildren );
  }

  /** Recursively find a todo.
   *
   * @param todo Pointer to the todo to find. This only finds exactly the
   *             same todo when their pointers are equal.
   * @return Pointer to the TodoTreeNode node which represents the todo
   *         searched for.
   */
  TodoTreeNode *find( const Todo *todo )
  {
    Q_ASSERT( todo );
    if ( mTodo && todo->uid() == mTodo->uid() ) {
      return this;
    }
    Q_FOREACH ( TodoTreeNode *node, mChildren ) {
      TodoTreeNode *tmp = node->find( todo );
      if ( tmp ) {
        return tmp;
      }
    }
    return 0;
  }

  /** Point to the todo this TodoTreeNode node represents.
   *  Note: public field for performance reason, and because this
   *  class is only used internally from KOTodoModel
   */
  Todo *mTodo;
  /** Pointer to the parent TodoTreeNode. Only the root element has no parent. */
  TodoTreeNode *mParent;
  /** List of pointer to the child nodes. */
  QList<TodoTreeNode*> mChildren;
};

KOTodoModel::KOTodoModel( Calendar *cal, QObject *parent )
  : QAbstractItemModel( parent ), mColumnCount( DescriptionColumn + 1 )
{
  mRootNode = new TodoTreeNode( 0, 0 );
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
  reloadTodos();
}

void KOTodoModel::clearTodos()
{
  delete mRootNode;
  mRootNode = new TodoTreeNode( 0, 0 );
  // inform all views that we cleared our internal list
  reset();
}

void KOTodoModel::reloadTodos()
{
  clearTodos();

  Todo::List todoList = mCalendar->todos();
  Todo::List::ConstIterator it;
  for ( it = todoList.begin(); it != todoList.end(); ++it ) {
    if ( !findTodo( *it ) ) {
      insertTodo( *it );
    }
  }
}

void KOTodoModel::processChange( Incidence *incidence, int action )
{
  if ( incidence->type() != "Todo" ) {
    return;
  }

  kDebug() << "incidence->uid() = " << incidence->uid() << " action = " << action;

  Todo *todo = static_cast<Todo *>( incidence );

  if ( action == KOGlobals::INCIDENCEEDITED ) {
    TodoTreeNode *ttTodo = findTodo( todo );
    Q_ASSERT( ttTodo );

    // find the model index of the changed incidence
    QModelIndex miChanged = getModelIndex( ttTodo );

    // get the current parent
    TodoTreeNode *ttOldParent = ttTodo->mParent;

    // get the new parent
    Incidence *inc = todo->relatedTo();
    Todo *newParent = 0;
    if ( inc && inc->type() == "Todo" ) {
      newParent = static_cast<Todo *>( inc );
    }

    // check if the relation to the parent has changed
    if ( !( ( newParent && ttOldParent->mTodo &&
              newParent->uid() == ttOldParent->mTodo->uid() ) ||
            ( newParent == 0 && ttOldParent->mTodo == 0 ) ) ) {
      beginRemoveRows( miChanged.parent(), miChanged.row(), miChanged.row() );
      ttOldParent->mChildren.removeAt( miChanged.row() );
      endRemoveRows();

      // find the node and model index of the new parent
      TodoTreeNode *ttNewParent;
      if ( newParent ) {
        ttNewParent = findTodo( newParent );
      } else {
        ttNewParent = mRootNode;
      }
      QModelIndex miNewParent = getModelIndex( ttNewParent );

      // insert the changed todo
      beginInsertRows( miNewParent, ttNewParent->mChildren.size(),
                                    ttNewParent->mChildren.size() );
      ttNewParent->mChildren.append( ttTodo );
      ttTodo->mParent = ttNewParent;
      endInsertRows();

      QModelIndex miMoved = getModelIndex( ttTodo );

      // force the view to redraw the moved element, because we can't be sure
      // that only the relationship changed
      emit dataChanged( miMoved,
                        miMoved.sibling( miMoved.row(), mColumnCount - 1 ) );
    } else {
      // not the relationship changed, but something else. just force the view
      // to redraw the item.
      emit dataChanged( miChanged,
                        miChanged.sibling( miChanged.row(),
                                           mColumnCount - 1 ) );
    }
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
    Q_ASSERT( ttTodo->mChildren.isEmpty() );

    // find the model index of the deleted incidence
    QModelIndex miDeleted = getModelIndex( ttTodo );

    beginRemoveRows( miDeleted.parent(), miDeleted.row(), miDeleted.row() );
    ttTodo->mParent->mChildren.removeAt( miDeleted.row() );
    delete ttTodo;
    endRemoveRows();
  } else {
    kDebug() << "Not edited, added or deleted, so what is this all about? We just reload...";
    reloadTodos();
  }
}

void KOTodoModel::addTodo( const QString &summary )
{
  if ( !mChanger ) return;

  if ( !summary.trimmed().isEmpty() ) {
    Todo *todo = new Todo();
    todo->setSummary( summary.trimmed() );
    todo->setOrganizer( Person( KOPrefs::instance()->fullName(),
                                KOPrefs::instance()->email() ) );
    if ( !mChanger->addIncidence( todo ) ) {
      KODialogManager::errorSaveIncidence( 0, todo );
      delete todo;
    }
  }
}

void KOTodoModel::copyTodo( const QModelIndex &index, const QDate &date )
{
  if ( !mChanger || !index.isValid() ) return;

  TodoTreeNode *node = static_cast<TodoTreeNode *>( index.internalPointer() );
  Todo *todo = node->mTodo->clone();

  todo->setUid( QString() );

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
  int r = node->mParent->mChildren.indexOf( node );

  return createIndex( r, 0, node );
}

KOTodoModel::TodoTreeNode *KOTodoModel::findTodo( const Todo *todo )
{
  return mRootNode->find( todo );
}

KOTodoModel::TodoTreeNode *KOTodoModel::insertTodo( Todo *todo,
                                                    bool checkRelated )
{
  Incidence *incidence = todo->relatedTo();
  if ( checkRelated && incidence && incidence->type() == "Todo" ) {
    // Use static_cast, checked for type already
    Todo *relatedTodo = static_cast<Todo *>(incidence);

    // check if there are recursively linked todos
    Todo *tmp = relatedTodo;
    while ( tmp ) {
      if ( tmp->uid() == todo->uid() ) {
        // recursion detected, break recursion
        return insertTodo( todo, false );
      }
      incidence = tmp->relatedTo();
      if ( incidence && incidence->type() == "Todo" ) {
        tmp = static_cast<Todo *>(incidence);
      } else {
        tmp = 0;
      }
    }

    // if the parent is not already in the tree, we have to insert it first.
    // necessary because we can't rely on todos coming in a defined order.
    TodoTreeNode *parent = findTodo( relatedTodo );
    if ( !parent ) {
      parent = insertTodo( relatedTodo, checkRelated );
    }

    beginInsertRows( getModelIndex( parent ), parent->mChildren.size(),
                                              parent->mChildren.size() );

    // add the todo under it's parent
    TodoTreeNode *ret = new TodoTreeNode( todo, parent );
    parent->mChildren.append( ret );

    endInsertRows();

    return ret;
  } else {
    beginInsertRows( getModelIndex( mRootNode ), mRootNode->mChildren.size(),
                                                  mRootNode->mChildren.size() );

    // add the todo as root item
    TodoTreeNode *ret = new TodoTreeNode( todo, mRootNode );
    mRootNode->mChildren.append( ret );

    endInsertRows();
    return ret;
  }
}

Qt::ItemFlags KOTodoModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags ret = QAbstractItemModel::flags( index );

  if ( !index.isValid() ) {
    return ret | Qt::ItemIsDropEnabled;
  }

  ret |= Qt::ItemIsDragEnabled;

  Todo *todo = static_cast<TodoTreeNode *>( index.internalPointer() )->mTodo;
  if ( !todo->isReadOnly() ) {
    // the following columns are editable:
    switch ( index.column() ) {
    case SummaryColumn:
    case PriorityColumn:
    case PercentColumn:
    case DueDateColumn:
    case CategoriesColumn:
    case DescriptionColumn:
      ret |= Qt::ItemIsEditable;
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

  Q_ASSERT( row >= 0 );
  Q_ASSERT( column >= 0 && column < mColumnCount );

  if ( !parent.isValid() ) {
    return createIndex( row, column, mRootNode->mChildren[ row ] );
  } else {
    TodoTreeNode *node = static_cast<TodoTreeNode *>( parent.internalPointer() );
    return createIndex( row, column, node->mChildren[ row ] );
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
    return mRootNode->mChildren.size();
  }

  if ( parent.column() == 0 ) {
    // only items in the first column have children
    TodoTreeNode *node = static_cast<TodoTreeNode *>( parent.internalPointer() );
    return node->mChildren.size();
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
  Todo *todo = node->mTodo;

  if ( role == Qt::DisplayRole ) {
    switch ( index.column() ) {
    case SummaryColumn:
      return QVariant( todo->summary() );
    case PriorityColumn:
      if ( todo->priority() == 0 ) {
        return QVariant( QString::fromAscii( "--" ) );
      }
      return QVariant( todo->priority() );
    case PercentColumn:
      return QVariant( todo->percentComplete() );
    case DueDateColumn:
      if ( todo->hasDueDate() ) {
        QString dtStr = todo->dtDueDateStr();
        if ( !todo->allDay() ) {
          dtStr += ' ' + todo->dtDueTimeStr();
        }
        return QVariant( dtStr );
      } else {
        return QVariant();
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
      if ( todo->hasDueDate() ) {
        QString dtStr = todo->dtDueDateStr();
        if ( !todo->allDay() ) {
          dtStr += ' ' + todo->dtDueTimeStr();
        }
        return QVariant( dtStr );
      } else {
        return QVariant();
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

  // set the tooltip for every item
  if ( role == Qt::ToolTipRole ) {
    return QVariant( IncidenceFormatter::toolTipString( todo ) );
  }

  // indicate if a row is checket (=completed) only in the first column
  if ( role == Qt::CheckStateRole && index.column() == 0 ) {
    if ( todo->isCompleted() ) {
      return QVariant( Qt::Checked );
    } else {
      return QVariant( Qt::Unchecked );
    }
  }

  if ( role == Qt::DecorationRole && index.column() == RecurColumn ) {
    if ( todo->recurs() ) {
      return QVariant( QIcon( KOGlobals::self()->smallIcon( "task-recurring" ) ) );
    } else {
      return QVariant();
    }
  }

  if ( role == TodoRole ) {
    QVariant ret( QMetaType::VoidStar );
    ret.setValue( static_cast<void *>( todo ) );
    return ret;
  }

  return QVariant();
}

QVariant KOTodoModel::headerData( int column, Qt::Orientation, int role ) const
{
  Q_ASSERT( column >= 0 && column < mColumnCount );
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
      todo->setCompleted( value.toBool() );
      if ( todo->recurs() ) {
        modified = KOGlobals::COMPLETION_MODIFIED_WITH_RECURRENCE;
      } else {
        modified = KOGlobals::COMPLETION_MODIFIED;
      }
    }

    if ( role == Qt::EditRole ) {
      switch ( index.column() ) {
        case SummaryColumn:
          todo->setSummary( value.toString() );
          modified = KOGlobals::SUMMARY_MODIFIED;
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
          //TODO
          break;
        case CategoriesColumn:
          //TODO
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
  return Qt::CopyAction | Qt::MoveAction;
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
