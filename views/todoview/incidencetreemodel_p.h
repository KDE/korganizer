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

#ifndef INCIDENCE_TREEMODEL_P_H
#define INCIDENCE_TREEMODEL_P_H

#include "incidencetreemodel.h"

#include <Akonadi/Item>

#include <QHash>
#include <QObject>
#include <QVector>
#include <QModelIndex>
#include <QSharedPointer>
#include <QStringList>
#include <QPersistentModelIndex>
#include <KCalCore/Incidence>

typedef QString Uid;
typedef QString ParentUid;

struct Node {
  typedef QSharedPointer<Node> Ptr;
  typedef QMap<Akonadi::Item::Id,Ptr> Map;
  typedef QVector<Ptr> List;

  QPersistentModelIndex sourceIndex; // because ETM::modelIndexesForItem is so slow
  Akonadi::Item::Id id;
  Node::Ptr parentNode;
  QString parentUid;
  QString uid;
  List directChilds;
  int depth;
};

/** Just a struct to contain some data before we create the node */
struct PreNode {
  typedef QSharedPointer<PreNode> Ptr;
  typedef QList<Ptr> List;
  KCalCore::Incidence::Ptr incidence;
  QPersistentModelIndex sourceIndex;
  Akonadi::Item item;
  int depth;
  PreNode() : depth( -1 ) {}
};

class IncidenceTreeModel::Private : public QObject
{
  Q_OBJECT
public:
  Private( IncidenceTreeModel *qq, const QStringList &mimeTypes );
  void reset( bool silent = false );
  void insertNode( const PreNode::Ptr &node, bool silent = false );
  void insertNode( const QModelIndex &sourceIndex, bool silent = false );
  void removeNode( const Node::Ptr &node );
  QModelIndex indexForNode( const Node::Ptr &node ) const;
  int rowForNode( const Node::Ptr &node ) const;
  bool indexBeingRemoved( const QModelIndex & ) const; // Is it being removed?
  void dumpTree();
  void assert_and_dump( bool condition, const QString &message );
  Node::List sorted( const Node::List &nodes ) const;
  PreNode::Ptr prenodeFromSourceRow( int sourceRow ) const;

public:
  Node::Map m_nodeMap;
  Node::List m_toplevelNodeList;
  QHash<Uid,Node::Ptr> m_uidMap;
  QHash<Uid,Akonadi::Item> m_itemByUid;
  QMultiHash<ParentUid,Node::Ptr> m_waitingForParent;
  QList<Node*> m_removedNodes;
  const QStringList m_mimeTypes;

private Q_SLOTS:
  void onHeaderDataChanged( Qt::Orientation orientation, int first, int last );
  void onDataChanged( const QModelIndex &begin, const QModelIndex &end );

  void onRowsAboutToBeInserted( const QModelIndex &parent, int begin, int end );
  void onRowsInserted( const QModelIndex &parent, int begin, int end );
  void onRowsAboutToBeRemoved( const QModelIndex &parent, int begin, int end );
  void onRowsRemoved( const QModelIndex &parent, int begin, int end );
  void onRowsMoved( const QModelIndex &, int, int, const QModelIndex &, int );

  void onModelAboutToBeReset();
  void onModelReset();
  void onLayoutAboutToBeChanged();
  void onLayoutChanged();
private:
  IncidenceTreeModel *q;
};

#endif
