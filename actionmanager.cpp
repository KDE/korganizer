/*
  This file is part of KOrganizer.

  Copyright (c) 2002 Mike Pilone <mpilone@slac.com>
  Copyright (c) 2002 Don Sanders <sanders@kde.org>
  Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2010-2015 Laurent Montel <montel@kde.org>
  Copyright (c) 2012 Allen Winter <winter@kde.org>

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
#include "config-kdepim.h"
#include "actionmanager.h"
#include "akonadicollectionview.h"
#include "calendaradaptor.h"
#include "calendarview.h"
#include "job/htmlexportjob.h"
#include "htmlexportsettings.h"
#include "kocore.h"
#include "kodialogmanager.h"
#include "korganizeradaptor.h"
#include "koglobals.h"
#include "prefs/koprefs.h"
#include "koviewmanager.h"
#include "kowindowlist.h"
#include "KdepimDBusInterfaces/ReminderClient"
#include "kocheckableproxymodel.h"

#include <KHolidays/HolidayRegion>

#include <CalendarSupport/CollectionSelection>
#include <CalendarSupport/EventArchiver>
#include <CalendarSupport/KCalPrefs>
#include <CalendarSupport/Utils>

#include <IncidenceEditorsng/IncidenceEditorSettings>

#include <AkonadiCore/EntityDisplayAttribute>
#include <AkonadiWidgets/EntityTreeView>
#include <AkonadiCore/EntityTreeModel>
#include <AkonadiWidgets/ETMViewStateSaver>
#include <Akonadi/Calendar/History>
#include <Akonadi/Calendar/ICalImporter>

#include <KCalCore/FileStorage>
#include <KCalCore/ICalFormat>
#include <KCalCore/Person>

#include <KMime/KMimeMessage>
#include <KJobWidgets>
#include <KIO/StatJob>
#include <KIO/FileCopyJob>

#include <QAction>
#include <KActionCollection>
#include <QFileDialog>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <KMessageBox>
#include <KMimeTypeTrader>
#include <KProcess>
#include <KSelectAction>
#include <KShortcutsDialog>
#include <KStandardAction>

#include <QTemporaryFile>
#include <KToggleAction>
#include <KWindowSystem>
#include <KIO/NetAccess>
#include <KNS3/DownloadDialog>
#include <QIcon>
#include "korganizer_debug.h"
#include "korganizer_options.h"

#include <QApplication>
#include <QTimer>
#include <KSharedConfig>
#include <KLocale>
#include <QStandardPaths>

KOWindowList *ActionManager::mWindowList = Q_NULLPTR;

ActionManager::ActionManager(KXMLGUIClient *client, CalendarView *widget,
                             QObject *parent, KOrg::MainWindow *mainWindow,
                             bool isPart, QMenuBar *menuBar)
    : QObject(parent),
      mCollectionViewShowAction(Q_NULLPTR),
      mCollectionView(Q_NULLPTR), mCollectionViewStateSaver(Q_NULLPTR),
      mCollectionSelectionModelStateSaver(Q_NULLPTR)
{
    new KOrgCalendarAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Calendar"), this);

    mGUIClient = client;
    mACollection = mGUIClient->actionCollection();
    mCalendarView = widget;
    mIsPart = isPart;
    mTempFile = Q_NULLPTR;
    mHtmlExportSync = false;
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
                QString accel = mHideMenuBarAction->shortcut().toString();
                KMessageBox::information(mCalendarView,
                                         i18n("<qt>This will hide the menu bar completely."
                                              " You can show it again by typing %1.</qt>", accel),
                                         i18n("Hide menu bar"), QStringLiteral("HideMenuBarWarning"));
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

    // set up autoExporting stuff
    mAutoExportTimer = new QTimer(this);
    connect(mAutoExportTimer, &QTimer::timeout, this, &ActionManager::checkAutoExport);
    if (KOPrefs::instance()->mAutoExport &&
            KOPrefs::instance()->mAutoExportInterval > 0) {
        mAutoExportTimer->start(1000 * 60 * KOPrefs::instance()->mAutoExportInterval);
    }

    // set up autoSaving stuff
    mAutoArchiveTimer = new QTimer(this);
    mAutoArchiveTimer->setSingleShot(true);
    connect(mAutoArchiveTimer, &QTimer::timeout, this, &ActionManager::slotAutoArchive);

    // First auto-archive should be in 5 minutes (like in kmail).
    if (CalendarSupport::KCalPrefs::instance()->mAutoArchive) {
        mAutoArchiveTimer->start(5 * 60 * 1000);   // singleshot
    }

    setTitle();

    connect(mCalendarView, &CalendarView::modifiedChanged, this, &ActionManager::setTitle);
    connect(mCalendarView, &CalendarView::configChanged, this, &ActionManager::updateConfig);

    connect(mCalendarView, &CalendarView::incidenceSelected,
            this, &ActionManager::processIncidenceSelection);
    connect(mCalendarView, SIGNAL(exportHTML(KOrg::HTMLExportSettings*)),
            this, SLOT(exportHTML(KOrg::HTMLExportSettings*)));

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
    mCollectionSelectionModelStateSaver =
        new KViewStateMaintainer<Akonadi::ETMViewStateSaver>(
        config->group("GlobalCollectionSelection"));
    mCollectionSelectionModelStateSaver->setSelectionModel(calendar()->checkableProxyModel()->selectionModel());

    AkonadiCollectionViewFactory factory(mCalendarView);
    mCalendarView->addExtension(&factory);
    mCollectionView = factory.collectionView();
    mCollectionView->setObjectName(QStringLiteral("Resource View"));
    connect(mCollectionView, &AkonadiCollectionView::resourcesAddedRemoved, this, &ActionManager::slotResourcesAddedRemoved);
    connect(mCollectionView, &AkonadiCollectionView::defaultResourceChanged,
            this, &ActionManager::slotDefaultResourceChanged);
    connect(mCollectionView, SIGNAL(colorsChanged()),
            mCalendarView, SLOT(updateConfig()));

    mCollectionViewStateSaver = new KViewStateMaintainer<Akonadi::ETMViewStateSaver>(config->group("GlobalCollectionView"));
    mCollectionViewStateSaver->setView(mCollectionView->view());

    KCheckableProxyModel *checkableProxy = calendar()->checkableProxyModel();
    QItemSelectionModel  *selectionModel = checkableProxy->selectionModel();

    mCollectionView->setCollectionSelectionProxyModel(checkableProxy);

    CalendarSupport::CollectionSelection *collectionSelection = new CalendarSupport::CollectionSelection(selectionModel);
    EventViews::EventView::setGlobalCollectionSelection(collectionSelection);

    mCalendarView->readSettings();

    connect(calendar().data(), &Akonadi::ETMCalendar::calendarChanged,
            mCalendarView, &CalendarView::resourcesChanged);
    connect(mCalendarView, &CalendarView::configChanged, this, &ActionManager::updateConfig);

    calendar()->setOwner(KCalCore::Person::Ptr(new KCalCore::Person(CalendarSupport::KCalPrefs::instance()->fullName(),
                         CalendarSupport::KCalPrefs::instance()->email())));

}

void ActionManager::initActions()
{
    QAction *action;

    /*************************** FILE MENU **********************************/

    //~~~~~~~~~~~~~~~~~~~~~~~ LOADING / SAVING ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    if (mIsPart) {
        if (mMainWindow->hasDocument()) {
            QAction *a = mACollection->addAction(KStandardAction::Open, this, SLOT(file_open()));
            mACollection->addAction(QStringLiteral("korganizer_open"), a);
        }

        QAction *a = mACollection->addAction(KStandardAction::Print, mCalendarView, SLOT(print()));
        mACollection->addAction(QStringLiteral("korganizer_print"), a);
        a = mACollection->addAction(KStandardAction::PrintPreview, mCalendarView, SLOT(print()));
        mACollection->addAction(QStringLiteral("korganizer_print_preview"), a);
    } else {
        KStandardAction::open(this, SLOT(file_open()), mACollection);
        KStandardAction::print(mCalendarView, SLOT(print()), mACollection);
        KStandardAction::printPreview(mCalendarView, SLOT(printPreview()), mACollection);
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~ IMPORT / EXPORT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    mImportAction = new QAction(i18n("Import &Calendar..."), this);
    setHelpText(mImportAction, i18n("Merge the contents of another iCalendar"));
    mImportAction->setWhatsThis(
        i18n("Select this menu entry if you would like to merge the contents "
             "of another iCalendar into your current calendar."));
    mACollection->addAction(QStringLiteral("import_icalendar"), mImportAction);
    connect(mImportAction, &QAction::triggered, this, &ActionManager::file_import);

    QAction *importAction = new QAction(i18n("&Import From UNIX Ical Tool"), this);
    setHelpText(importAction, i18n("Import a calendar in another format"));
    importAction->setWhatsThis(
        i18n("Select this menu entry if you would like to import the contents "
             "of a non-iCalendar formatted file into your current calendar."));
    mACollection->addAction(QStringLiteral("import_ical"), importAction);
    connect(importAction, &QAction::triggered, this, &ActionManager::file_icalimport);

    action = new QAction(i18n("Get &Hot New Stuff..."), this);
    mACollection->addAction(QStringLiteral("downloadnewstuff"), action);
    connect(action, &QAction::triggered, this, &ActionManager::downloadNewStuff);

    action = new QAction(i18n("Export &Web Page..."), this);
    mACollection->addAction(QStringLiteral("export_web"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::exportWeb);

    action = new QAction(i18n("Export as &iCalendar..."), this);
    mACollection->addAction(QStringLiteral("export_icalendar"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::exportICalendar);

    action = new QAction(i18n("Export as &vCalendar..."), this);
    mACollection->addAction(QStringLiteral("export_vcalendar"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::exportVCalendar);

    //Laurent: 2009-03-24 comment it until upload will implement
    //action = new QAction( i18n( "Upload &Hot New Stuff..." ), this );
    //mACollection->addAction( "uploadnewstuff", action );
    //connect( action, SIGNAL(triggered(bool)), SLOT(uploadNewStuff()) );

    action = new QAction(i18n("Archive O&ld Entries..."), this);
    mACollection->addAction(QStringLiteral("file_archive"), action);
    connect(action, &QAction::triggered, this, &ActionManager::file_archive);

    action = new QAction(i18n("Pur&ge Completed To-dos"), mACollection);
    mACollection->addAction(QStringLiteral("purge_completed"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::purgeCompleted);

    /************************** EDIT MENU *********************************/

    QAction *pasteAction;
    Akonadi::History *history = mCalendarView->history();
    if (mIsPart) {
        // edit menu
        mCutAction = mACollection->addAction(KStandardAction::Cut, QStringLiteral("korganizer_cut"),
                                             mCalendarView, SLOT(edit_cut()));
        mCopyAction = mACollection->addAction(KStandardAction::Copy, QStringLiteral("korganizer_copy"),
                                              mCalendarView, SLOT(edit_copy()));
        pasteAction = mACollection->addAction(KStandardAction::Paste, QStringLiteral("korganizer_paste"),
                                              mCalendarView, SLOT(edit_paste()));
        mUndoAction = mACollection->addAction(KStandardAction::Undo, QStringLiteral("korganizer_undo"),
                                              history, SLOT(undo()));
        mRedoAction = mACollection->addAction(KStandardAction::Redo, QStringLiteral("korganizer_redo"),
                                              history, SLOT(redo()));
    } else {
        mCutAction = KStandardAction::cut(mCalendarView, SLOT(edit_cut()), mACollection);
        mCopyAction = KStandardAction::copy(mCalendarView, SLOT(edit_copy()), mACollection);
        pasteAction = KStandardAction::paste(mCalendarView, SLOT(edit_paste()), mACollection);
        mUndoAction = KStandardAction::undo(history, SLOT(undo()), mACollection);
        mRedoAction = KStandardAction::redo(history, SLOT(redo()), mACollection);
    }
    mDeleteAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18n("&Delete"), this);
    mACollection->addAction(QStringLiteral("edit_delete"), mDeleteAction);
    connect(mDeleteAction, &QAction::triggered, mCalendarView, &CalendarView::appointment_delete);
    if (mIsPart) {
        QAction *a =
            KStandardAction::find(mCalendarView->dialogManager(),
                                  SLOT(showSearchDialog()), mACollection);
        mACollection->addAction(QStringLiteral("korganizer_find"), a);
    } else {
        KStandardAction::find(mCalendarView->dialogManager(),
                              SLOT(showSearchDialog()), mACollection);
    }
    pasteAction->setEnabled(false);
    mUndoAction->setEnabled(false);
    mRedoAction->setEnabled(false);
    connect(mCalendarView, &CalendarView::pasteEnabled, pasteAction, &QAction::setEnabled);
    connect(history, &Akonadi::History::changed, this, &ActionManager::updateUndoRedoActions);

    /************************** VIEW MENU *********************************/

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ VIEWS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-upcoming-events")), i18n("What's &Next"), this);
    mACollection->addAction(QStringLiteral("view_whatsnext"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(),
            &KOViewManager::showWhatsNextView);

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-month")), i18n("&Month"), this);
    mACollection->addAction(QStringLiteral("view_month"), action);
    connect(action, &QAction::triggered,
            mCalendarView->viewManager(), &KOViewManager::showMonthView);

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-agenda")), i18n("&Agenda"), this);
    mACollection->addAction(QStringLiteral("view_agenda"), action);
    connect(action, &QAction::triggered,
            mCalendarView->viewManager(), &KOViewManager::showAgendaView);

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-list")), i18n("&Event List"), this);
    mACollection->addAction(QStringLiteral("view_list"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(),
            &KOViewManager::showListView);

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-tasks")), i18n("&To-do List"), this);
    mACollection->addAction(QStringLiteral("view_todo"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(),
            &KOViewManager::showTodoView);

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-journal")), i18n("&Journal"), this);
    mACollection->addAction(QStringLiteral("view_journal"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(),
            &KOViewManager::showJournalView);

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-timeline")), i18n("Time&line"), this);
    mACollection->addAction(QStringLiteral("view_timeline"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(),
            &KOViewManager::showTimeLineView);

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-time-spent")), i18n("Time&spent"), this);
    mACollection->addAction(QStringLiteral("view_timespent"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(),
            &KOViewManager::showTimeSpentView);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~ REFRESH ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    action = new QAction(i18n("&Refresh"), this);
    mACollection->addAction(QStringLiteral("update"), action);
    connect(action, SIGNAL(triggered(bool)), mCalendarView, SLOT(updateView()));

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~ FILTER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    mFilterAction = new KSelectAction(i18n("F&ilter"), this);
    mFilterAction->setToolBarMode(KSelectAction::MenuMode);
    mACollection->addAction(QStringLiteral("filter_select"), mFilterAction);
    mFilterAction->setEditable(false);
    connect(mFilterAction, SIGNAL(triggered(int)),
            mCalendarView, SLOT(filterActivated(int)));
    connect(mCalendarView, &CalendarView::filtersUpdated,
            this, &ActionManager::setItems);
    connect(mCalendarView, &CalendarView::filterChanged,
            this, &ActionManager::setTitle);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ZOOM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // TODO: try to find / create better icons for the following 4 actions
    action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in")), i18n("In Horizontally"), this);
    action->setEnabled(mCalendarView->currentView()->supportsZoom());
    mACollection->addAction(QStringLiteral("zoom_in_horizontally"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(),
            &KOViewManager::zoomInHorizontally);

    action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), i18n("Out Horizontally"), this);
    action->setEnabled(mCalendarView->currentView()->supportsZoom());
    mACollection->addAction(QStringLiteral("zoom_out_horizontally"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(),
            &KOViewManager::zoomOutHorizontally);

    action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in")), i18n("In Vertically"), this);
    action->setEnabled(mCalendarView->currentView()->supportsZoom());
    mACollection->addAction(QStringLiteral("zoom_in_vertically"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(),
            &KOViewManager::zoomInVertically);

    action = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), i18n("Out Vertically"), this);
    action->setEnabled(mCalendarView->currentView()->supportsZoom());
    mACollection->addAction(QStringLiteral("zoom_out_vertically"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(),
            &KOViewManager::zoomOutVertically);

    /************************** Actions MENU *********************************/
    bool isRTL = QApplication::isRightToLeft();

    action = new QAction(QIcon::fromTheme(QStringLiteral("go-jump-today")),
                         i18nc("@action Jump to today", "To &Today"), this);
    action->setIconText(i18n("Today"));
    setHelpText(action, i18n("Scroll to Today"));
    mACollection->addAction(QStringLiteral("go_today"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::goToday);

    action = new QAction(QIcon::fromTheme(isRTL ? QStringLiteral("go-next") : QStringLiteral("go-previous")),
                         i18nc("scroll backward", "&Backward"), this);
    action->setIconText(i18nc("scroll backward", "Back"));
    setHelpText(action, i18n("Scroll Backward"));
    mACollection->addAction(QStringLiteral("go_previous"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::goPrevious);

    // Changing the action text by setText makes the toolbar button disappear.
    // This has to be fixed first, before the connects below can be reenabled.
    /*
    connect( mCalendarView, SIGNAL(changeNavStringPrev(QString)),
             action, SLOT(setText(QString)) );
    connect( mCalendarView, SIGNAL(changeNavStringPrev(QString)),
             this, SLOT(dumpText(QString)) );*/

    action = new QAction(QIcon::fromTheme(isRTL ? QStringLiteral("go-previous") : QStringLiteral("go-next")),
                         i18nc("scroll forward", "&Forward"), this);
    action->setIconText(i18nc("scoll forward", "Forward"));
    setHelpText(action, i18n("Scroll Forward"));
    mACollection->addAction(QStringLiteral("go_next"), action);
    connect(action, &QAction::triggered, mCalendarView, &CalendarView::goNext);
    /*
    connect( mCalendarView,SIGNAL(changeNavStringNext(QString)),
             action,SLOT(setText(QString)) );
    */

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-day")), i18n("&Day"), this);
    mACollection->addAction(QStringLiteral("select_day"), action);
    action->setEnabled(mCalendarView->currentView()->supportsDateRangeSelection());
    connect(action, &QAction::triggered, mCalendarView->viewManager(),
            &KOViewManager::selectDay);

    mNextXDays = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-upcoming-days")), QString(), this);
    mNextXDays->setEnabled(mCalendarView->currentView()->supportsDateRangeSelection());
    mACollection->addAction(QStringLiteral("select_nextx"), mNextXDays);
    connect(mNextXDays, &QAction::triggered, mCalendarView->viewManager(),
            &KOViewManager::selectNextX);
    mNextXDays->setText(i18np("&Next Day", "&Next %1 Days", KOPrefs::instance()->mNextXDays));

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-workweek")), i18n("W&ork Week"), this);
    action->setEnabled(mCalendarView->currentView()->supportsDateRangeSelection());
    mACollection->addAction(QStringLiteral("select_workweek"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(),
            &KOViewManager::selectWorkWeek);

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-calendar-week")), i18n("&Week"), this);
    action->setEnabled(mCalendarView->currentView()->supportsDateRangeSelection());
    mACollection->addAction(QStringLiteral("select_week"), action);
    connect(action, &QAction::triggered, mCalendarView->viewManager(),
            &KOViewManager::selectWeek);

    /************************** Actions MENU *********************************/
    mNewEventAction = new QAction(QIcon::fromTheme(QStringLiteral("appointment-new")), i18n("New E&vent..."), this);
    //mNewEventAction->setIconText( i18nc( "@action:intoolbar create a new event", "Event" ) );
    setHelpText(mNewEventAction, i18n("Create a new Event"));

    mACollection->addAction(QStringLiteral("new_event"), mNewEventAction);
    connect(mNewEventAction, &QAction::triggered, this,
            &ActionManager::slotNewEvent);

    mNewTodoAction = new QAction(QIcon::fromTheme(QStringLiteral("task-new")), i18n("New &To-do..."), this);
    //mNewTodoAction->setIconText( i18n( "To-do" ) );
    setHelpText(mNewTodoAction, i18n("Create a new To-do"));
    mACollection->addAction(QStringLiteral("new_todo"), mNewTodoAction);
    connect(mNewTodoAction, &QAction::triggered, this,
            &ActionManager::slotNewTodo);

    mNewSubtodoAction = new QAction(i18n("New Su&b-to-do..."), this);
    mACollection->addAction(QStringLiteral("new_subtodo"), mNewSubtodoAction);
    connect(mNewSubtodoAction, &QAction::triggered, this,
            &ActionManager::slotNewSubTodo);
    mNewSubtodoAction->setEnabled(false);
    connect(mCalendarView, &CalendarView::todoSelected, mNewSubtodoAction,
            &QAction::setEnabled);

    mNewJournalAction = new QAction(QIcon::fromTheme(QStringLiteral("journal-new")), i18n("New &Journal..."), this);
    //mNewJournalAction->setIconText( i18n( "Journal" ) );
    setHelpText(mNewJournalAction, i18n("Create a new Journal"));
    mACollection->addAction(QStringLiteral("new_journal"), mNewJournalAction);
    connect(mNewJournalAction, &QAction::triggered, this,
            &ActionManager::slotNewJournal);

    mConfigureViewAction = new QAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("Configure View..."), this);
    mConfigureViewAction->setIconText(i18n("Configure"));
    setHelpText(mConfigureViewAction, i18n("Configure the view"));
    mConfigureViewAction->setEnabled(mCalendarView->currentView() &&
                                     mCalendarView->currentView()->hasConfigurationDialog());
    mACollection->addAction(QStringLiteral("configure_view"), mConfigureViewAction);
    connect(mConfigureViewAction, &QAction::triggered, mCalendarView,
            &CalendarView::configureCurrentView);

    mShowIncidenceAction = new QAction(i18n("&Show"), this);
    mACollection->addAction(QStringLiteral("show_incidence"), mShowIncidenceAction);
    connect(mShowIncidenceAction, SIGNAL(triggered(bool)), mCalendarView,
            SLOT(showIncidence()));

    mEditIncidenceAction = new QAction(i18n("&Edit..."), this);
    mACollection->addAction(QStringLiteral("edit_incidence"), mEditIncidenceAction);
    connect(mEditIncidenceAction, SIGNAL(triggered(bool)), mCalendarView,
            SLOT(editIncidence()));

    mDeleteIncidenceAction = new QAction(i18n("&Delete"), this);
    mACollection->addAction(QStringLiteral("delete_incidence"), mDeleteIncidenceAction);
    connect(mDeleteIncidenceAction, SIGNAL(triggered(bool)), mCalendarView,
            SLOT(deleteIncidence()));
    mACollection->setDefaultShortcut(mDeleteIncidenceAction, QKeySequence(Qt::Key_Delete));

    action = new QAction(i18n("&Make Sub-to-do Independent"), this);
    mACollection->addAction(QStringLiteral("unsub_todo"), action);
    connect(action, &QAction::triggered, mCalendarView,
            &CalendarView::todo_unsub);
    action->setEnabled(false);
    connect(mCalendarView, &CalendarView::subtodoSelected, action,
            &QAction::setEnabled);

// TODO: Add item to quickly toggle the reminder of a given incidence
//   mToggleAlarmAction = new KToggleAction( i18n( "&Activate Reminder" ), 0,
//                                         mCalendarView, SLOT(toggleAlarm()),
//                                         mACollection, "activate_alarm" );

    /************************** SCHEDULE MENU ********************************/
    mPublishEvent = new QAction(QIcon::fromTheme(QStringLiteral("mail-send")), i18n("&Publish Item Information..."), this);
    mACollection->addAction(QStringLiteral("schedule_publish"), mPublishEvent);
    connect(mPublishEvent, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_publish()));
    mPublishEvent->setEnabled(false);

    mSendInvitation =
        new QAction(QIcon::fromTheme(QStringLiteral("mail-send")), i18n("Send &Invitation to Attendees"), this);
    mACollection->addAction(QStringLiteral("schedule_request"), mSendInvitation);
    connect(mSendInvitation, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_request()));
    mSendInvitation->setEnabled(false);
    connect(mCalendarView, &CalendarView::organizerEventsSelected,
            mSendInvitation, &QAction::setEnabled);

    mRequestUpdate = new QAction(i18n("Re&quest Update"), this);
    mACollection->addAction(QStringLiteral("schedule_refresh"), mRequestUpdate);
    connect(mRequestUpdate, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_refresh()));
    mRequestUpdate->setEnabled(false);
    connect(mCalendarView, &CalendarView::groupEventsSelected,
            mRequestUpdate, &QAction::setEnabled);

    mSendCancel = new QAction(i18n("Send &Cancellation to Attendees"), this);
    mACollection->addAction(QStringLiteral("schedule_cancel"), mSendCancel);
    connect(mSendCancel, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_cancel()));
    mSendCancel->setEnabled(false);
    connect(mCalendarView, &CalendarView::organizerEventsSelected,
            mSendCancel, &QAction::setEnabled);

    mSendStatusUpdate =
        new QAction(QIcon::fromTheme(QStringLiteral("mail-reply-sender")), i18n("Send Status &Update"), this);
    mACollection->addAction(QStringLiteral("schedule_reply"), mSendStatusUpdate);
    connect(mSendStatusUpdate, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_reply()));
    mSendStatusUpdate->setEnabled(false);
    connect(mCalendarView, &CalendarView::groupEventsSelected,
            mSendStatusUpdate, &QAction::setEnabled);

    mRequestChange = new QAction(i18nc("counter proposal", "Request Chan&ge"), this);
    mACollection->addAction(QStringLiteral("schedule_counter"), mRequestChange);
    connect(mRequestChange, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_counter()));
    mRequestChange->setEnabled(false);
    connect(mCalendarView, &CalendarView::groupEventsSelected,
            mRequestChange, &QAction::setEnabled);

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
    mTodoViewShowAction->setChecked(
        config.readEntry("TodoViewVisible", false));    //mIsPart ? false : true ) );
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
    mHideMenuBarAction = KStandardAction::showMenubar(this, SLOT(toggleMenubar()), mACollection);
    mHideMenuBarAction->setChecked(KOPrefs::instance()->showMenuBar());
    toggleMenubar(true);

    action = new QAction(i18n("Configure &Date && Time..."), this);
    mACollection->addAction(QStringLiteral("conf_datetime"), action);
    connect(action, &QAction::triggered,
            this, &ActionManager::configureDateTime);
// TODO: Add an item to show the resource management dlg
//   new QAction( i18n( "Manage &Calendars..." ), 0,
//                     this, SLOT(manageResources()),
//                     mACollection, "conf_resources" );

    action = new QAction(QIcon::fromTheme(QStringLiteral("view-filter")), i18n("Manage View &Filters..."), this);
    mACollection->addAction(QStringLiteral("edit_filters"), action);
    connect(action, &QAction::triggered, mCalendarView,
            &CalendarView::editFilters);

    action = new QAction(i18n("Manage C&ategories..."), this);
    mACollection->addAction(QStringLiteral("edit_categories"), action);
    connect(action, &QAction::triggered, mCalendarView->dialogManager(),
            &KODialogManager::showCategoryEditDialog);

    if (mIsPart) {
        action = new QAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("&Configure KOrganizer..."), this);
        mACollection->addAction(QStringLiteral("korganizer_configure"), action);
        connect(action, &QAction::triggered, mCalendarView,
                &CalendarView::edit_options);
        mACollection->addAction(KStandardAction::KeyBindings, QStringLiteral("korganizer_configure_shortcuts"),
                                this, SLOT(keyBindings()));
    } else {
        KStandardAction::preferences(mCalendarView, SLOT(edit_options()), mACollection);
        KStandardAction::keyBindings(this, SLOT(keyBindings()), mACollection);
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
    sender()->deleteLater();
    mImportAction->setEnabled(true);
    Akonadi::ICalImporter *importer = qobject_cast<Akonadi::ICalImporter *>(sender());

    if (success) {
        mCalendarView->showMessage(i18np("1 incidence was imported successfully.", "%1 incidences were imported successfully.", total), KMessageWidget::Information);
    } else {
        mCalendarView->showMessage(i18n("There was an error while merging the calendar: %1", importer->errorMessage()), KMessageWidget::Error);
    }
}

void ActionManager::slotNewResourceFinished(bool success)
{
    Q_ASSERT(sender());
    sender()->deleteLater();
    Akonadi::ICalImporter *importer = qobject_cast<Akonadi::ICalImporter *>(sender());
    mImportAction->setEnabled(true);
    if (success) {
        mCalendarView->showMessage(i18n("New calendar added successfully"), KMessageWidget::Information);
    } else {
        mCalendarView->showMessage(i18n("Could not add a calendar. Error: %1", importer->errorMessage()), KMessageWidget::Error);
    }
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

/*
void ActionManager::file_new()
{
  Q_EMIT actionNewMainWindow();
}
*/

void ActionManager::file_open()
{
    const QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/korganizer/");
    const QUrl url = QFileDialog::getOpenFileUrl(dialogParent(), QString(), QUrl::fromLocalFile(defaultPath), QStringLiteral("text/calendar"));

    file_open(url);
}

void ActionManager::file_open(const QUrl &url)
{
    if (url.isEmpty()) {
        return;
    }

    // is that URL already opened somewhere else? Activate that window
    KOrg::MainWindow *korg = ActionManager::findInstance(url);
    if ((Q_NULLPTR != korg) && (korg != mMainWindow)) {
#if KDEPIM_HAVE_X11
        KWindowSystem::activateWindow(korg->topLevelWidget()->winId());
#endif
        return;
    }

    qCDebug(KORGANIZER_LOG) << url.toDisplayString();

    importCalendar(url);
}

void ActionManager::file_icalimport()
{
    // FIXME: eventually, we will need a dialog box to select import type, etc.
    // for now, hard-coded to ical file, $HOME/.calendar.
    int retVal = -1;
    QString progPath;
    QTemporaryFile tmpfn;
    tmpfn.open();

    QString homeDir = QDir::homePath() + QLatin1String("/.calendar");

    if (!QFile::exists(homeDir)) {
        KMessageBox::error(dialogParent(),
                           i18n("You have no ical file in your home directory.\n"
                                "Import cannot proceed.\n"));
        return;
    }

    KProcess proc;
    proc << QStringLiteral("ical2vcal") << tmpfn.fileName();
    retVal = proc.execute();

    if (retVal < 0) {
        qCDebug(KORGANIZER_LOG) << "Error executing ical2vcal.";
        return;
    }

    qCDebug(KORGANIZER_LOG) << "ical2vcal return value:" << retVal;

    if (retVal >= 0 && retVal <= 2) {
        // now we need to MERGE what is in the iCal to the current calendar.
        const bool success = importURL(QUrl::fromLocalFile(tmpfn.fileName()), /*merge=*/ true);
        if (!success) {
            mCalendarView->showMessage(i18n("KOrganizer encountered some unknown fields while "
                                            "parsing your .calendar ical file, and had to "
                                            "discard them; please check to see that all "
                                            "your relevant data was correctly imported."), KMessageWidget::Warning);

        } else {
            // else nothing, the operation is async and the use will se a message widget when the operation finishes, not now.
        }
    } else if (retVal == -1) {   // XXX this is bogus
        mCalendarView->showMessage(i18n("KOrganizer encountered an error parsing your "
                                        ".calendar file from ical; import has failed."), KMessageWidget::Error);
    } else if (retVal == -2) {   // XXX this is bogus
        mCalendarView->showMessage(i18n("KOrganizer does not think that your .calendar "
                                        "file is a valid ical calendar; import has failed."), KMessageWidget::Error);
    }
}

void ActionManager::file_import()
{
    const QUrl url = QFileDialog::getOpenFileUrl(dialogParent(), QString(), QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/korganizer/")),
                     QStringLiteral("text/calendar"));
    if (!url.isEmpty()) {   // isEmpty if user canceled the dialog
        importCalendar(url);
    }
}

void ActionManager::file_archive()
{
    mCalendarView->archiveCalendar();
}

bool ActionManager::importURL(const QUrl &url, bool merge)
{
    Akonadi::ICalImporter *importer = new Akonadi::ICalImporter();
    bool jobStarted;
    if (merge) {
        connect(importer, &Akonadi::ICalImporter::importIntoExistingFinished, this, &ActionManager::slotMergeFinished),
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
    QString ext;

    if (mURL.isLocalFile()) {
        ext = mFile.right(4);
    } else {
        ext = mURL.fileName().right(4);
    }

    if (ext == QLatin1String(".vcs")) {
        int result = KMessageBox::warningContinueCancel(
                         dialogParent(),
                         i18n("Your calendar will be saved in iCalendar format. Use "
                              "'Export vCalendar' to save in vCalendar format."),
                         i18n("Format Conversion"), KGuiItem(i18n("Proceed")),
                         KStandardGuiItem::cancel(),
                         QStringLiteral("dontaskFormatConversion"), KMessageBox::Notify);
        if (result != KMessageBox::Continue) {
            return false;
        }

        QString filename = mURL.fileName();
        filename.replace(filename.length() - 4, 4, QStringLiteral(".ics"));
        mURL = mURL.adjusted(QUrl::RemoveFilename);
        mURL.setPath(mURL.path() + filename);
        if (mURL.isLocalFile()) {
            mFile = mURL.toLocalFile();
        }
        setTitle();
    }

    if (!mCalendarView->saveCalendar(mFile)) {
        qCDebug(KORGANIZER_LOG) << "calendar view save failed.";
        return false;
    }

    if (!mURL.isLocalFile()) {
        auto job = KIO::file_copy(QUrl::fromLocalFile(mFile), mURL);
        KJobWidgets::setWindow(job, view());
        if (!job->exec()) {
            QString msg = i18n("Cannot upload calendar to '%1'",
                               mURL.toDisplayString());
            KMessageBox::error(dialogParent(), msg);
            return false;
        }
    }

    mMainWindow->showStatusMessage(i18n("Saved calendar '%1'.", mURL.toDisplayString()));

    return true;
}

void ActionManager::exportHTML()
{
    HTMLExportSettings *settings = new HTMLExportSettings(QStringLiteral("KOrganizer"));
    mSettingsToFree.insert(settings);
    // Manually read in the config, because parametrized kconfigxt objects don't
    // seem to load the config theirselves
    settings->load();

    const QDate qd1 = QDate::currentDate();
    QDate qd2 = qd1;

    if (settings->monthView()) {
        qd2 = qd2.addMonths(1);
    } else {
        qd2 = qd2.addDays(7);
    }
    settings->setDateStart(QDateTime(qd1));
    settings->setDateEnd(QDateTime(qd2));

    exportHTML(settings, true/*autoMode*/);
}

void ActionManager::exportHTML(KOrg::HTMLExportSettings *settings, bool autoMode)
{
    if (!settings) {
        qCWarning(KORGANIZER_LOG) << "Settings is null" << settings;
        return;
    }

    if (settings->outputFile().isEmpty()) {
        int result = KMessageBox::questionYesNo(
                         dialogParent(),
                         i18n("The HTML calendar export file has not been specified yet.\n"
                              "Do you want to set it now?\n\n"
                              "If you answer \"no\" then this export operation will be canceled"),
                         QString());
        if (result == KMessageBox::No) {
            mMainWindow->showStatusMessage(
                i18nc("@info:status",
                      "Calendar HTML operation canceled due to unspecified output file name"));
            return;
        }

        const QString fileName =
            QFileDialog::getSaveFileName(dialogParent(), i18n("Select path for HTML calendar export"),
                                         QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
                                         i18n("HTML Files (*.html)"));
        settings->setOutputFile(fileName);
        settings->save();
    }

    if (!autoMode && QFileInfo(settings->outputFile()).exists()) {
        if (KMessageBox::warningContinueCancel(
                    dialogParent(),
                    i18n("Do you want to overwrite file \"%1\"?",
                         settings->outputFile()),
                    QString(),
                    KStandardGuiItem::overwrite()) == KMessageBox::Cancel) {
            mMainWindow->showStatusMessage(
                i18nc("@info:status",
                      "Calendar HTML operation canceled due to output file overwrite"));
            return;
        }
    }

    settings->setEMail(CalendarSupport::KCalPrefs::instance()->email());
    settings->setName(CalendarSupport::KCalPrefs::instance()->fullName());

    settings->setCreditName(QStringLiteral("KOrganizer"));
    settings->setCreditURL(QStringLiteral("http://korganizer.kde.org"));

    KOrg::HtmlExportJob *exportJob = new KOrg::HtmlExportJob(calendar(), settings, autoMode, mMainWindow, view());

    if (KOGlobals::self()->holidays()) {
        KHolidays::Holiday::List holidays = KOGlobals::self()->holidays()->holidays(
                                                settings->dateStart().date(), settings->dateEnd().date());
        foreach (const KHolidays::Holiday &holiday, holidays) {
            exportJob->addHoliday(holiday.observedStartDate(), holiday.name());
        }
    }

    connect(exportJob, &KOrg::HtmlExportJob::result, this, &ActionManager::handleExportJobResult);
    exportJob->start();
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

    QTemporaryFile *tempFile = Q_NULLPTR;
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
        KIO::NetAccess::removeTempFile(fileOrig);
        setTitle();
    } else {
        KMessageBox::sorry(dialogParent(),
                           i18n("Unable to save calendar to the file %1.", mFile),
                           i18n("Error"));
        qCDebug(KORGANIZER_LOG) << "failed";
        mURL = URLOrig;
        mFile = fileOrig;
        delete tempFile;
    }

    return success;
}

#ifdef AKONADI_PORT_DISABLED // can go away, kept for reference
bool ActionManager::saveModifiedURL()
{
    qCDebug(KORGANIZER_LOG);

    // If calendar isn't modified do nothing.
    if (!mCalendarView->isModified()) {
        return true;
    }

    mHtmlExportSync = true;
    if (KOPrefs::instance()->mAutoSave && !mURL.isEmpty()) {
        // Save automatically, when auto save is enabled.
        return saveURL();
    } else {
        int result = KMessageBox::warningYesNoCancel(
                         dialogParent(),
                         i18n("The calendar has been modified.\nDo you want to save it?"),
                         QString(),
                         KStandardGuiItem::save(), KStandardGuiItem::discard());
        switch (result) {
        case KMessageBox::Yes:
            if (mURL.isEmpty()) {
                QUrl url = getSaveURL();
                return saveAsURL(url);
            } else {
                return saveURL();
            }
        case KMessageBox::No:
            return true;
        case KMessageBox::Cancel:
        default: {
            mHtmlExportSync = false;
            return false;
        }
        }
    }
}
#endif

QUrl ActionManager::getSaveURL()
{
    QUrl url =
        QFileDialog::getSaveFileUrl(dialogParent(), QString(), QString(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/korganizer/")),
                                    i18n("*.ics *.vcs|Calendar Files")
                                   );

    if (!url.isValid()) {
        return url;
    }

    QString filename = url.fileName();

    QString e = filename.right(4);
    if (e != QLatin1String(".vcs") && e != QLatin1String(".ics")) {
        // Default save format is iCalendar
        filename += QLatin1String(".ics");
    }
    url = url.adjusted(QUrl::RemoveFilename);
    url.setPath(url.path() + filename);

    qCDebug(KORGANIZER_LOG) << "url:" << url.url();

    return url;
}

void ActionManager::saveProperties(KConfigGroup &config)
{
    qCDebug(KORGANIZER_LOG);

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
    qCDebug(KORGANIZER_LOG);
    mNextXDays->setText(i18np("&Next Day", "&Next %1 Days",
                              KOPrefs::instance()->mNextXDays));

    KOCore::self()->reloadPlugins();

    /* Hide/Show the Reminder Daemon */
    if (!KOPrefs::instance()->mShowReminderDaemon) {
        KPIM::ReminderClient::hideDaemon();
    } else {
        KPIM::ReminderClient::showDaemon();
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
        KMessageBox::sorry(dialogParent(),
                           i18n("Could not start control module for date and time format."));
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
        return Q_NULLPTR;
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
        if (parser.isSet(QStringLiteral("import"))) {
            for (const QString &url : parser.positionalArguments()) {
                importURL(QUrl::fromUserInput(url), /*merge=*/false);
            }
        } else if (parser.isSet(QStringLiteral("merge"))) {
            for (const QString &url : parser.positionalArguments()) {
                importURL(QUrl::fromUserInput(url), /*merge=*/true);
            }
        } else {
            for (const QString &url : parser.positionalArguments()) {
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
    qCDebug(KORGANIZER_LOG);
    KNS3::DownloadDialog dialog(mCalendarView);
    dialog.exec();
    foreach (const KNS3::Entry &e, dialog.installedEntries()) {
        qCDebug(KORGANIZER_LOG) << " downloadNewStuff :";
        const QStringList lstFile = e.installedFiles();
        if (lstFile.count() != 1) {
            continue;
        }
        const QString file = lstFile.at(0);
        const QUrl filename = QUrl::fromLocalFile(file);
        qCDebug(KORGANIZER_LOG) << "filename :" << filename;
        if (! filename.isValid()) {
            continue;
        }

        KCalCore::FileStorage storage(calendar());
        storage.setFileName(file);
        storage.setSaveFormat(new KCalCore::ICalFormat);
        if (!storage.load()) {
            KMessageBox::error(mCalendarView, i18n("Could not load calendar %1.", file));
        } else {
            QStringList eventSummaries;
            KCalCore::Event::List events = calendar()->events();
            eventSummaries.reserve(events.count());
            foreach (const KCalCore::Event::Ptr &event, events) {
                eventSummaries.append(event->summary());
            }

            const int result = KMessageBox::warningContinueCancelList(mCalendarView,
                               i18n("The downloaded events will be merged into your current calendar."),
                               eventSummaries);

            if (result != KMessageBox::Continue) {
                // FIXME (KNS2): hm, no way out here :-)
            }

            if (importURL(QUrl::fromLocalFile(file), true)) {
                // FIXME (KNS2): here neither
            }
        }
    }
}

QString ActionManager::localFileName()
{
    return mFile;
}

class ActionManager::ActionStringsVisitor : public KCalCore::Visitor
{
public:
    ActionStringsVisitor() : mShow(Q_NULLPTR), mEdit(Q_NULLPTR), mDelete(Q_NULLPTR) {}

    bool act(KCalCore::IncidenceBase::Ptr incidence, QAction *show, QAction *edit, QAction *del)
    {
        mShow = show;
        mEdit = edit;
        mDelete = del;
        return incidence->accept(*this, incidence);
    }

protected:
    bool visit(const KCalCore::Event::Ptr &) Q_DECL_OVERRIDE {
        if (mShow)
        {
            mShow->setText(i18n("&Show Event"));
        }
        if (mEdit)
        {
            mEdit->setText(i18n("&Edit Event..."));
        }
        if (mDelete)
        {
            mDelete->setText(i18n("&Delete Event"));
        }
        return true;
    }

    bool visit(const KCalCore::Todo::Ptr &) Q_DECL_OVERRIDE {
        if (mShow)
        {
            mShow->setText(i18n("&Show To-do"));
        }
        if (mEdit)
        {
            mEdit->setText(i18n("&Edit To-do..."));
        }
        if (mDelete)
        {
            mDelete->setText(i18n("&Delete To-do"));
        }
        return true;
    }

    bool visit(const KCalCore::Journal::Ptr &) Q_DECL_OVERRIDE {
        return assignDefaultStrings();
    }

    bool visit(const KCalCore::FreeBusy::Ptr &) Q_DECL_OVERRIDE { // to inhibit hidden virtual compile warning
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
    QAction *mShow;
    QAction *mEdit;
    QAction *mDelete;
};

void ActionManager::processIncidenceSelection(const Akonadi::Item &item, const QDate &date)
{
    //qCDebug(KORGANIZER_LOG) << "ActionManager::processIncidenceSelection()";
    Q_UNUSED(date);

    const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
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
    KShortcutsDialog dlg(KShortcutsEditor::AllActions,
                         KShortcutsEditor::LetterShortcutsDisallowed, view());
    if (mMainWindow) {
        dlg.addCollection(mMainWindow->getActionCollection());
    }

    foreach (KOrg::Part *part, mParts) {
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

void ActionManager::openEventEditor(const QString &text)
{
    mCalendarView->newEvent(text);
}

void ActionManager::openEventEditor(const QString &summary,
                                    const QString &description,
                                    const QStringList &attachments)
{
    mCalendarView->newEvent(summary, description, attachments);
}

void ActionManager::openEventEditor(const QString &summary,
                                    const QString &description,
                                    const QStringList &attachments,
                                    const QStringList &attendees)
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
    } else if (IncidenceEditorNG::IncidenceEditorSettings::self()->defaultEmailAttachMethod() ==
               IncidenceEditorNG::IncidenceEditorSettings::Ask) {
        QMenu *menu = new QMenu(Q_NULLPTR);
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
        KMime::Message *msg = new KMime::Message();
        msg->setContent(f.readAll());
        msg->parse();
        if (msg == msg->textContent() || msg->textContent() == Q_NULLPTR) {   // no attachments
            attData = file;
        } else {
            if (KMessageBox::warningContinueCancel(
                        Q_NULLPTR,
                        i18n("Removing attachments from an email might invalidate its signature."),
                        i18n("Remove Attachments"), KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
                        QStringLiteral("BodyOnlyInlineAttachment")) != KMessageBox::Continue) {
                delete msg;
                return;
            }
            KMime::Message *newMsg = new KMime::Message();
            newMsg->setHead(msg->head());
            newMsg->setBody(msg->textContent()->body());
            newMsg->parse();
            newMsg->contentTransferEncoding()->from7BitString(
                msg->textContent()->contentTransferEncoding()->as7BitString());
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

    mCalendarView->newEvent(summary, description, QStringList(attData),
                            attendees, QStringList(attachmentMimetype),
                            action != IncidenceEditorNG::IncidenceEditorSettings::Link);
}

void ActionManager::openTodoEditor(const QString &text)
{
    mCalendarView->newTodo(text);
}

void ActionManager::openTodoEditor(const QString &summary,
                                   const QString &description,
                                   const QStringList &attachments)
{
    mCalendarView->newTodo(summary, description, attachments);
}
void ActionManager::openTodoEditor(const QString &summary,
                                   const QString &description,
                                   const QStringList &attachments,
                                   const QStringList &attendees)
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
        QMenu *menu = new QMenu(Q_NULLPTR);
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

    mCalendarView->newTodo(summary, description, QStringList(attData),
                           attendees, QStringList(attachmentMimetype),
                           action != KOPrefs::TodoAttachLink);
}

void ActionManager::openJournalEditor(const QDate &date)
{
    mCalendarView->newJournal(date);
}

void ActionManager::openJournalEditor(const QString &text, const QDate &date)
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

void ActionManager::goDate(const QDate &date)
{
    mCalendarView->goDate(date);
}

void ActionManager::goDate(const QString &date)
{
    goDate(KLocale::global()->readDate(date));
}

void ActionManager::showDate(const QDate &date)
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
        KMessageBox::error(dialogParent(),
                           i18n("URL '%1' is invalid.", url.toDisplayString()));
        return;
    }

    const QString questionText =
        i18nc("@info",
              "<p>Would you like to merge this calendar item into an existing calendar "
              "or use it to create a brand new calendar?</p>"
              "<p>If you select merge, then you will be given the opportunity to select "
              "the destination calendar.</p>"
              "<p>If you select add, then a new calendar will be created for you automatically.</p>");

    const int answer =
        KMessageBox::questionYesNoCancel(
            dialogParent(),
            questionText,
            i18nc("@title:window", "Import Calendar"),
            KGuiItem(i18n("Merge into existing calendar")),
            KGuiItem(i18n("Add as new calendar")));

    switch (answer)  {
    case KMessageBox::Yes: //merge
        importURL(url, true);
        break;
    case KMessageBox::No:  //import
        importURL(url, false);
        break;
    default:
        return;
    }
}

void ActionManager::slotAutoArchivingSettingsModified()
{
    if (CalendarSupport::KCalPrefs::instance()->mAutoArchive) {
        mAutoArchiveTimer->start(4 * 60 * 60 * 1000);   // check again in 4 hours
    } else {
        mAutoArchiveTimer->stop();
    }
}

void ActionManager::slotAutoArchive()
{
    if (!mCalendarView->calendar()) {   // can this happen?
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

void ActionManager::checkAutoExport()
{
    // Don't save if auto save interval is zero
    if (KOPrefs::instance()->mAutoExportInterval == 0) {
        return;
    }

    // has this calendar been saved before? If yes automatically save it.
    if (KOPrefs::instance()->mAutoExport) {
        exportHTML();
    }
}

void ActionManager::openTodoEditor(const QString &summary,
                                   const QString &description,
                                   const QStringList &attachmentUris,
                                   const QStringList &attendees,
                                   const QStringList &attachmentMimetypes,
                                   bool attachmentIsInline)
{
    Q_UNUSED(summary);
    Q_UNUSED(description);
    Q_UNUSED(attachmentUris);
    Q_UNUSED(attendees);
    Q_UNUSED(attachmentMimetypes);
    Q_UNUSED(attachmentIsInline);
    qCWarning(KORGANIZER_LOG) << "Not implemented in korg-desktop";
}

void ActionManager::openEventEditor(const QString &summary,
                                    const QString &description,
                                    const QStringList &attachmentUris,
                                    const QStringList &attendees,
                                    const QStringList &attachmentMimetypes,
                                    bool attachmentIsInline)
{
    Q_UNUSED(summary);
    Q_UNUSED(description);
    Q_UNUSED(attachmentUris);
    Q_UNUSED(attendees);
    Q_UNUSED(attachmentMimetypes);
    Q_UNUSED(attachmentIsInline);
    qCWarning(KORGANIZER_LOG) << "Not implemented in korg-desktop";
}

void ActionManager::handleExportJobResult(KJob *job)
{
    HtmlExportJob *htmlExportJob = qobject_cast<HtmlExportJob *>(job);
    Q_ASSERT(htmlExportJob);

    if (mSettingsToFree.contains(htmlExportJob->settings())) {
        mSettingsToFree.remove(htmlExportJob->settings());
        delete htmlExportJob->settings();
    }
}

void ActionManager::setHelpText(QAction *act, const QString &text)
{
    act->setStatusTip(text);
    act->setToolTip(text);
    if (act->whatsThis().isEmpty()) {
        act->setWhatsThis(text);
    }
}

