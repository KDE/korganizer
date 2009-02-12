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

#include <dcopref.h>
#include <kcolordialog.h>
#include <kdialog.h>
#include <klistview.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kiconloader.h>
#include <kresources/resource.h>
#include <kresources/configdialog.h>
#include <libkcal/calendarresources.h>
#include <kconfig.h>

#include <qhbox.h>
#include <qheader.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

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
                    mView, SLOT( resourcesChanged() ) );
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
  : QCheckListItem( parent, resource->resourceName(), CheckBox ),
    mResource( resource ), mView( view ), mBlockStateChange( false ),
    mIsSubresource( false ), mResourceIdentifier( QString::null ),
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
      QColor resourceColor = *KOPrefs::instance()->resourceColor( *it );
      item->setResourceColor( resourceColor );
      item->update();
    }
  }
  mSubItemsCreated = true;
}

ResourceItem::ResourceItem( KCal::ResourceCalendar *resource,
                            const QString& sub, const QString& label,
                            ResourceView *view, ResourceItem* parent )

  : QCheckListItem( parent, label, CheckBox ), mResource( resource ),
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
      // mView->requestClose must be called before mResource->save() because
      // save causes closeResource do be called.
      mView->requestClose( mResource );
      if ( mResource->save() ) {
        mResource->setActive( false );
      }
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
    setPixmap(0,0);
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
  QCheckListItem::paintCell( p, cg, column, width, alignment );
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

  QHBoxLayout *buttonBox = new QHBoxLayout();
  buttonBox->setSpacing( KDialog::spacingHint() );
  topLayout->addLayout( buttonBox );

  QLabel *calLabel = new QLabel( i18n( "Calendar" ), this );
  buttonBox->addWidget( calLabel );
  buttonBox->addStretch( 1 );

  mAddButton = new QPushButton( this, "add" );
  mAddButton->setIconSet( SmallIconSet( "add" ) );
  buttonBox->addWidget( mAddButton );
  QToolTip::add( mAddButton, i18n( "Add calendar" ) );
  QWhatsThis::add( mAddButton,
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
  mEditButton = new QPushButton( this, "edit" );
  mEditButton->setIconSet( SmallIconSet( "edit" ) );
  buttonBox->addWidget( mEditButton );
  QToolTip::add( mEditButton, i18n( "Edit calendar settings" ) );
  QWhatsThis::add( mEditButton,
                   i18n( "Press this button to edit the resource currently "
                         "selected on the KOrganizer resources list above." ) );
  mDeleteButton = new QPushButton( this, "del" );
  mDeleteButton->setIconSet( SmallIconSet( "remove" ) );
  buttonBox->addWidget( mDeleteButton );
  QToolTip::add( mDeleteButton, i18n( "Remove calendar" ) );
  QWhatsThis::add( mDeleteButton,
                   i18n( "Press this button to delete the resource currently "
                         "selected on the KOrganizer resources list above." ) );
  mDeleteButton->setDisabled( true );
  mEditButton->setDisabled( true );

  mListView = new KListView( this );
  mListView->header()->hide();
  QWhatsThis::add( mListView,
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
  mListView->setResizeMode( QListView::LastColumn );
  topLayout->addWidget( mListView );

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
  bool ok = false;
  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();
  ResourceItem *i = static_cast<ResourceItem*>( mListView->selectedItem() );
  if ( i && ( i->isSubresource() || i->resource()->canHaveSubresources() ) ) {
    const QString folderName = KInputDialog::getText( i18n( "Add Subresource" ),
            i18n( "Please enter a name for the new subresource" ), QString::null,
            &ok, this );
    if ( !ok )
      return;
    const QString parentId = i->isSubresource() ? i->resourceIdentifier() : QString:: null;
    if ( !i->resource()->addSubresource( folderName, parentId ) ) {
      KMessageBox::error( this, i18n("<qt>Unable to create subresource <b>%1</b>.</qt>")
                                .arg( folderName ) );
    }
    return;
  }

  QStringList types = manager->resourceTypeNames();
  QStringList descs = manager->resourceTypeDescriptions();
  QString desc = KInputDialog::getItem( i18n( "Resource Configuration" ),
      i18n( "Please select type of the new resource:" ), descs, 0, false, &ok,
            this );
  if ( !ok )
    return;

  QString type = types[ descs.findIndex( desc ) ];

  // Create new resource
  ResourceCalendar *resource = manager->createResource( type );
  if( !resource ) {
    KMessageBox::error( this, i18n("<qt>Unable to create resource of type <b>%1</b>.</qt>")
                              .arg( type ) );
    return;
  }

  resource->setResourceName( i18n("%1 resource").arg( type ) );

  KRES::ConfigDialog *dlg = new KRES::ConfigDialog( this, QString("calendar"), resource,
                          "KRES::ConfigDialog" );

  bool success = true;
  if ( !dlg || !dlg->exec() )
    success = false;

  if ( success ) {
    resource->setTimeZoneId( KOPrefs::instance()->mTimeZoneId );
    if ( resource->isActive() && ( !resource->open() || !resource->load() ) ) {
      // ### There is a resourceLoadError() signal declared in ResourceCalendar
      //     but no subclass seems to make use of it. We could do better.
      KMessageBox::error( this, i18n("Unable to create the resource.")
                                .arg( type ) );
      success = false;
    }
  }

  if ( success ) {
    manager->add( resource );
    // we have to call resourceAdded manually, because for in-process changes
    // the dcop signals are not connected, so the resource's signals would not
    // be connected otherwise
    mCalendar->resourceAdded( resource );
  }

  if ( !success )
    delete resource;

  delete dlg;

  //### maybe only do this if ( success )
  emitResourcesChanged();
}

void ResourceView::addResourceItem( ResourceCalendar *resource )
{

  ResourceItem *item=new ResourceItem( resource, this, mListView );

  // assign a color, but only if this is a resource that actually
  // hold items at top level
  if ( !resource->canHaveSubresources() || resource->subresources().isEmpty() ) {
      QColor resourceColor = *KOPrefs::instance()->resourceColor(resource->identifier());
      item->setResourceColor(resourceColor);
      item->update();
  }

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
  QListViewItem *i = mListView->findItem( calendar->resourceName(), 0 );
  if ( !i )
    // Not found
    return;

  if ( findItemByIdentifier( resource ) ) return;

  ResourceItem *item = static_cast<ResourceItem *>( i );
  ResourceItem *newItem = new ResourceItem( calendar, resource, label, this, item );
  QColor resourceColor = *KOPrefs::instance()->resourceColor( resource );
  newItem->setResourceColor( resourceColor );
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

  const QString warningMsg = item->isSubresource() ?
        i18n("<qt>Do you really want to remove the subresource <b>%1</b>? "
              "Note that its contents will be completely deleted. This "
              "operation cannot be undone. </qt>").arg( item->text( 0 ) ) :
        i18n("<qt>Do you really want to remove the resource <b>%1</b>?</qt>").arg( item->text( 0 ) );

  int km = KMessageBox::warningContinueCancel( this, warningMsg, "",
        KGuiItem( i18n("&Remove" ), "editdelete") );
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
    if ( !item->resource()->removeSubresource( item->resourceIdentifier() ) )
      KMessageBox::sorry( this,
              i18n ("<qt>Failed to remove the subresource <b>%1</b>. The "
                  "reason could be that it is a built-in one which cannot "
                  "be removed, or that the removal of the underlying storage "
                  "folder failed.</qt>").arg( item->resource()->name() ) );
      return;
  } else {
    mCalendar->resourceManager()->remove( item->resource() );
  }
    mListView->takeItem( item );
    delete item;

  updateResourceList();
  emit resourcesChanged();
}

