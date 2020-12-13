/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2003, 2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2009 Sebastian Sauer <sebsauer@kdab.net>
  SPDX-FileCopyrightText: 2010-2020 Laurent Montel <montel@kde.org>
  SPDX-FileCopyrightText: 2012 Sérgio Martins <iamsergio@gmail.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "akonadicollectionview.h"
#include "kocore.h"
#include "kohelper.h"
#include "prefs/koprefs.h"
#include "koglobals.h"
#include "manageshowcollectionproperties.h"
#include "views/collectionview/calendardelegate.h"
#include "views/collectionview/quickview.h"

#include <CalendarSupport/KCalPrefs>
#include <CalendarSupport/Utils>

#include <AkonadiCore/AgentFilterProxyModel>
#include <AkonadiCore/AgentInstanceCreateJob>
#include <AkonadiCore/AgentManager>
#include <AkonadiWidgets/AgentTypeDialog>
#include <AkonadiCore/CollectionDeleteJob>
#include <AkonadiCore/CollectionFilterProxyModel>
#include <AkonadiCore/EntityDisplayAttribute>
#include <AkonadiWidgets/EntityTreeView>
#include <AkonadiCore/EntityTreeModel>
#include <AkonadiWidgets/ETMViewStateSaver>
#include <Akonadi/Calendar/StandardCalendarActionManager>
#include <AkonadiCore/CollectionIdentificationAttribute>
#include <PimCommonAkonadi/ManageServerSideSubscriptionJob>
#include <PimCommonAkonadi/MailUtil>

#include <KActionCollection>
#include <KCheckableProxyModel>
#include <KConfigGroup>
#include <KMessageBox>

#include <QSortFilterProxyModel>
#include "korganizer_debug.h"

#include <QAction>
#include <QColorDialog>
#include <QHeaderView>
#include <QLineEdit>
#include <QStackedWidget>
#include <QVBoxLayout>

static Akonadi::EntityTreeModel *findEtm(QAbstractItemModel *model)
{
    QAbstractProxyModel *proxyModel = nullptr;
    while (model) {
        proxyModel = qobject_cast<QAbstractProxyModel *>(model);
        if (proxyModel && proxyModel->sourceModel()) {
            model = proxyModel->sourceModel();
        } else {
            break;
        }
    }
    return qobject_cast<Akonadi::EntityTreeModel *>(model);
}

/**
* Automatically checks new calendar entries
*/
class NewCalendarChecker : public QObject
{
    Q_OBJECT
public:
    NewCalendarChecker(QAbstractItemModel *model)
        : QObject(model)
        , mCheckableProxy(model)
    {
        connect(model, &QAbstractItemModel::rowsInserted, this,
                &NewCalendarChecker::onSourceRowsInserted);
        qRegisterMetaType<QPersistentModelIndex>("QPersistentModelIndex");
    }

private Q_SLOTS:
    void onSourceRowsInserted(const QModelIndex &parent, int start, int end)
    {
        Akonadi::EntityTreeModel *etm = findEtm(mCheckableProxy);
        //Only check new collections and not during initial population
        if (!etm || !etm->isCollectionTreeFetched()) {
            return;
        }
        for (int i = start; i <= end; ++i) {
            qCDebug(KORGANIZER_LOG) << "checking " << i << parent << mCheckableProxy->index(i, 0,
                                                                                            parent).
                data().toString();
            const QModelIndex index = mCheckableProxy->index(i, 0, parent);
            QMetaObject::invokeMethod(this, "setCheckState", Qt::QueuedConnection,
                                      QGenericReturnArgument(),
                                      Q_ARG(QPersistentModelIndex, index));
        }
    }

    void setCheckState(const QPersistentModelIndex &index)
    {
        mCheckableProxy->setData(index, Qt::Checked, Qt::CheckStateRole);
        if (mCheckableProxy->hasChildren(index)) {
            onSourceRowsInserted(index, 0, mCheckableProxy->rowCount(index) - 1);
        }
    }

private:
    QAbstractItemModel *const mCheckableProxy;
};

