/*
  Copyright (c) 2012 Sérgio Martins <iamsergio@gmail.com>

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

#include "incidencetreemodel_p.h"

#include <Akonadi/EntityTreeModel>
#include <KCalCore/Incidence>
#include <QElapsedTimer>

using namespace Akonadi;

IncidenceTreeModel::Private::Private( IncidenceTreeModel *qq,
                                      const QStringList &mimeTypes ) : QObject()
                                                                     , m_mimeTypes( mimeTypes )
                                                                     , q( qq )
                                                                     
{
}

QModelIndex IncidenceTreeModel::Private::indexForNode( const Node::Ptr &node ) const
{
  if ( !node )
    return QModelIndex();

  const int row = node->parentNode ? node->parentNode->directChilds.indexOf( node )
                                   : m_toplevelNodeList.indexOf( node );

  Q_ASSERT( row != -1 );
  return q->createIndex( row, 0, node.data() );
}

void IncidenceTreeModel::Private::reset( bool silent )
{
  if ( !silent )
    q->beginResetModel();
  m_toplevelNodeList.clear();
  m_nodeMap.clear();
  m_waitingForParent.clear();
  m_uidMap.clear();
  if ( q->sourceModel() ) {
    const int sourceCount = q->sourceModel()->rowCount();
    for ( int i=0; i<sourceCount; ++i ) {
      const QModelIndex sourceIndex = q->sourceModel()->index( i, 0, QModelIndex() );
      insertNode( sourceIndex, /**silent=*/true );
    }
  }
  if ( !silent )
    q->endResetModel();
}

void IncidenceTreeModel::Private::onHeaderDataChanged( Qt::Orientation orientation, int first, int last )
{
  emit q->headerDataChanged( orientation, first, last );
}

void IncidenceTreeModel::Private::onDataChanged( const QModelIndex &begin, const QModelIndex &end )
{
  Q_ASSERT( begin.isValid() );
  Q_ASSERT( end.isValid() );
  Q_ASSERT( q->sourceModel() );
  Q_ASSERT( !begin.parent().isValid() );
  Q_ASSERT( !end.parent().isValid() );
  Q_ASSERT( begin.row() <= end.row() );
  const int first_row = begin.row();
  const int last_row = end.row();

  for( int i=first_row; i<=last_row; ++i ) {
    QModelIndex sourceIndex = q->sourceModel()->index( i, 0 );
    Q_ASSERT( sourceIndex.isValid() );
    QModelIndex index = q->mapFromSource( sourceIndex );
    Q_ASSERT( index.isValid() );
    emit q->dataChanged( index, index );
  }
}

void IncidenceTreeModel::Private::onRowsAboutToBeInserted( const QModelIndex &parent, int, int )
{
  // We are a reparenting proxy, the source proxy is flat
  Q_ASSERT( !parent.isValid() );
  // Nothing to do yet. We don't know if all the new incidences in this range belong to the same
  // parent yet.
}

void IncidenceTreeModel::Private::onRowsInserted( const QModelIndex &parent, int begin, int end )
{
  //QElapsedTimer timer;
  //timer.start();
  Q_ASSERT( !parent.isValid() );
  Q_ASSERT( begin <= end );
  // TODO: Performance optimization: Order them by increasing depth, i.e: insert
  // the parents first.
  for ( int i=begin; i<=end; ++i ) {
    const QModelIndex sourceIndex = q->sourceModel()->index( i, 0, QModelIndex() );
    insertNode( sourceIndex );
  }
  //kDebug() << "Took " << timer.elapsed() << " to insert " << end-begin+1;
}

