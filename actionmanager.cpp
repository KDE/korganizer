/*
  This file is part of KOrganizer.

  Copyright (c) 2002 Mike Pilone <mpilone@slac.com>
  Copyright (c) 2002 Don Sanders <sanders@kde.org>
  Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

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

#include "actionmanager.h"
#include "calendaradaptor.h"
#include "calendarview.h"
#include "eventarchiver.h"
#include "freebusymanager.h"
#include "history.h"
#include "importdialog.h"
#include "kocore.h"
#include "kodialogmanager.h"
#include "koglobals.h"
#include "kogroupware.h"
#include "koprefs.h"
#include "koviewmanager.h"
#include "kowindowlist.h"
#include "reminderclient.h"
#include "resourceview.h"
#include "stdcalendar.h"

#include <KCal/CalendarLocal>
#include <KCal/FileStorage>
#include <KCal/HtmlExport>
#include <kcal/htmlexportsettings.h> //krazy:exclude=camelcase. it's generated
#include <KMime/KMimeMessage>

#include <KAction>
#include <KActionCollection>
#include <KFileDialog>
#include <KMenu>
#include <KNotification>
#include <KProcess>
#include <KRecentFilesAction>
#include <KSelectAction>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KStandardDirs>
#include <KTemporaryFile>
#include <KTipDialog>
#include <KToggleAction>
#include <KWindowSystem>
#include <KIO/NetAccess>
#include <KNS/Engine>

#include <QApplication>
#include <QTimer>

// FIXME: Several places in the file don't use KConfigXT yet!
KOWindowList *ActionManager::mWindowList = 0;

ActionManager::ActionManager( KXMLGUIClient *client, CalendarView *widget,
                              QObject *parent, KOrg::MainWindow *mainWindow,
                              bool isPart, KMenuBar *menuBar )
  : QObject( parent ), mRecent( 0 ),
    mResourceViewShowAction( 0 ), mCalendar( 0 ),
    mCalendarResources( 0 ), mResourceView( 0 ), mIsClosing( false )
{
  new CalendarAdaptor( this );
  QDBusConnection::sessionBus().registerObject( "/Calendar", this );

  mGUIClient = client;
  mACollection = mGUIClient->actionCollection();
  mCalendarView = widget;
  mIsPart = isPart;
  mTempFile = 0;
  mHtmlExportSync = false;
  mMainWindow = mainWindow;
  mMenuBar = menuBar;
}

ActionManager::~ActionManager()
{
  // Remove Part plugins
  KOCore::self()->unloadParts( mMainWindow, mParts );

  delete mTempFile;

  // Take this window out of the window list.
  mWindowList->removeWindow( mMainWindow );

  delete mCalendarView;
  delete mCalendar;
}

void ActionManager::toggleMenubar( bool dontShowWarning )
{
  if ( mMenuBar ) {
    if ( mHideMenuBarAction->isChecked() ) {
      mMenuBar->show();
    } else {
      if ( !dontShowWarning ) {
        QString accel = mHideMenuBarAction->shortcut().toString();
        KMessageBox::information( mCalendarView,
                                  i18n( "<qt>This will hide the menu bar completely."
                                        " You can show it again by typing %1.</qt>", accel ),
                                  "Hide menu bar", "HideMenuBarWarning" );
      }
      mMenuBar->hide();
    }
    KOPrefs::instance()->setShowMenuBar( mHideMenuBarAction->isChecked() );
  }
}
// see the Note: below for why this method is necessary
void ActionManager::init()
{
  // Construct the groupware object
  KOGroupware::create( mCalendarView, mCalendarResources );

  // add this instance of the window to the static list.
  if ( !mWindowList ) {
    mWindowList = new KOWindowList;
    // Show tip of the day, when the first calendar is shown.
    if ( !mIsPart ) {
      QTimer::singleShot( 0, this, SLOT(showTipOnStart()) );
    }
  }

  // Note: We need this ActionManager to be fully constructed, and
  // parent() to have a valid reference to it before the following
  // addWindow is called.
  mWindowList->addWindow( mMainWindow );

  initActions();

  // set up autoSaving stuff
  mAutoSaveTimer = new QTimer( this );
  connect( mAutoSaveTimer, SIGNAL(timeout()), SLOT(checkAutoSave()) );
  if ( KOPrefs::instance()->mAutoSave &&
       KOPrefs::instance()->mAutoSaveInterval > 0 ) {
    mAutoSaveTimer->start( 1000 * 60 * KOPrefs::instance()->mAutoSaveInterval );
  }

  mAutoArchiveTimer = new QTimer( this );
  mAutoArchiveTimer->setSingleShot( true );
  connect( mAutoArchiveTimer, SIGNAL(timeout()), SLOT(slotAutoArchive()) );

  // First auto-archive should be in 5 minutes (like in kmail).
  if ( KOPrefs::instance()->mAutoArchive ) {
    mAutoArchiveTimer->start( 5 * 60 * 1000 ); // singleshot
  }

  setTitle();

  connect( mCalendarView, SIGNAL(modifiedChanged(bool)), SLOT(setTitle()) );
  connect( mCalendarView, SIGNAL(configChanged()), SLOT(updateConfig()) );

  connect( mCalendarView, SIGNAL(incidenceSelected(Incidence *)),
           this, SLOT(processIncidenceSelection(Incidence *)) );
  connect( mCalendarView, SIGNAL(exportHTML(HTMLExportSettings *)),
           this, SLOT(exportHTML(HTMLExportSettings *)) );

  processIncidenceSelection( 0 );

  // Update state of paste action
  mCalendarView->checkClipboard();
}

void ActionManager::createCalendarLocal()
{
  mCalendar = new CalendarLocal( KOPrefs::instance()->timeSpec() );
  mCalendarView->setCalendar( mCalendar );
  mCalendarView->readSettings();

  initCalendar( mCalendar );
}

void ActionManager::createCalendarResources()
{
  mCalendarResources = KOrg::StdCalendar::self();

  CalendarResourceManager *manager = mCalendarResources->resourceManager();

  kDebug() << "CalendarResources used by KOrganizer:";
  CalendarResourceManager::Iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it ) {
    kDebug() << (*it)->resourceName();
    (*it)->setResolveConflict( true );
  }

  setDestinationPolicy();

  mCalendarView->setCalendar( mCalendarResources );
  mCalendarView->readSettings();

  ResourceViewFactory factory( mCalendarResources, mCalendarView );
  mCalendarView->addExtension( &factory );
  mResourceView = factory.resourceView();

  connect( mCalendarResources, SIGNAL(calendarChanged()),
           mCalendarView, SLOT(resourcesChanged()) );
  connect( mCalendarResources, SIGNAL(calendarLoaded()),
           mCalendarView, SLOT(resourcesChanged()) );
  connect( mCalendarResources, SIGNAL(signalErrorMessage(const QString &)),
           mCalendarView, SLOT(showErrorMessage(const QString &)) );

  connect( mCalendarView, SIGNAL(configChanged()), SLOT(updateConfig()) );

  initCalendar( mCalendarResources );
}

void ActionManager::initCalendar( Calendar *cal )
{
  cal->setOwner( Person( KOPrefs::instance()->fullName(),
                         KOPrefs::instance()->email() ) );
  // setting fullName and email do not really count as modifying the calendar
  mCalendarView->setModified( false );
}

void ActionManager::initActions()
{
  KAction *action;

  /*************************** FILE MENU **********************************/

  //~~~~~~~~~~~~~~~~~~~~~~~ LOADING / SAVING ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if ( mIsPart ) {
    if ( mMainWindow->hasDocument() ) {
      KAction *a = mACollection->addAction( KStandardAction::New, this, SLOT(file_new()) );
      mACollection->addAction( "korganizer_openNew", a );

      a = mACollection->addAction( KStandardAction::Open, this, SLOT(file_open()) );
      mACollection->addAction( "korganizer_open", a );

      mRecent = KStandardAction::openRecent( this, SLOT(file_open(const KUrl&)), mACollection );
      mACollection->addAction( "korganizer_openRecent", mRecent );

      mACollection->addAction( KStandardAction::Revert, "korganizer_revert",
                               this, SLOT(file_revert()) );

      mACollection->addAction( KStandardAction::SaveAs, this, SLOT(file_saveas()) );
      mACollection->addAction( "korganizer_saveAs", a );

      mACollection->addAction( KStandardAction::Save, this, SLOT(file_save()) );
      mACollection->addAction( "korganizer_save", a );
    }

    QAction *a = mACollection->addAction( KStandardAction::Print, mCalendarView, SLOT(print()) );
    mACollection->addAction( "korganizer_print", a );
    a = mACollection->addAction( KStandardAction::PrintPreview, mCalendarView, SLOT(print()) );
    mACollection->addAction( "korganizer_print_preview", a );
  } else {
    KStandardAction::openNew( this, SLOT(file_new()), mACollection );
    KStandardAction::open( this, SLOT(file_open()), mACollection );
    mRecent = KStandardAction::openRecent( this, SLOT(file_open(const KUrl&)), mACollection );
    if ( mMainWindow->hasDocument() ) {
      KStandardAction::revert( this, SLOT(file_revert()), mACollection );
      KStandardAction::saveAs( this, SLOT(file_saveas()), mACollection );
      KStandardAction::save( this, SLOT(file_save()), mACollection );
    }
    KStandardAction::print( mCalendarView, SLOT(print()), mACollection );
    KStandardAction::printPreview( mCalendarView, SLOT(printPreview()), mACollection );
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~ IMPORT / EXPORT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  KAction *mergeAction = new KAction( i18n( "Import &Calendar..." ), this );
  mergeAction->setHelpText(
    i18n( "Merge the contents of another iCalendar" ) );
  mergeAction->setWhatsThis(
    i18n( "Select this menu entry if you would like to merge the contents "
          "of another iCalendar into your current calendar." ) );
  mACollection->addAction( "import_icalendar", mergeAction );
  connect( mergeAction, SIGNAL(triggered(bool)), SLOT(file_merge()) );

  KAction *importAction = new KAction( i18n( "&Import From UNIX Ical Tool" ), this );
  importAction->setHelpText(
    i18n( "Import a calendar in another format" ) );
  importAction->setWhatsThis(
    i18n( "Select this menu entry if you would like to import the contents "
          "of a non-iCalendar formatted file into your current calendar." ) );
  mACollection->addAction( "import_ical", importAction );
  connect( importAction, SIGNAL(triggered(bool)), SLOT(file_icalimport()) );

  action = new KAction( i18n( "Get &Hot New Stuff..." ), this );
  mACollection->addAction( "downloadnewstuff", action );
  connect( action, SIGNAL(triggered(bool)), SLOT(downloadNewStuff()) );

  action = new KAction( KIcon( "document-export-html" ), i18n( "Export &Web Page..." ), this );
  mACollection->addAction( "export_web", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(exportWeb()) );

  action = new KAction( i18n( "&iCalendar..." ), this );
  mACollection->addAction( "export_icalendar", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(exportICalendar()) );

  action = new KAction( i18n( "&vCalendar..." ), this );
  mACollection->addAction( "export_vcalendar", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(exportVCalendar()) );

  //Laurent: 2009-03-24 comment it until upload will implement
  //action = new KAction( i18n( "Upload &Hot New Stuff..." ), this );
  //mACollection->addAction( "uploadnewstuff", action );
  //connect( action, SIGNAL(triggered(bool)), SLOT(uploadNewStuff()) );

  action = new KAction( i18n( "Archive O&ld Entries..." ), this );
  mACollection->addAction( "file_archive", action );
  connect( action, SIGNAL(triggered(bool)), SLOT(file_archive()) );

  action = new KAction( i18n( "Pur&ge Completed To-dos" ), mACollection );
  mACollection->addAction( "purge_completed", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(purgeCompleted()) );

  /************************** EDIT MENU *********************************/

  QAction *pasteAction;
  KOrg::History *h = mCalendarView->history();
  if ( mIsPart ) {
    // edit menu
    mCutAction = mACollection->addAction( KStandardAction::Cut, "korganizer_cut",
                                          mCalendarView, SLOT(edit_cut()) );
    mCopyAction = mACollection->addAction( KStandardAction::Copy, "korganizer_copy",
                                           mCalendarView, SLOT(edit_copy()) );
    pasteAction = mACollection->addAction( KStandardAction::Paste, "korganizer_paste",
                                           mCalendarView, SLOT(edit_paste()) );
    mUndoAction = mACollection->addAction( KStandardAction::Undo, "korganizer_undo",
                                           h, SLOT(undo()) );
    mRedoAction = mACollection->addAction( KStandardAction::Redo, "korganizer_redo",
                                           h, SLOT(redo()) );
  } else {
    mCutAction = KStandardAction::cut( mCalendarView, SLOT(edit_cut()), mACollection );
    mCopyAction = KStandardAction::copy( mCalendarView, SLOT(edit_copy()), mACollection );
    pasteAction = KStandardAction::paste( mCalendarView, SLOT(edit_paste()), mACollection );
    mUndoAction = KStandardAction::undo( h, SLOT(undo()), mACollection );
    mRedoAction = KStandardAction::redo( h, SLOT(redo()), mACollection );
  }
  mDeleteAction = new KAction( KIcon( "edit-delete" ), i18n( "&Delete" ), this );
  mACollection->addAction( "edit_delete", mDeleteAction );
  connect( mDeleteAction, SIGNAL(triggered(bool)), mCalendarView, SLOT(appointment_delete()) );
  if ( mIsPart ) {
    QAction *a =
      KStandardAction::find( mCalendarView->dialogManager(),
                             SLOT(showSearchDialog()), mACollection );
    mACollection->addAction( "korganizer_find", a );
  } else {
    KStandardAction::find( mCalendarView->dialogManager(),
                           SLOT(showSearchDialog()), mACollection );
  }
  pasteAction->setEnabled( false );
  mUndoAction->setEnabled( false );
  mRedoAction->setEnabled( false );
  connect( mCalendarView, SIGNAL(pasteEnabled(bool)), pasteAction, SLOT(setEnabled(bool)) );
  connect( h, SIGNAL(undoAvailable(const QString &)), SLOT(updateUndoAction(const QString &)) );
  connect( h, SIGNAL(redoAvailable(const QString &)), SLOT(updateRedoAction(const QString &)) );

  /************************** VIEW MENU *********************************/

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ VIEWS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  action = new KAction( KIcon( "view-calendar-upcoming-events" ), i18n( "What's &Next" ), this );
  mACollection->addAction( "view_whatsnext", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView->viewManager(),
           SLOT(showWhatsNextView()) );

  action = new KAction( KIcon( "view-calendar-day" ), i18n( "&Day" ), this );
  mACollection->addAction( "view_day", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView->viewManager(),
           SLOT(showDayView()) );

  mNextXDays = new KAction( KIcon( "view-calendar-upcoming-days" ), QString(), this );
  mACollection->addAction( "view_nextx", mNextXDays );
  connect( mNextXDays, SIGNAL(triggered(bool)), mCalendarView->viewManager(),
           SLOT(showNextXView()) );
  mNextXDays->setText( i18np( "&Next Day", "&Next %1 Days", KOPrefs::instance()->mNextXDays ) );
  action = new KAction( KIcon( "view-calendar-month" ), i18n( "&Month" ), this );
  mACollection->addAction( "view_month", action );
  connect( action, SIGNAL(triggered(bool)),
           mCalendarView->viewManager(), SLOT(showMonthView()) );

  action = new KAction( KIcon( "view-calendar-workweek" ), i18n( "W&ork Week" ), this );
  mACollection->addAction( "view_workweek", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView->viewManager(),
           SLOT(showWorkWeekView()) );

  action = new KAction( KIcon( "view-calendar-week" ), i18n( "&Week" ), this );
  mACollection->addAction( "view_week", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView->viewManager(),
           SLOT(showWeekView()) );

  action = new KAction( KIcon( "view-calendar-list" ), i18n( "&Event List" ), this );
  mACollection->addAction( "view_list", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView->viewManager(),
           SLOT(showListView()) );

  action = new KAction( KIcon( "view-calendar-tasks" ), i18n( "&To-do List" ), this );
  mACollection->addAction( "view_todo", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView->viewManager(),
           SLOT(showTodoView()) );

  action = new KAction( KIcon( "view-pim-journal" ), i18n( "&Journal" ), this );
  mACollection->addAction( "view_journal", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView->viewManager(),
           SLOT(showJournalView()) );

  action = new KAction( KIcon( "view-calendar-timeline" ), i18n( "Time&line" ), this );
  mACollection->addAction( "view_timeline", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView->viewManager(),
           SLOT(showTimeLineView()) );

  action = new KAction( KIcon( "view-calendar-time-spent" ), i18n( "Time&spent" ), this );
  mACollection->addAction( "view_timespent", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView->viewManager(),
           SLOT(showTimeSpentView()) );

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~ REFRESH ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  action = new KAction( i18n( "&Refresh" ), this );
  mACollection->addAction( "update", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(updateView()) );

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~ FILTER ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  mFilterAction = new KSelectAction( i18n( "F&ilter" ), this );
  mFilterAction->setToolBarMode( KSelectAction::MenuMode );
  mACollection->addAction( "filter_select", mFilterAction );
  mFilterAction->setEditable( false );
  connect( mFilterAction, SIGNAL(triggered(int)),
           mCalendarView, SLOT(filterActivated(int)) );
  connect( mCalendarView, SIGNAL(newFilterListSignal(const QStringList &)),
           this, SLOT(setItems( const QStringList &)) );
  connect( mCalendarView, SIGNAL(selectFilterSignal(int)),
           this, SLOT(slotChangeComboActionItem(int)) );
  connect( mCalendarView, SIGNAL(filterChanged()),
           this, SLOT(setTitle()) );

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ZOOM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // TODO: try to find / create better icons for the following 4 actions
  action = new KAction( KIcon( "zoom-in" ), i18n( "In Horizontally" ), this );
  mACollection->addAction( "zoom_in_horizontally", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView->viewManager(),
           SLOT(zoomInHorizontally()) );

  action = new KAction( KIcon( "zoom-out" ), i18n( "Out Horizontally" ), this );
  mACollection->addAction( "zoom_out_horizontally", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView->viewManager(),
           SLOT(zoomOutHorizontally()) );

  action = new KAction( KIcon( "zoom-in" ), i18n( "In Vertically" ), this );
  mACollection->addAction( "zoom_in_vertically", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView->viewManager(),
           SLOT(zoomInVertically()) );

  action = new KAction( KIcon( "zoom-out" ), i18n( "Out Vertically" ), this );
  mACollection->addAction( "zoom_out_vertically", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView->viewManager(),
           SLOT(zoomOutVertically()) );

  /************************** Actions MENU *********************************/
  bool isRTL = QApplication::isRightToLeft();

  action = new KAction( KIcon( "go-jump-today" ),
                        i18nc( "@action Jump to today", "To &Today" ), this );
  action->setIconText( i18n( "Today" ) );
  action->setHelpText( i18n( "Scroll to Today" ) );
  mACollection->addAction( "go_today", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(goToday()) );

  action = new KAction( KIcon( isRTL ? "go-next" : "go-previous" ),
                        i18nc( "scroll backward", "&Backward" ), this );
  action->setIconText( i18nc( "scroll backward", "Back" ) );
  action->setHelpText( i18n( "Scroll Backward" ) );
  mACollection->addAction( "go_previous", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(goPrevious()) );

  // Changing the action text by setText makes the toolbar button disappear.
  // This has to be fixed first, before the connects below can be reenabled.
  /*
  connect( mCalendarView, SIGNAL(changeNavStringPrev(const QString &)),
           action, SLOT(setText(const QString &)) );
  connect( mCalendarView, SIGNAL(changeNavStringPrev(const QString &)),
           this, SLOT(dumpText(const QString &)) );*/

  action = new KAction( KIcon( isRTL ? "go-previous" : "go-next" ),
                        i18nc( "scroll forward", "&Forward" ), this );
  action->setIconText( i18nc( "scoll forward", "Forward" ) );
  action->setHelpText( i18n( "Scroll Forward" ) );
  mACollection->addAction( "go_next", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(goNext()) );
  /*
  connect( mCalendarView,SIGNAL(changeNavStringNext(const QString &)),
           action,SLOT(setText(const QString &)) );
  */

  /************************** Actions MENU *********************************/
  action = new KAction( KIcon( "appointment-new" ), i18n( "New E&vent..." ), this );
  action->setIconText( i18nc( "@action:intoolbar create a new event", "Event" ) );
  action->setHelpText( i18n( "Create a new Event" ) );
  mACollection->addAction( "new_event", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView,
           SLOT(newEvent()) );

  action = new KAction( KIcon( "task-new" ), i18n( "New &To-do..." ), this );
  action->setIconText( i18n( "To-do" ) );
  action->setHelpText( i18n( "Create a new To-do" ) );
  mACollection->addAction( "new_todo", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView,
           SLOT(newTodo()) );

  action = new KAction( i18n( "New Su&b-to-do..." ), this );
  mACollection->addAction( "new_subtodo", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView,
           SLOT(newSubTodo() ));
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL(todoSelected(bool)), action,
           SLOT(setEnabled(bool)) );

  action = new KAction( KIcon( "journal-new" ), i18n( "New &Journal..." ), this );
  action->setIconText( i18n( "Journal" ) );
  action->setHelpText( i18n( "Create a new Journal" ) );
  mACollection->addAction( "new_journal", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView,
           SLOT(newJournal()) );

  mShowIncidenceAction = new KAction( i18n( "&Show" ), this );
  mACollection->addAction( "show_incidence", mShowIncidenceAction );
  connect( mShowIncidenceAction, SIGNAL(triggered(bool)), mCalendarView,
           SLOT(showIncidence()) );

  mEditIncidenceAction = new KAction( i18n( "&Edit..." ), this );
  mACollection->addAction( "edit_incidence", mEditIncidenceAction );
  connect( mEditIncidenceAction, SIGNAL(triggered(bool)), mCalendarView,
           SLOT(editIncidence()) );

  mDeleteIncidenceAction = new KAction( i18n( "&Delete" ), this );
  mACollection->addAction( "delete_incidence", mDeleteIncidenceAction );
  connect( mDeleteIncidenceAction, SIGNAL(triggered(bool)), mCalendarView,
           SLOT(deleteIncidence()) );
  mDeleteIncidenceAction->setShortcut( QKeySequence( Qt::Key_Delete ) );

  action = new KAction( i18n( "&Make Sub-to-do Independent" ), this );
  mACollection->addAction( "unsub_todo", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView,
           SLOT(todo_unsub()) );
  action->setEnabled( false );
  connect( mCalendarView, SIGNAL(subtodoSelected(bool)), action,
           SLOT(setEnabled(bool)) );