/**
* Handles expansion state of a treeview
*
* Persists state, and automatically expands new entries.
* With expandAll enabled this class simply ensures that all indexes are fully expanded.
*/
class NewNodeExpander : public QObject
{
    Q_OBJECT
public:
    NewNodeExpander(QTreeView *view, bool expandAll, const QString &treeStateConfig)
        : QObject(view)
        , mTreeView(view)
        , mExpandAll(expandAll)
        , mTreeStateConfig(treeStateConfig)
    {
        connect(
            view->model(), &QAbstractItemModel::rowsInserted, this,
            &NewNodeExpander::onSourceRowsInserted);
        connect(
            view->model(), &QAbstractItemModel::layoutChanged, this,
            &NewNodeExpander::onLayoutChanged);
        connect(view->model(), &QAbstractItemModel::modelReset, this,
                &NewNodeExpander::onModelReset);
        restoreTreeState();
    }

    ~NewNodeExpander() override
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
        for (int i = start; i <= end; ++i) {
            const QModelIndex index = mTreeView->model()->index(i, 0, parent);
            // qCDebug(KORGANIZER_LOG) << "expanding " << index.data().toString();
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
        KSharedConfig::Ptr config = KSharedConfig::openConfig();
        KConfigGroup group = config->group(mTreeStateConfig);
        treeStateSaver.setView(mTreeView);
        treeStateSaver.setSelectionModel(nullptr); // we only save expand state
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
        if (treeStateRestorer) {  // We don't need more than one to be running at the same time
            delete treeStateRestorer;
        }
        qCDebug(KORGANIZER_LOG) << "Restore tree state";
        treeStateRestorer = new Akonadi::ETMViewStateSaver(); // not a leak
        KConfigGroup group(KSharedConfig::openConfig(), mTreeStateConfig);
        treeStateRestorer->setView(mTreeView);
        treeStateRestorer->setSelectionModel(nullptr);   // we only restore expand state
        treeStateRestorer->restoreState(group);
    }

    QPointer<Akonadi::ETMViewStateSaver> treeStateRestorer;
    QTreeView *mTreeView = nullptr;
    bool mExpandAll = false;
    QString mTreeStateConfig;
};

AkonadiCollectionViewFactory::AkonadiCollectionViewFactory(CalendarView *view)
    : mView(view)
    , mAkonadiCollectionView(nullptr)
{
}

static bool hasCompatibleMimeTypes(const Akonadi::Collection &collection)
{
    static QStringList goodMimeTypes;

    if (goodMimeTypes.isEmpty()) {
        goodMimeTypes << QStringLiteral("text/calendar")
                      << KCalendarCore::Event::eventMimeType()
                      << KCalendarCore::Todo::todoMimeType()
                      << KCalendarCore::Journal::journalMimeType();
    }

    for (int i = 0; i < goodMimeTypes.count(); ++i) {
        if (collection.contentMimeTypes().contains(goodMimeTypes.at(i))) {
            return true;
        }
    }

    return false;
}

namespace {
class ColorProxyModel : public QSortFilterProxyModel
{
public:
    explicit ColorProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
        , mInitDefaultCalendar(false)
    {
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (!index.isValid()) {
            return QVariant();
        }
        if (role == Qt::DecorationRole) {
            const Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);

            if (hasCompatibleMimeTypes(collection)) {
                if (collection.hasAttribute<Akonadi::EntityDisplayAttribute>()
                    && !collection.attribute<Akonadi::EntityDisplayAttribute>()->iconName().isEmpty()) {
                    return collection.attribute<Akonadi::EntityDisplayAttribute>()->icon();
                }
            }
        } else if (role == Qt::FontRole) {
            const Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);
            if (!collection.contentMimeTypes().isEmpty()
                && KOHelper::isStandardCalendar(collection.id())
                && collection.rights() & Akonadi::Collection::CanCreateItem) {
                QFont font = qvariant_cast<QFont>(QSortFilterProxyModel::data(index, Qt::FontRole));
                font.setBold(true);
                if (!mInitDefaultCalendar) {
                    mInitDefaultCalendar = true;
                    CalendarSupport::KCalPrefs::instance()->setDefaultCalendarId(collection.id());
                }
                return font;
            }
        } else if (role == Qt::DisplayRole) {
            const Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);
            const Akonadi::Collection::Id colId = collection.id();

