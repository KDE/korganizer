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

#include "resourceview.h"
#include "koprefs.h"

#include <kresources/resource.h>
#include <kresources/configdialog.h>
#include <kcal/calendarresources.h>

#include <kcolordialog.h>
#include <kdialog.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kvbox.h>
#include <krandom.h>
#include <kcolorcollection.h>

#include <QLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QMenu>
#include <QPixmap>
#include <QBoxLayout>
#include <QVBoxLayout>

using namespace KCal;

ResourceViewFactory::ResourceViewFactory( KCal::CalendarResources *calendar,
                                          CalendarView *view )
  : mCalendar( calendar ), mView( view ), mResourceView( 0 )
{
}

CalendarViewExtension *ResourceViewFactory::create( QWidget *parent )
{
  mResourceView = new ResourceView( mCalendar, parent );

  QObject::connect( mResourceView, SIGNAL( resourcesChanged() ),
                    mView, SLOT( updateView() ) );
  QObject::connect( mResourceView, SIGNAL( resourcesChanged() ),
                    mView, SLOT( updateCategories() ) );

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
                            QTreeWidget *parent )
  : QTreeWidgetItem( parent ),
    mResource( resource ), mView( view ), mBlockStateChange( false ),
    mIsSubresource( false ), mResourceIdentifier( QString() ),
    mSubItemsCreated( false ), mIsStandardResource( false )
{
  setFlags( flags() | Qt::ItemIsUserCheckable );

  setText( 0, resource->resourceName() );

  mResourceColor = QColor();
  setGuiState();

  if ( mResource->isActive() ) {
    createSubresourceItems();
  }
}

void ResourceItem::createSubresourceItems()
{
  const QStringList subresources = mResource->subresources();
  if ( !subresources.isEmpty() ) {
    setExpanded( true );
    // This resource has subresources
    QStringList::ConstIterator it;
    for ( it = subresources.begin(); it != subresources.end(); ++it ) {
      ResourceItem *item = new ResourceItem( mResource, *it, mResource->labelForSubresource( *it ),
                                             mView, this );
      QColor resourceColor = KOPrefs::instance()->resourceColor( *it );
      item->setResourceColor( resourceColor );
    }
  }
  mSubItemsCreated = true;
}

ResourceItem::ResourceItem( KCal::ResourceCalendar *resource,
                            const QString &sub, const QString &label,
                            ResourceView *view, ResourceItem *parent )

  : QTreeWidgetItem( parent ), mResource( resource ),
    mView( view ), mBlockStateChange( false ), mIsSubresource( true ),
    mSubItemsCreated( false ), mIsStandardResource( false ), mActive( false )
{
  setFlags( flags() | Qt::ItemIsUserCheckable );
  setText( 0, label );
  mResourceColor = QColor();
  mResourceIdentifier = sub;
  setGuiState();
}

void ResourceItem::setGuiState()
{
  mBlockStateChange = true;
  if ( mIsSubresource ) {
    setOn( mResource->subresourceActive( mResourceIdentifier ) );
  } else {
    setOn( mResource->isActive() );
  }
  mBlockStateChange = false;
}

void ResourceItem::setOn( bool checked )
{
  if ( checked ) {
    setCheckState( 0, Qt::Checked );
  } else {
    setCheckState( 0, Qt::Unchecked );
  }
  mActive = checked;
}

void ResourceItem::stateChange( bool active )
{
  if ( mActive == active ) {
    return;
  }

  if ( mBlockStateChange ) {
    return;
  }

  if ( mIsSubresource ) {
    mResource->setSubresourceActive( mResourceIdentifier, active );
  } else {
    if ( active ) {
      if ( mResource->load() ) {
        mResource->setActive( true );
        if ( !mSubItemsCreated ) {
          createSubresourceItems();
        }
      }
    } else {
      if ( mResource->save() ) {
        mResource->setActive( false );
      }
      mView->requestClose( mResource );
    }

    setExpanded( mResource->isActive() && childCount() > 0 );
    setGuiState();
  }

  mView->emitResourcesChanged();
}

void ResourceItem::update()
{
  setGuiState();
}

