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

#include "resourceview.h"

#include <klistview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>

#include <qlayout.h>

using namespace KCal;

ResourceItem::ResourceItem( ResourceCalendar *resource, ResourceView *view,
                            KListView *parent ) :
      QCheckListItem( parent, resource->resourceName(), CheckBox ),
      mResource( resource ), mView( view )
{
  setOn( mResource->isActive() );
}

void ResourceItem::stateChange( bool active )
{
  if ( active ) {
    mResource->open();
    mResource->load();
  } else {
    mResource->sync();
    mResource->close();
  }
  mResource->setActive( active );

  mView->emitResourcesChanged();
}

ResourceView::ResourceView( KCal::CalendarResourceManager *manager,
                            QWidget *parent, const char *name )
  : QWidget( parent, name ),
    mManager( manager )
{
  mListView = new KListView( this );
  mListView->addColumn( i18n("Calendar") );

  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->addWidget( mListView );

  updateView();
}

ResourceView::~ResourceView()
{
}

void ResourceView::updateView()
{
  mListView->clear();

  KCal::CalendarResourceManager::Iterator it;
  for( it = mManager->begin(); it != mManager->end(); ++it ) {
    new ResourceItem(  (*it), this, mListView );
  }
}

void ResourceView::emitResourcesChanged()
{
  emit resourcesChanged();
}


#include "resourceview.moc"
