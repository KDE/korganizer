/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#include "reparentingmodel.h"
#include "korganizer_debug.h"

#include <algorithm>

/*
 * Notes:
 * * layoutChanged must never add or remove nodes.
 * * rebuildAll can therefore only be called if it doesn't introduce new nodes or within a reset.
 * * The node memory management is done using the node tree, nodes are deleted by being removed from the node tree.
 */

ReparentingModel::Node::Node(ReparentingModel &model)
    : personModel(model)
    , mIsSourceNode(false)
{
}

ReparentingModel::Node::Node(ReparentingModel &model, ReparentingModel::Node *parent, const QModelIndex &sourceIndex)
    : sourceIndex(sourceIndex)
    , parent(parent)
    , personModel(model)
    , mIsSourceNode(true)
{
    if (sourceIndex.isValid()) {
        personModel.mSourceNodes.append(this);
    }
    Q_ASSERT(parent);
}

ReparentingModel::Node::~Node()
{
    // The source index may be invalid meanwhile (it's a persistent index)
    personModel.mSourceNodes.removeOne(this);
}

bool ReparentingModel::Node::operator==(const ReparentingModel::Node &node) const
{
    return this == &node;
}

ReparentingModel::Node::Ptr ReparentingModel::Node::searchNode(ReparentingModel::Node *node)
{
    Node::Ptr nodePtr;
    if (node->parent) {
        // Reparent node
        QList<Node::Ptr>::iterator it = node->parent->children.begin();
        for (; it != node->parent->children.end(); ++it) {
            if (it->data() == node) {
                // Reuse smart pointer
                nodePtr = *it;
                node->parent->children.erase(it);
                break;
            }
        }
        Q_ASSERT(nodePtr);
    } else {
        nodePtr = Node::Ptr(node);
    }

    return nodePtr;
}

void ReparentingModel::Node::addChild(const ReparentingModel::Node::Ptr &node)
{
    node->parent = this;
    children.append(node);
}

void ReparentingModel::Node::clearHierarchy()
{
    parent = nullptr;
    children.clear();
}

bool ReparentingModel::Node::setData(const QVariant & /* value */, int /* role */)
{
    return false;
}

QVariant ReparentingModel::Node::data(int role) const
{
    if (sourceIndex.isValid()) {
        return sourceIndex.data(role);
    }
    return {};
}

bool ReparentingModel::Node::adopts(const QModelIndex & /* sourceIndex */)
{
    return false;
}

bool ReparentingModel::Node::isDuplicateOf(const QModelIndex & /* sourceIndex */)
{
    return false;
}

void ReparentingModel::Node::update(const Node::Ptr & /* node */)
{
}

bool ReparentingModel::Node::isSourceNode() const
{
    return mIsSourceNode;
}

int ReparentingModel::Node::row() const
{
    Q_ASSERT(parent);
    int row = 0;
    for (const Node::Ptr &node : std::as_const(parent->children)) {
        if (node.data() == this) {
            return row;
        }
        row++;
    }
    return -1;
}

ReparentingModel::ReparentingModel(QObject *parent)
    : QAbstractProxyModel(parent)
    , mRootNode(*this)
    , mNodeManager(NodeManager::Ptr(new NodeManager(*this)))
{
}

ReparentingModel::~ReparentingModel()
{
    // Otherwise we cannot guarantee that the nodes reference to *this is always valid
    mRootNode.children.clear();
    mProxyNodes.clear();
    mSourceNodes.clear();
}

