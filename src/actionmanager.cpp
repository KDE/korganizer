/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 2002 Mike Pilone <mpilone@slac.com>
  SPDX-FileCopyrightText: 2002 Don Sanders <sanders@kde.org>
  SPDX-FileCopyrightText: 2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  SPDX-FileCopyrightText: 2010-2021 Laurent Montel <montel@kde.org>
  SPDX-FileCopyrightText: 2012-2019 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/
#include "actionmanager.h"
#include "akonadicollectionview.h"
#include "calendaradaptor.h"
#include "calendarview.h"
#include "kocore.h"
#include "kodialogmanager.h"
#include "koglobals.h"
#include "korgacinterface.h"
#include "korganizeradaptor.h"
#include "koviewmanager.h"
#include "kowindowlist.h"
#include "prefs/koprefs.h"
#include <KAuthorized>
#include <config-korganizer.h>

#include <CalendarSupport/CollectionSelection>
#include <CalendarSupport/EventArchiver>
#include <CalendarSupport/KCalPrefs>
#include <CalendarSupport/Utils>

#include <IncidenceEditor/IncidenceEditorSettings>

#include <Akonadi/Calendar/History>
#include <Akonadi/Calendar/ICalImporter>
#include <AkonadiCore/EntityDisplayAttribute>
#include <AkonadiCore/EntityTreeModel>
#include <AkonadiWidgets/ETMViewStateSaver>
#include <AkonadiWidgets/EntityTreeView>

#include <KCalendarCore/FileStorage>
#include <KCalendarCore/ICalFormat>
#include <KCalendarCore/Person>

#include <KIO/FileCopyJob>
#include <KIO/StatJob>
#include <KJobWidgets>
#include <KMime/KMimeMessage>

#include <KActionCollection>
#include <KMessageBox>
#include <KProcess>
#include <KSelectAction>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <QAction>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>

#include "korganizer_debug.h"
#include "korganizer_options.h"
#include <KNewStuff3/KNS3/QtQuickDialogWrapper>
#include <KToggleAction>
#include <KWindowSystem>
#include <QIcon>
#include <QTemporaryFile>

#include <KCheckableProxyModel>
#include <KSharedConfig>
#include <QApplication>
#include <QStandardPaths>
#include <QTimer>

KOWindowList *ActionManager::mWindowList = nullptr;

ActionManager::ActionManager(KXMLGUIClient *client, CalendarView *widget, QObject *parent, KOrg::MainWindow *mainWindow, bool isPart, QMenuBar *menuBar)
    : QObject(parent)
{
    new KOrgCalendarAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Calendar"), this);

    mGUIClient = client;
    mACollection = mGUIClient->actionCollection();
    mCalendarView = widget;
    mIsPart = isPart;
    mMainWindow = mainWindow;
    mMenuBar = menuBar;
}

ActionManager::~ActionManager()
{
    // Remove Part plugins
    KOCore::self()->unloadParts(mMainWindow, mParts);

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
        if (mHideMenuBarAction->isChecked()) {
            mMenuBar->show();
        } else {
            if (!dontShowWarning) {
                const QString accel = mHideMenuBarAction->shortcut().toString();
                KMessageBox::information(mCalendarView,
                                         i18n("<qt>This will hide the menu bar completely."
                                              " You can show it again by typing %1.</qt>",
                                              accel),
                                         i18n("Hide menu bar"),
                                         QStringLiteral("HideMenuBarWarning"));
            }
            mMenuBar->hide();
        }
        KOPrefs::instance()->setShowMenuBar(mHideMenuBarAction->isChecked());
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
    mCollectionSelectionModelStateSaver = new KViewStateMaintainer<Akonadi::ETMViewStateSaver>(config->group("GlobalCollectionSelection"));
    mCollectionSelectionModelStateSaver->setSelectionModel(calendar()->checkableProxyModel()->selectionModel());

    AkonadiCollectionViewFactory factory(mCalendarView);
    mCalendarView->addExtension(&factory);
    mCollectionView = factory.collectionView();
    mCollectionView->setObjectName(QStringLiteral("Resource View"));
    connect(mCollectionView, &AkonadiCollectionView::resourcesAddedRemoved, this, &ActionManager::slotResourcesAddedRemoved);

    connect(mCollectionView, &AkonadiCollectionView::defaultResourceChanged, this, &ActionManager::slotDefaultResourceChanged);

    connect(mCollectionView, &AkonadiCollectionView::colorsChanged, mCalendarView, qOverload<>(&CalendarView::updateConfig));

    mCollectionViewStateSaver = new KViewStateMaintainer<Akonadi::ETMViewStateSaver>(config->group("GlobalCollectionView"));
    mCollectionViewStateSaver->setView(mCollectionView->view());

    KCheckableProxyModel *checkableProxy = calendar()->checkableProxyModel();
    QItemSelectionModel *selectionModel = checkableProxy->selectionModel();

    mCollectionView->setCollectionSelectionProxyModel(checkableProxy);

    auto collectionSelection = new CalendarSupport::CollectionSelection(selectionModel);
    EventViews::EventView::setGlobalCollectionSelection(collectionSelection);

    mCalendarView->readSettings();

    connect(calendar().data(), &Akonadi::ETMCalendar::calendarChanged, mCalendarView, &CalendarView::resourcesChanged);
    connect(mCalendarView, &CalendarView::configChanged, this, &ActionManager::updateConfig);

    calendar()->setOwner(KCalendarCore::Person(CalendarSupport::KCalPrefs::instance()->fullName(), CalendarSupport::KCalPrefs::instance()->email()));
}

