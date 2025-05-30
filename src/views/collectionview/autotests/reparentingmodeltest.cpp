/*
 * SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
 */

#include "reparentingmodel.h"
#include "korganizer_debug.h"
#include <QObject>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTest>

class DummyNode : public ReparentingModel::Node
{
public:
    DummyNode(ReparentingModel &personModel, const QString &name, const QString &data = QString())
        : ReparentingModel::Node(personModel)
        , mUid(name)
        , mParent(QStringLiteral("orphan"))
        , mName(name)
        , mData(data)
    {
    }

    ~DummyNode() override = default;

    bool operator==(const Node &node) const override
    {
        const auto dummyNode = dynamic_cast<const DummyNode *>(&node);
        if (dummyNode) {
            return dummyNode->mUid == mUid;
        }
        return false;
    }

    QString mUid;
    QString mParent;

private:
    [[nodiscard]] QVariant data(int role) const override
    {
        if (role == Qt::DisplayRole) {
            if (mName != mUid) {
                return QString(mUid + QLatin1Char('-') + mName);
            } else {
                return mName;
            }
        } else if (role == Qt::UserRole) {
            return mData;
        }
        return {};
    }

    bool setData(const QVariant &variant, int role) override
    {
        Q_UNUSED(variant)
        Q_UNUSED(role)
        return false;
    }

    bool isDuplicateOf(const QModelIndex &sourceIndex) override
    {
        return sourceIndex.data().toString() == mUid;
    }

    bool adopts(const QModelIndex &sourceIndex) override
    {
        return sourceIndex.data().toString().contains(mParent);
    }

    void update(const Node::Ptr &node) override
    {
        mName = node.staticCast<DummyNode>()->mName;
        mData = node.staticCast<DummyNode>()->mData;
    }

    QString mName;
    QString mData;
};

class ModelSignalSpy : public QObject
{
    Q_OBJECT
public:
    explicit ModelSignalSpy(const QAbstractItemModel &model)
    {
        connect(&model, &QAbstractItemModel::rowsInserted, this, &ModelSignalSpy::onRowsInserted);
        connect(&model, &QAbstractItemModel::rowsRemoved, this, &ModelSignalSpy::onRowsRemoved);
        connect(&model, &QAbstractItemModel::rowsMoved, this, &ModelSignalSpy::onRowsMoved);
        connect(&model, &QAbstractItemModel::dataChanged, this, &ModelSignalSpy::onDataChanged);
        connect(&model, &QAbstractItemModel::layoutChanged, this, &ModelSignalSpy::onLayoutChanged);
        connect(&model, &QAbstractItemModel::modelReset, this, &ModelSignalSpy::onModelReset);
    }

    QStringList mSignals;
    QModelIndex parent;
    QModelIndex topLeft, bottomRight;
    int start = 0;
    int end = 0;

public Q_SLOTS: // NOLINT(readability-redundant-access-specifiers)
    void onRowsInserted(const QModelIndex &p, int s, int e)
    {
        mSignals << QStringLiteral("rowsInserted");
        parent = p;
        start = s;
        end = e;
    }

    void onRowsRemoved(const QModelIndex &p, int s, int e)
    {
        mSignals << QStringLiteral("rowsRemoved");
        parent = p;
        start = s;
        end = e;
    }

    void onRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)
    {
        mSignals << QStringLiteral("rowsMoved");
    }

    void onDataChanged(const QModelIndex &t, const QModelIndex &b)
    {
        mSignals << QStringLiteral("dataChanged");
        topLeft = t;
        bottomRight = b;
    }

    void onLayoutChanged()
    {
        mSignals << QStringLiteral("layoutChanged");
    }

    void onModelReset()
    {
        mSignals << QStringLiteral("modelReset");
    }
};

static QModelIndex getIndex(const char *string, const QAbstractItemModel &model)
{
    QModelIndexList list = model.match(model.index(0, 0), Qt::DisplayRole, QString::fromLatin1(string), 1, Qt::MatchRecursive);
    if (list.isEmpty()) {
        return {};
    }
    return list.first();
}