void IncidenceTreeModel::Private::insertNode( const QModelIndex &sourceIndex, bool silent )
{
  Q_ASSERT( sourceIndex.isValid() );
  Q_ASSERT( sourceIndex.model() == q->sourceModel() );
  const Akonadi::Item item = sourceIndex.data( EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  Q_ASSERT( item.isValid() );
  KCalCore::Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>();
  Q_ASSERT( incidence );

  // if m_mimeTypes is empty, we ignore this feature
  if ( !m_mimeTypes.isEmpty() && !m_mimeTypes.contains( incidence->mimeType() ) )
    return;

  Node::Ptr node( new Node() );
  node->sourceIndex = sourceIndex;
  node->id = item.id();
  node->uid = incidence->uid();
  //kDebug() << "New node " << node.data() << node->uid << node->id;
  node->parentUid = incidence->relatedTo();
  Q_ASSERT( node->uid != node->parentUid );
  Q_ASSERT( !m_uidMap.contains( node->uid ) );
  Q_ASSERT( !m_nodeMap.contains( node->id ) );
  m_uidMap.insert( node->uid, node );
  m_nodeMap.insert( item.id(), node );

  int rowToUse = -1;
  bool mustInsertIntoParent = false;

  const bool hasParent = !node->parentUid.isEmpty();
  if ( hasParent ) {
    // We have a parent, did he arrive yet ?
    if ( m_uidMap.contains( node->parentUid ) ) {
      node->parentNode = m_uidMap.value( node->parentUid );

      // We can only insert after beginInsertRows(), because it affects rowCounts
      mustInsertIntoParent = true;
      rowToUse = node->parentNode->directChilds.count();
    } else {
      // Parent unknown, we are orphan for now
      Q_ASSERT( !m_waitingForParent.contains( node->parentUid, node ) );
      m_waitingForParent.insert( node->parentUid, node );
    }
  }

  if ( !node->parentNode ) {
    rowToUse = m_toplevelNodeList.count();
  }

  // Lets insert the row:
  const QModelIndex &parent = indexForNode( node->parentNode );
  if ( !silent )
    q->beginInsertRows( parent, rowToUse, rowToUse );

  if ( !node->parentNode ) {
    m_toplevelNodeList.append( node );
  }

  if ( mustInsertIntoParent ) {
    node->parentNode->directChilds.append( node );
  }

  if ( !silent )
    q->endInsertRows();

  // Are we a parent?
  if ( m_waitingForParent.contains( node->uid ) ) {
    Q_ASSERT( m_waitingForParent.count( node->uid ) > 0 );
    QList<Node::Ptr> childs = m_waitingForParent.values( node->uid );
    m_waitingForParent.remove( node->uid );
    Q_ASSERT( !childs.isEmpty() );
    const QModelIndex fromParent = QModelIndex();

    foreach( const Node::Ptr &child, childs ) {
      const int fromRow = m_toplevelNodeList.indexOf( child );
      Q_ASSERT( fromRow != -1 );
      const QModelIndex toParent = indexForNode( node );
      Q_ASSERT( toParent.isValid() );
      Q_ASSERT( toParent.model() == q );
      const int toRow = node->directChilds.count();

      if ( !silent ) {
        const bool res = q->beginMoveRows( /**fromParent*/QModelIndex(), fromRow,
                                           fromRow, toParent, toRow );
        //emit q->layoutAboutToBeChanged();
        Q_ASSERT( res );
      }
      child->parentNode = node;
      node->directChilds.append( child );
      m_toplevelNodeList.remove( fromRow );

      if ( !silent ) {
        q->endMoveRows();
        //emit q->layoutChanged();
      }
    }
  }
}

void IncidenceTreeModel::Private::onRowsAboutToBeRemoved( const QModelIndex &parent, int begin, int end )
{
  //QElapsedTimer timer;
  //timer.start();
  Q_ASSERT( !parent.isValid() );
  Q_ASSERT( begin <= end );
  for ( int i=begin; i<=end; ++i ) {
    QModelIndex sourceIndex = q->sourceModel()->index( i, 0, QModelIndex() );
    Q_ASSERT( sourceIndex.isValid() );
    Q_ASSERT( sourceIndex.model() == q->sourceModel() );
    const Akonadi::Item::Id id = sourceIndex.data( EntityTreeModel::ItemIdRole ).toLongLong();
    Q_ASSERT( id != -1 );
    // Go ahead and remove it now. We don't do it in ::onRowsRemoved(), because
    // while unparenting childs with moveRows() the view might call data() on the
    // item that is already removed from ETM.
    removeNode( id );
  }

  m_removedNodes.clear();
  //kDebug() << "Took " << timer.elapsed() << " to remove " << end-begin+1;
}

void IncidenceTreeModel::Private::removeNode( Akonadi::Item::Id id )
{
  Q_ASSERT( id != -1 );

  if ( !m_nodeMap.contains( id ) ) {
    // We don't know about this one because we're ignoring it's mime type.
    Q_ASSERT( m_mimeTypes.count() != 3 );
    return;
  }

  Node::Ptr node = m_nodeMap.value( id );
  Q_ASSERT( node );
  Q_ASSERT( node->id == id );

  //kDebug() << "Dealing with parent: " << node->id << node.data()
  //         << node->uid << node->directChilds.count() << indexForNode( node );

  // First, unparent the children
  if ( !node->directChilds.isEmpty() ) {
    Node::List childs = node->directChilds;
    const QModelIndex fromParent = indexForNode( node );
    Q_ASSERT( fromParent.isValid() );
    const int firstSourceRow = 0;
    const int lastSourceRow  = node->directChilds.count() - 1;
    const int toRow = m_toplevelNodeList.count();
    q->beginMoveRows( fromParent, firstSourceRow, lastSourceRow,
                      /**toParent is root*/QModelIndex(), toRow );
    node->directChilds.clear();
    foreach( const Node::Ptr &child, childs ) {
      //kDebug() << "Dealing with child: " << child.data() << child->uid;
      m_toplevelNodeList.append( child );
      child->parentNode = Node::Ptr();
      m_waitingForParent.insert( node->uid, child );
    }
    q->endMoveRows();
  }

  const QModelIndex parent = indexForNode( node->parentNode );

  const int rowToRemove =  node->parentNode ? node->parentNode->directChilds.indexOf( node )
                                            : m_toplevelNodeList.indexOf( node );

  Q_ASSERT ( rowToRemove != -1 );
  // Now remove the row
  q->beginRemoveRows( parent, rowToRemove, rowToRemove );

  if ( parent.isValid() ) {
    node->parentNode->directChilds.remove( rowToRemove );
    node->parentNode = Node::Ptr();
  } else {
    m_toplevelNodeList.remove( rowToRemove );
  }

  if ( !node->parentUid.isEmpty() ) {
    m_waitingForParent.remove( node->parentUid, node );
  }

  m_uidMap.remove( node->uid );
  m_nodeMap.remove( node->id );

  q->endRemoveRows();
  m_removedNodes << node.data();
}

void IncidenceTreeModel::Private::onRowsRemoved( const QModelIndex &parent, int begin, int end )
{
  Q_UNUSED( parent );
  Q_UNUSED( begin );
  Q_UNUSED( end );
  // Nothing to do here, see comment on ::onRowsAboutToBeRemoved()
}

void IncidenceTreeModel::Private::onModelAboutToBeReset()
{
  q->beginResetModel();
}

void IncidenceTreeModel::Private::onModelReset()
{
  reset( /**silent=*/false );
  q->endResetModel();
}

void IncidenceTreeModel::Private::onLayoutAboutToBeChanged()
{
  Q_ASSERT( q->persistentIndexList().isEmpty() );
  emit q->layoutAboutToBeChanged();
}

void IncidenceTreeModel::Private::onLayoutChanged()
{
  reset( /**silent=*/true );
  Q_ASSERT( q->persistentIndexList().isEmpty() );
  emit q->layoutChanged();
}

void IncidenceTreeModel::Private::onRowsMoved( const QModelIndex &, int, int, const QModelIndex &, int )
{
  // Not implemented yet
  Q_ASSERT( false );
}

IncidenceTreeModel::IncidenceTreeModel( QObject *parent ) : QAbstractProxyModel( parent )
                                                          , d( new Private( this, QStringList() ) )
{
  setObjectName( "IncidenceTreeModel" );
}

IncidenceTreeModel::IncidenceTreeModel( const QStringList &mimeTypes,
                                        QObject *parent ) : QAbstractProxyModel( parent )
                                                          , d( new Private( this, mimeTypes ) )
{
  setObjectName( "IncidenceTreeModel" );
}

IncidenceTreeModel::~IncidenceTreeModel()
{
  delete d;
}

QVariant IncidenceTreeModel::data( const QModelIndex &index, int role ) const
{
  Q_ASSERT( index.isValid() );
  if ( !index.isValid() || !sourceModel() ) {
    return QVariant();
  }

  QModelIndex sourceIndex = mapToSource( index );
  Q_ASSERT( sourceIndex.isValid() );

  return sourceModel()->data( sourceIndex, role );
}

int IncidenceTreeModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() ) {
    Q_ASSERT( parent.model() == this );
    Node *parentNode = reinterpret_cast<Node*>( parent.internalPointer() );
    Q_ASSERT( parentNode );
    Q_ASSERT( !d->m_removedNodes.contains( parentNode ) );
    const int count = parentNode->directChilds.count();
    return count;
  }

  return d->m_toplevelNodeList.count();
}

