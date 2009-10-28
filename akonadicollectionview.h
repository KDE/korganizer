/*
  This file is part of KOrganizer.

  Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2009 Sebastian Sauer <sebsauer@kdab.net>

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
#ifndef KORG_AKONADICOLLECTIONVIEW_H
#define KORG_AKONADICOLLECTIONVIEW_H

#include "calendarview.h"

#include <Akonadi/Collection>

#include <QTreeWidget>
#include <QList>

class QItemSelectionModel;

namespace KOrg {
    class AkonadiCalendar;
}

namespace Akonadi {
  class CalendarModel;
  class Collection;
  class CollectionSelectionProxyModel;
  class EntityTreeView;
  class EntityModelStateSaver;
  class StandardActionManager;
}

class KAction;
class KJob;
class AkonadiCollectionView;

/**
 * The factory for AkonadiCollectionView instances.
 */
class AkonadiCollectionViewFactory : public CalendarViewExtension::Factory
{
  public:
  AkonadiCollectionViewFactory( Akonadi::CalendarModel *model, CalendarView *view );
    virtual ~AkonadiCollectionViewFactory(){}

    CalendarView* view() const;
    AkonadiCollectionView* collectionView() const;

    CalendarViewExtension *create( QWidget * );

  private:
    Akonadi::CalendarModel *mModel;
    CalendarView *mView;
    AkonadiCollectionView *mAkonadiCollectionView;
};

/**
 * This class provides a view of calendar resources.
 */
class AkonadiCollectionView : public CalendarViewExtension
{
  Q_OBJECT
  public:
    AkonadiCollectionView( AkonadiCollectionViewFactory *factory, Akonadi::CalendarModel* model, QWidget *parent = 0 );
    ~AkonadiCollectionView();

private:
    Akonadi::CollectionSelection* collectionSelection() const;
public:
    Akonadi::EntityTreeView* view() const;

    Akonadi::CollectionSelectionProxyModel* collectionSelectionProxyModel() const;
    void setCollectionSelectionProxyModel( Akonadi::CollectionSelectionProxyModel* );

  signals:
    void resourcesChanged(bool enabled);
  private:
    void updateView();

  private Q_SLOTS:
    void selectionChanged();

    void newCalendar();
    void newCalendarDone( KJob* );

    void deleteCalendar();
    void deleteCalendarDone( KJob* );

  private:
    Akonadi::StandardActionManager* mActionManager;
    Akonadi::EntityTreeView *mCollectionview;
    QAbstractItemModel* mBaseModel;
    Akonadi::CollectionSelectionProxyModel *mSelectionProxyModel;
    //QList<ResourceCalendar*> mResourcesToClose;
    //QAbstractButton *mAddButton, *mEditButton, *mDeleteButton;
    KAction *mCreateAction;
    KAction *mDeleteAction;
    Akonadi::CollectionSelection *mCollectionSelection;
  };

#endif
