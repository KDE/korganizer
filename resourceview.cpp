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

#include "resourceview.h"

#include <klistview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kresources/resource.h>
#include <kresources/configdialog.h>
#include <kinputdialog.h>
#include <libkcal/calendarresources.h>
#include <resourceremote.h>

#include <qhbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>

using namespace KCal;

ResourceViewFactory::ResourceViewFactory( KCal::CalendarResources *calendar,
                                          CalendarView *view )
  : mCalendar( calendar ), mView( view ), mResourceView( 0 )
{
}

CalendarViewExtension *ResourceViewFactory::create( QWidget *parent )
{
  mResourceView = new ResourceView( mCalendar->resourceManager(),
                                                  parent );

  QObject::connect( mResourceView, SIGNAL( resourcesChanged() ),
                    mView, SLOT( updateView() ) );
  QObject::connect( mResourceView, SIGNAL( resourcesChanged() ),
                    mView, SLOT( updateCategories() ) );
  QObject::connect( mResourceView,
                    SIGNAL( signalErrorMessage( const QString & ) ),
                    mView, SLOT( showErrorMessage( const QString & ) ) );

  QObject::connect( mCalendar,
                    SIGNAL( signalResourceAdded( ResourceCalendar * ) ),
                    mResourceView,
                    SLOT( addResourceItem( ResourceCalendar * ) ) );
  QObject::connect( mCalendar,
                    SIGNAL( signalResourceModified( ResourceCalendar * ) ),
                    mResourceView,
                    SLOT( updateResourceItem( ResourceCalendar * ) ) );
  QObject::connect( mCalendar, SIGNAL( signalResourceAdded( ResourceCalendar * ) ),
                    mView, SLOT( updateCategories() ) );
  QObject::connect( mCalendar, SIGNAL( signalResourceModified( ResourceCalendar * ) ),
                    mView, SLOT( updateCategories() ) );

  return mResourceView;
}

ResourceView *ResourceViewFactory::resourceView() const
{
  return mResourceView;
}

ResourceItem::ResourceItem( ResourceCalendar *resource, ResourceView *view,
                            KListView *parent )
  : QCheckListItem( parent, resource->resourceName(), CheckBox ),
    mResource( resource ), mView( view ), mBlockStateChange( false ),
    mIsSubresource( false )
{
  setGuiState();

  const QStringList subresources = mResource->subresources();
  if ( !subresources.isEmpty() ) {
    setOpen( true );
    setExpandable( true );
    // This resource has subresources
    QStringList::ConstIterator it;
    for ( it=subresources.begin(); it!=subresources.end(); ++it ) {
      ( void )new ResourceItem( mResource, *it, mView, this );
    }
  }
}

ResourceItem::ResourceItem( KCal::ResourceCalendar *resource,
                            const QString& sub, ResourceView *view,
                            ResourceItem* parent )
  : QCheckListItem( parent, sub, CheckBox ), mResource( resource ),
    mView( view ), mBlockStateChange( false ), mIsSubresource( true )
{
  setGuiState();
}

void ResourceItem::setGuiState()
{
  mBlockStateChange = true;
  if ( mIsSubresource )
    setOn( mResource->subresourceActive( text( 0 ) ) );
  else
    setOn( mResource->isActive() );
  mBlockStateChange = false;
}

void ResourceItem::stateChange( bool active )
{
  if ( mBlockStateChange ) return;

  if ( !mIsSubresource ) {
    // Handle a full resource
    bool toActivate = active;
    if ( active ) {
      bool success = mResource->open();
      if ( success ) {
        success = mResource->load();
      }
      if ( !success ) {
        QString msg = mResource->errorMessage();
        if ( msg.isEmpty() ) {
          msg = i18n("Error loading calendar '%1'")
                .arg( mResource->resourceName() );
        }
        if ( !msg.isEmpty() ) mView->emitErrorMessage( msg );
        toActivate = false;
      }
    } else {
      mResource->save();
      mView->requestClose( mResource );
    }
    mResource->setActive( toActivate );

    setOpen( toActivate && childCount() > 0 );

    setGuiState();
  } else {
    // Handle a subresource
    mResource->setSubresourceActive( text( 0 ), active );
  }

  mView->emitResourcesChanged();
}

void ResourceItem::update()
{
  setGuiState();
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
  connect( mListView, SIGNAL( doubleClicked ( QListViewItem *, const QPoint &,
                                              int ) ),
           SLOT( editResource() ) );
  connect( mListView, SIGNAL( contextMenuRequested ( QListViewItem *,
                                                     const QPoint &, int ) ),
           SLOT( contextMenuRequested( QListViewItem *, const QPoint &,
                                       int ) ) );

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
  QStringList types = mManager->resourceTypeNames();
  QStringList descs = mManager->resourceTypeDescriptions();
  bool ok = false;
  QString desc = KInputDialog::getItem( i18n( "Resource Configuration" ),
      i18n( "Please select type of the new resource:" ), descs, 0, false, &ok,
            this );
  if ( !ok )
    return;

  QString type = types[ descs.findIndex( desc ) ];

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
    if ( resource->isActive() ) {
      resource->open();
      resource->load();
    }

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

  connect( resource, SIGNAL( signalSubresourceAdded( ResourceCalendar *,
                                                     const QString &,
                                                     const QString & ) ),
           SLOT( slotSubresourceAdded( ResourceCalendar *, const QString &,
                                       const QString & ) ) );
  connect( resource, SIGNAL( signalSubresourceRemoved( ResourceCalendar *,
                                                       const QString &,
                                                       const QString & ) ),
           SLOT( slotSubresourceRemoved( ResourceCalendar *, const QString &,
                                         const QString & ) ) );

  connect( resource, SIGNAL( resourceSaved( ResourceCalendar * ) ),
           SLOT( closeResource( ResourceCalendar * ) ) );

  emitResourcesChanged();
}