int IncidenceTreeModel::columnCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    Q_ASSERT( parent.model() == this );

  return sourceModel() ? sourceModel()->columnCount() : 1;
}

void IncidenceTreeModel::setSourceModel( QAbstractItemModel *model )
{
  if ( model == sourceModel() )
    return;

  beginResetModel();

  if ( sourceModel() ) {
    disconnect( sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                d, SLOT(onDataChanged(QModelIndex,QModelIndex)) );

    disconnect( sourceModel(), SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
                d, SLOT(onHeaderDataChanged(Qt::Orientation,int,int)) );

    disconnect( sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
                d, SLOT(onRowsInserted(QModelIndex,int,int)) );

    disconnect( sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
                d, SLOT(onRowsRemoved(QModelIndex,int,int)) );

    disconnect( sourceModel(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
                d, SLOT(onRowsMoved(QModelIndex,int,int,QModelIndex,int)) );

    disconnect( sourceModel(), SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
                d, SLOT(onRowsAboutToBeInserted(QModelIndex,int,int)) );

    disconnect( sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
                d, SLOT(onRowsAboutToBeRemoved(QModelIndex,int,int)) );

    disconnect( sourceModel(), SIGNAL(modelAboutToBeReset()),
                d, SLOT(onModelAboutToBeReset()) );

    disconnect( sourceModel(), SIGNAL(modelReset()),
                d, SLOT(onModelReset()) );

    disconnect( sourceModel(), SIGNAL(layoutAboutToBeChanged()),
                d, SLOT(onLayoutAboutToBeChanged()) );

    disconnect( sourceModel(), SIGNAL(layoutChanged()),
                d, SLOT(onLayoutChanged()) );
  }

  QAbstractProxyModel::setSourceModel( model );

  if ( sourceModel() ) {
    connect( sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             d, SLOT(onDataChanged(QModelIndex,QModelIndex)) );

    connect( sourceModel(), SIGNAL(headerDataChanged(Qt::Orientation,int,int)),
             d, SLOT(onHeaderDataChanged(Qt::Orientation,int,int)) );

    connect( sourceModel(), SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
             d, SLOT(onRowsAboutToBeInserted(QModelIndex,int,int)) );

    connect( sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)),
             d, SLOT(onRowsInserted(QModelIndex,int,int)) );

    connect( sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
             d, SLOT(onRowsAboutToBeRemoved(QModelIndex,int,int)) );

    connect( sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
             d, SLOT(onRowsRemoved(QModelIndex,int,int)) );

    connect( sourceModel(), SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
             d, SLOT(onRowsMoved(QModelIndex,int,int,QModelIndex,int)) );

    connect( sourceModel(), SIGNAL(modelAboutToBeReset()),
             d, SLOT(onModelAboutToBeReset()) );

    connect( sourceModel(), SIGNAL(modelReset()),
             d, SLOT(onModelReset()) );

    connect( sourceModel(), SIGNAL(layoutAboutToBeChanged()),
             d, SLOT(onLayoutAboutToBeChanged()) );

    connect( sourceModel(), SIGNAL(layoutChanged()),
             d, SLOT(onLayoutChanged()) );
  }

  d->reset( /**silent=*/true );
  endResetModel();
}

