/*
  This file is part of KOrganizer.

  Copyright (c) 2002 Mike Pilone <mpilone@slac.com>
  Copyright (c) 2002 Don Sanders <sanders@kde.org>

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

#include <qapplication.h>
#include <qtimer.h>

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

#include <libkcal/htmlexport.h>

#include "alarmclient.h"
#include "calendarview.h"
#include "kocore.h"
#include "kodialogmanager.h"
#include "koglobals.h"
#include "koprefs.h"
#include "koviewmanager.h"
#include "kowindowlist.h"
#include "korganizer.h"
#include "kprocess.h"
#include "konewstuff.h"
#include "history.h"
#include "kogroupware.h"


KOWindowList *ActionManager::windowList = 0;
bool ActionManager::startedKAddressBook = false;

ActionManager::ActionManager( KXMLGUIClient *client, CalendarView *widget,
                              QObject *parent, KOrg::MainWindow *mainWindow,
                              bool isPart )
    : QObject(parent), KCalendarIface()
{
  mGUIClient = client;
  mACollection = mGUIClient->actionCollection();
  mCalendarView = widget;
  mIsPart = isPart;
  mTempFile = 0;
  mActive = false;
  mNewStuff = 0;
  mHtmlExportSync = false;
  mMainWindow = mainWindow;
}

//see the Note: below for why this method is necessary
void ActionManager::ActionManager::init()
{
  // add this instance of the window to the static list.
  if (!windowList) {
    windowList = new KOWindowList;
    // Show tip of the day, when the first calendar is shown.
    QTimer::singleShot(0,this,SLOT(showTipOnStart()));
  }
  //Note: We need this ActionManager to be fully constructed, and
  //parent() to have a valid reference to it before the following
  //addWindow is called.
  windowList->addWindow(mMainWindow);

  initActions();

  // set up autoSaving stuff
  mAutoSaveTimer = new QTimer(this);
  connect(mAutoSaveTimer,SIGNAL(timeout()),SLOT(checkAutoSave()));
  if (KOPrefs::instance()->mAutoSave &&
      KOPrefs::instance()->mAutoSaveInterval > 0) {
    mAutoSaveTimer->start(1000*60*KOPrefs::instance()->mAutoSaveInterval);
  }

  setTitle();

  connect( mCalendarView, SIGNAL( modifiedChanged( bool ) ), SLOT( setTitle() ) );
  connect( mCalendarView, SIGNAL( configChanged() ), SLOT( updateConfig() ) );

  connect( mCalendarView, SIGNAL( incidenceSelected( Incidence * ) ),
           this, SLOT( processIncidenceSelection( Incidence * ) ) );

  processIncidenceSelection( 0 );

  // Update state of paste action
  mCalendarView->checkClipboard();

  mCalendarView->lookForOutgoingMessages();
  mCalendarView->lookForIncomingMessages();
}

ActionManager::~ActionManager()
{
  delete mNewStuff;

  // Remove Part plugins
  if ( mPluginMenu ) mPluginMenu->popupMenu()->clear();
  KOCore::self()->unloadParts( mMainWindow, mParts );
  //close down KAddressBook if we started it
  if ( ActionManager::startedKAddressBook == true ) {
    kdDebug(5850) << "Closing down kaddressbook" << endl;
    DCOPClient *client = KApplication::kApplication()->dcopClient();
    const QByteArray noParamData;
    client->send("kaddressbook", "KAddressBookIface", "exit()",  noParamData);
  }

  delete mTempFile;

  // Take this window out of the window list.
  windowList->removeWindow( mMainWindow );

  delete mCalendarView;

  kdDebug(5850) << "~ActionManager() done" << endl;
}

void ActionManager::initActions()
{
  KAction *action;

  // File menu.
  if ( mIsPart ) {
    new KAction(i18n("&New"), "filenew", CTRL+Key_N, this,
		SLOT(file_new()), mACollection, "korganizer_openNew" );
    new KAction(i18n("&Open"), "fileopen", CTRL+Key_O, this,
	       SLOT(file_open()), mACollection, "korganizer_open" );
    mRecent = new KRecentFilesAction(i18n("Open &Recent"), 0, 0, this,
		SLOT(file_openRecent(const KURL&)), mACollection, "korganizer_openRecent" );
    new KAction(i18n("Re&vert"), "revert", 0, this,
		SLOT(file_revert()), mACollection, "korganizer_revert" );
    new KAction(i18n("&Save"), "filesave", CTRL+Key_S, this,
		SLOT(file_save()), mACollection, "korganizer_save" );
    new KAction(i18n("Save &As..."), "filesaveas", 0, this,
		SLOT(file_saveas()), mACollection, "korganizer_saveAs" );
    new KAction(i18n("&Close"), "fileclose", CTRL+Key_W, this,
		SLOT(file_close()), mACollection, "korganizer_close" );
  } else {
    KStdAction::openNew(this, SLOT(file_new()), mACollection);
    KStdAction::open(this, SLOT(file_open()), mACollection);
    mRecent = KStdAction::openRecent(this, SLOT(file_openRecent(const KURL&)),
				     mACollection);
    KStdAction::revert(this,SLOT(file_revert()),mACollection);
    KStdAction::save(this, SLOT(file_save()), mACollection);
    KStdAction::saveAs(this, SLOT(file_saveas()), mACollection);
    KStdAction::close(this, SLOT(file_close()), mACollection);
  }

  (void)new KAction(i18n("&Import From Ical"), 0, this, SLOT(file_import()),
                    mACollection, "import_ical");
  (void)new KAction(i18n("&Merge Calendar..."), 0, this, SLOT(file_merge()),
                    mACollection, "merge_calendar");
  (void)new KAction(i18n("Archive Old Entries..."), 0, this, SLOT(file_archive()),
                    mACollection, "file_archive");

  (void)new KAction(i18n("Make Active"),0,this,SLOT(makeActive()),
                    mACollection,"make_active");

  // Settings menu.

  (void)new KAction(i18n("Configure &Date && Time..."), 0,
                    this,SLOT(configureDateTime()),
                    mACollection, "conf_datetime");

  mFilterViewAction = new KToggleAction(i18n("Show Filter"),0,this,
                                        SLOT(toggleFilterView()),
                                        mACollection,
                                        "show_filter");

  (void)new KAction(i18n("&Tip of the Day"), 0,
                    this, SLOT(showTip()), mACollection, "help_tipofday");

  new KAction( i18n("Get Hot New Stuff..."), 0, this,
               SLOT( downloadNewStuff() ), mACollection,
               "downloadnewstuff" );

  new KAction( i18n("Upload Hot New Stuff..."), 0, this,
               SLOT( uploadNewStuff() ), mACollection,
               "uploadnewstuff" );

  (void)new KAction(i18n("iCalendar..."), 0,
                    mCalendarView, SLOT(exportICalendar()),
                    mACollection, "export_icalendar");
  (void)new KAction(i18n("vCalendar..."), 0,
                    mCalendarView, SLOT(exportVCalendar()),
                    mACollection, "export_vcalendar");

// This is now done by KPrinter::setup().
#if 0
  (void)new KAction(i18n("Print Setup..."), 0,
                    mCalendarView, SLOT(printSetup()),
                    mACollection, "print_setup");
#endif

  if (mIsPart) {
    new KAction(i18n("&Print..."), "fileprint", CTRL+Key_P, mCalendarView,
		SLOT(print()), mACollection, "korganizer_print" );
  } else {
    KStdAction::print(mCalendarView, SLOT(print()), mACollection);
  }

#if 1
  if (mIsPart) {
    new KAction(i18n("Print Previe&w..."), "filequickprint", 0, mCalendarView,
		SLOT(printPreview()), mACollection, "korganizer_quickprint" );
  } else {
    KStdAction::printPreview(mCalendarView, SLOT(printPreview()),
			     mACollection);
  }
#endif

  new KAction( i18n("delete completed To-Dos","Purge Completed"), 0,
	       mCalendarView, SLOT( purgeCompleted() ), mACollection,
	       "purge_completed" );

  KOrg::History *h = mCalendarView->history();

  KAction *pasteAction;

  if ( mIsPart ) {
    // edit menu
    mCutAction = new KAction(i18n("Cu&t"), "editcut", CTRL+Key_X, mCalendarView,
			     SLOT(edit_cut()), mACollection, "korganizer_cut");
    mCopyAction = new KAction(i18n("&Copy"), "editcopy", CTRL+Key_C, mCalendarView,
			      SLOT(edit_copy()), mACollection, "korganizer_copy");
    pasteAction = new KAction(i18n("&Paste"), "editpaste", CTRL+Key_V, mCalendarView,
	                      SLOT(edit_paste()), mACollection, "korganizer_paste");
    mUndoAction = new KAction( i18n("&Undo"), "undo", CTRL+Key_Z, h,
                               SLOT( undo() ), mACollection, "korganizer_undo" );
    mRedoAction = new KAction( i18n("Re&do"), "redo", CTRL+SHIFT+Key_Z, h,
                               SLOT( redo() ), mACollection, "korganizer_redo" );
  } else {
    mCutAction = KStdAction::cut(mCalendarView,SLOT(edit_cut()),
				 mACollection);

    mCopyAction = KStdAction::copy(mCalendarView,SLOT(edit_copy()),
				   mACollection);

    pasteAction = KStdAction::paste(mCalendarView,SLOT(edit_paste()),
			       mACollection);

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

  mDeleteAction = new KAction(i18n("&Delete"),"editdelete",0,
                              mCalendarView,SLOT(appointment_delete()),
                              mACollection, "edit_delete");

  if ( mIsPart ) {
    new KAction(i18n("&Find..."),"find",CTRL+Key_F,
		mCalendarView->dialogManager(), SLOT(showSearchDialog()),
		mACollection, "korganizer_find");
  } else {
    KStdAction::find(mCalendarView->dialogManager(), SLOT(showSearchDialog()),
		     mACollection);
  }

  // view menu

  (void)new KAction(i18n("What's &Next"), "whatsnext", 0,
                    mCalendarView->viewManager(), SLOT(showWhatsNextView()),
                    mACollection, "view_whatsnext");
  (void)new KAction(i18n("&List"), "list", 0,
                    mCalendarView->viewManager(), SLOT(showListView()),
                    mACollection, "view_list");
  (void)new KAction(i18n("&Day"), "1day", 0,
                    mCalendarView->viewManager(), SLOT(showDayView()),
                    mACollection, "view_day");
  (void)new KAction(i18n("W&ork Week"), "5days", 0,
                    mCalendarView->viewManager(), SLOT(showWorkWeekView()),
                    mACollection, "view_workweek");
  (void)new KAction(i18n("&Week"), "7days", 0,
                    mCalendarView->viewManager(), SLOT(showWeekView()),
                    mACollection, "view_week");
  mNextXDays = new KAction("", "xdays", 0,mCalendarView->viewManager(),
                    SLOT(showNextXView()),mACollection, "view_nextx");
  mNextXDays->setText(i18n("&Next Day", "&Next %n Days", KOPrefs::instance()->mNextXDays));
  (void)new KAction(i18n("&Month"), "month", 0,
                    mCalendarView->viewManager(), SLOT(showMonthView()),
                    mACollection, "view_month");
  (void)new KAction(i18n("&To-Do List"), "todo", 0,
                    mCalendarView->viewManager(), SLOT(showTodoView()),
                    mACollection, "view_todo");
  (void)new KAction(i18n("&Journal"), "journal", 0,
                    mCalendarView->viewManager(), SLOT(showJournalView()),
                    mACollection, "view_journal");
  (void)new KAction(i18n("&Time Span"), "timespan", 0,
                    mCalendarView->viewManager(), SLOT(showTimeSpanView()),
                    mACollection, "view_timespan");
  (void)new KAction(i18n("&Update"), 0,
                    mCalendarView, SLOT(update()),
                    mACollection, "update");

  // actions menu

  (void)new KAction(i18n("New E&vent..."), "appointment", 0,
                    mCalendarView,SLOT( newEvent() ),
                    mACollection, "new_event");
  (void)new KAction(i18n("New &To-Do..."), "newtodo", 0,
                    mCalendarView,SLOT(newTodo()),
                    mACollection, "new_todo");
  action = new KAction(i18n("New Su&b-To-Do..."), 0,
                    mCalendarView,SLOT(newSubTodo()),
                    mACollection, "new_subtodo");
  action->setEnabled(false);
  connect(mCalendarView,SIGNAL(todoSelected(bool)),
          action,SLOT(setEnabled(bool)));

  mShowIncidenceAction = new KAction(i18n("&Show"), 0,
                         mCalendarView,SLOT(showIncidence()),
                         mACollection, "show_incidence");
  mEditIncidenceAction = new KAction(i18n("&Edit..."), 0,
                         mCalendarView,SLOT(editIncidence()),
                         mACollection, "edit_incidence");
  mDeleteIncidenceAction = new KAction(i18n("&Delete"), 0,
                         mCalendarView,SLOT(deleteIncidence()),
                         mACollection, "delete_incidence");

#if 0
  action = new KAction(i18n("T&ake over Event"), 0,
                       mCalendarView,SLOT(takeOverEvent()),
                       mACollection, "takeover_event");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  (void)new KAction(i18n("T&ake over Calendar"), 0,
                    mCalendarView,SLOT(takeOverCalendar()),
                    mACollection, "takeover_calendar");

  action = new KAction(i18n("&Mail Appointment"), "mail_generic", 0,
                    mCalendarView,SLOT(action_mail()),
                    mACollection, "mail_appointment");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
#endif

  action = new KAction(i18n("&Make Sub-To-Do Independent"), 0,
                    mCalendarView,SLOT(todo_unsub()),
                    mACollection, "unsub_todo");
  action->setEnabled(false);
  connect(mCalendarView,SIGNAL(todoSelected(bool)),
          action,SLOT(setEnabled(bool)));

  // Schedule menu.

  (void)new KAction(i18n("Outgoing Messages"),0,
                    mCalendarView->dialogManager(),SLOT(showOutgoingDialog()),
                    mACollection,"outgoing");
  (void)new KAction(i18n("Incoming Messages"),0,
                    mCalendarView->dialogManager(),SLOT(showIncomingDialog()),
                    mACollection,"incoming");
  mPublishEvent = new KAction(i18n("Publish..."),"mail_send",0,
                       mCalendarView,SLOT(schedule_publish()),
                       mACollection,"publish");
  mPublishEvent->setEnabled(false);
  action = new KAction(i18n("Request"),"mail_generic",0,
                       mCalendarView,SLOT(schedule_request()),
                       mACollection,"request");
  action->setEnabled(false);
  connect(mCalendarView,SIGNAL(organizerEventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("Refresh"),0,
                       mCalendarView,SLOT(schedule_refresh()),
                       mACollection,"refresh");
  action->setEnabled(false);
  connect(mCalendarView,SIGNAL(groupEventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("Cancel"),0,
                       mCalendarView,SLOT(schedule_cancel()),
                       mACollection,"cancel");
  action->setEnabled(false);
  connect(mCalendarView,SIGNAL(organizerEventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
/*  action = new KAction(i18n("Add"),0,
                       mCalendarView,SLOT(schedule_add()),
                       mACollection,"add");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
*/  action = new KAction(i18n("Reply"),"mail_reply",0,
                       mCalendarView,SLOT(schedule_reply()),
                       mACollection,"reply");
  action->setEnabled(false);
  connect(mCalendarView,SIGNAL(groupEventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("counter proposal","Counter"),0,
                       mCalendarView,SLOT(schedule_counter()),
                       mACollection,"counter");
  action->setEnabled(false);
  connect(mCalendarView,SIGNAL(groupEventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("Publish Free Busy Information"),0,
                       mCalendarView,SLOT(schedule_publish_freebusy()),
                       mACollection,"publish_freebusy");
  action->setEnabled(true);
/*  action = new KAction(i18n("Decline Counter"),0,
                       mCalendarView,SLOT(schedule_declinecounter()),
                       mACollection,"declinecounter");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
*/
  if ( !mIsPart ) {
      action = new KAction(i18n("Addressbook"),"contents",0,
			   mCalendarView,SLOT(openAddressbook()),
			   mACollection,"addressbook");
  }

  // Navigation menu
  bool isRTL = QApplication::reverseLayout();

  (void)new KAction(i18n("Go to &Today"), "today", 0,
                    mCalendarView,SLOT(goToday()),
                    mACollection, "go_today");
  action = new KAction(i18n("Go &Backward"), isRTL ? "1rightarrow" : "1leftarrow", 0,
                       mCalendarView,SLOT(goPrevious()),
                       mACollection, "go_previous");

// Changing the action text by setText makes the toolbar button disappear.
// This has to be fixed first, before the connects below can be reenabled.
/*
  connect(mCalendarView,SIGNAL(changeNavStringPrev(const QString &)),
          action,SLOT(setText(const QString &)));
  connect(mCalendarView,SIGNAL(changeNavStringPrev(const QString &)),
          this,SLOT(dumpText(const QString &)));
*/
  action = new KAction(i18n("Go &Forward"), isRTL ? "1leftarrow" : "1rightarrow", 0,
                       mCalendarView,SLOT(goNext()),
                       mACollection, "go_next");
/*
  connect(mCalendarView,SIGNAL(changeNavStringNext(const QString &)),
          action,SLOT(setText(const QString &)));
*/


  if ( mIsPart ) {
    new KAction( i18n("&Configure KOrganizer..."),
		 "configure", 0, mCalendarView,
		 SLOT(edit_options()), mACollection,
		 "korganizer_configure" );
    new KAction( i18n("Configure S&hortcuts..."),
		 "configure_shortcuts", 0, this,
		 SLOT(keyBindings()), mACollection,
		 "korganizer_configure_shortcuts" );
  } else {
    KStdAction::preferences(mCalendarView, SLOT(edit_options()),
			    mACollection);
    KStdAction::keyBindings(this, SLOT(keyBindings()), mACollection);
  }

  (void)new KAction(i18n("Edit C&ategories..."), 0,
                    mCalendarView->dialogManager(),
                    SLOT(showCategoryEditDialog()),
                    mACollection,"edit_categories");
  (void)new KAction(i18n("Edit &Filters..."), 0,
                    mCalendarView,SLOT(editFilters()),
                    mACollection,"edit_filters");
  (void)new KAction(i18n("Configure &Plugins..."), 0,
                    mCalendarView->dialogManager(),SLOT(showPluginDialog()),
                    mACollection,"configure_plugins");

#if 0
  (void)new KAction(i18n("Show Intro Page"), 0,
                    mCalendarView,SLOT(showIntro()),
                    mACollection,"show_intro");
#endif

  if (mIsPart) {
    mPluginMenu = new KActionMenu(i18n("Plugins"), 0, mACollection, "plugins" );
  } else {
    mPluginMenu = 0;
  }

  KConfig *config = KOGlobals::config();
  config->setGroup("Settings");
  mFilterViewAction->setChecked(config->readBoolEntry("Filter Visible",false));
  toggleFilterView();
}

void ActionManager::readSettings()
{
  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig *config = KOGlobals::config();
  mRecent->loadEntries( config );
  mCalendarView->readSettings();
}

void ActionManager::writeSettings()
{
  kdDebug(5850) << "ActionManager::writeSettings" << endl;
  KConfig *config = KOGlobals::config();
  mCalendarView->writeSettings();

  config->setGroup( "Settings" );
  config->writeEntry( "Filter Visible", mFilterViewAction->isChecked() );
  mRecent->saveEntries( config );
}

void ActionManager::file_new()
{
  emit actionNew();
}

void ActionManager::file_open()
{
  KURL url;
  QString defaultPath = locateLocal("data","korganizer/");
  url = KFileDialog::getOpenURL(defaultPath,i18n("*.vcs *.ics|Calendar Files"),
				mCalendarView->topLevelWidget());

  if (url.isEmpty()) return;

  KOrg::MainWindow *korg=ActionManager::findInstance(url);
  if ((0 != korg)&&(korg != mMainWindow)) {
    KWin::setActiveWindow(korg->topLevelWidget()->winId());
    return;
  }

  kdDebug(5850) << "ActionManager::file_open(): " << url.prettyURL() << endl;

  if (!mCalendarView->isModified() && mFile.isEmpty()) {
    openURL(url);
  } else {
    emit actionNew( url );
  }
}

void ActionManager::file_openRecent(const KURL& url)
{
  if (!url.isEmpty()) {
    KOrg::MainWindow *korg=ActionManager::findInstance(url);
    if ((0 != korg)&&(korg != mMainWindow)) {
      KWin::setActiveWindow(korg->topLevelWidget()->winId());
      return;
    }
    openURL(url);
  }
}

void ActionManager::file_import()
{
  // eventually, we will need a dialog box to select import type, etc.
  // for now, hard-coded to ical file, $HOME/.calendar.
  int retVal = -1;
  QString progPath;
  KTempFile tmpfn;

  QString homeDir = QDir::homeDirPath() + QString::fromLatin1("/.calendar");

  if (!QFile::exists(homeDir)) {
    KMessageBox::error(mCalendarView->topLevelWidget(),
		       i18n("You have no ical file in your home directory.\n"
		            "Import cannot proceed.\n"));
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

  if (retVal >= 0 && retVal <= 2) {
    // now we need to MERGE what is in the iCal to the current calendar.
    mCalendarView->openCalendar(tmpfn.name(),1);
    if (!retVal)
      KMessageBox::information(mCalendarView->topLevelWidget(),
			       i18n("KOrganizer successfully imported and "
				    "merged your .calendar file from ical "
				    "into the currently opened calendar."));
    else
      KMessageBox::information(mCalendarView->topLevelWidget(),
			   i18n("KOrganizer encountered some unknown fields while "
				"parsing your .calendar ical file, and had to "
				"discard them. Please check to see that all "
				"your relevant data was correctly imported."),
                                 i18n("ICal Import Successful With Warning"));
  } else if (retVal == -1) {
    KMessageBox::error(mCalendarView->topLevelWidget(),
			 i18n("KOrganizer encountered an error parsing your "
			      ".calendar file from ical. Import has failed."));
  } else if (retVal == -2) {
    KMessageBox::error(mCalendarView->topLevelWidget(),
			 i18n("KOrganizer doesn't think that your .calendar "
			      "file is a valid ical calendar. Import has failed."));
  }
  tmpfn.unlink();
}

void ActionManager::file_merge()
{
  KURL url = KFileDialog::getOpenURL(locateLocal("data","korganizer/"),
                                     i18n("*.vcs *.ics|Calendar Files"),
				     mCalendarView->topLevelWidget());
  openURL(url,true);
}

void ActionManager::file_archive()
{
  mCalendarView->archiveCalendar();
}

void ActionManager::file_revert()
{
  openURL(mURL);
}

void ActionManager::file_saveas()
{
  KURL url = getSaveURL();

  if (url.isEmpty()) return;

  saveAsURL(url);
}

void ActionManager::file_save()
{
  if ( mMainWindow->hasDocument() ) {
    if (mURL.isEmpty()) {
      file_saveas();
    } else {
      saveURL();
    }
  } else {
    mCalendarView->calendar()->save();
  }
}

void ActionManager::file_close()
{
  if (!saveModifiedURL()) return;

  mCalendarView->closeCalendar();
  KIO::NetAccess::removeTempFile(mFile);
  mURL="";
  mFile="";

  setActive(false);

  setTitle();
}

bool ActionManager::openURL(const KURL &url,bool merge)
{
  kdDebug(5850) << "ActionManager::openURL()" << endl;

  if (url.isEmpty()) {
    kdDebug(5850) << "ActionManager::openURL(): Error! Empty URL." << endl;
    return false;
  }
  if (url.isMalformed()) {
    kdDebug(5850) << "ActionManager::openURL(): Error! URL is malformed." << endl;
    return false;
  }

  QString tmpFile;
  if(KIO::NetAccess::download(url,tmpFile)) {
    kdDebug(5850) << "--- Downloaded to " << tmpFile << endl;
    bool success = mCalendarView->openCalendar(tmpFile,merge);
    if (merge) {
      KIO::NetAccess::removeTempFile(tmpFile);
      if (success)
        mMainWindow->showStatusMessage(i18n("Merged calendar '%1'.").arg(url.prettyURL()));
    } else {
      if (success) {
        KIO::NetAccess::removeTempFile(mFile);
        mURL = url;
        mFile = tmpFile;
	KConfig *config = KOGlobals::config();
	config->setGroup("General");
	QString active = config->readPathEntry("Active Calendar");
        if (KURL(active) == mURL) setActive(true);
        else setActive(false);
        setTitle();
        kdDebug(5850) << "-- Add recent URL: " << url.prettyURL() << endl;
        mRecent->addURL(url);
	mMainWindow->showStatusMessage(i18n("Opened calendar '%1'.").arg(mURL.prettyURL()));
      }
    }
    return success;
  } else {
    QString msg;
    msg = i18n("Cannot download calendar from '%1'.").arg(url.prettyURL());
    KMessageBox::error(mCalendarView->topLevelWidget(),msg);
    return false;
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

  if (mURL.isLocalFile()) {
    ext = mFile.right(4);
  } else {
    ext = mURL.filename().right(4);
  }

  if (ext == ".vcs") {
    int result = KMessageBox::warningContinueCancel(
	mCalendarView->topLevelWidget(),
        i18n("Your calendar will be saved in iCalendar format. Use "
              "'Export vCalendar' to save in vCalendar format."),
        i18n("Format Conversion"),i18n("Proceed"),"dontaskFormatConversion",
        true);
    if (result != KMessageBox::Continue) return false;

    // Tell the alarm daemon to stop monitoring the vCalendar file
    if ( !KOGlobals::self()->alarmClient()->removeCalendar( mURL.url() ) ) {
      kdDebug(5850) << "ActionManager::saveURL(): dcop send failed" << endl;
    }

    QString filename = mURL.fileName();
    filename.replace(filename.length()-4,4,".ics");
    mURL.setFileName(filename);
    if (mURL.isLocalFile()) {
      mFile = mURL.path();
    }
    writeActiveState();
    setTitle();
    mRecent->addURL(mURL);
  }

  if (!mCalendarView->saveCalendar(mFile)) {
    kdDebug(5850) << "ActionManager::saveURL(): calendar view save failed." << endl;
    return false;
  } else {
    mCalendarView->setModified( false );
  }

  if (!mURL.isLocalFile()) {
    if (!KIO::NetAccess::upload(mFile,mURL)) {
      QString msg = i18n("Cannot upload calendar to '%1'").arg(mURL.prettyURL());
      KMessageBox::error(mCalendarView->topLevelWidget(),msg);
      return false;
    }
  }

  if (isActive()) {
    kdDebug(5850) << "ActionManager::saveURL(): Notify alarm daemon" << endl;
    if ( !KOGlobals::self()->alarmClient()->reloadCalendar( mURL.url() ) ) {
      kdDebug(5850) << "ActionManager::saveUrl(): reloadCal call failed." << endl;
    }
  }

  // keep saves on a regular interval
  if (KOPrefs::instance()->mAutoSave) {
    mAutoSaveTimer->stop();
    mAutoSaveTimer->start(1000*60*KOPrefs::instance()->mAutoSaveInterval);
  }

  mMainWindow->showStatusMessage(i18n("Saved calendar '%1'.").arg(mURL.prettyURL()));

  // export to HTML
  if ( KOPrefs::instance()->mHtmlWithSave==true &&
        !KOPrefs::instance()->mHtmlExportFile.isNull() ) {
    KURL dest( KOPrefs::instance()->mHtmlExportFile );
    KCal::HtmlExport mExport( mCalendarView->calendar() );
    mExport.setEmail( KOPrefs::instance()->email() );
    mExport.setFullName( KOPrefs::instance()->fullName() );

    KConfig *cfg = KOGlobals::config();
    cfg->setGroup( "HtmlExport" );

    mExport.setMonthViewEnabled( cfg->readBoolEntry( "Month", false ) );
    mExport.setEventsEnabled( cfg->readBoolEntry( "Event", true ) );
    mExport.setTodosEnabled( cfg->readBoolEntry( "Todo", true ) );
    mExport.setCategoriesEventEnabled( cfg->readBoolEntry( "CategoriesEvent", false ) );
    mExport.setAttendeesEventEnabled( cfg->readBoolEntry( "AttendeesEvent", false ) );
    mExport.setExcludePrivateEventEnabled( cfg->readBoolEntry( "ExcludePrivateEvent", true ) );
    mExport.setExcludeConfidentialEventEnabled( cfg->readBoolEntry( "ExcludeConfidentialEvent", true ) );
    mExport.setCategoriesTodoEnabled( cfg->readBoolEntry( "CategoriesTodo", false ) );
    mExport.setAttendeesTodoEnabled( cfg->readBoolEntry( "AttendeesTodo", false ) );
    mExport.setExcludePrivateTodoEnabled( cfg->readBoolEntry( "ExcludePrivateTodo", true ) );
    mExport.setExcludeConfidentialTodoEnabled( cfg->readBoolEntry( "ExcludeConfidentialTodo", true ) );
    mExport.setDueDateEnabled( cfg->readBoolEntry( "DueDates", true ) );
    QDate qd1;
    qd1 = QDate::currentDate();
    QDate qd2;
    qd2 = QDate::currentDate();
    if ( mExport.monthViewEnabled() )
      qd2.addMonths( 1 );
    else
      qd2.addDays( 7 );

    mExport.setDateRange( qd1, qd2 );
    QDate cdate=qd1;
    while (cdate<=qd2)
    {
      if ( !KOCore::self()->holiday(cdate).isEmpty() )
        mExport.addHoliday( cdate, KOCore::self()->holiday(cdate) );
      cdate = cdate.addDays(1);
    }

    if ( dest.isLocalFile() ) {
      mExport.save( dest.path() );
    } else {
      KTempFile tf;
      QString tfile = tf.name();
      tf.close();
      mExport.save( tfile );
      if (!KIO::NetAccess::upload(tfile, dest) ) {
        KNotifyClient::event ( "Could not upload file." );
      }
      tf.unlink();
    }
  }

  return true;
}

bool ActionManager::saveAsURL(const KURL &url)
{
  kdDebug(5850) << "ActionManager::saveAsURL() " << url.prettyURL() << endl;

  if (url.isEmpty()) {
    kdDebug(5850) << "ActionManager::saveAsURL(): Empty URL." << endl;
    return false;
  }
  if (url.isMalformed()) {
    kdDebug(5850) << "ActionManager::saveAsURL(): Malformed URL." << endl;
    return false;
  }

  QString fileOrig = mFile;
  KURL URLOrig = mURL;

  KTempFile *tempFile = 0;
  if (url.isLocalFile()) {
    mFile = url.path();
  } else {
    tempFile = new KTempFile;
    mFile = tempFile->name();
  }
  mURL = url;

  bool success = saveURL(); // Save local file and upload local file
  if (success) {
    delete mTempFile;
    mTempFile = tempFile;
    KIO::NetAccess::removeTempFile(fileOrig);
    KConfig *config = KOGlobals::config();
    config->setGroup("General");
    QString active = config->readPathEntry("Active Calendar");
    if (KURL(active) == mURL) {
      setActive(true);
      emit calendarActivated(mMainWindow);
    } else {
      setActive(false);
    }
    setTitle();
    mRecent->addURL(mURL);
  } else {
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
  if (!mCalendarView->isModified()) return true;

  mHtmlExportSync = true;
  if (KOPrefs::instance()->mAutoSave && !mURL.isEmpty()) {
    // Save automatically, when auto save is enabled.
    return saveURL();
  } else {
    int result = KMessageBox::warningYesNoCancel(
	mCalendarView->topLevelWidget(),
        i18n("The calendar has been modified.\nDo you want to save it?"),
        QString::null,
        KStdGuiItem::save(), KStdGuiItem::discard());
    switch(result) {
      case KMessageBox::Yes:
        if (mURL.isEmpty()) {
          KURL url = getSaveURL();
          return saveAsURL(url);
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
  KURL url = KFileDialog::getSaveURL(locateLocal("data","korganizer/"),
                                     i18n("*.vcs *.ics|Calendar Files"),
				     mCalendarView->topLevelWidget());

  if (url.isEmpty()) return url;

  QString filename = url.fileName(false);

  QString e = filename.right(4);
  if (e != ".vcs" && e != ".ics") {
    // Default save format is iCalendar
    filename += ".ics";
  }

  url.setFileName(filename);

  kdDebug(5850) << "ActionManager::getSaveURL(): url: " << url.url() << endl;

  return url;
}

void ActionManager::saveProperties(KConfig *config)
{
  config->writePathEntry("Calendar",mURL.url());
}

void ActionManager::readProperties(KConfig *config)
{
  QString calendarUrl = config->readPathEntry("Calendar");
  if (!calendarUrl.isEmpty()) {
    KURL u(calendarUrl);
    openURL(u);
    KConfig *config = KOGlobals::config();
    config->setGroup("General");
    QString active = config->readPathEntry("Active Calendar");
    if (active == calendarUrl) setActive(true);
  }
}

void ActionManager::checkAutoSave()
{
  kdDebug(5850) << "ActionManager::checkAutoSave()" << endl;

  // Don't save if auto save interval is zero
  if (KOPrefs::instance()->mAutoSaveInterval == 0) return;

  // has this calendar been saved before? If yes automatically save it.
  if (KOPrefs::instance()->mAutoSave && !mURL.isEmpty()) {
    saveURL();
  }
}


// Configuration changed as a result of the options dialog.
void ActionManager::updateConfig()
{
  kdDebug(5850) << "ActionManager::updateConfig()" << endl;

  if (KOPrefs::instance()->mAutoSave && !mAutoSaveTimer->isActive()) {
    checkAutoSave();
    if (KOPrefs::instance()->mAutoSaveInterval > 0) {
      mAutoSaveTimer->start(1000*60*KOPrefs::instance()->mAutoSaveInterval);
    }
  }
  if (!KOPrefs::instance()->mAutoSave) mAutoSaveTimer->stop();
  mNextXDays->setText(i18n("&Next Day", "&Next %n Days", KOPrefs::instance()->mNextXDays));

  if (mPluginMenu)
      mPluginMenu->popupMenu()->clear();
  KOCore::self()->reloadPlugins();
  mParts = KOCore::self()->reloadParts( mMainWindow, mParts );
}

void ActionManager::configureDateTime()
{
  KProcess *proc = new KProcess;
  *proc << "kcmshell" << "language";

  connect(proc,SIGNAL(processExited(KProcess *)),
          SLOT(configureDateTimeFinished(KProcess *)));

  if (!proc->start()) {
      KMessageBox::sorry(mCalendarView->topLevelWidget(),
        i18n("Couldn't start control module for date and time format."));
  }
}

void ActionManager::showTip()
{
  KTipDialog::showTip(mCalendarView->topLevelWidget(),QString::null,true);
}

void ActionManager::showTipOnStart()
{
  KTipDialog::showTip(mCalendarView->topLevelWidget());
}

KOrg::MainWindow* ActionManager::findInstance(const KURL &url)
{
  if (windowList)
    return windowList->findInstance(url);
  else
    return 0;
}

void ActionManager::setActive(bool active)
{
  if (active == mActive) return;

  mActive = active;
  setTitle();
}

void ActionManager::makeActive()
{
  if (mURL.isEmpty()) {
    KMessageBox::sorry(mCalendarView->topLevelWidget(),
		       i18n("The calendar does not have a filename. "
			    "Please save it before activating."));
    return;
  }

  if (!mURL.isLocalFile()) {
    int result = KMessageBox::warningContinueCancel(
      mCalendarView->topLevelWidget(),
      i18n("Your calendar is a remote file. Activating it can cause "
           "synchronization problems leading to data loss.\n"
           "Make sure that it is accessed by no more than one single "
           "KOrganizer instance at the same time."),
      i18n("Activating Calendar."),i18n("Activate Calendar"),"dontaskActivate",
      true);
    if (result == KMessageBox::Cancel) return;
  }

  writeActiveState();
  if ( !KOGlobals::self()->alarmClient()->reloadCalendar( mURL.url() ) ) {
    kdDebug(5850) << "ActionManager::makeActive(): dcop send failed" << endl;
  }
  setActive();
  emit calendarActivated(mMainWindow);
}

void ActionManager::writeActiveState()
{
  KConfig *config = KOGlobals::config();
  config->setGroup("General");
  config->writePathEntry("Active Calendar",mURL.url());
  config->sync();
}

void ActionManager::dumpText(const QString &str)
{
  kdDebug(5850) << "ActionManager::dumpText(): " << str << endl;
}

void ActionManager::toggleFilterView()
{
  bool visible = mFilterViewAction->isChecked();
  mCalendarView->showFilter(visible);
}

bool ActionManager::openURL(QString url)
{
  return openURL(KURL(url));
}

bool ActionManager::mergeURL(QString url)
{
  return openURL(KURL(url),true);
}

bool ActionManager::saveAsURL(QString url)
{
  return saveAsURL(KURL(url));
}

QString ActionManager::getCurrentURLasString() const
{
  return mURL.url();
}

bool ActionManager::deleteEvent(QString uid)
{
  return mCalendarView->deleteEvent(uid);
}

bool ActionManager::eventRequest(QString request, QCString receiver,
				 QString ical)
{
  if( !KOGroupware::instance() ) return false;
  return KOGroupware::instance()->incomingEventRequest(request, receiver,
						       ical);
}

void ActionManager::configureDateTimeFinished(KProcess *proc)
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

void ActionManager::processIncidenceSelection( Incidence *incidence )
{
//  kdDebug(5850) << "ActionManager::processIncidenceSelection()" << endl;

  if ( !incidence ) {
    enableIncidenceActions( false );
    return;
  }

  enableIncidenceActions( true );

  if ( incidence->type() == "Event" ) {
    mShowIncidenceAction->setText( i18n("&Show Event") );
    mEditIncidenceAction->setText( i18n("&Edit Event...") );
    mDeleteIncidenceAction->setText( i18n("&Delete Event") );
  } else if ( incidence->type() == "Todo" ) {
    mShowIncidenceAction->setText( i18n("&Show To-Do") );
    mEditIncidenceAction->setText( i18n("&Edit To-Do...") );
    mDeleteIncidenceAction->setText( i18n("&Delete To-Do") );
  } else {
    mShowIncidenceAction->setText( i18n("&Show") );
    mShowIncidenceAction->setText( i18n("&Edit...") );
    mShowIncidenceAction->setText( i18n("&Delete") );
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
  emit actionKeyBindings();
}


void ActionManager::loadParts()
{
  if ( mPluginMenu ) mPluginMenu->popupMenu()->clear();
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

void ActionManager::openEventEditor( QString text )
{
  mCalendarView->newEvent( text );
}

void ActionManager::showTodoView()
{
  mCalendarView->viewManager()->showTodoView();
}

void ActionManager::showEventView()
{
  mCalendarView->viewManager()->showEventView();
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
    mRedoAction->setText( i18n("Redo") );
  } else {
    mRedoAction->setEnabled( true );
    if ( text.isEmpty() ) mRedoAction->setText( i18n("Redo") );
    else mRedoAction->setText( i18n("Redo (%1)").arg( text ) );
  }
}


#include "actionmanager.moc"