void ResourceItem::setResourceColor( QColor &color )
{
  if ( color.isValid() ) {
    if ( mResourceColor != color ) {
      int height = treeWidget()->visualItemRect(this).height();
      QPixmap px( height - 4, height - 4 );
      mResourceColor = color;
      px.fill(color);
      setData( 0, Qt::DecorationRole, px );
    }
  } else {
    mResourceColor = color ;
    setData( 0, Qt::DecorationRole, QPixmap() );
  }
}

void ResourceItem::setStandardResource( bool std )
{
  QFont font = qvariant_cast<QFont>(data(0, Qt::FontRole));
  font.setBold(std);
  setData(0, Qt::FontRole, font);
}

ResourceView::ResourceView( KCal::CalendarResources *calendar, QWidget *parent )
  : CalendarViewExtension( parent ), mCalendar( calendar )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  mListView = new QTreeWidget( this );
  mListView->setWhatsThis( i18n( "<qt><p>Select on this list the active KOrganizer "
                                 "resources. Check the resource box to make it "
                                 "active. Use the context menu to add, remove or edit "
                                 "resources in the list.</p>"
                                 "<p>Events, journal entries and to-dos are retrieved "
                                 "and stored on resources. Available "
                                 "resources include groupware servers, local files, "
                                 "journal entries as blogs on a server, etc...</p>"
                                 "<p>If you have more than one active resource, "
                                 "when creating incidents you will either automatically "
                                 "use the default resource or be prompted "
                                 "to select the resource to use.</p></qt>" ) );
  mListView->setRootIsDecorated( false );
  mListView->setHeaderLabel( i18n( "Calendars" ) );
  topLayout->addWidget( mListView );

  connect( mListView, SIGNAL( itemDoubleClicked ( QTreeWidgetItem *, int ) ),
           SLOT( editResource() ) );

  mListView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mListView, SIGNAL( customContextMenuRequested( const QPoint & ) ),
           SLOT( showContextMenu( const QPoint & ) ) );

  connect( mListView, SIGNAL( itemClicked( QTreeWidgetItem *, int ) ),
           SLOT( slotItemClicked( QTreeWidgetItem *, int ) ) );

  updateView();
}

ResourceView::~ResourceView()
{
}

void ResourceView::updateView()
{
  mListView->clear();

  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();

  KCal::CalendarResourceManager::Iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it ) {
    addResourceItem( *it );
  }
}

void ResourceView::emitResourcesChanged()
{
  mCalendar->resourceManager()->writeConfig();
  emit resourcesChanged();
}

void ResourceView::addResource()
{
  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();
  QStringList types = manager->resourceTypeNames();
  QStringList descs = manager->resourceTypeDescriptions();
  bool ok = false;
  QString desc = KInputDialog::getItem( i18n( "Resource Configuration" ),
                                        i18n( "Please select type of the new resource:" ),
                                        descs, 0, false, &ok, this );
  if ( !ok ) {
    return;
  }

  QString type = types.at( descs.indexOf( desc ) );

  // Create new resource
  ResourceCalendar *resource = manager->createResource( type );
  if( !resource ) {
    KMessageBox::error( this,
                        i18n( "<qt>Unable to create resource of type <b>%1</b>.</qt>", type ) );
    return;
  }

  resource->setResourceName( i18n( "%1 resource", type ) );

  KRES::ConfigDialog *dlg = new KRES::ConfigDialog( this, QString( "calendar" ), resource );

  // TODO: Add a fallback (KColorCollection::setName() broken?)
  KColorCollection collection("Oxygen.colors");
  // TODO: Be smarter than this
  int rand = ( KRandom::random() % collection.count() ) + 1;
  QColor color = collection.color( rand );

  KOPrefs::instance()->setResourceColor( resource->identifier(),  color);

  if ( dlg && dlg->exec() ) {
    resource->setTimeSpec( KOPrefs::instance()->timeSpec() );
    manager->add( resource );
    // we have to call resourceAdded manually, because for in-process changes
    // the dcop signals are not connected, so the resource's signals would not
    // be connected otherwise
    mCalendar->resourceAdded( resource );
  } else {
    delete resource;
    resource = 0;
  }
  delete dlg;
  emitResourcesChanged();
}

