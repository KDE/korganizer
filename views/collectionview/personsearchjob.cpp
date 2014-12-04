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
#include "personsearchjob.h"

#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionFetchScope>
#include <KLocale>
#include <baloo/pim/collectionquery.h>
#include <akonadi/collectionidentificationattribute.h>

PersonSearchJob::PersonSearchJob(const QString& searchString, QObject* parent)
    : KJob(parent),
    mSearchString(searchString)
{
    connect(&mLdapSearch, SIGNAL(searchData(const QList<KLDAP::LdapResultObject> &)),
            SLOT(onLDAPSearchData(const QList<KLDAP::LdapResultObject> &)));

    connect(&mLdapSearch, SIGNAL(searchDone()),
            SLOT(onLDAPSearchDone()));
}

PersonSearchJob::~PersonSearchJob()
{
    mLdapSearch.cancelSearch();
}

bool PersonSearchJob::kill(KJob::KillVerbosity verbosity)
{
    mLdapSearch.cancelSearch();
    return KJob::kill(verbosity);
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

    mCollectionSearchDone = false;
    mLdapSearchDone = false;

    mLdapSearch.startSearch(QLatin1String("*") + mSearchString);

    if (collections.isEmpty()) {
        //We didn't find anything
        mCollectionSearchDone = true;
        return;
    }

    Akonadi::CollectionFetchJob *fetchJob = new Akonadi::CollectionFetchJob(collections, Akonadi::CollectionFetchJob::Base, this);
    fetchJob->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
    fetchJob->fetchScope().setListFilter(Akonadi::CollectionFetchScope::NoFilter);
    connect(fetchJob, SIGNAL(collectionsReceived(Akonadi::Collection::List)), this, SLOT(onCollectionsReceived(Akonadi::Collection::List)));
    connect(fetchJob, SIGNAL(result(KJob*)), this, SLOT(onCollectionsFetched(KJob*)));

    //The IMAP resource should add a "Person" attribute to the collections in the person namespace,
    //the ldap query can then be used to update the name (entitydisplayattribute) for the person.
}

void PersonSearchJob::onLDAPSearchData(const QList< KLDAP::LdapResultObject > &list)
{
    QList<Person> persons;
    Q_FOREACH(const KLDAP::LdapResultObject &item, list) {
        Person person;
        person.name = QString::fromUtf8(item.object.value(QLatin1String("cn")));
        person.mail = QString::fromUtf8(item.object.value(QLatin1String("mail")));

        const int depth = item.object.dn().depth();
        for ( int i = 0; i < depth; ++i ) {
            const QString rdnStr = item.object.dn().rdnString(i);
            if ( rdnStr.startsWith(QLatin1String("ou="), Qt::CaseInsensitive) ) {
                person.ou = rdnStr.mid(3);
                break;
            }
        }
        const QStringList &parts = person.mail.split(QLatin1Char('@'));
        if (parts.count() == 2) {
            const QString &uid = parts.at(0);
            person.uid = uid;
            if (mMatches.contains(uid)) {
                const Person &p = mMatches.value(uid);
                if (p.mail != person.mail ) {
                    if (p.rootCollection > -1) {
                        person.rootCollection = p.rootCollection;
                        person.updateDisplayName = p.updateDisplayName;
                        updatePersonCollection(person);
                        mMatches.insert(uid, person);
                    } else {
                        kWarning() << "That should not happen: we found two times persons with the same uid ("<< uid << "), but differnet name:" << p.name << "vs" << person.name;
                    }
                }
            } else {            //New person found
                mMatches.insert(uid, person);
                persons << person;
            }
        } else {
            kWarning() << item.object.dn().toString() << ": invalid email address" << person.mail;
        }
    }
    if (persons.count() > 0) {
        emit personsFound(persons);
    }
}

void PersonSearchJob::onLDAPSearchDone()
{
    mLdapSearchDone = true;
    if (mCollectionSearchDone) {
        emitResult();
    }
}

