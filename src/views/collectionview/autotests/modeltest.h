/****************************************************************************
**
** SPDX-FileCopyrightText: 2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Concurrent project on Trolltech Labs.
**
** SPDX-License-Identifier: GPL-2.0-only
**
****************************************************************************/
//krazy:excludeall=style

#ifndef MODELTEST_H
#define MODELTEST_H

#include <QObject>
#include <QAbstractItemModel>
#include <QStack>

class ModelTest : public QObject
{
    Q_OBJECT

public:
    explicit ModelTest(QAbstractItemModel *model, QObject *parent = nullptr);

private Q_SLOTS:
    void nonDestructiveBasicTest();
    void rowCount();
    void columnCount();
    void hasIndex();
    void index();
    void parent();
    void data();

protected Q_SLOTS:
    void runAllTests();
    void layoutAboutToBeChanged();
    void layoutChanged();
    void modelAboutToBeReset();
    void modelReset();
    void rowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
    void rowsRemoved(const QModelIndex &parent, int start, int end);
    void rowsAboutToBeMoved(const QModelIndex &, int, int, const QModelIndex &, int);
    void rowsMoved(const QModelIndex &, int, int, const QModelIndex &, int);

private:
    void checkChildren(const QModelIndex &parent, int currentDepth = 0);

    QAbstractItemModel *model = nullptr;

    struct Changing {
        QModelIndex parent;
        int oldSize;
        QVariant last;
        QVariant next;
    };
    QStack<Changing> insert;
    QStack<Changing> remove;

    bool fetchingMore;

    QList<QPersistentModelIndex> changing;
};

#endif