void ResourceView::addResourceItem( ResourceCalendar *resource )
{
  ResourceItem *item = new ResourceItem( resource, this, mListView );

  QColor resourceColor;

  resourceColor= KOPrefs::instance()->resourceColor( resource->identifier() );
  item->setResourceColor(resourceColor);

  connect( resource, SIGNAL( signalSubresourceAdded( ResourceCalendar *,
                                                     const QString &,
                                                     const QString &,
                                                     const QString & ) ),
           SLOT( slotSubresourceAdded( ResourceCalendar *, const QString &,
                                       const QString &, const QString & ) ) );

  connect( resource, SIGNAL( signalSubresourceRemoved( ResourceCalendar *,
                                                       const QString &,
                                                       const QString & ) ),
           SLOT( slotSubresourceRemoved( ResourceCalendar *, const QString &,
                                         const QString & ) ) );

  connect( resource, SIGNAL( resourceSaved( ResourceCalendar * ) ),
           SLOT( closeResource( ResourceCalendar * ) ) );

  updateResourceList();
  emit resourcesChanged();
}

// Add a new entry
void ResourceView::slotSubresourceAdded( ResourceCalendar *calendar,
                                         const QString &type,
                                         const QString &resource,
                                         const QString &label )
{
  Q_UNUSED( type );
  QList<QTreeWidgetItem *> items =
    mListView->findItems( calendar->resourceName(), Qt::MatchExactly, 0 );
  if ( items.isEmpty() ) {
    // Not found
    return;
  }

  ResourceItem *item = static_cast<ResourceItem *>( items.first() );
  ( void )new ResourceItem( calendar, resource, label, this, item );
  emitResourcesChanged();
}

// Remove an entry
void ResourceView::slotSubresourceRemoved( ResourceCalendar *calendar,
                                           const QString &type,
                                           const QString &resource )
{
  Q_UNUSED( calendar );
  Q_UNUSED( type );
  delete findItemByIdentifier( resource );
  emit resourcesChanged();
}

void ResourceView::closeResource( ResourceCalendar *r )
{
  if ( mResourcesToClose.contains( r ) ) {
    r->close();
    mResourcesToClose.removeAll( r );
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
  QTreeWidgetItem *item = mListView->currentItem();
  ResourceItem *rItem = static_cast<ResourceItem *>( item );
  return rItem;
}

void ResourceView::removeResource()
{
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }

  int km =
    KMessageBox::warningContinueCancel( this,
                                        i18n( "<qt>Do you really want to delete "
                                              "the resource <b>%1</b>?</qt>",
                                              item->text( 0 ) ),
                                        "", KStandardGuiItem::del() );
  if ( km == KMessageBox::Cancel ) {
    return;
  }

// Don't be so restricitve
#if 0
  if ( item->resource() == mCalendar->resourceManager()->standardResource() ) {
    KMessageBox::sorry( this, i18n( "You cannot delete your standard resource." ) );
    return;
  }
#endif
  if ( item->isSubresource() ) {
    // FIXME delete the folder in KMail
  } else {
    mCalendar->resourceManager()->remove( item->resource() );
    delete item;
  }
  updateResourceList();
  emit resourcesChanged();
}

void ResourceView::editResource()
{
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }
  ResourceCalendar *resource = item->resource();

  KRES::ConfigDialog dlg( this, QString( "calendar" ), resource );

  if ( dlg.exec() ) {
    item->setText( 0, resource->resourceName() );

    mCalendar->resourceManager()->change( resource );
  }
  emitResourcesChanged();
}

ResourceItem *ResourceView::findItem( ResourceCalendar *r )
{
  QList<QTreeWidgetItem *> items = mListView->findItems( "*", Qt::MatchWildcard );
  foreach ( QTreeWidgetItem *i, items ) {
    ResourceItem *item = static_cast<ResourceItem *>( i );
    if ( item->resource() == r ) {
      return item;
    }
  }
  return 0;
}

ResourceItem *ResourceView::findItemByIdentifier( const QString &id )
{
  QList<QTreeWidgetItem *> items = mListView->findItems( "*", Qt::MatchWildcard );
  foreach ( QTreeWidgetItem *i, items ) {
    ResourceItem *item = static_cast<ResourceItem *>( i );
    if ( item->resourceIdentifier() == id ) {
      return item;
    }
  }
  return 0;
}