void PersonSearchJob::onCollectionsReceived(const Akonadi::Collection::List &list)
{
    QList<Person> persons;
    Q_FOREACH(const Akonadi::Collection &col, list) {
        Person person;
        const QString &uid = col.name();
        const CollectionIdentificationAttribute *const attr = col.attribute<CollectionIdentificationAttribute>();
        const Akonadi::EntityDisplayAttribute *const displayname = col.attribute<Akonadi::EntityDisplayAttribute>();
        person.rootCollection = col.id();
        person.uid = uid;
        if (attr) {
            person.ou = QString::fromUtf8(attr->ou());
            person.mail = QString::fromUtf8(attr->mail());
            person.name = QString::fromUtf8(attr->identifier());
            if (!displayname || displayname->displayName().isEmpty() || displayname->displayName() == person.name) {
                person.updateDisplayName = true;
            }
        } else {
            person.name = col.displayName();
            if (!displayname || displayname->displayName().isEmpty()) {
                person.updateDisplayName = true;
            }
        }
        if (mMatches.contains(uid)) {
            Person p = mMatches.value(uid);
            if (p.rootCollection > -1) {
                //two collection with the same uid ?! 
                kWarning() << "Two collections match to same person" << p.rootCollection << person.rootCollection;
            } else if (p.mail != person.mail) {
                p.rootCollection = person.rootCollection;
                p.updateDisplayName = person.updateDisplayName;
                updatePersonCollection(p);
            } else {
                mMatches.insert(uid, person);
                emit personUpdate(person);
            }
        } else {
            mMatches.insert(uid, person);
            persons << person;
        }
    }

    if (persons.count() > 0) {
        emit personsFound(persons);
    }
}

void PersonSearchJob::updatePersonCollection(const Person &person)
{
    Akonadi::Collection c(person.rootCollection);
    CollectionIdentificationAttribute *identification = c.attribute<CollectionIdentificationAttribute>(Akonadi::Entity::AddIfMissing);

    if (person.updateDisplayName) {
        Akonadi::EntityDisplayAttribute *displayname  = c.attribute<Akonadi::EntityDisplayAttribute >(Akonadi::Entity::AddIfMissing);
        displayname->setDisplayName(person.name);
    }

    //identification->setIdentifier("Other Users/" + person.uid);
    identification->setIdentifier(person.name.toUtf8());
    identification->setName(person.name.toUtf8());
    identification->setCollectionNamespace("usertoplevel");
    identification->setMail(person.mail.toUtf8());
    identification->setOu(person.ou.toUtf8());

    Akonadi::CollectionModifyJob *job = new Akonadi::CollectionModifyJob( c, this );
    connect(job, SIGNAL(result(KJob*)), this, SLOT(modifyResult(KJob*)));
}

void PersonSearchJob::onCollectionsFetched(KJob *job)
{
    if (job->error()) {
        kWarning() << job->errorString();
    }
    mCollectionSearchDone = true;
    if (mLdapSearchDone) {
        emitResult();
    }
}

QList<Person> PersonSearchJob::matches() const
{
    return mMatches.values();
}

void PersonSearchJob::modifyResult(KJob *job)
{
    if (job->error()) {
        kWarning() << job->errorString();
        return;
    }

    const Akonadi::CollectionModifyJob *modifyJob = static_cast<Akonadi::CollectionModifyJob*>(job);
    const Akonadi::Collection &col = modifyJob->collection();

    const CollectionIdentificationAttribute *const attr = col.attribute<CollectionIdentificationAttribute>();
    const Akonadi::EntityDisplayAttribute *const displayname = col.attribute<Akonadi::EntityDisplayAttribute>();
    const QString &uid = col.name();
    Person &person = mMatches[col.name()];
    person.rootCollection = col.id();
    person.uid = uid;
    if (attr) {
        person.ou = QString::fromUtf8(attr->ou());
        person.mail = QString::fromUtf8(attr->mail());
        person.name = QString::fromUtf8(attr->identifier());
        if (!displayname || displayname->displayName().isEmpty() || displayname->displayName() == person.name) {
            person.updateDisplayName = true;
        }
    }
    kDebug() << "modified person to" << person.uid << person.name << person.rootCollection;

    mMatches.insert(person.uid, person);
    emit personUpdate(person);
}