static QModelIndexList getIndexList(const char *string, const QAbstractItemModel &model)
{
    return model.match(model.index(0, 0), Qt::DisplayRole, QString::fromLatin1(string), 1, Qt::MatchRecursive);
}

class ReparentingModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPopulation();
    void testAddRemoveSourceItem();
    void testInsertSourceRow();
    void testInsertSourceRowSubnode();
    void testAddRemoveProxyNode();
    void testDeduplicate();
    void testDeduplicateNested();
    void testDeduplicateProxyNodeFirst();
    void testNestedDeduplicateProxyNodeFirst();
    void testUpdateNode();
    void testReparent();
    void testReparentSubcollections();
    void testReparentResetWithoutCrash();
    void testAddReparentedSourceItem();
    void testRemoveReparentedSourceItem();
    void testNestedReparentedSourceItem();
    void testAddNestedReparentedSourceItem();
    void testSourceDataChanged();
    void testSourceLayoutChanged();
    void testInvalidLayoutChanged();
    void testAddRemoveNodeByNodeManager();
    void testRemoveNodeByNodeManagerWithDataChanged();
    void testDataChanged();
};

void ReparentingModelTest::testPopulation()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(new QStandardItem(QStringLiteral("row1")));
    sourceModel.appendRow(new QStandardItem(QStringLiteral("row2")));

    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&sourceModel);

    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 2);
    QVERIFY(getIndex("row1", reparentingModel).isValid());
    QVERIFY(getIndex("row2", reparentingModel).isValid());
}

void ReparentingModelTest::testAddRemoveSourceItem()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(new QStandardItem(QStringLiteral("row1")));

    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&sourceModel);
    ModelSignalSpy const spy(reparentingModel);

    sourceModel.appendRow(new QStandardItem(QStringLiteral("row2")));
    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 2);
    QVERIFY(getIndex("row1", reparentingModel).isValid());
    QVERIFY(getIndex("row2", reparentingModel).isValid());
    QCOMPARE(spy.parent, QModelIndex());
    QCOMPARE(spy.start, 1);
    QCOMPARE(spy.end, 1);

    sourceModel.removeRows(1, 1, QModelIndex());
    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 1);
    QVERIFY(getIndex("row1", reparentingModel).isValid());
    QVERIFY(!getIndex("row2", reparentingModel).isValid());
    QCOMPARE(spy.parent, QModelIndex());
    QCOMPARE(spy.start, 1);
    QCOMPARE(spy.end, 1);

    QCOMPARE(spy.mSignals, QStringList() << QStringLiteral("rowsInserted") << QStringLiteral("rowsRemoved"));
}

// Ensure the model can deal with rows that are inserted out of order
void ReparentingModelTest::testInsertSourceRow()
{
    QStandardItemModel sourceModel;
    auto row2 = new QStandardItem(QStringLiteral("row2"));
    sourceModel.appendRow(row2);

    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&sourceModel);
    ModelSignalSpy const spy(reparentingModel);

    auto row1 = new QStandardItem(QStringLiteral("row1"));
    sourceModel.insertRow(0, row1);
    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 2);
    QVERIFY(getIndex("row1", reparentingModel).isValid());
    QVERIFY(getIndex("row2", reparentingModel).isValid());

    // The model does not try to reorder. First come, first serve.
    QCOMPARE(getIndex("row1", reparentingModel).row(), 1);
    QCOMPARE(getIndex("row2", reparentingModel).row(), 0);
    reparentingModel.setData(reparentingModel.index(1, 0, QModelIndex()), QStringLiteral("row1foo"), Qt::DisplayRole);
    reparentingModel.setData(reparentingModel.index(0, 0, QModelIndex()), QStringLiteral("row2foo"), Qt::DisplayRole);
    QCOMPARE(row1->data(Qt::DisplayRole).toString(), QStringLiteral("row1foo"));
    QCOMPARE(row2->data(Qt::DisplayRole).toString(), QStringLiteral("row2foo"));
}