bool ReparentingModel::validateNode(const Node *node) const
{
    // Expected:
    // * Each node tree starts at mRootNode
    // * Each node is listed in the children of it's parent
    // * Root node never leaves the model and thus should never enter this function
    if (!node) {
        qCWarning(KORGANIZER_LOG) << "nullptr";
        return false;
    }
    if (node == &mRootNode) {
        qCWarning(KORGANIZER_LOG) << "is root node";
        return false;
    }
    const Node *n = node;
    int depth = 0;
    while (n) {
        if ((intptr_t)(n) < 1000) {
            // Detect corruptions with unlikely pointers
            qCWarning(KORGANIZER_LOG) << "corrupt pointer" << depth;
            return false;
        }
        if (!n->parent) {
            qCWarning(KORGANIZER_LOG) << "nullptr parent" << depth << n->isSourceNode();
            return false;
        }
        if (n->parent == n) {
            qCWarning(KORGANIZER_LOG) << "loop" << depth;
            return false;
        }

        bool found = false;
        for (const Node::Ptr &child : std::as_const(n->parent->children)) {
            if (child.data() == n) {
                found = true;
            }
        }
        if (!found) {
            qCWarning(KORGANIZER_LOG) << "not linked as child" << depth;
            return false;
        }
        depth++;
        if (depth > 1000) {
            qCWarning(KORGANIZER_LOG) << "loop detected" << depth;
            return false;
        }

        if (n->parent == &mRootNode) {
            return true;
        }
        // If the parent isn't root there is at least one more level
        if (!n->parent->parent) {
            qCWarning(KORGANIZER_LOG) << "missing parent parent" << depth;
            return false;
        }
        if (n->parent->parent == n) {
            qCWarning(KORGANIZER_LOG) << "parent parent loop" << depth;
            return false;
        }
        n = n->parent;
    }
    qCWarning(KORGANIZER_LOG) << "not linked to root" << depth;
    return false;
}

void ReparentingModel::addNode(const ReparentingModel::Node::Ptr &node)
{
    // We have to make this check before issuing the async method,
    // otherwise we run into the problem that while a node is being removed,
    // the async request could be triggered (due to a changed signal),
    // resulting in the node getting read immediately after it had been removed.
    if (std::ranges::any_of(mProxyNodes, [node](const auto &existing) {
            return *existing == *node;
        })) {
        return;
    }
    mNodesToAdd << node;
    qRegisterMetaType<Node::Ptr>("Node::Ptr");
    QMetaObject::invokeMethod(this, "doAddNode", Qt::QueuedConnection, Q_ARG(Node::Ptr, node));
}

void ReparentingModel::doAddNode(const Node::Ptr &node)
{
    if (std::ranges::any_of(mProxyNodes, [node](const auto &existing) {
            return *existing == *node;
        })) {
        return;
    }

    // If a datachanged call triggered this through checkSourceIndex, right after a person node has been removed.
    // We'd end-up re-inserting the node that has just been removed. Therefore removeNode can cancel the pending addNode
    // call through mNodesToAdd.
    bool addNodeAborted = true;
    for (int i = 0; i < mNodesToAdd.size(); ++i) {
        if (*mNodesToAdd.at(i) == *node) {
            mNodesToAdd.remove(i);
            addNodeAborted = false;
            break;
        }
    }
    if (addNodeAborted) {
        return;
    }

    if (!isDuplicate(node)) {
        const int targetRow = mRootNode.children.size();
        beginInsertRows(QModelIndex(), targetRow, targetRow);
        mProxyNodes << node;
        insertProxyNode(node);
        endInsertRows();
        reparentSourceNodes(node);
    } else {
        mProxyNodes << node;
    }
}

void ReparentingModel::updateNode(const ReparentingModel::Node::Ptr &node)
{
    const auto it = std::ranges::find_if(mProxyNodes, [node](const Node::Ptr &existing) {
        return *existing == *node;
    });
    if (it != mProxyNodes.end()) {
        (*it)->update(node);
        const QModelIndex i = index((*it).data());
        Q_EMIT dataChanged(i, i);
        return;
    }

    qCWarning(KORGANIZER_LOG) << objectName() << "no node to update, create new node";
    addNode(node);
}

void ReparentingModel::removeNode(const ReparentingModel::Node &node)
{
    // If there is an addNode in progress for that node, abort it.
    for (int i = 0; i < mNodesToAdd.size(); ++i) {
        if (*mNodesToAdd.at(i) == node) {
            mNodesToAdd.remove(i);
        }
    }
    for (int i = 0; i < mProxyNodes.size(); ++i) {
        if (*mProxyNodes.at(i) == node) {
            // TODO: this does not yet take care of un-reparenting reparented nodes.
            const Node &n = *mProxyNodes.at(i);
            Node *parentNode = n.parent;
            beginRemoveRows(index(parentNode), n.row(), n.row());
            parentNode->children.remove(n.row()); // deletes node
            mProxyNodes.remove(i);
            endRemoveRows();
            break;
        }
    }
}

