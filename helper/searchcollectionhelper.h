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

#ifndef KORG_SEARCHCOLLECTIONHELPER_H
#define KORG_SEARCHCOLLECTIONHELPER_H

#include <QObject>
#include <QString>

#include <AkonadiCore/Collection>
#include <KCalCore/Attendee>
#include <KIdentityManagement/KIdentityManagement/IdentityManager>

class KJob;

namespace KOrg
{

class SearchCollectionHelper: public QObject
{
    Q_OBJECT
public:
    explicit SearchCollectionHelper(QObject *parent = Q_NULLPTR);

private Q_SLOTS:
    void onSearchCollectionsFetched(KJob *job);
    void updateOpenInvitation();
    void updateDeclinedInvitation();

    void createSearchJobFinished(KJob *job);
    void modifyResult(KJob *job);

private:
    void setupSearchCollections();
    void updateSearchCollection(Akonadi::Collection col, KCalCore::Attendee::PartStat status, const QString &name, const QString &displayName);

private:
    KIdentityManagement::IdentityManager mIdentityManager;
    Akonadi::Collection mOpenInvitationCollection;
    Akonadi::Collection mDeclineCollection;
};
}
#endif