void ResourceView::showContextMenu( const QPoint &pos )
{
  QTreeWidgetItem *i = mListView->itemAt( pos );
  if ( !i ) {
    return;
  }

  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();
  ResourceItem *item = static_cast<ResourceItem *>( i );

  QMenu *menu = new QMenu( this );
  connect( menu, SIGNAL( aboutToHide() ), menu, SLOT( deleteLater() ) );
  if ( item ) {
    QAction *reloadAction = menu->addAction(
      i18nc( "reload the resource", "Re&load" ), this, SLOT( reloadResource() ) );
    reloadAction->setEnabled( item->resource()->isActive() );

    QAction *saveAction = menu->addAction(
      i18nc( "save the resource", "&Save" ), this, SLOT( saveResource() ) );
    saveAction->setEnabled( item->resource()->isActive() );

    menu->addSeparator();

    menu->addAction( i18n( "Show &Info" ), this, SLOT( showInfo() ) );
    //FIXME: This is better on the resource dialog
    if ( KOPrefs::instance()->agendaViewUsesResourceColor() ) {
      QMenu *assignMenu = menu->addMenu( i18n( "Resource Colors" ) );
      assignMenu->addAction( i18n( "&Assign Color" ), this, SLOT( assignColor() ) );
      if ( item->resourceColor().isValid() ) {
        assignMenu->addAction( i18n( "&Disable Color" ), this, SLOT( disableColor() ) );
      }
    }

    menu->addAction( i18n( "&Edit..." ), this, SLOT( editResource() ) );
    menu->addAction( i18n( "&Remove" ), this, SLOT( removeResource() ) );
    if ( item->resource() != manager->standardResource() ) {
      menu->addSeparator();
      menu->addAction( i18n( "Use as &Default Calendar" ), this, SLOT( setStandard() ) );
    }
    menu->addSeparator();
  }
  menu->addAction( i18n( "&Add..." ), this, SLOT( addResource() ) );
  menu->popup( mapToGlobal( pos ) );
}

void ResourceView::assignColor()
{
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }

  // A color without initialized is a color invalid
  QColor myColor;
  KCal::ResourceCalendar *cal = item->resource();

  QString identifier = cal->identifier();
  if ( item->isSubresource() ) {
    identifier = item->resourceIdentifier();
  }

  QColor defaultColor = KOPrefs::instance()->resourceColor( identifier );

  int result = KColorDialog::getColor( myColor, defaultColor );
  if ( result == KColorDialog::Accepted ) {
    KOPrefs::instance()->setResourceColor( identifier, myColor );
    item->setResourceColor( myColor );
    item->update();
    emitResourcesChanged();
  }
}

void ResourceView::disableColor()
{
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }

  QColor colorInvalid;
  KCal::ResourceCalendar *cal = item->resource();
  QString identifier = cal->identifier();
  if ( item->isSubresource() ) {
    identifier = item->resourceIdentifier();
  }
  KOPrefs::instance()->setResourceColor( identifier, colorInvalid );
  item->setResourceColor( colorInvalid );
  item->update();
  emitResourcesChanged();
}
void ResourceView::showInfo()
{
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }

  QString txt = "<qt>" + item->resource()->infoText() + "</qt>";
  KMessageBox::information( this, txt );
}

void ResourceView::reloadResource()
{
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }

  ResourceCalendar *r = item->resource();
  r->load();
}

void ResourceView::saveResource()
{
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }

  ResourceCalendar *r = item->resource();
  r->save();
}

void ResourceView::setStandard()
{
  ResourceItem *item = currentItem();
  if ( !item ) {
    return;
  }

  ResourceCalendar *r = item->resource();
  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();
  manager->setStandardResource( r );
  updateResourceList();
}

void ResourceView::updateResourceList()
{
  ResourceCalendar *stdRes = mCalendar->resourceManager()->standardResource();

  QList<QTreeWidgetItem *> items = mListView->findItems( "*", Qt::MatchWildcard );
  foreach ( QTreeWidgetItem *i, items ) {
    ResourceItem *item = static_cast<ResourceItem *>( i );
    item->setStandardResource( item->resource() == stdRes );
  }
}

void ResourceView::requestClose( ResourceCalendar *r )
{
  mResourcesToClose.append( r );
}

void ResourceView::slotItemClicked( QTreeWidgetItem *i, int )
{
  ResourceItem *item = static_cast<ResourceItem *>( i );
  if ( item ) {
    item->stateChange( item->checkState( 0 ) == Qt::Checked );
  }
}

#include "resourceview.moc"