// Ensure the model can deal with rows that are inserted out of order in a subnode
void ReparentingModelTest::testInsertSourceRowSubnode()
{
    auto parent = new QStandardItem(QStringLiteral("parent"));

    QStandardItemModel sourceModel;
    sourceModel.appendRow(parent);
    auto row2 = new QStandardItem(QStringLiteral("row2"));
    parent->appendRow(row2);

    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&sourceModel);
    ModelSignalSpy const spy(reparentingModel);

    auto row1 = new QStandardItem(QStringLiteral("row1"));
    parent->insertRow(0, row1);

    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 1);
    QVERIFY(getIndex("row1", reparentingModel).isValid());
    QVERIFY(getIndex("row2", reparentingModel).isValid());
    // The model does not try to reorder. First come, first serve.
    QCOMPARE(getIndex("row1", reparentingModel).row(), 1);
    QCOMPARE(getIndex("row2", reparentingModel).row(), 0);
    reparentingModel.setData(reparentingModel.index(1, 0, getIndex("parent", reparentingModel)), QStringLiteral("row1foo"), Qt::DisplayRole);
    reparentingModel.setData(reparentingModel.index(0, 0, getIndex("parent", reparentingModel)), QStringLiteral("row2foo"), Qt::DisplayRole);
    QCOMPARE(row1->data(Qt::DisplayRole).toString(), QStringLiteral("row1foo"));
    QCOMPARE(row2->data(Qt::DisplayRole).toString(), QStringLiteral("row2foo"));
}

void ReparentingModelTest::testAddRemoveProxyNode()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(new QStandardItem(QStringLiteral("row1")));

    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&sourceModel);

    ModelSignalSpy const spy(reparentingModel);

    reparentingModel.addNode(ReparentingModel::Node::Ptr(new DummyNode(reparentingModel, QStringLiteral("proxy1"))));

    QTest::qWait(0);

    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 2);
    QVERIFY(getIndex("row1", reparentingModel).isValid());
    QVERIFY(getIndex("proxy1", reparentingModel).isValid());

    reparentingModel.removeNode(DummyNode(reparentingModel, QStringLiteral("proxy1")));

    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 1);
    QVERIFY(getIndex("row1", reparentingModel).isValid());
    QVERIFY(!getIndex("proxy1", reparentingModel).isValid());

    QCOMPARE(spy.mSignals, QStringList() << QStringLiteral("rowsInserted") << QStringLiteral("rowsRemoved"));
}

void ReparentingModelTest::testDeduplicate()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(new QStandardItem(QStringLiteral("row1")));

    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&sourceModel);

    reparentingModel.addNode(ReparentingModel::Node::Ptr(new DummyNode(reparentingModel, QStringLiteral("row1"))));

    QTest::qWait(0);

    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 1);
    QCOMPARE(getIndexList("row1", reparentingModel).size(), 1);
    // TODO ensure we actually have the source index and not the proxy index
}

/**
 * rebuildAll detects and handles nested duplicates
 */
void ReparentingModelTest::testDeduplicateNested()
{
    QStandardItemModel sourceModel;
    auto item = new QStandardItem(QStringLiteral("row1"));
    item->appendRow(new QStandardItem(QStringLiteral("child1")));
    sourceModel.appendRow(item);

    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&sourceModel);

    reparentingModel.addNode(ReparentingModel::Node::Ptr(new DummyNode(reparentingModel, QStringLiteral("child1"))));

    QTest::qWait(0);

    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 1);
    QCOMPARE(getIndexList("child1", reparentingModel).size(), 1);
}

/**
 * onSourceRowsInserted detects and removes duplicates
 */
void ReparentingModelTest::testDeduplicateProxyNodeFirst()
{
    QStandardItemModel sourceModel;
    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&sourceModel);
    reparentingModel.addNode(ReparentingModel::Node::Ptr(new DummyNode(reparentingModel, QStringLiteral("row1"))));

    QTest::qWait(0);

    sourceModel.appendRow(new QStandardItem(QStringLiteral("row1")));

    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 1);
    QCOMPARE(getIndexList("row1", reparentingModel).size(), 1);
    // TODO ensure we actually have the source index and not the proxy index
}