void ReparentingModel::clear()
{
    beginResetModel();
    mProxyNodes.clear();
    rebuildAll();
    endResetModel();
}

void ReparentingModel::setNodeManager(const NodeManager::Ptr &nodeManager)
{
    mNodeManager = nodeManager;
}

void ReparentingModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    beginResetModel();
    QAbstractProxyModel::setSourceModel(sourceModel);
    if (sourceModel) {
        connect(sourceModel, &QAbstractProxyModel::rowsAboutToBeInserted, this, &ReparentingModel::onSourceRowsAboutToBeInserted);
        connect(sourceModel, &QAbstractProxyModel::rowsInserted, this, &ReparentingModel::onSourceRowsInserted);
        connect(sourceModel, &QAbstractProxyModel::rowsAboutToBeRemoved, this, &ReparentingModel::onSourceRowsAboutToBeRemoved);
        connect(sourceModel, &QAbstractProxyModel::rowsRemoved, this, &ReparentingModel::onSourceRowsRemoved);
        connect(sourceModel, &QAbstractProxyModel::rowsAboutToBeMoved, this, &ReparentingModel::onSourceRowsAboutToBeMoved);
        connect(sourceModel, &QAbstractProxyModel::rowsMoved, this, &ReparentingModel::onSourceRowsMoved);
        connect(sourceModel, &QAbstractProxyModel::modelAboutToBeReset, this, &ReparentingModel::onSourceModelAboutToBeReset);
        connect(sourceModel, &QAbstractProxyModel::modelReset, this, &ReparentingModel::onSourceModelReset);
        connect(sourceModel, &QAbstractProxyModel::dataChanged, this, &ReparentingModel::onSourceDataChanged);
        //       connect(sourceModel, &QAbstractProxyModel::headerDataChanged, this, &ReparentingModel::_k_sourceHeaderDataChanged);
        connect(sourceModel, &QAbstractProxyModel::layoutAboutToBeChanged, this, &ReparentingModel::onSourceLayoutAboutToBeChanged);
        connect(sourceModel, &QAbstractProxyModel::layoutChanged, this, &ReparentingModel::onSourceLayoutChanged);
        //       connect(sourceModel, &QAbstractProxyModel::destroyed, this, &ReparentingModel::onSourceModelDestroyed);
    }

    rebuildAll();
    endResetModel();
}

void ReparentingModel::onSourceRowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
}

ReparentingModel::Node *ReparentingModel::getReparentNode(const QModelIndex &sourceIndex)
{
    const auto it = std::ranges::find_if(mProxyNodes, [sourceIndex](const Node::Ptr &proxyNode) {
        // Re-parent source nodes according to the provided rules
        // The proxy can be ignored if it is a duplicate, so only re-parent to proxies that are in the model
        return (proxyNode->parent && proxyNode->adopts(sourceIndex));
    });
    if (it != mProxyNodes.end()) {
        Q_ASSERT(validateNode((*it).data()));
        return (*it).data();
    }
    return nullptr;
}

ReparentingModel::Node *ReparentingModel::getParentNode(const QModelIndex &sourceIndex)
{
    if (Node *node = getReparentNode(sourceIndex)) {
        return node;
    }
    const QModelIndex proxyIndex = mapFromSource(sourceIndex.parent());
    if (proxyIndex.isValid()) {
        return extractNode(proxyIndex);
    }
    return nullptr;
}

void ReparentingModel::appendSourceNode(Node *parentNode, const QModelIndex &sourceParent, const QModelIndexList &skip)
{
    mNodeManager->checkSourceIndex(sourceParent);

    const Node::Ptr node(new Node(*this, parentNode, sourceParent));
    parentNode->children.append(node);
    Q_ASSERT(validateNode(node.data()));
    rebuildFromSource(node.data(), sourceParent, skip);
}