// TODO: Add item to move the incidence to different resource
//   mAssignResourceAction = new KAction( i18n( "Assign to &Calendar..." ), 0,
//                                        mCalendarView, SLOT(assignResource()),
//                                        mACollection, "assign_resource" );
// TODO: Add item to quickly toggle the reminder of a given incidence
//   mToggleAlarmAction = new KToggleAction( i18n( "&Activate Reminder" ), 0,
//                                         mCalendarView, SLOT(toggleAlarm()),
//                                         mACollection, "activate_alarm" );

  /************************** SCHEDULE MENU ********************************/
  mPublishEvent = new KAction( KIcon( "mail-send" ), i18n( "&Publish Item Information..." ), this );
  mACollection->addAction( "schedule_publish", mPublishEvent );
  connect( mPublishEvent, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_publish()) );
  mPublishEvent->setEnabled( false );

  action = new KAction( KIcon( "mail-send" ), i18n( "Send &Invitation to Attendees" ), this );
  mACollection->addAction( "schedule_request", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_request()) );
  action->setEnabled( false );
  connect( mCalendarView, SIGNAL(organizerEventsSelected(bool)),
           action, SLOT(setEnabled(bool)) );

  action = new KAction( i18n( "Re&quest Update" ), this );
  mACollection->addAction( "schedule_refresh", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_refresh()) );
  action->setEnabled( false );
  connect( mCalendarView, SIGNAL(groupEventsSelected(bool)),
           action, SLOT(setEnabled(bool)) );

  action = new KAction( i18n( "Send &Cancellation to Attendees" ), this );
  mACollection->addAction( "schedule_cancel", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_cancel()) );
  action->setEnabled( false );
  connect( mCalendarView, SIGNAL(organizerEventsSelected(bool)),
           action, SLOT(setEnabled(bool)) );

  action = new KAction( KIcon( "mail-reply-sender" ), i18n( "Send Status &Update" ), this );
  mACollection->addAction( "schedule_reply", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_reply()) );
  action->setEnabled( false );
  connect( mCalendarView, SIGNAL(groupEventsSelected(bool)),
           action, SLOT(setEnabled(bool)) );

  action = new KAction( i18nc( "counter proposal", "Request Chan&ge" ), this );
  mACollection->addAction( "schedule_counter", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_counter()) );
  action->setEnabled( false );
  connect( mCalendarView, SIGNAL(groupEventsSelected(bool)),
           action, SLOT(setEnabled(bool)) );

  action = new KAction( i18n( "&Mail Free Busy Information..." ), this );
  mACollection->addAction( "mail_freebusy", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(mailFreeBusy()) );
  action->setEnabled( true );

  mForwardEvent = new KAction( KIcon( "mail-forward" ), i18n( "&Send as iCalendar..." ), this );
  mACollection->addAction( "schedule_forward", mForwardEvent );
  connect( mForwardEvent, SIGNAL(triggered(bool)), mCalendarView, SLOT(schedule_forward()) );
  mForwardEvent->setEnabled( false );

  action = new KAction( i18n( "&Upload Free Busy Information" ), this );
  mACollection->addAction( "upload_freebusy", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(uploadFreeBusy()) );
  action->setEnabled( true );

  if ( !mIsPart ) {
    action = new KAction( KIcon( "help-contents" ), i18n( "&Address Book" ), this );
    mACollection->addAction( "addressbook", action );
    connect( action, SIGNAL(triggered(bool)), mCalendarView, SLOT(openAddressbook()) );
  }

  /************************** SETTINGS MENU ********************************/

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SIDEBAR ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mDateNavigatorShowAction = new KToggleAction( i18n( "Show Date Navigator" ), this );
  mACollection->addAction( "show_datenavigator", mDateNavigatorShowAction );
  connect( mDateNavigatorShowAction, SIGNAL(triggered(bool)), SLOT(toggleDateNavigator()) );

  mTodoViewShowAction = new KToggleAction( i18n( "Show To-do View" ), this );
  mACollection->addAction( "show_todoview", mTodoViewShowAction );
  connect( mTodoViewShowAction, SIGNAL(triggered(bool)), SLOT(toggleTodoView()) );

  mEventViewerShowAction = new KToggleAction( i18n( "Show Item Viewer" ), this );
  mACollection->addAction( "show_eventviewer", mEventViewerShowAction );
  connect( mEventViewerShowAction, SIGNAL(triggered(bool)), SLOT(toggleEventViewer()) );
  KConfigGroup config( KOGlobals::self()->config(), "Settings" );
  mDateNavigatorShowAction->setChecked( config.readEntry( "DateNavigatorVisible", true ) );
  // if we are a kpart, then let's not show the todo in the left pane by
  // default since there's also a Todo part and we'll assume they'll be
  // using that as well, so let's not duplicate it (by default) here
  mTodoViewShowAction->setChecked( config.readEntry( "TodoViewVisible", mIsPart ? false : true ) );
  mEventViewerShowAction->setChecked( config.readEntry( "EventViewerVisible", true ) );
  toggleDateNavigator();
  toggleTodoView();
  toggleEventViewer();

  if ( !mMainWindow->hasDocument() ) {
    mResourceViewShowAction = new KToggleAction( i18n( "Show Calendar Manager" ), this );
    mACollection->addAction( "show_resourceview", mResourceViewShowAction );
    connect( mResourceViewShowAction, SIGNAL(triggered(bool)), SLOT(toggleResourceView()) );
    mResourceViewShowAction->setChecked( config.readEntry( "ResourceViewVisible", true ) );

    toggleResourceView();
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SIDEBAR ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mHideMenuBarAction = KStandardAction::showMenubar( this, SLOT(toggleMenubar()), mACollection );
  mHideMenuBarAction->setChecked( KOPrefs::instance()->showMenuBar() );
  toggleMenubar( true );

  action = new KAction( i18n( "Configure &Date && Time..." ), this );
  mACollection->addAction( "conf_datetime", action );
  connect( action, SIGNAL(triggered(bool)),
           SLOT(configureDateTime()) );
// TODO: Add an item to show the resource management dlg
//   new KAction( i18n( "Manage &Calendars..." ), 0,
//                     this, SLOT(manageResources()),
//                     mACollection, "conf_resources" );

  action = new KAction( KIcon( "view-filter" ), i18n( "Manage View &Filters..." ), this );
  mACollection->addAction( "edit_filters", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView,
           SLOT(editFilters()) );

  action = new KAction( i18n( "Manage C&ategories..." ), this );
  mACollection->addAction( "edit_categories", action );
  connect( action, SIGNAL(triggered(bool)), mCalendarView->dialogManager(),
           SLOT(showCategoryEditDialog()) );

  if ( mIsPart ) {
    action = new KAction( KIcon( "configure" ), i18n( "&Configure Calendar..." ), this );
    mACollection->addAction( "korganizer_configure", action );
    connect( action, SIGNAL(triggered(bool)), mCalendarView,
             SLOT(edit_options()) );
    mACollection->addAction( KStandardAction::KeyBindings, "korganizer_configure_shortcuts",
                             this, SLOT(keyBindings()) );
  } else {
    KStandardAction::preferences( mCalendarView, SLOT(edit_options()), mACollection );
    KStandardAction::keyBindings( this, SLOT(keyBindings()), mACollection );
  }

  /**************************** HELP MENU **********************************/
  QAction *a = mACollection->addAction( KStandardAction::TipofDay, this,
                                        SLOT(showTip()) );
  mACollection->addAction( "help_tipofday", a );

}

