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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

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
#include "freebusymanager.h"

#include <libkcal/calendarlocal.h>
#include <libkcal/calendarresources.h>
#include <libkcal/htmlexport.h>
#include <libkcal/htmlexportsettings.h>

#include <libkmime/kmime_message.h>

#include <dcopclient.h>
#include <kaction.h>
#include <kfiledialog.h>
#include <kiconloader.h>
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
#include <qcursor.h>
#include <qtimer.h>
#include <qlabel.h>


// FIXME: Several places in the file don't use KConfigXT yet!
KOWindowList *ActionManager::mWindowList = 0;

ActionManager::ActionManager( KXMLGUIClient *client, CalendarView *widget,
                              QObject *parent, KOrg::MainWindow *mainWindow,
                              bool isPart )
  : QObject( parent ), KCalendarIface(), mRecent( 0 ),
    mResourceButtonsAction( 0 ), mResourceViewShowAction( 0 ), mCalendar( 0 ),
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

  kdDebug(5850) << "~ActionManager() done" << endl;
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
    (*it)->setResolveConflict( true );
//    (*it)->dump();
  }

  setDestinationPolicy();

  mCalendarView->setCalendar( mCalendarResources );
  mCalendarView->readSettings();

  ResourceViewFactory factory( mCalendarResources, mCalendarView );
  mCalendarView->addExtension( &factory );
  mResourceView = factory.resourceView();

  connect( mCalendarResources, SIGNAL( calendarChanged() ),
           mCalendarView, SLOT( resourcesChanged() ) );
  connect( mCalendarResources, SIGNAL( signalErrorMessage( const QString & ) ),
           mCalendarView, SLOT( showErrorMessage( const QString & ) ) );

  connect( mCalendarView, SIGNAL( configChanged() ),
           SLOT( updateConfig() ) );

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


  //*************************** FILE MENU **********************************

  //~~~~~~~~~~~~~~~~~~~~~~~ LOADING / SAVING ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if ( mIsPart ) {
    if ( mMainWindow->hasDocument() ) {
      KStdAction::openNew( this, SLOT(file_new()), mACollection, "korganizer_openNew" );
      KStdAction::open( this, SLOT( file_open() ), mACollection, "korganizer_open" );
      mRecent = KStdAction::openRecent( this, SLOT( file_open( const KURL& ) ),
                                     mACollection, "korganizer_openRecent" );
      KStdAction::revert( this,SLOT( file_revert() ), mACollection, "korganizer_revert" );
      KStdAction::saveAs( this, SLOT( file_saveas() ), mACollection,
                   "korganizer_saveAs" );
      KStdAction::save( this, SLOT( file_save() ), mACollection, "korganizer_save" );
    }
    KStdAction::print( mCalendarView, SLOT( print() ), mACollection, "korganizer_print" );
  } else {
    KStdAction::openNew( this, SLOT( file_new() ), mACollection );
    KStdAction::open( this, SLOT( file_open() ), mACollection );
    mRecent = KStdAction::openRecent( this, SLOT( file_open( const KURL& ) ),
                                     mACollection );
    if ( mMainWindow->hasDocument() ) {
      KStdAction::revert( this,SLOT( file_revert() ), mACollection );
      KStdAction::save( this, SLOT( file_save() ), mACollection );
      KStdAction::saveAs( this, SLOT( file_saveas() ), mACollection );
    }
    KStdAction::print( mCalendarView, SLOT( print() ), mACollection );
  }


  //~~~~~~~~~~~~~~~~~~~~~~~~ IMPORT / EXPORT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  new KAction( i18n("Import &Calendar..."), 0, this, SLOT( file_merge() ),
               mACollection, "import_icalendar" );
  new KAction( i18n("&Import From UNIX Ical tool"), 0, this, SLOT( file_icalimport() ),
               mACollection, "import_ical" );
  new KAction( i18n("Get &Hot New Stuff..."), 0, this,
               SLOT( downloadNewStuff() ), mACollection,
               "downloadnewstuff" );

  new KAction( i18n("Export &Web Page..."), "webexport", 0,
               mCalendarView, SLOT( exportWeb() ),
               mACollection, "export_web" );
  new KAction( i18n("&iCalendar..."), 0,
               mCalendarView, SLOT( exportICalendar() ),
               mACollection, "export_icalendar" );
  new KAction( i18n("&vCalendar..."), 0,
               mCalendarView, SLOT( exportVCalendar() ),
               mACollection, "export_vcalendar" );
  new KAction( i18n("Upload &Hot New Stuff..."), 0, this,
               SLOT( uploadNewStuff() ), mACollection,
               "uploadnewstuff" );



  new KAction( i18n("Archive O&ld Entries..."), 0, this, SLOT( file_archive() ),
                    mACollection, "file_archive" );
  new KAction( i18n("delete completed to-dos", "Pur&ge Completed To-dos"), 0,
               mCalendarView, SLOT( purgeCompleted() ), mACollection,
               "purge_completed" );




  //************************** EDIT MENU *********************************
  KAction *pasteAction;
  KOrg::History *h = mCalendarView->history();
  if ( mIsPart ) {
    // edit menu
    mCutAction = KStdAction::cut( mCalendarView, SLOT( edit_cut() ),
                                  mACollection, "korganizer_cut" );
    mCopyAction = KStdAction::copy( mCalendarView, SLOT( edit_copy() ),
                                    mACollection, "korganizer_copy" );
    pasteAction = KStdAction::paste( mCalendarView, SLOT( edit_paste() ),
                                     mACollection, "korganizer_paste" );
    mUndoAction = KStdAction::undo( h, SLOT( undo() ),
                                    mACollection, "korganizer_undo" );
    mRedoAction = KStdAction::redo( h, SLOT( redo() ),
                                    mACollection, "korganizer_redo" );
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
  mDeleteAction = new KAction( i18n("&Delete"), "editdelete", 0,
                               mCalendarView, SLOT( appointment_delete() ),
                               mACollection, "edit_delete" );
  if ( mIsPart ) {
    KStdAction::find( mCalendarView->dialogManager(), SLOT( showSearchDialog() ),
                     mACollection, "korganizer_find" );
  } else {
    KStdAction::find( mCalendarView->dialogManager(), SLOT( showSearchDialog() ),
                     mACollection );
  }
  pasteAction->setEnabled( false );
  mUndoAction->setEnabled( false );
  mRedoAction->setEnabled( false );
  connect( mCalendarView, SIGNAL( pasteEnabled( bool ) ),
           pasteAction, SLOT( setEnabled( bool ) ) );
  connect( h, SIGNAL( undoAvailable( const QString & ) ),
           SLOT( updateUndoAction( const QString & ) ) );
  connect( h, SIGNAL( redoAvailable( const QString & ) ),
           SLOT( updateRedoAction( const QString & ) ) );




  //************************** VIEW MENU *********************************

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ VIEWS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  new KAction( i18n("What's &Next"),
               KOGlobals::self()->smallIcon( "whatsnext" ), 0,
               mCalendarView->viewManager(), SLOT( showWhatsNextView() ),
               mACollection, "view_whatsnext" );
  new KAction( i18n("&Day"),
               KOGlobals::self()->smallIcon( "1day" ), 0,
               mCalendarView->viewManager(), SLOT( showDayView() ),
               mACollection, "view_day" );
  mNextXDays = new KAction( "",
                            KOGlobals::self()->smallIcon( "xdays" ), 0,
                            mCalendarView->viewManager(),
                            SLOT( showNextXView() ),
                            mACollection, "view_nextx" );
  mNextXDays->setText( i18n( "&Next Day", "Ne&xt %n Days",
                             KOPrefs::instance()->mNextXDays ) );
  new KAction( i18n("W&ork Week"),
               KOGlobals::self()->smallIcon( "5days" ), 0,
               mCalendarView->viewManager(), SLOT( showWorkWeekView() ),
               mACollection, "view_workweek" );
  new KAction( i18n("&Week"),
               KOGlobals::self()->smallIcon( "7days" ), 0,
               mCalendarView->viewManager(), SLOT( showWeekView() ),
               mACollection, "view_week" );
  new KAction( i18n("&Month"),
               KOGlobals::self()->smallIcon( "month" ), 0,
               mCalendarView->viewManager(), SLOT( showMonthView() ),
               mACollection, "view_month" );
  new KAction( i18n("&List"),
               KOGlobals::self()->smallIcon( "list" ), 0,
               mCalendarView->viewManager(), SLOT( showListView() ),
               mACollection, "view_list" );
  new KAction( i18n("&To-do List"),
               KOGlobals::self()->smallIcon( "todo" ), 0,
               mCalendarView->viewManager(), SLOT( showTodoView() ),
               mACollection, "view_todo" );
  new KAction( i18n("&Journal"),
               KOGlobals::self()->smallIcon( "journal" ), 0,
               mCalendarView->viewManager(), SLOT( showJournalView() ),
               mACollection, "view_journal" );
  new KAction( i18n("&Timeline View"),
               KOGlobals::self()->smallIcon( "timeline" ), 0,
               mCalendarView->viewManager(), SLOT( showTimelineView() ),
               mACollection, "view_timeline" );

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~ FILTERS ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  new KAction( i18n("&Refresh"), 0,
                    mCalendarView, SLOT( updateView() ),
                    mACollection, "update" );
// TODO:
//   new KAction( i18n("Hide &Completed To-dos"), 0,
//                     mCalendarView, SLOT( toggleHideCompleted() ),
//                     mACollection, "hide_completed_todos" );

  mFilterAction = new KSelectAction( i18n("F&ilter"), 0,
                  mACollection, "filter_select" );
  mFilterAction->setEditable( false );
  connect( mFilterAction, SIGNAL( activated(int) ),
           mCalendarView, SLOT( filterActivated( int ) ) );
  connect( mCalendarView, SIGNAL( newFilterListSignal( const QStringList & ) ),
           mFilterAction, SLOT( setItems( const QStringList & ) ) );
  connect( mCalendarView, SIGNAL( selectFilterSignal( int ) ),
           mFilterAction, SLOT( setCurrentItem( int ) ) );
  connect( mCalendarView, SIGNAL( filterChanged() ),
           this, SLOT( setTitle() ) );


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ZOOM ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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




  //************************** Actions MENU *********************************

  new KAction( i18n("Go to &Today"), "today", 0,
                    mCalendarView,SLOT( goToday() ),
                    mACollection, "go_today" );
  bool isRTL = QApplication::reverseLayout();
  action = new KAction( i18n("Go &Backward"), isRTL ? "forward" : "back", 0,
                        mCalendarView,SLOT( goPrevious() ),
                        mACollection, "go_previous" );

  // Changing the action text by setText makes the toolbar button disappear.
  // This has to be fixed first, before the connects below can be reenabled.
  /*
  connect( mCalendarView, SIGNAL( changeNavStringPrev( const QString & ) ),
           action, SLOT( setText( const QString & ) ) );
  connect( mCalendarView, SIGNAL( changeNavStringPrev( const QString & ) ),
           this, SLOT( dumpText( const QString & ) ) );*/

  action = new KAction( i18n("Go &Forward"), isRTL ? "back" : "forward", 0,
                        mCalendarView,SLOT( goNext() ),
                        mACollection, "go_next" );
  /*
  connect( mCalendarView,SIGNAL( changeNavStringNext( const QString & ) ),
           action,SLOT( setText( const QString & ) ) );
  */


  //************************** Actions MENU *********************************
  new KAction( i18n("New E&vent..."),
               KOGlobals::self()->smallIcon( "newappointment" ), 0,
               mCalendarView, SLOT( newEvent() ),
               mACollection, "new_event" );
  new KAction( i18n("New &To-do..."),
               KOGlobals::self()->smallIcon( "newtodo" ), 0,
               mCalendarView, SLOT( newTodo() ),
               mACollection, "new_todo" );
  action = new KAction( i18n("New Su&b-to-do..."), 0,
                        mCalendarView,SLOT( newSubTodo() ),
                        mACollection, "new_subtodo" );
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( todoSelected( bool ) ),
           action,SLOT( setEnabled( bool ) ) );
  new KAction( i18n("New &Journal..."),
               KOGlobals::self()->smallIcon( "newjournal" ), 0,
               mCalendarView, SLOT( newJournal() ),
               mACollection, "new_journal" );

  mShowIncidenceAction = new KAction( i18n("&Show"), 0,
                                      mCalendarView,SLOT( showIncidence() ),
                                      mACollection, "show_incidence" );
  mEditIncidenceAction = new KAction( i18n("&Edit..."), 0,
                                      mCalendarView,SLOT( editIncidence() ),
                                      mACollection, "edit_incidence" );
  mDeleteIncidenceAction = new KAction( i18n("&Delete"), Key_Delete,
                                        mCalendarView,SLOT( deleteIncidence()),
                                        mACollection, "delete_incidence" );

  action = new KAction( i18n("&Make Sub-to-do Independent"), 0,
                        mCalendarView,SLOT( todo_unsub() ),
                        mACollection, "unsub_todo" );
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( subtodoSelected( bool ) ),
           action,SLOT( setEnabled( bool ) ) );