void ActionManager::initActions()
{
    /*************************** FILE MENU **********************************/

    //~~~~~~~~~~~~~~~~~~~~~~~ LOADING / SAVING ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (mIsPart) {
        if (mMainWindow->hasDocument()) {
            QAction *a = mACollection->addAction(KStandardAction::Open, this, SLOT(file_open()));
            mACollection->addAction(QStringLiteral("korganizer_open"), a);
        }

        QAction *a = mACollection->addAction(KStandardAction::Print, mCalendarView, SLOT(print()));
        mACollection->addAction(QStringLiteral("korganizer_print"), a);
        a = mACollection->addAction(KStandardAction::PrintPreview, mCalendarView, SLOT(printPreview()));
        mACollection->addAction(QStringLiteral("korganizer_print_preview"), a);
    } else {
        KStandardAction::open(this, qOverload<>(&ActionManager::file_open), mACollection);
        KStandardAction::print(mCalendarView, &CalendarView::print, mACollection);
        KStandardAction::printPreview(mCalendarView, &CalendarView::printPreview, mACollection);
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~ IMPORT / EXPORT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /** Import Action **/
    // TODO: Icon
    mImportAction = new QAction(i18n("Import &Calendar..."), this);
    mImportAction->setStatusTip(i18nc("@info:status", "Import a calendar"));
    mImportAction->setToolTip(i18nc("@info:tooltip", "Import an iCalendar or vCalendar file"));
    mImportAction->setWhatsThis(i18nc("@info:whatsthis",
                                      "Select this menu entry if you would like to import the contents of an "
                                      "iCalendar or vCalendar file into your current calendar collection."));
    mACollection->addAction(QStringLiteral("import_icalendar"), mImportAction);
    connect(mImportAction, &QAction::triggered, this, &ActionManager::file_import);

    QAction *action = nullptr;
    /** Get Hot New Stuff Action **/
    // TODO: Icon
    if (KAuthorized::authorize(QStringLiteral("ghns"))) {
        action = new QAction(i18n("Get &Hot New Stuff..."), this);
        action->setStatusTip(i18nc("@info:status", "Load a calendar from \"Get Hot New Stuff\""));
        action->setToolTip(i18nc("@info:tooltip", "Search \"Get Hot New Stuff\" for calendars to import"));
        action->setWhatsThis(i18nc("@info:whatsthis",
                                   "This menu entry opens the \"Get Hot New Stuff\" dialog that allows you "
                                   "to search and import fun and useful calendars donated to the community."));
        mACollection->addAction(QStringLiteral("downloadnewstuff"), action);
        connect(action, &QAction::triggered, this, &ActionManager::downloadNewStuff);
    }

    /** Export Action **/
    // TODO: Icon
    action = new QAction(i18n("Export as &iCalendar..."), this);
    action->setStatusTip(i18nc("@info:status", "Export calendar to file"));
    action->setToolTip(i18nc("@info:tooltip", "Export your calendar to an iCalendar file"));
    action->setWhatsThis(i18nc("@info:whatsthis", "Allows you to export your entire calendar collection to one iCalendar file."));
    mACollection->addAction(QStringLiteral("export_icalendar"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::exportICalendar);

    /** Archive Action **/
    // TODO: Icon
    action = new QAction(i18n("Archive O&ld Incidences..."), this);
    action->setStatusTip(i18nc("@info:status", "Archive incidences to file"));
    action->setToolTip(i18nc("@info:tooltip", "Archive old incidences to an iCalendar file"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "This menu entry opens a dialog that allows you to select old events and to-dos "
                               "that you can archive into an iCalendar file.  The archived incidences will "
                               "be removed from your existing calendar collection."));
    mACollection->addAction(QStringLiteral("file_archive"), action);
    connect(action, &QAction::triggered, this, &ActionManager::file_archive);

    /** Purge Todos Action **/
    // TODO: Icon
    action = new QAction(i18n("Pur&ge Completed To-dos"), mACollection);
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
        mCutAction = mACollection->addAction(KStandardAction::Cut, QStringLiteral("korganizer_cut"), mCalendarView, SLOT(edit_cut()));
        mCopyAction = mACollection->addAction(KStandardAction::Copy, QStringLiteral("korganizer_copy"), mCalendarView, SLOT(edit_copy()));
        pasteAction = mACollection->addAction(KStandardAction::Paste, QStringLiteral("korganizer_paste"), mCalendarView, SLOT(edit_paste()));
        mUndoAction = mACollection->addAction(KStandardAction::Undo, QStringLiteral("korganizer_undo"), history, SLOT(undo()));
        mRedoAction = mACollection->addAction(KStandardAction::Redo, QStringLiteral("korganizer_redo"), history, SLOT(redo()));
    } else {
        mCutAction = KStandardAction::cut(mCalendarView, &CalendarView::edit_cut, mACollection);
        mCopyAction = KStandardAction::copy(mCalendarView, &CalendarView::edit_copy, mACollection);
        pasteAction = KStandardAction::paste(mCalendarView, &CalendarView::edit_paste, mACollection);
        mUndoAction = KStandardAction::undo(history, SLOT(undo()), mACollection);
        mRedoAction = KStandardAction::redo(history, SLOT(redo()), mACollection);
    }
    mDeleteAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18n("&Delete"), this);
    mACollection->addAction(QStringLiteral("edit_delete"), mDeleteAction);
    connect(mDeleteAction, &QAction::triggered, mCalendarView, &CalendarView::appointment_delete);
    if (mIsPart) {
        QAction *a = KStandardAction::find(mCalendarView->dialogManager(), SLOT(showSearchDialog()), mACollection);
        mACollection->addAction(QStringLiteral("korganizer_find"), a);
    } else {
        KStandardAction::find(mCalendarView->dialogManager(), SLOT(showSearchDialog()), mACollection);
    }
    pasteAction->setEnabled(false);
    mUndoAction->setEnabled(false);
    mRedoAction->setEnabled(false);
    connect(mCalendarView, &CalendarView::pasteEnabled, pasteAction, &QAction::setEnabled);
    connect(history, &Akonadi::History::changed, this, &ActionManager::updateUndoRedoActions);

    /************************** VIEW MENU *********************************/

    /** Whats Next View Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-upcoming-events")), i18n("What's &Next"), this);
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
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-month")), i18n("&Month"), this);
    action->setStatusTip(i18nc("@info:status", "Month View"));
    action->setToolTip(i18nc("@info:tooltip", "Switch to the Month View"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Switches to the Month View, which shows all the events and due to-dos "
                               "in a familiar monthly calendar layout."));
    mACollection->addAction(QStringLiteral("view_month"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::showMonthView);

    /** Agenda View Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-agenda")), i18n("&Agenda"), this);
    action->setStatusTip(i18nc("@info:status", "Agenda View"));
    action->setToolTip(i18nc("@info:tooltip", "Switch to the Agenda View"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Switches to the Agenda View, which presents your events or due to-dos "
                               "for one or more days, sorted chronologically.  You can also see the length "
                               "of each event in the day timetable."));
    mACollection->addAction(QStringLiteral("view_agenda"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::showAgendaView);

    /** List View Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-list")), i18n("&Event List"), this);
    action->setStatusTip(i18nc("@info:status", "List View"));
    action->setToolTip(i18nc("@info:tooltip", "Switch to the List View"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Switches to the List View, which displays all your to-dos, events and "
                               "journal entries for the dates selected in the Date Navigator as a list."));
    mACollection->addAction(QStringLiteral("view_list"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::showListView);

    /** Todo View Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-tasks")), i18n("&To-do List"), this);
    action->setStatusTip(i18nc("@info:status", "To-do List View"));
    action->setToolTip(i18nc("@info:tooltip", "Switch to the To-do List View"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Switches to the To-do List view, which provides a place for you to "
                               "track tasks that need to be done."));
    mACollection->addAction(QStringLiteral("view_todo"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::showTodoView);

    /** Journal View Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-journal")), i18n("&Journal"), this);
    action->setStatusTip(i18nc("@info:status", "Journal View"));
    action->setToolTip(i18nc("@info:tooltip", "Switch to the Journal View"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Switches to the Journal View, which provides a place for you to record "
                               "your reflections, occurrences or experiences."));
    mACollection->addAction(QStringLiteral("view_journal"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::showJournalView);

    /** Timeline View Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-timeline")), i18n("Time&line"), this);
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
    action = new QAction(i18n("&Refresh"), this);
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
    mFilterAction = new KSelectAction(i18n("F&ilter"), this);
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
    action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in")), i18n("In Horizontally"), this);
    action->setEnabled(mCalendarView->currentView()->supportsZoom());
    mACollection->addAction(QStringLiteral("zoom_in_horizontally"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::zoomInHorizontally);

    action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), i18n("Out Horizontally"), this);
    action->setEnabled(mCalendarView->currentView()->supportsZoom());
    mACollection->addAction(QStringLiteral("zoom_out_horizontally"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::zoomOutHorizontally);

    action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in")), i18n("In Vertically"), this);
    action->setEnabled(mCalendarView->currentView()->supportsZoom());
    mACollection->addAction(QStringLiteral("zoom_in_vertically"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::zoomInVertically);

    action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), i18n("Out Vertically"), this);
    action->setEnabled(mCalendarView->currentView()->supportsZoom());
    mACollection->addAction(QStringLiteral("zoom_out_vertically"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::zoomOutVertically);

    /************************** Actions MENU *********************************/
    bool isRTL = QApplication::isRightToLeft();

    /** Scroll to Today Action **/
    action = new QAction(QIcon::fromTheme(QStringLiteral("go-jump-today")), i18nc("@action Jump to today", "To &Today"), this);
    action->setIconText(i18n("Today"));
    action->setStatusTip(i18nc("@info:status", "Scroll to Today"));
    action->setToolTip(i18nc("@info:tooltip", "Scroll the view to today"));
    action->setWhatsThis(i18nc("@info:whatsthis", "Scrolls the current view to the today's date."));
    mACollection->addAction(QStringLiteral("go_today"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::goToday);

    /** Scroll Backward Action **/
    action = new QAction(QIcon::fromTheme(isRTL ? QStringLiteral("go-next") : QStringLiteral("go-previous")), i18nc("scroll backward", "&Backward"), this);
    action->setIconText(i18nc("scroll backward", "Back"));
    action->setStatusTip(i18nc("@info:status", "Scroll Backward"));
    action->setToolTip(i18nc("@info:tooltip", "Scroll the view backward"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Scrolls backward by a day, week, month or year, depending on the "
                               "current calendar view."));
    mACollection->addAction(QStringLiteral("go_previous"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::goPrevious);

    /** Scroll Forward Action **/
    action = new QAction(QIcon::fromTheme(isRTL ? QStringLiteral("go-previous") : QStringLiteral("go-next")), i18nc("scroll forward", "&Forward"), this);
    action->setIconText(i18nc("scoll forward", "Forward"));
    action->setStatusTip(i18nc("@info:status", "Scroll Forward"));
    action->setToolTip(i18nc("@info:tooltip", "Scroll the view forward"));
    action->setWhatsThis(i18nc("@info:whatsthis",
                               "Scrolls forward by a day, week, month or year, depending on the "
                               "current calendar view."));
    mACollection->addAction(QStringLiteral("go_next"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::goNext);

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-day")), i18n("&Day"), this);
    mACollection->addAction(QStringLiteral("select_day"), action);
    action->setEnabled(mCalendarView->currentView()->supportsDateRangeSelection());
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::selectDay);

    mNextXDays = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-upcoming-days")), QString(), this);
    mNextXDays->setEnabled(mCalendarView->currentView()->supportsDateRangeSelection());
    mACollection->addAction(QStringLiteral("select_nextx"), mNextXDays);
    connect(mNextXDays, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::selectNextX);
    mNextXDays->setText(i18np("&Next Day", "&Next %1 Days", KOPrefs::instance()->mNextXDays));

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-workweek")), i18n("W&ork Week"), this);
    action->setEnabled(mCalendarView->currentView()->supportsDateRangeSelection());
    mACollection->addAction(QStringLiteral("select_workweek"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::selectWorkWeek);

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-week")), i18n("&Week"), this);
    action->setEnabled(mCalendarView->currentView()->supportsDateRangeSelection());
    mACollection->addAction(QStringLiteral("select_week"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(), &KOViewManager::selectWeek);

    /************************** Actions MENU *********************************/
    /** New Event Action **/
    mNewEventAction = new QAction(QIcon::fromTheme(QStringLiteral("appointment-new")), i18n("New E&vent..."), this);
    mNewEventAction->setStatusTip(i18nc("@info:status", "Create a new Event"));
    mNewEventAction->setToolTip(i18nc("@info:tooltip", "Create a new Event"));
    mNewEventAction->setWhatsThis(i18nc("@info:whatsthis",
                                        "Starts a dialog that allows you to create a new Event with reminders, "
                                        "attendees, recurrences and much more."));
    mACollection->addAction(QStringLiteral("new_event"), mNewEventAction);
    connect(mNewEventAction, &QAction::triggered, this, &ActionManager::slotNewEvent);

    /** New To-do Action **/
    mNewTodoAction = new QAction(QIcon::fromTheme(QStringLiteral("task-new")), i18n("New &To-do..."), this);
    mNewTodoAction->setStatusTip(i18nc("@info:status", "Create a new To-do"));
    mNewTodoAction->setToolTip(i18nc("@info:tooltip", "Create a new To-do"));
    mNewTodoAction->setWhatsThis(i18nc("@info:whatsthis",
                                       "Starts a dialog that allows you to create a new To-do with reminders, "
                                       "attendees, recurrences and much more."));
    mACollection->addAction(QStringLiteral("new_todo"), mNewTodoAction);
    connect(mNewTodoAction, &QAction::triggered, this, &ActionManager::slotNewTodo);

    /** New Sub-To-do Action **/
    // TODO: icon
    mNewSubtodoAction = new QAction(i18n("New Su&b-to-do..."), this);
    // TODO: statustip, tooltip, whatsthis
    mACollection->addAction(QStringLiteral("new_subtodo"), mNewSubtodoAction);
    connect(mNewSubtodoAction, &QAction::triggered, this, &ActionManager::slotNewSubTodo);
    mNewSubtodoAction->setEnabled(false);
    connect(mCalendarView, &CalendarView::todoSelected, mNewSubtodoAction, &QAction::setEnabled);

    /** New Journal Action **/
    mNewJournalAction = new QAction(QIcon::fromTheme(QStringLiteral("journal-new")), i18n("New &Journal..."), this);
    mNewJournalAction->setStatusTip(i18nc("@info:status", "Create a new Journal"));
    mNewJournalAction->setToolTip(i18nc("@info:tooltip", "Create a new Journal"));
    mNewJournalAction->setWhatsThis(i18nc("@info:whatsthis", "Starts a dialog that allows you to create a new Journal entry."));
    mACollection->addAction(QStringLiteral("new_journal"), mNewJournalAction);
    connect(mNewJournalAction, &QAction::triggered, this, &ActionManager::slotNewJournal);

    /** Configure Current View Action **/
    mConfigureViewAction = new QAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("Configure View..."), this);
    mConfigureViewAction->setIconText(i18n("Configure"));
    mConfigureViewAction->setStatusTip(i18nc("@info:status", "Configure the view"));
    mConfigureViewAction->setToolTip(i18nc("@info:tooltip", "Configure the current view"));
    mConfigureViewAction->setWhatsThis(i18nc("@info:whatsthis",
                                             "Starts a configuration dialog that allows you to change the settings "
                                             "for the current calendar view."));
    mConfigureViewAction->setEnabled(mCalendarView->currentView() && mCalendarView->currentView()->hasConfigurationDialog());
    mACollection->addAction(QStringLiteral("configure_view"), mConfigureViewAction);
    connect(mConfigureViewAction, &QAction::triggered, mCalendarView, &CalendarView::configureCurrentView);

    mShowIncidenceAction = new QAction(QIcon::fromTheme(QStringLiteral("document-preview")), i18n("&Show"), this);
    mACollection->addAction(QStringLiteral("show_incidence"), mShowIncidenceAction);
    connect(mShowIncidenceAction, &QAction::triggered, mCalendarView, qOverload<>(&CalendarView::showIncidence));

    mEditIncidenceAction = new QAction(QIcon::fromTheme(QStringLiteral("document-edit")), i18n("&Edit..."), this);
    mACollection->addAction(QStringLiteral("edit_incidence"), mEditIncidenceAction);
    connect(mEditIncidenceAction, &QAction::triggered, mCalendarView, qOverload<>(&CalendarView::editIncidence));

    mDeleteIncidenceAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18n("&Delete"), this);
    mACollection->addAction(QStringLiteral("delete_incidence"), mDeleteIncidenceAction);
    connect(mDeleteIncidenceAction, &QAction::triggered, mCalendarView, qOverload<>(&CalendarView::deleteIncidence));
    mACollection->setDefaultShortcut(mDeleteIncidenceAction, QKeySequence(Qt::Key_Delete));

    action = new QAction(i18n("&Make Sub-to-do Independent"), this);
    mACollection->addAction(QStringLiteral("unsub_todo"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::todo_unsub);
    action->setEnabled(false);
    connect(mCalendarView, &CalendarView::subtodoSelected, action, &QAction::setEnabled);

    // TODO: Add item to quickly toggle the reminder of a given incidence
    //   mToggleAlarmAction = new KToggleAction( i18n( "&Activate Reminder" ), 0,
    //                                         mCalendarView, SLOT(toggleAlarm()),
    //                                         mACollection, "activate_alarm" );

    /************************** SCHEDULE MENU ********************************/
    mPublishEvent = new QAction(QIcon::fromTheme(QStringLiteral("mail-send")), i18n("&Publish Item Information..."), this);
    mACollection->addAction(QStringLiteral("schedule_publish"), mPublishEvent);
    connect(mPublishEvent, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_publish()));
    mPublishEvent->setEnabled(false);

    mSendInvitation = new QAction(QIcon::fromTheme(QStringLiteral("mail-send")), i18n("Send &Invitation to Attendees"), this);
    mACollection->addAction(QStringLiteral("schedule_request"), mSendInvitation);
    connect(mSendInvitation, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_request()));
    mSendInvitation->setEnabled(false);
    connect(mCalendarView, &CalendarView::organizerEventsSelected, mSendInvitation, &QAction::setEnabled);

    mRequestUpdate = new QAction(i18n("Re&quest Update"), this);
    mACollection->addAction(QStringLiteral("schedule_refresh"), mRequestUpdate);
    connect(mRequestUpdate, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_refresh()));
    mRequestUpdate->setEnabled(false);
    connect(mCalendarView, &CalendarView::groupEventsSelected, mRequestUpdate, &QAction::setEnabled);

    mSendCancel = new QAction(i18n("Send &Cancellation to Attendees"), this);
    mACollection->addAction(QStringLiteral("schedule_cancel"), mSendCancel);
    connect(mSendCancel, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_cancel()));
    mSendCancel->setEnabled(false);
    connect(mCalendarView, &CalendarView::organizerEventsSelected, mSendCancel, &QAction::setEnabled);

    mSendStatusUpdate = new QAction(QIcon::fromTheme(QStringLiteral("mail-reply-sender")), i18n("Send Status &Update"), this);
    mACollection->addAction(QStringLiteral("schedule_reply"), mSendStatusUpdate);
    connect(mSendStatusUpdate, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_reply()));
    mSendStatusUpdate->setEnabled(false);
    connect(mCalendarView, &CalendarView::groupEventsSelected, mSendStatusUpdate, &QAction::setEnabled);

    mRequestChange = new QAction(i18nc("counter proposal", "Request Chan&ge"), this);
    mACollection->addAction(QStringLiteral("schedule_counter"), mRequestChange);
    connect(mRequestChange, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_counter()));
    mRequestChange->setEnabled(false);
    connect(mCalendarView, &CalendarView::groupEventsSelected, mRequestChange, &QAction::setEnabled);

    action = new QAction(i18n("&Mail Free Busy Information..."), this);
    mACollection->addAction(QStringLiteral("mail_freebusy"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::mailFreeBusy);
    action->setEnabled(true);

    mForwardEvent = new QAction(QIcon::fromTheme(QStringLiteral("mail-forward")), i18n("&Send as iCalendar..."), this);
    mACollection->addAction(QStringLiteral("schedule_forward"), mForwardEvent);
    connect(mForwardEvent, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_forward()));
    mForwardEvent->setEnabled(false);

    action = new QAction(i18n("&Upload Free Busy Information"), this);
    mACollection->addAction(QStringLiteral("upload_freebusy"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::uploadFreeBusy);
    action->setEnabled(true);

    if (!mIsPart) {
        action = new QAction(QIcon::fromTheme(QStringLiteral("help-contents")), i18n("&Address Book"), this);
        mACollection->addAction(QStringLiteral("addressbook"), action);
        connect(action, &QAction::triggered, mCalendarView, &CalendarView::openAddressbook);
    }

    /************************** SETTINGS MENU ********************************/

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SIDEBAR ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    mDateNavigatorShowAction = new KToggleAction(i18n("Show Date Navigator"), this);
    mACollection->addAction(QStringLiteral("show_datenavigator"), mDateNavigatorShowAction);
    connect(mDateNavigatorShowAction, &KToggleAction::triggered, this, &ActionManager::toggleDateNavigator);

    mTodoViewShowAction = new KToggleAction(i18n("Show To-do View"), this);
    mACollection->addAction(QStringLiteral("show_todoview"), mTodoViewShowAction);
    connect(mTodoViewShowAction, &KToggleAction::triggered, this, &ActionManager::toggleTodoView);

    mEventViewerShowAction = new KToggleAction(i18n("Show Item Viewer"), this);
    mACollection->addAction(QStringLiteral("show_eventviewer"), mEventViewerShowAction);
    connect(mEventViewerShowAction, &KToggleAction::triggered, this, &ActionManager::toggleEventViewer);

    KConfigGroup config(KSharedConfig::openConfig(), "Settings");
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
        mCollectionViewShowAction = new KToggleAction(i18n("Show Calendar Manager"), this);
        mACollection->addAction(QStringLiteral("show_resourceview"), mCollectionViewShowAction);
        connect(mCollectionViewShowAction, &KToggleAction::triggered, this, &ActionManager::toggleResourceView);
        mCollectionViewShowAction->setChecked(config.readEntry("ResourceViewVisible", true));

        toggleResourceView();
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SIDEBAR ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    mHideMenuBarAction = KStandardAction::showMenubar(this, &ActionManager::toggleMenubar, mACollection);
    mHideMenuBarAction->setChecked(KOPrefs::instance()->showMenuBar());
    toggleMenubar(true);

    action = new QAction(i18n("Configure &Date && Time..."), this);
    mACollection->addAction(QStringLiteral("conf_datetime"), action);
    connect(action, &QAction::triggered, this, &ActionManager::configureDateTime);

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-filter")), i18n("Manage View &Filters..."), this);
    mACollection->addAction(QStringLiteral("edit_filters"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::editFilters);

    action = new QAction(i18n("Manage C&ategories..."), this);
    mACollection->addAction(QStringLiteral("edit_categories"), action);
    connect(action, &QAction::triggered, mCalendarView->dialogManager(), &KODialogManager::showCategoryEditDialog);

    if (mIsPart) {
        action = new QAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("&Configure KOrganizer..."), this);
        mACollection->addAction(QStringLiteral("korganizer_configure"), action);
        connect(action, &QAction::triggered, mCalendarView, &CalendarView::edit_options);
        mACollection->addAction(KStandardAction::KeyBindings, QStringLiteral("korganizer_configure_shortcuts"), this, SLOT(keyBindings()));
    } else {
        KStandardAction::preferences(mCalendarView, &CalendarView::edit_options, mACollection);
        KStandardAction::keyBindings(this, &ActionManager::keyBindings, mACollection);
    }
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
        mCalendarView->showMessage(i18np("1 incidence was imported successfully.", "%1 incidences were imported successfully.", total),
                                   KMessageWidget::Information);
    } else {
        mCalendarView->showMessage(i18n("There was an error while merging the calendar: %1", importer->errorMessage()), KMessageWidget::Error);
    }
    sender()->deleteLater();
}