// Add a new entry
void ResourceView::slotSubresourceAdded( ResourceCalendar *calendar,
                                         const QString &/*type*/,
                                         const QString &resource )
{
  QListViewItem *i = mListView->findItem( calendar->resourceName(), 0 );
  if ( !i )
    // Not found
    return;

  ResourceItem *item = static_cast<ResourceItem *>( i );
  ( void )new ResourceItem( calendar, resource, this, item );
}

// Remove an entry
void ResourceView::slotSubresourceRemoved( ResourceCalendar */*calendar*/,
                                           const QString &/*type*/,
                                           const QString &resource )
{
  delete mListView->findItem( resource, 0 );
}

void ResourceView::closeResource( ResourceCalendar *r )
{
  if ( mResourcesToClose.find( r ) >= 0 ) {
    r->close();
    mResourcesToClose.remove( r );
  }
}

void ResourceView::updateResourceItem( ResourceCalendar *resource )
{
  ResourceItem *item = findItem( resource );
  if ( item ) {
    item->update();
  }
}

ResourceItem *ResourceView::currentItem()
{
  QListViewItem *item = mListView->currentItem();
  ResourceItem *rItem = static_cast<ResourceItem *>( item );
  return rItem;
}

void ResourceView::removeResource()
{
  ResourceItem *item = currentItem();
  if ( !item ) return;

  int km = KMessageBox::warningContinueCancel( this,
        i18n("<qt>Do you really want to remove the resource <b>%1</b>?</qt>")
        .arg( item->resource()->resourceName() ), "",
        KGuiItem( i18n("&Remove" ), "editdelete") );
  if ( km == KMessageBox::Cancel ) return;

// Don't be so restricitve
#if 0
  if ( item->resource() == mManager->standardResource() ) {
    KMessageBox::sorry( this,
                        i18n( "You cannot remove your standard resource." ) );
    return;
  }
#endif

  mManager->remove( item->resource() );

  mListView->takeItem( item );
  delete item;
  emitResourcesChanged();
}

void ResourceView::editResource()
{
  ResourceItem *item = currentItem();

  ResourceCalendar *resource = item->resource();

  KRES::ConfigDialog dlg( this, QString("calendar"), resource,
                          "KRES::ConfigDialog" );

  if ( dlg.exec() ) {
    item->setText( 0, resource->resourceName() );

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

void ResourceView::emitErrorMessage( const QString &msg )
{
  emit signalErrorMessage( msg );
}

ResourceItem *ResourceView::findItem( ResourceCalendar *r )
{
  QListViewItem *item;
  ResourceItem *i = 0;
  for( item = mListView->firstChild(); item; item = item->nextSibling() ) {
    i = static_cast<ResourceItem *>( item );
    if ( i->resource() == r ) break;
  }
  return i;
}

void ResourceView::contextMenuRequested ( QListViewItem *i,
                                          const QPoint &pos, int )
{
  ResourceItem *item = static_cast<ResourceItem *>( i );
  
  QPopupMenu *menu = new QPopupMenu( this );
  connect( menu, SIGNAL( aboutToHide() ), menu, SLOT( deleteLater() ) );
  if ( item ) {
    int reloadId = menu->insertItem( i18n("Reload"), this,
                                     SLOT( reloadResource() ) );
    menu->setItemEnabled( reloadId, item->resource()->isActive() );
    int saveId = menu->insertItem( i18n("Save"), this,
                                   SLOT( saveResource() ) );
    menu->setItemEnabled( saveId, item->resource()->isActive() );
    menu->insertSeparator();
    menu->insertItem( i18n("Show Info..."), this, SLOT( showInfo() ) );
    menu->insertItem( i18n("Edit..."), this, SLOT( editResource() ) );
    menu->insertItem( i18n("Remove"), this, SLOT( removeResource() ) );
    menu->insertSeparator();
  }
  menu->insertItem( i18n("Add..."), this, SLOT( addResource() ) );

  menu->popup( pos );
}

void ResourceView::showInfo()
{
  ResourceItem *item = currentItem();
  if ( !item ) return;
  
  QString txt = infoText( item->resource() );
  KMessageBox::information( this, txt );
}

void ResourceView::reloadResource()
{
  ResourceItem *item = currentItem();
  if ( !item ) return;
  
  ResourceCalendar *r = item->resource();
  r->load();
}

void ResourceView::saveResource()
{
  ResourceItem *item = currentItem();
  if ( !item ) return;
  
  ResourceCalendar *r = item->resource();
  r->save();
}

QString ResourceView::infoText( ResourceCalendar *r )
{
  QString txt = "<qt>";
  
  txt += "<b>" + r->resourceName() + "</b>";
  txt += "<br>";
  
  KRES::Factory *factory = KRES::Factory::self( "calendar" );
  QString type = factory->typeName( r->type() );
  txt += i18n("Type: %1").arg( type );

  if ( r->type() == "remote" ) {
    txt += "<br>";
    ResourceRemote *remote = static_cast<ResourceRemote *>( r );
    txt += i18n("URL: %1").arg( remote->downloadUrl().prettyURL() );
  }

  txt += "</qt>";

  return txt;
}

void ResourceView::showButtons( bool visible )
{
  if ( visible ) {
    mAddButton->show();
    mDeleteButton->show();
    mEditButton->show();
  } else {
    mAddButton->hide();
    mDeleteButton->hide();
    mEditButton->hide();
  }
}

void ResourceView::requestClose( ResourceCalendar *r )
{
  mResourcesToClose.append( r );
}

#include "resourceview.moc"
