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

using namespace Akonadi;

AkonadiCollectionViewFactory::AkonadiCollectionViewFactory( CalendarModel *model, CalendarView *view )
  : mModel( model ), mView( view ), mAkonadiCollectionView( 0 )
{
  Q_ASSERT( model );
}

CalendarViewExtension *AkonadiCollectionViewFactory::create( QWidget *parent )
{
  mAkonadiCollectionView = new AkonadiCollectionView( this, mModel, parent );
  QObject::connect( mAkonadiCollectionView, SIGNAL(resourcesChanged(bool)), mView, SLOT(resourcesChanged()) );
  QObject::connect( mAkonadiCollectionView, SIGNAL(resourcesChanged(bool)), mView, SLOT(updateCategories()) );
#if 0
  QObject::connect( mCalendar, SIGNAL(signalResourceAdded(ResourceCalendar *)), mAkonadiCollectionView, SLOT(addResourceItem(ResourceCalendar *)) );
  QObject::connect( mCalendar, SIGNAL(signalResourceModified(ResourceCalendar *)), mAkonadiCollectionView, SLOT(updateResourceItem(ResourceCalendar *)) );
  QObject::connect( mCalendar, SIGNAL(signalResourceAdded(ResourceCalendar *)), mView, SLOT(updateCategories()) );
  QObject::connect( mCalendar, SIGNAL(signalResourceModified(ResourceCalendar *)), mView, SLOT(updateCategories()) );
#endif
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

static Collection collectionFromIndex( const QModelIndex &index ) {
  return index.model()->data( index, Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
}

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
      if ( !index.isValid() )
        return QVariant();
      switch(role) {
        case Qt::DecorationRole: {
            const Akonadi::Collection collection = collectionFromIndex( index );
            return KOHelper::resourceColor( collection );
        }
        case Qt::CheckStateRole: {
          if ( index.column() != CalendarModel::CollectionTitle )
            return QVariant();
          return mView->checkedCollectionsModel()->isSelected( index ) ? Qt::Checked : Qt::Unchecked;
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
          if ( index.column() != CalendarModel::CollectionTitle )
            return false;
          const Akonadi::Collection collection = index.model()->data( index, Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
          Q_ASSERT( collection.isValid() );
          //const bool checked = value.toBool();
          const bool checked = value.toInt() == Qt::Checked;
          mView->checkedCollectionsModel()->select( index, checked ? QItemSelectionModel::Select : QItemSelectionModel::Deselect );
        } return true;
        default:
          return QSortFilterProxyModel::setData(index, value, role);
      }
    }

    bool filterAcceptsColumn( int source_column, const QModelIndex& source_parent ) const
    {
      return source_column == CalendarModel::CollectionTitle;
    }
  private:
    AkonadiCollectionView *mView;
};

AkonadiCollectionView::AkonadiCollectionView( AkonadiCollectionViewFactory *factory, CalendarModel* calendarModel, QWidget *parent )
  : CalendarViewExtension( parent ), mActionManager(0), mCollectionview(0)
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  Akonadi::CollectionFilterProxyModel *collectionproxymodel = new Akonadi::CollectionFilterProxyModel( this );
  collectionproxymodel->setSourceModel( calendarModel );
  collectionproxymodel->addMimeTypeFilter( QString::fromLatin1( "text/calendar" ) );

  mProxyModel = new CollectionProxyModel( this );
  mProxyModel->setDynamicSortFilter( true );
  mProxyModel->setSortCaseSensitivity( Qt::CaseInsensitive );
  mProxyModel->setSourceModel( collectionproxymodel );

  mCheckedCollectionsModel = new QItemSelectionModel( mProxyModel );

  mCollectionview = new Akonadi::EntityTreeView;
  topLayout->addWidget( mCollectionview );
  mCollectionview->header()->hide();
  mCollectionview->setModel( mProxyModel );
  mCollectionview->setRootIsDecorated( true );
  //mCollectionview->setSelectionMode( QAbstractItemView::NoSelection );
  KXMLGUIClient *xmlclient = KOCore::self()->xmlguiClient( factory->view() );
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
  
  updateView();
}

AkonadiCollectionView::~AkonadiCollectionView()
{
}

QItemSelectionModel* AkonadiCollectionView::checkedCollectionsModel() const
{
  return mCheckedCollectionsModel;
}

void AkonadiCollectionView::updateView()
{
  emit resourcesChanged( true );
}

void AkonadiCollectionView::selectionChanged()
{
  kDebug();
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

