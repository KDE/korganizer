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

namespace {
  struct Role {
    QByteArray identifier;
    int role;
    QVariant defaultValue;
  };
}

class ModelStateSaver::Private
{
  ModelStateSaver* const q;
public:
  explicit Private( QAbstractItemModel* m, ModelStateSaver* qq ) : q( qq ), model( m ) {}
  void rowsInserted( const QModelIndex&, int, int );

  QAbstractItemModel* const model;
  QHash<QString, QVector<QPair<int,QVariant> > > pendingProperties;
  QHash<QByteArray,Role> roles;

  void saveState( const QModelIndex &index, QHash<QByteArray,QVector<QPair<QString, QVariant> > > &values )
  {
    const QString cfgKey = q->key( index );

    Q_FOREACH( const Role &r, roles )
    {
      const QVariant v = index.data( r.role );
      if ( v != r.defaultValue )
        values[r.identifier].push_back( qMakePair( cfgKey, v ) );
    }
    const int rowCount = model->rowCount( index );
    for ( int i = 0; i < rowCount; ++i ) {
      const QModelIndex child = model->index( i, 0, index );
      saveState( child, values );
    }
  }

  void restoreState( const QModelIndex &index )
  {
    const QString key = q->key( index );
    if ( pendingProperties.contains( key ) ) {
      typedef QPair<int,QVariant> IntVariantPair;
      Q_FOREACH ( const IntVariantPair &i, pendingProperties.value( key ) )
        if ( index.data( i.first ) != i.second )
          model->setData( index, i.second, i.first );
      pendingProperties.remove( key );
    }

    const int rowCount = model->rowCount( index );
    for ( int i = 0; i < rowCount && !pendingProperties.isEmpty(); ++i ) {
      const QModelIndex child = model->index( i, 0, index );
      restoreState( child );
    }
  }
};

void ModelStateSaver::Private::rowsInserted( const QModelIndex &index, int start, int end )
{
  for ( int i = start; i <= end && !pendingProperties.isEmpty(); ++i ) {
    const QModelIndex child = model->index( i, 0, index );
    restoreState( child );
  }

  if ( pendingProperties.isEmpty() )
    model->disconnect( q );
}

ModelStateSaver::ModelStateSaver( QAbstractItemModel* model, QObject* parent ) : QObject( parent ), d( new Private( model, this ) )
{
}

ModelStateSaver::~ModelStateSaver()
{
  delete d;
}

void ModelStateSaver::restoreConfig( const KConfigGroup &configGroup )
{
  Q_FOREACH ( const Role &r, d->roles ) {
    const QByteArray ck = QByteArray("Role_") + r.identifier;
    const QStringList l = configGroup.readEntry( ck.constData(), QStringList() );
    if ( l.isEmpty() )
      continue;
    if ( l.size() % 2 != 0 ) {
      kWarning() << "Ignoring invalid configuration value because of odd number of list entries:" << ck;
      continue;
    }
    QStringList::ConstIterator it = l.constBegin();
    while ( it != l.constEnd() ) {
      const QString key = *it;
      ++it;
      const QVariant value = *it;
      ++it;
      d->pendingProperties[key].append( qMakePair( r.role, value ) );
    }
  }

  // initial restore run, for everything already loaded
  for ( int i = 0; i < d->model->rowCount() && !d->pendingProperties.isEmpty(); ++i ) {
    const QModelIndex index = d->model->index( i, 0 );
    d->restoreState( index );
  }

  // watch the model for stuff coming in delayed
  if ( !d->pendingProperties.isEmpty() )
    connect( d->model, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(rowsInserted(QModelIndex,int,int)), Qt::QueuedConnection );
}

void ModelStateSaver::saveConfig( KConfigGroup &configGroup )
{
  configGroup.deleteGroup();
  typedef QPair<QString, QVariant> StringVariantPair;
  typedef QHash<QByteArray, QVector<StringVariantPair> > ValueHash;
  ValueHash values;

  const int rowCount = d->model->rowCount();
  for ( int i = 0; i < rowCount; ++i ) {
    const QModelIndex index = d->model->index( i, 0 );
    d->saveState( index, values );
  }

  ValueHash::ConstIterator it = values.constBegin();
  while ( it != values.constEnd() ) {
    QStringList l;
    Q_FOREACH( const StringVariantPair &pair, it.value() ) {
      l.push_back( pair.first );
      l.push_back( pair.second.toString() );
    }
    configGroup.writeEntry( ( QByteArray("Role_") + it.key() ).constData(), l );
    ++it;
  }
}

void ModelStateSaver::addRole( int role, const QByteArray &identifier, const QVariant &defaultValue )
{
  Role r;
  r.role = role;
  r.identifier = identifier;
  r.defaultValue = defaultValue;
  d->roles.insert( identifier, r );
}

EntityModelStateSaver::EntityModelStateSaver( QAbstractItemModel* model, QObject* parent ) : ModelStateSaver( model, parent ), d( 0 ) {
}

EntityModelStateSaver::~EntityModelStateSaver() {
}


QString EntityModelStateSaver::key( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return QLatin1String( "x-1" );
  const Collection c = index.data( EntityTreeModel::CollectionRole ).value<Collection>();
  if ( c.isValid() )
    return QString::fromLatin1( "c%1" ).arg( c.id() );
  return QString::fromLatin1( "i%1" ).arg( index.data( EntityTreeModel::ItemIdRole ).value<Entity::Id>() );
}

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


class CollectionProxyModel : public QSortFilterProxyModel
{
  public:
    explicit CollectionProxyModel( QObject *parent=0 ) : QSortFilterProxyModel(parent), mCollectionSelection(0) {}
    ~CollectionProxyModel() {}

