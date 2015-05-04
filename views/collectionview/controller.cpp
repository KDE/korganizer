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

#include "controller.h"

#include <AkonadiCore/EntityTreeModel>
#include <AkonadiCore/EntityDisplayAttribute>
#include <AkonadiCore/CollectionModifyJob>
#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/CollectionFetchScope>
#include <AkonadiCore/AttributeFactory>
#include <QIcon>
#include <KCalCore/Event>
#include <KCalCore/Journal>
#include <KCalCore/Todo>
#include <baloo/pim/collectionquery.h>
#include <akonadi/collectionidentificationattribute.h>

#include <libkdepim/job/collectionsearchjob.h>
#include <libkdepim/job/personsearchjob.h>
#include "korganizer_debug.h"

CollectionNode::CollectionNode(ReparentingModel &personModel, const Akonadi::Collection &col)
    :   Node(personModel),
        mCollection(col),
        mCheckState(Qt::Unchecked),
        isSearchNode(false)
{
}

CollectionNode::~CollectionNode()
{

}

bool CollectionNode::operator==(const ReparentingModel::Node &node) const
{
    const CollectionNode *collectionNode = dynamic_cast<const CollectionNode *>(&node);
    if (collectionNode) {
        return (collectionNode->mCollection == mCollection);
    }
    return false;
}

QVariant CollectionNode::data(int role) const
{
    switch (role) {
    case Qt::DisplayRole: {
        QStringList path;
        Akonadi::Collection c = mCollection;
        while (c.isValid()) {
            path.prepend(c.name());
            c = c.parentCollection();
        }
        return path.join(QStringLiteral("/"));
    }
    case Qt::DecorationRole:
        if (mCollection.hasAttribute<Akonadi::EntityDisplayAttribute>()) {
            return mCollection.attribute<Akonadi::EntityDisplayAttribute>()->icon();
        }
        return QVariant();
    case Qt::CheckStateRole:
        if (isSearchNode) {
            return QVariant();
        }
        return mCheckState;
    case Qt::ToolTipRole:
        return i18nc("Collection: name collectionId", "Collection: %1(%2)", mCollection.name(), QString::number(mCollection.id()));
    case IsSearchResultRole:
        return isSearchNode;
    case CollectionRole:
    case Akonadi::EntityTreeModel::CollectionRole:
        return QVariant::fromValue(mCollection);
    case NodeTypeRole:
        return CollectionNodeRole;
    default:
        qCDebug((KORGANIZER_LOG) << "unknown role" << role;
        return QVariant();
    }
}

bool CollectionNode::setData(const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole) {
        mCheckState = static_cast<Qt::CheckState>(value.toInt());
        emitter.emitEnabled(mCheckState == Qt::Checked, mCollection);
        return true;
    }
    return false;
}

bool CollectionNode::isDuplicateOf(const QModelIndex &sourceIndex)
{
    return (sourceIndex.data(Akonadi::EntityTreeModel::CollectionIdRole).value<Akonadi::Collection::Id>() == mCollection.id());
}

PersonNode::PersonNode(ReparentingModel &personModel, const Person &person)
    :   Node(personModel),
        mPerson(person),
        mCheckState(Qt::Unchecked),
        isSearchNode(false)
{

}

PersonNode::~PersonNode()
{

}

