/*
  This file is part of KOrganizer.

  Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2009 Sebastian Sauer <sebsauer@kdab.net>
  Copyright (C) 2010 Laurent Montel <montel@kde.org>

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
#include <kcolordialog.h>
#include "kocore.h"
#include "kohelper.h"
#include "koprefs.h"
#include <KLineEdit>

#include <KDebug>
#include <KDialog>
#include <KAction>
#include <KActionCollection>
#include <kjob.h>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>

#include <akonadi/kcal/calendarmodel.h>
#include <akonadi/akonadi_next/collectionselectionproxymodel.h>
#include <akonadi/akonadi_next/entitymodelstatesaver.h>
#include <akonadi/kcal/collectionselection.h>
#include <akonadi/kcal/kcalprefs.h>
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
#include <akonadi/agentmanager.h>
#include <akonadi/agentinstance.h>
#include <akonadi/akonadi_next/krecursivefilterproxymodel.h>

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
    explicit ColorProxyModel( QObject* parent=0 )
      : QSortFilterProxyModel( parent ),
        mInitDefaultCalendar( false )
      {
      }

    /* reimp */ QVariant data( const QModelIndex &index, int role ) const
    {
        if ( !index.isValid() )
            return QVariant();
        if ( role == Qt::DecorationRole ) {
          const Akonadi::Collection collection = Akonadi::collectionFromIndex( index );
          if ( !collection.contentMimeTypes().isEmpty() ) {
            QColor col = KOHelper::resourceColor( collection );
            if ( col.isValid() )
              return col;
            else
              return QVariant();
          }
        } else if ( role == Qt::FontRole ) {
          const Akonadi::Collection collection = Akonadi::collectionFromIndex( index );
          if ( !collection.contentMimeTypes().isEmpty() && KOHelper::isStandardCalendar( collection.id() ) ) {
            QFont font = qvariant_cast<QFont>( QSortFilterProxyModel::data( index, Qt::FontRole ) );
            font.setBold( true );
            if ( !mInitDefaultCalendar ) {
              mInitDefaultCalendar = true;
              KCalPrefs::instance()->setDefaultCalendarId( collection.id() );
            }
            return font;
          }
        }
       return QSortFilterProxyModel::data( index, role );
     }
  private:
    mutable bool mInitDefaultCalendar;
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
  : CalendarViewExtension( parent ),
    mActionManager(0),
    mCollectionview(0),
    mBaseModel( 0 ),
    mSelectionProxyModel( 0 ),
    mDeleteAction( 0 ),
    mNotSendAddRemoveSignal( false ),
    mWasDefaultCalendar( false ),
    mInitDefaultCalendar( false )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  KLineEdit *searchCol = new KLineEdit( this );
  searchCol->setClearButtonShown( true );
  searchCol->setClickMessage( i18nc( "@info/plain Displayed grayed-out inside the "
                                                   "textbox, verb to search", "Search" ) );
  topLayout->addWidget( searchCol );


  Akonadi::CollectionFilterProxyModel *collectionproxymodel = new Akonadi::CollectionFilterProxyModel( this );
  collectionproxymodel->setDynamicSortFilter( true );
  collectionproxymodel->addMimeTypeFilter( QString::fromLatin1( "text/calendar" ) );
  //collectionproxymodel->addExcludedSpecialResources(Akonadi::Collection::SearchResource);

  ColorProxyModel* colorProxy = new ColorProxyModel( this );
  colorProxy->setDynamicSortFilter( true );
  colorProxy->setSourceModel( collectionproxymodel );
  mBaseModel = collectionproxymodel;

  mCollectionview = new Akonadi::EntityTreeView( this );
  topLayout->addWidget( mCollectionview );
  mCollectionview->header()->hide();
  mCollectionview->setRootIsDecorated( true );

  //Filter tree view.
  Future::KRecursiveFilterProxyModel* filterTreeViewModel = new Future::KRecursiveFilterProxyModel( this );
  filterTreeViewModel->setDynamicSortFilter( true );
  filterTreeViewModel->setSourceModel( colorProxy );
  filterTreeViewModel->setFilterCaseSensitivity( Qt::CaseInsensitive );
  mCollectionview->setModel( filterTreeViewModel );

  connect( searchCol, SIGNAL( textChanged(QString) ), filterTreeViewModel, SLOT( setFilterFixedString(QString) ) );

  connect( mCollectionview->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
           this, SLOT(selectionChanged()) );

  connect( mBaseModel, SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
           this, SLOT( rowsInserted( const QModelIndex&, int, int ) ) );

  //mCollectionview->setSelectionMode( QAbstractItemView::NoSelection );
  KXMLGUIClient *xmlclient = KOCore::self()->xmlguiClient( view );
  if( xmlclient ) {
    mCollectionview->setXmlGuiClient( xmlclient );

    mActionManager = new Akonadi::StandardActionManager( xmlclient->actionCollection(), mCollectionview );
    //Laurent: don't create all actions not necessary for korganizer and create action with shortcut which conflicts bug #229332
    //mActionManager->createAllActions();
    mActionManager->createAction( Akonadi::StandardActionManager::CreateCollection )->setText( i18n( "Add Calendar..." ) );
    //mActionManager->setActionText( Akonadi::StandardActionManager::CopyCollections, ki18np( "Copy Calendar", "Copy %1 Calendars" ) );
    mActionManager->createAction( Akonadi::StandardActionManager::DeleteCollections )->setText( i18n( "Delete Calendar" ) );
    mActionManager->createAction( Akonadi::StandardActionManager::SynchronizeCollections )->setText( i18n( "Reload" ) );
    mActionManager->createAction( Akonadi::StandardActionManager::CollectionProperties )->setText( i18n( "Properties..." ) );
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

    mDisableColor = new KAction( mCollectionview );
    mDisableColor->setText( "&Disable Color");
    mDisableColor->setEnabled( false );
    xmlclient->actionCollection()->addAction( QString::fromLatin1( "disable_color" ), mDisableColor );
    connect( mDisableColor, SIGNAL( triggered( bool ) ), this, SLOT(disableColor() ) );

    mAssignColor = new KAction( mCollectionview );
    mAssignColor->setText( i18n( "&Assign Color..." ) );
    mAssignColor->setEnabled( false );
    xmlclient->actionCollection()->addAction( QString::fromLatin1( "assign_color" ), mAssignColor );
    connect( mAssignColor, SIGNAL( triggered( bool ) ), this, SLOT(assignColor()) );

    mEditAction = new KAction( mCollectionview );
    mEditAction->setText( i18n( "&Edit..." ) );
    mEditAction->setEnabled( false );
    xmlclient->actionCollection()->addAction( QString::fromLatin1( "edit_calendar" ),mEditAction );
    connect( mEditAction, SIGNAL( triggered( bool ) ), this, SLOT( editCalendar()) );

    mDefaultCalendar = new KAction( mCollectionview );
    mDefaultCalendar->setText( i18n( "Use as &Default Calendar" ) );
    mDefaultCalendar->setEnabled( false );
    xmlclient->actionCollection()->addAction( QString::fromLatin1( "set_standard_calendar" ),mDefaultCalendar );
    connect( mDefaultCalendar, SIGNAL( triggered( bool ) ), this, SLOT( setDefaultCalendar()) );

  }
}