void ActionManager::slotNewResourceFinished(bool success)
{
    Q_ASSERT(sender());
    auto importer = qobject_cast<Akonadi::ICalImporter *>(sender());
    mImportAction->setEnabled(true);
    if (success) {
        mCalendarView->showMessage(i18n("New calendar added successfully"), KMessageWidget::Information);
    } else {
        mCalendarView->showMessage(i18n("Could not add a calendar. Error: %1", importer->errorMessage()), KMessageWidget::Error);
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
    KConfigGroup group = config->group("Settings");
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

    KConfigGroup selectionViewGroup = config->group("GlobalCollectionView");
    KConfigGroup selectionGroup = config->group("GlobalCollectionSelection");
    selectionGroup.sync();
    selectionViewGroup.sync();
    config->sync();
}

void ActionManager::file_open()
{
    const QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    const QUrl dir = QUrl::fromLocalFile(defaultPath + QLatin1String("/korganizer/"));
    const QUrl url =
        QFileDialog::getOpenFileUrl(dialogParent(), i18nc("@title:window", "Select Calendar File to Open"), dir, QStringLiteral("text/calendar (*.ics *.vcs)"));

    if (!url.isEmpty()) { // isEmpty if user canceled the dialog
        file_open(url);
    }
}

void ActionManager::file_open(const QUrl &url)
{
    // is that URL already opened somewhere else? Activate that window
    KOrg::MainWindow *korg = ActionManager::findInstance(url);
    if ((nullptr != korg) && (korg != mMainWindow)) {
#if KDEPIM_HAVE_X11
        KWindowSystem::activateWindow(korg->topLevelWidget()->winId());
#endif
        return;
    }

    qCDebug(KORGANIZER_LOG) << url.toDisplayString();

    importCalendar(url);
}

void ActionManager::file_import()
{
    const QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    const QUrl dir = QUrl::fromLocalFile(defaultPath + QLatin1String("/korganizer/"));
    const QUrl url = QFileDialog::getOpenFileUrl(dialogParent(),
                                                 i18nc("@title:window", "Select Calendar File to Import"),
                                                 dir,
                                                 QStringLiteral("text/calendar (*.ics *.vcs)"));

    if (!url.isEmpty()) { // isEmpty if user canceled the dialog
        importCalendar(url);
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
            mCalendarView->showMessage(i18n("An error occurred: %1", importer->errorMessage()), KMessageWidget::Error);
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
            const QString msg = i18n("Cannot upload calendar to '%1'", mURL.toDisplayString());
            KMessageBox::error(dialogParent(), msg);
            return false;
        }
    }

    mMainWindow->showStatusMessage(i18n("Saved calendar '%1'.", mURL.toDisplayString()));

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

    QString fileOrig = mFile;
    QUrl URLOrig = mURL;

    QTemporaryFile *tempFile = nullptr;
    if (url.isLocalFile()) {
        mFile = url.toLocalFile();
    } else {
        tempFile = new QTemporaryFile;
        tempFile->setAutoRemove(false);
        tempFile->open();
        mFile = tempFile->fileName();
    }
    mURL = url;

    bool success = saveURL(); // Save local file and upload local file
    if (success) {
        delete mTempFile;
        mTempFile = tempFile;
        setTitle();
    } else {
        KMessageBox::sorry(dialogParent(), i18n("Unable to save calendar to the file %1.", mFile), i18n("Error"));
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
    mNextXDays->setText(i18np("&Next Day", "&Next %1 Days", KOPrefs::instance()->mNextXDays));

    KOCore::self()->reloadPlugins();

    /* Hide/Show the Reminder Daemon */
    org::kde::korganizer::KOrgac korgacInterface{QStringLiteral("org.kde.korgac"), QStringLiteral("/ac"), QDBusConnection::sessionBus()};
    if (!KOPrefs::instance()->mShowReminderDaemon) {
        korgacInterface.hide();
    } else {
        korgacInterface.show();
    }

// Commented out because it crashes KOrganizer.
//  mParts = KOCore::self()->reloadParts( mMainWindow, mParts );
#ifdef AKONADI_PORT_DISABLED // shouldn't be required anymore
    if (mCollectionView) {
        mCollectionView->updateView();
    }
#endif
}

void ActionManager::configureDateTime()
{
    KProcess proc;
    proc << QStringLiteral("kcmshell5") << QStringLiteral("formats") << QStringLiteral("translations") << QStringLiteral("clock");

    if (!proc.startDetached()) {
        KMessageBox::sorry(dialogParent(), i18n("Could not start control module for date and time format."));
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

void ActionManager::dumpText(const QString &str)
{
    qCDebug(KORGANIZER_LOG) << str;
}

void ActionManager::toggleDateNavigator()
{
    bool visible = mDateNavigatorShowAction->isChecked();
    if (mCalendarView) {
        mCalendarView->showDateNavigator(visible);
    }
}

void ActionManager::toggleTodoView()
{
    bool visible = mTodoViewShowAction->isChecked();
    if (mCalendarView) {
        mCalendarView->showTodoView(visible);
    }
}

void ActionManager::toggleEventViewer()
{
    bool visible = mEventViewerShowAction->isChecked();
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
            for (const QString &url : argList) {
                importURL(QUrl::fromUserInput(url), /*merge=*/false);
            }
        } else if (parser.isSet(QStringLiteral("merge"))) {
            for (const QString &url : argList) {
                importURL(QUrl::fromUserInput(url), /*merge=*/true);
            }
        } else {
            for (const QString &url : argList) {
                mainWindow->actionManager()->importCalendar(QUrl::fromUserInput(url));
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

void ActionManager::downloadNewStuff()
{
    const auto installedEntries = KNS3::QtQuickDialogWrapper(QStringLiteral("korganizer.knsrc")).exec();
    for (const auto &e : installedEntries) {
        qCDebug(KORGANIZER_LOG) << " downloadNewStuff :";
        const QStringList lstFile = e.installedFiles();
        if (lstFile.count() != 1) {
            continue;
        }
        const QString file = lstFile.at(0);
        const QUrl filename = QUrl::fromLocalFile(file);
        qCDebug(KORGANIZER_LOG) << "filename :" << filename;
        if (!filename.isValid()) {
            continue;
        }

        KCalendarCore::FileStorage storage(calendar());
        storage.setFileName(file);
        storage.setSaveFormat(new KCalendarCore::ICalFormat);
        if (!storage.load()) {
            KMessageBox::error(mCalendarView, i18n("Could not load calendar %1.", file));
        } else {
            QStringList eventSummaries;
            const KCalendarCore::Event::List events = calendar()->events();
            eventSummaries.reserve(events.count());
            for (const KCalendarCore::Event::Ptr &event : events) {
                eventSummaries.append(event->summary());
            }

            const int result =
                KMessageBox::warningContinueCancelList(mCalendarView, i18n("The downloaded events will be merged into your current calendar."), eventSummaries);

            if (result != KMessageBox::Continue) {
                // FIXME (KNS2): hm, no way out here :-)
            }

            if (importURL(QUrl::fromLocalFile(file), true)) {
                // FIXME (KNS2): here neither
            }
        }
    }
}

QString ActionManager::localFileName() const
{
    return mFile;
}

class ActionManager::ActionStringsVisitor : public KCalendarCore::Visitor
{
public:
    ActionStringsVisitor()
    {
    }

    bool act(KCalendarCore::IncidenceBase::Ptr incidence, QAction *show, QAction *edit, QAction *del)
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
            mShow->setText(i18n("&Show Event"));
        }
        if (mEdit) {
            mEdit->setText(i18n("&Edit Event..."));
        }
        if (mDelete) {
            mDelete->setText(i18n("&Delete Event"));
        }
        return true;
    }

    bool visit(const KCalendarCore::Todo::Ptr &) override
    {
        if (mShow) {
            mShow->setText(i18n("&Show To-do"));
        }
        if (mEdit) {
            mEdit->setText(i18n("&Edit To-do..."));
        }
        if (mDelete) {
            mDelete->setText(i18n("&Delete To-do"));
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

protected:
    bool assignDefaultStrings()
    {
        if (mShow) {
            mShow->setText(i18n("&Show"));
        }
        if (mEdit) {
            mEdit->setText(i18n("&Edit..."));
        }
        if (mDelete) {
            mDelete->setText(i18n("&Delete"));
        }
        return true;
    }

    QAction *mShow = nullptr;
    QAction *mEdit = nullptr;
    QAction *mDelete = nullptr;
};

void ActionManager::processIncidenceSelection(const Akonadi::Item &item, QDate date)
{
    // qCDebug(KORGANIZER_LOG) << "ActionManager::processIncidenceSelection()";
    Q_UNUSED(date)

    const KCalendarCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
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
        mShowIncidenceAction->setText(i18n("&Show"));
        mEditIncidenceAction->setText(i18n("&Edit..."));
        mDeleteIncidenceAction->setText(i18n("&Delete"));
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
        return Akonadi::Collection();
    }

    return index.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
}

void ActionManager::keyBindings()
{
    KShortcutsDialog dlg(KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsDisallowed, view());
    if (mMainWindow) {
        dlg.addCollection(mMainWindow->getActionCollection());
    }

    for (KOrg::Part *part : qAsConst(mParts)) {
        if (part) {
            dlg.addCollection(part->actionCollection(), part->shortInfo());
        }
    }
    dlg.configure();
}

void ActionManager::loadParts()
{
    mParts = KOCore::self()->loadParts(mMainWindow);
}

void ActionManager::setTitle()
{
    mMainWindow->setTitle();
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
    if (attachmentMimetype != QLatin1String("message/rfc822")) {
        action = IncidenceEditorNG::IncidenceEditorSettings::Link;
    } else if (IncidenceEditorNG::IncidenceEditorSettings::self()->defaultEmailAttachMethod() == IncidenceEditorNG::IncidenceEditorSettings::Ask) {
        auto menu = new QMenu(nullptr);
        QAction *attachLink = menu->addAction(i18n("Attach as &link"));
        QAction *attachInline = menu->addAction(i18n("Attach &inline"));
        QAction *attachBody = menu->addAction(i18n("Attach inline &without attachments"));
        menu->addSeparator();
        menu->addAction(QIcon::fromTheme(QStringLiteral("dialog-cancel")), i18n("C&ancel"));

        QAction *ret = menu->exec(QCursor::pos());
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
                                                   i18n("Removing attachments from an email might invalidate its signature."),
                                                   i18n("Remove Attachments"),
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
    if (attachmentMimetype != QLatin1String("message/rfc822")) {
        action = KOPrefs::TodoAttachLink;
    } else if (KOPrefs::instance()->defaultTodoAttachMethod() == KOPrefs::TodoAttachAsk) {
        auto menu = new QMenu(nullptr);
        QAction *attachLink = menu->addAction(i18n("Attach as &link"));
        QAction *attachInline = menu->addAction(i18n("Attach &inline"));
        menu->addSeparator();
        menu->addAction(QIcon::fromTheme(QStringLiteral("dialog-cancel")), i18n("C&ancel"));

        QAction *ret = menu->exec(QCursor::pos());
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
    Akonadi::History *history = mCalendarView->incidenceChanger()->history();

    if (history->undoAvailable()) {
        mUndoAction->setEnabled(true);
        mUndoAction->setText(i18n("Undo: %1", history->nextUndoDescription()));
    } else {
        mUndoAction->setEnabled(false);
        mUndoAction->setText(i18n("Undo"));
    }

    if (history->redoAvailable()) {
        mRedoAction->setEnabled(true);
        mRedoAction->setText(i18n("Redo: %1", history->nextRedoDescription()));
    } else {
        mRedoAction->setEnabled(false);
        mRedoAction->setText(i18n("Redo"));
    }

    mUndoAction->setIconText(i18n("Undo"));
}

bool ActionManager::queryClose()
{
    return true;
}

void ActionManager::importCalendar(const QUrl &url)
{
    if (!url.isValid()) {
        KMessageBox::error(dialogParent(), i18n("URL '%1' is invalid.", url.toDisplayString()));
        return;
    }

    const QString questionText = i18nc("@info",
                                       "<p>Would you like to merge this calendar item into an existing calendar "
                                       "or use it to create a brand new calendar?</p>"
                                       "<p>If you select merge, then you will be given the opportunity to select "
                                       "the destination calendar.</p>"
                                       "<p>If you select add, then a new calendar will be created for you automatically.</p>");

    const int answer = KMessageBox::questionYesNoCancel(dialogParent(),
                                                        questionText,
                                                        i18nc("@title:window", "Import Calendar"),
                                                        KGuiItem(i18n("Merge into existing calendar")),
                                                        KGuiItem(i18n("Add as new calendar")));

    switch (answer) {
    case KMessageBox::Yes: // merge
        importURL(url, true);
        break;
    case KMessageBox::No: // import
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
