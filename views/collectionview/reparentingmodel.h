/*
  Copyright (C) 2014 Christian Mollekopf <mollekopf@kolabsys.com>

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

#ifndef KORG_REPARENTINGMODEL_H
#define KORG_REPARENTINGMODEL_H

#include <QAbstractProxyModel>
#include <QSharedPointer>
#include <QVector>

/**
 * A model that can hold an extra set of nodes which can "adopt" (reparent),
 * source nodes.
 */
class ReparentingModel : public QAbstractProxyModel
{
    Q_OBJECT
public:
    struct Node {
        typedef QSharedPointer<Node> Ptr;
        virtual ~Node();
        virtual bool operator==(const Node &) const;

    protected:
        Node(ReparentingModel &personModel);

    private:
        friend class ReparentingModel;
        Node(ReparentingModel &personModel, Node *parent, const QModelIndex &sourceIndex);
        virtual QVariant data(int role) const;
        virtual bool setData(const QVariant &variant, int role);
        virtual bool adopts(const QModelIndex &sourceIndex);
        virtual bool isDuplicateOf(const QModelIndex &sourceIndex);

        bool isSourceNode() const;
        void reparent(Node *node);
        void addChild(const Node::Ptr &node);
        int row() const;
        void clearHierarchy();

        QPersistentModelIndex sourceIndex;
        QVector<Ptr> children;
        Node *parent;
        ReparentingModel &personModel;
        bool mIsSourceNode;
    };

    struct NodeManager {
        typedef QSharedPointer<NodeManager> Ptr;

        NodeManager(ReparentingModel &m) :model(m){};
        virtual ~NodeManager(){};

    protected:
        ReparentingModel &model;

    private:
        friend class ReparentingModel;

        //Allows the implementation to create proxy nodes as necessary
        virtual void checkSourceIndex(const QModelIndex &/* sourceIndex */){};
        virtual void checkSourceIndexRemoval(const QModelIndex &/* sourceIndex */){};
        virtual void updateSourceIndex(const QModelIndex & sourceIndex ){checkSourceIndex(sourceIndex);};
    };

public:
    explicit ReparentingModel(QObject* parent = 0);
    virtual ~ReparentingModel();

    void setNodeManager(const NodeManager::Ptr &nodeManager);
    void addNode(const Node::Ptr &node);
    void updateNode(const Node::Ptr &node);
    void removeNode(const Node &node);
    void setNodes(const QList<Node::Ptr> &nodes);
    void clear();

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& child) const;
    virtual QVariant data(const QModelIndex& proxyIndex, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex buddy(const QModelIndex& index) const;

    virtual void setSourceModel(QAbstractItemModel* sourceModel);
    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;

private Q_SLOTS:
    void onSourceRowsAboutToBeInserted(QModelIndex,int,int);
    void onSourceRowsInserted(QModelIndex,int,int);
    void onSourceRowsAboutToBeRemoved(QModelIndex,int,int);
    void onSourceRowsRemoved(QModelIndex,int,int);
    void onSourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int);
    void onSourceRowsMoved(QModelIndex,int,int,QModelIndex,int);
    void onSourceDataChanged(QModelIndex,QModelIndex);
    void onSourceLayoutAboutToBeChanged();
    void onSourceLayoutChanged();
    void onSourceModelAboutToBeReset();
    void onSourceModelReset();
    void doAddNode(const Node::Ptr &node);

private:
    void rebuildFromSource(Node *parentNode, const QModelIndex &idx, const QModelIndexList &skip = QModelIndexList());
    bool isDuplicate(const Node::Ptr &proxyNode) const;
    void insertProxyNode(const Node::Ptr &proxyNode);
    void reparentSourceNodes(const Node::Ptr &proxyNode);
    void rebuildAll();
    QModelIndex index(Node *node) const;
    int row(Node *node) const;
    Node *getReparentNode(const QModelIndex &sourceIndex);
    Node *getParentNode(const QModelIndex &sourceIndex);
    bool validateNode(const Node *node) const;
    Node *extractNode(const QModelIndex &index) const;
    void appendSourceNode(Node *parentNode, const QModelIndex &sourceIndex, const QModelIndexList &skip = QModelIndexList());
    QModelIndexList descendants(const QModelIndex &sourceIndex);
    void removeDuplicates(const QModelIndex &sourceIndex);
    Node *getSourceNode(const QModelIndex &sourceIndex) const;

    Node mRootNode;
    QList<Node*> mSourceNodes;
    QVector<Node::Ptr> mProxyNodes;
    QVector<Node::Ptr> mNodesToAdd;
    NodeManager::Ptr mNodeManager;
    // QModelIndexList mLayoutChangedProxyIndexes;
    // QList<QPersistentModelIndex> mLayoutChangedSourcePersistentModelIndexes;
};


#endif
