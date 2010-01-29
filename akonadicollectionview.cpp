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
#include "kocore.h"
#include "kohelper.h"
#include "koprefs.h"

#include <KDebug>
#include <KDialog>
#include <KAction>
#include <KActionCollection>
#include <kjob.h>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>

#include <akonadi/kcal/calendarmodel.h>
#include <akonadi/kcal/collectionselectionproxymodel.h>
#include <akonadi/kcal/entitymodelstatesaver.h>
#include <akonadi/kcal/collectionselection.h>
#include <akonadi/kcal/utils.h>

#include <akonadi/collection.h>
#include <akonadi/collectionview.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectiondeletejob.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/entitytreeview.h>
#include <akonadi/standardactionmanager.h>
#include <akonadi/agenttypedialog.h>
#include <akonadi/agentinstancewidget.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/agentfilterproxymodel.h>
#include <akonadi/control.h>
#include <akonadi/session.h>
#include <akonadi/changerecorder.h>

#include <QHash>

using namespace Akonadi;

AkonadiCollectionViewFactory::AkonadiCollectionViewFactory( CalendarView *view )
  : mView( view ), mAkonadiCollectionView( 0 )
{
}

namespace {
  class ColorProxyModel : public QSortFilterProxyModel
  {
  public:
    explicit ColorProxyModel( QObject* parent=0 ) : QSortFilterProxyModel( parent ) {}

    /* reimp */ QVariant data( const QModelIndex &index, int role ) const
    {
        if ( !index.isValid() )
            return QVariant();
        if ( role == Qt::DecorationRole ) {
          const Akonadi::Collection collection = Akonadi::collectionFromIndex( index );
          if ( !collection.contentMimeTypes().isEmpty() )
            return KOHelper::resourceColor( collection );
        }
       return QSortFilterProxyModel::data( index, role );
     }
  };
}

CalendarViewExtension *AkonadiCollectionViewFactory::create( QWidget *parent )
{
  mAkonadiCollectionView = new AkonadiCollectionView( view(), parent );
  QObject::connect( mAkonadiCollectionView, SIGNAL(resourcesChanged(bool)), mView, SLOT(resourcesChanged()) );
  QObject::connect( mAkonadiCollectionView, SIGNAL(resourcesChanged(bool)), mView, SLOT(updateCategories()) );
  QObject::connect( mAkonadiCollectionView, SIGNAL( resourcesAddedRemoved() ), mView, SLOT( resourcesChanged() ) );
  QObject::connect( mAkonadiCollectionView, SIGNAL( resourcesAddedRemoved() ), mView, SLOT( updateCategories() ) );
  return mAkonadiCollectionView;
}

CalendarView* AkonadiCollectionViewFactory::view() const
{
  return mView;
}

AkonadiCollectionView* AkonadiCollectionViewFactory::collectionView() const
{
  return mAkonadiCollectionView;
}

AkonadiCollectionView::AkonadiCollectionView( CalendarView* view, QWidget *parent )
  : CalendarViewExtension( parent ), mActionManager(0), mCollectionview(0), mBaseModel( 0 ), mSelectionProxyModel( 0 ), mDeleteAction( 0 ), mNotSendAddRemoveSignal( false )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  Akonadi::CollectionFilterProxyModel *collectionproxymodel = new Akonadi::CollectionFilterProxyModel( this );
  collectionproxymodel->setDynamicSortFilter( true );
  collectionproxymodel->addMimeTypeFilter( QString::fromLatin1( "text/calendar" ) );

  ColorProxyModel* colorProxy = new ColorProxyModel( this );
  colorProxy->setDynamicSortFilter( true );
  colorProxy->setSourceModel( collectionproxymodel );
  mBaseModel = collectionproxymodel;

  mCollectionview = new Akonadi::EntityTreeView( this );
  topLayout->addWidget( mCollectionview );
  mCollectionview->header()->hide();
  mCollectionview->setRootIsDecorated( true );
  mCollectionview->setModel( colorProxy );

  connect( mCollectionview->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
           this, SLOT(selectionChanged()) );

  connect( mBaseModel, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
           this, SLOT( rowsInserted( const QModelIndex&, int, int ) ) );

  //mCollectionview->setSelectionMode( QAbstractItemView::NoSelection );
  KXMLGUIClient *xmlclient = KOCore::self()->xmlguiClient( view );
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
}

AkonadiCollectionView::~AkonadiCollectionView()
{
}

void AkonadiCollectionView::setCollectionSelectionProxyModel( CollectionSelectionProxyModel* m )
{
  if ( mSelectionProxyModel == m )
    return;
  mSelectionProxyModel = m;
  if ( !mSelectionProxyModel )
    return;
  mBaseModel->setSourceModel( mSelectionProxyModel );
  connect( mSelectionProxyModel->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged()) );
}

Akonadi::EntityTreeView* AkonadiCollectionView::view() const
{
  return mCollectionview;
}

void AkonadiCollectionView::updateView()
{
  emit resourcesChanged( mSelectionProxyModel ? mSelectionProxyModel->selectionModel()->hasSelection() : false );
}

void AkonadiCollectionView::selectionChanged()
{
  kDebug();
  if ( mDeleteAction )
    mDeleteAction->setEnabled( mCollectionview->selectionModel()->hasSelection() );
  updateView();
}

void AkonadiCollectionView::newCalendar()
{
  kDebug();
  Akonadi::AgentTypeDialog dlg( this );
  dlg.setWindowTitle( i18n( "Add Calendar" ) );
  dlg.agentFilterProxyModel()->addMimeTypeFilter( QString::fromLatin1( "text/calendar" ) );
  dlg.agentFilterProxyModel()->addCapabilityFilter( "Resource" ); // show only resources, no agents
  if ( dlg.exec() ) {
    mNotSendAddRemoveSignal = true;
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
    //TODO(AKONADI_PORT) this should show an error dialog and should be merged with the identical code in ActionManager
    kWarning( 5250 ) << "Create calendar failed:" << createjob->errorString();
    mNotSendAddRemoveSignal = false;
    return;
  }
  mNotSendAddRemoveSignal = false;
  //TODO
}

void AkonadiCollectionView::deleteCalendar()
{
  kDebug();

  QModelIndex index = mCollectionview->selectionModel()->currentIndex(); //selectedRows()
  Q_ASSERT( index.isValid() );
  const Akonadi::Collection collection = collectionFromIndex( index );
  Q_ASSERT( collection.isValid() );
  //Q_ASSERT( mCollectionview->selectionModel()->isSelected(index) );

  const QString displayname = index.model()->data( index, Qt::DisplayRole ).toString();
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
    mNotSendAddRemoveSignal = true;
    Akonadi::CollectionDeleteJob *job = new Akonadi::CollectionDeleteJob( collection, this );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( deleteCalendarDone( KJob* ) ) );
  }
}

void AkonadiCollectionView::deleteCalendarDone( KJob *job )
{
  kDebug();
  Akonadi::CollectionDeleteJob *deletejob = static_cast<Akonadi::CollectionDeleteJob*>( job );
  if ( deletejob->error() ) {
    kWarning( 5250 ) << "Delete calendar failed:" << deletejob->errorString();
    mNotSendAddRemoveSignal = false;
    return;
  }
  mNotSendAddRemoveSignal = false;
  //TODO
}

void AkonadiCollectionView::rowsInserted( const QModelIndex&, int, int )
{
  if ( !mNotSendAddRemoveSignal )
    emit resourcesAddedRemoved();
}

#include "akonadicollectionview.moc" // for EntityModelStateSaver Q_PRIVATE_SLOT
