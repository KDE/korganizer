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

#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

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

#include <QAction>
#include <KActionCollection>
#include <KCheckableProxyModel>
#include <QColorDialog>
#include <KMessageBox>
#include <KRecursiveFilterProxyModel>
#include <QDebug>
#include <QDialog>

#include <QHeaderView>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QVBoxLayout>
#include <KConfigGroup>

AkonadiCollectionViewFactory::AkonadiCollectionViewFactory(CalendarView *view)
    : mView(view), mAkonadiCollectionView(0)
{
}

namespace
{

static bool hasCompatibleMimeTypes(const Akonadi::Collection &collection)
{
    static QStringList goodMimeTypes;

    if (goodMimeTypes.isEmpty()) {
        goodMimeTypes << QLatin1String("text/calendar")
                      << KCalCore::Event::eventMimeType()
                      << KCalCore::Todo::todoMimeType()
                      << KCalCore::Journal::journalMimeType();
    }

    for (int i = 0; i < goodMimeTypes.count(); ++i) {
        if (collection.contentMimeTypes().contains(goodMimeTypes.at(i))) {
            return true;
        }
    }

    return false;
}

class ColorDelegate : public QStyledItemDelegate
{
public:
    explicit ColorDelegate(QObject *parent = 0) : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const
    {
        QStyledItemDelegate::paint(painter, option, index);
        QStyleOptionViewItemV4 v4 = option;
        initStyleOption(&v4, index);
        if (v4.checkState == Qt::Checked) {
            const Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);
            QColor color = KOHelper::resourceColor(collection);
            if (color.isValid()) {
                QRect r = v4.rect;
                const int h = r.height() - 4;
                r.adjust(r.width() - h - 2, 2, - 2, -2);
                painter->save();
                painter->setRenderHint(QPainter::Antialiasing);
                QPen pen = painter->pen();
                pen.setColor(color);
                QPainterPath path;
                path.addRoundedRect(r, 5, 5);
                color.setAlpha(200);
                painter->fillPath(path, color);
                painter->strokePath(path, pen);
                painter->restore();
            }
        }
    }
};

class ColorProxyModel : public QSortFilterProxyModel
{
public:
    explicit ColorProxyModel(QObject *parent = 0)
        : QSortFilterProxyModel(parent), mInitDefaultCalendar(false)
    {
    }

    /* reimp */
    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid()) {
            return QVariant();
        }
        if (role == Qt::DecorationRole) {
            const Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);

            if (hasCompatibleMimeTypes(collection)) {
                if (collection.hasAttribute<Akonadi::EntityDisplayAttribute>() &&
                        !collection.attribute<Akonadi::EntityDisplayAttribute>()->iconName().isEmpty()) {
                    return collection.attribute<Akonadi::EntityDisplayAttribute>()->icon();
                }
            }
        } else if (role == Qt::FontRole) {
            const Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);
            if (!collection.contentMimeTypes().isEmpty() &&
                    KOHelper::isStandardCalendar(collection.id()) &&
                    collection.rights() & Akonadi::Collection::CanCreateItem) {
                QFont font = qvariant_cast<QFont>(QSortFilterProxyModel::data(index, Qt::FontRole));
                font.setBold(true);
                if (!mInitDefaultCalendar) {
                    mInitDefaultCalendar = true;
                    CalendarSupport::KCalPrefs::instance()->setDefaultCalendarId(collection.id());
                }
                return font;
            }
        }

        return QSortFilterProxyModel::data(index, role);
    }

    /* reimp */
    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        return Qt::ItemIsSelectable | QSortFilterProxyModel::flags(index);
    }

private:
    mutable bool mInitDefaultCalendar;
};

} // anonymous namespace