void ActionManager::slotChangeComboActionItem( int index )
{
  mFilterAction->setCurrentItem( index );
}

void ActionManager::setItems( const QStringList &lst )
{
  mFilterAction->setItems( lst );
}

void ActionManager::readSettings()
{
  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig *config = KOGlobals::self()->config();
  if ( mRecent ) {
    mRecent->loadEntries( config->group( "RecentFiles" ) );
  }
  mCalendarView->readSettings();
}

void ActionManager::writeSettings()
{
  kDebug();

  KConfigGroup config = KOGlobals::self()->config()->group( "Settings" );
  mCalendarView->writeSettings();

  if ( mDateNavigatorShowAction ) {
    config.writeEntry( "DateNavigatorVisible", mDateNavigatorShowAction->isChecked() );
  }

  if ( mTodoViewShowAction ) {
    config.writeEntry( "TodoViewVisible", mTodoViewShowAction->isChecked() );
  }

  if ( mResourceViewShowAction ) {
    config.writeEntry( "ResourceViewVisible", mResourceViewShowAction->isChecked() );
  }

  if ( mEventViewerShowAction ) {
    config.writeEntry( "EventViewerVisible", mEventViewerShowAction->isChecked() );
  }

  if ( mRecent ) {
    mRecent->saveEntries( KOGlobals::self()->config()->group( "RecentFiles" ) );
  }

  config.sync();

  if ( mCalendarResources ) {
    mCalendarResources->resourceManager()->writeConfig();
  }
}

