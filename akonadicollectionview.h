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
#include <AkonadiCore/Collection>

class AkonadiCollectionView;

namespace Akonadi {
  class EntityTreeView;
  class EntityTreeModel;
  class StandardCalendarActionManager;
}

class KAction;
class KJob;
class QAbstractProxyModel;
class QModelIndex;

/**
 * The factory for AkonadiCollectionView instances.
 */
class AkonadiCollectionViewFactory : public CalendarViewExtension::Factory
{
  public:
    explicit AkonadiCollectionViewFactory( CalendarView *view );

    CalendarView *view() const;
    AkonadiCollectionView *collectionView() const;

    CalendarViewExtension *create( QWidget * );

  private:
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
    explicit AkonadiCollectionView( CalendarView *view, bool hasContextMenu = true,
                                    QWidget *parent = 0 );
    ~AkonadiCollectionView();

    Akonadi::EntityTreeView *view() const;

    KCheckableProxyModel *collectionSelectionProxyModel() const;
    void setCollectionSelectionProxyModel( KCheckableProxyModel * );

    Akonadi::Collection selectedCollection() const;
    Akonadi::Collection::List checkedCollections() const;
    bool isChecked(const Akonadi::Collection &) const;

  Q_SIGNALS:
    void resourcesChanged( bool enabled );
    void resourcesAddedRemoved();
    void defaultResourceChanged( const Akonadi::Collection & );
    void colorsChanged();

  private Q_SLOTS:
    void updateView();
    void updateMenu();
    void restoreTreeState();
    void checkNewCalendar( const QModelIndex &parent, int begin, int end );

    void newCalendar();
    void newCalendarDone( KJob * );

    void deleteCalendar();
    void deleteCalendarDone( KJob * );
    void rowsInserted( const QModelIndex &, int, int );
    void assignColor();
    void disableColor();
    void setDefaultCalendar();

  private:
    Akonadi::EntityTreeModel *entityTreeModel() const;

    Akonadi::StandardCalendarActionManager *mActionManager;
    Akonadi::EntityTreeView *mCollectionView;
    QAbstractProxyModel *mBaseModel;
    KCheckableProxyModel *mSelectionProxyModel;
    KAction *mAssignColor;
    KAction *mDisableColor;
    KAction *mDefaultCalendar;
    bool mNotSendAddRemoveSignal;
    bool mWasDefaultCalendar;
    bool mHasContextMenu;
  };

#endif
