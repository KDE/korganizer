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

#include <QTreeWidget>
#include <QList>

namespace KCal {
  class AkonadiCalendar;
  //class AkonadiCalendar;
  //class ResourceCalendar;
}

namespace Akonadi {
  class Collection;
  class StandardActionManager;
  class CollectionView;
}

class KAction;
class KJob;
class AkonadiCollectionView;

class AkonadiCollectionViewFactory : public CalendarViewExtension::Factory
{
  public:
    AkonadiCollectionViewFactory( KCal::AkonadiCalendar *calendar, CalendarView *view );
    virtual ~AkonadiCollectionViewFactory(){}

    KCal::AkonadiCalendar* calendar() const;
    CalendarView* view() const;
    AkonadiCollectionView* collectionView() const;

    CalendarViewExtension *create( QWidget * );

  private:
    KCal::AkonadiCalendar *mCalendar;
    CalendarView *mView;
    AkonadiCollectionView *mAkonadiCollectionView;
};

/**
  This class provides a view of calendar resources.
*/
class AkonadiCollectionView : public CalendarViewExtension
{
  Q_OBJECT
  public:
    AkonadiCollectionView( AkonadiCollectionViewFactory *factory, KCal::AkonadiCalendar *calendar, QWidget *parent = 0 );
    ~AkonadiCollectionView();
    AkonadiCalendar *calendar() const { return mCalendar; }

    void updateView();

#if 0 //sebsauer
    void emitResourcesChanged();
    void requestClose( ResourceCalendar * );
    ResourceItem *findItemByIdentifier( const QString &id );
  public slots:
    void addResourceItem( ResourceCalendar *, bool emitSignal = true );
    void updateResourceItem( ResourceCalendar * );
  signals:
    void resourcesChanged();
  protected:
    ResourceItem *findItem( ResourceCalendar * );
    ResourceItem *currentItem();
  protected slots:
    void addResource();
    void removeResource();
    void editResource();
    void slotSubresourceAdded( ResourceCalendar *, const QString &,
                               const QString &resource, const QString &label );
    void slotSubresourceRemoved( ResourceCalendar *, const QString &,
                                 const QString & );
    void closeResource( ResourceCalendar * );
    void showContextMenu( const QPoint &pos );
    void slotAddButtonClicked();
    void assignColor();
    void disableColor();
    void showInfo();
    void reloadResource();
    void saveResource();
    void setStandard();
    void updateResourceList();
    void slotItemClicked( QTreeWidgetItem *, int );
    void currentChanged();
#else
  private Q_SLOTS:
    void selectionChanged();

    void newCalendar();
    void newCalendarDone( KJob* );

    void deleteCalendar();
    void deleteCalendarDone( KJob* );
#endif

  private:
    AkonadiCollectionViewFactory *mFactory;
#if 0 //sebsauer
    /*
     * Holds:
     * the ResourceItem that's selected when adding a resource,
     * 0 if no item is selected,
     * 0 if an item is selected but the user pressed RMB in the blank space
     */
    ResourceItem *mSelectedParent;
    QTreeWidget *mListView;
#endif
    KCal::AkonadiCalendar *mCalendar;
    Akonadi::StandardActionManager* mActionManager;
    Akonadi::CollectionView *mCollectionview;
#if 0
    QList<ResourceCalendar*> mResourcesToClose;
    QAbstractButton *mAddButton, *mEditButton, *mDeleteButton;
#endif
    KAction *mCreateAction;
    KAction *mDeleteAction;
};

#endif