bool PersonNode::operator==(const Node &node) const
{
    const PersonNode *personNode = dynamic_cast<const PersonNode *>(&node);
    if (personNode) {
        return (personNode->mPerson.uid == mPerson.uid);
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
    switch (role) {
    case Qt::DisplayRole: {
        QString name = mPerson.name;
        if (!mPerson.ou.isEmpty()) {
            name += QStringLiteral(" (") + mPerson.ou + QStringLiteral(")");
        }
        return name;
    }
    case Qt::DecorationRole:
        return QIcon::fromTheme(QStringLiteral("meeting-participant"));
    case Qt::CheckStateRole:
        if (isSearchNode) {
            return QVariant();
        }
        return mCheckState;
    case Qt::ToolTipRole: {
        QString tooltip = i18n("Person: %1", mPerson.name);
        if (!mPerson.mail.isEmpty()) {
            tooltip += QStringLiteral("\n") + i18n("Mail: %1", mPerson.mail);
        }
        if (!mPerson.ou.isEmpty()) {
            tooltip += QStringLiteral("\n") + i18n("Organization Unit: %1", mPerson.ou);
        }
        return tooltip;
    }
    case PersonRole:
        return QVariant::fromValue(mPerson);
    case IsSearchResultRole:
        return isSearchNode;
    case NodeTypeRole:
        return PersonNodeRole;
    case CollectionRole:
    case Akonadi::EntityTreeModel::CollectionRole:
        return QVariant::fromValue(Akonadi::Collection(mPerson.rootCollection));
    default:
        qCDebug((KORGANIZER_LOG) << "unknown role" << role;
        return QVariant();
    }
}

bool PersonNode::setData(const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole) {
        mCheckState = static_cast<Qt::CheckState>(value.toInt());
        emitter.emitEnabled(mCheckState == Qt::Checked, mPerson);
        return true;
    }
    return false;
}

bool PersonNode::adopts(const QModelIndex &sourceIndex)
{
    const Akonadi::Collection &parent = sourceIndex.data(Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
    if (parent.id() == mPerson.rootCollection) {
        return true;
    }

    const Akonadi::Collection &col = sourceIndex.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    // qCDebug(KORGANIZER_LOG) << col.displayName();
    //FIXME: we need a way to compare the path we get from LDAP to the folder in akonadi.
    //TODO: get it from the folder attribute
    if ((col.isValid() && mPerson.folderPaths.contains(col.displayName())) || mPerson.collections.contains(col.id())) {
        // qCDebug(KORGANIZER_LOG) << "reparenting " << col.displayName() << " to " << mPerson.name;
        return true;
    }
    return false;
}

bool PersonNode::isDuplicateOf(const QModelIndex &sourceIndex)
{
    return (sourceIndex.data(PersonRole).value<Person>().uid == mPerson.uid);
}

void PersonNode::update(const Node::Ptr &node)
{
    mPerson = node.staticCast<PersonNode>()->mPerson;
}

Person PersonNodeManager::person(const QModelIndex &sourceIndex)
{
    Person person;
    const Akonadi::Collection col = sourceIndex.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    if (col.isValid()) {
        CollectionIdentificationAttribute *attr = col.attribute<CollectionIdentificationAttribute>();
        if (attr && attr->collectionNamespace() == "usertoplevel") {
            person.name = col.displayName();
            person.mail = QString::fromUtf8(attr->mail());
            person.ou = QString::fromUtf8(attr->ou());
            person.uid = col.name();
            person.rootCollection = col.id();
        }
    }
    return person;
}

void PersonNodeManager::checkSourceIndex(const QModelIndex &sourceIndex)
{
    const Person &p = person(sourceIndex);
    if (p.rootCollection > -1) {
        model.addNode(ReparentingModel::Node::Ptr(new PersonNode(model, p)));
    }
}

void PersonNodeManager::updateSourceIndex(const QModelIndex &sourceIndex)
{
    const Person &p = person(sourceIndex);
    if (p.rootCollection > -1) {
        model.updateNode(ReparentingModel::Node::Ptr(new PersonNode(model, p)));
    }
}

void PersonNodeManager::checkSourceIndexRemoval(const QModelIndex &sourceIndex)
{
    const Person &p = person(sourceIndex);
    if (p.rootCollection > -1) {
        model.removeNode(PersonNode(model, p));
    }
}

Controller::Controller(ReparentingModel *personModel, ReparentingModel *searchModel, QObject *parent)
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
    Q_EMIT searchIsActive(!searchString.isEmpty());
    const bool showAllPersonalFolders = (searchString == QStringLiteral("*"));
    if (searchString.size() < 2 && !showAllPersonalFolders) {
        Q_EMIT searching(false);
        return;
    }

    if (!showAllPersonalFolders) {
        Q_EMIT searching(true);

        mPersonSearchJob = new PersonSearchJob(searchString, this);
        connect(mPersonSearchJob, &PersonSearchJob::personsFound, this, &Controller::onPersonsFound);
        connect(mPersonSearchJob, &PersonSearchJob::personUpdate, this, &Controller::onPersonUpdate);
        connect(mPersonSearchJob, &PersonSearchJob::result, this, &Controller::onPersonsFound);
        mPersonSearchJob->start();
    }

    mCollectionSearchJob = new CollectionSearchJob(searchString, QStringList() << QStringLiteral("text/calendar"), this);
    connect(mCollectionSearchJob, &CollectionSearchJob::result, this, &Controller::onCollectionsFound);
    mCollectionSearchJob->start();
}

void Controller::onCollectionsFound(KJob *job)
{
    mCollectionSearchJob = 0;
    if (!mPersonSearchJob) {
        Q_EMIT searching(false);
    }
    if (job->error()) {
        qCWarning(KORGANIZER_LOG) << job->errorString();
        return;
    }
    Q_FOREACH(const Akonadi::Collection & col, static_cast<CollectionSearchJob *>(job)->matchingCollections()) {
        CollectionNode *collectionNode = new CollectionNode(*mSearchModel, col);
        collectionNode->isSearchNode = true;
        //toggled by the checkbox, results in collection getting monitored
        // connect(&collectionNode->emitter, SIGNAL(enabled(bool,Akonadi::Collection)), this, SLOT(onCollectionEnabled(bool,Akonadi::Collection)));
        mSearchModel->addNode(ReparentingModel::Node::Ptr(collectionNode));
    }
}

void Controller::onPersonsFound(const QList<Person> &persons)
{
    Q_FOREACH(const Person & p, persons) {
        PersonNode *personNode = new PersonNode(*mSearchModel, p);
        personNode->isSearchNode = true;
        //toggled by the checkbox, results in person getting added to main model
        // connect(&personNode->emitter, SIGNAL(enabled(bool,Person)), this, SLOT(onPersonEnabled(bool,Person)));
        mSearchModel->addNode(ReparentingModel::Node::Ptr(personNode));
    }
}

void Controller::onPersonUpdate(const Person &person)
{
    PersonNode *personNode = new PersonNode(*mSearchModel, person);
    personNode->isSearchNode = true;
    mSearchModel->updateNode(ReparentingModel::Node::Ptr(personNode));
}

void Controller::onPersonsFound(KJob *job)
{
    mPersonSearchJob = 0;
    if (!mCollectionSearchJob) {
        Q_EMIT searching(false);
    }
    if (job->error()) {
        qCWarning(KORGANIZER_LOG) << job->errorString();
        return;
    }
}

static Akonadi::EntityTreeModel *findEtm(QAbstractItemModel *model)
{
    QAbstractProxyModel *proxyModel;
    while (model) {
        proxyModel = qobject_cast<QAbstractProxyModel *>(model);
        if (proxyModel && proxyModel->sourceModel()) {
            model = proxyModel->sourceModel();
        } else {
            break;
        }
    }
    return qobject_cast<Akonadi::EntityTreeModel *>(model);
}

void Controller::setCollectionState(const Akonadi::Collection &collection, CollectionState collectionState, bool recursive)
{
    //We removed the children first, so the children in the tree are removed before the parents
    if (recursive) {
        //We have to include all mimetypes since mimetypes are not available yet (they will be synced once the collectoins are referenced)
        Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(collection, Akonadi::CollectionFetchJob::Recursive, this);
        fetchJob->setProperty("collectionState", static_cast<int>(collectionState));
        fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
        connect(fetchJob, &Akonadi::CollectionFetchJob::result, this, &Controller::onPersonCollectionsFetched);
    }
    {
        Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(collection, Akonadi::CollectionFetchJob::Base, this);
        fetchJob->setProperty("collectionState", static_cast<int>(collectionState));
        fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
        connect(fetchJob, &Akonadi::CollectionFetchJob::result, this, &Controller::onPersonCollectionsFetched);
    }
}

void Controller::onPersonCollectionsFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(KORGANIZER_LOG) << "Failed to fetch collections " << job->errorString();
        return;
    }
    Akonadi::EntityTreeModel *etm = findEtm(mPersonModel);
    if (!etm) {
        qCWarning(KORGANIZER_LOG) << "Couldn't find etm";
        return;
    }

    const CollectionState collectionState = static_cast<CollectionState>(job->property("collectionState").toInt());
    Q_FOREACH(const Akonadi::Collection & col, static_cast<Akonadi::CollectionFetchJob *>(job)->collections()) {
        // qCDebug(KORGANIZER_LOG) << col.displayName() << "do enable " << enabled;
        Akonadi::Collection modifiedCollection = col;
        if (collectionState == Enabled) {
            modifiedCollection.setShouldList(Akonadi::Collection::ListDisplay, true);
        }
        if (collectionState == Disabled) {
            modifiedCollection.setShouldList(Akonadi::Collection::ListDisplay, false);
        }
        //HACK: We have no way of getting to the correct session as used by the etm,
        //and two concurrent jobs end up overwriting the enabled state of each other.
        etm->setCollectionReferenced(modifiedCollection, collectionState == Referenced);
    }
}