void ActionManager::file_new()
{
  emit actionNew();
}

void ActionManager::file_open()
{
  KUrl url;
  QString defaultPath = KStandardDirs::locateLocal( "data","korganizer/" );
  url = KFileDialog::getOpenUrl( defaultPath, i18n( "*.vcs *.ics|Calendar Files" ),
                                 dialogParent() );

  file_open( url );
}

void ActionManager::file_open( const KUrl &url )
{
  if ( url.isEmpty() ) {
    return;
  }

  // is that URL already opened somewhere else? Activate that window
  KOrg::MainWindow *korg = ActionManager::findInstance( url );
  if ( ( 0 != korg )&&( korg != mMainWindow ) ) {
#ifdef Q_WS_X11
    KWindowSystem::activateWindow( korg->topLevelWidget()->winId() );
#endif
    return;
  }

  kDebug() << url.prettyUrl();

  // Open the calendar file in the same window only if we have an empty calendar
  // window, and not the resource calendar.
  if ( !mCalendarView->isModified() && mFile.isEmpty() && !mCalendarResources ) {
    openURL( url );
  } else {
    emit actionNew( url );
  }
}

void ActionManager::file_icalimport()
{
  // FIXME: eventually, we will need a dialog box to select import type, etc.
  // for now, hard-coded to ical file, $HOME/.calendar.
  int retVal = -1;
  QString progPath;
  KTemporaryFile tmpfn;
  tmpfn.open();

  QString homeDir = QDir::homePath() + QString::fromLatin1( "/.calendar" );

  if ( !QFile::exists( homeDir ) ) {
    KMessageBox::error( dialogParent(),
                        i18n( "You have no ical file in your home directory.\n"
                              "Import cannot proceed.\n" ) );
    return;
  }

  KProcess proc;
  proc << "ical2vcal" << tmpfn.fileName();
  retVal = proc.execute();

  if ( retVal < 0 ) {
    kDebug() << "Error executing ical2vcal.";
    return;
  }

  kDebug() << "ical2vcal return value:" << retVal;

  if ( retVal >= 0 && retVal <= 2 ) {
    // now we need to MERGE what is in the iCal to the current calendar.
    mCalendarView->openCalendar( tmpfn.fileName(), 1 );
    if ( !retVal ) {
      KMessageBox::information( dialogParent(),
                                i18n( "KOrganizer successfully imported and "
                                      "merged your .calendar file from ical "
                                      "into the currently opened calendar." ),
                                "dotCalendarImportSuccess" );
    } else {
      KMessageBox::information( dialogParent(),
                                i18n( "KOrganizer encountered some unknown fields while "
                                      "parsing your .calendar ical file, and had to "
                                      "discard them; please check to see that all "
                                      "your relevant data was correctly imported." ),
                                i18n( "ICal Import Successful with Warning" ) );
    }
  } else if ( retVal == -1 ) { // XXX this is bogus
    KMessageBox::error( dialogParent(),
                         i18n( "KOrganizer encountered an error parsing your "
                              ".calendar file from ical; import has failed." ) );
  } else if ( retVal == -2 ) { // XXX this is bogus
    KMessageBox::error( dialogParent(),
                         i18n( "KOrganizer does not think that your .calendar "
                              "file is a valid ical calendar; import has failed." ) );
  }
}

void ActionManager::file_merge()
{
  KUrl url = KFileDialog::getOpenUrl( KStandardDirs::locateLocal( "data","korganizer/" ),
                                      i18n( "*.vcs *.ics|Calendar Files" ),
                                      dialogParent() );
  if ( !url.isEmpty() ) { // isEmpty if user canceled the dialog
    importCalendar( url );
  }
}

void ActionManager::file_archive()
{
  mCalendarView->archiveCalendar();
}

void ActionManager::file_revert()
{
  openURL( mURL );
}

void ActionManager::file_saveas()
{
  KUrl url = getSaveURL();

  if ( url.isEmpty() ) {
    return;
  }

  saveAsURL( url );
}

