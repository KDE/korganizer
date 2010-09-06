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
#include "koviewmanager.h"
#include "multiagendaview.h"

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
#include <qregexp.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include "koprefs.h"

using namespace KCal;

static QString scrubDirectory( const QString &subRes )
{
  QString nsubRes = subRes;

  // first, replace leading ".foo.directory" with "foo"
  if ( subRes.contains( QRegExp( "^\\..*\\.directory" ) ) ) {
    nsubRes.remove( ".directory" );
    nsubRes.remove( 0, 1 ); // remove leading '.'
  }

  // now replace "/." with "/"  (needed for sub-sublevel-directories)
  nsubRes.replace( QRegExp( "/\\." ),  "/" );

  // cleaning complete
  return nsubRes;
}

static QString labelFromSubResName( ResourceCalendar *resource, const QString &subRes )
{

  DCOPRef ref( "kmail", "KMailICalIface" );
  DCOPReply reply = ref.call( "dimapAccounts" );
  if ( !reply.isValid() ) {
    kdDebug() << "DCOP Call dimapAccounts() failed " << endl;
    return QString();
  }

  QString label;
  if ( (int)reply > 1 ) {
    if( resource && !resource->resourceName().isEmpty() ) {
      label = i18n( "My %1 (%2)" ).arg( subRes, resource->resourceName() );
    } else {
      label = i18n( "My %1" ).arg( scrubDirectory( subRes ) );
    }
  } else {
    label = i18n( "My %1" ).arg( scrubDirectory( subRes ) );
  }
  return label;
}

static QString labelFromIdentifier( ResourceCalendar *resource, const QString &identifier )
{
  QString subResLabel;
  if ( identifier.contains( "/.INBOX.directory/" ) ) { // my subresource
    QString subResName = identifier;
    subResName.remove( QRegExp( "^.*/\\.INBOX\\.directory/" ) );
    subResLabel = labelFromSubResName( resource, subResName );
  }
  return subResLabel;
}

ResourceViewFactory::ResourceViewFactory( CalendarResources *calendar, CalendarView *view )
  : mCalendar( calendar ), mCalendarView( view ), mResourceView( 0 )
{
}

CalendarViewExtension *ResourceViewFactory::create( QWidget *parent )
{
  mResourceView = new ResourceView( mCalendar, mCalendarView, parent );

  QObject::connect( mResourceView, SIGNAL( resourcesChanged() ),
                    mCalendarView, SLOT( resourcesChanged() ) );
  QObject::connect( mResourceView, SIGNAL( resourcesChanged() ),
                    mCalendarView, SLOT( updateCategories() ) );

  QObject::connect( mCalendar,
                    SIGNAL( signalResourceAdded( ResourceCalendar * ) ),
                    mResourceView,
                    SLOT( addResourceItem( ResourceCalendar * ) ) );
  QObject::connect( mCalendar,
                    SIGNAL( signalResourceModified( ResourceCalendar * ) ),
                    mResourceView,
                    SLOT( updateResourceItem( ResourceCalendar * ) ) );
  QObject::connect( mCalendar, SIGNAL( signalResourceAdded( ResourceCalendar * ) ),
                    mCalendarView, SLOT( updateCategories() ) );
  QObject::connect( mCalendar, SIGNAL( signalResourceModified( ResourceCalendar * ) ),
                    mCalendarView, SLOT( updateCategories() ) );

  return mResourceView;
}

ResourceView *ResourceViewFactory::resourceView() const
{
  return mResourceView;
}

ResourceItem::ResourceItem( ResourceCalendar *resource, ResourceView *view,
                            KListView *parent )
  : QCheckListItem( parent, resource->resourceName(), CheckBox ),
    mResource( resource ), mResourceView( view ), mBlockStateChange( false ),
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
      QString text = labelFromIdentifier( mResource, *it );
      if ( text.isEmpty() ) {
        text = mResource->labelForSubresource( *it );
      }
      ResourceItem *item = new ResourceItem( mResource, *it, text, mResourceView, this );
      QColor resourceColor = *KOPrefs::instance()->resourceColor( *it );
      item->setResourceColor( resourceColor );
      item->update();
    }
  }
  mSubItemsCreated = true;
}

ResourceItem::ResourceItem( ResourceCalendar *resource, const QString &identifier,
                            const QString &label, ResourceView *view, ResourceItem *parent )
  : QCheckListItem( parent, label, CheckBox ), mResource( resource ),
    mResourceView( view ), mBlockStateChange( false ), mIsSubresource( true ),
    mSubItemsCreated( false ), mIsStandardResource( false )
{
  mResourceColor = QColor();
  mResourceIdentifier = identifier;
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
      // mResourceView->requestClose must be called before mResource->save() because
      // save causes closeResource do be called.
      mResourceView->requestClose( mResource );
      if ( mResource->save() ) {
        mResource->setActive( false );
      }
    }

    setOpen( mResource->isActive() && childCount() > 0 );

    setGuiState();
  }

  mResourceView->emitResourcesChanged();
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


