/*
    This file is part of KOrganizer.

    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KORG_RESOURCEVIEW_H
#define KORG_RESOURCEVIEW_H

#include "calendarview.h"

#include <libkcal/resourcecalendar.h>
#include <qlistview.h>

namespace KCal {
class CalendarResources;
}
using namespace KCal;
class KListView;
class ResourceView;
class QPushButton;

class ResourceViewFactory : public CalendarViewExtension::Factory
{
  public:
    ResourceViewFactory( KCal::CalendarResources *calendar,
                         CalendarView *view );

    CalendarViewExtension *create( QWidget * );

    ResourceView *resourceView() const;

  private:
    KCal::CalendarResources *mCalendar;
    CalendarView *mView;
    ResourceView *mResourceView;
};


class ResourceItem : public QCheckListItem
{
  public:
    ResourceItem( KCal::ResourceCalendar *resource, ResourceView *view,
                  KListView *parent );
    ResourceItem( KCal::ResourceCalendar *resource, const QString& sub,
                  const QString& label, ResourceView *view, 
                  ResourceItem* parent );

    KCal::ResourceCalendar *resource() { return mResource; }
    const QString& resourceIdentifier() { return mResourceIdentifier; }
    bool isSubresource() const { return mIsSubresource; }
    void createSubresourceItems();

    void update();

  protected:
    void stateChange( bool active );

    void setGuiState();

  private:
    KCal::ResourceCalendar *mResource;
    ResourceView *mView;
    bool mBlockStateChange;
    bool mIsSubresource;
    QString mResourceIdentifier;
    bool mSubItemsCreated;
};

/**
  This class provides a view of calendar resources.
*/
class ResourceView : public CalendarViewExtension
{
    Q_OBJECT
  public:
    ResourceView( KCal::CalendarResources *calendar, QWidget *parent = 0,
                  const char *name = 0 );
    ~ResourceView();

    KCal::CalendarResources *calendar() const { return mCalendar; }

    void updateView();

    void emitResourcesChanged();

    void requestClose( ResourceCalendar * );

    void showButtons( bool visible );

  public slots:
    void addResourceItem( ResourceCalendar * );
    void updateResourceItem( ResourceCalendar * );

  signals:
    void resourcesChanged();

  protected:
    ResourceItem *findItem( ResourceCalendar * );
    ResourceItem *findItemByIdentifier( const QString& id );
    ResourceItem *currentItem();

  protected slots:
    void addResource();
    void removeResource();
    void editResource();
    void currentChanged( QListViewItem* );
    void slotSubresourceAdded( ResourceCalendar *, const QString &,
                               const QString &resource,const QString& label );
    // FIXME proko2: merge once we are back in head by porting imap resource
    void slotSubresourceAdded( ResourceCalendar *, const QString &,
                               const QString &resource );

    void slotSubresourceRemoved( ResourceCalendar *, const QString &,
                                 const QString &resource );
    void closeResource( ResourceCalendar * );

    void contextMenuRequested ( QListViewItem *i, const QPoint &pos, int );

    void showInfo();
  
    void reloadResource();
    void saveResource();

  private:
    KListView *mListView;
    KCal::CalendarResources *mCalendar;
    QPushButton *mAddButton;
    QPushButton *mDeleteButton;
    QPushButton *mEditButton;
    QPtrList<ResourceCalendar> mResourcesToClose;
};

#endif