            if (colId == CalendarSupport::KCalPrefs::instance()->defaultCalendarId()) {
                return i18nc("@item this is the default calendar", "%1 (Default)", collection.displayName());
            }
        }

        return QSortFilterProxyModel::data(index, role);
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        return Qt::ItemIsSelectable | QSortFilterProxyModel::flags(index);
    }

private:
    mutable bool mInitDefaultCalendar;
};

class CollectionFilter : public QSortFilterProxyModel
{
public:
    explicit CollectionFilter(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
        setDynamicSortFilter(true);
    }

protected:
    bool filterAcceptsRow(int row, const QModelIndex &sourceParent) const override
    {
        const QModelIndex sourceIndex = sourceModel()->index(row, 0, sourceParent);
        Q_ASSERT(sourceIndex.isValid());

        const Akonadi::Collection &col
            = sourceIndex.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
        const auto *attr
            = col.attribute<Akonadi::CollectionIdentificationAttribute>();

        //We filter the user folders because we insert person nodes for user folders.
        if ((attr && attr->collectionNamespace().startsWith("usertoplevel"))
            || col.name().contains(QLatin1String("Other Users"))) {
            return false;
        }
        return true;
    }
};

class CalendarDelegateModel : public QSortFilterProxyModel
{
public:
    explicit CalendarDelegateModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
    }

protected:
    bool checkChildren(const QModelIndex &index, int role, const QVariant &value) const
    {
        const QModelIndex sourceIndex = mapToSource(index);
        for (int i = 0; i < sourceModel()->rowCount(sourceIndex); ++i) {
            const QModelIndex child = sourceModel()->index(i, 0, sourceIndex);
            if (child.data(role) != value) {
                return false;
            }
        }
        return true;
    }

    void setChildren(const QModelIndex &sourceIndex, const QVariant &value, int role) const
    {
        if (!sourceIndex.isValid()) {
            return;
        }
        for (int i = 0; i < sourceModel()->rowCount(sourceIndex); ++i) {
            const QModelIndex child = sourceModel()->index(i, 0, sourceIndex);
            sourceModel()->setData(child, value, role);
            setChildren(child, value, role);
        }
    }
};
}

