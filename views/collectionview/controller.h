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

#ifndef KORG_CONTROLLER_H
#define KORG_CONTROLLER_H

#include <QObject>
#include <QStringList>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/Collection>
#include "reparentingmodel.h"

enum DataRole {
    PersonRole = Akonadi::EntityTreeModel::UserRole + 1,
    IsSearchResultRole,
    CollectionRole
};

struct Person
{
    Person(): rootCollection(-1){};
    QString name;
    Akonadi::Collection::Id rootCollection;
    
    //FIXME not sure we actually require those two
    QStringList folderPaths;
    QList<Akonadi::Collection::Id> collections;
};

Q_DECLARE_METATYPE(Person);

/**
 * We need to emit signals in the subclass but don't want to make the parent a QObject
 */
class Emitter : public QObject {
  Q_OBJECT
public:
    void emitEnabled(bool state, const Person &person)
    {
        emit enabled(state, person);
    }

    void emitEnabled(bool state, const Akonadi::Collection &collection)
    {
        emit enabled(state, collection);
    }

  Q_SIGNALS:
    void enabled(bool, Person);
    void enabled(bool, Akonadi::Collection);
};

/**
 * A node representing a person
 */
class PersonNode : public ReparentingModel::Node
{
public:
    PersonNode(ReparentingModel &personModel, const Person &person);
    virtual ~PersonNode();
    virtual bool operator==(const Node &) const;

    void setChecked(bool);

    virtual QVariant data(int role) const;

    Emitter emitter;
    bool isSearchNode;

private:
    virtual bool setData(const QVariant& variant, int role);
    virtual bool adopts(const QModelIndex& sourceIndex);
    virtual bool isDuplicateOf(const QModelIndex& sourceIndex);

    Person mPerson;
    Qt::CheckState mCheckState;
};

class CollectionNode : public ReparentingModel::Node
{
public:
    CollectionNode(ReparentingModel &personModel, const Akonadi::Collection &col);
    virtual ~CollectionNode();
    virtual bool operator==(const Node &) const;

    Emitter emitter;
    bool isSearchNode;

private:
    virtual QVariant data(int role) const;
    virtual bool setData(const QVariant& variant, int role);
    virtual bool isDuplicateOf(const QModelIndex& sourceIndex);
    Akonadi::Collection mCollection;
    Qt::CheckState mCheckState;
};

class PersonNodeManager : public ReparentingModel::NodeManager
{
public:
    PersonNodeManager(ReparentingModel &personModel) : ReparentingModel::NodeManager(personModel){};
private:
    void checkSourceIndex(const QModelIndex &sourceIndex);
    void checkSourceIndexRemoval(const QModelIndex &sourceIndex);
};

class CollectionSearchJob : public KJob
{
    Q_OBJECT
public:
    explicit CollectionSearchJob(const QString &searchString, QObject* parent = 0);

    virtual void start();

    Akonadi::Collection::List matchingCollections() const;

private Q_SLOTS:
    void onCollectionsReceived(const Akonadi::Collection::List &);
    void onCollectionsFetched(KJob *);
    void onAncestorsFetched(KJob *);

private:
    QString mSearchString;
    Akonadi::Collection::List mMatchingCollections;
    Akonadi::Collection::List mAncestors;
};

class PersonSearchJob : public KJob
{
    Q_OBJECT
public:
    explicit PersonSearchJob(const QString &searchString, QObject* parent = 0);

    virtual void start();

    QList<Person> matches() const;

private Q_SLOTS:
    void onCollectionsReceived(const Akonadi::Collection::List &);
    void onCollectionsFetched(KJob *);

private:
    QString mSearchString;
    QList<Person> mMatches;
};

/**
 * Add search results to the search model, and use the selection to add results to the person model.
 */
class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(ReparentingModel *personModel, ReparentingModel *searchModel, QObject* parent = 0);
    /**
     *  This model will be used to select the collections that are available in the ETM
     */
    void setEntityTreeModel(Akonadi::EntityTreeModel *etm);

    void setCollectionReferenced(bool enabled, const Akonadi::Collection &collection);
    void setCollectionEnabled(bool enabled, const Akonadi::Collection &collection);
    void setCollection(const Akonadi::Collection &collection, bool enabled, bool referenced);
    void setPersonEnabled(const Person &person, bool enabled);

Q_SIGNALS:
    void searchIsActive(bool);

public Q_SLOTS:
    void setSearchString(const QString &);

private Q_SLOTS:
    void onCollectionsFound(KJob *job);
    void onPersonsFound(KJob *job);
    void onPersonEnabled(bool enabled, const Person &person);
    void onPersonCollectionsFetched(KJob *job);
    void onCollectionEnabled(bool enabled, const Akonadi::Collection &collection);

private:
    ReparentingModel *mPersonModel;
    ReparentingModel *mSearchModel;
    CollectionSearchJob *mCollectionSearchJob;
    PersonSearchJob *mPersonSearchJob;
};

#endif
