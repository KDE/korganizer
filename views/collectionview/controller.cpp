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
#include "controller.h"

#include <Akonadi/EntityTreeModel>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/AttributeFactory>
#include <KIcon>
#include <KCalCore/Event>
#include <KCalCore/Journal>
#include <KCalCore/Todo>
#include <baloo/pim/collectionquery.h>
#include <akonadi/collectionidentificationattribute.h>

CollectionNode::CollectionNode(ReparentingModel& personModel, const Akonadi::Collection& col)
:   Node(personModel),
    mCollection(col),
    mCheckState(Qt::Unchecked)
{
}

CollectionNode::~CollectionNode()
{

}

bool CollectionNode::operator==(const ReparentingModel::Node &node) const
{
    const CollectionNode *collectionNode = dynamic_cast<const CollectionNode*>(&node);
    if (collectionNode) {
        return (collectionNode->mCollection == mCollection);
    }
    return false;
}

QVariant CollectionNode::data(int role) const
{
    if (role == Qt::DisplayRole) {
        QStringList path;
        Akonadi::Collection c = mCollection;
        while (c.isValid()) {
            path.prepend(c.name());
            c = c.parentCollection();
        }
        return path.join(QLatin1String("/"));
    }
    if (role == Qt::DecorationRole) {
        if (mCollection.hasAttribute<Akonadi::EntityDisplayAttribute>()) {
            return mCollection.attribute<Akonadi::EntityDisplayAttribute>()->icon();
        }
        return QVariant();
    }
    if (role == Qt::CheckStateRole) {
        return mCheckState;
    }
    if (role == Qt::ToolTipRole) {
        return QString(QLatin1String("Collection: ") + mCollection.name() + QString::number(mCollection.id()));
    }
    return QVariant();
}

bool CollectionNode::setData(const QVariant& value, int role)
{
    if (role == Qt::CheckStateRole) {
        mCheckState = static_cast<Qt::CheckState>(value.toInt());
        emitter.emitEnabled(mCheckState == Qt::Checked, mCollection);
        return true;
    }
    return false;
}

bool CollectionNode::isDuplicateOf(const QModelIndex& sourceIndex)
{
    return (sourceIndex.data(Akonadi::EntityTreeModel::CollectionIdRole).value<Akonadi::Collection::Id>() == mCollection.id());
}


PersonNode::PersonNode(ReparentingModel& personModel, const Person& person)
:   Node(personModel),
    mPerson(person),
    mCheckState(Qt::Unchecked)
{

}

PersonNode::~PersonNode()
{

}

bool PersonNode::operator==(const Node &node) const
{
    const PersonNode *personNode = dynamic_cast<const PersonNode*>(&node);
    if (personNode) {
        return (personNode->mPerson.name == mPerson.name);
    }
    return false;
}

void PersonNode::setChecked(bool enabled)
{
    if (enabled) {
        mCheckState = Qt::Checked;
    } else {
        mCheckState = Qt::Unchecked;
    }
}

QVariant PersonNode::data(int role) const
{
    if (role == Qt::DisplayRole) {
        return mPerson.name;
    }
    if (role == Qt::DecorationRole) {
        return KIcon(QLatin1String("meeting-participant"));
    }
    if (role == Qt::CheckStateRole) {
        return mCheckState;
    }
    if (role == Qt::ToolTipRole) {
        return QString(QLatin1String("Person: ") + mPerson.name);
    }
    if (role == PersonRole) {
        return QVariant::fromValue(mPerson);
    }
    return QVariant();
}

bool PersonNode::setData(const QVariant& value, int role)
{
    if (role == Qt::CheckStateRole) {
        mCheckState = static_cast<Qt::CheckState>(value.toInt());
        emitter.emitEnabled(mCheckState == Qt::Checked, mPerson);
        return true;
    }
    return false;
}

