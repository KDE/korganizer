/*
  This file is part of KOrganizer.

  Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2009 Sebastian Sauer <sebsauer@kdab.net>
  Copyright (C) 2010 Laurent Montel <montel@kde.org>
  Copyright (C) 2012 Sérgio Martins <iamsergio@gmail.com>

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
#include "koglobals.h"
#include "views/collectionview/reparentingmodel.h"
#include "views/collectionview/calendardelegate.h"

#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <Akonadi/AgentFilterProxyModel>
#include <Akonadi/AgentInstanceCreateJob>
#include <Akonadi/AgentManager>
#include <Akonadi/AgentTypeDialog>
#include <Akonadi/CollectionDeleteJob>
#include <Akonadi/CollectionFilterProxyModel>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/EntityTreeView>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/ETMViewStateSaver>
#include <Akonadi/Calendar/StandardCalendarActionManager>
#include <akonadi/collectionidentificationattribute.h>

#include <KAction>
#include <KActionCollection>
#include <KCheckableProxyModel>
#include <KColorDialog>
#include <KMessageBox>
#include <KRecursiveFilterProxyModel>
#include <KLineEdit>

#include <QHeaderView>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QAction>

static Akonadi::EntityTreeModel *findEtm(QAbstractItemModel *model)
{
    QAbstractProxyModel *proxyModel;
    while (model) {
        proxyModel = qobject_cast<QAbstractProxyModel*>(model);
        if (proxyModel && proxyModel->sourceModel()) {
            model = proxyModel->sourceModel();
        } else {
            break;
        }
    }
    return qobject_cast<Akonadi::EntityTreeModel*>(model);
}

/**
* Automatically checks new calendar entries
*/
class NewCalendarChecker : public QObject {
    Q_OBJECT
public:
    NewCalendarChecker(QAbstractItemModel *model)
        :QObject(model),
        mCheckableProxy(model)
    {
        connect(model, SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(onSourceRowsInserted(QModelIndex, int, int)));
    }

private slots:
    void onSourceRowsInserted(const QModelIndex &parent, int start, int end)
    {
        Akonadi::EntityTreeModel *etm = findEtm(mCheckableProxy);
        //Only check new collections and not during initial population
        if (!etm || !etm->isCollectionTreeFetched()) {
            return;
        }
        for (int i = start; i <= end; i++) {
            kDebug() << "checking " << mCheckableProxy->index(i, 0, parent).data().toString();
            const QModelIndex index = mCheckableProxy->index(i, 0, parent);
            mCheckableProxy->setData(index, Qt::Checked, Qt::CheckStateRole);
            if (mCheckableProxy->hasChildren(index)) {
                onSourceRowsInserted(index, 0, mCheckableProxy->rowCount(index) - 1);
            }
        }
    }

private:
    QAbstractItemModel *mCheckableProxy;
};


/**
* Handles expansion state of a treeview
*
* Persists state, and automatically expands new entries.
* With expandAll enabled this class simply ensures that all indexes are fully expanded.
*/
class NewNodeExpander : public QObject {
    Q_OBJECT
public:
    NewNodeExpander(QTreeView *view, bool expandAll, const QString &treeStateConfig)
        :QObject(view),
        mTreeView(view),
        mExpandAll(expandAll),
        mTreeStateConfig(treeStateConfig)
    {
        connect(view->model(), SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(onSourceRowsInserted(QModelIndex, int, int)));
        connect(view->model(), SIGNAL(layoutChanged()), this, SLOT(onLayoutChanged()));
        connect(view->model(), SIGNAL(modelReset()), this, SLOT(onModelReset()));
        restoreTreeState();
    }

    virtual ~NewNodeExpander()
    {
        //Ideally we'd automatically save the treestate of the parent view here,
        //but that unfortunately doesn't seem to work
    }

public Q_SLOTS:
    void saveState()
    {
        saveTreeState();
    }

private Q_SLOTS:
    void onSourceRowsInserted(const QModelIndex &parent, int start, int end)
    {
        //The initial expansion is handled by the state saver
        if (!mExpandAll) {
            Akonadi::EntityTreeModel *etm = findEtm(mTreeView->model());
            if (!etm || !etm->isCollectionTreeFetched()) {
                restoreTreeState();
                return;
            }
        }
        for (int i = start; i <= end; i++) {
            const QModelIndex index = mTreeView->model()->index(i, 0, parent);
            // kDebug() << "expanding " << index.data().toString();
            mTreeView->expand(index);
            if (mTreeView->model()->hasChildren(index)) {
                onSourceRowsInserted(index, 0, mTreeView->model()->rowCount(index) - 1);
            }
        }
    }

