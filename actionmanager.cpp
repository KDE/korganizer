/*
  This file is part of KOrganizer.

  Copyright (c) 2002 Mike Pilone <mpilone@slac.com>
  Copyright (c) 2002 Don Sanders <sanders@kde.org>
  Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "actionmanager.h"

#include "alarmclient.h"
#include "calendarview.h"
#include "kocore.h"
#include "kodialogmanager.h"
#include "koglobals.h"
#include "koprefs.h"
#include "koviewmanager.h"
#include "kowindowlist.h"
#include "kprocess.h"
#include "konewstuff.h"
#include "history.h"
#include "kogroupware.h"
#include "resourceview.h"
#include "importdialog.h"
#include "eventarchiver.h"
#include "stdcalendar.h"

#include <libkcal/calendarlocal.h>
#include <libkcal/calendarresources.h>
#include <libkcal/resourcelocal.h>
#include <resourceremote.h>
#include <libkcal/htmlexport.h>
#include <libkcal/htmlexportsettings.h>

#include <dcopclient.h>
#include <kaction.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <kkeydialog.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <ktip.h>
#include <ktempfile.h>
#include <kxmlguiclient.h>
#include <kwin.h>
#include <knotifyclient.h>
#include <kstdguiitem.h>
#include <kdeversion.h>
#include <kactionclasses.h>

#include <qapplication.h>
#include <qtimer.h>
#include <qlabel.h>


// FIXME: Several places in the file don't use KConfigXT yet!
KOWindowList *ActionManager::mWindowList = 0;

ActionManager::ActionManager( KXMLGUIClient *client, CalendarView *widget,
                              QObject *parent, KOrg::MainWindow *mainWindow,
                              bool isPart )
  : QObject( parent ), KCalendarIface(), mRecent( 0 ),
    mResourceButtonsAction( 0 ), mCalendar( 0 ),
    mCalendarResources( 0 ), mResourceView( 0 ), mIsClosing( false )
{
  mGUIClient = client;
  mACollection = mGUIClient->actionCollection();
  mCalendarView = widget;
  mIsPart = isPart;
  mTempFile = 0;
  mNewStuff = 0;
  mHtmlExportSync = false;
  mMainWindow = mainWindow;
}

ActionManager::~ActionManager()
{
  delete mNewStuff;

  // Remove Part plugins
  KOCore::self()->unloadParts( mMainWindow, mParts );

  delete mTempFile;

  // Take this window out of the window list.
  mWindowList->removeWindow( mMainWindow );

  delete mCalendarView;

  delete mCalendar;
  delete mCalendarResources;

  kdDebug(5850) << "~ActionManager() done" << endl;
}

// see the Note: below for why this method is necessary
void ActionManager::ActionManager::init()
{
  // Construct the groupware object
  KOGroupware::create( mCalendarView, mCalendarResources );

  // add this instance of the window to the static list.
  if ( !mWindowList ) {
    mWindowList = new KOWindowList;
    // Show tip of the day, when the first calendar is shown.
    if ( !mIsPart )
      QTimer::singleShot( 0, this, SLOT( showTipOnStart() ) );
  }
  // Note: We need this ActionManager to be fully constructed, and
  // parent() to have a valid reference to it before the following
  // addWindow is called.
  mWindowList->addWindow( mMainWindow );

  initActions();

  // set up autoSaving stuff
  mAutoSaveTimer = new QTimer( this );
  connect( mAutoSaveTimer,SIGNAL( timeout() ), SLOT( checkAutoSave() ) );
  if ( KOPrefs::instance()->mAutoSave &&
       KOPrefs::instance()->mAutoSaveInterval > 0 ) {
    mAutoSaveTimer->start( 1000 * 60 * KOPrefs::instance()->mAutoSaveInterval );
  }

  mAutoArchiveTimer = new QTimer( this );
  connect( mAutoArchiveTimer, SIGNAL( timeout() ), SLOT( slotAutoArchive() ) );
  // First auto-archive should be in 5 minutes (like in kmail).
  if ( KOPrefs::instance()->mAutoArchive )
    mAutoArchiveTimer->start( 5 * 60 * 1000, true ); // singleshot

  setTitle();

  connect( mCalendarView, SIGNAL( modifiedChanged( bool ) ), SLOT( setTitle() ) );
  connect( mCalendarView, SIGNAL( configChanged() ), SLOT( updateConfig() ) );

  connect( mCalendarView, SIGNAL( incidenceSelected( Incidence * ) ),
           this, SLOT( processIncidenceSelection( Incidence * ) ) );
  connect( mCalendarView, SIGNAL( exportHTML( HTMLExportSettings * ) ),
           this, SLOT( exportHTML( HTMLExportSettings * ) ) );

  processIncidenceSelection( 0 );

  // Update state of paste action
  mCalendarView->checkClipboard();

  mCalendarView->lookForOutgoingMessages();
  mCalendarView->lookForIncomingMessages();
}

void ActionManager::createCalendarLocal()
{
  mCalendar = new CalendarLocal( KOPrefs::instance()->mTimeZoneId );
  mCalendarView->setCalendar( mCalendar );
  mCalendarView->readSettings();

  initCalendar( mCalendar );
}

void ActionManager::createCalendarResources()
{
  mCalendarResources = KOrg::StdCalendar::self();

  CalendarResourceManager *manager = mCalendarResources->resourceManager();

  kdDebug(5850) << "CalendarResources used by KOrganizer:" << endl;
  CalendarResourceManager::Iterator it;
  for( it = manager->begin(); it != manager->end(); ++it ) {
    kdDebug(5850) << "  " << (*it)->resourceName() << endl;
//    (*it)->dump();
  }

  setDestinationPolicy();

  mCalendarView->setCalendar( mCalendarResources );
  mCalendarView->readSettings();

  ResourceViewFactory factory( mCalendarResources, mCalendarView );
  mCalendarView->addExtension( &factory );
  mResourceView = factory.resourceView();

  connect( mCalendarResources, SIGNAL( calendarChanged() ),
           mCalendarView, SLOT( slotCalendarChanged() ) );
  connect( mCalendarResources, SIGNAL( signalErrorMessage( const QString & ) ),
           mCalendarView, SLOT( showErrorMessage( const QString & ) ) );

  connect( mCalendarView, SIGNAL( configChanged() ),
           SLOT( updateConfig() ) );

  mResourceButtonsAction = new KToggleAction( i18n("Show Resource Buttons"), 0,
                                              this,
                                              SLOT( toggleResourceButtons() ),
                                              mACollection,
                                              "show_resourcebuttons" );

  KConfig *config = KOGlobals::self()->config();
  config->setGroup( "Settings" );
  mResourceButtonsAction->setChecked(
      config->readBoolEntry( "ResourceButtonsVisible", true ) );
  toggleResourceButtons();

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

  // File menu.
  if ( mIsPart ) {
    if ( mMainWindow->hasDocument() ) {
      new KAction( i18n("&New"), "filenew", CTRL+Key_N, this,
                   SLOT( file_new() ), mACollection, "korganizer_openNew" );
      KStdAction::open( this, SLOT( file_open() ), mACollection, "korganizer_open" );
      mRecent = new KRecentFilesAction( i18n("Open &Recent"), 0, 0, this,
                                        SLOT( file_openRecent( const KURL & ) ),
                                        mACollection, "korganizer_openRecent" );
      new KAction( i18n("Re&vert"), "revert", 0, this,
                   SLOT( file_revert() ), mACollection, "korganizer_revert" );
      KStdAction::saveAs( this, SLOT( file_saveas() ), mACollection,
                   "korganizer_saveAs" );
      KStdAction::close( this,
                   SLOT( file_close() ), mACollection, "korganizer_close" );
    }
    KStdAction::save( this,
                 SLOT( file_save() ), mACollection, "korganizer_save" );
  } else {
    KStdAction::openNew( this, SLOT( file_new() ), mACollection );
    KStdAction::open( this, SLOT( file_open() ), mACollection );
    mRecent = KStdAction::openRecent( this, SLOT( file_openRecent( const KURL& ) ),
                                     mACollection );
    KStdAction::revert( this,SLOT( file_revert() ),mACollection );
    KStdAction::save( this, SLOT( file_save() ), mACollection );
    KStdAction::saveAs( this, SLOT( file_saveas() ), mACollection );
    KStdAction::close( this, SLOT( file_close() ), mACollection );
  }

  new KAction( i18n("&Import From Ical"), 0, this, SLOT( file_import() ),
                    mACollection, "import_ical" );
  new KAction( i18n("Import &Calendar..."), 0, this, SLOT( file_merge() ),
                    mACollection, "import_icalendar" );
  new KAction( i18n("Archive O&ld Entries..."), 0, this, SLOT( file_archive() ),
                    mACollection, "file_archive" );

  // Settings menu.

  new KAction( i18n("Configure &Date && Time..."), 0,
                    this,SLOT( configureDateTime() ),
                    mACollection, "conf_datetime" );

  KStdAction::tipOfDay( this, SLOT( showTip() ), mACollection,
                        "help_tipofday" );

  new KAction( i18n("Get &Hot New Stuff..."), 0, this,
               SLOT( downloadNewStuff() ), mACollection,
               "downloadnewstuff" );

  new KAction( i18n("Upload &Hot New Stuff..."), 0, this,
               SLOT( uploadNewStuff() ), mACollection,
               "uploadnewstuff" );

  new KAction( i18n("&iCalendar..."), 0,
                    mCalendarView, SLOT( exportICalendar() ),
                    mACollection, "export_icalendar" );
  new KAction( i18n("&vCalendar..."), 0,
                    mCalendarView, SLOT( exportVCalendar() ),
                    mACollection, "export_vcalendar" );
  new KAction( i18n("Export &Web Page..."), "webexport", 0,
                    mCalendarView, SLOT( exportWeb() ),
                    mACollection, "export_web" );

  if ( mIsPart ) {
    KStdAction::print( mCalendarView, SLOT( print() ), mACollection, "korganizer_print" );
  } else {
    KStdAction::print( mCalendarView, SLOT( print() ), mACollection );
  }

  new KAction( i18n("delete completed To-dos","Pur&ge Completed"), 0,
               mCalendarView, SLOT( purgeCompleted() ), mACollection,
               "purge_completed" );

  KOrg::History *h = mCalendarView->history();

  KAction *pasteAction;

  if ( mIsPart ) {
    // edit menu
    mCutAction = new KAction( i18n("Cu&t"), "editcut", CTRL+Key_X, mCalendarView,
                             SLOT( edit_cut() ), mACollection, "korganizer_cut" );
    mCopyAction = new KAction( i18n("&Copy"), "editcopy", CTRL+Key_C, mCalendarView,
                              SLOT( edit_copy() ), mACollection, "korganizer_copy" );
    pasteAction = new KAction( i18n("&Paste"), "editpaste", CTRL+Key_V, mCalendarView,
                              SLOT( edit_paste() ), mACollection, "korganizer_paste" );
    mUndoAction = new KAction( i18n("&Undo"), "undo", CTRL+Key_Z, h,
                               SLOT( undo() ), mACollection, "korganizer_undo" );
    mRedoAction = new KAction( i18n("Re&do"), "redo", CTRL+SHIFT+Key_Z, h,
                               SLOT( redo() ), mACollection, "korganizer_redo" );
  } else {
    mCutAction = KStdAction::cut( mCalendarView,SLOT( edit_cut() ),
                                 mACollection );

    mCopyAction = KStdAction::copy( mCalendarView,SLOT( edit_copy() ),
                                   mACollection );

    pasteAction = KStdAction::paste( mCalendarView,SLOT( edit_paste() ),
                               mACollection );

    mUndoAction = KStdAction::undo( h, SLOT( undo() ), mACollection );
    mRedoAction = KStdAction::redo( h, SLOT( redo() ), mACollection );
  }

  pasteAction->setEnabled( false );
  connect( mCalendarView, SIGNAL( pasteEnabled( bool ) ),
           pasteAction, SLOT( setEnabled( bool ) ) );

  connect( h, SIGNAL( undoAvailable( const QString & ) ),
           SLOT( updateUndoAction( const QString & ) ) );
  connect( h, SIGNAL( redoAvailable( const QString & ) ),
           SLOT( updateRedoAction( const QString & ) ) );
  mUndoAction->setEnabled( false );
  mRedoAction->setEnabled( false );

  mDeleteAction = new KAction( i18n("&Delete"),"editdelete",0,
                              mCalendarView,SLOT( appointment_delete() ),
                              mACollection, "edit_delete" );

  if ( mIsPart ) {
    new KAction( i18n("&Find..."),"find",CTRL+Key_F,
                mCalendarView->dialogManager(), SLOT( showSearchDialog() ),
                mACollection, "korganizer_find" );
  } else {
    KStdAction::find( mCalendarView->dialogManager(), SLOT( showSearchDialog() ),
                     mACollection );
  }

  // view menu

  // TODO: try to find / create better icons for the following 4 actions
  new KAction( i18n( "Zoom In Horizontally" ), "viewmag+", 0,
                    mCalendarView->viewManager(), SLOT( zoomInHorizontally() ),
                    mACollection, "zoom_in_horizontally" );
  new KAction( i18n( "Zoom Out Horizontally" ), "viewmag-", 0,
                    mCalendarView->viewManager(), SLOT( zoomOutHorizontally() ),
                    mACollection, "zoom_out_horizontally" );
  new KAction( i18n( "Zoom In Vertically" ), "viewmag+", 0,
                    mCalendarView->viewManager(), SLOT( zoomInVertically() ),
                    mACollection, "zoom_in_vertically" );
  new KAction( i18n( "Zoom Out Vertically" ), "viewmag-", 0,
                    mCalendarView->viewManager(), SLOT( zoomOutVertically() ),
                    mACollection, "zoom_out_vertically" );

//--
  new KAction( i18n("What's &Next"), "whatsnext", 0,
                    mCalendarView->viewManager(), SLOT( showWhatsNextView() ),
                    mACollection, "view_whatsnext" );
  new KAction( i18n("&List"), "list", 0,
                    mCalendarView->viewManager(), SLOT( showListView() ),
                    mACollection, "view_list" );
  new KAction( i18n("&Day"), "1day", 0,
                    mCalendarView->viewManager(), SLOT( showDayView() ),
                    mACollection, "view_day" );
  new KAction( i18n("W&ork Week"), "5days", 0,
                    mCalendarView->viewManager(), SLOT( showWorkWeekView() ),
                    mACollection, "view_workweek" );
  new KAction( i18n("&Week"), "7days", 0,
                    mCalendarView->viewManager(), SLOT( showWeekView() ),
                    mACollection, "view_week" );
  mNextXDays = new KAction( "", "xdays", 0, mCalendarView->viewManager(),
                    SLOT( showNextXView() ), mACollection, "view_nextx" );
  mNextXDays->setText( i18n( "&Next Day", "Ne&xt %n Days", KOPrefs::instance()->mNextXDays ) );
  new KAction( i18n("&Month"), "month", 0,
                    mCalendarView->viewManager(), SLOT( showMonthView() ),
                    mACollection, "view_month" );
  new KAction( i18n("&To-do List"), "todo", 0,
                    mCalendarView->viewManager(), SLOT( showTodoView() ),
                    mACollection, "view_todo" );
  new KAction( i18n("&Journal"), "journal", 0,
                    mCalendarView->viewManager(), SLOT( showJournalView() ),
                    mACollection, "view_journal" );
  new KAction( i18n("&Update"), 0,
                    mCalendarView, SLOT( updateView() ),
                    mACollection, "update" );

  // actions menu

  new KAction( i18n("New E&vent..."), "appointment", 0,
                    mCalendarView,SLOT( newEvent() ),
                    mACollection, "new_event" );
  new KAction( i18n("New &To-do..."), "newtodo", 0,
                    mCalendarView,SLOT( newTodo() ),
                    mACollection, "new_todo" );
  action = new KAction( i18n("New Su&b-to-do..."), 0,
                    mCalendarView,SLOT( newSubTodo() ),
                    mACollection, "new_subtodo" );
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( todoSelected( bool ) ),
          action,SLOT( setEnabled( bool ) ) );

  mShowIncidenceAction = new KAction( i18n("&Show"), 0,
                         mCalendarView,SLOT( showIncidence() ),
                         mACollection, "show_incidence" );
  mEditIncidenceAction = new KAction( i18n("&Edit..."), 0,
                         mCalendarView,SLOT( editIncidence() ),
                         mACollection, "edit_incidence" );
  mDeleteIncidenceAction = new KAction( i18n("&Delete"), Key_Delete,
                         mCalendarView,SLOT( deleteIncidence() ),
                         mACollection, "delete_incidence" );

  action = new KAction( i18n("&Make Sub-to-do Independent"), 0,
                    mCalendarView,SLOT( todo_unsub() ),
                    mACollection, "unsub_todo" );
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( subtodoSelected( bool ) ),
          action,SLOT( setEnabled( bool ) ) );

  // Schedule menu.

  new KAction( i18n("&Outgoing Messages"),0,
                    mCalendarView->dialogManager(),SLOT( showOutgoingDialog() ),
                    mACollection,"outgoing" );
  new KAction( i18n("&Incoming Messages"),0,
                    mCalendarView->dialogManager(),SLOT( showIncomingDialog() ),
                    mACollection,"incoming" );
  mPublishEvent = new KAction( i18n("&Publish..."),"mail_send",0,
                       mCalendarView,SLOT( schedule_publish() ),
                       mACollection,"publish" );
  mPublishEvent->setEnabled( false );
  action = new KAction( i18n("Re&quest"),"mail_generic",0,
                       mCalendarView,SLOT( schedule_request() ),
                       mACollection,"request" );
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( organizerEventsSelected( bool ) ),
          action,SLOT( setEnabled( bool ) ) );
  action = new KAction( i18n("Re&fresh"),0,
                       mCalendarView,SLOT( schedule_refresh() ),
                       mACollection,"refresh" );
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( groupEventsSelected( bool ) ),
          action,SLOT( setEnabled( bool ) ) );
  action = new KAction( KStdGuiItem::cancel(),0,
                       mCalendarView,SLOT( schedule_cancel() ),
                       mACollection,"cancel" );
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( organizerEventsSelected( bool ) ),
          action,SLOT( setEnabled( bool ) ) );
/*  action = new KAction( i18n("Add"),0,
                       mCalendarView,SLOT( schedule_add() ),
                       mACollection,"add" );
  connect( mCalendarView,SIGNAL( eventsSelected( bool ) ),
          action,SLOT( setEnabled( bool ) ) );
*/  action = new KAction( i18n("&Reply"),"mail_reply",0,
                       mCalendarView,SLOT( schedule_reply() ),
                       mACollection,"reply" );
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( groupEventsSelected( bool ) ),
          action,SLOT( setEnabled( bool ) ) );
  action = new KAction( i18n("counter proposal","Coun&ter"),0,
                       mCalendarView,SLOT( schedule_counter() ),
                       mACollection,"counter" );
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( groupEventsSelected( bool ) ),
          action,SLOT( setEnabled( bool ) ) );

  action = new KAction( i18n("&Mail Free Busy Information"), 0,
                        mCalendarView, SLOT( mailFreeBusy() ),
                        mACollection, "mail_freebusy" );
  action->setEnabled( true );

  action = new KAction( i18n("&Upload Free Busy Information"), 0,
                        mCalendarView, SLOT( uploadFreeBusy() ),
                        mACollection, "upload_freebusy" );
  action->setEnabled( true );

  if ( !mIsPart ) {
      action = new KAction( i18n("&Addressbook"),"contents",0,
                           mCalendarView,SLOT( openAddressbook() ),
                           mACollection,"addressbook" );
  }

  // Navigation menu
  bool isRTL = QApplication::reverseLayout();

  new KAction( i18n("Go to &Today"), "today", 0,
                    mCalendarView,SLOT( goToday() ),
                    mACollection, "go_today" );

  action = new KAction( i18n("Go &Backward"), isRTL ? "1rightarrow" : "1leftarrow", 0,
                       mCalendarView,SLOT( goPrevious() ),
                       mACollection, "go_previous" );