CalendarViewExtension *AkonadiCollectionViewFactory::create(QWidget *parent)
{
    mAkonadiCollectionView = new AkonadiCollectionView(view(), true, parent);
    QObject::connect(mAkonadiCollectionView, &AkonadiCollectionView::resourcesChanged, mView,
                     &CalendarView::resourcesChanged);
    QObject::connect(mAkonadiCollectionView, &AkonadiCollectionView::resourcesAddedRemoved, mView,
                     &CalendarView::resourcesChanged);
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

AkonadiCollectionView::AkonadiCollectionView(CalendarView *view, bool hasContextMenu, QWidget *parent)
    : CalendarViewExtension(parent)
    , mActionManager(nullptr)
    , mCollectionView(nullptr)
    , mBaseModel(nullptr)
    , mSelectionProxyModel(nullptr)
    , mAssignColor(nullptr)
    , mDefaultCalendar(nullptr)
    , mServerSideSubscription(nullptr)
    , mNotSendAddRemoveSignal(false)
    , mWasDefaultCalendar(false)
    , mHasContextMenu(hasContextMenu)
{
    mManagerShowCollectionProperties = new ManageShowCollectionProperties(this, this);

    auto *topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins({});

    auto *searchCol = new QLineEdit(this);
    searchCol->setToolTip(i18nc("@info:tooltip", "Set search keyword"));
    searchCol->setWhatsThis(
        i18nc("@info:whatsthis", "Lets you search for a keyword in your calendars"));
    searchCol->setClearButtonEnabled(true);
    searchCol->setPlaceholderText(i18nc("@info/plain Displayed grayed-out inside the "
                                        "textbox, verb to search", "Search..."));
    topLayout->addWidget(searchCol);

    auto *colorProxy = new ColorProxyModel(this);
    colorProxy->setObjectName(QStringLiteral("Show calendar colors"));
    colorProxy->setDynamicSortFilter(true);
    mBaseModel = colorProxy;

    auto *calendarDelegateModel = new CalendarDelegateModel(this);
    calendarDelegateModel->setSourceModel(mBaseModel);

    //Hide collections that are not required
    auto *collectionFilter = new CollectionFilter(this);
    collectionFilter->setSourceModel(calendarDelegateModel);

    mCollectionView = new Akonadi::EntityTreeView(this);
    mCollectionView->header()->hide();
    mCollectionView->setRootIsDecorated(true);
    // mCollectionView->setSorting( true );
    {
        auto *delegate = new StyledCalendarDelegate(mCollectionView);
        connect(delegate, &StyledCalendarDelegate::action, this, &AkonadiCollectionView::onAction);
        mCollectionView->setItemDelegate(delegate);
    }
    mCollectionView->setModel(collectionFilter);
    connect(
        mCollectionView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
        &AkonadiCollectionView::updateMenu);
    mNewNodeExpander
        = new NewNodeExpander(mCollectionView, false, QStringLiteral("CollectionTreeView"));

    //Filter tree view.
    auto *searchProxy = new ReparentingModel(this);
    searchProxy->setSourceModel(collectionFilter);
    searchProxy->setObjectName(QStringLiteral("searchProxy"));

    auto *filterTreeViewModel = new QSortFilterProxyModel(this);
    filterTreeViewModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    filterTreeViewModel->setRecursiveFilteringEnabled(true);
    filterTreeViewModel->setDynamicSortFilter(true);
    filterTreeViewModel->setSourceModel(searchProxy);
    connect(searchCol, &QLineEdit::textChanged, filterTreeViewModel,
            &QSortFilterProxyModel::setFilterWildcard);

    auto *mSearchView = new Akonadi::EntityTreeView(this);
    mSearchView->header()->hide();
    mSearchView->setRootIsDecorated(true);
    {
        auto *delegate = new StyledCalendarDelegate(mCollectionView);
        connect(delegate, &StyledCalendarDelegate::action, this, &AkonadiCollectionView::onAction);
        mSearchView->setItemDelegate(delegate);
    }
    mSearchView->setModel(filterTreeViewModel);
    new NewNodeExpander(mSearchView, true, QString());

    mStackedWidget = new QStackedWidget(this);
    mStackedWidget->addWidget(mCollectionView);
    mStackedWidget->addWidget(mSearchView);
    mStackedWidget->setCurrentWidget(mCollectionView);

    topLayout->addWidget(mStackedWidget);

    connect(mBaseModel, &QAbstractProxyModel::rowsInserted, this,
            &AkonadiCollectionView::rowsInserted);

    KXMLGUIClient *xmlclient = KOCore::self()->xmlguiClient(view);
    if (xmlclient) {
        mCollectionView->setXmlGuiClient(xmlclient);

        mActionManager
            = new Akonadi::StandardCalendarActionManager(
                  xmlclient->actionCollection(), mCollectionView);

        QList<Akonadi::StandardActionManager::Type> standardActions;
        standardActions << Akonadi::StandardActionManager::CreateCollection
                        << Akonadi::StandardActionManager::DeleteCollections
                        << Akonadi::StandardActionManager::SynchronizeCollections
                        << Akonadi::StandardActionManager::CollectionProperties
                        << Akonadi::StandardActionManager::CopyItems
                        << Akonadi::StandardActionManager::Paste
                        << Akonadi::StandardActionManager::DeleteItems
                        << Akonadi::StandardActionManager::CutItems
                        << Akonadi::StandardActionManager::CreateResource
                        << Akonadi::StandardActionManager::DeleteResources
                        << Akonadi::StandardActionManager::ResourceProperties
                        << Akonadi::StandardActionManager::SynchronizeResources
                        << Akonadi::StandardActionManager::SynchronizeCollectionTree
                        << Akonadi::StandardActionManager::CopyCollectionToMenu
                        << Akonadi::StandardActionManager::MoveCollectionToMenu;

        for (Akonadi::StandardActionManager::Type standardAction : qAsConst(standardActions)) {
            mActionManager->createAction(standardAction);
        }

        QList<Akonadi::StandardCalendarActionManager::Type> calendarActions;
        calendarActions << Akonadi::StandardCalendarActionManager::CreateEvent
                        << Akonadi::StandardCalendarActionManager::CreateTodo
                        << Akonadi::StandardCalendarActionManager::CreateSubTodo
                        << Akonadi::StandardCalendarActionManager::CreateJournal
                        << Akonadi::StandardCalendarActionManager::EditIncidence;

        for (Akonadi::StandardCalendarActionManager::Type calendarAction : qAsConst(calendarActions)) {
            mActionManager->createAction(calendarAction);
        }

        mActionManager->setCollectionSelectionModel(mCollectionView->selectionModel());

        mActionManager->interceptAction(Akonadi::StandardActionManager::CreateResource);
        mActionManager->interceptAction(Akonadi::StandardActionManager::DeleteResources);
        mActionManager->interceptAction(Akonadi::StandardActionManager::DeleteCollections);

        connect(mActionManager->action(
                    Akonadi::StandardActionManager::CreateResource), &QAction::triggered,
                this, &AkonadiCollectionView::newCalendar);
        connect(mActionManager->action(
                    Akonadi::StandardActionManager::DeleteResources), &QAction::triggered,
                this, &AkonadiCollectionView::deleteCalendar);
        connect(mActionManager->action(
                    Akonadi::StandardActionManager::DeleteCollections), &QAction::triggered,
                this, &AkonadiCollectionView::deleteCalendar);

        mActionManager->setContextText(Akonadi::StandardActionManager::CollectionProperties,
                                       Akonadi::StandardActionManager::DialogTitle,
                                       ki18nc("@title:window", "Properties of Calendar Folder %1"));

        mActionManager->action(Akonadi::StandardActionManager::CreateCollection)->setProperty(
            "ContentMimeTypes",
            QStringList() << Akonadi::Collection::mimeType()
                          << KCalendarCore::Event::eventMimeType());

        mActionManager->interceptAction(Akonadi::StandardActionManager::CollectionProperties);
        connect(mActionManager->action(
                    Akonadi::StandardActionManager::CollectionProperties), &QAction::triggered,
                mManagerShowCollectionProperties, &ManageShowCollectionProperties::showCollectionProperties);

        mAssignColor = new QAction(mCollectionView);
        mAssignColor->setText(i18nc("@action:inmenu", "&Set Folder Color..."));
        mAssignColor->setEnabled(false);
        xmlclient->actionCollection()->addAction(QStringLiteral("assign_color"), mAssignColor);
        connect(mAssignColor, &QAction::triggered, this, &AkonadiCollectionView::assignColor);

        mDefaultCalendar = new QAction(mCollectionView);
        mDefaultCalendar->setText(i18nc("@action:inmenu", "Set as &Default Folder"));
        mDefaultCalendar->setEnabled(false);
        xmlclient->actionCollection()->addAction(QStringLiteral("set_standard_calendar"),
                                                 mDefaultCalendar);
        connect(mDefaultCalendar, &QAction::triggered, this,
                &AkonadiCollectionView::setDefaultCalendar);

        mServerSideSubscription = new QAction(QIcon::fromTheme(QStringLiteral("folder-bookmarks")),
                                              i18nc("@action:inmenu", "Serverside Subscription..."), this);
        xmlclient->actionCollection()->addAction(QStringLiteral("serverside_subscription"),
                                                 mServerSideSubscription);
        connect(mServerSideSubscription, &QAction::triggered, this,
                &AkonadiCollectionView::slotServerSideSubscription);
    }
}

AkonadiCollectionView::~AkonadiCollectionView()
{
    // Need this because it seems impossible to detect in the NodeExpander when to save the state
    // before the view is deleted.
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

void AkonadiCollectionView::slotServerSideSubscription()
{
    const QModelIndex index = mCollectionView->selectionModel()->currentIndex(); //selectedRows()
    Q_ASSERT(index.isValid());
    const Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);
    if (!collection.isValid()) {
        return;
    }
    auto *job
        = new PimCommon::ManageServerSideSubscriptionJob(this);
    job->setCurrentCollection(collection);
    job->setParentWidget(this);
    job->start();
}

Akonadi::Collection AkonadiCollectionView::currentCalendar() const
{
    const QModelIndex index = mCollectionView->selectionModel()->currentIndex(); //selectedRows()
    Q_ASSERT(index.isValid());
    Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);
    return collection;
}