void ResourceView::editResource()
{
  bool ok = false;
  ResourceItem *item = currentItem();
  if (!item) return;
  ResourceCalendar *resource = item->resource();

   if ( item->isSubresource() ) {
     if ( resource->type() == "imap" || resource->type() == "scalix" ) {
        QString identifier = item->resourceIdentifier();
        const QString newResourceName = KInputDialog::getText( i18n( "Rename Subresource" ),
               i18n( "Please enter a new name for the subresource" ), item->text(),
                    &ok, this );
        if ( !ok )
          return;

        DCOPRef ref( "kmail", "KMailICalIface" );
        DCOPReply reply = ref.call( "changeResourceUIName", identifier, newResourceName );
        if ( !reply.isValid() ) {
           kdDebug() << "DCOP Call changeResourceUIName() failed " << endl;
        }
     } else {
           KMessageBox::sorry( this,
                               i18n ("<qt>Cannot edit the subresource <b>%1</b>.</qt>").arg( item->resource()->name() ) );
       }
   } else {
     KRES::ConfigDialog dlg( this, QString("calendar"), resource,
                          "KRES::ConfigDialog" );

     if ( dlg.exec() ) {
       item->setText( 0, resource->resourceName() );

      mCalendar->resourceManager()->change( resource );
     }
   }
   emitResourcesChanged();

}

void ResourceView::currentChanged( QListViewItem *item )
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
  QListViewItem *item;
  ResourceItem *i = 0;
  for( item = mListView->firstChild(); item; item = item->nextSibling() ) {
    i = static_cast<ResourceItem *>( item );
    if ( i->resource() == r ) break;
  }
  return i;
}

ResourceItem *ResourceView::findItemByIdentifier( const QString& id )
{
  QListViewItem *item;
  ResourceItem *i = 0;
  for( item = mListView->firstChild(); item; item = item->itemBelow() ) {
    i = static_cast<ResourceItem *>( item );
    if ( i->resourceIdentifier() == id )
       return i;
  }
  return 0;
}


void ResourceView::contextMenuRequested ( QListViewItem *i,
                                          const QPoint &pos, int )
{
  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();
  ResourceItem *item = static_cast<ResourceItem *>( i );

  QPopupMenu *menu = new QPopupMenu( this );
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
    if ( KOPrefs::instance()->agendaViewColors() != KOPrefs::CategoryOnly ) {
      QPopupMenu *assignMenu= new QPopupMenu( menu );
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

  QColor defaultColor =*KOPrefs::instance()->resourceColor( identifier );

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
  QListViewItemIterator it( mListView );
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