/**
 * onSourceRowsInserted detects and removes nested duplicates
 */
void ReparentingModelTest::testNestedDeduplicateProxyNodeFirst()
{
    QStandardItemModel sourceModel;
    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&sourceModel);
    reparentingModel.addNode(ReparentingModel::Node::Ptr(new DummyNode(reparentingModel, QStringLiteral("child1"))));

    QTest::qWait(0);

    auto item = new QStandardItem(QStringLiteral("row1"));
    item->appendRow(new QStandardItem(QStringLiteral("child1")));
    sourceModel.appendRow(item);

    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 1);
    QCOMPARE(getIndexList("child1", reparentingModel).size(), 1);
    // TODO ensure we actually have the source index and not the proxy index
}

/**
 * updateNode should update the node data
 */
void ReparentingModelTest::testUpdateNode()
{
    QStandardItemModel sourceModel;
    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&sourceModel);
    reparentingModel.addNode(ReparentingModel::Node::Ptr(new DummyNode(reparentingModel, QStringLiteral("proxy1"), QStringLiteral("blub"))));

    QTest::qWait(0);

    QModelIndex const index = getIndex("proxy1", reparentingModel);
    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 1);
    QVERIFY(index.isValid());
    QCOMPARE(reparentingModel.data(index, Qt::UserRole).toString(), QStringLiteral("blub"));

    ModelSignalSpy spy(reparentingModel);
    reparentingModel.updateNode(ReparentingModel::Node::Ptr(new DummyNode(reparentingModel, QStringLiteral("proxy1"), QStringLiteral("new data"))));
    QTest::qWait(0);

    QModelIndex const i2 = getIndex("proxy1", reparentingModel);
    QCOMPARE(i2.column(), index.column());
    QCOMPARE(i2.row(), index.row());

    QCOMPARE(spy.mSignals.count(), 1);
    QCOMPARE(spy.mSignals.takeLast(), QStringLiteral("dataChanged"));
    QCOMPARE(spy.topLeft, i2);
    QCOMPARE(spy.bottomRight, i2);

    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 1);
    QCOMPARE(reparentingModel.data(i2, Qt::UserRole).toString(), QStringLiteral("new data"));
}

void ReparentingModelTest::testReparent()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(new QStandardItem(QStringLiteral("orphan")));

    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&sourceModel);

    reparentingModel.addNode(ReparentingModel::Node::Ptr(new DummyNode(reparentingModel, QStringLiteral("proxy1"))));

    QTest::qWait(0);

    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 1);
    QVERIFY(getIndex("proxy1", reparentingModel).isValid());
    QCOMPARE(reparentingModel.rowCount(getIndex("proxy1", reparentingModel)), 1);
}

void ReparentingModelTest::testReparentSubcollections()
{
    QStandardItemModel sourceModel;
    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&sourceModel);

    /* Source structure
     -- +
        -- + orphan
           -- + col1
              -- sub1
              -- sub2
           -- col2
    */
    sourceModel.appendRow(new QStandardItem(QStringLiteral("orphan")));
    sourceModel.item(0, 0)->appendRow(new QStandardItem(QStringLiteral("col1")));
    sourceModel.item(0, 0)->child(0, 0)->appendRow(new QStandardItem(QStringLiteral("sub1")));
    sourceModel.item(0, 0)->child(0, 0)->appendRow(new QStandardItem(QStringLiteral("sub2")));
    sourceModel.item(0, 0)->appendRow(new QStandardItem(QStringLiteral("col2")));

    auto node = new DummyNode(reparentingModel, QStringLiteral("col1"));
    node->mUid = QStringLiteral("uid");
    node->mParent = QStringLiteral("col");

    /* new srutcure:
     -- +
        -- orphan
        -- + uid-col1
           -- + col1
              -- sub1
              -- sub2
           -- col2
    */
    reparentingModel.addNode(ReparentingModel::Node::Ptr(node));

    QTest::qWait(0);

    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 2);
    QVERIFY(getIndex("col1", reparentingModel).isValid());
    QCOMPARE(getIndex("col1", reparentingModel).parent(), getIndex("uid-col1", reparentingModel));
    QCOMPARE(reparentingModel.rowCount(getIndex("col1", reparentingModel)), 2);
    QCOMPARE(reparentingModel.rowCount(getIndex("uid-col1", reparentingModel)), 2);

    node = new DummyNode(reparentingModel, QStringLiteral("xxx"));
    node->mUid = QStringLiteral("uid");
    node->mParent = QStringLiteral("col");

    // same structure but new data
    reparentingModel.updateNode(ReparentingModel::Node::Ptr(node));

    QTest::qWait(0);

    QCOMPARE(getIndex("col1", reparentingModel).parent(), getIndex("uid-xxx", reparentingModel));
    QCOMPARE(reparentingModel.rowCount(getIndex("col1", reparentingModel)), 2);
    QCOMPARE(reparentingModel.rowCount(getIndex("uid-xxx", reparentingModel)), 2);
}

