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
#include "kocollectionpropertiesdialog.h"

#include <calendarsupport/calendarmodel.h>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/entitymodelstatesaver.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <KLineEdit>
#include <KDebug>
#include <KDialog>
#include <KAction>
#include <KActionCollection>
#include <krecursivefilterproxymodel.h>
#include <kjob.h>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QItemSelectionModel>

#include <akonadi_next/kcheckableproxymodel.h>

#include <akonadi/calendar/standardcalendaractionmanager.h>
#include <akonadi/collection.h>
#include <akonadi/collectionview.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectiondeletejob.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/entitytreeview.h>
#include <akonadi/entitydisplayattribute.h>
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

#include <QHash>

using namespace Future;

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
          const Akonadi::Collection collection = CalendarSupport::collectionFromIndex( index );

          if ( !collection.contentMimeTypes().isEmpty() ) {
            if ( collection.hasAttribute<Akonadi::EntityDisplayAttribute>() &&
                 !collection.attribute<Akonadi::EntityDisplayAttribute>()->iconName().isEmpty() ) {
              return collection.attribute<Akonadi::EntityDisplayAttribute>()->icon();
            } else {
              QColor col = KOHelper::resourceColor( collection );
              return col.isValid() ? col : QVariant();
            }
          }
        } else if ( role == Qt::FontRole ) {
          const Akonadi::Collection collection = CalendarSupport::collectionFromIndex( index );
          if ( !collection.contentMimeTypes().isEmpty() && KOHelper::isStandardCalendar( collection.id() ) &&
               collection.rights() & Akonadi::Collection::CanCreateItem) {
            QFont font = qvariant_cast<QFont>( QSortFilterProxyModel::data( index, Qt::FontRole ) );
            font.setBold( true );
            if ( !mInitDefaultCalendar ) {
              mInitDefaultCalendar = true;
              CalendarSupport::KCalPrefs::instance()->setDefaultCalendarId( collection.id() );
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
  mAkonadiCollectionView = new AkonadiCollectionView( view(), true, parent );
  QObject::connect( mAkonadiCollectionView, SIGNAL(resourcesChanged(bool)),
                    mView, SLOT(resourcesChanged()) );
  QObject::connect( mAkonadiCollectionView, SIGNAL(resourcesChanged(bool)),
                    mView, SLOT(updateCategories()) );
  QObject::connect( mAkonadiCollectionView, SIGNAL( resourcesAddedRemoved() ),
                    mView, SLOT( resourcesChanged() ) );
  QObject::connect( mAkonadiCollectionView, SIGNAL( resourcesAddedRemoved() ),
                    mView, SLOT( updateCategories() ) );
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

AkonadiCollectionView::AkonadiCollectionView( CalendarView* view, bool hasContextMenu, QWidget *parent )
  : CalendarViewExtension( parent ),
    mActionManager(0),
    mCollectionview(0),
    mBaseModel( 0 ),
    mSelectionProxyModel( 0 ),
    mNotSendAddRemoveSignal( false ),
    mWasDefaultCalendar( false ),
    mInitDefaultCalendar( false ),
    mHasContextMenu( hasContextMenu )
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
  KRecursiveFilterProxyModel* filterTreeViewModel = new KRecursiveFilterProxyModel( this );
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
  if ( xmlclient ) {
    mCollectionview->setXmlGuiClient( xmlclient );

    mActionManager = new Akonadi::StandardCalendarActionManager( xmlclient->actionCollection(), mCollectionview );
    mActionManager->createAllActions();
    mActionManager->setCollectionSelectionModel( mCollectionview->selectionModel() );

    mActionManager->interceptAction( Akonadi::StandardActionManager::CreateResource );
    mActionManager->interceptAction( Akonadi::StandardActionManager::DeleteResources );
    mActionManager->interceptAction( Akonadi::StandardActionManager::DeleteCollections );

    connect( mActionManager->action( Akonadi::StandardActionManager::CreateResource ), SIGNAL( triggered( bool ) ),
             this, SLOT( newCalendar() ) );
    connect( mActionManager->action( Akonadi::StandardActionManager::DeleteResources ), SIGNAL( triggered( bool ) ),
             this, SLOT( deleteCalendar() ) );
    connect( mActionManager->action( Akonadi::StandardActionManager::DeleteCollections ), SIGNAL( triggered( bool ) ),
             this, SLOT( deleteCalendar() ) );

    mActionManager->interceptAction( Akonadi::StandardActionManager::CollectionProperties );
    connect( mActionManager->action( Akonadi::StandardActionManager::CollectionProperties ), SIGNAL( triggered( bool ) ), this, SLOT( slotCollectionProperties() ) );

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
  const Akonadi::Collection collection = CalendarSupport::collectionFromIndex( index );
  CalendarSupport::KCalPrefs::instance()->setDefaultCalendarId( collection.id() );
  CalendarSupport::KCalPrefs::instance()->usrWriteConfig();
  updateMenu();
  updateView();

  emit defaultResourceChanged( collection );
}

void AkonadiCollectionView::assignColor()
{
  QModelIndex index = mCollectionview->selectionModel()->currentIndex(); //selectedRows()
  Q_ASSERT( index.isValid() );
  const Akonadi::Collection collection = CalendarSupport::collectionFromIndex( index );
  Q_ASSERT( collection.isValid() );

  const QString identifier = QString::number( collection.id() );
  const QColor defaultColor = KOPrefs::instance()->resourceColor( identifier );
  QColor myColor;
  const int result = KColorDialog::getColor( myColor, defaultColor );
  if ( result == KColorDialog::Accepted && myColor != defaultColor ) {
    KOPrefs::instance()->setResourceColor( identifier, myColor );
    emit colorsChanged();
    updateMenu();
    updateView();
  }
}

void AkonadiCollectionView::disableColor()
{
  QModelIndex index = mCollectionview->selectionModel()->currentIndex(); //selectedRows()
  Q_ASSERT( index.isValid() );
  const Akonadi::Collection collection = CalendarSupport::collectionFromIndex( index );
  Q_ASSERT( collection.isValid() );
  const QString identifier = QString::number( collection.id() );
  KOPrefs::instance()->setResourceColor( identifier, QColor() );
  updateMenu();
  updateView();
  emit colorsChanged();
}

void AkonadiCollectionView::setCollectionSelectionProxyModel( KCheckableProxyModel* m )
{
  if ( mSelectionProxyModel == m )
    return;
  mSelectionProxyModel = m;
  if ( !mSelectionProxyModel )
    return;
  mBaseModel->setSourceModel( mSelectionProxyModel );
  connect( mSelectionProxyModel->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(selectionChanged()) );
}

KCheckableProxyModel *AkonadiCollectionView::collectionSelectionProxyModel() const
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
  if ( !mHasContextMenu ) {
    return;
  }
  bool enableAction = mCollectionview->selectionModel()->hasSelection();
  enableAction = enableAction && ( KOPrefs::instance()->agendaViewColors() != KOPrefs::CategoryOnly );
  mAssignColor->setEnabled( enableAction );
  QModelIndex index = mCollectionview->selectionModel()->currentIndex(); //selectedRows()

  bool disableStuff = false;

  if ( index.isValid() ) {
    const Akonadi::Collection collection = CalendarSupport::collectionFromIndex( index );
    Q_ASSERT( collection.isValid() );

    if ( !collection.contentMimeTypes().isEmpty() ) {
      const QString identifier = QString::number( collection.id() );
      const QColor defaultColor = KOPrefs::instance()->resourceColor( identifier );
      enableAction = enableAction && defaultColor.isValid();
      mDisableColor->setEnabled( enableAction );
      mDefaultCalendar->setEnabled( !KOHelper::isStandardCalendar( collection.id() ) &&
                                    collection.rights() & Akonadi::Collection::CanCreateItem );
    } else {
      disableStuff = true;
    }
  } else {
    disableStuff = true;
  }

  if ( disableStuff ) {
    mDisableColor->setEnabled( false );
    mDefaultCalendar->setEnabled( false );
    mAssignColor->setEnabled( false );
  }
}

void AkonadiCollectionView::selectionChanged()
{
  updateMenu();
  updateView();
}

void AkonadiCollectionView::newCalendar()
{
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

  QModelIndex index = mCollectionview->selectionModel()->currentIndex(); //selectedRows()
  Q_ASSERT( index.isValid() );
  const Akonadi::Collection collection = CalendarSupport::collectionFromIndex( index );
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

    bool isTopLevel = collection.parentCollection() == Akonadi::Collection::root();

    mNotSendAddRemoveSignal = true;
    mWasDefaultCalendar = KOHelper::isStandardCalendar( collection.id() );

    if ( !isTopLevel ) {
      // deletes contents
      Akonadi::CollectionDeleteJob *job = new Akonadi::CollectionDeleteJob( collection, this );
      connect( job, SIGNAL( result( KJob* ) ), this, SLOT( deleteCalendarDone( KJob* ) ) );
    } else {
      // deletes the agent, not the contents
      const Akonadi::AgentInstance instance = Akonadi::AgentManager::self()->instance( collection.resource() );
      if ( instance.isValid() ) {
        Akonadi::AgentManager::self()->removeInstance( instance );
      }
    }
  }
}

void AkonadiCollectionView::deleteCalendarDone( KJob *job )
{
  Akonadi::CollectionDeleteJob *deletejob = static_cast<Akonadi::CollectionDeleteJob*>( job );
  if ( deletejob->error() ) {
    kWarning() << "Delete calendar failed:" << deletejob->errorString();
    mNotSendAddRemoveSignal = false;
    return;
  }
  if ( mWasDefaultCalendar ) {
    CalendarSupport::KCalPrefs::instance()->setDefaultCalendarId( Akonadi::Collection().id() );
  }
  mNotSendAddRemoveSignal = false;
  //TODO
}

void AkonadiCollectionView::rowsInserted( const QModelIndex&, int, int )
{
  if ( !mNotSendAddRemoveSignal )
    emit resourcesAddedRemoved();
}


void AkonadiCollectionView::slotCollectionProperties()
{
  QModelIndex index = mCollectionview->selectionModel()->currentIndex(); //selectedRows()
  Q_ASSERT( index.isValid() );
  const Akonadi::Collection collection = CalendarSupport::collectionFromIndex( index );
  Q_ASSERT( collection.isValid() );

  KOCollectionPropertiesDialog* dlg = new KOCollectionPropertiesDialog( collection, this );

  dlg->setCaption(  i18nc( "@title:window", "Properties of Calendar Folder %1" , collection.name() ) );
  dlg->resize( 400, 500 );
  dlg->show();
}

#include "akonadicollectionview.moc" // for EntityModelStateSaver Q_PRIVATE_SLOT