QModelIndexList ReparentingModel::descendants(const QModelIndex &sourceIndex)
{
    if (!sourceModel()) {
        return {};
    }
    QModelIndexList list;
    if (sourceModel()->hasChildren(sourceIndex)) {
        const int count = sourceModel()->rowCount(sourceIndex);
        list.reserve(count * 2);
        for (int i = 0; i < count; ++i) {
            const QModelIndex idx = sourceModel()->index(i, 0, sourceIndex);
            list << idx;
            list << descendants(idx);
        }
    }
    return list;
}

void ReparentingModel::removeDuplicates(const QModelIndex &sourceIndex)
{
    const QModelIndexList list = QModelIndexList() << sourceIndex << descendants(sourceIndex);
    for (const QModelIndex &descendant : list) {
        for (const Node::Ptr &proxyNode : std::as_const(mProxyNodes)) {
            if (proxyNode->isDuplicateOf(descendant)) {
                // Removenode from proxy
                if (!proxyNode->parent) {
                    qCWarning(KORGANIZER_LOG) << objectName() << "Found proxy that is already not part of the model "
                                              << proxyNode->data(Qt::DisplayRole).toString();
                    continue;
                }
                const int targetRow = proxyNode->row();
                beginRemoveRows(index(proxyNode->parent), targetRow, targetRow);
                proxyNode->parent->children.remove(targetRow);
                proxyNode->parent = nullptr;
                endRemoveRows();
            }
        }
    }
}

void ReparentingModel::onSourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    // qCDebug(KORGANIZER_LOG) << objectName() << parent << start << end;
    for (int r = start; r <= end; r++) {
        const QModelIndex sourceIndex = sourceModel()->index(r, 0, parent);
        Q_ASSERT(sourceIndex.isValid());
        Node *parentNode = getParentNode(sourceIndex);
        if (!parentNode) {
            parentNode = &mRootNode;
        } else {
            Q_ASSERT(validateNode(parentNode));
        }
        Q_ASSERT(parentNode);

        // Remove any duplicates that we are going to replace
        removeDuplicates(sourceIndex);

        QModelIndexList reparented;
        // Check for children to reparent
        {
            const auto descendantsItem = descendants(sourceIndex);
            for (const QModelIndex &descendant : descendantsItem) {
                if (Node *proxyNode = getReparentNode(descendant)) {
                    qCDebug(KORGANIZER_LOG) << "reparenting " << descendant.data().toString();
                    const int targetRow = proxyNode->children.size();
                    beginInsertRows(index(proxyNode), targetRow, targetRow);
                    appendSourceNode(proxyNode, descendant);
                    reparented << descendant;
                    endInsertRows();
                }
            }
        }

        /* cppcheck-suppress knownConditionTrueFalse */
        if (parentNode->isSourceNode()) {
            const int targetRow = parentNode->children.size();
            beginInsertRows(mapFromSource(parent), targetRow, targetRow);
            appendSourceNode(parentNode, sourceIndex, reparented);
            endInsertRows();
        } else { // Reparented
            const int targetRow = parentNode->children.size();
            beginInsertRows(index(parentNode), targetRow, targetRow);
            appendSourceNode(parentNode, sourceIndex);
            endInsertRows();
        }
    }
}

void ReparentingModel::onSourceRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    // qCDebug(KORGANIZER_LOG) << objectName() << parent << start << end;
    // we remove in reverse order as otherwise the indexes in parentNode->children wouldn't be correct
    for (int r = end; r >= start; r--) {
        const QModelIndex sourceIndex = sourceModel()->index(r, 0, parent);
        Q_ASSERT(sourceIndex.isValid());

        const QModelIndex proxyIndex = mapFromSource(sourceIndex);
        // If the indexes have already been removed (e.g. by removeNode)this can indeed return an invalid index
        if (proxyIndex.isValid()) {
            const Node *node = extractNode(proxyIndex);
            Node *parentNode = node->parent;
            Q_ASSERT(parentNode);
            const int targetRow = node->row();
            beginRemoveRows(index(parentNode), targetRow, targetRow);
            parentNode->children.remove(targetRow); // deletes node
            endRemoveRows();
        }
    }
    // Allows the node manager to remove nodes that are no longer relevant
    for (int r = start; r <= end; r++) {
        mNodeManager->checkSourceIndexRemoval(sourceModel()->index(r, 0, parent));
    }
}