// TODO: Add item to move the incidence to different resource
//   mAssignResourceAction = new KAction( i18n("Assign &Resource..."), 0,
//                                        mCalendarView, SLOT( assignResource()),
//                                        mACollection, "assign_resource" );
// TODO: Add item to quickly toggle the reminder of a given incidence
//   mToggleAlarmAction = new KToggleAction( i18n("&Activate Reminder"), 0,
//                                         mCalendarView, SLOT( toggleAlarm()),
//                                         mACollection, "activate_alarm" );




  //************************** SCHEDULE MENU ********************************
  mPublishEvent = new KAction( i18n("&Publish Item Information..."), "mail_send", 0,
                               mCalendarView, SLOT( schedule_publish() ),
                               mACollection, "schedule_publish" );
  mPublishEvent->setEnabled( false );

  action = new KAction( i18n("Send &Invitation to Attendees"),"mail_generic",0,
                        mCalendarView,SLOT( schedule_request() ),
                        mACollection,"schedule_request" );
  action->setEnabled( false );
  connect( mCalendarView, SIGNAL( organizerEventsSelected( bool ) ),
           action, SLOT( setEnabled( bool ) ) );

  action = new KAction( i18n("Re&quest Update"), 0,
                        mCalendarView, SLOT( schedule_refresh() ),
                        mACollection, "schedule_refresh" );
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( groupEventsSelected( bool ) ),
           action,SLOT( setEnabled( bool ) ) );

  action = new KAction( i18n("Send &Cancelation to Attendees"), 0,
                        mCalendarView, SLOT( schedule_cancel() ),
                        mACollection, "schedule_cancel" );
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( organizerEventsSelected( bool ) ),
           action,SLOT( setEnabled( bool ) ) );

  action = new KAction( i18n("Send Status &Update"),"mail_reply",0,
                        mCalendarView,SLOT( schedule_reply() ),
                        mACollection,"schedule_reply" );
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( groupEventsSelected( bool ) ),
           action,SLOT( setEnabled( bool ) ) );

  action = new KAction( i18n("counter proposal","Request Chan&ge"),0,
                        mCalendarView,SLOT( schedule_counter() ),
                        mACollection, "schedule_counter" );
  action->setEnabled( false );
  connect( mCalendarView,SIGNAL( groupEventsSelected( bool ) ),
           action,SLOT( setEnabled( bool ) ) );

  mForwardEvent = new KAction( i18n("&Send as iCalendar..."), "mail_forward", 0,
                               mCalendarView, SLOT(schedule_forward()),
                               mACollection, "schedule_forward" );
  mForwardEvent->setEnabled( false );

  action = new KAction( i18n("&Mail Free Busy Information..."), 0,
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




  //************************** SETTINGS MENU ********************************

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SIDEBAR ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  mDateNavigatorShowAction = new KToggleAction( i18n("Show Date Navigator"), 0,
                      this, SLOT( toggleDateNavigator() ),
                      mACollection, "show_datenavigator" );
  mTodoViewShowAction = new KToggleAction ( i18n("Show To-do View"), 0,
                      this, SLOT( toggleTodoView() ),
                      mACollection, "show_todoview" );
  mEventViewerShowAction = new KToggleAction ( i18n("Show Item Viewer"), 0,
                      this, SLOT( toggleEventViewer() ),
                      mACollection, "show_eventviewer" );
  KConfig *config = KOGlobals::self()->config();
  config->setGroup( "Settings" );
  mDateNavigatorShowAction->setChecked(
      config->readBoolEntry( "DateNavigatorVisible", true ) );
  // if we are a kpart, then let's not show the todo in the left pane by
  // default since there's also a Todo part and we'll assume they'll be
  // using that as well, so let's not duplicate it (by default) here
  mTodoViewShowAction->setChecked(
      config->readBoolEntry( "TodoViewVisible", mIsPart ? false : true ) );
  mEventViewerShowAction->setChecked(
      config->readBoolEntry( "EventViewerVisible", true ) );
  toggleDateNavigator();
  toggleTodoView();
  toggleEventViewer();

  if ( !mMainWindow->hasDocument() ) {
    mResourceViewShowAction = new KToggleAction ( i18n("Show Resource View"), 0,
                        this, SLOT( toggleResourceView() ),
                        mACollection, "show_resourceview" );
    mResourceButtonsAction = new KToggleAction( i18n("Show &Resource Buttons"), 0,
                        this, SLOT( toggleResourceButtons() ),
                        mACollection, "show_resourcebuttons" );
    mResourceViewShowAction->setChecked(
        config->readBoolEntry( "ResourceViewVisible", true ) );
    mResourceButtonsAction->setChecked(
        config->readBoolEntry( "ResourceButtonsVisible", true ) );

    toggleResourceView();
    toggleResourceButtons();
  }


  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SIDEBAR ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  new KAction( i18n("Configure &Date && Time..."), 0,
                    this, SLOT( configureDateTime() ),
                    mACollection, "conf_datetime" );
// TODO: Add an item to show the resource management dlg
//   new KAction( i18n("Manage &Resources..."), 0,
//                     this, SLOT( manageResources() ),
//                     mACollection, "conf_resources" );
  new KAction( i18n("Manage View &Filters..."), "configure", 0,
               mCalendarView, SLOT( editFilters() ),
               mACollection, "edit_filters" );
  new KAction( i18n("Manage C&ategories..."), 0,
               mCalendarView->dialogManager(), SLOT( showCategoryEditDialog() ),
               mACollection, "edit_categories" );
  if ( mIsPart ) {
    new KAction( i18n("&Configure Calendar..."), "configure", 0,
                 mCalendarView, SLOT( edit_options() ),
                 mACollection, "korganizer_configure" );
    KStdAction::keyBindings( this, SLOT( keyBindings() ),
                             mACollection, "korganizer_configure_shortcuts" );
  } else {
    KStdAction::preferences( mCalendarView, SLOT( edit_options() ),
                            mACollection );
    KStdAction::keyBindings( this, SLOT( keyBindings() ), mACollection );
  }




  //**************************** HELP MENU **********************************
  KStdAction::tipOfDay( this, SLOT( showTip() ), mACollection,
                        "help_tipofday" );
//   new KAction( i18n("Show Intro Page"), 0,
//                     mCalendarView,SLOT( showIntro() ),
//                     mACollection,"show_intro" );




  //************************* TOOLBAR ACTIONS *******************************
  QLabel *filterLabel = new QLabel( i18n("Filter: "), mCalendarView );
  filterLabel->hide();
  new KWidgetAction( filterLabel, i18n("Filter: "), 0, 0, 0,
                     mACollection, "filter_label" );

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
  if ( mDateNavigatorShowAction ) {
    config->writeEntry( "DateNavigatorVisible",
                        mDateNavigatorShowAction->isChecked() );
  }
  if ( mTodoViewShowAction ) {
    config->writeEntry( "TodoViewVisible",
                        mTodoViewShowAction->isChecked() );
  }
  if ( mResourceViewShowAction ) {
    config->writeEntry( "ResourceViewVisible",
                        mResourceViewShowAction->isChecked() );
  }
  if ( mEventViewerShowAction ) {
    config->writeEntry( "EventViewerVisible",
                        mEventViewerShowAction->isChecked() );
  }

  if ( mRecent ) mRecent->saveEntries( config );

  config->sync();

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

  file_open( url );
}

