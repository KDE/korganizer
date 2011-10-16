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

#include <calendarsupport/calendarmodel.h>
#include <calendarsupport/collectionselection.h>
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

#include <kcheckableproxymodel.h>

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
#include <akonadi/agentinstancecreatejob.h>
#include <akonadi/agentfilterproxymodel.h>
#include <akonadi/control.h>
#include <akonadi/session.h>
#include <akonadi/changerecorder.h>
#include <akonadi/agentmanager.h>
#include <akonadi/agentinstance.h>

#include <QHash>
#include <QStyledItemDelegate>
#include <QPainter>

AkonadiCollectionViewFactory::AkonadiCollectionViewFactory( CalendarView *view )
  : mView( view ), mAkonadiCollectionView( 0 )
{
}

namespace {

class ColorDelegate : public QStyledItemDelegate
{
  public:
    explicit ColorDelegate( QObject * parent = 0 ) : QStyledItemDelegate( parent )
    {
    }

    void paint ( QPainter *painter, const QStyleOptionViewItem &option,
                 const QModelIndex &index ) const
    {
      QStyledItemDelegate::paint( painter, option, index );
      QStyleOptionViewItemV4 v4 = option;
      initStyleOption( &v4, index );
      if ( v4.checkState == Qt::Checked ) {
        const Akonadi::Collection collection = CalendarSupport::collectionFromIndex( index );
        QColor color = KOHelper::resourceColor( collection );
        const bool collectionHasIcon = index.data( Qt::DecorationRole ).canConvert<QIcon>();
        if ( color.isValid() && collectionHasIcon ) {
          QRect r = v4.rect;
          const int h = r.height() - 4;
          r.adjust( r.width() - h - 2, 2, - 2, -2 );
          painter->save();
          painter->setRenderHint( QPainter::Antialiasing );
          QPen pen = painter->pen();
          pen.setColor( color );
          QPainterPath path;
          path.addRoundedRect( r, 5, 5 );
          color.setAlpha( 200 );
          painter->fillPath( path, color );
          painter->strokePath( path, pen );
          painter->restore();
        }
      }
    }
};

class ColorProxyModel : public QSortFilterProxyModel
{
  public:
    explicit ColorProxyModel( QObject *parent=0 )
      : QSortFilterProxyModel( parent ), mInitDefaultCalendar( false )
    {
    }

    /* reimp */
    QVariant data( const QModelIndex &index, int role ) const
    {
      if ( !index.isValid() ) {
        return QVariant();
      }
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
        if ( !collection.contentMimeTypes().isEmpty() &&
             KOHelper::isStandardCalendar( collection.id() ) &&
             collection.rights() & Akonadi::Collection::CanCreateItem ) {
          QFont font = qvariant_cast<QFont>( QSortFilterProxyModel::data( index, Qt::FontRole ) );
          font.setBold( true );
          if ( !mInitDefaultCalendar ) {
            mInitDefaultCalendar = true;
            CalendarSupport::KCalPrefs::instance()->setDefaultCalendarId( collection.id() );
          }
          return font;
        }
      } else if ( role == Qt::CheckStateRole ) {
        // Don't show the checkbox if the collection can't contain incidences
        const Akonadi::Collection collection = CalendarSupport::collectionFromIndex( index );
        if ( AkonadiCollectionView::isStructuralCollection( collection ) ) {
          return QVariant();
        }
      }
      return QSortFilterProxyModel::data( index, role );
    }

    /* reimp */
    Qt::ItemFlags flags( const QModelIndex &index ) const
    {
      return Qt::ItemIsSelectable | QSortFilterProxyModel::flags( index );
    }

  private:
    mutable bool mInitDefaultCalendar;
};

} // anonymous namespace

CalendarViewExtension *AkonadiCollectionViewFactory::create( QWidget *parent )
{
  mAkonadiCollectionView = new AkonadiCollectionView( view(), true, parent );
  QObject::connect( mAkonadiCollectionView, SIGNAL(resourcesChanged(bool)),
                    mView, SLOT(resourcesChanged()) );
  QObject::connect( mAkonadiCollectionView, SIGNAL(resourcesChanged(bool)),
                    mView, SLOT(updateCategories()) );
  QObject::connect( mAkonadiCollectionView, SIGNAL(resourcesAddedRemoved()),
                    mView, SLOT(resourcesChanged()) );
  QObject::connect( mAkonadiCollectionView, SIGNAL(resourcesAddedRemoved()),
                    mView, SLOT(updateCategories()) );
  return mAkonadiCollectionView;
}

CalendarView *AkonadiCollectionViewFactory::view() const
{
  return mView;
}