    /* reimp */ Qt::ItemFlags flags(const QModelIndex &index) const
    {
      if ( !index.isValid() )
        return QSortFilterProxyModel::flags( index );
      const Akonadi::Collection collection = Akonadi::collectionFromIndex( index );
      if ( collection.contentMimeTypes().isEmpty() )
        return QSortFilterProxyModel::flags( index ) | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
      return QSortFilterProxyModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }

    /* reimp */ QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
      if ( !index.isValid() )
        return QVariant();
      if ( !mCollectionSelection )
        return QVariant();
      switch(role) {
        case Qt::DecorationRole: {
            const Akonadi::Collection collection = Akonadi::collectionFromIndex( index );
            if ( collection.contentMimeTypes().isEmpty() )
              return QSortFilterProxyModel::data( index, role ); 
            return KOHelper::resourceColor( collection );
        }
        case Qt::CheckStateRole: {
          if ( index.column() != CalendarModel::CollectionTitle )
            return QVariant();
          const Akonadi::Collection collection = Akonadi::collectionFromIndex( index );
          if ( collection.contentMimeTypes().isEmpty() )
            return QVariant();
          return mCollectionSelection->model()->isSelected( index ) ? Qt::Checked : Qt::Unchecked;
        } break;
        default:
          return QSortFilterProxyModel::data(index, role);
      }
    }

    /* reimp */ bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)
    {
        switch(role) {
        case Qt::CheckStateRole: {
          Q_ASSERT( index.isValid() );
          if ( index.column() != CalendarModel::CollectionTitle )
            return false;
          const Akonadi::Collection collection = Akonadi::collectionFromIndex( index );
          Q_ASSERT( collection.isValid() );
          //const bool checked = value.toBool();
          const bool checked = value.toInt() == Qt::Checked;
          mCollectionSelection->model()->select( index, checked ? QItemSelectionModel::Select : QItemSelectionModel::Deselect );
        } return true;
        default:
          return QSortFilterProxyModel::setData(index, value, role);
      }
    }

    /* reimp */ bool filterAcceptsColumn( int source_column, const QModelIndex& source_parent ) const
    {
      return source_column == CalendarModel::CollectionTitle;
    }

    CollectionSelection* mCollectionSelection;
};


AkonadiCollectionView::AkonadiCollectionView( AkonadiCollectionViewFactory *factory, CalendarModel* calendarModel, QWidget *parent )
  : CalendarViewExtension( parent ), mActionManager(0), mCollectionview(0), mCollectionSelection(0)
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  Akonadi::CollectionFilterProxyModel *collectionproxymodel = new Akonadi::CollectionFilterProxyModel( mCollectionSelection );
  collectionproxymodel->setSourceModel( calendarModel );
  collectionproxymodel->addMimeTypeFilter( QString::fromLatin1( "text/calendar" ) );

  mProxyModel = new CollectionProxyModel( this );
  mProxyModel->setDynamicSortFilter( true );
  mProxyModel->setSortCaseSensitivity( Qt::CaseInsensitive );
  mProxyModel->setSourceModel( collectionproxymodel );
  mStateSaver = new EntityModelStateSaver( mProxyModel, this );
  mStateSaver->addRole( Qt::CheckStateRole, "CheckState", Qt::Unchecked );
  mCollectionSelection = new CollectionSelection( new QItemSelectionModel( mProxyModel ) );
  mProxyModel->mCollectionSelection = mCollectionSelection;

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
    mActionManager->action( Akonadi::StandardActionManager::CreateCollection )->setText( i18n( "Add Calendr..." ) );
    mActionManager->setActionText( Akonadi::StandardActionManager::CopyCollections, ki18np( "Copy Calendar", "Copy %1 Calendars" ) );
    mActionManager->action( Akonadi::StandardActionManager::DeleteCollections )->setText( i18n( "Delete Calendar" ) );
    mActionManager->action( Akonadi::StandardActionManager::SynchronizeCollections )->setText( i18n( "Reload" ) );
    mActionManager->action( Akonadi::StandardActionManager::CollectionProperties )->setText( i18n( "Properties..." ) );
    mActionManager->setCollectionSelectionModel( mCollectionview->selectionModel() );

    mCreateAction = new KAction( mCollectionview );
    mCreateAction->setIcon( KIcon( "appointment-new" ) );
    mCreateAction->setText( i18n( "New Calendr..." ) );
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
  connect( mCollectionSelection, SIGNAL(selectionChanged(Akonadi::Collection::List,Akonadi::Collection::List)), this, SLOT(selectionChanged()) );
  
  updateView();
}

void AkonadiCollectionView::restoreConfig( const KConfigGroup &configGroup )
{
  mStateSaver->restoreConfig( configGroup );
}

void AkonadiCollectionView::saveConfig( KConfigGroup &configGroup )
{
  mStateSaver->saveConfig( configGroup );
}

AkonadiCollectionView::~AkonadiCollectionView()
{
}

CollectionSelection* AkonadiCollectionView::collectionSelection() const
{
  return mCollectionSelection;
}

Akonadi::EntityTreeView* AkonadiCollectionView::view() const
{
  return mCollectionview;
}

void AkonadiCollectionView::updateView()
{
  emit resourcesChanged( mCollectionSelection->hasSelection() );
}

void AkonadiCollectionView::selectionChanged()
{
  kDebug();
  mDeleteAction->setEnabled( mCollectionview->selectionModel()->hasSelection() );
  updateView();
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
    //TODO(AKONADI_PORT) this should show an error dialog and should be merged with the identical code in ActionManager
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

#include "akonadicollectionview.moc" // for EntityModelStateSaver Q_PRIVATE_SLOT