void Controller::addPerson(const Person &person)
{
    qCDebug(KORGANIZER_LOG) << person.uid << person.name << person.rootCollection;
    Person p = person;

    if (person.rootCollection == -1) {
        Baloo::PIM::CollectionQuery query;
        query.setNamespace(QStringList() << QStringLiteral("usertoplevel"));
        query.pathMatches(QStringLiteral("/Other Users/") + p.uid);
        query.setLimit(1);
        Baloo::PIM::ResultIterator it = query.exec();
        Akonadi::Collection::List collections;
        while (it.next()) {
            collections << Akonadi::Collection(it.id());
        }
        qCDebug(KORGANIZER_LOG) << "Found collections " << collections.size() << "for" << p.name;
        if (collections.size() == 1) {
            qCDebug(KORGANIZER_LOG) << "Set rootCollection=" << collections.at(0).id();
            p.rootCollection = collections.at(0).id();
        }
    }

    PersonNode *personNode = new PersonNode(*mPersonModel, p);
    personNode->setChecked(true);
    mPersonModel->addNode(ReparentingModel::Node::Ptr(personNode));

    if (p.rootCollection > -1) {
        setCollectionState(Akonadi::Collection(p.rootCollection), Referenced, true);
    } else {
        qCDebug(KORGANIZER_LOG) << "well this only a ldap search object without a collection";
        //TODO: use freebusy data into calendar
    }
}

void Controller::removePerson(const Person &person)
{
    qCDebug(KORGANIZER_LOG) << person.uid << person.name << person.rootCollection;
    mPersonModel->removeNode(PersonNode(*mPersonModel, person));

    if (person.rootCollection > -1) {
        setCollectionState(Akonadi::Collection(person.rootCollection), Disabled, true);
    } else {
        qCDebug(KORGANIZER_LOG) << "well this only a ldap search object without a collection";
        //TODO: delete freebusy data from calendar
    }
}