bool PersonNode::adopts(const QModelIndex& sourceIndex)
{
    const Akonadi::Collection &parent = sourceIndex.data(Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
    if (parent.id() == mPerson.rootCollection) {
        return true;
    }

    const Akonadi::Collection &col = sourceIndex.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    // kDebug() << col.displayName();
    //FIXME: we need a way to compare the path we get from LDAP to the folder in akonadi.
    //TODO: get it from the folder attribute
    if ((col.isValid() && mPerson.folderPaths.contains(col.displayName())) || mPerson.collections.contains(col.id())) {
        // kDebug() << "reparenting " << col.displayName() << " to " << mPerson.name;
        return true;
    }
    return false;
}

bool PersonNode::isDuplicateOf(const QModelIndex& sourceIndex)
{
    return (sourceIndex.data(PersonRole).value<Person>().name == mPerson.name);
}

void PersonNodeManager::checkSourceIndex(const QModelIndex &sourceIndex)
{
    const Akonadi::Collection col = sourceIndex.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    kDebug() << col.displayName() << col.enabled();
    if (col.isValid()) {
        CollectionIdentificationAttribute *attr = col.attribute<CollectionIdentificationAttribute>();
        if (attr && attr->collectionNamespace() == "usertoplevel") {
            kDebug() << "Found user folder, creating person node";
            Person person;
            person.name = col.displayName();
            person.rootCollection = col.id();

            model.addNode(ReparentingModel::Node::Ptr(new PersonNode(model, person)));
        }
    }
}

CollectionSearchJob::CollectionSearchJob(const QString& searchString, QObject* parent)
    : KJob(parent),
    mSearchString(searchString)
{
}

void CollectionSearchJob::start()
{
    Baloo::PIM::CollectionQuery query;
    //We exclude the other users namespace
    query.setNamespace(QStringList() << QLatin1String("shared") << QLatin1String(""));
    query.pathMatches(mSearchString);
    query.setMimetype(QStringList() << QLatin1String("text/calendar"));
    query.setLimit(200);
    Baloo::PIM::ResultIterator it = query.exec();
    Akonadi::Collection::List collections;
    while (it.next()) {
        collections << Akonadi::Collection(it.id());
    }
    kDebug() << "Found collections " << collections.size();
    
    if (collections.isEmpty()) {
        //We didn't find anything
        emitResult();
        return;
    }

    Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(collections, Akonadi::CollectionFetchJob::Base, this);
    fetchJob->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
    fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
    connect(fetchJob, SIGNAL(collectionsReceived(Akonadi::Collection::List)), this, SLOT(onCollectionsReceived(Akonadi::Collection::List)));
    connect(fetchJob, SIGNAL(result(KJob*)), this, SLOT(onCollectionsFetched(KJob*)));
}

void CollectionSearchJob::onCollectionsReceived(const Akonadi::Collection::List &list)
{
    Q_FOREACH(const Akonadi::Collection &col, list) {
        if (col.name().contains(mSearchString)) {
            mMatchingCollections << col;
            Akonadi::Collection ancestor = col.parentCollection();
            while (ancestor.isValid() && (ancestor != Akonadi::Collection::root())) {
                if (!mAncestors.contains(ancestor)) {
                    mAncestors << ancestor;
                }
                ancestor = ancestor.parentCollection();
            }
        }
    }
}

void CollectionSearchJob::onCollectionsFetched(KJob *job)
{
    if (job->error()) {
        kWarning() << job->errorString();
    }
    if (!mAncestors.isEmpty()) {
        Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(mAncestors, Akonadi::CollectionFetchJob::Base, this);
        fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
        connect(fetchJob, SIGNAL(result(KJob*)), this, SLOT(onAncestorsFetched(KJob*)));
    } else {
        //We didn't find anything
        emitResult();
    }
}

static Akonadi::Collection replaceParent(Akonadi::Collection col, const Akonadi::Collection::List &ancestors)
{
    if (!col.isValid()) {
        return col;
    }
    const Akonadi::Collection parent = replaceParent(col.parentCollection(), ancestors);
    Q_FOREACH (const Akonadi::Collection &c, ancestors) {
        if (col == c) {
            col = c;
        }
    }
    col.setParentCollection(parent);
    return col;
}

void CollectionSearchJob::onAncestorsFetched(KJob *job)
{
    if (job->error()) {
        kWarning() << job->errorString();
    }
    Akonadi::CollectionFetchJob *fetchJob = static_cast<Akonadi::CollectionFetchJob*>(job);
    Akonadi::Collection::List matchingCollections;
    Q_FOREACH (const Akonadi::Collection &c, mMatchingCollections) {
        //We need to replace the parents with the version that contains the name, so we can display it accordingly
        matchingCollections << replaceParent(c, fetchJob->collections());
    }
    mMatchingCollections = matchingCollections;
    emitResult();
}

Akonadi::Collection::List CollectionSearchJob::matchingCollections() const
{
    return mMatchingCollections;
}


PersonSearchJob::PersonSearchJob(const QString& searchString, QObject* parent)
    : KJob(parent),
    mSearchString(searchString)
{
}

void PersonSearchJob::start()
{
    Baloo::PIM::CollectionQuery query;
    query.setNamespace(QStringList() << QLatin1String("usertoplevel"));
    query.nameMatches(mSearchString);
    query.setLimit(200);
    Baloo::PIM::ResultIterator it = query.exec();
    Akonadi::Collection::List collections;
    while (it.next()) {
        collections << Akonadi::Collection(it.id());
    }
    kDebug() << "Found persons " << collections.size();

    if (collections.isEmpty()) {
        //We didn't find anything
        emitResult();
        return;
    }

    Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(collections, Akonadi::CollectionFetchJob::Base, this);
    fetchJob->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
    fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
    connect(fetchJob, SIGNAL(collectionsReceived(Akonadi::Collection::List)), this, SLOT(onCollectionsReceived(Akonadi::Collection::List)));
    connect(fetchJob, SIGNAL(result(KJob*)), this, SLOT(onCollectionsFetched(KJob*)));
    //TODO query ldap for available persons and their folders.
    //TODO identify imap folders as person folders and list them here (after indexing them in baloo).
    //
    //The IMAP resource should add a "Person" attribute to the collections in the person namespace,
    //the ldap query can then be used to update the name (entitydisplayattribute) for the person.
}

void PersonSearchJob::onCollectionsReceived(const Akonadi::Collection::List &list)
{
    Q_FOREACH(const Akonadi::Collection &col, list) {
        Person person;
        person.name = col.displayName();
        person.rootCollection = col.id();
        mMatches << person;
    }
}

void PersonSearchJob::onCollectionsFetched(KJob *job)
{
    if (job->error()) {
        kWarning() << job->errorString();
    }
    emitResult();
}

QList<Person> PersonSearchJob::matches() const
{
    return mMatches;
}


Controller::Controller(ReparentingModel* personModel, ReparentingModel* searchModel, QObject* parent)
    : QObject(parent),
    mPersonModel(personModel),
    mSearchModel(searchModel),
    mCollectionSearchJob(0),
    mPersonSearchJob(0)
{
    Akonadi::AttributeFactory::registerAttribute<CollectionIdentificationAttribute>();
}

void Controller::setSearchString(const QString &searchString)
{
    if (mCollectionSearchJob) {
        disconnect(mCollectionSearchJob, 0, this, 0);
        mCollectionSearchJob->kill(KJob::Quietly);
        mCollectionSearchJob = 0;
    }
    if (mPersonSearchJob) {
        disconnect(mPersonSearchJob, 0, this, 0);
        mPersonSearchJob->kill(KJob::Quietly);
        mPersonSearchJob = 0;
    }
    //TODO: Delay and abort when results are found
    mSearchModel->clear();
    emit searchIsActive(!searchString.isEmpty());
    if (searchString.size() < 2) {
        return;
    }

    mPersonSearchJob = new PersonSearchJob(searchString, this);
    connect(mPersonSearchJob, SIGNAL(result(KJob*)), this, SLOT(onPersonsFound(KJob*)));
    mPersonSearchJob->start();

    mCollectionSearchJob = new CollectionSearchJob(searchString, this);
    connect(mCollectionSearchJob, SIGNAL(result(KJob*)), this, SLOT(onCollectionsFound(KJob*)));
    mCollectionSearchJob->start();
}

void Controller::onPersonEnabled(bool enabled, const Person& person)
{
    // kDebug() << person.name << enabled;
    if (enabled) {
        PersonNode *personNode = new PersonNode(*mPersonModel, person);
        personNode->setChecked(true);
        mPersonModel->addNode(ReparentingModel::Node::Ptr(personNode));
        Akonadi::Collection rootCollection(person.rootCollection);
        if (rootCollection.isValid()) {
            //Reference the persons collections if available
            Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(rootCollection, Akonadi::CollectionFetchJob::Recursive, this);
            fetchJob->setProperty("enable", enabled);
            fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
            fetchJob->fetchScope().setContentMimeTypes(QStringList() << QLatin1String("text/calendar"));
            connect(fetchJob, SIGNAL(result(KJob*)), this, SLOT(onPersonCollectionsFetched(KJob*)));
        }
    } else {
        //If we accidentally added a person and want to remove it again
        mPersonModel->removeNode(PersonNode(*mPersonModel, person));
        //Dereference subcollections
    }

}

void Controller::onPersonCollectionsFetched(KJob* job)
{
    if (job->error()) {
        kWarning() << "Failed to fetch collections " << job->errorString();
        return;
    }
    const bool enable = job->property("enable").toBool();
    Q_FOREACH(const Akonadi::Collection &col, static_cast<Akonadi::CollectionFetchJob*>(job)->collections()) {
        setCollectionReferenced(enable, col);
    }
}

void Controller::onCollectionsFound(KJob* job)
{
    if (job->error()) {
        kWarning() << job->errorString();
        mCollectionSearchJob = 0;
        return;
    }
    Q_ASSERT(mCollectionSearchJob == static_cast<CollectionSearchJob*>(job));
    Q_FOREACH(const Akonadi::Collection &col, mCollectionSearchJob->matchingCollections()) {
        CollectionNode *collectionNode = new CollectionNode(*mSearchModel, col);
        //toggled by the checkbox, results in collection getting monitored
        connect(&collectionNode->emitter, SIGNAL(enabled(bool, Akonadi::Collection)), this, SLOT(onCollectionEnabled(bool, Akonadi::Collection)));
        mSearchModel->addNode(ReparentingModel::Node::Ptr(collectionNode));
    }
    mCollectionSearchJob = 0;
}

void Controller::onPersonsFound(KJob* job)
{
    if (job->error()) {
        kWarning() << job->errorString();
        mPersonSearchJob = 0;
        return;
    }
    Q_ASSERT(mPersonSearchJob == static_cast<PersonSearchJob*>(job));
    Q_FOREACH(const Person &p, mPersonSearchJob->matches()) {
        PersonNode *personNode = new PersonNode(*mSearchModel, p);
        //toggled by the checkbox, results in person getting added to main model
        connect(&personNode->emitter, SIGNAL(enabled(bool, Person)), this, SLOT(onPersonEnabled(bool, Person)));
        mSearchModel->addNode(ReparentingModel::Node::Ptr(personNode));
    }
    mPersonSearchJob = 0;
}

static Akonadi::EntityTreeModel *findEtm(QAbstractItemModel *model)
{
    QAbstractProxyModel *proxyModel;
    while (model) {
        proxyModel = qobject_cast<QAbstractProxyModel*>(model);
        if (proxyModel && proxyModel->sourceModel()) {
            model = proxyModel->sourceModel();
        } else {
            break;
        }
    }
    return qobject_cast<Akonadi::EntityTreeModel*>(model);
}

void Controller::setCollectionReferenced(bool enabled, const Akonadi::Collection& collection)
{
    kDebug() << collection.displayName() << "do reference " << enabled;
    kDebug() << "current " << collection.referenced();
    Akonadi::EntityTreeModel *etm = findEtm(mPersonModel);
    Q_ASSERT(etm);
    etm->setCollectionReferenced(collection, enabled);
}

void Controller::setCollectionEnabled(bool enabled, const Akonadi::Collection& collection)
{
    kDebug() << collection.displayName() << "do enable " << enabled;
    kDebug() << "current " << collection.enabled();

    Akonadi::Collection modifiedCollection = collection;
    modifiedCollection.setShouldList(Akonadi::Collection::ListDisplay, enabled);
    new Akonadi::CollectionModifyJob(modifiedCollection);
}

void Controller::onCollectionEnabled(bool enabled, const Akonadi::Collection& collection)
{
    setCollectionReferenced(enabled, collection);
}

void Controller::setCollection(const Akonadi::Collection &collection, bool enabled, bool referenced)
{
    Akonadi::EntityTreeModel *etm = findEtm(mPersonModel);
    if (!etm) {
        kWarning() << "Couldn't find etm";
        return;
    }
    kDebug() << collection.displayName() << "do enable " << enabled;
    Akonadi::Collection modifiedCollection = collection;
    modifiedCollection.setShouldList(Akonadi::Collection::ListDisplay, enabled);
    //HACK: We have no way of getting to the correct session as used by the etm,
    //and two concurrent jobs end up overwriting the enabled state of each other.
    etm->setCollectionReferenced(modifiedCollection, referenced);
}

void Controller::setPersonDisabled(const Person &person)
{
    mPersonModel->removeNode(PersonNode(*mPersonModel, person));
}

