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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "resourceview.h"

#include <klistview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kresources/resource.h>
#include <kresources/configdialog.h>
#include <kinputdialog.h>

#include <qlayout.h>
#include <qlabel.h>

using namespace KCal;

ResourceViewFactory::ResourceViewFactory( KCal::CalendarResources *calendar,
                                          CalendarView *view )
{
  mCalendar = calendar;
  mView = view;
}

CalendarViewExtension *ResourceViewFactory::create( QWidget *parent )
{
  ResourceView *view = new ResourceView( mCalendar->resourceManager(), parent );
  view->updateView();
  QObject::connect( view, SIGNAL( resourcesChanged() ),
                    mView, SLOT( updateView() ) );
  QObject::connect( mCalendar, SIGNAL( signalResourceAdded( ResourceCalendar * ) ),
                    view, SLOT( addResourceItem( ResourceCalendar * ) ) );

  return view;
}

ResourceItem::ResourceItem( ResourceCalendar *resource, ResourceView *view,
                            KListView *parent )
  : QCheckListItem( parent, resource->resourceName(), CheckBox ),
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
    mResource->save();
    mResource->close();
  }
  mResource->setActive( active );

  mView->emitResourcesChanged();
}

ResourceView::ResourceView( KCal::CalendarResourceManager *manager,
                            QWidget *parent, const char *name )
  : CalendarViewExtension( parent, name ),
    mManager( manager )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );

  mListView = new KListView( this );
  mListView->addColumn( i18n("Calendar") );
  mListView->setResizeMode( QListView::LastColumn );
  topLayout->addWidget( mListView );

  QHBox *buttonBox = new QHBox( this );
  topLayout->addWidget( buttonBox );

  mAddButton = new QPushButton( i18n("Add..."), buttonBox, "add" );
  mDeleteButton = new QPushButton( i18n("Remove"), buttonBox, "del" );
  mEditButton = new QPushButton( i18n("Edit..."), buttonBox, "edit" );
  mDeleteButton->setDisabled( true );
  mEditButton->setDisabled( true );

  connect( mListView, SIGNAL( clicked( QListViewItem * ) ),
           SLOT( currentChanged( QListViewItem * ) ) );
  connect( mAddButton, SIGNAL( clicked() ), SLOT( addResource() ) );
  connect( mDeleteButton, SIGNAL( clicked() ), SLOT( removeResource() ) );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( editResource() ) );
  connect( mListView, SIGNAL( doubleClicked ( QListViewItem *, const QPoint &, int ) ), SLOT( editResource() ) );
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
    addResourceItem( *it );
  }
}

void ResourceView::emitResourcesChanged()
{
  emit resourcesChanged();
}

void ResourceView::addResource()
{
  kdDebug(5850) << "ResourceView::addResource()" << endl;
  QStringList types = mManager->resourceTypeNames();
  bool ok = false;
  QString type = KInputDialog::getItem( i18n( "Resource Configuration" ),
      i18n( "Please select type of the new resource:" ), types, 0, false, &ok,
            this );
  if ( !ok )
    return;

  // Create new resource
  ResourceCalendar *resource = mManager->createResource( type );
  if( !resource ) {
    KMessageBox::error( this, i18n("<qt>Unable to create resource of type <b>%1</b>.</qt>")
                              .arg( type ) );
    return;
  }

  resource->setResourceName( i18n("%1 resource").arg( type ) );

  KRES::ConfigDialog dlg( this, QString("calendar"), resource,
                          "KRES::ConfigDialog" );

  if ( dlg.exec() ) {
    mManager->add( resource );
    addResourceItem( resource );
  } else {
    delete resource;
    resource = 0;
  }
}

void ResourceView::addResourceItem( ResourceCalendar *resource )
{
  new ResourceItem( resource, this, mListView );
}

void ResourceView::removeResource()
{
  QListViewItem *item = mListView->currentItem();
  ResourceItem *rItem = static_cast<ResourceItem *>( item );

  if ( !rItem )
    return;

  int km = KMessageBox::warningYesNo( this,
        i18n("<qt>Do you really want to delete the resource <b>%1</b>?</qt>")
        .arg( rItem->resource()->resourceName() ) );
  if ( km == KMessageBox::No ) return;

  if ( rItem->resource() == mManager->standardResource() ) {
    KMessageBox::sorry( this,
                        i18n( "You cannot remove your standard resource!" ) );
    return;
  }

  mManager->remove( rItem->resource() );

  mListView->takeItem( item );
  delete item;
}

void ResourceView::editResource()
{
  QListViewItem *item = mListView->currentItem();
  ResourceItem *rItem = static_cast<ResourceItem *>( item );
  if ( !rItem )
    return;

  ResourceCalendar *resource = rItem->resource();

  KRES::ConfigDialog dlg( this, QString("calendar"), resource,
                          "KRES::ConfigDialog" );

  if ( dlg.exec() ) {
    rItem->setText( 0, resource->resourceName() );

    mManager->change( resource );
  }
}

void ResourceView::currentChanged( QListViewItem *item)
{
  bool selected = true;
  if ( !item ) selected = false;
  mDeleteButton->setEnabled( selected );
  mEditButton->setEnabled( selected );
}

#include "resourceview.moc"
