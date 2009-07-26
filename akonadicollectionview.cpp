/*
  This file is part of KOrganizer.

  Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2009 Sebastian Sauer <sebsauer@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "akonadicollectionview.h"
#include "akonadicollectionview.moc"
#include "akonadicalendar.h"
#include "kocore.h"
#include "koprefs.h"

#include <KDebug>
#include <KDialog>
#include <KAction>
#include <KActionCollection>
#include <kjob.h>
#include <QVBoxLayout>
#include <QHeaderView>

#include <akonadi/collection.h>
#include <akonadi/collectionview.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/collectionview.h>
#include <akonadi/collectiondeletejob.h>
#include <akonadi/standardactionmanager.h>
#include <akonadi/agenttypedialog.h>
#include <akonadi/agentinstancewidget.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/agentfilterproxymodel.h>
#include <akonadi/control.h>

AkonadiCollectionViewFactory::AkonadiCollectionViewFactory( KCal::AkonadiCalendar *calendar, CalendarView *view )
  : mCalendar( calendar ) , mView( view ), mAkonadiCollectionView( 0 )
{
}

CalendarViewExtension *AkonadiCollectionViewFactory::create( QWidget *parent )
{
  mAkonadiCollectionView = new AkonadiCollectionView( this, mCalendar, parent );
#if 0
  QObject::connect( mAkonadiCollectionView, SIGNAL(resourcesChanged()), mView, SLOT(resourcesChanged()) );
  QObject::connect( mAkonadiCollectionView, SIGNAL(resourcesChanged()), mView, SLOT(updateCategories()) );
  QObject::connect( mCalendar, SIGNAL(signalResourceAdded(ResourceCalendar *)), mAkonadiCollectionView, SLOT(addResourceItem(ResourceCalendar *)) );
  QObject::connect( mCalendar, SIGNAL(signalResourceModified(ResourceCalendar *)), mAkonadiCollectionView, SLOT(updateResourceItem(ResourceCalendar *)) );
  QObject::connect( mCalendar, SIGNAL(signalResourceAdded(ResourceCalendar *)), mView, SLOT(updateCategories()) );
  QObject::connect( mCalendar, SIGNAL(signalResourceModified(ResourceCalendar *)), mView, SLOT(updateCategories()) );
#endif
  return mAkonadiCollectionView;
}

KCal::AkonadiCalendar* AkonadiCollectionViewFactory::calendar() const
{
  return mCalendar;
}

CalendarView* AkonadiCollectionViewFactory::view() const
{
  return mView;
}

AkonadiCollectionView* AkonadiCollectionViewFactory::collectionView() const
{
  return mAkonadiCollectionView;
}

#if 0
ResourceItem::ResourceItem( ResourceCalendar *resource, AkonadiCollectionView *view, QTreeWidget *parent )
  : QTreeWidgetItem( parent ),
    mResource( resource ), mView( view ), mBlockStateChange( false ),
    mIsSubresource( false ), mResourceIdentifier( QString() ),
    mSubItemsCreated( false ), mIsStandardResource( false ),
    mIsReloading( false )
{
  setFlags( flags() | Qt::ItemIsUserCheckable );

  setText( 0, resource->resourceName() );
  mResourceColor = KOPrefs::instance()->resourceColor( resource->identifier() );
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
    for ( it = subresources.constBegin(); it != subresources.constEnd(); ++it ) {
      if ( !mView->findItemByIdentifier( *it ) ) {
        new ResourceItem( mResource, *it, mResource->labelForSubresource( *it ),
                          mView, this );
      }
    }
  }
  mSubItemsCreated = true;
}

bool ResourceItem::useColors() const
{
  // assign a color, but only if this is a resource that actually
  // hold items at top level
  return ( KOPrefs::instance()->agendaViewColors() != KOPrefs::CategoryOnly ||
           KOPrefs::instance()->monthViewColors()  != KOPrefs::MonthItemCategoryOnly )  &&
         ( mIsSubresource || ( !mIsReloading && mResource->subresources().isEmpty() ) ||
           !mResource->canHaveSubresources() );
}

QVariant ResourceItem::data( int column, int role ) const
{
  if ( column == 0 &&
       role == Qt::DecorationRole &&
       mResourceColor.isValid() &&
       useColors() ) {
    return QVariant( mResourceColor );
  } else {
    return QTreeWidgetItem::data( column, role );
  }
}

ResourceItem::ResourceItem( KCal::ResourceCalendar *resource,
                            const QString &sub, const QString &label,
                            AkonadiCollectionView *view, ResourceItem *parent )

  : QTreeWidgetItem( parent ), mResource( resource ),
    mView( view ), mBlockStateChange( false ), mIsSubresource( true ),
    mSubItemsCreated( false ), mIsStandardResource( false ), mActive( false ),
    mIsReloading( false )
{
  setFlags( flags() | Qt::ItemIsUserCheckable );
  setText( 0, label );
  mResourceColor = KOPrefs::instance()->resourceColor( sub );
  mResourceIdentifier = sub;
  setGuiState();

  treeWidget()->setRootIsDecorated( true );
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
      // mView->requestClose must be called before mResource->save() because
      // save causes closeResource do be called.
      mView->requestClose( mResource );
      if ( mResource->save() ) {
        mResource->setActive( false );
      }
    }

    setExpanded( mResource->isActive() && childCount() > 0 );
  }

  setGuiState();
  mView->emitResourcesChanged();
}

void ResourceItem::update()
{
  setGuiState();
}

void ResourceItem::setResourceColor( QColor &color )
{
  mResourceColor = color ;
}

void ResourceItem::setStandardResource( bool std )
{
  QFont font = qvariant_cast<QFont>( data( 0, Qt::FontRole ) );
  font.setBold( std );
  setData( 0, Qt::FontRole, font );
}
#endif

class CollectionProxyModel : public QSortFilterProxyModel
{
  public:
    explicit CollectionProxyModel(AkonadiCollectionView *view) : QSortFilterProxyModel(view), mView(view) {}
    ~CollectionProxyModel() {}

    virtual Qt::ItemFlags flags(const QModelIndex &index) const
    {
      return QSortFilterProxyModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
      switch(role) {
        case Qt::CheckStateRole: {
          Q_ASSERT( index.isValid() );
          const Akonadi::Collection collection = index.model()->data( index, Akonadi::CollectionModel::CollectionRole ).value<Akonadi::Collection>();
          Q_ASSERT( collection.isValid() );
          return mView->calendar()->hasCollection( collection ) ? Qt::Checked : Qt::Unchecked;
        } break;
        default:
          return QSortFilterProxyModel::data(index, role);
      }
    }

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)
    {
      switch(role) {
        case Qt::CheckStateRole: {
          Q_ASSERT( index.isValid() );
          const Akonadi::Collection collection = index.model()->data( index, Akonadi::CollectionModel::CollectionRole ).value<Akonadi::Collection>();
          Q_ASSERT( collection.isValid() );
          //const bool checked = value.toBool();
          const bool checked = value.toInt() == Qt::Checked;
          if( checked ) {
            Q_ASSERT( ! mView->calendar()->hasCollection( collection ) );
            mView->calendar()->addCollection( collection );
          } else {
            Q_ASSERT( mView->calendar()->hasCollection( collection ) );
            mView->calendar()->removeCollection( collection );
          }
        } return true;
        default:
          return QSortFilterProxyModel::setData(index, value, role);
      }
    }

  private:
    AkonadiCollectionView *mView;
};

AkonadiCollectionView::AkonadiCollectionView( AkonadiCollectionViewFactory *factory, KCal::AkonadiCalendar *calendar, QWidget *parent )
  : CalendarViewExtension( parent ), mFactory(factory), mCalendar( calendar ), mActionManager(0), mCollectionview(0)
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
#if 0
  QHBoxLayout *buttonBox = new QHBoxLayout();
  buttonBox->setSpacing( KDialog::spacingHint() );
  topLayout->addLayout( buttonBox );

  mListView = new QTreeWidget( this );
  mListView->setWhatsThis(
    i18n( "<qt><p>Select on this list the active KOrganizer "
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
  mListView->header()->hide();
  topLayout->addWidget( mListView );

  mSelectedParent = 0;

  connect( mListView, SIGNAL(itemDoubleClicked(QTreeWidgetItem *,int)),
           SLOT(editResource()) );
  connect( mListView, SIGNAL(itemClicked(QTreeWidgetItem *,int)),
           SLOT(slotItemClicked(QTreeWidgetItem *,int)) );
  connect( mListView, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
           SLOT(currentChanged()) );

  mListView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mListView, SIGNAL(customContextMenuRequested(const QPoint &)),
           SLOT(showContextMenu(const QPoint &)) );

  QLabel *calLabel = new QLabel( i18n( "Calendar" ), this );
  buttonBox->addWidget( calLabel );
  buttonBox->addStretch( 1 );

  mAddButton = new QToolButton( this );
  mAddButton->setIcon( KIcon( "list-add" ) );
  buttonBox->addWidget( mAddButton );
  mAddButton->setToolTip( i18n( "Add calendar" ) );
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
  mEditButton = new QToolButton( this );
  mEditButton->setIcon( KIcon( "document-properties" ) );
  buttonBox->addWidget( mEditButton );
  mEditButton->setToolTip( i18n( "Edit calendar settings" ) );
  mEditButton->setWhatsThis(
                   i18n( "Press this button to edit the resource currently "
                         "selected on the KOrganizer resources list above." ) );
  mDeleteButton = new QToolButton( this );
  mDeleteButton->setIcon( KIcon( "edit-delete" ) );
  buttonBox->addWidget( mDeleteButton );
  mDeleteButton->setToolTip( i18n( "Remove calendar" ) );
  mDeleteButton->setWhatsThis(
                   i18n( "Press this button to delete the resource currently "
                         "selected on the KOrganizer resources list above." ) );
  mDeleteButton->setDisabled( true );
  mEditButton->setDisabled( true );

  connect( mAddButton, SIGNAL( clicked() ), SLOT( slotAddButtonClicked() ) );
  connect( mDeleteButton, SIGNAL( clicked() ), SLOT( removeResource() ) );
  connect( mEditButton, SIGNAL( clicked() ), SLOT( editResource() ) );

  setMinimumHeight( 50 );
  mListView->setSortingEnabled( true );
#else
  Akonadi::CollectionModel *collectionmodel = new Akonadi::CollectionModel( this );
  Akonadi::CollectionFilterProxyModel *collectionproxymodel = new Akonadi::CollectionFilterProxyModel( this );
  collectionproxymodel->setSourceModel( collectionmodel );
  collectionproxymodel->addMimeTypeFilter( QString::fromLatin1( "text/calendar" ) );

  CollectionProxyModel *sortmodel = new CollectionProxyModel( this );
  sortmodel->setDynamicSortFilter( true );
  sortmodel->setSortCaseSensitivity( Qt::CaseInsensitive );
  sortmodel->setSourceModel( collectionproxymodel );

  mCollectionview = new Akonadi::CollectionView();
  mCollectionview->header()->hide();
  mCollectionview->setModel( sortmodel );
  mCollectionview->setRootIsDecorated( false );
  //mCollectionview->setSelectionMode( QAbstractItemView::NoSelection );
  KXMLGUIClient *xmlclient = KOCore::self()->xmlguiClient( mFactory->view() );
  if( xmlclient ) {
    mCollectionview->setXmlGuiClient( xmlclient );

    mActionManager = new Akonadi::StandardActionManager( xmlclient->actionCollection(), mCollectionview );
    mActionManager->createAllActions();
    mActionManager->action( Akonadi::StandardActionManager::CreateCollection )->setText( i18n( "Add Calendar..." ) );
    mActionManager->setActionText( Akonadi::StandardActionManager::CopyCollections, ki18np( "Copy Calendar", "Copy %1 Calendars" ) );
    mActionManager->action( Akonadi::StandardActionManager::DeleteCollections )->setText( i18n( "Delete Calendar" ) );
    mActionManager->action( Akonadi::StandardActionManager::SynchronizeCollections )->setText( i18n( "Reload" ) );
    mActionManager->action( Akonadi::StandardActionManager::CollectionProperties )->setText( i18n( "Properties..." ) );
    mActionManager->setCollectionSelectionModel( mCollectionview->selectionModel() );

    mCreateAction = new KAction( mCollectionview );
    mCreateAction->setIcon( KIcon( "appointment-new" ) );
    mCreateAction->setText( i18n( "New Calendar..." ) );
    //mCreateAction->setWhatsThis( i18n( "Create a new contact<p>You will be presented with a dialog where you can add all data about a person, including addresses and phone numbers.</p>" ) );
    xmlclient->actionCollection()->addAction( QString::fromLatin1( "akonadi_calendar_create" ), mCreateAction );
    connect( mCreateAction, SIGNAL( triggered( bool ) ), this, SLOT( newCalendar() ) );

    mDeleteAction = new KAction( mCollectionview );
    mDeleteAction->setIcon( KIcon( "edit-delete" ) );
    mDeleteAction->setText( i18n( "Delete Calendar" ) );
    mDeleteAction->setEnabled( false );
    //mDeleteAction->setWhatsThis( i18n( "Create a new contact<p>You will be presented with a dialog where you can add all data about a person, including addresses and phone numbers.</p>" ) );
    xmlclient->actionCollection()->addAction( QString::fromLatin1( "akonadi_calendar_delete" ), mDeleteAction );
    connect( mDeleteAction, SIGNAL( triggered( bool ) ), this, SLOT( deleteCalendar() ) );
  }
  connect( mCollectionview->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged()) );
  
  topLayout->addWidget( mCollectionview );
#endif
  updateView();
}

AkonadiCollectionView::~AkonadiCollectionView()
{
}

void AkonadiCollectionView::updateView()
{
  kDebug();
#if 0
  mListView->clear();
  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();
  KCal::CalendarResourceManager::Iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it )addResourceItem( *it, false );
  mListView->sortItems( 0, Qt::AscendingOrder );
  emit emitResourcesChanged();
#endif
}

void AkonadiCollectionView::selectionChanged()
{
  kDebug();
mCollectionview->reset();
mCollectionview->model();
  mDeleteAction->setEnabled( mCollectionview->selectionModel()->hasSelection() );
}

void AkonadiCollectionView::newCalendar()
{
  kDebug();
  Akonadi::AgentTypeDialog dlg( this );
  dlg.setWindowTitle( i18n( "Add Calendar" ) );
  dlg.agentFilterProxyModel()->addMimeTypeFilter( QString::fromLatin1( "text/calendar" ) );
  //dlg.agentFilterProxyModel()->addCapabilityFilter( "Resource" ); // show only resources, no agents
  if ( dlg.exec() ) {
    const Akonadi::AgentType agentType = dlg.agentType();
    if ( agentType.isValid() ) {
      Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob( agentType, this );
      job->configure( this );
      connect( job, SIGNAL( result( KJob* ) ), this, SLOT( newCalendarDone( KJob* ) ) );
      job->start();
    }
  }
}

void AkonadiCollectionView::newCalendarDone( KJob *job )
{
  kDebug();
  Akonadi::AgentInstanceCreateJob *createjob = static_cast<Akonadi::AgentInstanceCreateJob*>( job );
  if ( createjob->error() ) {
      kWarning( 5250 ) << "Create calendar failed:" << createjob->errorString();
      return;
  }
  //TODO
}

void AkonadiCollectionView::deleteCalendar()
{
  kDebug();

  QModelIndex index = mCollectionview->selectionModel()->currentIndex(); //selectedRows()
  Q_ASSERT( index.isValid() );
  const Akonadi::Collection collection = index.model()->data( index, Akonadi::CollectionModel::CollectionRole ).value<Akonadi::Collection>();
  Q_ASSERT( collection.isValid() );
  //Q_ASSERT( mCollectionview->selectionModel()->isSelected(index) );
  
  QString displayname = index.model()->data( index, Qt::DisplayRole ).value<QString>();
  Q_ASSERT( ! displayname.isEmpty() );

  if( KMessageBox::questionYesNo( this,
                                  i18n( "Do you really want to delete calendar %1?", displayname ),
                                  i18n( "Delete Calendar" ),
                                  KStandardGuiItem::del(),
                                  KStandardGuiItem::cancel(),
                                  QString(),
                                  KMessageBox::Dangerous )
    == KMessageBox::Yes )
  {
    Akonadi::CollectionDeleteJob *job = new Akonadi::CollectionDeleteJob( collection /* , m_session */ );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( deleteCalendarDone( KJob* ) ) );
  }
}

void AkonadiCollectionView::deleteCalendarDone( KJob *job )
{
  kDebug();
  Akonadi::CollectionDeleteJob *createjob = static_cast<Akonadi::CollectionDeleteJob*>( job );
  if ( createjob->error() ) {
      kWarning( 5250 ) << "Delete calendar failed:" << createjob->errorString();
      return;
  }
  //TODO
}