// Changing the action text by setText makes the toolbar button disappear.
// This has to be fixed first, before the connects below can be reenabled.
/*
  connect( mCalendarView,SIGNAL( changeNavStringPrev( const QString & ) ),
          action,SLOT( setText( const QString & ) ) );
  connect( mCalendarView,SIGNAL( changeNavStringPrev( const QString & ) ),
          this,SLOT( dumpText( const QString & ) ) );
*/

  action = new KAction( i18n("Go &Forward"), isRTL ? "1leftarrow" : "1rightarrow", 0,
                       mCalendarView,SLOT( goNext() ),
                       mACollection, "go_next" );

/*
  connect( mCalendarView,SIGNAL( changeNavStringNext( const QString & ) ),
          action,SLOT( setText( const QString & ) ) );
*/


  if ( mIsPart ) {
    new KAction( i18n("&Configure KOrganizer..."),
                 "configure", 0, mCalendarView,
                 SLOT( edit_options() ), mACollection,
                 "korganizer_configure" );
    new KAction( i18n("Configure S&hortcuts..."),
                 "configure_shortcuts", 0, this,
                 SLOT( keyBindings() ), mACollection,
                 "korganizer_configure_shortcuts" );
  } else {
    KStdAction::preferences( mCalendarView, SLOT( edit_options() ),
                            mACollection );
    KStdAction::keyBindings( this, SLOT( keyBindings() ), mACollection );
  }

  new KAction( i18n("Edit C&ategories..."), 0,
                    mCalendarView->dialogManager(),
                    SLOT( showCategoryEditDialog() ),
                    mACollection,"edit_categories" );

  new KAction( i18n("Edit &Filters..."), "configure", 0,
                    mCalendarView, SLOT( editFilters() ),
                    mACollection, "edit_filters" );

  QLabel *filterLabel = new QLabel( i18n("Filter: "), mCalendarView );
  new KWidgetAction( filterLabel, i18n("Filter: "), 0, 0, 0,
                     mACollection, "filter_label" );

  mFilterAction = new KSelectAction( i18n("F&ilter"), 0,
                  mACollection, "filter_select" );
  mFilterAction->setEditable( false );
  connect( mFilterAction, SIGNAL( activated(int) ),
           this, SIGNAL( filterActivated( int ) ) );
  connect( mCalendarView, SIGNAL( newFilterListSignal( const QStringList & ) ),
           mFilterAction, SLOT( setItems( const QStringList & ) ) );
  connect( mCalendarView, SIGNAL( selectFilterSignal( int ) ),
           mFilterAction, SLOT( setCurrentItem( int ) ) );
  connect( this, SIGNAL( filterActivated( int ) ),
           mCalendarView, SLOT( filterActivated( int ) ) );
