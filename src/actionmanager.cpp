/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2002 Mike Pilone <mpilone@slac.com>
  SPDX-FileCopyrightText: 2002 Don Sanders <sanders@kde.org>
  SPDX-FileCopyrightText: 2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  SPDX-FileCopyrightText: 2010-2025 Laurent Montel <montel@kde.org>
  SPDX-FileCopyrightText: 2004-2025 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH LicenseRef-Qt-Commercial-exception-1.0
*/
#include "actionmanager.h"
#include "akonadicollectionview.h"
#include "calendaradaptor.h"
#include "calendarinterfaceadaptor.h"
#include "calendarview.h"
#include "kocore.h"
#include "kodialogmanager.h"
#include "koglobals.h"
#include "korganizeradaptor.h"
#include "koviewmanager.h"
#include "kowindowlist.h"
#include "prefs/koprefs.h"
#include <KAuthorized>
#include <config-korganizer.h>

#include <CalendarSupport/CollectionSelection>
#include <CalendarSupport/EventArchiver>
#include <CalendarSupport/KCalPrefs>

#include <IncidenceEditor/IncidenceEditorSettings>

#include <Akonadi/CalendarUtils>
#include <Akonadi/ETMViewStateSaver>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/EntityTreeView>
#include <Akonadi/History>
#include <Akonadi/ICalImporter>

#include <KCalendarCore/FileStorage>
#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/Person>

#include <KIO/FileCopyJob>
#include <KIO/StatJob>
#include <KJobWidgets>
#include <KMime/Message>

#include <KActionCollection>
#include <KActionMenu>
#include <KColorSchemeMenu>
#include <KMessageBox>
#include <KSelectAction>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <QAction>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QProcess>

#include "korganizer_debug.h"
#include "korganizer_options.h"
#include <KToggleAction>
#include <KWindowSystem>
#include <QIcon>
#include <QTemporaryFile>

#include <KCheckableProxyModel>
#include <KColorSchemeManager>
#include <KSharedConfig>
#include <QApplication>
#include <QStandardPaths>
#include <QTimer>
#include <QToolBar>
#include <QWindow>

KOWindowList *ActionManager::mWindowList = nullptr;

ActionManager::ActionManager(KXMLGUIClient *client,
                             CalendarView *widget,
                             QObject *parent,
                             KOrg::MainWindow *mainWindow,
                             bool isPart,
                             QMenuBar *menuBar,
                             QToolBar *toolBar)
    : QObject(parent)
{
    new KOrgCalendarAdaptor(this);

    // reminder daemon interface
    new CalendarInterfaceAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Calendar"), this);

    mGUIClient = client;
    mACollection = mGUIClient->actionCollection();
    mCalendarView = widget;
    mIsPart = isPart;
    mMainWindow = mainWindow;
    mMenuBar = menuBar;
    mToolBar = toolBar;
}

ActionManager::~ActionManager()
{
    delete mTempFile;

    // Take this window out of the window list.
    mWindowList->removeWindow(mMainWindow);

    delete mCollectionSelectionModelStateSaver;
    delete mCollectionViewStateSaver;

    delete mCalendarView;
}

void ActionManager::toggleMenubar(bool dontShowWarning)
{
    if (mMenuBar) {
        if (mShowMenuBarAction->isChecked()) {
            mMenuBar->show();
        } else {
            if (!dontShowWarning && (!mToolBar->isVisible() || !mToolBar->actions().contains(mHamburgerMenu))) {
                const QString accel = mShowMenuBarAction->shortcut().toString(QKeySequence::NativeText);
                KMessageBox::information(mCalendarView,
                                         i18nc("@info",
                                               "This will hide the menu bar completely. "
                                               "You can show it again by typing %1.",
                                               accel),
                                         i18nc("@title:window", "Hide menu bar"),
                                         QStringLiteral("HideMenuBarWarning"));
            }
            mMenuBar->hide();
        }
        KOPrefs::instance()->setShowMenuBar(mShowMenuBarAction->isChecked());
    }
}

// see the Note: below for why this method is necessary
void ActionManager::init()
{
    // add this instance of the window to the static list.
    if (!mWindowList) {
        mWindowList = new KOWindowList;
    }

    // Note: We need this ActionManager to be fully constructed, and
    // parent() to have a valid reference to it before the following
    // addWindow is called.
    mWindowList->addWindow(mMainWindow);

    // initialize the QAction instances
    initActions();

    // set up autoSaving stuff
    mAutoArchiveTimer = new QTimer(this);
    mAutoArchiveTimer->setSingleShot(true);
    connect(mAutoArchiveTimer, &QTimer::timeout, this, &ActionManager::slotAutoArchive);

    // First auto-archive should be in 5 minutes (like in kmail).
    if (CalendarSupport::KCalPrefs::instance()->mAutoArchive) {
        mAutoArchiveTimer->start(5 * 60 * 1000); // singleshot
    }

    setTitle();

    connect(mCalendarView, &CalendarView::modifiedChanged, this, &ActionManager::setTitle);
    connect(mCalendarView, &CalendarView::configChanged, this, &ActionManager::updateConfig);

    connect(mCalendarView, &CalendarView::incidenceSelected, this, &ActionManager::processIncidenceSelection);

    processIncidenceSelection(Akonadi::Item(), QDate());

    // Update state of paste action
    mCalendarView->checkClipboard();
}

Akonadi::ETMCalendar::Ptr ActionManager::calendar() const
{
    return mCalendarView->calendar();
}

void ActionManager::createCalendarAkonadi()
{
    Q_ASSERT(calendar());

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    mCollectionSelectionModelStateSaver = new KViewStateMaintainer<Akonadi::ETMViewStateSaver>(config->group(QStringLiteral("GlobalCollectionSelection")));
    mCollectionSelectionModelStateSaver->setSelectionModel(calendar()->checkableProxyModel()->selectionModel());

    AkonadiCollectionViewFactory factory(mCalendarView);
    mCalendarView->addExtension(&factory);
    mCollectionView = factory.collectionView();
    mCollectionView->setObjectName(QLatin1StringView("Resource View"));
    connect(mCollectionView, &AkonadiCollectionView::resourcesAddedRemoved, this, &ActionManager::slotResourcesAddedRemoved);

    connect(mCollectionView, &AkonadiCollectionView::defaultResourceChanged, this, &ActionManager::slotDefaultResourceChanged);

    connect(mCollectionView, &AkonadiCollectionView::colorsChanged, mCalendarView, qOverload<>(&CalendarView::updateConfig));

    mCollectionViewStateSaver = new KViewStateMaintainer<Akonadi::ETMViewStateSaver>(config->group(QStringLiteral("GlobalCollectionView")));
    mCollectionViewStateSaver->setView(mCollectionView->view());

    KCheckableProxyModel *checkableProxy = calendar()->checkableProxyModel();
    QItemSelectionModel *selectionModel = checkableProxy->selectionModel();

    mCollectionView->setCollectionSelectionProxyModel(checkableProxy);

    auto collectionSelection = new CalendarSupport::CollectionSelection(selectionModel);
    EventViews::EventView::setGlobalCollectionSelection(collectionSelection);

    mCalendarView->readSettings();

    // connect(calendar().data(), &Akonadi::ETMCalendar::calendarChanged, mCalendarView, &CalendarView::resourcesChanged);
    connect(calendar().data(), &Akonadi::ETMCalendar::calendarSelectionEdited, this, &ActionManager::writeSettings);
    connect(mCalendarView, &CalendarView::configChanged, this, &ActionManager::updateConfig);

    calendar()->setOwner(KCalendarCore::Person(CalendarSupport::KCalPrefs::instance()->fullName(), CalendarSupport::KCalPrefs::instance()->email()));
}