void AkonadiCollectionView::setDefaultCalendar()
{
    QModelIndex index = mCollectionView->selectionModel()->currentIndex(); //selectedRows()
    Q_ASSERT(index.isValid());
    const Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);

    // Ask if they really want to do this
    const Akonadi::Collection curCol(CalendarSupport::KCalPrefs::instance()->defaultCalendarId());
    if (curCol.isValid()
        && KMessageBox::warningContinueCancel(
            this,
            i18nc("@info",
                  "Do you really want replace your current default calendar with \"%1\"?",
                  collection.displayName()),
            i18nc("@title:window", "Replace Default Calendar?")) != KMessageBox::Continue) {
        return;
    }

    CalendarSupport::KCalPrefs::instance()->setDefaultCalendarId(collection.id());
    CalendarSupport::KCalPrefs::instance()->usrSave();
    updateMenu();
    updateView();

    Q_EMIT defaultResourceChanged(collection);
}

void AkonadiCollectionView::assignColor()
{
    QModelIndex index = mCollectionView->selectionModel()->currentIndex(); //selectedRows()
    Q_ASSERT(index.isValid());
    const Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);
    Q_ASSERT(collection.isValid());

    const QColor defaultColor = KOHelper::resourceColor(collection);
    QColor myColor;
    myColor = QColorDialog::getColor(defaultColor);
    if (myColor.isValid() && myColor != defaultColor) {
        KOHelper::setResourceColor(collection, myColor);
        Q_EMIT colorsChanged();
        updateMenu();
        updateView();
    }
}

