/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KORG_RESOURCEVIEW_H
#define KORG_RESOURCEVIEW_H

#include "customlistviewitem.h"

#include <libkcal/resourcecalendar.h>

#include <klistview.h>

#include <qwidget.h>
#include <qpushbutton.h>

class KListView;
class ResourceView;

class ResourceItem : public QCheckListItem
{
  public:
    ResourceItem( KCal::ResourceCalendar *resource, ResourceView *view,
                  KListView *parent );

    KCal::ResourceCalendar *resource() { return mResource; }

  protected:
    void stateChange( bool active );

  private:
    KCal::ResourceCalendar *mResource;
    ResourceView *mView;
};

/**
  This class provides a view of calendar resources.
*/
class ResourceView : public QWidget
{
    Q_OBJECT
  public:
    ResourceView( KCal::CalendarResourceManager *manager, QWidget *parent = 0,
	          const char *name = 0);
    ~ResourceView();

    void updateView();

    void emitResourcesChanged();

  signals:
    void resourcesChanged();

  private slots:
    void addResource();
    void removeResource();
    void editResource();
    void currentChanged( QListViewItem* );

  private:
    KListView *mListView;
    KCal::CalendarResourceManager *mManager;
    QPushButton *add, *del, *edit;
};

#endif