QModelIndex IncidenceTreeModel::mapFromSource( const QModelIndex &sourceIndex ) const
{
  if ( !sourceIndex.isValid() ) {
    kWarning() << "IncidenceTreeModel::mapFromSource() source index is invalid";
    // Q_ASSERT( false );
    return QModelIndex();
  }

  if ( !sourceModel() )
    return QModelIndex();

  Q_ASSERT( sourceIndex.column() < sourceModel()->columnCount() );
  Q_ASSERT( sourceModel() == sourceIndex.model() );
  const Akonadi::Item::Id id = sourceIndex.data( Akonadi::EntityTreeModel::ItemIdRole ).toLongLong();

  if ( id == -1 || !d->m_nodeMap.contains( id ) )
    return QModelIndex();

  const Node::Ptr node = d->m_nodeMap.value( id );
  Q_ASSERT( node );

  return d->indexForNode( node );
}

QModelIndex IncidenceTreeModel::mapToSource( const QModelIndex &proxyIndex ) const
{
  if ( !proxyIndex.isValid() || !sourceModel() ) {
    return QModelIndex();
  }

  Q_ASSERT( proxyIndex.column() < columnCount() );
  Q_ASSERT( proxyIndex.internalPointer() );
  Q_ASSERT( proxyIndex.model() == this );
  Node *node = reinterpret_cast<Node*>( proxyIndex.internalPointer() );
  //QModelIndexList indexes = EntityTreeModel::modelIndexesForItem( sourceModel(), Akonadi::Item( node->id ) );
  /*if ( indexes.isEmpty() ) {
    Q_ASSERT( sourceModel() );
    kError() << "IncidenceTreeModel::mapToSource() no indexes."
             << proxyIndex << node->id << "; source.rowCount() = "
             << sourceModel()->rowCount() << "; source=" << sourceModel()
             << "rowCount()" << rowCount();
    Q_ASSERT( false );
    return QModelIndex();
  }
  QModelIndex index = indexes.first();*/
  QModelIndex index = node->sourceIndex;
  if ( !index.isValid() ) {
    kWarning() << "IncidenceTreeModel::mapToSource(): sourceModelIndex is invalid";
    Q_ASSERT( false );
    return QModelIndex();
  }
  Q_ASSERT( index.model() == sourceModel() );

  return index.sibling( index.row(), proxyIndex.column() );
}

