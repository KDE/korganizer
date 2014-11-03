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
#include "collectionsearchjob.h"

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <baloo/pim/collectionquery.h>

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
            break;
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

