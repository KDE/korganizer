/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2015 Sandro Knauß <knauss@kolabsys.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include <QObject>
#include <QString>

#include <Akonadi/Collection>
#include <KCalendarCore/Attendee>
#include <KIdentityManagementCore/IdentityManager>

class KJob;

namespace KOrg
{
class SearchCollectionHelper : public QObject
{
    Q_OBJECT
public:
    explicit SearchCollectionHelper(QObject *parent = nullptr);

private:
    void onSearchCollectionsFetched(KJob *job);
    void updateOpenInvitation();
    void updateDeclinedInvitation();

    void createSearchJobFinished(KJob *job);
    void modifyResult(KJob *job);

    void setupSearchCollections();
    void updateSearchCollection(Akonadi::Collection col, KCalendarCore::Attendee::PartStat status, const QString &name, const QString &displayName);

private:
    KIdentityManagementCore::IdentityManager *const mIdentityManager;
    Akonadi::Collection mOpenInvitationCollection;
    Akonadi::Collection mDeclineCollection;
};
}
