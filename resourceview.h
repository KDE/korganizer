/*
    This file is part of KOrganizer.

    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KORG_RESOURCEVIEW_H
#define KORG_RESOURCEVIEW_H

#include "calendarview.h"

#include <qlistview.h>

namespace KCal {
  class CalendarResources;
  class ResourceCalendar;
}
using namespace KCal;
class KListView;
class ResourceView;
class QPushButton;

class ResourceViewFactory : public CalendarViewExtension::Factory
{
  public:
    ResourceViewFactory( CalendarResources *calendar, CalendarView *view );

    CalendarViewExtension *create( QWidget * );

    ResourceView *resourceView() const;

  private:
    CalendarResources *mCalendar;
    CalendarView *mCalendarView;
    ResourceView *mResourceView;
};


class ResourceItem : public QCheckListItem
{
  public:
    ResourceItem( ResourceCalendar *resource, ResourceView *view, KListView *parent );
    ResourceItem( ResourceCalendar *resource, const QString &identifier,
                  const QString &label, ResourceView *view, ResourceItem *parent );

    ResourceCalendar *resource() { return mResource; }
    const QString& resourceIdentifier() { return mResourceIdentifier; }
    bool isSubresource() const { return mIsSubresource; }
    void createSubresourceItems();
    void setStandardResource( bool std );
    void update();

    virtual void paintCell(QPainter *p, const QColorGroup &cg,
      int column, int width, int alignment);

    void setResourceColor(QColor& color);
    QColor &resourceColor() {return mResourceColor;}

  protected:
    void stateChange( bool active );
    void setGuiState();
    QColor mResourceColor;

  private:
    ResourceCalendar *mResource;
    ResourceView *mResourceView;
    bool mBlockStateChange;
    bool mIsSubresource;
    QString mResourceIdentifier;
    bool mSubItemsCreated;
    bool mIsStandardResource;
};

/**
  This class provides a view of calendar resources.
*/
class ResourceView : public CalendarViewExtension
{
    Q_OBJECT
  public:
    ResourceView( CalendarResources *calendar, CalendarView *view,
                  QWidget *parent = 0, const char *name = 0 );
    ~ResourceView();

    CalendarResources *calendar() const { return mCalendar; }

    void updateView();

    void emitResourcesChanged();

    void requestClose( ResourceCalendar *resource );

    void showButtons( bool visible );

  public slots:
    void addResourceItem( ResourceCalendar *resource );
    void updateResourceItem( ResourceCalendar *resource );

  signals:
    void resourcesChanged();

  protected:
    ResourceItem *findItem( ResourceCalendar *resource );
    ResourceItem *findItemByIdentifier( const QString &identifier );
    ResourceItem *currentItem();

  protected slots:
    void addResource();
    void removeResource();
    void editResource();
    void currentChanged( QListViewItem *lvitem );
    void slotSubresourceAdded( ResourceCalendar *resource, const QString &type,
                               const QString &identifier, const QString &label );

    void slotSubresourceRemoved( ResourceCalendar *resource, const QString &type,
                                 const QString &identifier );
    void closeResource( ResourceCalendar *resource );

    void contextMenuRequested ( QListViewItem *lvitem, const QPoint &pos, int );

    void assignColor();
    void disableColor();
    void showInfo();

    void reloadResource();
    void saveResource();

    void setStandard();
    void updateResourceList();

  private:
    KListView *mListView;
    CalendarResources *mCalendar;
    CalendarView *mCalendarView;
    QPushButton *mAddButton;
    QPushButton *mDeleteButton;
    QPushButton *mEditButton;
    QPtrList<ResourceCalendar> mResourcesToClose;
};

#endif