void AkonadiCollectionView::setCollectionSelectionProxyModel(KCheckableProxyModel *m)
{
    if (mSelectionProxyModel == m) {
        return;
    }

    mSelectionProxyModel = m;
    if (!mSelectionProxyModel) {
        return;
    }

    new NewCalendarChecker(m);
    mBaseModel->setSourceModel(mSelectionProxyModel);
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
    Q_EMIT resourcesChanged(mSelectionProxyModel
                            ? mSelectionProxyModel->selectionModel()->hasSelection() : false);
}

void AkonadiCollectionView::updateMenu()
{
    if (!mHasContextMenu) {
        return;
    }
    bool enableAction = mCollectionView->selectionModel()->hasSelection();
    enableAction = enableAction && (KOPrefs::instance()->agendaViewColors() != KOPrefs::CategoryOnly);
    mAssignColor->setEnabled(enableAction);
    QModelIndex index = mCollectionView->selectionModel()->currentIndex(); //selectedRows()

    bool disableStuff = true;

    if (index.isValid()) {
        //Returns an invalid collection on person nodes
        const Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);

        if (collection.isValid() && !collection.contentMimeTypes().isEmpty()) {
            if (collection.remoteId() == QLatin1String("akonadi_birthdays_resource")) {
                mAssignColor->setEnabled(false);
            }

            mDefaultCalendar->setEnabled(
                !KOHelper::isStandardCalendar(collection.id())
                && (collection.rights() & Akonadi::Collection::CanCreateItem)
                && !collection.isVirtual()
                && collection.contentMimeTypes().contains(KCalendarCore::Event::eventMimeType()));
            disableStuff = false;
        }
        bool isOnline;
        mServerSideSubscription->setEnabled(PimCommon::MailUtil::isImapFolder(collection,
                                                                              isOnline));
    } else {
        mServerSideSubscription->setEnabled(false);
    }
    if (disableStuff) {
        mDefaultCalendar->setEnabled(false);
        mAssignColor->setEnabled(false);
    }
}