#if 0
  new KAction( i18n("Show Intro Page"), 0,
                    mCalendarView,SLOT( showIntro() ),
                    mACollection,"show_intro" );
#endif
}

void ActionManager::readSettings()
{
  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig *config = KOGlobals::self()->config();
  if ( mRecent ) mRecent->loadEntries( config );
  mCalendarView->readSettings();
}

void ActionManager::writeSettings()
{
  kdDebug(5850) << "ActionManager::writeSettings" << endl;

  KConfig *config = KOGlobals::self()->config();
  mCalendarView->writeSettings();

  config->setGroup( "Settings" );
  if ( mResourceButtonsAction ) {
    config->writeEntry( "ResourceButtonsVisible",
                        mResourceButtonsAction->isChecked() );
  }
  if ( mRecent ) mRecent->saveEntries( config );

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
  KURL url;
  QString defaultPath = locateLocal( "data","korganizer/" );
  url = KFileDialog::getOpenURL( defaultPath,i18n("*.vcs *.ics|Calendar Files"),
                                dialogParent() );

  if ( url.isEmpty() ) return;

  // is that URL already opened somewhere else? Activate that window
  KOrg::MainWindow *korg=ActionManager::findInstance( url );
  if ( ( 0 != korg )&&( korg != mMainWindow ) ) {
    KWin::setActiveWindow( korg->topLevelWidget()->winId() );
    return;
  }

  kdDebug(5850) << "ActionManager::file_open(): " << url.prettyURL() << endl;

  // Open the calendar file in the same window only if we have an empty calendar window, and not the resource calendar
  if ( !mCalendarView->isModified() && mFile.isEmpty() && !mCalendarResources ) {
    openURL( url );
  } else {
    emit actionNew( url );
  }
}

