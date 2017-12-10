/*
 * Copyright (C) 2014 Christian Mollekopf <mollekopf@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * As a special exception, permission is given to link this program
 * with any edition of Qt, and distribute the resulting executable,
 * without including the source code for Qt in the source distribution.
 */

#ifndef KORG_CONTROLLER_H
#define KORG_CONTROLLER_H

#include <QObject>
#include <AkonadiCore/EntityTreeModel>
#include <AkonadiCore/Collection>
#include "reparentingmodel.h"
#include <LibkdepimAkonadi/Person>

#include <Libkdepim/LdapClientSearch>

enum DataRoles {
    PersonRole = Akonadi::EntityTreeModel::UserRole + 1,
    IsSearchResultRole,
    CollectionRole,
    NodeTypeRole,
    EnabledRole
};

enum NodeTypeRoles {
    SourceNodeRole,
    PersonNodeRole,
    CollectionNodeRole
};

/**
 * We need to Q_EMIT signals in the subclass but don't want to make the parent a QObject
 */
class Emitter : public QObject
{
    Q_OBJECT
public:
    void emitEnabled(bool state, const KPIM::Person &person)
    {
        Q_EMIT enabled(state, person);
    }

    void emitEnabled(bool state, const Akonadi::Collection &collection)
    {
        Q_EMIT enabled(state, collection);
    }

Q_SIGNALS:
    void enabled(bool, const KPIM::Person &);
    void enabled(bool, const Akonadi::Collection &);
};

/**
 * A node representing a person
 */
class PersonNode : public ReparentingModel::Node
{
public:
    PersonNode(ReparentingModel &personModel, const KPIM::Person &person);
    virtual ~PersonNode();
    bool operator==(const Node &) const override;

    void setChecked(bool);

    QVariant data(int role) const override;

    Emitter emitter;
    bool isSearchNode;

private:
    bool setData(const QVariant &variant, int role) override;
    bool adopts(const QModelIndex &sourceIndex) override;
    bool isDuplicateOf(const QModelIndex &sourceIndex) override;
    void update(const Node::Ptr &node) override;

    KPIM::Person mPerson;
    Qt::CheckState mCheckState;
};

class CollectionNode : public ReparentingModel::Node
{
public:
    CollectionNode(ReparentingModel &personModel, const Akonadi::Collection &col);
    virtual ~CollectionNode();
    bool operator==(const Node &) const override;

    Emitter emitter;
    bool isSearchNode;

private:
    QVariant data(int role) const override;
    bool setData(const QVariant &variant, int role) override;
    bool isDuplicateOf(const QModelIndex &sourceIndex) override;
    Akonadi::Collection mCollection;
    Qt::CheckState mCheckState;
};

class PersonNodeManager : public ReparentingModel::NodeManager
{
public:
    PersonNodeManager(ReparentingModel &personModel) : ReparentingModel::NodeManager(personModel)
    {
    }

private:
    KPIM::Person person(const QModelIndex &sourceIndex);
    void checkSourceIndex(const QModelIndex &sourceIndex) override;
    void checkSourceIndexRemoval(const QModelIndex &sourceIndex) override;
    void updateSourceIndex(const QModelIndex &sourceIndex) override;
};

namespace KPIM {
class CollectionSearchJob;
class PersonSearchJob;
}

/**
 * Add search results to the search model, and use the selection to add results to the person model.
 */
class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(ReparentingModel *personModel, ReparentingModel *searchModel,
                        QObject *parent = nullptr);
    /**
     *  This model will be used to select the collections that are available in the ETM
     */
    void setEntityTreeModel(Akonadi::EntityTreeModel *etm);

    enum CollectionState {
        Disabled,
        Referenced,
        Enabled
    };
    void setCollectionState(const Akonadi::Collection &collection, CollectionState collectionState,
                            bool recursive = false);

    void addPerson(const KPIM::Person &person);
    void removePerson(const KPIM::Person &person);

Q_SIGNALS:
    void searchIsActive(bool);
    void searching(bool);

public Q_SLOTS:
    void setSearchString(const QString &);

private Q_SLOTS:
    void onCollectionsFound(KJob *job);
    void onPersonsFound(const QVector<KPIM::Person> &persons);
    void onPersonUpdate(const KPIM::Person &person);
    void onPersonsFound(KJob *job);
    void onPersonCollectionsFetched(KJob *job);

private:
    ReparentingModel *mPersonModel = nullptr;
    ReparentingModel *mSearchModel = nullptr;
    KPIM::CollectionSearchJob *mCollectionSearchJob = nullptr;
    KPIM::PersonSearchJob *mPersonSearchJob = nullptr;
};

#endif