    void onLayoutChanged()
    {
        if (mExpandAll) {
            onSourceRowsInserted(QModelIndex(), 0, mTreeView->model()->rowCount(QModelIndex()) - 1);
        }
    }

    void onModelReset()
    {
        if (mExpandAll) {
            onSourceRowsInserted(QModelIndex(), 0, mTreeView->model()->rowCount(QModelIndex()) - 1);
        }
    }

private:
    void saveTreeState()
    {
        Akonadi::ETMViewStateSaver treeStateSaver;
        KConfigGroup group(KOGlobals::self()->config(), mTreeStateConfig);
        treeStateSaver.setView(mTreeView);
        treeStateSaver.setSelectionModel(0); // we only save expand state
        treeStateSaver.saveState(group);
    }

    void restoreTreeState()
    {
        if (mTreeStateConfig.isEmpty()) {
            return;
        }
        //Otherwise ETMViewStateSaver crashes
        if (!findEtm(mTreeView->model())) {
            return;
        }
        if ( treeStateRestorer ) {// We don't need more than one to be running at the same time
            delete treeStateRestorer;
        }
        kDebug() << "Restore tree state";
        treeStateRestorer = new Akonadi::ETMViewStateSaver(); // not a leak
        KConfigGroup group( KOGlobals::self()->config(), mTreeStateConfig );
        treeStateRestorer->setView( mTreeView );
        treeStateRestorer->setSelectionModel( 0 ); // we only restore expand state
        treeStateRestorer->restoreState( group );
    }

    QPointer<Akonadi::ETMViewStateSaver> treeStateRestorer;
    QTreeView *mTreeView;
    bool mExpandAll;
    QString mTreeStateConfig;
};




AkonadiCollectionViewFactory::AkonadiCollectionViewFactory( CalendarView *view )
  : mView( view ), mAkonadiCollectionView( 0 )
{
}

namespace {

static bool hasCompatibleMimeTypes( const Akonadi::Collection &collection )
{
  static QStringList goodMimeTypes;

  if ( goodMimeTypes.isEmpty() ) {
    goodMimeTypes << QLatin1String( "text/calendar" )
                  << KCalCore::Event::eventMimeType()
                  << KCalCore::Todo::todoMimeType()
                  << KCalCore::Journal::journalMimeType();
  }

  for ( int i=0; i<goodMimeTypes.count(); ++i ) {
    if ( collection.contentMimeTypes().contains( goodMimeTypes.at( i ) ) ) {
      return true;
    }
  }

  return false;
}

class SortProxyModel : public QSortFilterProxyModel
{
  public:
    explicit SortProxyModel( QObject *parent=0 )
      : QSortFilterProxyModel( parent )
    {
        setDynamicSortFilter(true);
    }