void ActionManager::file_save()
{
  if ( mMainWindow->hasDocument() ) {
    if ( mURL.isEmpty() ) {
      file_saveas();
      return;
    } else {
      saveURL();
    }
  } else {
    mCalendarView->calendar()->save();
  }

  // export to HTML
  if ( KOPrefs::instance()->mHtmlWithSave ) {
    exportHTML();
  }
}

void ActionManager::file_close()
{
  if ( !saveModifiedURL() ) {
    return;
  }

  mCalendarView->closeCalendar();
  KIO::NetAccess::removeTempFile( mFile );
  mURL = "";
  mFile = "";

  setTitle();
}

bool ActionManager::openURL( const KUrl &url, bool merge )
{
  kDebug();

  if ( url.isEmpty() ) {
    kDebug() << "Error! Empty URL.";
    return false;
  }
  if ( !url.isValid() ) {
    kDebug() << "Error! URL is malformed.";
    return false;
  }

  if ( url.isLocalFile() ) {
    mURL = url;
    mFile = url.toLocalFile();
    if ( !KStandardDirs::exists( mFile ) ) {
      mMainWindow->showStatusMessage( i18n( "New calendar '%1'.", url.prettyUrl() ) );
      mCalendarView->setModified();
    } else {
      bool success = mCalendarView->openCalendar( mFile, merge );
      if ( success ) {
        showStatusMessageOpen( url, merge );
      }
    }
    setTitle();
  } else {
    QString tmpFile;
    if ( KIO::NetAccess::download( url, tmpFile, view() ) ) {
      kDebug() << "--- Downloaded to" << tmpFile;
      bool success = mCalendarView->openCalendar( tmpFile, merge );
      if ( merge ) {
        KIO::NetAccess::removeTempFile( tmpFile );
        if ( success ) {
          showStatusMessageOpen( url, merge );
        }
      } else {
        if ( success ) {
          KIO::NetAccess::removeTempFile( mFile );
          mURL = url;
          mFile = tmpFile;
          setTitle();
          kDebug() << "-- Add recent URL:" << url.prettyUrl();
          if ( mRecent ) {
            mRecent->addUrl( url );
          }
          showStatusMessageOpen( url, merge );
        }
      }
      return success;
    } else {
      QString msg;
      msg = i18n( "Cannot download calendar from '%1'.", url.prettyUrl() );
      KMessageBox::error( dialogParent(), msg );
      return false;
    }
  }
  return true;
}

bool ActionManager::addResource( const KUrl &mUrl )
{
  CalendarResources *cr = KOrg::StdCalendar::self();
  CalendarResourceManager *manager = cr->resourceManager();
  ResourceCalendar *resource = 0;
  QString name;

  kDebug() << "URL:" << mUrl;
  if ( mUrl.isLocalFile() ) {
    kDebug() << "Local Resource";
    resource = manager->createResource( "file" );
    if ( resource ) {
      resource->setValue( "File", mUrl.toLocalFile() );
    }
    name = mUrl.toLocalFile();
  } else {
    kDebug() << "Remote Resource";
    resource = manager->createResource( "remote" );
    if ( resource ) {
      resource->setValue( "DownloadURL", mUrl.url() );
      resource->setReadOnly( true );
    }
    name = mUrl.prettyUrl();
  }

  if ( resource ) {
    resource->setTimeSpec( KOPrefs::instance()->timeSpec() );
    resource->setResourceName( name );
    manager->add( resource );
    mMainWindow->showStatusMessage( i18n( "Added calendar for URL '%1'.", name ) );
    // we have to call resourceAdded manually, because for in-process changes
    // the dcop signals are not connected, so the resource's signals would not
    // be connected otherwise
    if ( mCalendarResources ) {
      mCalendarResources->resourceAdded( resource );
    }
  } else {
    QString msg = i18n( "Unable to create calendar '%1'.", name );
    KMessageBox::error( dialogParent(), msg );
  }
  return true;
}

void ActionManager::showStatusMessageOpen( const KUrl &url, bool merge )
{
  if ( merge ) {
    mMainWindow->showStatusMessage( i18n( "Merged calendar '%1'.",
                                      url.prettyUrl() ) );
  } else {
    mMainWindow->showStatusMessage( i18n( "Opened calendar '%1'.",
                                      url.prettyUrl() ) );
  }
}

void ActionManager::closeUrl()
{
  kDebug();

  file_close();
}

bool ActionManager::saveURL()
{
  QString ext;

  if ( mURL.isLocalFile() ) {
    ext = mFile.right( 4 );
  } else {
    ext = mURL.fileName().right( 4 );
  }

  if ( ext == QLatin1String( ".vcs" ) ) {
    int result = KMessageBox::warningContinueCancel(
      dialogParent(),
      i18n( "Your calendar will be saved in iCalendar format. Use "
            "'Export vCalendar' to save in vCalendar format." ),
      i18n( "Format Conversion" ), KGuiItem( i18n( "Proceed" ) ),
      KStandardGuiItem::cancel(),
      QString( "dontaskFormatConversion" ), KMessageBox::Notify );
    if ( result != KMessageBox::Continue ) {
      return false;
    }

    QString filename = mURL.fileName();
    filename.replace( filename.length() - 4, 4, ".ics" );
    mURL.setFileName( filename );
    if ( mURL.isLocalFile() ) {
      mFile = mURL.toLocalFile();
    }
    setTitle();
    if ( mRecent ) {
      mRecent->addUrl( mURL );
    }
  }

  if ( !mCalendarView->saveCalendar( mFile ) ) {
    kDebug() << "calendar view save failed.";
    return false;
  } else {
    mCalendarView->setModified( false );
  }

  if ( !mURL.isLocalFile() ) {
    if ( !KIO::NetAccess::upload( mFile, mURL, view() ) ) {
      QString msg = i18n( "Cannot upload calendar to '%1'",
                      mURL.prettyUrl() );
      KMessageBox::error( dialogParent(), msg );
      return false;
    }
  }

  // keep saves on a regular interval
  if ( KOPrefs::instance()->mAutoSave ) {
    mAutoSaveTimer->stop();
    mAutoSaveTimer->start( 1000 * 60 * KOPrefs::instance()->mAutoSaveInterval );
  }

  mMainWindow->showStatusMessage( i18n( "Saved calendar '%1'.", mURL.prettyUrl() ) );

  return true;
}

void ActionManager::exportHTML()
{
  HTMLExportSettings settings( "KOrganizer" );
  // Manually read in the config, because parametrized kconfigxt objects don't
  // seem to load the config theirselves
  settings.readConfig();

  QDate qd1;
  qd1 = QDate::currentDate();
  QDate qd2;
  qd2 = QDate::currentDate();
  if ( settings.monthView() ) {
    qd2.addMonths( 1 );
  } else {
    qd2.addDays( 7 );
  }
  settings.setDateStart( QDateTime( qd1 ) );
  settings.setDateEnd( QDateTime( qd2 ) );
  exportHTML( &settings );
}

void ActionManager::exportHTML( HTMLExportSettings *settings )
{
  if ( !settings || settings->outputFile().isEmpty() ) {
    return;
  }

  if ( QFileInfo( settings->outputFile() ).exists() ) {
    if( KMessageBox::questionYesNo(
          dialogParent(),
          i18n( "Do you want to overwrite file \"%1\"",
                settings->outputFile() ) ) == KMessageBox::No ) {
      return;
    }
  }

  QApplication::setOverrideCursor( QCursor ( Qt::WaitCursor ) );
  settings->setEMail( KOPrefs::instance()->email() );
  settings->setName( KOPrefs::instance()->fullName() );

  settings->setCreditName( "KOrganizer" );
  settings->setCreditURL( "http://korganizer.kde.org" );

  KCal::HtmlExport mExport( mCalendarView->calendar(), settings );

  QDate cdate = settings->dateStart().date();
  QDate qd2 = settings->dateEnd().date();
  while ( cdate <= qd2 ) {
    QStringList holidays = KOGlobals::self()->holiday( cdate );
    if ( !holidays.isEmpty() ) {
      QStringList::ConstIterator it = holidays.constBegin();
      for ( ; it != holidays.constEnd(); ++it ) {
        mExport.addHoliday( cdate, *it );
      }
    }
    cdate = cdate.addDays( 1 );
  }

  KUrl dest( settings->outputFile() );
  if ( dest.isLocalFile() ) {
    mExport.save( dest.toLocalFile() );
  } else {
    KTemporaryFile tf;
    tf.open();
    QString tfile = tf.fileName();
    mExport.save( tfile );
    if ( !KIO::NetAccess::upload( tfile, dest, view() ) ) {
      KNotification::event ( KNotification::Error,
                             i18n( "Could not upload file." ) );
    }
  }
  QApplication::restoreOverrideCursor();
}

