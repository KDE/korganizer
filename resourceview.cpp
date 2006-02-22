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

#include <kcolordialog.h>
#include <kdialog.h>
#include <klistview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kresources/resource.h>
#include <kresources/configdialog.h>
#include <kinputdialog.h>
#include <libkcal/calendarresources.h>


#include <qlayout.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <q3popupmenu.h>

//Added by qt3to4:
#include <QPixmap>
#include <QBoxLayout>
#include <QVBoxLayout>
#include <kvbox.h>

#include "koprefs.h"

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
                            KListView *parent )
  : Q3CheckListItem( parent, resource->resourceName(), CheckBox ),
    mResource( resource ), mView( view ), mBlockStateChange( false ),
    mIsSubresource( false ), mResourceIdentifier( QString() ),
    mSubItemsCreated( false ), mIsStandardResource( false )
{
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
    setOpen( true );
    setExpandable( true );
    // This resource has subresources
    QStringList::ConstIterator it;
    for ( it=subresources.begin(); it!=subresources.end(); ++it ) {
      ResourceItem *item = new ResourceItem( mResource, *it, mResource->labelForSubresource( *it ),
                                             mView, this );
      QColor resourceColor = KOPrefs::instance()->resourceColor( *it );
      item->setResourceColor( resourceColor );
    }
  }
  mSubItemsCreated = true;
}

ResourceItem::ResourceItem( KCal::ResourceCalendar *resource,
                            const QString& sub, const QString& label,
                            ResourceView *view, ResourceItem* parent )

  : Q3CheckListItem( parent, label, CheckBox ), mResource( resource ),
    mView( view ), mBlockStateChange( false ), mIsSubresource( true ),
    mSubItemsCreated( false ), mIsStandardResource( false )
{
  mResourceColor = QColor();
  mResourceIdentifier = sub;
  setGuiState();
}

void ResourceItem::setGuiState()
{
  mBlockStateChange = true;
  if ( mIsSubresource )
    setOn( mResource->subresourceActive( mResourceIdentifier ) );
  else
    setOn( mResource->isActive() );
  mBlockStateChange = false;
}

void ResourceItem::stateChange( bool active )
{
  if ( mBlockStateChange ) return;

  if ( mIsSubresource ) {
    mResource->setSubresourceActive( mResourceIdentifier, active );
  } else {
    if ( active ) {
      if ( mResource->load() ) {
        mResource->setActive( true );
        if ( !mSubItemsCreated )
          createSubresourceItems();
      }
    } else {
      if ( mResource->save() ) mResource->setActive( false );
      mView->requestClose( mResource );
    }

    setOpen( mResource->isActive() && childCount() > 0 );

    setGuiState();
  }

  mView->emitResourcesChanged();
}

void ResourceItem::update()
{
  setGuiState();
}

void ResourceItem::setResourceColor(QColor& color)
{
  if ( color.isValid() ) {
    if ( mResourceColor != color ) {
      QPixmap px(height()-4,height()-4);
      mResourceColor = color;
      px.fill(color);
      setPixmap(0,px);
    }
  } else {
    mResourceColor = color ;
    setPixmap( 0, QPixmap() );
  }
}

void ResourceItem::setStandardResource( bool std )
{
  if ( mIsStandardResource != std ) {
    mIsStandardResource = std;
    repaint();
  }
}

void ResourceItem::paintCell(QPainter *p, const QColorGroup &cg,
      int column, int width, int alignment)
{
  QFont oldFont = p->font();
  QFont newFont = oldFont;
  newFont.setBold( mIsStandardResource && !mIsSubresource );
  p->setFont( newFont );
  Q3CheckListItem::paintCell( p, cg, column, width, alignment );
  p->setFont( oldFont );
/*  QColorGroup _cg = cg;
  if(!mResource) return;
  _cg.setColor(QColorGroup::Base, getTextColor(mResourceColor));*/
}