void ActionManager::file_openRecent( const KURL& url )
{
  if ( !url.isEmpty() ) {
    KOrg::MainWindow *korg=ActionManager::findInstance( url );
    if ( ( 0 != korg )&&( korg != mMainWindow ) ) {
      // already open in a different windows, activate that one
      KWin::setActiveWindow( korg->topLevelWidget()->winId() );
      return;
    }
    openURL( url );
  }
}

void ActionManager::file_import()
{
  // FIXME: eventually, we will need a dialog box to select import type, etc.
  // for now, hard-coded to ical file, $HOME/.calendar.
  int retVal = -1;
  QString progPath;
  KTempFile tmpfn;

  QString homeDir = QDir::homeDirPath() + QString::fromLatin1( "/.calendar" );

  if ( !QFile::exists( homeDir ) ) {
    KMessageBox::error( dialogParent(),
                       i18n( "You have no ical file in your home directory.\n"
                            "Import cannot proceed.\n" ) );
    return;
  }

  KProcess proc;
  proc << "ical2vcal" << tmpfn.name();
  bool success = proc.start( KProcess::Block );

  if ( !success ) {
    kdDebug(5850) << "Error starting ical2vcal." << endl;
    return;
  } else {
    retVal = proc.exitStatus();
  }

  kdDebug(5850) << "ical2vcal return value: " << retVal << endl;

  if ( retVal >= 0 && retVal <= 2 ) {
    // now we need to MERGE what is in the iCal to the current calendar.
    mCalendarView->openCalendar( tmpfn.name(),1 );
    if ( !retVal )
      KMessageBox::information( dialogParent(),
                               i18n( "KOrganizer successfully imported and "
                                    "merged your .calendar file from ical "
                                    "into the currently opened calendar." ),
                               "dotCalendarImportSuccess" );
    else
      KMessageBox::information( dialogParent(),
                           i18n( "KOrganizer encountered some unknown fields while "
                                "parsing your .calendar ical file, and had to "
                                "discard them; please check to see that all "
                                "your relevant data was correctly imported." ),
                                 i18n("ICal Import Successful with Warning") );
  } else if ( retVal == -1 ) {
    KMessageBox::error( dialogParent(),
                         i18n( "KOrganizer encountered an error parsing your "
                              ".calendar file from ical; import has failed." ) );
  } else if ( retVal == -2 ) {
    KMessageBox::error( dialogParent(),
                         i18n( "KOrganizer does not think that your .calendar "
                              "file is a valid ical calendar; import has failed." ) );
  }
  tmpfn.unlink();
}