bool ActionManager::saveAsURL( const KUrl &url )
{
  kDebug() << url.prettyUrl();

  if ( url.isEmpty() ) {
    kDebug() << "Empty URL.";
    return false;
  }
  if ( !url.isValid() ) {
    kDebug() << "Malformed URL.";
    return false;
  }

  QString fileOrig = mFile;
  KUrl URLOrig = mURL;

  KTemporaryFile *tempFile = 0;
  if ( url.isLocalFile() ) {
    mFile = url.toLocalFile();
  } else {
    tempFile = new KTemporaryFile;
    tempFile->setAutoRemove(false);
    tempFile->open();
    mFile = tempFile->fileName();
  }
  mURL = url;

  bool success = saveURL(); // Save local file and upload local file
  if ( success ) {
    delete mTempFile;
    mTempFile = tempFile;
    KIO::NetAccess::removeTempFile( fileOrig );
    setTitle();
    if ( mRecent ) {
      mRecent->addUrl( mURL );
    }
  } else {
    KMessageBox::sorry( dialogParent(),
                        i18n( "Unable to save calendar to the file %1.", mFile ),
                        i18n( "Error" ) );
    kDebug() << "failed";
    mURL = URLOrig;
    mFile = fileOrig;
    delete tempFile;
  }

  return success;
}

bool ActionManager::saveModifiedURL()
{
  kDebug();

  // If calendar isn't modified do nothing.
  if ( !mCalendarView->isModified() ) {
    return true;
  }

  mHtmlExportSync = true;
  if ( KOPrefs::instance()->mAutoSave && !mURL.isEmpty() ) {
    // Save automatically, when auto save is enabled.
    return saveURL();
  } else {
    int result = KMessageBox::warningYesNoCancel(
        dialogParent(),
        i18n( "The calendar has been modified.\nDo you want to save it?" ),
        QString(),
        KStandardGuiItem::save(), KStandardGuiItem::discard() );
    switch( result ) {
      case KMessageBox::Yes:
        if ( mURL.isEmpty() ) {
          KUrl url = getSaveURL();
          return saveAsURL( url );
        } else {
          return saveURL();
        }
      case KMessageBox::No:
        return true;
      case KMessageBox::Cancel:
      default:
        {
          mHtmlExportSync = false;
          return false;
        }
    }
  }
}

KUrl ActionManager::getSaveURL()
{
  KUrl url =
    KFileDialog::getSaveUrl( KStandardDirs::locateLocal( "data","korganizer/" ),
                             i18n( "*.ics *.vcs|Calendar Files" ),
                             dialogParent() );

  if ( url == KUrl() ) {
    return url;
  }

  QString filename = url.fileName();

  QString e = filename.right( 4 );
  if ( e != QLatin1String( ".vcs" ) && e != QLatin1String( ".ics" ) ) {
    // Default save format is iCalendar
    filename += ".ics";
  }

  url.setFileName( filename );

  kDebug() << "url:" << url.url();

  return url;
}

void ActionManager::saveProperties( KConfigGroup &config )
{
  kDebug();

  config.writeEntry( "UseResourceCalendar", !mMainWindow->hasDocument() );
  if ( mMainWindow->hasDocument() ) {
    config.writePathEntry( "Calendar", mURL.url() );
  }
}

void ActionManager::readProperties( const KConfigGroup &config )
{
  kDebug();

  bool isResourceCalendar(
    config.readEntry( "UseResourceCalendar", true ) );
  QString calendarUrl = config.readPathEntry( "Calendar", QString() );

  if ( !isResourceCalendar && !calendarUrl.isEmpty() ) {
    mMainWindow->init( true );
    KUrl u( calendarUrl );
    openURL( u );
  } else {
    mMainWindow->init( false );
  }
}

void ActionManager::checkAutoSave()
{
  kDebug();

  // Don't save if auto save interval is zero
  if ( KOPrefs::instance()->mAutoSaveInterval == 0 ) {
    return;
  }

  // has this calendar been saved before? If yes automatically save it.
  if ( KOPrefs::instance()->mAutoSave ) {
    if ( mCalendarResources || ( mCalendar && !url().isEmpty() ) ) {
      saveCalendar();
    }
  }
}

// Configuration changed as a result of the options dialog.
void ActionManager::updateConfig()
{
  kDebug();

  if ( KOPrefs::instance()->mAutoSave && !mAutoSaveTimer->isActive() ) {
    checkAutoSave();
    if ( KOPrefs::instance()->mAutoSaveInterval > 0 ) {
      mAutoSaveTimer->start( 1000 * 60 * KOPrefs::instance()->mAutoSaveInterval );
    }
  }
  if ( !KOPrefs::instance()->mAutoSave ) {
    mAutoSaveTimer->stop();
  }
  mNextXDays->setText( i18np( "&Next Day", "&Next %1 Days",
                              KOPrefs::instance()->mNextXDays ) );

  KOCore::self()->reloadPlugins();

  /* Hide/Show the Reminder Daemon */
  if ( !KOPrefs::instance()->mShowReminderDaemon ) {
    KOGlobals::self()->reminderClient()->hideDaemon();
  } else {
    KOGlobals::self()->reminderClient()->showDaemon();
  }

// Commented out because it crashes KOrganizer.
//  mParts = KOCore::self()->reloadParts( mMainWindow, mParts );

  setDestinationPolicy();

  if ( mResourceView ) {
    mResourceView->updateView();
  }

  KOGroupware::instance()->freeBusyManager()->setBrokenUrl( false );
}

void ActionManager::setDestinationPolicy()
{
  if ( mCalendarResources ) {
    if ( KOPrefs::instance()->mDestination == KOPrefs::askDestination ) {
      mCalendarResources->setAskDestinationPolicy();
    } else {
      mCalendarResources->setStandardDestinationPolicy();
    }
  }
}

void ActionManager::configureDateTime()
{
  KProcess proc;
  proc << "kcmshell4" << "language";

  if ( !proc.startDetached() ) {
    KMessageBox::sorry( dialogParent(),
                        i18n( "Could not start control module for date and time format." ) );
  }
}

void ActionManager::showTip()
{
  KTipDialog::showTip( dialogParent(), QString(), true );
}

void ActionManager::showTipOnStart()
{
  KTipDialog::showTip( dialogParent() );
}

KOrg::MainWindow *ActionManager::findInstance( const KUrl &url )
{
  if ( mWindowList ) {
    if ( url.isEmpty() ) {
      return mWindowList->defaultInstance();
    } else {
      return mWindowList->findInstance( url );
    }
  } else {
    return 0;
  }
}

void ActionManager::dumpText( const QString &str )
{
  kDebug() << str;
}

void ActionManager::toggleDateNavigator()
{
  bool visible = mDateNavigatorShowAction->isChecked();
  if ( mCalendarView ) {
    mCalendarView->showDateNavigator( visible );
  }
}

void ActionManager::toggleTodoView()
{
  bool visible = mTodoViewShowAction->isChecked();
  if ( mCalendarView ) {
    mCalendarView->showTodoView( visible );
  }
}

void ActionManager::toggleEventViewer()
{
  bool visible = mEventViewerShowAction->isChecked();
  if ( mCalendarView ) {
    mCalendarView->showEventViewer( visible );
  }
}

void ActionManager::toggleResourceView()
{
  kDebug();

  bool visible = mResourceViewShowAction->isChecked();
  if ( mResourceView ) {
    if ( visible ) {
      mResourceView->show();
    } else {
      mResourceView->hide();
    }
  }
}

bool ActionManager::openURL( const QString &url )
{
  return openURL( KUrl( url ) );
}

bool ActionManager::mergeURL( const QString &url )
{
  return openURL( KUrl( url ), true );
}

bool ActionManager::saveAsURL( const QString &url )
{
  return saveAsURL( KUrl( url ) );
}

QString ActionManager::getCurrentURLasString() const
{
  return mURL.url();
}

bool ActionManager::editIncidence( const QString &uid )
{
  return mCalendarView->editIncidence( uid );
}

bool ActionManager::showIncidence( const QString &uid )
{
  return mCalendarView->showIncidence( uid );
}

bool ActionManager::showIncidenceContext( const QString &uid )
{
  return mCalendarView->showIncidenceContext( uid );
}

bool ActionManager::deleteIncidence( const QString &uid, bool force )
{
  return mCalendarView->deleteIncidence( uid, force );
}

bool ActionManager::addIncidence( const QString &ical )
{
  return mCalendarView->addIncidence( ical );
}

