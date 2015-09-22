/*
  This file is part of KOrganizer.

  Copyright (c) 2015 Sandro Knauß <knauss@kolabsys.com>

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

#include "searchcollectionhelper.h"

#include <AkonadiCore/SearchCreateJob>
#include <AkonadiCore/CollectionModifyJob>
#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/SearchQuery>
#include <AkonadiCore/persistentsearchattribute.h>
#include <AkonadiCore/entitydisplayattribute.h>
#include <AkonadiCore/collectionfetchscope.h>

#include <KCalCore/Todo>
#include <KCalCore/Event>
#include <KCalCore/Journal>

#include <klocalizedstring.h>
#include "korganizer_debug.h"

using namespace KOrg;

SearchCollectionHelper::SearchCollectionHelper(QObject *parent)
    : QObject(parent)
    , mIdentityManager(/*ro=*/ true)
{
    setupSearchCollections();
    connect(&mIdentityManager, static_cast<void (KIdentityManagement::IdentityManager::*)()>(&KIdentityManagement::IdentityManager::changed), this, &SearchCollectionHelper::updateOpenInvitation);
    connect(&mIdentityManager, static_cast<void (KIdentityManagement::IdentityManager::*)()>(&KIdentityManagement::IdentityManager::changed), this, &SearchCollectionHelper::updateDeclinedInvitation);
}

void SearchCollectionHelper::setupSearchCollections()
{
    //Collection "Search", has always ID 1
    Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(Akonadi::Collection(1), Akonadi::CollectionFetchJob::FirstLevel);
    fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
    connect(fetchJob, &Akonadi::CollectionFetchJob::result, this, &SearchCollectionHelper::onSearchCollectionsFetched);
}

void SearchCollectionHelper::onSearchCollectionsFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(KORGANIZER_LOG) << "Search failed: " << job->errorString();
    } else {
        Akonadi::CollectionFetchJob *fetchJob = static_cast<Akonadi::CollectionFetchJob *>(job);
        Q_FOREACH (const Akonadi::Collection &col, fetchJob->collections()) {
            if (col.name() == QLatin1String("OpenInvitations")) {
                mOpenInvitationCollection = col;
            } else if (col.name() == QLatin1String("DeclinedInvitations")) {
                mDeclineCollection = col;
            }
        }
    }
    updateOpenInvitation();
    updateDeclinedInvitation();
}

void SearchCollectionHelper::updateSearchCollection(Akonadi::Collection col, KCalCore::Attendee::PartStat status, const QString &name, const QString &displayName)
{
    // Update or create search collections

    Akonadi::SearchQuery query(Akonadi::SearchTerm::RelOr);
    foreach (const QString email, mIdentityManager.allEmails()) {
        if (!email.isEmpty()) {
            query.addTerm(Akonadi::IncidenceSearchTerm(Akonadi::IncidenceSearchTerm::PartStatus, QString(email + QString::number(status))));
        }
    }

    if (!col.isValid()) {
        Akonadi::SearchCreateJob *job = new Akonadi::SearchCreateJob(name, query);
        job->setRemoteSearchEnabled(false);
        job->setSearchMimeTypes(QStringList() << KCalCore::Event::eventMimeType()
                                << KCalCore::Todo::todoMimeType()
                                << KCalCore::Journal::journalMimeType());
        connect(job, &Akonadi::SearchCreateJob::result, this, &SearchCollectionHelper::createSearchJobFinished);
        qCDebug(KORGANIZER_LOG) << "We have to create a " << name << " virtual Collection";
    } else {
        Akonadi::PersistentSearchAttribute *attribute = col.attribute<Akonadi::PersistentSearchAttribute>(Akonadi::Collection::AddIfMissing);
        Akonadi::EntityDisplayAttribute *displayname = col.attribute<Akonadi::EntityDisplayAttribute >(Akonadi::Collection::AddIfMissing);
        attribute->setQueryString(QString::fromLatin1(query.toJSON()));
        attribute->setRemoteSearchEnabled(false);
        displayname->setDisplayName(displayName);
        col.setEnabled(true);
        Akonadi::CollectionModifyJob *job = new Akonadi::CollectionModifyJob(col, this);
        connect(job, &Akonadi::CollectionModifyJob::result, this, &SearchCollectionHelper::modifyResult);
        qCDebug(KORGANIZER_LOG) << "updating " << name << " (" << col.id() << ") virtual Collection";
        qCDebug(KORGANIZER_LOG) << query.toJSON();
    }
}

void SearchCollectionHelper::updateDeclinedInvitation()
{
    updateSearchCollection(mDeclineCollection, KCalCore::Attendee::Declined,
                           QStringLiteral("DeclinedInvitations"),
                           i18nc("A collection of all declined invidations.", "Declined Invitations"));
}

void SearchCollectionHelper::updateOpenInvitation()
{
    updateSearchCollection(mOpenInvitationCollection, KCalCore::Attendee::NeedsAction,
                           QStringLiteral("OpenInvitations"),
                           i18nc("A collection of all open invidations.", "Open Invitations"));
}

void SearchCollectionHelper::createSearchJobFinished(KJob *job)
{
    Akonadi::SearchCreateJob *createJob = qobject_cast<Akonadi::SearchCreateJob *>(job);
    const Akonadi::Collection searchCollection = createJob->createdCollection();
    if (job->error()) {
        qCWarning(KORGANIZER_LOG) << "Error occurred " << searchCollection.name() << job->errorString();
        return;
    }
    qCDebug(KORGANIZER_LOG) << "Created search folder successfully " << searchCollection.name();

    if (searchCollection.name() == QLatin1String("OpenInvitations")) {
        mOpenInvitationCollection = searchCollection;
        updateOpenInvitation();
    } else if (searchCollection.name() == QLatin1String("DeclinedInvitations")) {
        mDeclineCollection = searchCollection;
        updateDeclinedInvitation();
    }
}

void SearchCollectionHelper::modifyResult(KJob *job)
{
    if (job->error()) {
        qCWarning(KORGANIZER_LOG) << "Error occurred " << job->errorString();
    } else {
        qCDebug(KORGANIZER_LOG) << "modify was successfull";
    }
}