CalendarViewExtension *AkonadiCollectionViewFactory::create(QWidget *parent)
{
    mAkonadiCollectionView = new AkonadiCollectionView(view(), true, parent);
    QObject::connect(mAkonadiCollectionView, &AkonadiCollectionView::resourcesChanged, mView, &CalendarView::resourcesChanged);
    QObject::connect(mAkonadiCollectionView, &AkonadiCollectionView::resourcesAddedRemoved, mView, &CalendarView::resourcesChanged);
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

AkonadiCollectionView::AkonadiCollectionView(CalendarView *view, bool hasContextMenu,
        QWidget *parent)
    : CalendarViewExtension(parent),
      mActionManager(0),
      mCollectionView(0),
      mBaseModel(0),
      mSelectionProxyModel(0),
      mNotSendAddRemoveSignal(false),
      mWasDefaultCalendar(false),
      mHasContextMenu(hasContextMenu)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setMargin(0);
//TODO PORT QT5   topLayout->setSpacing( QDialog::spacingHint() );

    //KLineEdit *searchCol = new KLineEdit( this );
    //searchCol->setClearButtonShown( true );
    //searchCol->setPlaceholderText( i18nc( "@info/plain Displayed grayed-out inside the "
    //                                                   "textbox, verb to search", "Search" ) );
    //topLayout->addWidget( searchCol );

    ColorProxyModel *colorProxy = new ColorProxyModel(this);
    colorProxy->setObjectName(QLatin1String("Show calendar colors"));
    colorProxy->setDynamicSortFilter(true);
    mBaseModel = colorProxy;

    mCollectionView = new Akonadi::EntityTreeView(this);
    topLayout->addWidget(mCollectionView);
    mCollectionView->header()->hide();
    mCollectionView->setRootIsDecorated(true);
    mCollectionView->setItemDelegate(new ColorDelegate(this));

    //Filter tree view.
    //KRecursiveFilterProxyModel *filterTreeViewModel = new KRecursiveFilterProxyModel( this );
    //filterTreeViewModel->setDynamicSortFilter( true );
    //filterTreeViewModel->setSourceModel( colorProxy );
    //filterTreeViewModel->setFilterCaseSensitivity( Qt::CaseInsensitive );
    //filterTreeViewModel->setObjectName( "Recursive filtering, for the search bar" );
    mCollectionView->setModel(colorProxy);
    connect(mCollectionView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(updateMenu()));

    connect(mCollectionView->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            SLOT(checkNewCalendar(QModelIndex,int,int)));

    //connect( searchCol, SIGNAL(textChanged(QString)),
    //         filterTreeViewModel, SLOT(setFilterFixedString(QString)) );

    connect(mBaseModel, &QAbstractProxyModel::rowsInserted, this, &AkonadiCollectionView::rowsInserted);

    //mCollectionView->setSelectionMode( QAbstractItemView::NoSelection );
    KXMLGUIClient *xmlclient = KOCore::self()->xmlguiClient(view);
    if (xmlclient) {
        mCollectionView->setXmlGuiClient(xmlclient);

        mActionManager =
            new Akonadi::StandardCalendarActionManager(xmlclient->actionCollection(), mCollectionView);

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
                        << Akonadi::StandardActionManager::SynchronizeCollectionsRecursive;

        Q_FOREACH (Akonadi::StandardActionManager::Type standardAction, standardActions) {
            mActionManager->createAction(standardAction);
        }

        QList<Akonadi::StandardCalendarActionManager::Type> calendarActions;
        calendarActions << Akonadi::StandardCalendarActionManager::CreateEvent
                        << Akonadi::StandardCalendarActionManager::CreateTodo
                        << Akonadi::StandardCalendarActionManager::CreateSubTodo
                        << Akonadi::StandardCalendarActionManager::CreateJournal
                        << Akonadi::StandardCalendarActionManager::EditIncidence;

        Q_FOREACH (Akonadi::StandardCalendarActionManager::Type calendarAction, calendarActions) {
            mActionManager->createAction(calendarAction);
        }

        mActionManager->setCollectionSelectionModel(mCollectionView->selectionModel());

        mActionManager->interceptAction(Akonadi::StandardActionManager::CreateResource);
        mActionManager->interceptAction(Akonadi::StandardActionManager::DeleteResources);
        mActionManager->interceptAction(Akonadi::StandardActionManager::DeleteCollections);

        connect(mActionManager->action(Akonadi::StandardActionManager::CreateResource), SIGNAL(triggered(bool)),
                this, SLOT(newCalendar()));
        connect(mActionManager->action(Akonadi::StandardActionManager::DeleteResources), SIGNAL(triggered(bool)),
                this, SLOT(deleteCalendar()));
        connect(mActionManager->action(Akonadi::StandardActionManager::DeleteCollections), SIGNAL(triggered(bool)),
                this, SLOT(deleteCalendar()));

        mActionManager->setContextText(Akonadi::StandardActionManager::CollectionProperties,
                                       Akonadi::StandardActionManager::DialogTitle,
                                       ki18nc("@title:window", "Properties of Calendar Folder %1"));

        const QStringList pages =
            QStringList() << QLatin1String("CalendarSupport::CollectionGeneralPage")
            << QLatin1String("Akonadi::CachePolicyPage")
            << QLatin1String("PimCommon::CollectionAclPage");

        mActionManager->setCollectionPropertiesPageNames(pages);

        mDisableColor = new QAction(mCollectionView);
        mDisableColor->setText(i18n("&Disable Color"));
        mDisableColor->setEnabled(false);
        xmlclient->actionCollection()->addAction(QStringLiteral("disable_color"),
                mDisableColor);
        connect(mDisableColor, &QAction::triggered, this, &AkonadiCollectionView::disableColor);

        mAssignColor = new QAction(mCollectionView);
        mAssignColor->setText(i18n("&Assign Color..."));
        mAssignColor->setEnabled(false);
        xmlclient->actionCollection()->addAction(QStringLiteral("assign_color"), mAssignColor);
        connect(mAssignColor, &QAction::triggered, this, &AkonadiCollectionView::assignColor);

        mDefaultCalendar = new QAction(mCollectionView);
        mDefaultCalendar->setText(i18n("Use as &Default Calendar"));
        mDefaultCalendar->setEnabled(false);
        xmlclient->actionCollection()->addAction(QStringLiteral("set_standard_calendar"),
                mDefaultCalendar);
        connect(mDefaultCalendar, &QAction::triggered, this, &AkonadiCollectionView::setDefaultCalendar);
    }
}