/*
 * This test ensures we properly deal with reparented source nodes if the model is reset.
 * This is important since source nodes are removed during the model reset while the proxy nodes (to which the source nodes have been reparented) remain.
 *
 * Note that this test is only useful with the model internal asserts.
 */
void ReparentingModelTest::testReparentResetWithoutCrash()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(new QStandardItem(QStringLiteral("orphan")));

    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&sourceModel);

    reparentingModel.addNode(ReparentingModel::Node::Ptr(new DummyNode(reparentingModel, QStringLiteral("proxy1"))));
    QTest::qWait(0);

    reparentingModel.setSourceModel(&sourceModel);

    QTest::qWait(0);

    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 1);
}

void ReparentingModelTest::testAddReparentedSourceItem()
{
    QStandardItemModel sourceModel;

    ReparentingModel reparentingModel;
    reparentingModel.addNode(ReparentingModel::Node::Ptr(new DummyNode(reparentingModel, QStringLiteral("proxy1"))));
    reparentingModel.setSourceModel(&sourceModel);

    QTest::qWait(0);

    ModelSignalSpy const spy(reparentingModel);

    sourceModel.appendRow(new QStandardItem(QStringLiteral("orphan")));

    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 1);
    QVERIFY(getIndex("proxy1", reparentingModel).isValid());
    QCOMPARE(spy.mSignals, QStringList() << QStringLiteral("rowsInserted"));
    QCOMPARE(spy.parent, getIndex("proxy1", reparentingModel));
    QCOMPARE(spy.start, 0);
    QCOMPARE(spy.end, 0);
}

void ReparentingModelTest::testRemoveReparentedSourceItem()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(new QStandardItem(QStringLiteral("orphan")));
    ReparentingModel reparentingModel;
    reparentingModel.addNode(ReparentingModel::Node::Ptr(new DummyNode(reparentingModel, QStringLiteral("proxy1"))));
    reparentingModel.setSourceModel(&sourceModel);

    QTest::qWait(0);

    ModelSignalSpy const spy(reparentingModel);

    sourceModel.removeRows(0, 1, QModelIndex());

    QTest::qWait(0);

    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 1);
    QVERIFY(getIndex("proxy1", reparentingModel).isValid());
    QVERIFY(!getIndex("orphan", reparentingModel).isValid());
    QCOMPARE(spy.mSignals, QStringList() << QStringLiteral("rowsRemoved"));
    QCOMPARE(spy.parent, getIndex("proxy1", reparentingModel));
    QCOMPARE(spy.start, 0);
    QCOMPARE(spy.end, 0);
}

void ReparentingModelTest::testNestedReparentedSourceItem()
{
    QStandardItemModel sourceModel;
    auto item = new QStandardItem(QStringLiteral("parent"));
    item->appendRow(QList<QStandardItem *>() << new QStandardItem(QStringLiteral("orphan")));
    sourceModel.appendRow(item);

    ReparentingModel reparentingModel;
    reparentingModel.addNode(ReparentingModel::Node::Ptr(new DummyNode(reparentingModel, QStringLiteral("proxy1"))));
    reparentingModel.setSourceModel(&sourceModel);

    QTest::qWait(0);

    // toplevel should have both parent and proxy
    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 2);
    QVERIFY(getIndex("orphan", reparentingModel).isValid());
    QCOMPARE(getIndex("orphan", reparentingModel).parent(), getIndex("proxy1", reparentingModel));
}