void ActionManager::downloadNewStuff()
{
  kDebug();

  // FIXME (KNS2): use mCalendarView as parent widget
  KNS::Entry::List entries = KNS::Engine::download();

  KNS::Entry::List::Iterator it;
  for ( it = entries.begin(); it != entries.end(); ++it ) {
    KNS::Entry *entry = (*it);

    if( entry->status() != KNS::Entry::Installed ) {
      continue;
    }

    CalendarLocal cal( KOPrefs::instance()->timeSpec() );

    // FIXME (KNS2): According to Reinhold, this should cope with remote URIs
    FileStorage storage( &cal, entry->payload().representation() );
    if ( !storage.load() ) {
      KMessageBox::error( mCalendarView, i18n( "Could not load calendar." ) );
      return;
    }

    Event::List events = cal.events();

    QStringList eventList;

    Event::List::ConstIterator it;
    for ( it = events.constBegin(); it != events.constEnd(); ++it ) {
      QString text = (*it)->summary();
      eventList.append( text );
    }

    int result = KMessageBox::warningContinueCancelList( mCalendarView,
      i18n( "The downloaded events will be merged into your current calendar." ),
      eventList );

    if ( result != KMessageBox::Continue ) {
      // FIXME (KNS2): hm, no way out here :-)
    }

    if ( mCalendarView->openCalendar( entry->payload().representation(), true ) ) {
      // FIXME (KNS2): here neither
    }
  }

  // FIXME (KNS2): monday change
  //KNS::Engine::cleanup();
}

void ActionManager::uploadNewStuff()
{
  KTemporaryFile tmpfile;
  if ( !tmpfile.open() ) {
    return;
  }
  QString tmpfilename = tmpfile.fileName();
  tmpfile.close();

  // FIXME (KNS2): use mCalendarView as parent widget
  if( mCalendarView->saveCalendar( tmpfilename ) ) {
    //Laurent 2008-10-25: comment until KNS::Engine::upload is implemented
    //                    otherwise it asserts by default...
    //KNS::Engine::upload( tmpfilename );
  } else {
    // FIXME (KNS2): display error here
  }
}

QString ActionManager::localFileName()
{
  return mFile;
}

class ActionManager::ActionStringsVisitor : public IncidenceBase::Visitor
{
  public:
    ActionStringsVisitor() : mShow( 0 ), mEdit( 0 ), mDelete( 0 ) {}

    bool act( IncidenceBase *incidence, QAction *show, QAction *edit, QAction *del )
    {
      mShow = show;
      mEdit = edit;
      mDelete = del;
      return incidence->accept( *this );
    }

  protected:
    bool visit( Event * )
    {
      if ( mShow ) {
        mShow->setText( i18n( "&Show Event" ) );
      }
      if ( mEdit ) {
        mEdit->setText( i18n( "&Edit Event..." ) );
      }
      if ( mDelete ) {
        mDelete->setText( i18n( "&Delete Event" ) );
      }
      return true;
    }
    bool visit( Todo * )
    {
      if ( mShow ) {
        mShow->setText( i18n( "&Show To-do" ) );
      }
      if ( mEdit ) {
        mEdit->setText( i18n( "&Edit To-do..." ) );
      }
      if ( mDelete ) {
        mDelete->setText( i18n( "&Delete To-do" ) );
      }
      return true;
    }
    bool visit( Journal * )
    {
      return assignDefaultStrings();
    }
    bool visit( FreeBusy * ) // to inhibit hidden virtual compile warning
    {
      return false;
    }

  protected:
    bool assignDefaultStrings()
    {
      if ( mShow ) {
        mShow->setText( i18n( "&Show" ) );
      }
      if ( mEdit ) {
        mEdit->setText( i18n( "&Edit..." ) );
      }
      if ( mDelete ) {
        mDelete->setText( i18n( "&Delete" ) );
      }
      return true;
    }
    QAction *mShow;
    QAction *mEdit;
    QAction *mDelete;
};

void ActionManager::processIncidenceSelection( Incidence *incidence )
{
//  kDebug(5850) << "ActionManager::processIncidenceSelection()";

  if ( !incidence ) {
    enableIncidenceActions( false );
    return;
  }

  enableIncidenceActions( true );

  if ( incidence->isReadOnly() ) {
    mCutAction->setEnabled( false );
    mDeleteAction->setEnabled( false );
  }

  ActionStringsVisitor v;
  if ( !v.act( incidence, mShowIncidenceAction, mEditIncidenceAction, mDeleteIncidenceAction ) ) {
    mShowIncidenceAction->setText( i18n( "&Show" ) );
    mEditIncidenceAction->setText( i18n( "&Edit..." ) );
    mDeleteIncidenceAction->setText( i18n( "&Delete" ) );
  }
}

void ActionManager::enableIncidenceActions( bool enabled )
{
  mShowIncidenceAction->setEnabled( enabled );
  mEditIncidenceAction->setEnabled( enabled );
  mDeleteIncidenceAction->setEnabled( enabled );
//   mAssignResourceAction->setEnabled( enabled );

  mCutAction->setEnabled( enabled );
  mCopyAction->setEnabled( enabled );
  mDeleteAction->setEnabled( enabled );
  mPublishEvent->setEnabled( enabled );
  mForwardEvent->setEnabled( enabled );
}

void ActionManager::keyBindings()
{
  KShortcutsDialog dlg( KShortcutsEditor::AllActions,
                        KShortcutsEditor::LetterShortcutsDisallowed, view() );
  if ( mMainWindow ) {
    dlg.addCollection( mMainWindow->getActionCollection() );
  }

  foreach ( KOrg::Part *part, mParts ) {
    if ( part ) {
      dlg.addCollection( part->actionCollection(), part->shortInfo() );
    }
  }
  dlg.configure();
}

void ActionManager::loadParts()
{
  mParts = KOCore::self()->loadParts( mMainWindow );
}

void ActionManager::setTitle()
{
  mMainWindow->setTitle();
}

ActionManager::ResourceRequestReply
ActionManager::resourceRequest( const QList<QPair<QDateTime, QDateTime> >&,
                                const QByteArray &resource,
                                const QString &vCalIn )
{
    kDebug() << "resource=" << resource << " vCalIn=" << vCalIn;
    ActionManager::ResourceRequestReply reply;
    reply.vCalOut = "VCalOut";
    return reply;
}

void ActionManager::openEventEditor( const QString &text )
{
  mCalendarView->newEvent( text );
}

void ActionManager::openEventEditor( const QString &summary,
                                     const QString &description,
                                     const QStringList &attachments )
{
  mCalendarView->newEvent( summary, description, attachments );
}

void ActionManager::openEventEditor( const QString &summary,
                                     const QString &description,
                                     const QStringList &attachments,
                                     const QStringList &attendees )
{
  mCalendarView->newEvent( summary, description, attachments, attendees );
}

void ActionManager::openEventEditor( const QString &summary,
                                     const QString &description,
                                     const QString &uri,
                                     const QString &file,
                                     const QStringList &attendees,
                                     const QString &attachmentMimetype )
{
  int action = KOPrefs::instance()->defaultEmailAttachMethod();
  if ( attachmentMimetype != "message/rfc822" ) {
    action = KOPrefs::Link;
  } else if ( KOPrefs::instance()->defaultEmailAttachMethod() == KOPrefs::Ask ) {
    KMenu *menu = new KMenu( 0 );
    QAction *attachLink = menu->addAction( i18n( "Attach as &link" ) );
    QAction *attachInline = menu->addAction( i18n( "Attach &inline" ) );
    QAction *attachBody = menu->addAction( i18n( "Attach inline &without attachments" ) );
    menu->addSeparator();
    menu->addAction( KIcon( "dialog-cancel" ), i18n( "C&ancel" ) );

    QAction *ret = menu->exec( QCursor::pos() );
    delete menu;

    if ( ret == attachLink ) {
      action = KOPrefs::Link;
    } else if ( ret == attachInline ) {
      action = KOPrefs::InlineFull;
    } else if ( ret == attachBody ) {
      action = KOPrefs::InlineBody;
    } else {
      return;
    }
  }

  QString attData;
  KTemporaryFile tf;
  tf.setAutoRemove( true );
  switch ( action ) {
    case KOPrefs::Link:
      attData = uri;
      break;
    case KOPrefs::InlineFull:
      attData = file;
      break;
    case KOPrefs::InlineBody:
    {
      QFile f( file );
      if ( !f.open( IO_ReadOnly ) ) {
        return;
      }
      KMime::Message *msg = new KMime::Message();
      msg->setContent( f.readAll() );
      msg->parse();
      if ( msg == msg->textContent() || msg->textContent() == 0 ) { // no attachments
        attData = file;
      } else {
        if ( KMessageBox::warningContinueCancel(
               0,
               i18n( "Removing attachments from an email might invalidate its signature." ),
               i18n( "Remove Attachments" ), KStandardGuiItem::cont(), KStandardGuiItem::cancel(),
               "BodyOnlyInlineAttachment" ) != KMessageBox::Continue ) {
          return;
        }
        KMime::Message *newMsg = new KMime::Message();
        newMsg->setHead( msg->head() );
        newMsg->setBody( msg->textContent()->body() );
        newMsg->parse();
        newMsg->contentTransferEncoding()->from7BitString(
              msg->textContent()->contentTransferEncoding()->as7BitString() );
        newMsg->contentType()->from7BitString( msg->textContent()->contentType()->as7BitString() );
        newMsg->assemble();
        tf.write( newMsg->encodedContent() );
        attData = tf.fileName();
      }
      tf.close();
      delete msg;
      break;
    }
    default:
      return;
  }

  mCalendarView->newEvent( summary, description, QStringList(attData),
                           attendees, QStringList(attachmentMimetype),
                           action != KOPrefs::Link );
}

