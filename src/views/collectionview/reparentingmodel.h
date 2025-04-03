/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#pragma once

#include <QAbstractProxyModel>
#include <QList>
#include <QSharedPointer>

/**
 * A model that can hold an extra set of nodes which can "adopt" (reparent),
 * source nodes.
 */
class ReparentingModel : public QAbstractProxyModel
{
    Q_OBJECT
public:
    struct Node {
        using Ptr = QSharedPointer<Node>;
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
        virtual void update(const Node::Ptr &node);

        bool isSourceNode() const;
        Node::Ptr searchNode(Node *node);
        void reparent(Node *node);
        void addChild(const Node::Ptr &node);
        int row() const;
        void clearHierarchy();

        QPersistentModelIndex sourceIndex;
        QList<Ptr> children;
        Node *parent = nullptr;
        ReparentingModel &personModel;
        const bool mIsSourceNode = false;
    };

    struct NodeManager {
        using Ptr = QSharedPointer<NodeManager>;

        NodeManager(ReparentingModel &m)
            : model(m)
        {
        }

        virtual ~NodeManager() = default;

    protected:
        ReparentingModel &model;

    private:
        friend class ReparentingModel;

        // Allows the implementation to create proxy nodes as necessary
        virtual void checkSourceIndex(const QModelIndex & /* sourceIndex */)
        {
        }

        virtual void checkSourceIndexRemoval(const QModelIndex & /* sourceIndex */)
        {
        }

        virtual void updateSourceIndex(const QModelIndex &sourceIndex)
        {
            checkSourceIndex(sourceIndex);
        }
    };

public:
    explicit ReparentingModel(QObject *parent = nullptr);
    ~ReparentingModel() override;

    void setNodeManager(const NodeManager::Ptr &nodeManager);
    void addNode(const Node::Ptr &node);
    void updateNode(const Node::Ptr &node);
    void removeNode(const Node &node);
    void setNodes(const QList<Node::Ptr> &nodes);
    void clear();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QVariant data(const QModelIndex &proxyIndex, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex buddy(const QModelIndex &index) const override;

    void setSourceModel(QAbstractItemModel *sourceModel) override;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;

private Q_SLOTS:
    void onSourceRowsAboutToBeInserted(const QModelIndex &, int, int);
    void onSourceRowsInserted(const QModelIndex &, int, int);
    void onSourceRowsAboutToBeRemoved(const QModelIndex &, int, int);
    void onSourceRowsRemoved(const QModelIndex &, int, int);
    void onSourceRowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int);
    void onSourceRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int);
    void onSourceDataChanged(const QModelIndex &, const QModelIndex &);
    void onSourceLayoutAboutToBeChanged();
    void onSourceLayoutChanged();
    void onSourceModelAboutToBeReset();
    void onSourceModelReset();
    void doAddNode(const ReparentingModel::Node::Ptr &node);

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
    QList<Node *> mSourceNodes;
    QList<Node::Ptr> mProxyNodes;
    QList<Node::Ptr> mNodesToAdd;
    NodeManager::Ptr mNodeManager;
};