AkonadiCollectionView *AkonadiCollectionViewFactory::collectionView() const
{
  return mAkonadiCollectionView;
}

AkonadiCollectionView::AkonadiCollectionView( CalendarView *view, bool hasContextMenu,
                                              QWidget *parent )
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
  topLayout->setMargin( 0 );
  topLayout->setSpacing( KDialog::spacingHint() );

  //KLineEdit *searchCol = new KLineEdit( this );
  //searchCol->setClearButtonShown( true );
  //searchCol->setClickMessage( i18nc( "@info/plain Displayed grayed-out inside the "
  //                                                   "textbox, verb to search", "Search" ) );
  //topLayout->addWidget( searchCol );

  Akonadi::CollectionFilterProxyModel *collectionproxymodel =
    new Akonadi::CollectionFilterProxyModel( this );
  collectionproxymodel->setObjectName( "Only show collections" );
  collectionproxymodel->setDynamicSortFilter( true );
  collectionproxymodel->addMimeTypeFilter( QString::fromLatin1( "text/calendar" ) );
  collectionproxymodel->setExcludeVirtualCollections( true );

  ColorProxyModel *colorProxy = new ColorProxyModel( this );
  colorProxy->setObjectName( "Show calendar colors" );
  colorProxy->setDynamicSortFilter( true );
  colorProxy->setSourceModel( collectionproxymodel );
  mBaseModel = collectionproxymodel;

  mCollectionview = new Akonadi::EntityTreeView( this );
  topLayout->addWidget( mCollectionview );
  mCollectionview->header()->hide();
  mCollectionview->setRootIsDecorated( true );
  mCollectionview->setItemDelegate( new ColorDelegate( this ) );

  //Filter tree view.
  KRecursiveFilterProxyModel *filterTreeViewModel = new KRecursiveFilterProxyModel( this );
  filterTreeViewModel->setDynamicSortFilter( true );
  filterTreeViewModel->setSourceModel( colorProxy );
  filterTreeViewModel->setFilterCaseSensitivity( Qt::CaseInsensitive );
  filterTreeViewModel->setObjectName( "Recursive filtering, for the search bar" );
  mCollectionview->setModel( filterTreeViewModel );

  //connect( searchCol, SIGNAL(textChanged(QString)),
  //         filterTreeViewModel, SLOT(setFilterFixedString(QString)) );

  connect( mBaseModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
           this, SLOT(rowsInserted(QModelIndex,int,int)) );

  //mCollectionview->setSelectionMode( QAbstractItemView::NoSelection );
  KXMLGUIClient *xmlclient = KOCore::self()->xmlguiClient( view );
  if ( xmlclient ) {
    mCollectionview->setXmlGuiClient( xmlclient );

    mActionManager =
      new Akonadi::StandardCalendarActionManager( xmlclient->actionCollection(), mCollectionview );
    mActionManager->createAllActions();
    mActionManager->setCollectionSelectionModel( mCollectionview->selectionModel() );

    mActionManager->interceptAction( Akonadi::StandardActionManager::CreateResource );
    mActionManager->interceptAction( Akonadi::StandardActionManager::DeleteResources );
    mActionManager->interceptAction( Akonadi::StandardActionManager::DeleteCollections );

    connect( mActionManager->action( Akonadi::StandardActionManager::CreateResource ),
             SIGNAL(triggered(bool)),
             this, SLOT(newCalendar()) );
    connect( mActionManager->action( Akonadi::StandardActionManager::DeleteResources ),
             SIGNAL(triggered(bool)),
             this, SLOT(deleteCalendar()) );
    connect( mActionManager->action( Akonadi::StandardActionManager::DeleteCollections ),
             SIGNAL(triggered(bool)),
             this, SLOT(deleteCalendar()) );

    mActionManager->setContextText( Akonadi::StandardActionManager::CollectionProperties,
                                    Akonadi::StandardActionManager::DialogTitle,
                                    i18nc( "@title:window", "Properties of Calendar Folder %1" ) );

    mActionManager->action( Akonadi::StandardActionManager::CreateCollection )->
      setProperty( "ContentMimeTypes", QStringList( KCalCore::Event::eventMimeType() ) );

    const QStringList pages =
      QStringList() << QLatin1String( "CalendarSupport::CollectionGeneralPage" )
                    << QLatin1String( "Akonadi::CachePolicyPage" );

    mActionManager->setCollectionPropertiesPageNames( pages );

    mDisableColor = new KAction( mCollectionview );
    mDisableColor->setText( i18n( "&Disable Color" ) );
    mDisableColor->setEnabled( false );
    xmlclient->actionCollection()->addAction( QString::fromLatin1( "disable_color" ),
                                              mDisableColor );
    connect( mDisableColor, SIGNAL(triggered(bool)), this, SLOT(disableColor()) );

    mAssignColor = new KAction( mCollectionview );
    mAssignColor->setText( i18n( "&Assign Color..." ) );
    mAssignColor->setEnabled( false );
    xmlclient->actionCollection()->addAction( QString::fromLatin1( "assign_color" ), mAssignColor );
    connect( mAssignColor, SIGNAL(triggered(bool)), this, SLOT(assignColor()) );

    mDefaultCalendar = new KAction( mCollectionview );
    mDefaultCalendar->setText( i18n( "Use as &Default Calendar" ) );
    mDefaultCalendar->setEnabled( false );
    xmlclient->actionCollection()->addAction( QString::fromLatin1( "set_standard_calendar" ),
                                              mDefaultCalendar );
    connect( mDefaultCalendar, SIGNAL(triggered(bool)), this, SLOT(setDefaultCalendar()) );
  }
  mCollectionview->expandAll();
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