ResourceView::ResourceView( KCal::CalendarResources *calendar,
                            QWidget *parent, const char *name )
  : CalendarViewExtension( parent, name ), mCalendar( calendar )
{
  QBoxLayout *topLayout = new QVBoxLayout( this, 0, KDialog::spacingHint() );

  mListView = new KListView( this );
  mListView->setWhatsThis(
                   i18n( "<qt><p>Select on this list the active KOrganizer "
                         "resources. Check the resource box to make it "
                         "active. Press the \"Add...\" button below to add new "
                         "resources to the list.</p>"
                         "<p>Events, journal entries and to-dos are retrieved "
                         "and stored on resources. Available "
                         "resources include groupware servers, local files, "
                         "journal entries as blogs on a server, etc...</p>"
                         "<p>If you have more than one active resource, "
                         "when creating incidents you will either automatically "
                         "use the default resource or be prompted "
                         "to select the resource to use.</p></qt>" ) );
  mListView->addColumn( i18n("Calendar") );
  mListView->setResizeMode( Q3ListView::LastColumn );
  topLayout->addWidget( mListView );

  KHBox *buttonBox = new KHBox( this );
  buttonBox->setSpacing( KDialog::spacingHint() );
  topLayout->addWidget( buttonBox );

  mAddButton = new QPushButton( i18n("Add..."), buttonBox, "add" );
  mAddButton->setWhatsThis(
                   i18n( "<qt><p>Press this button to add a resource to "
                         "KOrganizer.</p>"
                         "<p>Events, journal entries and to-dos are retrieved "
                         "and stored on resources. Available "
                         "resources include groupware servers, local files, "
                         "journal entries as blogs on a server, etc... </p>"
                         "<p>If you have more than one active resource, "
                         "when creating incidents you will either automatically "
                         "use the default resource or be prompted "
                         "to select the resource to use.</p></qt>" ) );
  mEditButton = new QPushButton( i18n("Edit..."), buttonBox, "edit" );
  mEditButton->setWhatsThis(
                   i18n( "Press this button to edit the resource currently "
                         "selected on the KOrganizer resources list above." ) );
  mDeleteButton = new QPushButton( i18n("Remove"), buttonBox, "del" );
  mDeleteButton->setWhatsThis(
                   i18n( "Press this button to delete the resource currently "
                         "selected on the KOrganizer resources list above." ) );
  mDeleteButton->setDisabled( true );
  mEditButton->setDisabled( true );

  connect( mListView, SIGNAL( clicked( Q3ListViewItem * ) ),
           SLOT( currentChanged( Q3ListViewItem * ) ) );
  connect( mAddButton, SIGNAL( clicked() ), SLOT( addResource() ) );
  connect( mDeleteButton, SIGNAL( clicked() ), SLOT( removeResource() ) );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( editResource() ) );
  connect( mListView, SIGNAL( doubleClicked ( Q3ListViewItem *, const QPoint &,
                                              int ) ),
           SLOT( editResource() ) );
  connect( mListView, SIGNAL( contextMenuRequested ( Q3ListViewItem *,
                                                     const QPoint &, int ) ),
           SLOT( contextMenuRequested( Q3ListViewItem *, const QPoint &,
                                       int ) ) );

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
  for( it = manager->begin(); it != manager->end(); ++it ) {
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
      i18n( "Please select type of the new resource:" ), descs, 0, false, &ok,
            this );
  if ( !ok )
    return;

  QString type = types[ descs.indexOf( desc ) ];

  // Create new resource
  ResourceCalendar *resource = manager->createResource( type );
  if( !resource ) {
    KMessageBox::error( this, i18n("<qt>Unable to create resource of type <b>%1</b>.</qt>")
                              .arg( type ) );
    return;
  }

  resource->setResourceName( i18n("%1 resource").arg( type ) );

  KRES::ConfigDialog dlg( this, QString("calendar"), resource,
                          "KRES::ConfigDialog" );

  if ( dlg.exec() ) {
    resource->setTimeZoneId( KOPrefs::instance()->mTimeZoneId );
    manager->add( resource );
    // we have to call resourceAdded manually, because for in-process changes
    // the dcop signals are not connected, so the resource's signals would not
    // be connected otherwise
    mCalendar->resourceAdded( resource );
  } else {
    delete resource;
    resource = 0;
  }
  emitResourcesChanged();
}