void ReparentingModel::onSourceRowsRemoved(const QModelIndex & /* parent */, int /* start */, int /* end */)
{
}

void ReparentingModel::onSourceRowsAboutToBeMoved(const QModelIndex & /* sourceParent */,
                                                  int /* sourceStart */,
                                                  int /* sourceEnd */,
                                                  const QModelIndex & /* destParent */,
                                                  int /* dest */)
{
    qCWarning(KORGANIZER_LOG) << "not implemented";
    // TODO
    beginResetModel();
}

void ReparentingModel::onSourceRowsMoved(const QModelIndex & /* sourceParent */,
                                         int /* sourceStart */,
                                         int /* sourceEnd */,
                                         const QModelIndex & /* destParent */,
                                         int /* dest */)
{
    qCWarning(KORGANIZER_LOG) << "not implemented";
    // TODO
    endResetModel();
}

void ReparentingModel::onSourceLayoutAboutToBeChanged()
{
    Q_EMIT layoutAboutToBeChanged();
    // Q_FOREACH(const QModelIndex &proxyPersistentIndex, persistentIndexList()) {
    //     Q_ASSERT(proxyPersistentIndex.isValid());
    //     const QPersistentModelIndex srcPersistentIndex = mapToSource(proxyPersistentIndex);
    //     // TODO also update the proxy persistent indexes
    //     //Skip indexes that are not in the source model
    //     if (!srcPersistentIndex.isValid()) {
    //         continue;
    //     }
    //     mLayoutChangedProxyIndexes << proxyPersistentIndex;
    //     mLayoutChangedSourcePersistentModelIndexes << srcPersistentIndex;
    // }
}

void ReparentingModel::onSourceLayoutChanged()
{
    // By ignoring this we miss structural changes in the sourcemodel, which is mostly ok.
    // Before we can re-enable this we need to properly deal with skipped duplicates, because
    // a layout change MUST NOT add/remove new nodes (only shuffling allowed)
    //
    // Our source indexes are not endagered since we use persistend model indexes anyways

    // rebuildAll();

    // for (int i = 0; i < mLayoutChangedProxyIndexes.size(); ++i) {
    //     const QModelIndex oldProxyIndex = mLayoutChangedProxyIndexes.at(i);
    //     const QModelIndex newProxyIndex = mapFromSource(mLayoutChangedSourcePersistentModelIndexes.at(i));
    //     if (oldProxyIndex != newProxyIndex) {
    //         changePersistentIndex(oldProxyIndex, newProxyIndex);
    //     }
    // }

    // mLayoutChangedProxyIndexes.clear();
    // mLayoutChangedSourcePersistentModelIndexes.clear();

    // layoutChanged();
}

void ReparentingModel::onSourceDataChanged(const QModelIndex &begin, const QModelIndex &end)
{
    // qCDebug(KORGANIZER_LOG) << objectName() << begin << end;
    for (int r = begin.row(); r <= end.row(); r++) {
        mNodeManager->updateSourceIndex(sourceModel()->index(r, begin.column(), begin.parent()));
    }
    Q_EMIT dataChanged(mapFromSource(begin), mapFromSource(end));
}

void ReparentingModel::onSourceModelAboutToBeReset()
{
    beginResetModel();
}

void ReparentingModel::onSourceModelReset()
{
    rebuildAll();
    endResetModel();
}

ReparentingModel::Node *ReparentingModel::extractNode(const QModelIndex &index) const
{
    Node *node = static_cast<Node *>(index.internalPointer());
    Q_ASSERT(node);
    Q_ASSERT(validateNode(node));
    return node;
}