void AkonadiCollectionView::setCollectionSelectionProxyModel( KCheckableProxyModel *m )
{
  if ( mSelectionProxyModel == m ) {
    return;
  }

  mSelectionProxyModel = m;
  if ( !mSelectionProxyModel ) {
    return;
  }

  mBaseModel->setSourceModel( mSelectionProxyModel );
  connect( mSelectionProxyModel->selectionModel(),
           SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
           this,
           SLOT(selectionChanged()) );
}

KCheckableProxyModel *AkonadiCollectionView::collectionSelectionProxyModel() const
{
  return mSelectionProxyModel;
}

Akonadi::EntityTreeView *AkonadiCollectionView::view() const
{
  return mCollectionview;
}

void AkonadiCollectionView::updateView()
{
  emit resourcesChanged( mSelectionProxyModel ?
                           mSelectionProxyModel->selectionModel()->hasSelection() :
                           false );
}

void AkonadiCollectionView::updateMenu()
{
  if ( !mHasContextMenu ) {
    return;
  }
  bool enableAction = mCollectionview->selectionModel()->hasSelection();
  enableAction = enableAction &&
                 ( KOPrefs::instance()->agendaViewColors() != KOPrefs::CategoryOnly );
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
      connect( job, SIGNAL(result(KJob*)), this, SLOT(newCalendarDone(KJob*)) );
      job->start();
    }
  }
}

void AkonadiCollectionView::newCalendarDone( KJob *job )
{
  Akonadi::AgentInstanceCreateJob *createjob = static_cast<Akonadi::AgentInstanceCreateJob*>( job );
  if ( createjob->error() ) {
    //TODO(AKONADI_PORT)
    // this should show an error dialog and should be merged
    // with the identical code in ActionManager
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

  if ( KMessageBox::warningContinueCancel(
         this,
         i18n( "Do you really want to delete calendar %1?", displayname ),
         i18n( "Delete Calendar" ),
         KStandardGuiItem::del(),
         KStandardGuiItem::cancel(),
         QString(),
         KMessageBox::Dangerous )  == KMessageBox::Continue ) {

    bool isTopLevel = collection.parentCollection() == Akonadi::Collection::root();

    mNotSendAddRemoveSignal = true;
    mWasDefaultCalendar = KOHelper::isStandardCalendar( collection.id() );

    if ( !isTopLevel ) {
      // deletes contents
      Akonadi::CollectionDeleteJob *job = new Akonadi::CollectionDeleteJob( collection, this );
      connect( job, SIGNAL(result(KJob*)), this, SLOT(deleteCalendarDone(KJob*)) );
    } else {
      // deletes the agent, not the contents
      const Akonadi::AgentInstance instance =
        Akonadi::AgentManager::self()->instance( collection.resource() );
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

void AkonadiCollectionView::rowsInserted( const QModelIndex &, int, int )
{
  if ( !mNotSendAddRemoveSignal ) {
    emit resourcesAddedRemoved();
  }
  mCollectionview->expandAll();
}

/** static */
bool AkonadiCollectionView::isStructuralCollection( const Akonadi::Collection &collection )
{
  QStringList mimeTypes;
  mimeTypes << QLatin1String( "text/calendar" ) << KCalCore::Event::eventMimeType()
            << Todo::todoMimeType() << Journal::journalMimeType();
  const QStringList collectionMimeTypes = collection.contentMimeTypes();
  foreach ( const QString &mimeType, mimeTypes ) {
    if ( collectionMimeTypes.contains( mimeType ) ) {
      return false;
    }
  }
  return true;
}

#include "akonadicollectionview.moc" // for EntityModelStateSaver Q_PRIVATE_SLOT