void ActionManager::initActions()
{
    /*************************** FILE MENU **********************************/

    //~~~~~~~~~~~~~~~~~~~~~~~ LOADING / SAVING ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (mIsPart) {
        if (mMainWindow->hasDocument()) {
            mACollection->addAction(KStandardActions::Open, QStringLiteral("korganizer_open"), this, qOverload<>(&ActionManager::file_open));
        }
        mACollection->addAction(KStandardActions::Print, QStringLiteral("korganizer_print"), mCalendarView, &CalendarView::print);
        mACollection->addAction(KStandardActions::PrintPreview, QStringLiteral("korganizer_print_preview"), mCalendarView, &CalendarView::printPreview);
    } else {
        KStandardActions::open(this, qOverload<>(&ActionManager::file_open), mACollection);
        KStandardActions::print(mCalendarView, &CalendarView::print, mACollection);
        KStandardActions::printPreview(mCalendarView, &CalendarView::printPreview, mACollection);
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~ IMPORT / EXPORT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /** Import Action **/
    // TODO: Icon
    mImportAction = new QAction(i18nc("@action:inmenu", "Import &Calendar…"), this);
    mImportAction->setStatusTip(i18nc("@info:status", "Import a calendar"));
    mImportAction->setToolTip(i18nc("@info:tooltip", "Import an iCalendar or vCalendar file"));
    mImportAction->setWhatsThis(i18nc("@info:whatsthis",
                                      "Select this menu entry if you would like to import the contents of an "
                                      "iCalendar or vCalendar file into your current calendar collection."));
    mACollection->addAction(QStringLiteral("import_icalendar"), mImportAction);
    connect(mImportAction, &QAction::triggered, this, &ActionManager::file_import);

    QAction *action = nullptr;
    /** Export Action **/
    // TODO: Icon
    action = new QAction(i18nc("@action:inmenu", "Export as &iCalendar…"), this);
    action->setStatusTip(i18nc("@info:status", "Export calendar to file"));
    action->setToolTip(i18nc("@info:tooltip", "Export your calendar to an iCalendar file"));
    action->setWhatsThis(i18nc("@info:whatsthis", "Allows you to export your entire calendar collection to one iCalendar file."));
    mACollection->addAction(QStringLiteral("export_icalendar"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::exportICalendar);

    /** Archive Action **/
    // TODO: Icon
    action = new QAction(i18nc("@action:inmenu", "Archive O&ld Incidences…"), this);
    action->setStatusTip(i18nc("@info:status", "Archive events and to-dos to a file"));
    action->setToolTip(i18nc("@info:tooltip", "Archive old events and to-dos to an iCalendar file"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "This menu entry opens a dialog that allows you to select old events and to-dos "
                               "that you can archive into an iCalendar file.  They will "
                               "be removed from your existing calendar collection."));
    mACollection->addAction(QStringLiteral("file_archive"), action);
    connect(action, &QAction::triggered, this, &ActionManager::file_archive);

    /** Purge Todos Action **/
    // TODO: Icon
    action = new QAction(i18nc("@action:inmenu", "Pur&ge Completed To-dos"), mACollection);
    action->setStatusTip(i18nc("@info:status", "Purge completed to-dos"));
    action->setToolTip(i18nc("@info:tooltip", "Remove completed to-dos from your calendar"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Allows you to remove all completed to-dos from your calendar collection. "
                               "This action cannot be undone!"));
    mACollection->addAction(QStringLiteral("purge_completed"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::purgeCompleted);

    /************************** EDIT MENU *********************************/

    QAction *pasteAction = nullptr;
    Akonadi::History *history = mCalendarView->history();
    if (mIsPart) {
        // edit menu
        mCutAction = mACollection->addAction(KStandardActions::Cut, QStringLiteral("korganizer_cut"), mCalendarView, &CalendarView::edit_cut);
        mCopyAction = mACollection->addAction(KStandardActions::Copy, QStringLiteral("korganizer_copy"), mCalendarView, &CalendarView::edit_copy);
        pasteAction = mACollection->addAction(KStandardActions::Paste, QStringLiteral("korganizer_paste"), mCalendarView, &CalendarView::edit_paste);
        mUndoAction = mACollection->addAction(KStandardActions::Undo, QStringLiteral("korganizer_undo"), this, [history]() {
            history->undo();
        });
        mRedoAction = mACollection->addAction(KStandardActions::Redo, QStringLiteral("korganizer_redo"), this, [history]() {
            history->redo();
        });
    } else {
        mCutAction = KStandardActions::cut(mCalendarView, &CalendarView::edit_cut, mACollection);
        mCopyAction = KStandardActions::copy(mCalendarView, &CalendarView::edit_copy, mACollection);
        pasteAction = KStandardActions::paste(mCalendarView, &CalendarView::edit_paste, mACollection);
        mUndoAction = KStandardActions::undo(
            this,
            [history]() {
                history->undo();
            },
            mACollection);
        mRedoAction = KStandardActions::redo(
            this,
            [history]() {
                history->redo();
            },
            mACollection);
    }
    mDeleteAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18nc("@action:inmenu", "&Delete"), this);
    mACollection->addAction(QStringLiteral("edit_delete"), mDeleteAction);
    connect(mDeleteAction, &QAction::triggered, mCalendarView, &CalendarView::appointment_delete);
    if (mIsPart) {
        mACollection->addAction(KStandardActions::Find, QStringLiteral("korganizer_find"), mCalendarView->dialogManager(), &KODialogManager::showSearchDialog);
    } else {
        KStandardActions::find(mCalendarView->dialogManager(), &KODialogManager::showSearchDialog, mACollection);
    }
    pasteAction->setEnabled(false);
    mUndoAction->setEnabled(false);
    mRedoAction->setEnabled(false);
    connect(mCalendarView, &CalendarView::pasteEnabled, pasteAction, &QAction::setEnabled);
    connect(history, &Akonadi::History::changed, this, &ActionManager::updateUndoRedoActions);

    /************************** VIEW MENU *********************************/

    /** What's Next View Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-upcoming-events")), i18nc("@action:inmenu", "What's &Next"), this);
    action->setStatusTip(i18nc("@info:status", "What's Next View"));
    action->setToolTip(i18nc("@info:tooltip", "Switch to the What's Next View"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Switches to the \"What's Next\" View, which shows your events and to-dos "
                               "that are \"coming soon\" in a short list for quick reading.  All open to-dos "
                               "will be displayed, but only the events from the days selected in the "
                               "Date Navigator sidebar will be shown."));
    mACollection->addAction(QStringLiteral("view_whatsnext"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::showWhatsNextView);

    /** Month View Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-month")), i18nc("@action:inmenu", "&Month"), this);
    action->setStatusTip(i18nc("@info:status", "Month View"));
    action->setToolTip(i18nc("@info:tooltip", "Switch to the Month View"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Switches to the Month View, which shows all the events and due to-dos "
                               "in a familiar monthly calendar layout."));
    mACollection->addAction(QStringLiteral("view_month"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::showMonthView);

    /** Agenda View Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-agenda")), i18nc("@action:inmenu", "&Agenda"), this);
    auto agendaMenu = new QMenu(nullptr);
    action->setMenu(agendaMenu);
    action->setStatusTip(i18nc("@info:status", "Agenda View"));
    action->setToolTip(i18nc("@info:tooltip", "Switch to the Agenda View"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Switches to the Agenda View, which presents your events or due to-dos "
                               "for one or more days, sorted chronologically.  You can also see the length "
                               "of each event in the day timetable."));
    mACollection->addAction(QStringLiteral("view_agenda"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::showAgendaView);

    /** List View Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-list")), i18nc("@action:inmenu", "&Event List"), this);
    action->setStatusTip(i18nc("@info:status", "List View"));
    action->setToolTip(i18nc("@info:tooltip", "Switch to the List View"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Switches to the List View, which displays all your to-dos, events and "
                               "journal entries for the dates selected in the Date Navigator as a list."));
    mACollection->addAction(QStringLiteral("view_list"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::showListView);

    /** Todo View Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-tasks")), i18nc("@action:inmenu", "&To-do List"), this);
    action->setStatusTip(i18nc("@info:status", "To-do List View"));
    action->setToolTip(i18nc("@info:tooltip", "Switch to the To-do List View"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Switches to the To-do List view, which provides a place for you to "
                               "track tasks that need to be done."));
    mACollection->addAction(QStringLiteral("view_todo"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::showTodoView);

    /** Journal View Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-journal")), i18nc("@action:inmenu", "&Journal"), this);
    action->setStatusTip(i18nc("@info:status", "Journal View"));
    action->setToolTip(i18nc("@info:tooltip", "Switch to the Journal View"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Switches to the Journal View, which provides a place for you to record "
                               "your reflections, occurrences or experiences."));
    mACollection->addAction(QStringLiteral("view_journal"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::showJournalView);

    /** Timeline View Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-timeline")), i18nc("@action:inmenu", "Time&line"), this);
    action->setStatusTip(i18nc("@info:status", "Timeline View"));
    action->setToolTip(i18nc("@info:tooltip", "Switch to the Timeline View"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Switches to the Timeline View, which shows all events for the selected "
                               "timespan in a Gantt view. Each calendar is displayed in a separate line."));
    mACollection->addAction(QStringLiteral("view_timeline"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::showTimeLineView);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~ REFRESH ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    /** Refresh Action **/
    // TODO: icon
    action = new QAction(i18nc("@action:inmenu", "&Refresh"), this);
    action->setStatusTip(i18nc("@info:status", "Refresh"));
    action->setToolTip(i18nc("@info:tooltip", "Refresh the display"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "This action will refresh and redraw the current calendar view. "
                               "It does not sync or update any calendar folders."));
    mACollection->addAction(QStringLiteral("update"), action);
    connect(action, &QAction::triggered, mCalendarView, qOverload<>(&CalendarView::updateView));

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~ FILTER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    /** Filter Action **/
    // TODO: icon
    mFilterAction = new KSelectAction(i18nc("@action:inmenu", "F&ilter"), this);
    mFilterAction->setStatusTip(i18nc("@info:status", "Filter incidences"));
    mFilterAction->setToolTip(i18nc("@info:tooltip", "Filter incidences from the calendar"));
    mFilterAction->setWhatsThis(i18nc("@info:whatsthis",
                                      "Runs user-defined view filters on the calendar collection. Filters must be "
                                      "created first. See \"Manage View Filters\" option in the Settings menu."));
    mFilterAction->setToolBarMode(KSelectAction::MenuMode);
    mACollection->addAction(QStringLiteral("filter_select"), mFilterAction);
    mFilterAction->setEditable(false);

    connect(mFilterAction, &KSelectAction::indexTriggered, mCalendarView, &CalendarView::filterActivated);
    connect(mCalendarView, &CalendarView::filtersUpdated, this, &ActionManager::setItems);
    connect(mCalendarView, &CalendarView::filterChanged, this, &ActionManager::setTitle);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ZOOM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // TODO: try to find / create better icons for the following 4 actions
    action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in")), i18nc("@action:inmenu", "In Horizontally"), this);
    action->setEnabled(mCalendarView->currentView()->supportsZoom());
    mACollection->addAction(QStringLiteral("zoom_in_horizontally"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::zoomInHorizontally);

    action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), i18nc("@action:inmenu", "Out Horizontally"), this);
    action->setEnabled(mCalendarView->currentView()->supportsZoom());
    mACollection->addAction(QStringLiteral("zoom_out_horizontally"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::zoomOutHorizontally);

    action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in")), i18nc("@action:inmenu", "In Vertically"), this);
    action->setEnabled(mCalendarView->currentView()->supportsZoom());
    mACollection->addAction(QStringLiteral("zoom_in_vertically"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::zoomInVertically);

    action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), i18nc("@action:inmenu", "Out Vertically"), this);
    action->setEnabled(mCalendarView->currentView()->supportsZoom());
    mACollection->addAction(QStringLiteral("zoom_out_vertically"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::zoomOutVertically);

    /************************** Actions MENU *********************************/
    bool const isRTL = QApplication::isRightToLeft();

    /** Scroll to Today Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("go-jump-today")), i18nc("@action:inmenu Jump to today", "To &Today"), this);
    action->setIconText(i18nc("@action:button", "Today"));
    action->setStatusTip(i18nc("@info:status", "Scroll to Today"));
    action->setToolTip(i18nc("@info:tooltip", "Scroll the view to today"));
    action->setWhatsThis(i18nc("@info:whatsthis", "Scrolls the current view to the today's date."));
    mACollection->addAction(QStringLiteral("go_today"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::goToday);

    /** Scroll Backward Action **/
    action = new QAction(QIcon::fromTheme(isRTL ? QStringLiteral("go-next") : QStringLiteral("go-previous")),
                         i18nc("@action:inmenu scroll backward", "&Backward"),
                         this);
    action->setIconText(i18nc("@action:button scroll backward", "Back"));
    action->setStatusTip(i18nc("@info:status", "Scroll Backward"));
    action->setToolTip(i18nc("@info:tooltip", "Scroll the view backward"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Scrolls backward by a day, week, month or year, depending on the "
                               "current calendar view."));
    mACollection->addAction(QStringLiteral("go_previous"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::goPrevious);

    /** Scroll Forward Action **/
    action = new QAction(QIcon::fromTheme(isRTL ? QStringLiteral("go-previous") : QStringLiteral("go-next")),
                         i18nc("@action:inmenu scroll forward", "&Forward"),
                         this);
    action->setIconText(i18nc("@action:button scroll forward", "Forward"));
    action->setStatusTip(i18nc("@info:status", "Scroll Forward"));
    action->setToolTip(i18nc("@info:tooltip", "Scroll the view forward"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Scrolls forward by a day, week, month or year, depending on the "
                               "current calendar view."));
    mACollection->addAction(QStringLiteral("go_next"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::goNext);

    /** Agenda: Day View **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-day")), i18nc("@action:inmenu", "&Day"), this);
    action->setText(i18nc("@action:inmenu agenda view show 1 day at a time", "Day"));
    action->setStatusTip(i18nc("@info:status", "View 1 day at a time"));
    action->setToolTip(i18nc("@info:tooltip", "Show 1 day at a time in Agenda View"));
    action->setWhatsThis(i18nc("@info:whatsthis", "Select this option to show only 1 day at a time in Agenda View."));
    agendaMenu->addAction(action);
    mACollection->addAction(QStringLiteral("select_day"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::selectDay);

    /** Agenda: Next X Days View **/
    mNextXDays = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-upcoming-days")), QString(), this);
    mNextXDays->setText(i18nc("@action:inmenu agenda view show a few days at a time", "&Next Days"));
    mNextXDays->setStatusTip(i18nc("@info:status", "View a few days at a time"));
    mNextXDays->setToolTip(i18nc("@info:tooltip", "Show the next few days at a time in Agenda View"));
    mNextXDays->setWhatsThis(
        xi18nc("@info:whatsthis",
               "Select this option to show the next few days at a time in Agenda View. Configure the <placeholder>Next X Days</placeholder> setting in the "
               "<interface>Views->General</interface> settings page to adjust the number of days shown."));
    agendaMenu->addAction(mNextXDays);
    mACollection->addAction(QStringLiteral("select_nextx"), mNextXDays);
    connect(mNextXDays, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::selectNextX);

    /** Agenda: Work Week View **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-workweek")), i18nc("@action:inmenu", "W&eek Schedule"), this);
    action->setText(i18nc("@action:inmenu agenda view show the weekly schedule days only (eg. no weekends)", "Week Schedule"));
    action->setStatusTip(i18nc("@info:status", "Show a week at a time showing days included in your weekly schedule only"));
    action->setToolTip(i18nc("@info:tooltip",
                             "Show a week at a time in Agenda View, excluding days that are not included in your weekly schedule (ie. weekends are excluded)"));
    action->setWhatsThis(xi18nc(
        "@info:whatsthis",
        "Select this option to show a week at a time, excluding the days that are not included in your weekly schedule. Configure your schedule by selecting "
        "days in the "
        "<placeholder>Weekly Schedule</placeholder> setting in the <interface>Time and Date->Regional</interface> page.<para>Typically this view is intended "
        "for "
        "showing the week without weekend days.</para>"));
    agendaMenu->addAction(action);
    mACollection->addAction(QStringLiteral("select_workweek"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::selectWorkWeek);

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-week")), i18nc("@action:inmenu", "&Week"), this);
    action->setText(i18nc("@action:inmenu agenda view show a week at a time", "Week"));
    action->setStatusTip(i18nc("@info:status", "View a week at a time"));
    action->setToolTip(i18nc("@info:tooltip", "Show 1 week at a time in Agenda View"));
    action->setWhatsThis(i18nc("@info:whatsthis", "Select this option to show a week at a time in Agenda View."));
    agendaMenu->addAction(action);
    mACollection->addAction(QStringLiteral("select_week"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::selectWeek);

    /************************** Actions MENU *********************************/
    /** New Event Action **/
    mNewEventAction = new QAction(QIcon::fromTheme(QStringLiteral("appointment-new")), i18nc("@action:inmenu", "New E&vent…"), this);
    mNewEventAction->setStatusTip(i18nc("@info:status", "Create a new Event"));
    mNewEventAction->setToolTip(i18nc("@info:tooltip", "Create a new Event"));
    mNewEventAction->setWhatsThis(i18nc("@info:whatsthis",
                                        "Starts a dialog that allows you to create a new Event with reminders, "
                                        "attendees, recurrences and much more."));
    mACollection->addAction(QStringLiteral("new_event"), mNewEventAction);
    connect(mNewEventAction, &QAction::triggered, this, &ActionManager::slotNewEvent);

    /** New To-do Action **/
    mNewTodoAction = new QAction(QIcon::fromTheme(QStringLiteral("task-new")), i18nc("@action:inmenu", "New &To-do…"), this);
    mNewTodoAction->setStatusTip(i18nc("@info:status", "Create a new To-do"));
    mNewTodoAction->setToolTip(i18nc("@info:tooltip", "Create a new To-do"));
    mNewTodoAction->setWhatsThis(i18nc("@info:whatsthis",
                                       "Starts a dialog that allows you to create a new To-do with reminders, "
                                       "attendees, recurrences and much more."));
    mACollection->addAction(QStringLiteral("new_todo"), mNewTodoAction);
    connect(mNewTodoAction, &QAction::triggered, this, &ActionManager::slotNewTodo);

    /** New Sub-To-do Action **/
    // TODO: icon
    mNewSubtodoAction = new QAction(i18nc("@action:inmenu", "New Su&b-to-do…"), this);
    // TODO: statustip, tooltip, whatsthis
    mACollection->addAction(QStringLiteral("new_subtodo"), mNewSubtodoAction);
    connect(mNewSubtodoAction, &QAction::triggered, this, &ActionManager::slotNewSubTodo);
    mNewSubtodoAction->setEnabled(false);
    connect(mCalendarView, &CalendarView::todoSelected, mNewSubtodoAction, &QAction::setEnabled);

    /** New Journal Action **/
    mNewJournalAction = new QAction(QIcon::fromTheme(QStringLiteral("journal-new")), i18nc("@action:inmenu", "New &Journal…"), this);
    mNewJournalAction->setStatusTip(i18nc("@info:status", "Create a new Journal"));
    mNewJournalAction->setToolTip(i18nc("@info:tooltip", "Create a new Journal"));
    mNewJournalAction->setWhatsThis(i18nc("@info:whatsthis", "Starts a dialog that allows you to create a new Journal entry."));
    mACollection->addAction(QStringLiteral("new_journal"), mNewJournalAction);
    connect(mNewJournalAction, &QAction::triggered, this, &ActionManager::slotNewJournal);

    /** Scroll to Date Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("go-jump")), i18nc("@action:inmenu Jump to date", "&Pick a Date"), this);
    action->setIconText(i18nc("@action:button", "Date"));
    action->setStatusTip(i18nc("@info:status", "Scroll the view to user selected dates"));
    action->setToolTip(i18nc("@info:tooltip", "Scroll the view to user selected dates"));
    action->setWhatsThis(i18nc("@info:whatsthis", "Opens a date selection dialog for quickly navigating the view."));
    mACollection->addAction(QStringLiteral("pick_date"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::goSelectADate);

    /** Configure Current View Action **/
    mConfigureViewAction = new QAction(QIcon::fromTheme(QStringLiteral("configure")), i18nc("@action:inmenu", "Configure View…"), this);
    mConfigureViewAction->setIconText(i18nc("@action:button", "Configure"));
    mConfigureViewAction->setStatusTip(i18nc("@info:status", "Configure the multi-calendar view"));
    mConfigureViewAction->setToolTip(i18nc("@info:tooltip", "Configure the multi-calendar view"));
    mConfigureViewAction->setWhatsThis(i18nc("@info:whatsthis",
                                             "In the multi-calendar view, starts a dialog that allows configuring the view. "
                                             "Currently only the multi-calendar view is supported. "
                                             "See the Multiple Calendars options in the Views->Agenda Views settings."));
    mConfigureViewAction->setEnabled(mCalendarView->currentView() && mCalendarView->currentView()->hasConfigurationDialog());
    mACollection->addAction(QStringLiteral("configure_view"), mConfigureViewAction);
    connect(mConfigureViewAction, &QAction::triggered, mCalendarView, &CalendarView::configureCurrentView);

    mShowIncidenceAction = new QAction(QIcon::fromTheme(QStringLiteral("document-preview")), i18nc("@action:inmenu", "&Show"), this);
    mACollection->addAction(QStringLiteral("show_incidence"), mShowIncidenceAction);
    connect(mShowIncidenceAction, &QAction::triggered, mCalendarView, qOverload<>(&CalendarView::showIncidence));

    mEditIncidenceAction = new QAction(QIcon::fromTheme(QStringLiteral("document-edit")), i18nc("@action:inmenu", "&Edit…"), this);
    mACollection->addAction(QStringLiteral("edit_incidence"), mEditIncidenceAction);
    connect(mEditIncidenceAction, &QAction::triggered, mCalendarView, qOverload<>(&CalendarView::editIncidence));

    mDeleteIncidenceAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18nc("@action:inmenu", "&Delete"), this);
    mACollection->addAction(QStringLiteral("delete_incidence"), mDeleteIncidenceAction);
    connect(mDeleteIncidenceAction, &QAction::triggered, mCalendarView, qOverload<>(&CalendarView::deleteIncidence));
    mACollection->setDefaultShortcut(mDeleteIncidenceAction, QKeySequence(Qt::Key_Delete));

    action = new QAction(i18nc("@action:inmenu", "&Make Sub-to-do Independent"), this);
    mACollection->addAction(QStringLiteral("unsub_todo"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::todo_unsub);
    action->setEnabled(false);
    connect(mCalendarView, &CalendarView::subtodoSelected, action, &QAction::setEnabled);

    // TODO: Add item to quickly toggle the reminder of a given incidence
    //   mToggleAlarmAction = new KToggleAction( i18n( "&Activate Reminder" ), 0,
    //                                         mCalendarView, SLOT(toggleAlarm()),
    //                                         mACollection, "activate_alarm" );

    /************************** SCHEDULE MENU ********************************/
    mPublishEvent = new QAction(QIcon::fromTheme(QStringLiteral("mail-send")), i18nc("@action:inmenu", "&Publish Item Information…"), this);
    mACollection->addAction(QStringLiteral("schedule_publish"), mPublishEvent);
    connect(mPublishEvent, &QAction::triggered, this, [this](bool) {
        mCalendarView->schedule_publish();
    });
    mPublishEvent->setEnabled(false);

    mSendInvitation = new QAction(QIcon::fromTheme(QStringLiteral("mail-send")), i18nc("@action:inmenu", "Send &Invitation to Attendees"), this);
    mACollection->addAction(QStringLiteral("schedule_request"), mSendInvitation);
    connect(mSendInvitation, &QAction::triggered, this, [this](bool) {
        mCalendarView->schedule_request();
    });
    mSendInvitation->setEnabled(false);
    connect(mCalendarView, &CalendarView::organizerEventsSelected, mSendInvitation, &QAction::setEnabled);

    mRequestUpdate = new QAction(i18nc("@action:inmenu", "Re&quest Update"), this);
    mACollection->addAction(QStringLiteral("schedule_refresh"), mRequestUpdate);
    connect(mRequestUpdate, &QAction::triggered, this, [this](bool) {
        mCalendarView->schedule_refresh();
    });
    mRequestUpdate->setEnabled(false);
    connect(mCalendarView, &CalendarView::groupEventsSelected, mRequestUpdate, &QAction::setEnabled);

    mSendCancel = new QAction(i18nc("@action:inmenu", "Send &Cancellation to Attendees"), this);
    mACollection->addAction(QStringLiteral("schedule_cancel"), mSendCancel);
    connect(mSendCancel, &QAction::triggered, this, [this](bool) {
        mCalendarView->schedule_cancel();
    });
    mSendCancel->setEnabled(false);
    connect(mCalendarView, &CalendarView::organizerEventsSelected, mSendCancel, &QAction::setEnabled);

    mSendStatusUpdate = new QAction(QIcon::fromTheme(QStringLiteral("mail-reply-sender")), i18nc("@action:inmenu", "Send Status &Update"), this);
    mACollection->addAction(QStringLiteral("schedule_reply"), mSendStatusUpdate);
    connect(mSendStatusUpdate, &QAction::triggered, this, [this](bool) {
        mCalendarView->schedule_reply();
    });
    mSendStatusUpdate->setEnabled(false);
    connect(mCalendarView, &CalendarView::groupEventsSelected, mSendStatusUpdate, &QAction::setEnabled);

    mRequestChange = new QAction(i18nc("@action:inmenu counter proposal", "Request Chan&ge"), this);
    mACollection->addAction(QStringLiteral("schedule_counter"), mRequestChange);
    connect(mRequestChange, &QAction::triggered, this, [this](bool) {
        mCalendarView->schedule_counter();
    });
    mRequestChange->setEnabled(false);
    connect(mCalendarView, &CalendarView::groupEventsSelected, mRequestChange, &QAction::setEnabled);

    action = new QAction(i18nc("@action:inmenu", "&Mail Free Busy Information…"), this);
    mACollection->addAction(QStringLiteral("mail_freebusy"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::mailFreeBusy);
    action->setEnabled(true);

    mForwardEvent = new QAction(QIcon::fromTheme(QStringLiteral("mail-forward")), i18nc("@action:inmenu", "&Send as iCalendar…"), this);
    mACollection->addAction(QStringLiteral("schedule_forward"), mForwardEvent);
    connect(mForwardEvent, &QAction::triggered, this, [this](bool) {
        mCalendarView->schedule_forward();
    });
    mForwardEvent->setEnabled(false);

    action = new QAction(i18nc("@action:inmenu", "&Upload Free Busy Information"), this);
    mACollection->addAction(QStringLiteral("upload_freebusy"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::uploadFreeBusy);
    action->setEnabled(true);

    if (!mIsPart) {
        action = new QAction(QIcon::fromTheme(QStringLiteral("help-contents")), i18nc("@action:inmenu", "&Address Book"), this);
        mACollection->addAction(QStringLiteral("addressbook"), action);
        connect(action, &QAction::triggered, mCalendarView, &CalendarView::openAddressbook);
    }

    /************************** SETTINGS MENU ********************************/

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SIDEBAR ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    mDateNavigatorShowAction = new KToggleAction(i18nc("@action:inmenu", "Show Date Navigator"), this);
    mACollection->addAction(QStringLiteral("show_datenavigator"), mDateNavigatorShowAction);
    connect(mDateNavigatorShowAction, &KToggleAction::triggered, this, &ActionManager::toggleDateNavigator);

    mTodoViewShowAction = new KToggleAction(i18nc("@action:inmenu", "Show To-do View"), this);
    mACollection->addAction(QStringLiteral("show_todoview"), mTodoViewShowAction);
    connect(mTodoViewShowAction, &KToggleAction::triggered, this, &ActionManager::toggleTodoView);

    mEventViewerShowAction = new KToggleAction(i18nc("@action:inmenu", "Show Item Viewer"), this);
    mACollection->addAction(QStringLiteral("show_eventviewer"), mEventViewerShowAction);
    connect(mEventViewerShowAction, &KToggleAction::triggered, this, &ActionManager::toggleEventViewer);

    KConfigGroup const config(KSharedConfig::openConfig(), QStringLiteral("Settings"));
    mDateNavigatorShowAction->setChecked(config.readEntry("DateNavigatorVisible", true));
    // if we are a kpart, then let's not show the todo in the left pane by
    // default since there's also a Todo part and we'll assume they'll be
    // using that as well, so let's not duplicate it (by default) here
    mTodoViewShowAction->setChecked(config.readEntry("TodoViewVisible", false)); // mIsPart ? false : true ) );
    mEventViewerShowAction->setChecked(config.readEntry("EventViewerVisible", true));
    toggleDateNavigator();
    toggleTodoView();
    toggleEventViewer();

    if (!mMainWindow->hasDocument()) {
        mCollectionViewShowAction = new KToggleAction(i18nc("@action:inmenu", "Show Calendar Manager"), this);
        mACollection->addAction(QStringLiteral("show_resourceview"), mCollectionViewShowAction);
        connect(mCollectionViewShowAction, &KToggleAction::triggered, this, &ActionManager::toggleResourceView);
        mCollectionViewShowAction->setChecked(config.readEntry("ResourceViewVisible", true));

        toggleResourceView();
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SIDEBAR ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    mShowMenuBarAction = KStandardAction::showMenubar(this, &ActionManager::toggleMenubar, mACollection);
    mShowMenuBarAction->setChecked(KOPrefs::instance()->showMenuBar());
    toggleMenubar(true);

#if !defined(Q_OS_WIN) && !defined(Q_OS_MACOS)
    action = new QAction(i18nc("@action:inmenu", "Configure &Date && Time…"), this);
    mACollection->addAction(QStringLiteral("conf_datetime"), action);
    connect(action, &QAction::triggered, this, &ActionManager::configureDateTime);
#endif
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-filter")), i18nc("@action:inmenu", "Manage View &Filters…"), this);
    mACollection->addAction(QStringLiteral("edit_filters"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::editFilters);

    action = new QAction(i18nc("@action:inmenu", "Manage T&ags…"), this);
    mACollection->addAction(QStringLiteral("edit_categories"), action);
    connect(action, &QAction::triggered, mCalendarView->dialogManager(), &KODialogManager::showCategoryEditDialog);

    auto manager = KColorSchemeManager::instance();
    mACollection->addAction(QStringLiteral("colorscheme_menu"), KColorSchemeMenu::createMenu(manager, this));

    action = new QAction(QIcon::fromTheme(QStringLiteral("korganizer")), i18nc("@action:inmenu", "What's new"), this);
    mACollection->addAction(QStringLiteral("whatsnew"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::slotWhatsNew);

    if (mIsPart) {
        action = new QAction(QIcon::fromTheme(QStringLiteral("configure")), i18nc("@action:inmenu", "&Configure KOrganizer…"), this);
        mACollection->addAction(QStringLiteral("korganizer_configure"), action);
        connect(action, &QAction::triggered, mCalendarView, &CalendarView::edit_options);
        mACollection->addAction(KStandardActions::KeyBindings, QStringLiteral("korganizer_configure_shortcuts"), this, &ActionManager::keyBindings);
    } else {
        KStandardActions::preferences(mCalendarView, &CalendarView::edit_options, mACollection);
        KStandardActions::keyBindings(this, &ActionManager::keyBindings, mACollection);
    }
    if (mMenuBar) {
        mHamburgerMenu = KStandardAction::hamburgerMenu(nullptr, nullptr, mACollection);
        mHamburgerMenu->setShowMenuBarAction(mShowMenuBarAction);
        mHamburgerMenu->setMenuBar(mMenuBar);
        connect(mHamburgerMenu, &KHamburgerMenu::aboutToShowMenu, this, [this]() {
            updateHamburgerMenu();
            // Immediately disconnect. We only need to run this once, but on demand.
            // NOTE: The nullptr at the end disconnects all connections between
            // q and mHamburgerMenu's aboutToShowMenu signal.
            disconnect(mHamburgerMenu, &KHamburgerMenu::aboutToShowMenu, this, nullptr);
        });
    }
}

void ActionManager::updateHamburgerMenu()
{
    delete mHamburgerMenu->menu();
    auto menu = new QMenu;

    menu->addAction(mACollection->action(QStringLiteral("conf_datetime")));
    menu->addSeparator();
    menu->addAction(mACollection->action(KStandardActions::name(KStandardActions::Print)));
    menu->addSeparator();
    menu->addAction(mACollection->action(KStandardActions::name(KStandardActions::Quit)));
    mHamburgerMenu->setMenu(menu);
}

void ActionManager::setItems(const QStringList &lst, int idx)
{
    mFilterAction->setItems(lst);
    mFilterAction->setCurrentItem(idx);
}

void ActionManager::slotResourcesAddedRemoved()
{
    restoreCollectionViewSetting();
}

void ActionManager::slotDefaultResourceChanged(const Akonadi::Collection &collection)
{
    mCalendarView->incidenceChanger()->setDefaultCollection(collection);
}

void ActionManager::slotNewEvent()
{
    mCalendarView->newEvent();
}

void ActionManager::slotNewTodo()
{
    mCalendarView->newTodo(selectedCollection());
}

void ActionManager::slotNewSubTodo()
{
    mCalendarView->newSubTodo(selectedCollection());
}

void ActionManager::slotNewJournal()
{
    mCalendarView->newJournal(selectedCollection());
}

void ActionManager::slotMergeFinished(bool success, int total)
{
    Q_ASSERT(sender());
    mImportAction->setEnabled(true);
    auto importer = qobject_cast<Akonadi::ICalImporter *>(sender());

    if (success) {
        mCalendarView->showMessage(i18ncp("@info", "1 incidence was imported successfully.", "%1 incidences were imported successfully.", total),
                                   KMessageWidget::Information);
    } else {
        mCalendarView->showMessage(xi18nc("@info", "There was an error while merging the calendar: <message>%1</message>", importer->errorMessage()),
                                   KMessageWidget::Error);
    }
    sender()->deleteLater();
}

void ActionManager::slotNewResourceFinished(bool success)
{
    Q_ASSERT(sender());
    auto importer = qobject_cast<Akonadi::ICalImporter *>(sender());
    mImportAction->setEnabled(true);
    if (success) {
        mCalendarView->showMessage(i18nc("@info", "New calendar added successfully"), KMessageWidget::Information);
    } else {
        mCalendarView->showMessage(xi18nc("@info", "Could not add a calendar. Error: <message>%1</message>", importer->errorMessage()), KMessageWidget::Error);
    }
    sender()->deleteLater();
}

void ActionManager::readSettings()
{
    // read settings from the KConfig, supplying reasonable
    // defaults where none are to be found

    mCalendarView->readSettings();
    restoreCollectionViewSetting();
}

void ActionManager::restoreCollectionViewSetting()
{
    mCollectionSelectionModelStateSaver->restoreState();
    mCollectionViewStateSaver->restoreState();
}

void ActionManager::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(QStringLiteral("Settings"));
    mCalendarView->writeSettings();

    if (mDateNavigatorShowAction) {
        group.writeEntry("DateNavigatorVisible", mDateNavigatorShowAction->isChecked());
    }

    if (mTodoViewShowAction) {
        group.writeEntry("TodoViewVisible", mTodoViewShowAction->isChecked());
    }

    if (mCollectionViewShowAction) {
        group.writeEntry("ResourceViewVisible", mCollectionViewShowAction->isChecked());
    }

    if (mEventViewerShowAction) {
        group.writeEntry("EventViewerVisible", mEventViewerShowAction->isChecked());
    }

    mCollectionViewStateSaver->saveState();
    mCollectionSelectionModelStateSaver->saveState();

    KConfigGroup selectionViewGroup = config->group(QStringLiteral("GlobalCollectionView"));
    KConfigGroup selectionGroup = config->group(QStringLiteral("GlobalCollectionSelection"));
    selectionGroup.sync();
    selectionViewGroup.sync();
    config->sync();
}

void ActionManager::file_open()
{
    const QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    const QUrl dir = QUrl::fromLocalFile(defaultPath + QLatin1StringView("/korganizer/"));
    const QUrl fileUrl =
        QFileDialog::getOpenFileUrl(dialogParent(), i18nc("@title:window", "Select Calendar File to Open"), dir, QStringLiteral("text/calendar (*.ics *.vcs)"));

    if (!fileUrl.isEmpty()) { // isEmpty if user canceled the dialog
        file_open(fileUrl);
    }
}

void ActionManager::file_open(const QUrl &url)
{
    // is that URL already opened somewhere else? Activate that window
    KOrg::MainWindow *korg = ActionManager::findInstance(url); /* cppcheck-suppress constVariablePointer */
    if ((nullptr != korg) && (korg != mMainWindow)) {
#if KDEPIM_HAVE_X11
        KWindowSystem::activateWindow(korg->topLevelWidget()->windowHandle());
#endif
        return;
    }

    qCDebug(KORGANIZER_LOG) << url.toDisplayString();

    importCalendar(url);
}

void ActionManager::file_import()
{
    const QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QUrl dir = QUrl::fromLocalFile(defaultPath);

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group = config->group(QStringLiteral("Settings"));
    const QUrl lastLocation(group.readEntry("LastImportLocation", QString()));
    if (lastLocation.isValid()) {
        dir = lastLocation.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash);
    }

    const QUrl fileUrl = QFileDialog::getOpenFileUrl(dialogParent(),
                                                     i18nc("@title:window", "Select Calendar File to Import"),
                                                     dir,
                                                     QStringLiteral("text/calendar (*.ics *.vcs)"));

    if (!fileUrl.isEmpty()) { // empty if user canceled the dialog
        importCalendar(fileUrl);
        group.writeEntry("LastImportLocation", fileUrl.toString());
    }
}

void ActionManager::file_archive()
{
    mCalendarView->archiveCalendar();
}

bool ActionManager::importURL(const QUrl &url, bool merge)
{
    auto importer = new Akonadi::ICalImporter();
    bool jobStarted;
    if (merge) {
        connect(importer, &Akonadi::ICalImporter::importIntoExistingFinished, this, &ActionManager::slotMergeFinished);
        jobStarted = importer->importIntoExistingResource(url, Akonadi::Collection());
    } else {
        connect(importer, &Akonadi::ICalImporter::importIntoNewFinished, this, &ActionManager::slotNewResourceFinished);
        jobStarted = importer->importIntoNewResource(url.path());
    }

    if (jobStarted) {
        mImportAction->setEnabled(false);
    } else {
        // empty error message means user canceled.
        if (!importer->errorMessage().isEmpty()) {
            mCalendarView->showMessage(xi18nc("@info", "An error occurred: <message>%1 (url: %2)</message>", importer->errorMessage(), url.path()),
                                       KMessageWidget::Error);
        }
    }

    return jobStarted;
}

bool ActionManager::saveURL()
{
    if (!mCalendarView->saveCalendar(mFile)) {
        qCDebug(KORGANIZER_LOG) << "calendar view save failed.";
        return false;
    }

    if (!mURL.isLocalFile()) {
        auto job = KIO::file_copy(QUrl::fromLocalFile(mFile), mURL);
        KJobWidgets::setWindow(job, view());
        if (!job->exec()) {
            const QString msg = xi18nc("@info", "Cannot upload calendar to <filename>%1</filename>", mURL.toDisplayString());
            KMessageBox::error(dialogParent(), msg);
            return false;
        }
    }

    mMainWindow->showStatusMessage(xi18nc("@info", "Saved calendar <filename>%1</filename>", mURL.toDisplayString()));

    return true;
}

bool ActionManager::saveAsURL(const QUrl &url)
{
    qCDebug(KORGANIZER_LOG) << url.toDisplayString();

    if (url.isEmpty()) {
        qCDebug(KORGANIZER_LOG) << "Empty URL.";
        return false;
    }
    if (!url.isValid()) {
        qCDebug(KORGANIZER_LOG) << "Malformed URL.";
        return false;
    }

    QString const fileOrig = mFile;
    QUrl const URLOrig = mURL;

    QTemporaryFile *tempFile = nullptr;
    if (url.isLocalFile()) {
        mFile = url.toLocalFile();
    } else {
        tempFile = new QTemporaryFile;
        if (!tempFile->open()) {
            qCWarning(KORGANIZER_LOG) << "Impossible to open temporary file";
            delete mTempFile;
            return false;
        }
        tempFile->setAutoRemove(false);
        mFile = tempFile->fileName();
    }
    mURL = url;

    bool const success = saveURL(); // Save local file and upload local file
    if (success) {
        delete mTempFile;
        mTempFile = tempFile;
        setTitle();
    } else {
        KMessageBox::error(dialogParent(),
                           xi18nc("@info", "Unable to save calendar to the file <filename>%1</filename>", mFile),
                           i18nc("@title:window", "Error"));
        qCDebug(KORGANIZER_LOG) << "failed";
        mURL = URLOrig;
        mFile = fileOrig;
        delete tempFile;
    }

    return success;
}

void ActionManager::saveProperties(KConfigGroup &config)
{
    config.writeEntry("UseResourceCalendar", !mMainWindow->hasDocument());
    if (mMainWindow->hasDocument()) {
        config.writePathEntry("Calendar", mURL.url());
    }
}

void ActionManager::readProperties(const KConfigGroup &)
{
    mMainWindow->init(false);
}

// Configuration changed as a result of the options dialog.
void ActionManager::updateConfig()
{
    mNextXDays->setText(i18ncp("@action:inmenu", "&Next Day", "&Next %1 Days", KOPrefs::instance()->mNextXDays));

    KOCore::self()->reloadPlugins();
}

void ActionManager::configureDateTime()
{
    QProcess const proc;
    const QString program = QStringLiteral("kcmshell6");
    QStringList arguments;
    arguments << QStringLiteral("kcm_regionandlang") << QStringLiteral("clock");

    if (!proc.startDetached(program, arguments)) {
        KMessageBox::error(dialogParent(), i18nc("@info", "Could not start control module for date and time format."));
    }
}

KOrg::MainWindow *ActionManager::findInstance(const QUrl &url)
{
    if (mWindowList) {
        if (url.isEmpty()) {
            return mWindowList->defaultInstance();
        } else {
            return mWindowList->findInstance(url);
        }
    } else {
        return nullptr;
    }
}

bool ActionManager::openURL(const QString &url)
{
    importCalendar(QUrl::fromLocalFile(url));
    return true;
}

/* cppcheck-suppress unusedFunction */
void ActionManager::dumpText(const QString &str)
{
    qCDebug(KORGANIZER_LOG) << str;
}

void ActionManager::toggleDateNavigator()
{
    const bool visible = mDateNavigatorShowAction->isChecked();
    if (mCalendarView) {
        mCalendarView->showDateNavigator(visible);
    }
}

void ActionManager::toggleTodoView()
{
    const bool visible = mTodoViewShowAction->isChecked();
    if (mCalendarView) {
        mCalendarView->showTodoView(visible);
    }
}

void ActionManager::toggleEventViewer()
{
    const bool visible = mEventViewerShowAction->isChecked();
    if (mCalendarView) {
        mCalendarView->showEventViewer(visible);
    }
}

void ActionManager::toggleResourceView()
{
    const bool visible = mCollectionViewShowAction->isChecked();
    if (mCollectionView) {
        if (visible) {
            mCollectionView->show();
        } else {
            mCollectionView->hide();
        }
    }
}

bool ActionManager::mergeURL(const QString &url)
{
    return importURL(QUrl::fromLocalFile(url), true);
}

bool ActionManager::saveAsURL(const QString &url)
{
    return saveAsURL(QUrl::fromLocalFile(url));
}

QString ActionManager::getCurrentURLasString() const
{
    return mURL.url();
}

bool ActionManager::editIncidence(Akonadi::Item::Id id)
{
    return mCalendarView->editIncidence(id);
}

bool ActionManager::showIncidence(Akonadi::Item::Id id)
{
    return mCalendarView->showIncidence(id);
}

bool ActionManager::showIncidenceContext(Akonadi::Item::Id id)
{
    return mCalendarView->showIncidenceContext(id);
}

void ActionManager::showIncidenceByUid(const QString &uid, const QDateTime &occurrence, const QString &xdgActivationToken)
{
    mCalendarView->showIncidenceByUid(uid, occurrence.date());
    mCalendarView->showDate(occurrence.date());

    KWindowSystem::setCurrentXdgActivationToken(xdgActivationToken);
    KOrg::MainWindow *mainWindow = ActionManager::findInstance(QUrl());
    if (mainWindow) {
        KWindowSystem::activateWindow(mainWindow->topLevelWidget()->windowHandle());
        mainWindow->topLevelWidget()->show();
        mainWindow->topLevelWidget()->windowHandle()->raise();
    }
}

bool ActionManager::handleCommandLine(const QStringList &args)
{
    QCommandLineParser parser;
    korganizer_options(&parser);
    parser.process(args);

    KOrg::MainWindow *mainWindow = ActionManager::findInstance(QUrl());

    bool ret = true;

    if (!mainWindow) {
        qCCritical(KORGANIZER_LOG) << "Unable to find default calendar resources view.";
        ret = false;
    } else if (parser.positionalArguments().isEmpty()) {
        // No filenames given => all other args are meaningless, show main Window
        mainWindow->topLevelWidget()->show();
    } else {
        // Import, merge, or ask => we need the resource calendar window anyway.
        mainWindow->topLevelWidget()->show();

        // Check for import, merge or ask
        const QStringList argList = parser.positionalArguments();
        if (parser.isSet(QStringLiteral("import"))) {
            for (const QString &argUrl : argList) {
                importURL(QUrl::fromUserInput(argUrl), /*merge=*/false);
            }
        } else if (parser.isSet(QStringLiteral("merge"))) {
            for (const QString &argUrl : argList) {
                importURL(QUrl::fromUserInput(argUrl), /*merge=*/true);
            }
        } else {
            for (const QString &argUrl : argList) {
                mainWindow->actionManager()->importCalendar(QUrl::fromUserInput(argUrl));
            }
        }
    }
    return ret;
}

bool ActionManager::deleteIncidence(Akonadi::Item::Id id, bool force)
{
    return mCalendarView->deleteIncidence(id, force);
}

bool ActionManager::addIncidence(const QString &ical)
{
    return mCalendarView->addIncidence(ical);
}

class ActionManager::ActionStringsVisitor : public KCalendarCore::Visitor
{
public:
    ActionStringsVisitor() = default;

    bool act(const KCalendarCore::IncidenceBase::Ptr &incidence, QAction *show, QAction *edit, QAction *del)
    {
        mShow = show;
        mEdit = edit;
        mDelete = del;
        return incidence->accept(*this, incidence);
    }

protected:
    bool visit(const KCalendarCore::Event::Ptr &) override
    {
        if (mShow) {
            mShow->setText(i18nc("@action:inmenu", "&Show Event"));
        }
        if (mEdit) {
            mEdit->setText(i18nc("@action:inmenu", "&Edit Event…"));
        }
        if (mDelete) {
            mDelete->setText(i18nc("@action:inmenu", "&Delete Event"));
        }
        return true;
    }

    bool visit(const KCalendarCore::Todo::Ptr &) override
    {
        if (mShow) {
            mShow->setText(i18nc("@action:inmenu", "&Show To-do"));
        }
        if (mEdit) {
            mEdit->setText(i18nc("@action:inmenu", "&Edit To-do…"));
        }
        if (mDelete) {
            mDelete->setText(i18nc("@action:inmenu", "&Delete To-do"));
        }
        return true;
    }

    bool visit(const KCalendarCore::Journal::Ptr &) override
    {
        return assignDefaultStrings();
    }

    bool visit(const KCalendarCore::FreeBusy::Ptr &) override // to inhibit hidden virtual compile warning
    {
        return false;
    }

    bool assignDefaultStrings()
    {
        if (mShow) {
            mShow->setText(i18nc("@action:inmenu", "&Show"));
        }
        if (mEdit) {
            mEdit->setText(i18nc("@action:inmenu", "&Edit…"));
        }
        if (mDelete) {
            mDelete->setText(i18nc("@action:inmenu", "&Delete"));
        }
        return true;
    }

private:
    QAction *mShow = nullptr;
    QAction *mEdit = nullptr;
    QAction *mDelete = nullptr;
};

void ActionManager::processIncidenceSelection(const Akonadi::Item &item, QDate date)
{
    // qCDebug(KORGANIZER_LOG) << "ActionManager::processIncidenceSelection()";
    Q_UNUSED(date)

    if (!item.isValid()) {
        enableIncidenceActions(false);
        return;
    }

    const KCalendarCore::Incidence::Ptr incidence = Akonadi::CalendarUtils::incidence(item);
    if (!incidence) {
        enableIncidenceActions(false);
        return;
    }

    enableIncidenceActions(true);

    if (!mCalendarView->calendar()->hasRight(item, Akonadi::Collection::CanDeleteItem)) {
        mCutAction->setEnabled(false);
        mDeleteAction->setEnabled(false);
    }

    ActionStringsVisitor v;
    if (!v.act(incidence, mShowIncidenceAction, mEditIncidenceAction, mDeleteIncidenceAction)) {
        mShowIncidenceAction->setText(i18nc("@action:inmenu", "&Show"));
        mEditIncidenceAction->setText(i18nc("@action:inmenu", "&Edit…"));
        mDeleteIncidenceAction->setText(i18nc("@action:inmenu", "&Delete"));
    }
}

void ActionManager::enableIncidenceActions(bool enabled)
{
    mShowIncidenceAction->setEnabled(enabled);
    mEditIncidenceAction->setEnabled(enabled);
    mDeleteIncidenceAction->setEnabled(enabled);

    mCutAction->setEnabled(enabled);
    mCopyAction->setEnabled(enabled);
    mDeleteAction->setEnabled(enabled);
    mPublishEvent->setEnabled(enabled);
    mForwardEvent->setEnabled(enabled);
    mSendInvitation->setEnabled(enabled);
    mSendCancel->setEnabled(enabled);
    mSendStatusUpdate->setEnabled(enabled);
    mRequestChange->setEnabled(enabled);
    mRequestUpdate->setEnabled(enabled);
}

Akonadi::Collection ActionManager::selectedCollection() const
{
    const QModelIndex index = mCollectionView->view()->currentIndex();
    if (!index.isValid()) {
        return {};
    }

    return index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
}

void ActionManager::keyBindings()
{
    KShortcutsDialog dlg(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsDisallowed, view());
    if (mMainWindow) {
        dlg.addCollection(mMainWindow->getActionCollection());
    }

    dlg.configure();
}

void ActionManager::setTitle()
{
    mMainWindow->setTitle();
}

void ActionManager::openEventEditor()
{
    mCalendarView->newEvent();
}

void ActionManager::openEventEditor(const QString &summary)
{
    mCalendarView->newEvent(summary);
}

void ActionManager::openEventEditor(const QString &summary, const QString &description, const QStringList &attachments)
{
    mCalendarView->newEvent(summary, description, attachments);
}

void ActionManager::openEventEditor(const QString &summary, const QString &description, const QStringList &attachments, const QStringList &attendees)
{
    mCalendarView->newEvent(summary, description, attachments, attendees);
}

void ActionManager::openEventEditor(const QString &summary,
                                    const QString &description,
                                    const QString &uri,
                                    const QString &file,
                                    const QStringList &attendees,
                                    const QString &attachmentMimetype)
{
    int action = IncidenceEditorNG::IncidenceEditorSettings::self()->defaultEmailAttachMethod();
    if (attachmentMimetype != QLatin1StringView("message/rfc822")) {
        action = IncidenceEditorNG::IncidenceEditorSettings::Link;
    } else if (file.isEmpty()) {
        action = KOPrefs::TodoAttachLink;
    } else if (IncidenceEditorNG::IncidenceEditorSettings::self()->defaultEmailAttachMethod() == IncidenceEditorNG::IncidenceEditorSettings::Ask) {
        auto menu = new QMenu(nullptr);
        const QAction *attachLink = menu->addAction(i18nc("@action:inmenu", "Attach as &link"));
        const QAction *attachInline = menu->addAction(i18nc("@action:inmenu", "Attach &inline"));
        const QAction *attachBody = menu->addAction(i18nc("@action:inmenu", "Attach inline &without attachments"));
        menu->addSeparator();
        menu->addAction(QIcon::fromTheme(QStringLiteral("dialog-cancel")), i18nc("@action:inmenu", "C&ancel"));

        const QAction *ret = menu->exec(QCursor::pos());
        delete menu;

        if (ret == attachLink) {
            action = IncidenceEditorNG::IncidenceEditorSettings::Link;
        } else if (ret == attachInline) {
            action = IncidenceEditorNG::IncidenceEditorSettings::InlineFull;
        } else if (ret == attachBody) {
            action = IncidenceEditorNG::IncidenceEditorSettings::InlineBody;
        } else {
            return;
        }
    }

    QString attData;
    QTemporaryFile tf;
    tf.setAutoRemove(true);
    switch (action) {
    case IncidenceEditorNG::IncidenceEditorSettings::Link:
        attData = uri;
        break;
    case IncidenceEditorNG::IncidenceEditorSettings::InlineFull:
        attData = file;
        break;
    case IncidenceEditorNG::IncidenceEditorSettings::InlineBody: {
        QFile f(file);
        if (!f.open(QFile::ReadOnly)) {
            return;
        }
        auto msg = new KMime::Message();
        msg->setContent(f.readAll());
        msg->parse();
        if (msg == msg->textContent() || msg->textContent() == nullptr) { // no attachments
            attData = file;
        } else {
            if (KMessageBox::warningContinueCancel(nullptr,
                                                   i18nc("@info", "Removing attachments from an email might invalidate its signature."),
                                                   i18nc("@title:window", "Remove Attachments"),
                                                   KStandardGuiItem::cont(),
                                                   KStandardGuiItem::cancel(),
                                                   QStringLiteral("BodyOnlyInlineAttachment"))
                != KMessageBox::Continue) {
                delete msg;
                return;
            }
            auto newMsg = new KMime::Message();
            newMsg->setHead(msg->head());
            newMsg->setBody(msg->textContent()->body());
            newMsg->parse();
            newMsg->contentTransferEncoding()->from7BitString(msg->textContent()->contentTransferEncoding()->as7BitString());
            newMsg->contentType()->from7BitString(msg->textContent()->contentType()->as7BitString());
            newMsg->assemble();
            tf.write(newMsg->encodedContent());
            attData = tf.fileName();
        }
        tf.close();
        delete msg;
        break;
    }
    default:
        return;
    }

    mCalendarView->newEvent(summary,
                            description,
                            QStringList(attData),
                            attendees,
                            QStringList(attachmentMimetype),
                            action != IncidenceEditorNG::IncidenceEditorSettings::Link);
}

void ActionManager::openTodoEditor(const QString &text)
{
    mCalendarView->newTodo(text);
}

void ActionManager::openTodoEditor(const QString &summary, const QString &description, const QStringList &attachments)
{
    mCalendarView->newTodo(summary, description, attachments);
}

void ActionManager::openTodoEditor(const QString &summary, const QString &description, const QStringList &attachments, const QStringList &attendees)
{
    mCalendarView->newTodo(summary, description, attachments, attendees);
}

void ActionManager::openTodoEditor(const QString &summary,
                                   const QString &description,
                                   const QString &uri,
                                   const QString &file,
                                   const QStringList &attendees,
                                   const QString &attachmentMimetype)
{
    int action = KOPrefs::instance()->defaultTodoAttachMethod();
    if ((attachmentMimetype != QLatin1StringView("message/rfc822")) || file.isEmpty()) {
        action = KOPrefs::TodoAttachLink;
    } else if (KOPrefs::instance()->defaultTodoAttachMethod() == KOPrefs::TodoAttachAsk) {
        auto menu = new QMenu(nullptr);
        const QAction *attachLink = menu->addAction(i18nc("@action:inmenu", "Attach as &link"));
        const QAction *attachInline = menu->addAction(i18nc("@action:inmenu", "Attach &inline"));
        menu->addSeparator();
        menu->addAction(QIcon::fromTheme(QStringLiteral("dialog-cancel")), i18nc("@action:inmenu", "C&ancel"));

        const QAction *ret = menu->exec(QCursor::pos());
        delete menu;

        if (ret == attachLink) {
            action = KOPrefs::TodoAttachLink;
        } else if (ret == attachInline) {
            action = KOPrefs::TodoAttachInlineFull;
        } else {
            return;
        }
    }

    QString attData;
    switch (action) {
    case KOPrefs::TodoAttachLink:
        attData = uri;
        break;
    case KOPrefs::TodoAttachInlineFull:
        attData = file;
        break;
    default:
        return;
    }

    mCalendarView->newTodo(summary, description, QStringList(attData), attendees, QStringList(attachmentMimetype), action != KOPrefs::TodoAttachLink);
}

void ActionManager::openJournalEditor(QDate date)
{
    mCalendarView->newJournal(date);
}

void ActionManager::openJournalEditor(const QString &text, QDate date)
{
    mCalendarView->newJournal(text, date);
}

void ActionManager::openJournalEditor(const QString &text)
{
    mCalendarView->newJournal(text);
}

void ActionManager::showJournalView()
{
    mCalendarView->viewManager()->showJournalView();
}

void ActionManager::showTodoView()
{
    mCalendarView->viewManager()->showTodoView();
}

void ActionManager::showEventView()
{
    mCalendarView->viewManager()->showEventView();
}

void ActionManager::goDate(QDate date)
{
    mCalendarView->goDate(date);
}

void ActionManager::goDate(const QString &date)
{
    goDate(QLocale().toDate(date));
}

void ActionManager::showDate(QDate date)
{
    mCalendarView->showDate(date);
}

void ActionManager::updateUndoRedoActions()
{
    const Akonadi::History *history = mCalendarView->incidenceChanger()->history();

    if (history->undoAvailable()) {
        mUndoAction->setEnabled(true);
        QString nextUndoDescription = history->nextUndoDescription();
        if (nextUndoDescription.length() > 30) {
            nextUndoDescription.truncate(30);
            nextUndoDescription += QStringLiteral("…");
        }
        mUndoAction->setText(i18nc("@action:inmenu", "Undo: %1", nextUndoDescription));
    } else {
        mUndoAction->setEnabled(false);
        mUndoAction->setText(i18nc("@action:inmenu", "Undo"));
    }

    if (history->redoAvailable()) {
        mRedoAction->setEnabled(true);
        QString nextRedoDescription = history->nextRedoDescription();
        if (nextRedoDescription.length() > 30) {
            nextRedoDescription.truncate(30);
            nextRedoDescription += QStringLiteral("…");
        }
        mRedoAction->setText(i18nc("@action:inmenu", "Redo: %1", nextRedoDescription));
    } else {
        mRedoAction->setEnabled(false);
        mRedoAction->setText(i18nc("@action:inmenu", "Redo"));
    }

    mUndoAction->setIconText(i18nc("@action:button", "Undo"));
}

bool ActionManager::queryClose()
{
    return true;
}

void ActionManager::importCalendar(const QUrl &url)
{
    if (!url.isValid()) {
        KMessageBox::error(dialogParent(), xi18nc("@info", "URL <filename>%1</filename> is invalid.", url.toDisplayString()));
        return;
    }

    const QString questionText = xi18nc("@info",
                                        "<para>Would you like to merge this calendar item into an existing calendar "
                                        "or use it to create a brand new calendar?</para>"
                                        "<para>If you select merge, then you will be given the opportunity to select "
                                        "the destination calendar.</para>"
                                        "<para>If you select add, then a new calendar will be created for you automatically.</para>");

    const int answer = KMessageBox::questionTwoActionsCancel(dialogParent(),
                                                             questionText,
                                                             i18nc("@title:window", "Import Calendar"),
                                                             KGuiItem(i18nc("@action:button", "Merge into existing calendar")),
                                                             KGuiItem(i18nc("@action:button", "Add as new calendar")));

    switch (answer) {
    case KMessageBox::ButtonCode::PrimaryAction: // merge
        importURL(url, true);
        break;
    case KMessageBox::ButtonCode::SecondaryAction: // import
        importURL(url, false);
        break;
    default:
        return;
    }
}

void ActionManager::slotAutoArchivingSettingsModified()
{
    if (CalendarSupport::KCalPrefs::instance()->mAutoArchive) {
        mAutoArchiveTimer->start(4 * 60 * 60 * 1000); // check again in 4 hours
    } else {
        mAutoArchiveTimer->stop();
    }
}

void ActionManager::slotAutoArchive()
{
    if (!mCalendarView->calendar()) { // can this happen?
        return;
    }

    mAutoArchiveTimer->stop();
    CalendarSupport::EventArchiver archiver;

    archiver.runAuto(calendar(), mCalendarView->incidenceChanger(), mCalendarView, false /*no gui*/);

    // restart timer with the correct delay ( especially useful for the first time )
    slotAutoArchivingSettingsModified();
}

QWidget *ActionManager::dialogParent()
{
    return mCalendarView->topLevelWidget();
}

void ActionManager::openTodoEditor(const QString &summary,
                                   const QString &description,
                                   const QStringList &attachmentUris,
                                   const QStringList &attendees,
                                   const QStringList &attachmentMimetypes,
                                   bool attachmentIsInline)
{
    Q_UNUSED(summary)
    Q_UNUSED(description)
    Q_UNUSED(attachmentUris)
    Q_UNUSED(attendees)
    Q_UNUSED(attachmentMimetypes)
    Q_UNUSED(attachmentIsInline)
    qCWarning(KORGANIZER_LOG) << "Not implemented in korg-desktop";
}

void ActionManager::openEventEditor(const QString &summary,
                                    const QString &description,
                                    const QStringList &attachmentUris,
                                    const QStringList &attendees,
                                    const QStringList &attachmentMimetypes,
                                    bool attachmentIsInline)
{
    Q_UNUSED(summary)
    Q_UNUSED(description)
    Q_UNUSED(attachmentUris)
    Q_UNUSED(attendees)
    Q_UNUSED(attachmentMimetypes)
    Q_UNUSED(attachmentIsInline)
    qCWarning(KORGANIZER_LOG) << "Not implemented in korg-desktop";
}

#include "moc_actionmanager.cpp"
