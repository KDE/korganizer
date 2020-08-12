/*
   SPDX-FileCopyrightText: 2016-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "manageshowcollectionproperties.h"
#include "akonadicollectionview.h"
#include "korganizer_debug.h"
#include <AkonadiWidgets/CollectionPropertiesDialog>
#include <AkonadiWidgets/CollectionMaintenancePage>
#include <AkonadiCore/CollectionAttributesSynchronizationJob>
#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/CollectionFetchScope>

ManageShowCollectionProperties::ManageShowCollectionProperties(
    AkonadiCollectionView *collectionView, QObject *parent)
    : QObject(parent)
    , mCollectionView(collectionView)
{
    mPages = QStringList() << QStringLiteral("CalendarSupport::CollectionGeneralPage")
                           << QStringLiteral("Akonadi::CachePolicyPage")
                           << QStringLiteral("PimCommon::CollectionAclPage")
                           << QStringLiteral("Akonadi::CollectionMaintenancePage");
}

ManageShowCollectionProperties::~ManageShowCollectionProperties()
{
}

void ManageShowCollectionProperties::showCollectionProperties()
{
    const Akonadi::Collection col = mCollectionView->currentCalendar();
    const Akonadi::Collection::Id id = col.id();
    QPointer<Akonadi::CollectionPropertiesDialog> dlg = mHashDialogBox.value(id);
    if (dlg) {
        dlg->activateWindow();
        dlg->raise();
        return;
    }
    Akonadi::CollectionAttributesSynchronizationJob *sync
        = new Akonadi::CollectionAttributesSynchronizationJob(col);
    sync->setProperty("collectionId", id);
    connect(sync, &Akonadi::CollectionAttributesSynchronizationJob::result,
            this, &ManageShowCollectionProperties::slotCollectionPropertiesContinued);
    sync->start();
}

void ManageShowCollectionProperties::slotCollectionPropertiesContinued(KJob *job)
{
    if (job) {
        Akonadi::CollectionAttributesSynchronizationJob *sync
            = qobject_cast<Akonadi::CollectionAttributesSynchronizationJob *>(job);
        Q_ASSERT(sync);
        if (sync->property("collectionId") != mCollectionView->currentCalendar().id()) {
            return;
        }
    }
    Akonadi::CollectionFetchJob *fetch = new Akonadi::CollectionFetchJob(
        mCollectionView->currentCalendar(),
        Akonadi::CollectionFetchJob::Base);
    fetch->fetchScope().setIncludeStatistics(true);
    connect(fetch, &KJob::result,
            this, &ManageShowCollectionProperties::slotCollectionPropertiesFinished);
}

void ManageShowCollectionProperties::slotCollectionPropertiesFinished(KJob *job)
{
    if (!job) {
        return;
    }

    Akonadi::CollectionFetchJob *fetch = qobject_cast<Akonadi::CollectionFetchJob *>(job);
    Q_ASSERT(fetch);
    if (fetch->collections().isEmpty()) {
        qCWarning(KORGANIZER_LOG) << "no collection";
        return;
    }

    const Akonadi::Collection::List collections = fetch->collections();
    if (!collections.isEmpty()) {
        const Akonadi::Collection collection = collections.first();

        QPointer<Akonadi::CollectionPropertiesDialog> dlg
            = new Akonadi::CollectionPropertiesDialog(collection, mPages, mCollectionView);
        dlg->setWindowTitle(i18nc("@title:window", "Properties of Calendar Folder %1",
                                  collection.name()));

        dlg->show();
        mHashDialogBox.insert(collection.id(), dlg);
    }
}