void ReparentingModelTest::testAddNestedReparentedSourceItem()
{
    QStandardItemModel sourceModel;

    ReparentingModel reparentingModel;
    reparentingModel.addNode(ReparentingModel::Node::Ptr(new DummyNode(reparentingModel, QStringLiteral("proxy1"))));
    reparentingModel.setSourceModel(&sourceModel);

    QTest::qWait(0);

    ModelSignalSpy const spy(reparentingModel);

    auto item = new QStandardItem(QStringLiteral("parent"));
    item->appendRow(QList<QStandardItem *>() << new QStandardItem(QStringLiteral("orphan")));
    sourceModel.appendRow(item);

    QTest::qWait(0);

    // toplevel should have both parent and proxy
    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 2);
    QVERIFY(getIndex("orphan", reparentingModel).isValid());
    QCOMPARE(getIndex("orphan", reparentingModel).parent(), getIndex("proxy1", reparentingModel));
    QCOMPARE(spy.mSignals, QStringList() << QStringLiteral("rowsInserted") << QStringLiteral("rowsInserted"));
}

void ReparentingModelTest::testSourceDataChanged()
{
    QStandardItemModel sourceModel;
    auto item = new QStandardItem(QStringLiteral("row1"));
    sourceModel.appendRow(item);

    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&sourceModel);

    item->setText(QStringLiteral("rowX"));

    QVERIFY(!getIndex("row1", reparentingModel).isValid());
    QVERIFY(getIndex("rowX", reparentingModel).isValid());
}

void ReparentingModelTest::testSourceLayoutChanged()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(new QStandardItem(QStringLiteral("row2")));
    sourceModel.appendRow(new QStandardItem(QStringLiteral("row1")));

    QSortFilterProxyModel filter;
    filter.setSourceModel(&sourceModel);

    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&filter);
    ModelSignalSpy const spy(reparentingModel);

    QPersistentModelIndex const index1 = reparentingModel.index(0, 0, QModelIndex());
    QPersistentModelIndex const index2 = reparentingModel.index(1, 0, QModelIndex());

    // Emits layout changed and sorts the items the other way around
    filter.sort(0, Qt::AscendingOrder);

    QCOMPARE(reparentingModel.rowCount(QModelIndex()), 2);
    QVERIFY(getIndex("row1", reparentingModel).isValid());
    // Right now we don't even care about the order
    // QCOMPARE(spy.mSignals, QStringList() << QStringLiteral("layoutChanged"));
    QCOMPARE(index1.data().toString(), QStringLiteral("row2"));
    QCOMPARE(index2.data().toString(), QStringLiteral("row1"));
}

/*
 * This is a very implementation specific test that tries to crash the model
 */
// Test for invalid implementation of layoutChanged
//*have proxy node in model
//*insert duplicate from source
//*issue layout changed so the model gets rebuilt
//*access node (which is not actually existing anymore)
// => crash
void ReparentingModelTest::testInvalidLayoutChanged()
{
    QStandardItemModel sourceModel;
    QSortFilterProxyModel filter;
    filter.setSourceModel(&sourceModel);
    ReparentingModel reparentingModel;
    reparentingModel.setSourceModel(&filter);
    reparentingModel.addNode(ReparentingModel::Node::Ptr(new DummyNode(reparentingModel, QStringLiteral("row1"))));

    QTest::qWait(0);

    // Take reference to proxy node
    const QModelIndexList row1List = getIndexList("row1", reparentingModel);
    QVERIFY(!row1List.isEmpty());
    QPersistentModelIndex const persistentIndex = row1List.first();
    QVERIFY(persistentIndex.isValid());

    sourceModel.appendRow(new QStandardItem(QStringLiteral("row1")));
    sourceModel.appendRow(new QStandardItem(QStringLiteral("row2")));

    // This rebuilds the model and invalidates the reference
    // Emits layout changed and sorts the items the other way around
    filter.sort(0, Qt::AscendingOrder);

    // This fails because the persistenIndex is no longer valid
    persistentIndex.data().toString();
    QVERIFY(!persistentIndex.isValid());
}