QModelIndex IncidenceTreeModel::parent( const QModelIndex &child ) const
{
  if ( !child.isValid() ) {
    kWarning() << "IncidenceTreeModel::parent(): child is invalid";
    Q_ASSERT( false );
    return QModelIndex();
  }

  Q_ASSERT( child.model() == this );
  Q_ASSERT( child.internalPointer() );
  Node *childNode = reinterpret_cast<Node*>( child.internalPointer() );
  if ( d->m_removedNodes.contains( childNode ) ) {
    kWarning() << "IncidenceTreeModel::parent() Node already removed.";
    return QModelIndex();
  }

  if ( !childNode->parentNode ) {
    return QModelIndex();
  }

  const QModelIndex parentIndex = d->indexForNode( childNode->parentNode );

  if ( !parentIndex.isValid() ) {
    kWarning() << "IncidenceTreeModel::parent(): proxyModelIndex is invalid.";
    Q_ASSERT( false );
    return QModelIndex();
  }

  Q_ASSERT( parentIndex.model() == this );
  Q_ASSERT( childNode->parentNode.data() );

  // Parent is always at row 0
  return parentIndex;
}

QModelIndex IncidenceTreeModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( row < 0 || row >= rowCount( parent ) ) {
    // This is ok apparently
    /*kWarning() << "IncidenceTreeModel::index() parent.isValid()" << parent.isValid()
               << "; row=" << row << "; column=" << column
               << "; rowCount() = " << rowCount( parent ); */
    // Q_ASSERT( false );
    return QModelIndex();
  }

  Q_ASSERT( column >= 0 );
  Q_ASSERT( column < columnCount() );

  if ( parent.isValid() ) {
    Q_ASSERT( parent.model() == this );
    Q_ASSERT( parent.internalPointer() );
    Node *parentNode = reinterpret_cast<Node*>( parent.internalPointer() );

    if ( row >= parentNode->directChilds.count() ) {
      kError() << "IncidenceTreeModel::index() row=" << row << "; column=" << column;
      Q_ASSERT( false );
      return QModelIndex();
    }

    return createIndex( row, column,
                        parentNode->directChilds.at( row ).data() );
  } else {
    Q_ASSERT( row < d->m_toplevelNodeList.count() );
    Node::Ptr node = d->m_toplevelNodeList.at( row );
    Q_ASSERT( node );
    return createIndex( row, column, node.data() );
  }
}

bool IncidenceTreeModel::hasChildren( const QModelIndex &parent ) const
{
  if ( parent.isValid() ) {
    Q_ASSERT( parent.column() < columnCount() );
    if ( parent.column() != 0 ) {
      // Indexes at column >0 don't have parent, says Qt documentation
      return false;
    }
    Node *parentNode = reinterpret_cast<Node*>( parent.internalPointer() );
    Q_ASSERT( parentNode );
    return !parentNode->directChilds.isEmpty();
  } else {
    return !d->m_toplevelNodeList.isEmpty();
  }
}