AkonadiCollectionView::~AkonadiCollectionView()
{
}

void AkonadiCollectionView::setDefaultCalendar()
{
  QModelIndex index = mCollectionview->selectionModel()->currentIndex(); //selectedRows()
  Q_ASSERT( index.isValid() );
  const Akonadi::Collection collection = collectionFromIndex( index );
  KCalPrefs::instance()->setDefaultCalendarId( collection.id() );
  KCalPrefs::instance()->usrWriteConfig();
  updateMenu();
  updateView();

  emit defaultResourceChanged( collection );
}

void AkonadiCollectionView::editCalendar()
{
  QModelIndex index = mCollectionview->selectionModel()->currentIndex(); //selectedRows()
  Q_ASSERT( index.isValid() );
  const Akonadi::Collection collection = collectionFromIndex( index );
  Q_ASSERT( collection.isValid() );
  const QString resource = collection.resource();
  Akonadi::AgentInstance instance = Akonadi::AgentManager::self()->instance( resource );
  if ( instance.isValid() )
    instance.configure( this );
}

void AkonadiCollectionView::assignColor()
{
  QModelIndex index = mCollectionview->selectionModel()->currentIndex(); //selectedRows()
  Q_ASSERT( index.isValid() );
  const Akonadi::Collection collection = collectionFromIndex( index );
  Q_ASSERT( collection.isValid() );

  const QString identifier = QString::number( collection.id() );
  const QColor defaultColor = KOPrefs::instance()->resourceColor( identifier );
  QColor myColor;
  int result = KColorDialog::getColor( myColor, defaultColor );
  if ( result == KColorDialog::Accepted ) {
    KOPrefs::instance()->setResourceColor( identifier, myColor );
    updateMenu();
    updateView();
  }
}