void ActionManager::file_merge()
{
  KURL url = KFileDialog::getOpenURL( locateLocal( "data","korganizer/" ),
                                     i18n("*.vcs *.ics|Calendar Files"),
                                     dialogParent() );
  if ( ! url.isEmpty() )  // isEmpty if user cancelled the dialog
    importCalendar( url );
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
  KURL url = getSaveURL();

  if ( url.isEmpty() ) return;

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
  if ( !saveModifiedURL() ) return;

  mCalendarView->closeCalendar();
  KIO::NetAccess::removeTempFile( mFile );
  mURL="";
  mFile="";

  setTitle();
}

bool ActionManager::openURL( const KURL &url,bool merge )
{
  kdDebug(5850) << "ActionManager::openURL()" << endl;

  if ( url.isEmpty() ) {
    kdDebug(5850) << "ActionManager::openURL(): Error! Empty URL." << endl;
    return false;
  }
  if ( !url.isValid() ) {
    kdDebug(5850) << "ActionManager::openURL(): Error! URL is malformed." << endl;
    return false;
  }

  if ( url.isLocalFile() ) {
    mURL = url;
    mFile = url.path();
    if ( !KStandardDirs::exists( mFile ) ) {
      mMainWindow->showStatusMessage( i18n("New calendar '%1'.")
                                      .arg( url.prettyURL() ) );
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
    if( KIO::NetAccess::download( url, tmpFile, view() ) ) {
      kdDebug(5850) << "--- Downloaded to " << tmpFile << endl;
      bool success = mCalendarView->openCalendar( tmpFile, merge );
      if ( merge ) {
        KIO::NetAccess::removeTempFile( tmpFile );
        if ( success )
          showStatusMessageOpen( url, merge );
      } else {
        if ( success ) {
          KIO::NetAccess::removeTempFile( mFile );
          mURL = url;
          mFile = tmpFile;
          KConfig *config = KOGlobals::self()->config();
          config->setGroup( "General" );
          setTitle();
          kdDebug(5850) << "-- Add recent URL: " << url.prettyURL() << endl;
          if ( mRecent ) mRecent->addURL( url );
          showStatusMessageOpen( url, merge );
        }
      }
      return success;
    } else {
      QString msg;
      msg = i18n("Cannot download calendar from '%1'.").arg( url.prettyURL() );
      KMessageBox::error( dialogParent(), msg );
      return false;
    }
  }
  return true;
}

bool ActionManager::addResource( const KURL &mUrl )
{
  CalendarResources *cr = KOrg::StdCalendar::self();

  CalendarResourceManager *manager = cr->resourceManager();

  ResourceCalendar *resource = 0;

  QString name;

  kdDebug(5850) << "URL: " << mUrl << endl;
  if ( mUrl.isLocalFile() ) {
    kdDebug(5850) << "Local Resource" << endl;
    resource = new ResourceLocal( mUrl.path() );
    resource->setTimeZoneId( KOPrefs::instance()->mTimeZoneId );
    name = mUrl.path();
  } else {
    kdDebug(5850) << "Remote Resource" << endl;
    resource = new ResourceRemote( mUrl );
    resource->setTimeZoneId( KOPrefs::instance()->mTimeZoneId );
    name = mUrl.prettyURL();
    resource->setReadOnly( true );
  }

  if ( resource ) {
    resource->setResourceName( name );
    manager->add( resource );
    // we have to call resourceAdded manually, because for in-process changes
    // the dcop signals are not connected, so the resource's signals would not
    // be connected otherwise
    if ( mCalendarResources )
      mCalendarResources->resourceAdded( resource );
  }
  return true;
}


void ActionManager::showStatusMessageOpen( const KURL &url, bool merge )
{
  if ( merge ) {
    mMainWindow->showStatusMessage( i18n("Merged calendar '%1'.")
                                    .arg( url.prettyURL() ) );
  } else {
    mMainWindow->showStatusMessage( i18n("Opened calendar '%1'.")
                                    .arg( url.prettyURL() ) );
  }
}

void ActionManager::closeURL()
{
  kdDebug(5850) << "ActionManager::closeURL()" << endl;

  file_close();
}

bool ActionManager::saveURL()
{
  QString ext;

  if ( mURL.isLocalFile() ) {
    ext = mFile.right( 4 );
  } else {
    ext = mURL.filename().right( 4 );
  }

  if ( ext == ".vcs" ) {
    int result = KMessageBox::warningContinueCancel(
        dialogParent(),
        i18n( "Your calendar will be saved in iCalendar format. Use "
              "'Export vCalendar' to save in vCalendar format." ),
        i18n("Format Conversion"), i18n("Proceed"), "dontaskFormatConversion",
        true );
    if ( result != KMessageBox::Continue ) return false;

    QString filename = mURL.fileName();
    filename.replace( filename.length() - 4, 4, ".ics" );
    mURL.setFileName( filename );
    if ( mURL.isLocalFile() ) {
      mFile = mURL.path();
    }
    setTitle();
    if ( mRecent ) mRecent->addURL( mURL );
  }

  if ( !mCalendarView->saveCalendar( mFile ) ) {
    kdDebug(5850) << "ActionManager::saveURL(): calendar view save failed."
                  << endl;
    return false;
  } else {
    mCalendarView->setModified( false );
  }

  if ( !mURL.isLocalFile() ) {
    if ( !KIO::NetAccess::upload( mFile, mURL, view() ) ) {
      QString msg = i18n("Cannot upload calendar to '%1'")
                    .arg( mURL.prettyURL() );
      KMessageBox::error( dialogParent() ,msg );
      return false;
    }
  }

  // keep saves on a regular interval
  if ( KOPrefs::instance()->mAutoSave ) {
    mAutoSaveTimer->stop();
    mAutoSaveTimer->start( 1000*60*KOPrefs::instance()->mAutoSaveInterval );
  }

  mMainWindow->showStatusMessage( i18n("Saved calendar '%1'.").arg( mURL.prettyURL() ) );

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
  if ( settings.monthView() )
    qd2.addMonths( 1 );
  else
    qd2.addDays( 7 );
  settings.setDateStart( qd1 );
  settings.setDateEnd( qd2 );
  exportHTML( &settings );
}

void ActionManager::exportHTML( HTMLExportSettings *settings )
{
  if ( !settings || settings->outputFile().isEmpty() )
    return;
  settings->setEMail( KOPrefs::instance()->email() );
  settings->setName( KOPrefs::instance()->fullName() );

  settings->setCreditName( "KOrganizer" );
  settings->setCreditURL( "http://korganizer.kde.org" );

  KCal::HtmlExport mExport( mCalendarView->calendar(), settings );

  QDate cdate = settings->dateStart().date();
  QDate qd2 = settings->dateEnd().date();
  while ( cdate <= qd2 ) {
    if ( !KOCore::self()->holiday( cdate ).isEmpty() )
      mExport.addHoliday( cdate, KOCore::self()->holiday( cdate ) );
    cdate = cdate.addDays( 1 );
  }

  KURL dest( settings->outputFile() );
  if ( dest.isLocalFile() ) {
    mExport.save( dest.path() );
  } else {
    KTempFile tf;
    QString tfile = tf.name();
    tf.close();
    mExport.save( tfile );
    if ( !KIO::NetAccess::upload( tfile, dest, view() ) ) {
      KNotifyClient::event ( view()->winId(),
                            i18n("Could not upload file.") );
    }
    tf.unlink();
  }
}

bool ActionManager::saveAsURL( const KURL &url )
{
  kdDebug(5850) << "ActionManager::saveAsURL() " << url.prettyURL() << endl;

  if ( url.isEmpty() ) {
    kdDebug(5850) << "ActionManager::saveAsURL(): Empty URL." << endl;
    return false;
  }
  if ( !url.isValid() ) {
    kdDebug(5850) << "ActionManager::saveAsURL(): Malformed URL." << endl;
    return false;
  }

  QString fileOrig = mFile;
  KURL URLOrig = mURL;

  KTempFile *tempFile = 0;
  if ( url.isLocalFile() ) {
    mFile = url.path();
  } else {
    tempFile = new KTempFile;
    mFile = tempFile->name();
  }
  mURL = url;

  bool success = saveURL(); // Save local file and upload local file
  if ( success ) {
    delete mTempFile;
    mTempFile = tempFile;
    KIO::NetAccess::removeTempFile( fileOrig );
    KConfig *config = KOGlobals::self()->config();
    config->setGroup( "General" );
    setTitle();
    if ( mRecent ) mRecent->addURL( mURL );
  } else {
    KMessageBox::sorry( dialogParent(), i18n("Unable to save calendar to the file %1.").arg( mFile ), i18n("Error") );
    kdDebug(5850) << "ActionManager::saveAsURL() failed" << endl;
    mURL = URLOrig;
    mFile = fileOrig;
    delete tempFile;
  }

  return success;
}


bool ActionManager::saveModifiedURL()
{
  kdDebug(5850) << "ActionManager::saveModifiedURL()" << endl;

  // If calendar isn't modified do nothing.
  if ( !mCalendarView->isModified() ) return true;

  mHtmlExportSync = true;
  if ( KOPrefs::instance()->mAutoSave && !mURL.isEmpty() ) {
    // Save automatically, when auto save is enabled.
    return saveURL();
  } else {
    int result = KMessageBox::warningYesNoCancel(
        dialogParent(),
        i18n("The calendar has been modified.\nDo you want to save it?"),
        QString::null,
        KStdGuiItem::save(), KStdGuiItem::discard() );
    switch( result ) {
      case KMessageBox::Yes:
        if ( mURL.isEmpty() ) {
          KURL url = getSaveURL();
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


KURL ActionManager::getSaveURL()
{
  KURL url = KFileDialog::getSaveURL( locateLocal( "data","korganizer/" ),
                                     i18n("*.vcs *.ics|Calendar Files"),
                                     dialogParent() );

  if ( url.isEmpty() ) return url;

  QString filename = url.fileName( false );

  QString e = filename.right( 4 );
  if ( e != ".vcs" && e != ".ics" ) {
    // Default save format is iCalendar
    filename += ".ics";
  }

  url.setFileName( filename );

  kdDebug(5850) << "ActionManager::getSaveURL(): url: " << url.url() << endl;

  return url;
}

void ActionManager::saveProperties( KConfig *config )
{
  kdDebug(5850) << "ActionManager::saveProperties" << endl;

  config->writeEntry( "UseResourceCalendar", !mMainWindow->hasDocument() );
  if ( mMainWindow->hasDocument() ) {
    config->writePathEntry( "Calendar",mURL.url() );
  }
}

void ActionManager::readProperties( KConfig *config )
{
  kdDebug(5850) << "ActionManager::readProperties" << endl;

  bool isResourceCalendar(
    config->readBoolEntry( "UseResourceCalendar", true ) );
  QString calendarUrl = config->readPathEntry( "Calendar" );

  if ( !isResourceCalendar && !calendarUrl.isEmpty() ) {
    mMainWindow->init( true );
    KURL u( calendarUrl );
    openURL( u );
  } else {
    mMainWindow->init( false );
  }
}

void ActionManager::checkAutoSave()
{
  kdDebug(5850) << "ActionManager::checkAutoSave()" << endl;

  // Don't save if auto save interval is zero
  if ( KOPrefs::instance()->mAutoSaveInterval == 0 ) return;

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
  kdDebug(5850) << "ActionManager::updateConfig()" << endl;

  if ( KOPrefs::instance()->mAutoSave && !mAutoSaveTimer->isActive() ) {
    checkAutoSave();
    if ( KOPrefs::instance()->mAutoSaveInterval > 0 ) {
      mAutoSaveTimer->start( 1000 * 60 *
                             KOPrefs::instance()->mAutoSaveInterval );
    }
  }
  if ( !KOPrefs::instance()->mAutoSave ) mAutoSaveTimer->stop();
  mNextXDays->setText( i18n( "&Next Day", "&Next %n Days",
                             KOPrefs::instance()->mNextXDays ) );

  KOCore::self()->reloadPlugins();
  mParts = KOCore::self()->reloadParts( mMainWindow, mParts );

  setDestinationPolicy();

  mResourceView->updateView();
}

void ActionManager::setDestinationPolicy()
{
  if ( mCalendarResources ) {
    if ( KOPrefs::instance()->mDestination == KOPrefs::askDestination )
      mCalendarResources->setAskDestinationPolicy();
    else
      mCalendarResources->setStandardDestinationPolicy();
  }
}

void ActionManager::configureDateTime()
{
  KProcess *proc = new KProcess;
  *proc << "kcmshell" << "language";

  connect( proc,SIGNAL( processExited( KProcess * ) ),
          SLOT( configureDateTimeFinished( KProcess * ) ) );

  if ( !proc->start() ) {
      KMessageBox::sorry( dialogParent(),
        i18n("Could not start control module for date and time format.") );
      delete proc;
  }
}

void ActionManager::showTip()
{
  KTipDialog::showTip( dialogParent(),QString::null,true );
}

void ActionManager::showTipOnStart()
{
  KTipDialog::showTip( dialogParent() );
}

KOrg::MainWindow *ActionManager::findInstance( const KURL &url )
{
  if ( mWindowList ) {
    if ( url.isEmpty() ) return mWindowList->defaultInstance();
    else return mWindowList->findInstance( url );
  } else {
    return 0;
  }
}

void ActionManager::dumpText( const QString &str )
{
  kdDebug(5850) << "ActionManager::dumpText(): " << str << endl;
}

void ActionManager::toggleResourceButtons()
{
  bool visible = mResourceButtonsAction->isChecked();

  kdDebug(5850) << "RESOURCE VIEW " << long( mResourceView ) << endl;

  if ( mResourceView ) mResourceView->showButtons( visible );
}

bool ActionManager::openURL( QString url )
{
  return openURL( KURL( url ) );
}

bool ActionManager::mergeURL( QString url )
{
  return openURL( KURL( url ),true );
}

bool ActionManager::saveAsURL( QString url )
{
  return saveAsURL( KURL( url ) );
}

QString ActionManager::getCurrentURLasString() const
{
  return mURL.url();
}

bool ActionManager::editIncidence( const QString& uid )
{
  return mCalendarView->editIncidence( uid );
}

bool ActionManager::deleteIncidence( const QString& uid )
{
  return mCalendarView->deleteIncidence( uid );
}

void ActionManager::configureDateTimeFinished( KProcess *proc )
{
  delete proc;
}

void ActionManager::downloadNewStuff()
{
  kdDebug(5850) << "ActionManager::downloadNewStuff()" << endl;

  if ( !mNewStuff ) mNewStuff = new KONewStuff( mCalendarView );
  mNewStuff->download();
}

void ActionManager::uploadNewStuff()
{
  if ( !mNewStuff ) mNewStuff = new KONewStuff( mCalendarView );
  mNewStuff->upload();
}

QString ActionManager::localFileName()
{
  return mFile;
}

class ActionManager::ActionStringsVisitor : public IncidenceBase::Visitor
{
  public:
    ActionStringsVisitor() : mShow( 0 ), mEdit( 0 ), mDelete( 0 ) {}

    bool act( IncidenceBase *incidence, KAction *show, KAction *edit, KAction *del )
    {
      mShow = show;
      mEdit = edit;
      mDelete = del;
      return incidence->accept( *this );
    }

  protected:
    bool visit( Event * ) {
      if ( mShow ) mShow->setText( i18n("&Show Event") );
      if ( mEdit ) mEdit->setText( i18n("&Edit Event...") );
      if ( mDelete ) mDelete->setText( i18n("&Delete Event") );
      return true;
    }
    bool visit( Todo * ) {
      if ( mShow ) mShow->setText( i18n("&Show To-do") );
      if ( mEdit ) mEdit->setText( i18n("&Edit To-do...") );
      if ( mDelete ) mDelete->setText( i18n("&Delete To-do") );
      return true;
    }
    bool visit( Journal * ) { return assignDefaultStrings(); }
  protected:
    bool assignDefaultStrings() {
      if ( mShow ) mShow->setText( i18n("&Show") );
      if ( mEdit ) mEdit->setText( i18n("&Edit...") );
      if ( mDelete ) mDelete->setText( i18n("&Delete") );
      return true;
    }
    KAction *mShow;
    KAction *mEdit;
    KAction *mDelete;
};

void ActionManager::processIncidenceSelection( Incidence *incidence )
{
//  kdDebug(5850) << "ActionManager::processIncidenceSelection()" << endl;

  if ( !incidence ) {
    enableIncidenceActions( false );
    return;
  }

  enableIncidenceActions( true );

  ActionStringsVisitor v;
  if ( !v.act( incidence, mShowIncidenceAction, mEditIncidenceAction, mDeleteIncidenceAction ) ) {
    mShowIncidenceAction->setText( i18n("&Show") );
    mEditIncidenceAction->setText( i18n("&Edit...") );
    mDeleteIncidenceAction->setText( i18n("&Delete") );
  }
}

void ActionManager::enableIncidenceActions( bool enabled )
{
  mShowIncidenceAction->setEnabled( enabled );
  mEditIncidenceAction->setEnabled( enabled );
  mDeleteIncidenceAction->setEnabled( enabled );

  mCutAction->setEnabled( enabled );
  mCopyAction->setEnabled( enabled );
  mDeleteAction->setEnabled( enabled );
  mPublishEvent->setEnabled( enabled );
}

void ActionManager::keyBindings()
{
  KKeyDialog dlg( false, view() );
  if ( mMainWindow )
    dlg.insert( mMainWindow->getActionCollection() );

  KOrg::Part *part;
  for ( part = mParts.first(); part; part = mParts.next() ) {
    dlg.insert( part->actionCollection(), part->shortInfo() );
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

KCalendarIface::ResourceRequestReply ActionManager::resourceRequest( const QValueList<QPair<QDateTime, QDateTime> >&,
 const QCString& resource,
 const QString& vCalIn )
{
    kdDebug(5850) << k_funcinfo << "resource=" << resource << " vCalIn=" << vCalIn << endl;
    KCalendarIface::ResourceRequestReply reply;
    reply.vCalOut = "VCalOut";
    return reply;
}

void ActionManager::openEventEditor( const QString& text )
{
  mCalendarView->newEvent( text );
}

void ActionManager::openEventEditor( const QString& summary,
                                     const QString& description,
                                     const QString& attachment )
{
  mCalendarView->newEvent( summary, description, attachment );
}

void ActionManager::openEventEditor( const QString& summary,
                                     const QString& description,
                                     const QString& attachment,
                                     const QStringList& attendees )
{
  mCalendarView->newEvent( summary, description, attachment, attendees );
}

void ActionManager::openTodoEditor( const QString& text )
{
  mCalendarView->newTodo( text );
}

void ActionManager::openTodoEditor( const QString& summary,
                                    const QString& description,
                                    const QString& attachment )
{
  mCalendarView->newTodo( summary, description, attachment );
}

void ActionManager::openTodoEditor( const QString& summary,
                                    const QString& description,
                                    const QString& attachment,
                                    const QStringList& attendees )
{
  mCalendarView->newTodo( summary, description, attachment, attendees );
}

void ActionManager::openJournalEditor( const QDate& date )
{
  mCalendarView->newJournal( date );
}

void ActionManager::openJournalEditor( const QString& text, const QDate& date )
{
  mCalendarView->newJournal( text, date );
}

void ActionManager::openJournalEditor( const QString& text )
{
  mCalendarView->newJournal( text );
}

//TODO:
// void ActionManager::openJournalEditor( const QString& summary,
//                                        const QString& description,
//                                        const QString& attachment )
// {
//   mCalendarView->newJournal( summary, description, attachment );
// }


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

void ActionManager::goDate( const QDate& date )
{
  mCalendarView->goDate( date );
}

void ActionManager::goDate( const QString& date )
{
  goDate( KGlobal::locale()->readDate( date ) );
}

void ActionManager::updateUndoAction( const QString &text )
{
  if ( text.isNull() ) {
    mUndoAction->setEnabled( false );
    mUndoAction->setText( i18n("Undo") );
  } else {
    mUndoAction->setEnabled( true );
    if ( text.isEmpty() ) mUndoAction->setText( i18n("Undo") );
    else mUndoAction->setText( i18n("Undo (%1)").arg( text ) );
  }
}

void ActionManager::updateRedoAction( const QString &text )
{
  if ( text.isNull() ) {
    mRedoAction->setEnabled( false );
    mRedoAction->setText( i18n( "Redo" ) );
  } else {
    mRedoAction->setEnabled( true );
    if ( text.isEmpty() ) mRedoAction->setText( i18n("Redo") );
    else mRedoAction->setText( i18n( "Redo (%1)" ).arg( text ) );
  }
}

bool ActionManager::queryClose()
{
  kdDebug(5850) << "ActionManager::queryClose()" << endl;

  bool close = true;

  if ( mCalendar ) {
    int res = KMessageBox::questionYesNoCancel( dialogParent(),
      i18n("The calendar contains unsaved changes. Do you want to save them before exiting?") );
    // Exit on yes and no, don't exit on cancel. If saving fails, ask for exiting.
    if ( res == KMessageBox::Yes ) {
      close = saveModifiedURL();
      if ( !close ) {
        int res1 = KMessageBox::questionYesNo( dialogParent(), i18n("Unable to save the calendar. Do you still want to close this window?") );
        close = ( res1 == KMessageBox::Yes );
      }
    } else {
      close = ( res == KMessageBox::No );
    }
  } else if ( mCalendarResources ) {
    if ( !mIsClosing ) {
      kdDebug(5850) << "!mIsClosing" << endl;
      if ( !saveResourceCalendar() ) return false;

      // FIXME: Put main window into a state indicating final saving.
      mIsClosing = true;
// FIXME: Close main window when save is finished
//      connect( mCalendarResources, SIGNAL( calendarSaved() ),
//               mMainWindow, SLOT( close() ) );
    }
    if ( mCalendarResources->isSaving() ) {
      kdDebug(5850) << "ActionManager::queryClose(): isSaving" << endl;
      close = false;
      KMessageBox::information( dialogParent(),
          i18n("Unable to exit. Saving still in progress.") );
    } else {
      kdDebug(5850) << "ActionManager::queryClose(): close = true" << endl;
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
        QString location = locateLocal( "data", "korganizer/kontact.ics" );
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
  if ( !mCalendarResources ) return false;
  CalendarResourceManager *m = mCalendarResources->resourceManager();

  CalendarResourceManager::ActiveIterator it;
  for ( it = m->activeBegin(); it != m->activeEnd(); ++it ) {
    if ( (*it)->readOnly() ) continue;
    if ( !(*it)->save() ) {
      int result = KMessageBox::warningContinueCancel( view(),
        i18n( "Saving of '%1' failed. Check that the resource is "
             "properly configured.\nIgnore problem and continue without "
             "saving or cancel save?" ).arg( (*it)->resourceName() ),
        i18n("Save Error"), KStdGuiItem::dontSave() );
      if ( result == KMessageBox::Cancel ) return false;
    }
  }
  return true;
}

void ActionManager::importCalendar( const KURL &url )
{
  if ( !url.isValid() ) {
    KMessageBox::error( dialogParent(),
                        i18n("URL '%1' is invalid.").arg( url.prettyURL() ) );
    return;
  }

  ImportDialog *dialog;
  dialog = new ImportDialog( url, mMainWindow->topLevelWidget() );
  connect( dialog, SIGNAL( dialogFinished( ImportDialog * ) ),
           SLOT( slotImportDialogFinished( ImportDialog * ) ) );
  connect( dialog, SIGNAL( openURL( const KURL &, bool ) ),
           SLOT( openURL( const KURL &, bool ) ) );
  connect( dialog, SIGNAL( newWindow( const KURL & ) ),
           SIGNAL( actionNew( const KURL & ) ) );
  connect( dialog, SIGNAL( addResource( const KURL & ) ),
           SLOT( addResource( const KURL & ) ) );

  dialog->show();
}

void ActionManager::slotImportDialogFinished( ImportDialog *dlg )
{
  dlg->deleteLater();
  mCalendarView->updateView();
}

void ActionManager::slotAutoArchivingSettingsModified()
{
  if ( KOPrefs::instance()->mAutoArchive )
    mAutoArchiveTimer->start( 4 * 60 * 60 * 1000, true ); // check again in 4 hours
  else
    mAutoArchiveTimer->stop();
}

void ActionManager::slotAutoArchive()
{
  if ( !mCalendarView->calendar() ) // can this happen?
    return;
  mAutoArchiveTimer->stop();
  EventArchiver archiver;
  connect( &archiver, SIGNAL( eventsDeleted() ), mCalendarView, SLOT( updateView() ) );
  archiver.runAuto( mCalendarView->calendar(), mCalendarView, false /*no gui*/ );
  // restart timer with the correct delay ( especially useful for the first time )
  slotAutoArchivingSettingsModified();
}

QWidget *ActionManager::dialogParent()
{
  return mCalendarView->topLevelWidget();
}

#include "actionmanager.moc"
