/*
   SPDX-FileCopyrightText: 2016-2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <Akonadi/Collection>
#include <QObject>
#include <QPointer>
namespace Akonadi
{
class CollectionPropertiesDialog;
}
class KJob;
class AkonadiCollectionView;
class ManageShowCollectionProperties : public QObject
{
    Q_OBJECT
public:
    explicit ManageShowCollectionProperties(AkonadiCollectionView *collectionView, QObject *parent = nullptr);
    ~ManageShowCollectionProperties() override;

public:
    void showCollectionProperties();

private:
    void slotCollectionPropertiesContinued(KJob *job);
    void slotCollectionPropertiesFinished(KJob *job);
    QHash<Akonadi::Collection::Id, QPointer<Akonadi::CollectionPropertiesDialog>> mHashDialogBox;
    const QStringList mPages;
    AkonadiCollectionView *const mCollectionView;
};