void AkonadiCollectionView::disableColor()
{
  QModelIndex index = mCollectionview->selectionModel()->currentIndex(); //selectedRows()
  Q_ASSERT( index.isValid() );
  const Akonadi::Collection collection = collectionFromIndex( index );
  Q_ASSERT( collection.isValid() );
  const QString identifier = QString::number( collection.id() );
  KOPrefs::instance()->setResourceColor( identifier, QColor() );
  updateMenu();
  updateView();
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

CollectionSelectionProxyModel *AkonadiCollectionView::collectionSelectionProxyModel() const
{
  return mSelectionProxyModel;
}

Akonadi::EntityTreeView* AkonadiCollectionView::view() const
{
  return mCollectionview;
}

void AkonadiCollectionView::updateView()
{
  emit resourcesChanged( mSelectionProxyModel ? mSelectionProxyModel->selectionModel()->hasSelection() : false );
}

void AkonadiCollectionView::updateMenu()
{
  if ( mDeleteAction ) {
    bool enableAction = mCollectionview->selectionModel()->hasSelection();
    mDeleteAction->setEnabled( enableAction );
    enableAction = enableAction && ( KOPrefs::instance()->agendaViewColors() != KOPrefs::CategoryOnly );
    mAssignColor->setEnabled( enableAction );
    QModelIndex index = mCollectionview->selectionModel()->currentIndex(); //selectedRows()
    if ( index.isValid() ) {
      const Akonadi::Collection collection = collectionFromIndex( index );
      Q_ASSERT( collection.isValid() );

      const QString identifier = QString::number( collection.id() );
      const QColor defaultColor = KOPrefs::instance()->resourceColor( identifier );
      enableAction = enableAction && defaultColor.isValid();
      mDisableColor->setEnabled( enableAction );
      const QString resource = collection.resource();
      Akonadi::AgentInstance instance = Akonadi::AgentManager::self()->instance( resource );
      mEditAction->setEnabled( !instance.type().capabilities().contains( QLatin1String( "NoConfig" ) ) );
      mDefaultCalendar->setEnabled( !KOHelper::isStandardCalendar( collection.id() ) );
    } else {
      mDisableColor->setEnabled( false );
      mEditAction->setEnabled( false );
      mDefaultCalendar->setEnabled( false );
    }
  }
}

void AkonadiCollectionView::selectionChanged()
{
  kDebug();
  updateMenu();
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
    kWarning() << "Create calendar failed:" << createjob->errorString();
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


  const QString displayname = index.model()->data( index, Qt::DisplayRole ).toString();
  Q_ASSERT( !displayname.isEmpty() );

  if ( KMessageBox::questionYesNo( this,
                                   i18n( "Do you really want to delete calendar %1?", displayname ),
                                   i18n( "Delete Calendar" ),
                                   KStandardGuiItem::del(),
                                   KStandardGuiItem::cancel(),
                                   QString(),
                                   KMessageBox::Dangerous )
       == KMessageBox::Yes ) {

    bool isTopLevel = collection.parentCollection() == Collection::root();

    mNotSendAddRemoveSignal = true;
    mWasDefaultCalendar = KOHelper::isStandardCalendar( collection.id() );

    if ( !isTopLevel ) {
      // deletes contents
      Akonadi::CollectionDeleteJob *job = new Akonadi::CollectionDeleteJob( collection, this );
      connect( job, SIGNAL( result( KJob* ) ), this, SLOT( deleteCalendarDone( KJob* ) ) );
    } else {
      // deletes the agent, not the contents
      const AgentInstance instance = Akonadi::AgentManager::self()->instance( collection.resource() );
      if ( instance.isValid() ) {
        Akonadi::AgentManager::self()->removeInstance( instance );
      }
    }
  }
}

void AkonadiCollectionView::deleteCalendarDone( KJob *job )
{
  kDebug();
  Akonadi::CollectionDeleteJob *deletejob = static_cast<Akonadi::CollectionDeleteJob*>( job );
  if ( deletejob->error() ) {
    kWarning() << "Delete calendar failed:" << deletejob->errorString();
    mNotSendAddRemoveSignal = false;
    return;
  }
  if ( mWasDefaultCalendar ) {
    KCalPrefs::instance()->setDefaultCalendarId( Akonadi::Collection().id() );
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