QModelIndex ReparentingModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0) {
        return {};
    }
    // qCDebug(KORGANIZER_LOG) << parent << row;
    const Node *parentNode;
    if (parent.isValid()) {
        parentNode = extractNode(parent);
    } else {
        parentNode = &mRootNode;
    }
    // At least QAbstractItemView expects that we deal with this properly (see rowsAboutToBeRemoved "find the next visible and enabled item")
    // Also QAbstractItemModel::match does all kinds of weird shit including passing row=-1
    if (parentNode->children.size() <= row) {
        return {};
    }
    Node *node = parentNode->children.at(row).data();
    Q_ASSERT(validateNode(node));
    return createIndex(row, column, node);
}

QModelIndex ReparentingModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!proxyIndex.isValid() || !sourceModel()) {
        return {};
    }
    Node *node = extractNode(proxyIndex);
    /* cppcheck-suppress knownConditionTrueFalse */
    if (!node->isSourceNode()) {
        return {};
    }
    Q_ASSERT(node->sourceIndex.model() == sourceModel());
    return node->sourceIndex;
}

ReparentingModel::Node *ReparentingModel::getSourceNode(const QModelIndex &sourceIndex) const
{
    const auto it = std::ranges::find_if(mSourceNodes, [sourceIndex](const Node *n) {
        return n->sourceIndex == sourceIndex;
    });
    return it == mSourceNodes.end() ? nullptr : (*it);
}

QModelIndex ReparentingModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    // qCDebug(KORGANIZER_LOG) << sourceIndex << sourceIndex.data().toString();
    if (!sourceIndex.isValid()) {
        return {};
    }
    Node *node = getSourceNode(sourceIndex);
    if (!node) {
        // This can happen if a source nodes is hidden (person collections)
        return {};
    }
    Q_ASSERT(validateNode(node));
    return index(node);
}

void ReparentingModel::rebuildFromSource(Node *parentNode, const QModelIndex &sourceParent, const QModelIndexList &skip)
{
    Q_ASSERT(parentNode);
    if (!sourceModel()) {
        return;
    }
    for (int i = 0; i < sourceModel()->rowCount(sourceParent); ++i) {
        const QModelIndex &sourceIndex = sourceModel()->index(i, 0, sourceParent);
        // Skip indexes that should be excluded because they have been reparented
        if (skip.contains(sourceIndex)) {
            continue;
        }
        appendSourceNode(parentNode, sourceIndex, skip);
    }
}

bool ReparentingModel::isDuplicate(const Node::Ptr &proxyNode) const
{
    return std::ranges::any_of(mSourceNodes, [proxyNode](const Node *n) {
        return proxyNode->isDuplicateOf(n->sourceIndex);
    });
}

void ReparentingModel::insertProxyNode(const Node::Ptr &proxyNode)
{
    // qCDebug(KORGANIZER_LOG) << "checking " << proxyNode->data(Qt::DisplayRole).toString();
    proxyNode->parent = &mRootNode;
    mRootNode.addChild(proxyNode);
    Q_ASSERT(validateNode(proxyNode.data()));
}

void ReparentingModel::reparentSourceNodes(const Node::Ptr &proxyNode)
{
    // Reparent source nodes according to the provided rules
    for (Node *n : std::as_const(mSourceNodes)) {
        if (proxyNode->adopts(n->sourceIndex)) {
            // qCDebug(KORGANIZER_LOG) << "reparenting" << n->data(Qt::DisplayRole).toString() << "from" << n->parent->data(Qt::DisplayRole).toString()
            //         << "to" << proxyNode->data(Qt::DisplayRole).toString();

            // WARNING: While a beginMoveRows/endMoveRows would be more suitable, QSortFilterProxyModel can't deal with that. Therefore we
            // cannot use them.
            const int oldRow = n->row();
            beginRemoveRows(index(n->parent), oldRow, oldRow);
            const Node::Ptr nodePtr = proxyNode->searchNode(n);
            // We lie about the row being removed already, but the view can deal with that better than if we call endRemoveRows after beginInsertRows
            endRemoveRows();

            const int newRow = proxyNode->children.size();
            beginInsertRows(index(proxyNode.data()), newRow, newRow);
            proxyNode->addChild(nodePtr);
            endInsertRows();
            Q_ASSERT(validateNode(n));
        }
    }
}