void ActionManager::openTodoEditor( const QString &text )
{
  mCalendarView->newTodo( text );
}

void ActionManager::openTodoEditor( const QString &summary,
                                    const QString &description,
                                    const QStringList &attachments )
{
  mCalendarView->newTodo( summary, description, attachments );
}

void ActionManager::openTodoEditor( const QString &summary,
                                    const QString &description,
                                    const QStringList &attachments,
                                    const QStringList &attendees )
{
  mCalendarView->newTodo( summary, description, attachments, attendees );
}

void ActionManager::openTodoEditor( const QString &summary,
                                    const QString &description,
                                    const QString &uri,
                                    const QString &file,
                                    const QStringList &attendees,
                                    const QString &attachmentMimetype,
                                    bool isTask )
{
  int action = KOPrefs::instance()->defaultTodoAttachMethod();
  if ( attachmentMimetype != "message/rfc822" ) {
    action = KOPrefs::TodoAttachLink;
  } else if ( KOPrefs::instance()->defaultTodoAttachMethod() == KOPrefs::TodoAttachAsk ) {
    KMenu *menu = new KMenu( 0 );
    QAction *attachLink = menu->addAction( i18n( "Attach as &link" ) );
    QAction *attachInline = menu->addAction( i18n( "Attach &inline" ) );
    menu->addSeparator();
    menu->addAction( KIcon( "dialog-cancel" ), i18n( "C&ancel" ) );

    QAction *ret = menu->exec( QCursor::pos() );
    delete menu;

    if ( ret == attachLink ) {
      action = KOPrefs::TodoAttachLink;
    } else if ( ret == attachInline ) {
      action = KOPrefs::TodoAttachInlineFull;
    } else {
      return;
    }
  }

  QString attData;
  switch ( action ) {
    case KOPrefs::TodoAttachLink:
      attData = uri;
      break;
    case KOPrefs::TodoAttachInlineFull:
      attData = file;
      break;
    default:
      return;
  }

  mCalendarView->newTodo( summary, description, QStringList(attData),
                          attendees, QStringList(attachmentMimetype),
                          action != KOPrefs::Link, isTask );
}

void ActionManager::openJournalEditor( const QDate &date )
{
  mCalendarView->newJournal( date );
}

void ActionManager::openJournalEditor( const QString &text, const QDate &date )
{
  mCalendarView->newJournal( text, date );
}

void ActionManager::openJournalEditor( const QString &text )
{
  mCalendarView->newJournal( text );
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

void ActionManager::goDate( const QDate &date )
{
  mCalendarView->goDate( date );
}

void ActionManager::goDate( const QString &date )
{
  goDate( KGlobal::locale()->readDate( date ) );
}

void ActionManager::showDate( const QDate &date )
{
  mCalendarView->showDate( date );
}

void ActionManager::updateUndoAction( const QString &text )
{
  mUndoAction->setText( i18n( "Undo" ) );
  if ( text.isEmpty() ) {
    mUndoAction->setEnabled( false );
  } else {
    mUndoAction->setEnabled( true );
    if ( !text.isEmpty() ) {
      mUndoAction->setText( i18n( "Undo: %1", text ) );
    }
  }
  mUndoAction->setIconText( i18n( "Undo" ) );
}

void ActionManager::updateRedoAction( const QString &text )
{
  if ( text.isEmpty() ) {
    mRedoAction->setEnabled( false );
    mRedoAction->setText( i18n( "Redo" ) );
  } else {
    mRedoAction->setEnabled( true );
    if ( text.isEmpty() ) {
      mRedoAction->setText( i18n( "Redo" ) );
    } else {
      mRedoAction->setText( i18n( "Redo (%1)", text ) );
    }
  }
}

bool ActionManager::queryClose()
{
  kDebug();

  bool close = true;

  if ( mCalendar && mCalendar->isModified() ) {
    int res = KMessageBox::questionYesNoCancel(
      dialogParent(),
      i18n( "The calendar contains unsaved changes. Do you want to save them before exiting?" ),
      QString(), KStandardGuiItem::save(), KStandardGuiItem::discard() );
    // Exit on yes and no, don't exit on cancel. If saving fails, ask for exiting.
    if ( res == KMessageBox::Yes ) {
      close = saveModifiedURL();
      if ( !close ) {
        int res1 = KMessageBox::questionYesNo(
          dialogParent(),
          i18n( "Unable to save the calendar. Do you still want to close this window?" ),
          QString(), KStandardGuiItem::close(), KStandardGuiItem::cancel() );
        close = ( res1 == KMessageBox::Yes );
      }
    } else {
      close = ( res == KMessageBox::No );
    }
  } else if ( mCalendarResources ) {
    if ( !mIsClosing ) {
      kDebug() << "!mIsClosing";
      if ( !saveResourceCalendar() ) {
        return false;
      }

      // FIXME: Put main window into a state indicating final saving.
      mIsClosing = true;
// FIXME: Close main window when save is finished
//      connect( mCalendarResources, SIGNAL(calendarSaved()),
//               mMainWindow, SLOT(close()) );
    }
    if ( mCalendarResources->isSaving() ) {
      kDebug() << "isSaving";
      close = false;
      KMessageBox::information( dialogParent(),
                                i18n( "Unable to exit. Saving still in progress." ) );
    } else {
      kDebug() << "close = true";
      close = true;
    }
  } else {
    close = true;
  }

  return close;
}

void ActionManager::saveCalendar()
{
  if ( mCalendar ) {
    if ( view()->isModified() ) {
      if ( !url().isEmpty() ) {
        saveURL();
      } else {
        QString location = KStandardDirs::locateLocal( "data", "korganizer/kontact.ics" );
        saveAsURL( location );
      }
    }
  } else if ( mCalendarResources ) {
    mCalendarResources->save();
    // FIXME: Make sure that asynchronous saves don't fail.
  }
}

bool ActionManager::saveResourceCalendar()
{
  if ( !mCalendarResources ) {
    return false;
  }

  CalendarResourceManager *m = mCalendarResources->resourceManager();
  CalendarResourceManager::ActiveIterator it;
  for ( it = m->activeBegin(); it != m->activeEnd(); ++it ) {
    if ( (*it)->readOnly() ) {
      continue;
    }
    if ( !(*it)->save() ) {
      int result = KMessageBox::warningContinueCancel(
        view(),
        i18n( "Saving of '%1' failed. Check that the calendar is "
              "properly configured.\nIgnore problem and save remaining "
              "resources or cancel save?", (*it)->resourceName() ),
        i18n( "Save Error" ),
        KGuiItem( i18n( "Continue Save" ) ),
        KGuiItem( i18n( "Cancel Save" ) ) );
      if ( result == KMessageBox::Cancel ) {
        return false;
      }
    }
  }
  return true;
}

void ActionManager::importCalendar( const KUrl &url )
{
  if ( !url.isValid() ) {
    KMessageBox::error( dialogParent(),
                        i18n( "URL '%1' is invalid.", url.prettyUrl() ) );
    return;
  }

  ImportDialog *dialog;
  dialog = new ImportDialog( url, mMainWindow->topLevelWidget() );
  connect( dialog, SIGNAL(dialogFinished(ImportDialog *)),
           SLOT(slotImportDialogFinished(ImportDialog *)) );
  connect( dialog, SIGNAL(openURL(const KUrl &, bool)),
           SLOT(openURL(const KUrl &, bool)) );
  connect( dialog, SIGNAL(newWindow(const KUrl &)),
           SIGNAL(actionNew(const KUrl &)) );
  connect( dialog, SIGNAL(addResource(const KUrl &)),
           SLOT(addResource(const KUrl &)) );

  dialog->show();
}

void ActionManager::slotImportDialogFinished( ImportDialog *dlg )
{
  dlg->deleteLater();
  mCalendarView->updateView();
}

void ActionManager::slotAutoArchivingSettingsModified()
{
  if ( KOPrefs::instance()->mAutoArchive ) {
    mAutoArchiveTimer->start( 4 * 60 * 60 * 1000 ); // check again in 4 hours
  } else {
    mAutoArchiveTimer->stop();
  }
}

void ActionManager::slotAutoArchive()
{
  if ( !mCalendarView->calendar() ) { // can this happen?
    return;
  }

  mAutoArchiveTimer->stop();
  EventArchiver archiver;
  connect( &archiver, SIGNAL(eventsDeleted()), mCalendarView, SLOT(updateView()) );
  archiver.runAuto( mCalendarView->calendar(), mCalendarView, false /*no gui*/);

  // restart timer with the correct delay ( especially useful for the first time )
  slotAutoArchivingSettingsModified();
}

QWidget *ActionManager::dialogParent()
{
  return mCalendarView->topLevelWidget();
}

#include "actionmanager.moc"