AkonadiCollectionView::~AkonadiCollectionView()
{
    Akonadi::ETMViewStateSaver treeStateSaver;
    KConfigGroup group(KOGlobals::self()->config(), "CollectionTreeView");
    treeStateSaver.setView(mCollectionView);
    treeStateSaver.setSelectionModel(0);   // we only save expand state
    treeStateSaver.saveState(group);
}

void AkonadiCollectionView::restoreTreeState()
{
    static QPointer<Akonadi::ETMViewStateSaver> treeStateRestorer;
    if (treeStateRestorer) {  // We don't need more than one to be running at the same time
        delete treeStateRestorer;
    }
    treeStateRestorer = new Akonadi::ETMViewStateSaver(); // not a leak
    KConfigGroup group(KOGlobals::self()->config(), "CollectionTreeView");
    treeStateRestorer->setView(mCollectionView);
    treeStateRestorer->setSelectionModel(0);   // we only restore expand state
    treeStateRestorer->restoreState(group);
}

void AkonadiCollectionView::setDefaultCalendar()
{
    QModelIndex index = mCollectionView->selectionModel()->currentIndex(); //selectedRows()
    Q_ASSERT(index.isValid());
    const Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);
    CalendarSupport::KCalPrefs::instance()->setDefaultCalendarId(collection.id());
    CalendarSupport::KCalPrefs::instance()->usrSave();
    updateMenu();
    updateView();

    emit defaultResourceChanged(collection);
}

void AkonadiCollectionView::assignColor()
{
    QModelIndex index = mCollectionView->selectionModel()->currentIndex(); //selectedRows()
    Q_ASSERT(index.isValid());
    const Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);
    Q_ASSERT(collection.isValid());

    const QString identifier = QString::number(collection.id());
    const QColor defaultColor = KOPrefs::instance()->resourceColor(identifier);
    QColor myColor;
    myColor = QColorDialog::getColor(defaultColor);
    if (myColor.isValid() && myColor != defaultColor) {
        KOPrefs::instance()->setResourceColor(identifier, myColor);
        emit colorsChanged();
        updateMenu();
        updateView();
    }
}

void AkonadiCollectionView::disableColor()
{
    QModelIndex index = mCollectionView->selectionModel()->currentIndex(); //selectedRows()
    Q_ASSERT(index.isValid());
    const Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);
    Q_ASSERT(collection.isValid());
    const QString identifier = QString::number(collection.id());
    KOPrefs::instance()->setResourceColor(identifier, QColor());
    updateMenu();
    updateView();
    emit colorsChanged();
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
    emit resourcesChanged(mSelectionProxyModel ?
                          mSelectionProxyModel->selectionModel()->hasSelection() :
                          false);
}

void AkonadiCollectionView::updateMenu()
{
    if (!mHasContextMenu) {
        return;
    }
    bool enableAction = mCollectionView->selectionModel()->hasSelection();
    enableAction = enableAction &&
                   (KOPrefs::instance()->agendaViewColors() != KOPrefs::CategoryOnly);
    mAssignColor->setEnabled(enableAction);
    QModelIndex index = mCollectionView->selectionModel()->currentIndex(); //selectedRows()

    bool disableStuff = false;

    if (index.isValid()) {
        const Akonadi::Collection collection = CalendarSupport::collectionFromIndex(index);
        Q_ASSERT(collection.isValid());

        if (!collection.contentMimeTypes().isEmpty()) {
            const QString identifier = QString::number(collection.id());
            const QColor defaultColor = KOPrefs::instance()->resourceColor(identifier);
            enableAction = enableAction && defaultColor.isValid();
            mDisableColor->setEnabled(enableAction);
            mDefaultCalendar->setEnabled(!KOHelper::isStandardCalendar(collection.id()) &&
                                         collection.rights() & Akonadi::Collection::CanCreateItem);
        } else {
            disableStuff = true;
        }
    } else {
        disableStuff = true;
    }

    if (disableStuff) {
        mDisableColor->setEnabled(false);
        mDefaultCalendar->setEnabled(false);
        mAssignColor->setEnabled(false);
    }
}