void AkonadiCollectionView::newCalendar()
{
    QPointer<Akonadi::AgentTypeDialog> dlg = new Akonadi::AgentTypeDialog(this);
    dlg->setWindowTitle(i18nc("@title:window", "Add Calendar"));
    dlg->agentFilterProxyModel()->addMimeTypeFilter(QStringLiteral("text/calendar"));
    dlg->agentFilterProxyModel()->addCapabilityFilter(QStringLiteral("Resource"));   // show only resources, no agents
    if (dlg->exec()) {
        mNotSendAddRemoveSignal = true;
        const Akonadi::AgentType agentType = dlg->agentType();
        if (agentType.isValid()) {
            auto *job = new Akonadi::AgentInstanceCreateJob(agentType,
                                                                                       this);
            job->configure(this);
            connect(job, &Akonadi::AgentInstanceCreateJob::result, this,
                    &AkonadiCollectionView::newCalendarDone);
            job->start();
        }
    }
    delete dlg;
}

void AkonadiCollectionView::newCalendarDone(KJob *job)
{
    auto *createjob
        = static_cast<Akonadi::AgentInstanceCreateJob *>(job);
    if (createjob->error()) {
        //TODO(AKONADI_PORT)
        // this should show an error dialog and should be merged
        // with the identical code in ActionManager
        qCWarning(KORGANIZER_LOG) << "Create calendar failed:" << createjob->errorString();
        mNotSendAddRemoveSignal = false;
        return;
    }
    mNotSendAddRemoveSignal = false;
    //TODO
}

void AkonadiCollectionView::deleteCalendar()
{
    QModelIndex index = mCollectionView->selectionModel()->currentIndex(); //selectedRows()
    Q_ASSERT(index.isValid());
    const Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);
    Q_ASSERT(collection.isValid());

    const QString displayname = index.model()->data(index, Qt::DisplayRole).toString();
    Q_ASSERT(!displayname.isEmpty());

    if (KMessageBox::warningContinueCancel(
            this,
            i18nc("@info", "Do you really want to delete calendar %1?", displayname),
            i18nc("@title:window", "Delete Calendar"),
            KStandardGuiItem::del(),
            KStandardGuiItem::cancel(),
            QString(),
            KMessageBox::Dangerous) == KMessageBox::Continue) {
        bool isTopLevel = collection.parentCollection() == Akonadi::Collection::root();

        mNotSendAddRemoveSignal = true;
        mWasDefaultCalendar = KOHelper::isStandardCalendar(collection.id());

        if (!isTopLevel) {
            // deletes contents
            auto *job = new Akonadi::CollectionDeleteJob(collection, this);
            connect(job, &Akonadi::AgentInstanceCreateJob::result, this,
                    &AkonadiCollectionView::deleteCalendarDone);
        } else {
            // deletes the agent, not the contents
            const Akonadi::AgentInstance instance
                = Akonadi::AgentManager::self()->instance(collection.resource());
            if (instance.isValid()) {
                Akonadi::AgentManager::self()->removeInstance(instance);
            }
        }
    }
}