void ReparentingModel::rebuildAll()
{
    mRootNode.children.clear();
    for (const Node::Ptr &proxyNode : std::as_const(mProxyNodes)) {
        proxyNode->clearHierarchy();
    }
    Q_ASSERT(mSourceNodes.isEmpty());
    mSourceNodes.clear();
    rebuildFromSource(&mRootNode, QModelIndex());
    for (const Node::Ptr &proxyNode : std::as_const(mProxyNodes)) {
        // qCDebug(KORGANIZER_LOG) << "checking " << proxyNode->data(Qt::DisplayRole).toString();
        // Avoid inserting a node that is already part of the source model
        if (isDuplicate(proxyNode)) {
            continue;
        }
        insertProxyNode(proxyNode);
        reparentSourceNodes(proxyNode);
    }
}

QVariant ReparentingModel::data(const QModelIndex &proxyIndex, int role) const
{
    if (!proxyIndex.isValid()) {
        return {};
    }
    const Node *node = extractNode(proxyIndex);
    /* cppcheck-suppress knownConditionTrueFalse */
    if (node->isSourceNode()) {
        const QModelIndex cur = mapToSource(proxyIndex);
        if (cur.isValid()) {
            return sourceModel()->data(cur, role);
        }
        return {};
    }
    return node->data(role);
}

bool ReparentingModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }
    Q_ASSERT(index.isValid());
    if (!sourceModel()) {
        return false;
    }
    Node *node = extractNode(index);
    /* cppcheck-suppress knownConditionTrueFalse */
    if (node->isSourceNode()) {
        const QModelIndex cur = mapToSource(index);
        if (cur.isValid()) {
            return sourceModel()->setData(cur, value, role);
        }
        return false;
    }
    return node->setData(value, role);
}

Qt::ItemFlags ReparentingModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || !sourceModel()) {
        return Qt::NoItemFlags;
    }
    const Node *node = extractNode(index);
    /* cppcheck-suppress knownConditionTrueFalse */
    if (!node->isSourceNode()) {
        return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }
    const QModelIndex cur = mapToSource(index);
    if (cur.isValid()) {
        return sourceModel()->flags(cur);
    }
    return Qt::NoItemFlags;
}

int ReparentingModel::row(ReparentingModel::Node *node) const
{
    Q_ASSERT(node);
    if (node == &mRootNode) {
        return -1;
    }
    Q_ASSERT(validateNode(node));
    int row = 0;
    for (const Node::Ptr &c : std::as_const(node->parent->children)) {
        if (c.data() == node) {
            return row;
        }
        row++;
    }
    return -1;
}

QModelIndex ReparentingModel::index(Node *node) const
{
    const int r = row(node);
    if (r < 0) {
        return {};
    }
    return createIndex(r, 0, node);
}

QModelIndex ReparentingModel::parent(const QModelIndex &child) const
{
    // qCDebug(KORGANIZER_LOG) << child << child.data().toString();
    if (!child.isValid()) {
        return {};
    }
    const Node *node = extractNode(child);
    return index(node->parent);
}

QModelIndex ReparentingModel::buddy(const QModelIndex &index) const
{
    if (!index.isValid() || !sourceModel()) {
        return {};
    }
    const Node *node = extractNode(index);
    /* cppcheck-suppress knownConditionTrueFalse */
    if (node->isSourceNode()) {
        const QModelIndex cur = mapToSource(index);
        if (cur.isValid()) {
            return mapFromSource(sourceModel()->buddy(cur));
        }
        return {};
    }
    return index;
}

int ReparentingModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return mRootNode.children.size();
    }

    if (parent.column() != 0) {
        return 0;
    }

    Node *node = extractNode(parent);
    return node->children.size();
}

bool ReparentingModel::hasChildren(const QModelIndex &parent) const
{
    return rowCount(parent) != 0;
}

int ReparentingModel::columnCount(const QModelIndex & /* parent */) const
{
    return 1;
}

#include "moc_reparentingmodel.cpp"