void ActionManager::file_open( const KURL &url )
{
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

void ActionManager::file_icalimport()
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
    resource = manager->createResource( "file" );
    if ( resource )
      resource->setValue( "File", mUrl.path() );
    name = mUrl.path();
  } else {
    kdDebug(5850) << "Remote Resource" << endl;
    resource = manager->createResource( "remote" );
    if ( resource )
      resource->setValue( "DownloadURL", mUrl.url() );
    name = mUrl.prettyURL();
    resource->setReadOnly( true );
  }

  if ( resource ) {
    resource->setTimeZoneId( KOPrefs::instance()->mTimeZoneId );
    resource->setResourceName( name );
    manager->add( resource );
    mMainWindow->showStatusMessage( i18n( "Added calendar resource for URL '%1'." )
               .arg( name ) );
    // we have to call resourceAdded manually, because for in-process changes
    // the dcop signals are not connected, so the resource's signals would not
    // be connected otherwise
    if ( mCalendarResources )
      mCalendarResources->resourceAdded( resource );
  } else {
    QString msg = i18n("Unable to create calendar resource '%1'.")
                      .arg( name );
    KMessageBox::error( dialogParent(), msg );
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
    QStringList holidays = KOGlobals::self()->holiday( cdate );
    if ( !holidays.isEmpty() ) {
      QStringList::ConstIterator it = holidays.begin();
      for ( ; it != holidays.end(); ++it ) {
        mExport.addHoliday( cdate, *it );
      }
    }
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

  if ( mResourceView )
    mResourceView->updateView();

  KOGroupware::instance()->freeBusyManager()->setBrokenUrl( false );
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

void ActionManager::toggleDateNavigator()
{
  bool visible = mDateNavigatorShowAction->isChecked();
  if ( mCalendarView ) mCalendarView->showDateNavigator( visible );
}

void ActionManager::toggleTodoView()
{
  bool visible = mTodoViewShowAction->isChecked();
  if ( mCalendarView ) mCalendarView->showTodoView( visible );
}

void ActionManager::toggleEventViewer()
{
  bool visible = mEventViewerShowAction->isChecked();
  if ( mCalendarView ) mCalendarView->showEventViewer( visible );
}

void ActionManager::toggleResourceView()
{
  bool visible = mResourceViewShowAction->isChecked();
  kdDebug(5850) << "toggleResourceView: " << endl;
  if ( mResourceView ) {
    if ( visible ) mResourceView->show();
    else mResourceView->hide();
  }
}

void ActionManager::toggleResourceButtons()
{
  bool visible = mResourceButtonsAction->isChecked();

  kdDebug(5850) << "RESOURCE VIEW " << long( mResourceView ) << endl;

  if ( mResourceView ) mResourceView->showButtons( visible );
}

bool ActionManager::openURL( const QString &url )
{
  return openURL( KURL( url ) );
}

bool ActionManager::mergeURL( const QString &url )
{
  return openURL( KURL( url ),true );
}

bool ActionManager::saveAsURL( const QString &url )
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

bool ActionManager::deleteIncidence( const QString& uid, bool force )
{
  return mCalendarView->deleteIncidence( uid, force );
}

bool ActionManager::addIncidence( const QString& ical )
{
  return mCalendarView->addIncidence( ical );
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

  if ( incidence->isReadOnly() ) {
    mCutAction->setEnabled( false );
    mDeleteAction->setEnabled( false );
  }

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
//   mAssignResourceAction->setEnabled( enabled );

  mCutAction->setEnabled( enabled );
  mCopyAction->setEnabled( enabled );
  mDeleteAction->setEnabled( enabled );
  mPublishEvent->setEnabled( enabled );
  mForwardEvent->setEnabled( enabled );
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

void ActionManager::openEventEditor( const QString & summary,
                                     const QString & description,
                                     const QString & uri,
                                     const QString & file,
                                     const QStringList & attendees,
                                     const QString & attachmentMimetype )
{
  int action = KOPrefs::instance()->defaultEmailAttachMethod();
  if ( attachmentMimetype != "message/rfc822" ) {
    action = KOPrefs::Link;
  } else if ( KOPrefs::instance()->defaultEmailAttachMethod() == KOPrefs::Ask ) {
    KPopupMenu *menu = new KPopupMenu( 0 );
    menu->insertItem( i18n("Attach as &link"), KOPrefs::Link );
    menu->insertItem( i18n("Attach &inline"), KOPrefs::InlineFull );
    menu->insertItem( i18n("Attach inline &without attachments"), KOPrefs::InlineBody );
    menu->insertSeparator();
    menu->insertItem( SmallIcon("cancel"), i18n("C&ancel"), KOPrefs::Ask );
    action = menu->exec( QCursor::pos(), 0 );
    delete menu;
  }

  QString attData;
  KTempFile tf;
  tf.setAutoDelete( true );
  switch ( action ) {
    case KOPrefs::Ask:
      return;
    case KOPrefs::Link:
      attData = uri;
      break;
    case KOPrefs::InlineFull:
      attData = file;
      break;
    case KOPrefs::InlineBody:
    {
      QFile f( file );
      if ( !f.open( IO_ReadOnly ) )
        return;
      KMime::Message *msg = new KMime::Message();
      msg->setContent( QCString( f.readAll() ) );
      QCString head = msg->head();
      msg->parse();
      if ( msg == msg->textContent() || msg->textContent() == 0 ) { // no attachments
        attData = file;
      } else {
        if ( KMessageBox::warningContinueCancel( 0,
              i18n("Removing attachments from an email might invalidate its signature."),
              i18n("Remove Attachments"), KStdGuiItem::cont(), "BodyOnlyInlineAttachment" )
              != KMessageBox::Continue )
          return;
        // due to kmime shortcomings in KDE3, we need to assemble the result manually
        int begin = 0;
        int end = head.find( '\n' );
        bool skipFolded = false;
        while ( end >= 0 && end > begin ) {
          if ( head.find( "Content-Type:", begin, false ) != begin &&
                head.find( "Content-Transfer-Encoding:", begin, false ) != begin &&
                !(skipFolded && (head[begin] == ' ' || head[end] == '\t')) ) {
            QCString line = head.mid( begin, end - begin );
            tf.file()->writeBlock( line.data(), line.length() );
            tf.file()->writeBlock( "\n", 1 );
            skipFolded = false;
          } else {
            skipFolded = true;
          }

          begin = end + 1;
          end = head.find( '\n', begin );
          if ( end < 0 && begin < (int)head.length() )
            end = head.length() - 1;
        }
        QCString cte = msg->textContent()->contentTransferEncoding()->as7BitString();
        if ( !cte.stripWhiteSpace().isEmpty() ) {
          tf.file()->writeBlock( cte.data(), cte.length() );
          tf.file()->writeBlock( "\n", 1 );
        }
        QCString ct = msg->textContent()->contentType()->as7BitString();
        if ( !ct.stripWhiteSpace().isEmpty() )
          tf.file()->writeBlock( ct.data(), ct.length() );
        tf.file()->writeBlock( "\n", 1 );
        tf.file()->writeBlock( msg->textContent()->body() );
        attData = tf.name();
      }
      tf.close();
      delete msg;
      break;
    }
    default:
      // menu could have been closed by cancel, if so, do nothing
      return;
  }

  mCalendarView->newEvent( summary, description, attData, attendees, attachmentMimetype, action != KOPrefs::Link );
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

void ActionManager::openTodoEditor(const QString & summary,
                                   const QString & description,
                                   const QString & uri,
                                   const QString & file,
                                   const QStringList & attendees,
                                   const QString & attachmentMimetype)
{
  int action = KOPrefs::instance()->defaultTodoAttachMethod();
  if ( attachmentMimetype != "message/rfc822" ) {
    action = KOPrefs::TodoAttachLink;
  } else if ( KOPrefs::instance()->defaultTodoAttachMethod() == KOPrefs::TodoAttachAsk ) {
    KPopupMenu *menu = new KPopupMenu( 0 );
    menu->insertItem( i18n("Attach as &link"), KOPrefs::TodoAttachLink );
    menu->insertItem( i18n("Attach &inline"), KOPrefs::TodoAttachInlineFull );
    menu->insertSeparator();
    menu->insertItem( SmallIcon("cancel"), i18n("C&ancel"), KOPrefs::TodoAttachAsk );
    action = menu->exec( QCursor::pos(), 0 );
    delete menu;
  }

  QString attData;
  switch ( action ) {
    case KOPrefs::TodoAttachAsk:
      return;
    case KOPrefs::TodoAttachLink:
      attData = uri;
      break;
    case KOPrefs::TodoAttachInlineFull:
      attData = file;
      break;
    default:
      // menu could have been closed by cancel, if so, do nothing
      return;
  }

  mCalendarView->newTodo( summary, description, attData, attendees, attachmentMimetype, action != KOPrefs::Link );
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

void ActionManager::showDate(const QDate & date)
{
  mCalendarView->showDate( date );
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

  if ( mCalendar && mCalendar->isModified() ) {
    int res = KMessageBox::questionYesNoCancel( dialogParent(),
      i18n("The calendar contains unsaved changes. Do you want to save them before exiting?"), QString::null, KStdGuiItem::save(), KStdGuiItem::discard() );
    // Exit on yes and no, don't exit on cancel. If saving fails, ask for exiting.
    if ( res == KMessageBox::Yes ) {
      close = saveModifiedURL();
      if ( !close ) {
        int res1 = KMessageBox::questionYesNo( dialogParent(), i18n("Unable to save the calendar. Do you still want to close this window?"), QString::null, KStdGuiItem::close(), KStdGuiItem::cancel() );
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

void ActionManager::loadProfile( const QString & path )
{
  KOPrefs::instance()->writeConfig();
  KConfig* const cfg = KOPrefs::instance()->config();

  const KConfig profile( path+"/korganizerrc", /*read-only=*/false, /*useglobals=*/false );
  const QStringList groups = profile.groupList();
  for ( QStringList::ConstIterator it = groups.begin(), end = groups.end(); it != end; ++it )
  {
    cfg->setGroup( *it );
    typedef QMap<QString, QString> StringMap;
    const StringMap entries = profile.entryMap( *it );
    for ( StringMap::ConstIterator it2 = entries.begin(), end = entries.end(); it2 != end; ++it2 )
    {
      cfg->writeEntry( it2.key(), it2.data() );
    }
  }

  cfg->sync();
  KOPrefs::instance()->readConfig();
}

namespace {
    void copyConfigEntry( KConfig* source, KConfig* dest, const QString& group, const QString& key, const QString& defaultValue=QString() )
    {
        source->setGroup( group );
        dest->setGroup( group );
        dest->writeEntry( key, source->readEntry( key, defaultValue ) );
    }
}

void ActionManager::saveToProfile( const QString & path ) const
{
  KOPrefs::instance()->writeConfig();
  KConfig* const cfg = KOPrefs::instance()->config();

  KConfig profile( path+"/korganizerrc", /*read-only=*/false, /*useglobals=*/false );
  ::copyConfigEntry( cfg, &profile, "Views", "Agenda View Calendar Display" );
}

QWidget *ActionManager::dialogParent()
{
  return mCalendarView->topLevelWidget();
}

#include "actionmanager.moc"