class DummyNodeManager : public ReparentingModel::NodeManager
{
public:
    explicit DummyNodeManager(ReparentingModel &m)
        : ReparentingModel::NodeManager(m)
    {
    }

private:
    void checkSourceIndex(const QModelIndex &sourceIndex) override
    {
        if (sourceIndex.data().toString() == QLatin1StringView("personfolder")) {
            model.addNode(ReparentingModel::Node::Ptr(new DummyNode(model, QStringLiteral("personnode"))));
        }
    }

    void checkSourceIndexRemoval(const QModelIndex &sourceIndex) override
    {
        if (sourceIndex.data().toString() == QLatin1StringView("personfolder")) {
            model.removeNode(DummyNode(model, QStringLiteral("personnode")));
        }
    }
};

void ReparentingModelTest::testAddRemoveNodeByNodeManager()
{
    QStandardItemModel sourceModel;
    sourceModel.appendRow(new QStandardItem(QStringLiteral("personfolder")));
    ReparentingModel reparentingModel;
    reparentingModel.setNodeManager(ReparentingModel::NodeManager::Ptr(new DummyNodeManager(reparentingModel)));
    reparentingModel.setSourceModel(&sourceModel);

    QTest::qWait(0);

    QVERIFY(getIndex("personnode", reparentingModel).isValid());
    QVERIFY(getIndex("personfolder", reparentingModel).isValid());

    sourceModel.removeRows(0, 1, QModelIndex());

    QTest::qWait(0);
    QVERIFY(!getIndex("personnode", reparentingModel).isValid());
    QVERIFY(!getIndex("personfolder", reparentingModel).isValid());
}

/*
 * This tests a special case that is caused by the delayed doAddNode call,
 * causing a removed node to be read immediately if it's removed while
 * a doAddNode call is pending (that can be triggered by dataChanged).
 */
void ReparentingModelTest::testRemoveNodeByNodeManagerWithDataChanged()
{
    QStandardItemModel sourceModel;
    auto item = new QStandardItem(QStringLiteral("personfolder"));
    sourceModel.appendRow(item);
    ReparentingModel reparentingModel;
    reparentingModel.setNodeManager(ReparentingModel::NodeManager::Ptr(new DummyNodeManager(reparentingModel)));
    reparentingModel.setSourceModel(&sourceModel);

    QTest::qWait(0);

    QVERIFY(getIndex("personnode", reparentingModel).isValid());
    QVERIFY(getIndex("personfolder", reparentingModel).isValid());

    // Trigger data changed
    item->setStatusTip(QStringLiteral("sldkfjlfsj"));
    sourceModel.removeRows(0, 1, QModelIndex());

    QTest::qWait(0);
    QVERIFY(!getIndex("personnode", reparentingModel).isValid());
    QVERIFY(!getIndex("personfolder", reparentingModel).isValid());
}

void ReparentingModelTest::testDataChanged()
{
    QStandardItemModel sourceModel;
    auto item = new QStandardItem(QStringLiteral("folder"));
    sourceModel.appendRow(item);
    ReparentingModel reparentingModel;
    reparentingModel.setNodeManager(ReparentingModel::NodeManager::Ptr(new DummyNodeManager(reparentingModel)));
    reparentingModel.setSourceModel(&sourceModel);
    ModelSignalSpy const spy(reparentingModel);

    QTest::qWait(0);

    // Trigger data changed
    item->setStatusTip(QStringLiteral("sldkfjlfsj"));

    QTest::qWait(0);

    QCOMPARE(spy.mSignals, QStringList() << QStringLiteral("dataChanged"));
}

QTEST_MAIN(ReparentingModelTest)

#include "reparentingmodeltest.moc"