void AkonadiCollectionView::newCalendar()
{
    Akonadi::AgentTypeDialog dlg(this);
    dlg.setWindowTitle(i18n("Add Calendar"));
    dlg.agentFilterProxyModel()->addMimeTypeFilter(QStringLiteral("text/calendar"));
    dlg.agentFilterProxyModel()->addCapabilityFilter(QLatin1String("Resource"));   // show only resources, no agents
    if (dlg.exec()) {
        mNotSendAddRemoveSignal = true;
        const Akonadi::AgentType agentType = dlg.agentType();
        if (agentType.isValid()) {
            Akonadi::AgentInstanceCreateJob *job = new Akonadi::AgentInstanceCreateJob(agentType, this);
            job->configure(this);
            connect(job, &Akonadi::AgentInstanceCreateJob::result, this, &AkonadiCollectionView::newCalendarDone);
            job->start();
        }
    }
}

void AkonadiCollectionView::newCalendarDone(KJob *job)
{
    Akonadi::AgentInstanceCreateJob *createjob = static_cast<Akonadi::AgentInstanceCreateJob *>(job);
    if (createjob->error()) {
        //TODO(AKONADI_PORT)
        // this should show an error dialog and should be merged
        // with the identical code in ActionManager
        qWarning() << "Create calendar failed:" << createjob->errorString();
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
                i18n("Do you really want to delete calendar %1?", displayname),
                i18n("Delete Calendar"),
                KStandardGuiItem::del(),
                KStandardGuiItem::cancel(),
                QString(),
                KMessageBox::Dangerous)  == KMessageBox::Continue) {

        bool isTopLevel = collection.parentCollection() == Akonadi::Collection::root();

        mNotSendAddRemoveSignal = true;
        mWasDefaultCalendar = KOHelper::isStandardCalendar(collection.id());

        if (!isTopLevel) {
            // deletes contents
            Akonadi::CollectionDeleteJob *job = new Akonadi::CollectionDeleteJob(collection, this);
            connect(job, &Akonadi::AgentInstanceCreateJob::result, this, &AkonadiCollectionView::deleteCalendarDone);
        } else {
            // deletes the agent, not the contents
            const Akonadi::AgentInstance instance =
                Akonadi::AgentManager::self()->instance(collection.resource());
            if (instance.isValid()) {
                Akonadi::AgentManager::self()->removeInstance(instance);
            }
        }
    }
}

void AkonadiCollectionView::deleteCalendarDone(KJob *job)
{
    Akonadi::CollectionDeleteJob *deletejob = static_cast<Akonadi::CollectionDeleteJob *>(job);
    if (deletejob->error()) {
        qWarning() << "Delete calendar failed:" << deletejob->errorString();
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
        emit resourcesAddedRemoved();
    }
    restoreTreeState();
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
        collection = indexes.first().data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
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
    QModelIndexList indexes = selectionModel->selectedIndexes();
    foreach (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            Akonadi::Collection collection = index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
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
    QModelIndexList indexes = selectionModel->selectedIndexes();
    foreach (const QModelIndex &index, indexes) {
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
    QAbstractProxyModel *proxy = qobject_cast<QAbstractProxyModel *>(mCollectionView->model());
    while (proxy) {
        Akonadi::EntityTreeModel *etm = qobject_cast<Akonadi::EntityTreeModel *>(proxy->sourceModel());
        if (etm) {
            return etm;
        }
        proxy = qobject_cast<QAbstractProxyModel *>(proxy->sourceModel());
    }

    qWarning() << "Couldn't find EntityTreeModel";
    return 0;
}

void AkonadiCollectionView::checkNewCalendar(const QModelIndex &parent, int begin, int end)
{
    // HACK: Check newly created calendars
    Akonadi::EntityTreeModel *etm = entityTreeModel();
    if (etm && entityTreeModel()->isCollectionTreeFetched()) {
        for (int row = begin; row <= end; ++row) {
            QModelIndex index = mCollectionView->model()->index(row, 0, parent);
            if (index.isValid()) {
                mCollectionView->model()->setData(index, Qt::Checked, Qt::CheckStateRole);
            }
        }
        if (parent.isValid()) {
            mCollectionView->setExpanded(parent, true);
        }
    }
}