void ResourceView::addResourceItem( ResourceCalendar *resource )
{

  ResourceItem *item=new ResourceItem( resource, this, mListView );

  QColor resourceColor;

  resourceColor= KOPrefs::instance()->resourceColor(resource->identifier());
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
                                         const QString& /*type*/,
                                         const QString& resource,
                                         const QString& label)
{
  Q3ListViewItem *i = mListView->findItem( calendar->resourceName(), 0 );
  if ( !i )
    // Not found
    return;

  ResourceItem *item = static_cast<ResourceItem *>( i );
  ( void )new ResourceItem( calendar, resource, label, this, item );
  emitResourcesChanged();
}

// Remove an entry
void ResourceView::slotSubresourceRemoved( ResourceCalendar * /*calendar*/,
                                           const QString &/*type*/,
                                           const QString &resource )
{
  delete findItemByIdentifier( resource );
  emit resourcesChanged();
}

void ResourceView::closeResource( ResourceCalendar *r )
{
  if ( mResourcesToClose.contains( r ) ) {
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
  Q3ListViewItem *item = mListView->currentItem();
  ResourceItem *rItem = static_cast<ResourceItem *>( item );
  return rItem;
}

void ResourceView::removeResource()
{
  ResourceItem *item = currentItem();
  if ( !item ) return;

  int km = KMessageBox::warningContinueCancel( this,
        i18n("<qt>Do you really want to delete the resource <b>%1</b>?</qt>")
        .arg( item->text( 0 ) ), "", KStdGuiItem::del() );
  if ( km == KMessageBox::Cancel ) return;

// Don't be so restricitve
#if 0
  if ( item->resource() == mCalendar->resourceManager()->standardResource() ) {
    KMessageBox::sorry( this,
                        i18n( "You cannot delete your standard resource." ) );
    return;
  }
#endif
  if ( item->isSubresource() ) {
    // FIXME delete the folder in KMail
  } else {
    mCalendar->resourceManager()->remove( item->resource() );
    mListView->takeItem( item );
    delete item;
  }
  updateResourceList();
  emit resourcesChanged();
}

void ResourceView::editResource()
{
  ResourceItem *item = currentItem();

  ResourceCalendar *resource = item->resource();

  KRES::ConfigDialog dlg( this, QString("calendar"), resource,
                          "KRES::ConfigDialog" );

  if ( dlg.exec() ) {
    item->setText( 0, resource->resourceName() );

    mCalendar->resourceManager()->change( resource );
  }
  emitResourcesChanged();
}

void ResourceView::currentChanged( Q3ListViewItem *item)
{
   ResourceItem *i = currentItem();
   if ( !item || i->isSubresource() ) {
     mDeleteButton->setEnabled( false );
     mEditButton->setEnabled( false );
   } else {
     mDeleteButton->setEnabled( true );
     mEditButton->setEnabled( true );
   }
}

ResourceItem *ResourceView::findItem( ResourceCalendar *r )
{
  Q3ListViewItem *item;
  ResourceItem *i = 0;
  for( item = mListView->firstChild(); item; item = item->nextSibling() ) {
    i = static_cast<ResourceItem *>( item );
    if ( i->resource() == r ) break;
  }
  return i;
}

ResourceItem *ResourceView::findItemByIdentifier( const QString& id )
{
  Q3ListViewItem *item;
  ResourceItem *i = 0;
  for( item = mListView->firstChild(); item; item = item->itemBelow() ) {
    i = static_cast<ResourceItem *>( item );
    if ( i->resourceIdentifier() == id )
       return i;
  }
  return 0;
}


void ResourceView::contextMenuRequested ( Q3ListViewItem *i,
                                          const QPoint &pos, int )
{
  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();
  ResourceItem *item = static_cast<ResourceItem *>( i );

  Q3PopupMenu *menu = new Q3PopupMenu( this );
  connect( menu, SIGNAL( aboutToHide() ), menu, SLOT( deleteLater() ) );
  if ( item ) {
    int reloadId = menu->insertItem( i18n("Re&load"), this,
                                     SLOT( reloadResource() ) );
    menu->setItemEnabled( reloadId, item->resource()->isActive() );
    int saveId = menu->insertItem( i18n("&Save"), this,
                                   SLOT( saveResource() ) );
    menu->setItemEnabled( saveId, item->resource()->isActive() );
    menu->insertSeparator();

    menu->insertItem( i18n("Show &Info"), this, SLOT( showInfo() ) );
    //FIXME: This is better on the resource dialog
    if ( KOPrefs::instance()->agendaViewUsesResourceColor() ) {
      Q3PopupMenu *assignMenu= new Q3PopupMenu( menu );
      assignMenu->insertItem( i18n( "&Assign Color" ), this, SLOT( assignColor() ) );
      if ( item->resourceColor().isValid() )
        assignMenu->insertItem( i18n( "&Disable Color" ), this, SLOT( disableColor() ) );
      menu->insertItem( i18n( "Resources Colors" ), assignMenu );
    }

    menu->insertItem( i18n("&Edit..."), this, SLOT( editResource() ) );
    menu->insertItem( i18n("&Remove"), this, SLOT( removeResource() ) );
    if ( item->resource() != manager->standardResource() ) {
      menu->insertSeparator();
      menu->insertItem( i18n("Use as &Default Calendar"), this,
                        SLOT( setStandard() ) );
    }

    menu->insertSeparator();
 }
  menu->insertItem( i18n("&Add..."), this, SLOT( addResource() ) );

  menu->popup( pos );
}

void ResourceView::assignColor()
{
  ResourceItem *item = currentItem();
  if ( !item )
    return;
  // A color without initialized is a color invalid
  QColor myColor;
  KCal::ResourceCalendar *cal = item->resource();

  QString identifier = cal->identifier();
  if ( item->isSubresource() )
    identifier = item->resourceIdentifier();

  QColor defaultColor = KOPrefs::instance()->resourceColor( identifier );

  int result = KColorDialog::getColor( myColor,defaultColor);

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
  if ( !item )
    return;
  QColor colorInvalid;
  KCal::ResourceCalendar *cal = item->resource();
  QString identifier = cal->identifier();
  if ( item->isSubresource() )
    identifier = item->resourceIdentifier();
  KOPrefs::instance()->setResourceColor( identifier, colorInvalid );
  item->setResourceColor( colorInvalid );
  item->update();
  emitResourcesChanged();
}
void ResourceView::showInfo()
{
  ResourceItem *item = currentItem();
  if ( !item ) return;

  QString txt = "<qt>" + item->resource()->infoText() + "</qt>";
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

void ResourceView::setStandard()
{
  ResourceItem *item = currentItem();
  if ( !item ) return;

  ResourceCalendar *r = item->resource();
  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();
  manager->setStandardResource( r );
  updateResourceList();
}

void ResourceView::updateResourceList()
{
  Q3ListViewItemIterator it( mListView );
  ResourceCalendar* stdRes = mCalendar->resourceManager()->standardResource();
  while ( it.current() ) {
    ResourceItem *item = static_cast<ResourceItem *>( it.current() );
    item->setStandardResource( item->resource() == stdRes );
    ++it;
  }
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