ResourceView::ResourceView( CalendarResources *calendar,
                            CalendarView *view, QWidget *parent, const char *name )
  : CalendarViewExtension( parent, name ), mCalendar( calendar ), mCalendarView( view )
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

  CalendarResourceManager *manager = mCalendar->resourceManager();

  CalendarResourceManager::Iterator it;
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
  CalendarResourceManager *manager = mCalendar->resourceManager();
  ResourceItem *item = static_cast<ResourceItem*>( mListView->selectedItem() );
  if ( item && ( item->isSubresource() || item->resource()->canHaveSubresources() ) ) {
    const QString folderName =
      KInputDialog::getText( i18n( "Add Subresource" ),
                             i18n( "Please enter a name for the new subresource" ), QString::null,
                             &ok, this );
    if ( !ok )
      return;
    const QString parentId = item->isSubresource() ? item->resourceIdentifier() : QString:: null;
    if ( !item->resource()->addSubresource( folderName, parentId ) ) {
      KMessageBox::error(
        this,
        i18n( "<qt>Unable to create subresource <b>%1</b>.</qt>" ).arg( folderName ) );
    }
    return;
  }

  QStringList types = manager->resourceTypeNames();
  QStringList descs = manager->resourceTypeDescriptions();
  QString desc =
    KInputDialog::getItem( i18n( "Resource Configuration" ),
                           i18n( "Please select type of the new resource:" ),
                           descs, 0, false, &ok, this );
  if ( !ok ) {
    return;
  }

  QString type = types[ descs.findIndex( desc ) ];

  // Create new resource
  ResourceCalendar *resource = manager->createResource( type );
  if( !resource ) {
    KMessageBox::error(
      this,
      i18n( "<qt>Unable to create resource of type <b>%1</b>.</qt>" ).arg( type ) );
    return;
  }

  KRES::ConfigDialog *dlg =
    new KRES::ConfigDialog( this, QString( "calendar" ), resource, "KRES::ConfigDialog" );

  bool success = true;
  if ( !dlg || !dlg->exec() )
    success = false;

  if ( success ) {
    resource->setTimeZoneId( KOPrefs::instance()->mTimeZoneId );
    if ( resource->isActive() && ( !resource->open() || !resource->load() ) ) {
      // ### There is a resourceLoadError() signal declared in ResourceCalendar
      //     but no subclass seems to make use of it. We could do better.
      KMessageBox::error( this, i18n("Unable to create the resource.").arg( type ) );
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

  ResourceItem *item = new ResourceItem( resource, this, mListView );

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
void ResourceView::slotSubresourceAdded( ResourceCalendar *resource,
                                         const QString &type,
                                         const QString &identifier,
                                         const QString &label )
{
  Q_UNUSED( type );

  QListViewItem *lvitem = mListView->findItem( resource->resourceName(), 0 );
  if ( !lvitem )
    // Not found
    return;

  if ( findItemByIdentifier( identifier ) ) return;

  QString text = labelFromIdentifier( resource, identifier );
  if ( text.isEmpty() ) {
    text = label;
  }
  ResourceItem *item = static_cast<ResourceItem *>( lvitem );
  ResourceItem *newItem = new ResourceItem( resource, identifier, text, this, item );
  QColor resourceColor = *KOPrefs::instance()->resourceColor( identifier );
  newItem->setResourceColor( resourceColor );
}

// Remove an entry
void ResourceView::slotSubresourceRemoved( ResourceCalendar *resource,
                                           const QString &type,
                                           const QString &identifier )
{
  Q_UNUSED( resource );
  Q_UNUSED( type );

  delete findItemByIdentifier( identifier );
  emit resourcesChanged();
}

void ResourceView::closeResource( ResourceCalendar *resource )
{
  if ( mResourcesToClose.find( resource ) >= 0 ) {
    resource->close();
    mResourcesToClose.remove( resource );
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

  // Do not allow a non-subresource folder to be removed if it is the standard resource.
  if ( !item->isSubresource() ) {
    if ( item->resource() == mCalendar->resourceManager()->standardResource() ) {
      KMessageBox::sorry(
        this,
        i18n( "<qt>You may not delete your standard calendar resource.<p>"
              "You can change the standard calendar resource in the "
              "KDE Control Center using the KDE Resource settings under the "
              "KDE Components area.</qt>" ) );
      return;
    }
  }

  QString moreInfo;
  if ( item->resource()->type() == "imap" || item->resource()->type() == "scalix" ) {
    moreInfo = i18n( "This is a groupware folder so you can always re-subscribe to the folder "
                     "later as you desire." );
  } else {
    moreInfo = i18n( "The contents will not be removed so you can always re-add this calendar "
                     "later as you desire." );
  }

  int km =
    KMessageBox::warningContinueCancel(
      this,
      i18n( "<qt>Do you really want to remove the calendar <b>%1</b>?<p><b>Note:</b> %2</qt>" ).
      arg( item->text( 0 ), moreInfo ),
      "", KGuiItem( i18n( "&Remove" ) ) );
  if ( km == KMessageBox::Cancel ) {
    return;
  }

  if ( item->isSubresource() ) {
    if ( !item->resource()->removeSubresource( item->resourceIdentifier() ) )
      KMessageBox::sorry(
        this,
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
      if ( !identifier.contains( "/.INBOX.directory/" ) ) {
        KMessageBox::sorry(
          this,
          i18n( "Cannot rename someone else's calendar folder." ) );
        return;
      }

      QString oldSubResourceName = identifier;
      oldSubResourceName.remove( QRegExp( "^.*/\\.INBOX\\.directory/" ) );
      QString newSubResourceName =
        KInputDialog::getText(
          i18n( "Rename Subresource" ),
          i18n( "<qt>Enter a new name for the subresource<p>"
                "<b>Note:</b> the new name will take affect after the next sync.</qt>" ),
          oldSubResourceName, &ok, this );
      if ( !ok ) {
        return;
      }

      DCOPRef ref( "kmail", "KMailICalIface" );
      DCOPReply reply = ref.call( "changeResourceUIName", identifier, newSubResourceName );
      if ( !reply.isValid() ) {
        KMessageBox::sorry(
          this,
          i18n( "Communication with KMail failed when attempting to change the folder name." ) );
        return;
      }

      item->setText( 0, labelFromSubResName( resource, newSubResourceName ) );

      KOrg::BaseView *cV = mCalendarView->viewManager()->currentView();
      if ( cV && cV == mCalendarView->viewManager()->multiAgendaView() ) {
        mCalendarView->viewManager()->multiAgendaView()->deSelectAgendaView();
      }
    } else {
      KMessageBox::sorry(
        this,
        i18n ("<qt>Cannot edit the subresource <b>%1</b>.</qt>").arg( item->resource()->name() ) );
    }
  } else {
    KRES::ConfigDialog dlg( this, QString("calendar"), resource, "KRES::ConfigDialog" );

    if ( dlg.exec() ) {
      item->setText( 0, resource->resourceName() );
      mCalendar->resourceManager()->change( resource );
    }
  }
  emitResourcesChanged();
}

void ResourceView::currentChanged( QListViewItem *lvitem )
{
  ResourceItem *item = currentItem();
  if ( !lvitem || item->isSubresource() ) {
    mDeleteButton->setEnabled( false );
    mEditButton->setEnabled( false );
  } else {
    mDeleteButton->setEnabled( true );
    mEditButton->setEnabled( true );
  }
}

ResourceItem *ResourceView::findItem( ResourceCalendar *resource )
{
  QListViewItem *lvitem;
  ResourceItem *item = 0;
  for( lvitem = mListView->firstChild(); lvitem; lvitem = lvitem->nextSibling() ) {
    item = static_cast<ResourceItem *>( lvitem );
    if ( item->resource() == resource ) break;
  }
  return item;
}

ResourceItem *ResourceView::findItemByIdentifier( const QString &identifier )
{
  QListViewItem *lvitem;
  ResourceItem *item = 0;
  for ( lvitem = mListView->firstChild(); lvitem; lvitem = lvitem->itemBelow() ) {
    item = static_cast<ResourceItem *>( lvitem );
    if ( item->resourceIdentifier() == identifier )
       return item;
  }
  return 0;
}

void ResourceView::contextMenuRequested ( QListViewItem *lvitem, const QPoint &pos, int )
{
  CalendarResourceManager *manager = mCalendar->resourceManager();
  ResourceItem *item = static_cast<ResourceItem *>( lvitem );

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

    if ( item->isSubresource() &&
         ( item->resource()->type() == "imap" || item->resource()->type() == "scalix" ) ) {
      if ( item->resourceIdentifier().contains( "/.INBOX.directory/" ) ) {
        menu->insertItem( i18n("&Rename..."), this, SLOT( editResource() ) );
      }
    } else {
      menu->insertItem( i18n("&Edit..."), this, SLOT( editResource() ) );
    }
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
  ResourceCalendar *cal = item->resource();

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
  if ( !item ) {
    return;
  }

  QColor colorInvalid;
  ResourceCalendar *cal = item->resource();
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
  if ( !item ) return;

  QString identifier;
  if ( item->isSubresource() ) {
    identifier = "<p>" + item->resourceIdentifier();
  }

  QString txt = "<qt>" + item->resource()->infoText() + identifier + "</qt>";
  KMessageBox::information( this, txt );
}

void ResourceView::reloadResource()
{
  ResourceItem *item = currentItem();
  if ( !item ) return;

  ResourceCalendar *resource = item->resource();
  resource->load();
}

void ResourceView::saveResource()
{
  ResourceItem *item = currentItem();
  if ( !item ) return;

  ResourceCalendar *resource = item->resource();
  resource->save();
}

void ResourceView::setStandard()
{
  ResourceItem *item = currentItem();
  if ( !item ) return;

  ResourceCalendar *resource = item->resource();
  CalendarResourceManager *manager = mCalendar->resourceManager();
  manager->setStandardResource( resource );
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
