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
#include <kresources/resourceconfigdlg.h>

#include <qinputdialog.h>
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
    mResource->save();
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

  QBoxLayout *buttonLayout = new QHBoxLayout( this );

  add = new QPushButton( i18n("Add"), this, "add" );
  del = new QPushButton( i18n("Remove"), this, "del" );
  edit = new QPushButton( i18n("Edit"), this, "edit" );
  del->setDisabled( true );
  edit->setDisabled( true );
  buttonLayout->addWidget( add );
  buttonLayout->addWidget( del );
  buttonLayout->addWidget( edit );
  topLayout->addLayout( buttonLayout );

  connect( mListView, SIGNAL(clicked(QListViewItem*)),
           SLOT(currentChanged(QListViewItem*)) );
  connect( add, SIGNAL(clicked()), SLOT(addResource()) );
  connect( del, SIGNAL(clicked()), SLOT(removeResource()) );
  connect( edit, SIGNAL(clicked()), SLOT(editResource()) );

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

void ResourceView::addResource()
{
  kdDebug() << "ResourceView::addResource()" << endl;
  QStringList types = mManager->resourceTypeNames();
  bool ok = false;
  QString type = QInputDialog::getItem( i18n( "Resource Configuration" ),
      i18n( "Please select type of the new resource:" ), types, 0, false, &ok, this );
  if ( !ok )
    return;

  // Create new resource
  ResourceCalendar *resource = mManager->createResource( type );
  if( !resource ) {
    KMessageBox::error( this, i18n("Unable to create resource of type '%1'.")
                              .arg( type ) );
    return;
  }

  resource->setResourceName( type + "-resource" );

  KRES::ResourceConfigDlg dlg( this, QString("calendar"), resource, "ResourceConfigDlg" );

  if ( dlg.exec() ) {
    mManager->add( resource );
    ResourceItem *item = new ResourceItem( resource, this, mListView );
  } else {
    delete resource;
    resource = 0;
  }
}

void ResourceView::removeResource()
{
  QListViewItem *item = mListView->currentItem();
  ResourceItem *rItem = static_cast<ResourceItem*>( item );

  if ( !rItem )
    return;

  int km = KMessageBox::warningYesNo(this,
        i18n("Do you really want to delete the resource")+ " '" +
             rItem->resource()->resourceName() + "'?" );
  if ( km == KMessageBox::No ) return;

  if ( rItem->resource() == mManager->standardResource() ) {
    KMessageBox::sorry( this, i18n( "You cannot remove your standard resource!" ) );
    return;
  }

  mManager->remove( rItem->resource() );

  mListView->takeItem( item );
  delete item;

}

void ResourceView::editResource()
{
  QListViewItem *item = mListView->currentItem();
  ResourceItem *rItem = static_cast<ResourceItem*>( item );
  if ( !rItem )
    return;

  ResourceCalendar *resource = rItem->resource();

  KRES::ResourceConfigDlg dlg( this, QString("calendar"), resource, "ResourceConfigDlg" );

  if ( dlg.exec() ) {
    rItem->setText( 0, resource->resourceName() );

    mManager->resourceChanged( resource );
  }
}

void ResourceView::currentChanged( QListViewItem *item)
{
  bool selected = true;
  if ( !item ) selected = false;
  del->setEnabled( selected );
  edit->setEnabled( selected );
}

#include "resourceview.moc"