    static int score(const QModelIndex &index)
    {
        int score = 0;
        if (index.data(PersonRole).isValid()) {
            score += 1;
        }
        if (index.data(IsSearchResultRole).toBool()) {
            score += 2;
        }
        return score;
    }

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const
    {
        const int leftScore = score(left);
        const int rightScore = score(right);
        // kDebug() << left.data().toString() << leftScore << " : " << right.data().toString() << rightScore;
        if (leftScore != rightScore) {
            return leftScore < rightScore;
        }

        return QString::localeAwareCompare(left.data().toString(), right.data().toString()) < 0;
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

        if ( hasCompatibleMimeTypes( collection ) ) {
          if ( collection.hasAttribute<Akonadi::EntityDisplayAttribute>() &&
               !collection.attribute<Akonadi::EntityDisplayAttribute>()->iconName().isEmpty() ) {
            return collection.attribute<Akonadi::EntityDisplayAttribute>()->icon();
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

class CollectionFilter : public QSortFilterProxyModel
{
  public:
    explicit CollectionFilter( QObject *parent=0 )
      : QSortFilterProxyModel( parent )
    {
        setDynamicSortFilter(true);
    }

  protected:
    virtual bool filterAcceptsRow(int row, const QModelIndex &sourceParent) const {
        const QModelIndex sourceIndex = sourceParent.child(row, 0);
        const Akonadi::Collection &col = sourceIndex.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
        CollectionIdentificationAttribute *attr = col.attribute<CollectionIdentificationAttribute>();
        //We filter the user folders because we insert person nodes for user folders.
        if (attr && ((attr->collectionNamespace() == "usertoplevel") || (attr->collectionNamespace() == "usertoplevel"))
                || (col.name().contains(QLatin1String("Other Users")))) {
            return false;
        }
        return true;
    }
};

class EnabledModel : public QSortFilterProxyModel
{
public:
    explicit EnabledModel(QObject *parent=0)
      : QSortFilterProxyModel(parent)
    {
    }

protected:

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if (role == EnabledRole) {
            Akonadi::Collection col = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
            if (col.shouldList(Akonadi::Collection::ListDisplay)) {
                return Qt::Checked;
            } else {
                return Qt::Unchecked;
            }
        }
        return QSortFilterProxyModel::data(index, role);
    }
};

class CalendarDelegateModel : public QSortFilterProxyModel
{
public:
    explicit CalendarDelegateModel(QObject *parent=0)
      : QSortFilterProxyModel(parent)
    {
    }

protected:
    bool checkChildren(const QModelIndex &index, int role, const QVariant &value) const
    {
        const QModelIndex sourceIndex = mapToSource(index);
        for (int i = 0; i < sourceModel()->rowCount(sourceIndex); i++) {
            const QModelIndex child = sourceModel()->index(i, 0, sourceIndex);
            if (child.data(role) != value) {
                return false;
            }
        }
        return true;
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if (role == Qt::CheckStateRole) {
            if (sourceModel()->hasChildren(mapToSource(index)) && index.data(NodeTypeRole).toInt() == PersonNodeRole) {
                bool allChecked = checkChildren(index, role, Qt::Checked);
                bool allUnchecked = checkChildren(index, role, Qt::Unchecked);
                if (allChecked) {
                    return Qt::Checked;
                } else if (allUnchecked) {
                    return Qt::Unchecked;
                } else {
                    return Qt::PartiallyChecked;
                }
            }
        }
        if (role == EnabledRole) {
            if (sourceModel()->hasChildren(mapToSource(index)) && index.data(NodeTypeRole).toInt() == PersonNodeRole) {
                bool allChecked = checkChildren(index, role, Qt::Checked);
                bool allUnchecked = checkChildren(index, role, Qt::Unchecked);
                // kDebug() << "person node " << index.data().toString() << allChecked << allUnchecked;
                if (allChecked) {
                    return Qt::Checked;
                } else if (allUnchecked) {
                    return Qt::Unchecked;
                } else {
                    return Qt::PartiallyChecked;
                }
            }
        }

        return QSortFilterProxyModel::data(index, role);
    }

    void setChildren(const QModelIndex &sourceIndex, const QVariant &value, int role) const
    {
        if (!sourceIndex.isValid()) {
            return;
        }
        for (int i = 0; i < sourceModel()->rowCount(sourceIndex); i++) {
            const QModelIndex child = sourceModel()->index(i, 0, sourceIndex);
            sourceModel()->setData(child, value, role);
            setChildren(child, value, role);
        }
    }

    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)
    {
        if (role == Qt::CheckStateRole) {
            if (sourceModel()->hasChildren(mapToSource(index)) && index.data(NodeTypeRole).toInt() == PersonNodeRole) {
                setChildren(mapToSource(index), value, role);
            }
        }
        return QSortFilterProxyModel::setData(index, value, role);
    }
};

} // anonymous namespace

CalendarViewExtension *AkonadiCollectionViewFactory::create( QWidget *parent )
{
  mAkonadiCollectionView = new AkonadiCollectionView( view(), true, parent );
  QObject::connect( mAkonadiCollectionView, SIGNAL(resourcesChanged(bool)),
                    mView, SLOT(resourcesChanged()) );
  QObject::connect( mAkonadiCollectionView, SIGNAL(resourcesAddedRemoved()),
                    mView, SLOT(resourcesChanged()) );
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
    mCollectionView(0),
    mBaseModel( 0 ),
    mSelectionProxyModel( 0 ),
    mNotSendAddRemoveSignal( false ),
    mWasDefaultCalendar( false ),
    mHasContextMenu( hasContextMenu )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setMargin( 0 );
  topLayout->setSpacing( KDialog::spacingHint() );

  KLineEdit *searchCol = new KLineEdit( this );
  searchCol->setClearButtonShown( true );
  searchCol->setClickMessage( i18nc( "@info/plain Displayed grayed-out inside the "
                                                    "textbox, verb to search", "Search" ) );
  topLayout->addWidget( searchCol );

  ColorProxyModel *colorProxy = new ColorProxyModel( this );
  colorProxy->setObjectName( QLatin1String("Show calendar colors") );
  colorProxy->setDynamicSortFilter( true );
  mBaseModel = colorProxy;


  //Model that displays users
  ReparentingModel *userProxy = new ReparentingModel( this );
  userProxy->setNodeManager(ReparentingModel::NodeManager::Ptr(new PersonNodeManager(*userProxy)));
  userProxy->setSourceModel(colorProxy);

  EnabledModel *enabledModel = new EnabledModel(this);
  enabledModel->setSourceModel(userProxy);

  CalendarDelegateModel *calendarDelegateModel = new CalendarDelegateModel(this);
  calendarDelegateModel->setSourceModel(enabledModel);

  //Hide collections that are not required
  CollectionFilter *collectionFilter = new CollectionFilter( this );
  collectionFilter->setSourceModel( calendarDelegateModel );

  SortProxyModel *sortProxy = new SortProxyModel( this );
  sortProxy->setSourceModel(collectionFilter);

  mCollectionView = new Akonadi::EntityTreeView( this );
  mCollectionView->header()->hide();
  mCollectionView->setRootIsDecorated( true );
  // mCollectionView->setSorting( true );
  {
    StyledCalendarDelegate *delegate = new StyledCalendarDelegate(mCollectionView);
    connect(delegate, SIGNAL(action(QModelIndex, int)), this, SLOT(onAction(QModelIndex, int)));
    mCollectionView->setItemDelegate( delegate );
  }
  mCollectionView->setModel( sortProxy );
  connect( mCollectionView->selectionModel(),
           SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
           SLOT(updateMenu()) );
  mNewNodeExpander = new NewNodeExpander(mCollectionView, false, QLatin1String("CollectionTreeView"));


  //Filter tree view.
  ReparentingModel *searchProxy = new ReparentingModel( this );
  searchProxy->setSourceModel(collectionFilter);
  searchProxy->setObjectName(QLatin1String("searchProxy"));

  KRecursiveFilterProxyModel *filterTreeViewModel = new KRecursiveFilterProxyModel( this );
  filterTreeViewModel->setDynamicSortFilter( true );
  filterTreeViewModel->setSourceModel( searchProxy );
  filterTreeViewModel->setFilterCaseSensitivity( Qt::CaseInsensitive );
//   filterTreeViewModel->setObjectName( "Recursive filtering, for the search bar" );
  connect( searchCol, SIGNAL(textChanged(QString)),
          filterTreeViewModel, SLOT(setFilterFixedString(QString)) );

  SortProxyModel *searchSortProxy = new SortProxyModel( this );
  searchSortProxy->setSourceModel(filterTreeViewModel);

  Akonadi::EntityTreeView *mSearchView = new Akonadi::EntityTreeView( this );
  mSearchView->header()->hide();
  mSearchView->setRootIsDecorated( true );
  {
    StyledCalendarDelegate *delegate = new StyledCalendarDelegate(mCollectionView);
    connect(delegate, SIGNAL(action(QModelIndex, int)), this, SLOT(onAction(QModelIndex, int)));
    mSearchView->setItemDelegate( delegate );
  }
  mSearchView->setModel( searchSortProxy );
  new NewNodeExpander(mSearchView, true, QString());

  mController = new Controller(userProxy, searchProxy, this);
  connect( searchCol, SIGNAL(textChanged(QString)),
          mController, SLOT(setSearchString(QString)) );
  connect( mController, SIGNAL(searchIsActive(bool)),
          this, SLOT(onSearchIsActive(bool)) );

  mStackedWidget = new QStackedWidget(this);
  mStackedWidget->addWidget(mCollectionView);
  mStackedWidget->addWidget(mSearchView);
  mStackedWidget->setCurrentWidget(mCollectionView);

  topLayout->addWidget( mStackedWidget );

  connect( mBaseModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
           this, SLOT(rowsInserted(QModelIndex,int,int)) );

  KXMLGUIClient *xmlclient = KOCore::self()->xmlguiClient( view );
  if ( xmlclient ) {
    mCollectionView->setXmlGuiClient( xmlclient );

    mActionManager =
      new Akonadi::StandardCalendarActionManager( xmlclient->actionCollection(), mCollectionView );

    QList<Akonadi::StandardActionManager::Type> standardActions;
    standardActions << Akonadi::StandardActionManager::CreateCollection
                    << Akonadi::StandardActionManager::CopyCollections
                    << Akonadi::StandardActionManager::DeleteCollections
                    << Akonadi::StandardActionManager::SynchronizeCollections
                    << Akonadi::StandardActionManager::CollectionProperties
                    << Akonadi::StandardActionManager::CopyItems
                    << Akonadi::StandardActionManager::Paste
                    << Akonadi::StandardActionManager::DeleteItems
                    << Akonadi::StandardActionManager::CutItems
                    << Akonadi::StandardActionManager::CutCollections
                    << Akonadi::StandardActionManager::CreateResource
                    << Akonadi::StandardActionManager::DeleteResources
                    << Akonadi::StandardActionManager::ResourceProperties
                    << Akonadi::StandardActionManager::SynchronizeResources
                    << Akonadi::StandardActionManager::SynchronizeCollectionTree
                    << Akonadi::StandardActionManager::SynchronizeCollectionsRecursive;

    Q_FOREACH( Akonadi::StandardActionManager::Type standardAction, standardActions ) {
      mActionManager->createAction( standardAction );
    }

    QList<Akonadi::StandardCalendarActionManager::Type> calendarActions;
    calendarActions << Akonadi::StandardCalendarActionManager::CreateEvent
                   << Akonadi::StandardCalendarActionManager::CreateTodo
                   << Akonadi::StandardCalendarActionManager::CreateSubTodo
                   << Akonadi::StandardCalendarActionManager::CreateJournal
                   << Akonadi::StandardCalendarActionManager::EditIncidence;

    Q_FOREACH( Akonadi::StandardCalendarActionManager::Type calendarAction, calendarActions ) {
      mActionManager->createAction( calendarAction );
    }

    mActionManager->setCollectionSelectionModel( mCollectionView->selectionModel() );

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
                                    ki18nc( "@title:window", "Properties of Calendar Folder %1" ) );

    const QStringList pages =
      QStringList() << QLatin1String( "CalendarSupport::CollectionGeneralPage" )
                    << QLatin1String( "Akonadi::CachePolicyPage" )
                    << QLatin1String( "PimCommon::CollectionAclPage" );

    mActionManager->setCollectionPropertiesPageNames( pages );

    mDisableColor = new KAction( mCollectionView );
    mDisableColor->setText( i18n( "&Disable Color" ) );
    mDisableColor->setEnabled( false );
    xmlclient->actionCollection()->addAction( QString::fromLatin1( "disable_color" ),
                                              mDisableColor );
    connect( mDisableColor, SIGNAL(triggered(bool)), this, SLOT(disableColor()) );

    mAssignColor = new KAction( mCollectionView );
    mAssignColor->setText( i18n( "&Assign Color..." ) );
    mAssignColor->setEnabled( false );
    xmlclient->actionCollection()->addAction( QString::fromLatin1( "assign_color" ), mAssignColor );
    connect( mAssignColor, SIGNAL(triggered(bool)), this, SLOT(assignColor()) );

    mDefaultCalendar = new KAction( mCollectionView );
    mDefaultCalendar->setText( i18n( "Use as &Default Calendar" ) );
    mDefaultCalendar->setEnabled( false );
    xmlclient->actionCollection()->addAction( QString::fromLatin1( "set_standard_calendar" ),
                                              mDefaultCalendar );

    //Disable a calendar or remove a referenced calendar
    QAction *disableAction = xmlclient->actionCollection()->addAction( QLatin1String("collection_disable"), this, SLOT(edit_disable()) );
    disableAction->setText( i18n( "Remove from list" ) );
    disableAction->setIcon(KIconLoader().loadIcon(QLatin1String("list-remove"), KIconLoader::Small));

    //Enable (subscribe) to a calendar.
    mEnableAction = xmlclient->actionCollection()->addAction( QLatin1String("collection_enable"), this, SLOT(edit_enable()) );
    mEnableAction->setText( i18n( "Add to list permanently" ) );
    mEnableAction->setIcon(KIconLoader().loadIcon(QLatin1String("bookmarks"), KIconLoader::Small));

    connect( mDefaultCalendar, SIGNAL(triggered(bool)), this, SLOT(setDefaultCalendar()) );
  }
}

AkonadiCollectionView::~AkonadiCollectionView()
{
    //Necessary because it's apparently impossible to detect in the note expander when to save the state before view get's deleted
    mNewNodeExpander->saveState();
}

void AkonadiCollectionView::onSearchIsActive(bool active)
{
    if (!active) {
        mStackedWidget->setCurrentIndex(0);
    } else {
        mStackedWidget->setCurrentIndex(1);
    }
}

void AkonadiCollectionView::setDefaultCalendar()
{
  QModelIndex index = mCollectionView->selectionModel()->currentIndex(); //selectedRows()
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
  QModelIndex index = mCollectionView->selectionModel()->currentIndex(); //selectedRows()
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
  QModelIndex index = mCollectionView->selectionModel()->currentIndex(); //selectedRows()
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

  new NewCalendarChecker( m );
  mBaseModel->setSourceModel( mSelectionProxyModel );
}

KCheckableProxyModel *AkonadiCollectionView::collectionSelectionProxyModel() const
{
  return mSelectionProxyModel;
}

Akonadi::EntityTreeView *AkonadiCollectionView::view() const
{
  return mCollectionView;
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
  bool enableAction = mCollectionView->selectionModel()->hasSelection();
  enableAction = enableAction &&
                 ( KOPrefs::instance()->agendaViewColors() != KOPrefs::CategoryOnly );
  mAssignColor->setEnabled( enableAction );
  QModelIndex index = mCollectionView->selectionModel()->currentIndex(); //selectedRows()

  bool disableStuff = true;

  if ( index.isValid() ) {
    //Returns an invalid collection on person nodes
    const Akonadi::Collection collection = CalendarSupport::collectionFromIndex( index );

    if ( collection.isValid() && !collection.contentMimeTypes().isEmpty() ) {
      const QString identifier = QString::number( collection.id() );
      const QColor defaultColor = KOPrefs::instance()->resourceColor( identifier );
      enableAction = enableAction && defaultColor.isValid();
      mDisableColor->setEnabled( enableAction );
      mDefaultCalendar->setEnabled( !KOHelper::isStandardCalendar( collection.id() ) &&
                                    collection.rights() & Akonadi::Collection::CanCreateItem );
      disableStuff = false;
    }
    if ( collection.isValid() && collection.shouldList(Akonadi::Collection::ListDisplay) ) {
        mEnableAction->setEnabled(false);
    } else {
        mEnableAction->setEnabled(true);
    }
  }
  if ( disableStuff ) {
    mDisableColor->setEnabled( false );
    mDefaultCalendar->setEnabled( false );
    mAssignColor->setEnabled( false );
  }
}

void AkonadiCollectionView::newCalendar()
{
  Akonadi::AgentTypeDialog dlg( this );
  dlg.setWindowTitle( i18n( "Add Calendar" ) );
  dlg.agentFilterProxyModel()->addMimeTypeFilter( QString::fromLatin1( "text/calendar" ) );
  dlg.agentFilterProxyModel()->addCapabilityFilter( QLatin1String("Resource") ); // show only resources, no agents
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
  QModelIndex index = mCollectionView->selectionModel()->currentIndex(); //selectedRows()
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
}

Akonadi::Collection AkonadiCollectionView::selectedCollection() const
{
  Akonadi::Collection collection;
  QItemSelectionModel *selectionModel = mCollectionView->selectionModel();
  if ( !selectionModel ) {
    return collection;
  }
  QModelIndexList indexes = selectionModel->selectedIndexes();
  if ( !indexes.isEmpty() ) {
    collection = indexes.first().data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
  }
  return collection;
}

Akonadi::Collection::List AkonadiCollectionView::checkedCollections() const
{
  Akonadi::Collection::List collections;
  if ( !mSelectionProxyModel ) {
    return collections;
  }
  QItemSelectionModel *selectionModel = mSelectionProxyModel->selectionModel();
  if ( !selectionModel ) {
    return collections;
  }
  QModelIndexList indexes = selectionModel->selectedIndexes();
  foreach( const QModelIndex &index, indexes ) {
    if ( index.isValid() ) {
      Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
      if ( collection.isValid() )
        collections << collection;
    }
  }
  return collections;
}

bool AkonadiCollectionView::isChecked(const Akonadi::Collection &collection) const
{
    if (!mSelectionProxyModel)
        return false;
    QItemSelectionModel *selectionModel = mSelectionProxyModel->selectionModel();
    if (!selectionModel)
        return false;
    QModelIndexList indexes = selectionModel->selectedIndexes();
    foreach(const QModelIndex &index, indexes) {
        if (index.isValid()) {
            Akonadi::Collection c = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
            if (c.id() == collection.id()) {
                return true;
            }
        }
    }
    return false;
}

Akonadi::EntityTreeModel *AkonadiCollectionView::entityTreeModel() const
{
  QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel*>( mCollectionView->model() );
  while( proxy ) {
    Akonadi::EntityTreeModel *etm = qobject_cast<Akonadi::EntityTreeModel*>( proxy->sourceModel() );
    if ( etm ) {
      return etm;
    }
    proxy = qobject_cast<QAbstractProxyModel*>( proxy->sourceModel() );
  }

  kWarning() << "Couldn't find EntityTreeModel";
  return 0;
}

void AkonadiCollectionView::edit_disable()
{
    Akonadi::Collection col = mCollectionView->currentIndex().data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    if (col.isValid()) {
        mController->setCollectionState(col, Controller::Disabled);
    }
    const QVariant var = mCollectionView->currentIndex().data(PersonRole);
    if (var.isValid()) {
        mController->removePerson(var.value<Person>());
    }
}

void AkonadiCollectionView::edit_enable()
{
    Akonadi::Collection col = mCollectionView->currentIndex().data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    kDebug() << col.name();
    if (col.isValid()) {
        mController->setCollectionState(col, Controller::Enabled);
    }
    const QVariant var = mCollectionView->currentIndex().data(PersonRole);
    if (var.isValid()) {
        mController->addPerson(var.value<Person>());
    }
}

void AkonadiCollectionView::onAction(const QModelIndex &index, int a)
{
    const StyledCalendarDelegate::Action action = static_cast<StyledCalendarDelegate::Action>(a);
    switch (action) {
        case StyledCalendarDelegate::AddToList: {
            const Akonadi::Collection col = index.data(CollectionRole).value<Akonadi::Collection>();
            if (col.isValid()) {
                mController->setCollectionState(col, Controller::Referenced);
            } else {
                const QVariant var = index.data(PersonRole);
                if (var.isValid()) {
                    mController->addPerson(var.value<Person>());
                }
            }
        }
        break;
        case StyledCalendarDelegate::RemoveFromList: {
            const Akonadi::Collection col = CalendarSupport::collectionFromIndex(index);
            if (col.isValid()) {
                mController->setCollectionState(col, Controller::Disabled);
            } else {
                const QVariant var = index.data(PersonRole);
                if (var.isValid()) {
                    mController->removePerson(var.value<Person>());
                }
            }
        }
        break;
        case StyledCalendarDelegate::Enable: {
            const Akonadi::Collection col = CalendarSupport::collectionFromIndex(index);
            if (col.isValid()) {
                mController->setCollectionState(col, Controller::Enabled);
            } else {
                const QVariant var = index.data(PersonRole);
                if (var.isValid()) {
                    mController->setCollectionState(Akonadi::Collection(var.value<Person>().rootCollection), Controller::Enabled, true);
                }
            }
        }
        break;
    }
}

#include "akonadicollectionview.moc"