void AkonadiCollectionView::deleteCalendarDone(KJob *job)
{
    auto *deletejob = static_cast<Akonadi::CollectionDeleteJob *>(job);
    if (deletejob->error()) {
        qCWarning(KORGANIZER_LOG) << "Delete calendar failed:" << deletejob->errorString();
        mNotSendAddRemoveSignal = false;
        return;
    }
    if (mWasDefaultCalendar) {
        CalendarSupport::KCalPrefs::instance()->setDefaultCalendarId(Akonadi::Collection().id());
    }
    mNotSendAddRemoveSignal = false;
    //TODO
}

void AkonadiCollectionView::rowsInserted(const QModelIndex &, int, int)
{
    if (!mNotSendAddRemoveSignal) {
        Q_EMIT resourcesAddedRemoved();
    }
}

Akonadi::Collection AkonadiCollectionView::selectedCollection() const
{
    Akonadi::Collection collection;
    QItemSelectionModel *selectionModel = mCollectionView->selectionModel();
    if (!selectionModel) {
        return collection;
    }
    QModelIndexList indexes = selectionModel->selectedIndexes();
    if (!indexes.isEmpty()) {
        collection
            = indexes.first().data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    }
    return collection;
}

Akonadi::Collection::List AkonadiCollectionView::checkedCollections() const
{
    Akonadi::Collection::List collections;
    if (!mSelectionProxyModel) {
        return collections;
    }
    QItemSelectionModel *selectionModel = mSelectionProxyModel->selectionModel();
    if (!selectionModel) {
        return collections;
    }
    const QModelIndexList indexes = selectionModel->selectedIndexes();
    for (const QModelIndex &index : indexes) {
        if (index.isValid()) {
            const Akonadi::Collection collection = index.data(
                Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
            if (collection.isValid()) {
                collections << collection;
            }
        }
    }
    return collections;
}

bool AkonadiCollectionView::isChecked(const Akonadi::Collection &collection) const
{
    if (!mSelectionProxyModel) {
        return false;
    }
    QItemSelectionModel *selectionModel = mSelectionProxyModel->selectionModel();
    if (!selectionModel) {
        return false;
    }
    const QModelIndexList indexes = selectionModel->selectedIndexes();
    for (const QModelIndex &index : indexes) {
        if (index.isValid()) {
            const Akonadi::Collection c
                = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
            if (c.id() == collection.id()) {
                return true;
            }
        }
    }
    return false;
}

Akonadi::EntityTreeModel *AkonadiCollectionView::entityTreeModel() const
{
    auto *proxy = qobject_cast<QAbstractProxyModel *>(mCollectionView->model());
    while (proxy) {
        auto *etm = qobject_cast<Akonadi::EntityTreeModel *>(
            proxy->sourceModel());
        if (etm) {
            return etm;
        }
        proxy = qobject_cast<QAbstractProxyModel *>(proxy->sourceModel());
    }

    qCWarning(KORGANIZER_LOG) << "Couldn't find EntityTreeModel";
    return nullptr;
}

void AkonadiCollectionView::onAction(const QModelIndex &index, int a)
{
    const auto action = static_cast<StyledCalendarDelegate::Action>(a);
    switch (action) {
    case StyledCalendarDelegate::Quickview:
    {
        Quickview *quickview = new Quickview(CalendarSupport::collectionFromIndex(index));
        quickview->setAttribute(Qt::WA_DeleteOnClose, true);
        quickview->show();
        break;
    }
    case StyledCalendarDelegate::Total:
        //TODO: anything to implement here?
        break;
    }
}

#include "akonadicollectionview.moc"
