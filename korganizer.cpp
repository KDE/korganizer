/*
    This file is part of KOrganizer.
    Copyright (c) 1997, 1998, 1999
    Preston Brown (preston.brown@yale.edu)
    Fester Zigterman (F.J.F.ZigtermanRustenburg@student.utwente.nl)
    Ian Dawes (iadawes@globalserve.net)
    Laszlo Boloni (boloni@cs.purdue.edu)
    Copyright (c) 2000, 2001, 2002 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <stdlib.h>

#include <qcursor.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qfile.h>

#include <kglobal.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kstdaccel.h>
#include <kfiledialog.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kedittoolbar.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <dcopclient.h>
#include <kprocess.h>
#include <kwin.h>
#include <kkeydialog.h>
#include <ktip.h>
#include <kstdguiitem.h>

#include <korganizer/part.h>

#include "komailclient.h"
#include "calprinter.h"
#include "calendarview.h"
#include "koviewmanager.h"
#include "kodialogmanager.h"
#include "kowindowlist.h"
#include "koprefs.h"
#include "kocore.h"
#include "konewstuff.h"

#include "korganizer.h"
using namespace KOrg;
#include "korganizer.moc"


KOWindowList *KOrganizer::windowList = 0;

KOrganizer::KOrganizer( const char *name ) 
  : MainWindow(name), DCOPObject("KOrganizerIface"),
    mAlarmDaemonIface("kalarmd","ad")
{
  kdDebug() << "KOrganizer::KOrganizer()" << endl;

  mTempFile = 0;
  mActive = false;
  mNewStuff = 0;

  // add this instance of the window to the static list.
  if (!windowList) {
    windowList = new KOWindowList;
    // Show tip of the day, when the first main window is shown.
    QTimer::singleShot(0,this,SLOT(showTipOnStart()));
  }
  windowList->addWindow(this);

//  setMinimumSize(600,400);	// make sure we don't get resized too small...

  mCalendarView = new CalendarView( this, "KOrganizer::CalendarView" ); 
  setCentralWidget(mCalendarView);

  initActions();

  initParts();
//  initViews();

  statusBar()->insertItem("",ID_GENERAL,10);

//  statusBar()->insertFixedItem(i18n("Active"),ID_ACTIVE);

  statusBar()->insertItem(i18n(" Incoming messages: %1 ").arg(0),
                            ID_MESSAGES_IN);
  statusBar()->insertItem(i18n(" Outgoing messages: %2 ").arg(0),
                            ID_MESSAGES_OUT);
  statusBar()->setItemAlignment(ID_MESSAGES_IN,AlignRight);
  statusBar()->setItemAlignment(ID_MESSAGES_OUT,AlignRight);
  connect(statusBar(),SIGNAL(pressed(int)),SLOT(statusBarPressed(int)));

  readSettings();
  mCalendarView->readSettings();

  // set up autoSaving stuff
  mAutoSaveTimer = new QTimer(this);
  connect(mAutoSaveTimer,SIGNAL(timeout()),SLOT(checkAutoSave()));
  if (KOPrefs::instance()->mAutoSave && 
      KOPrefs::instance()->mAutoSaveInterval > 0) {
    mAutoSaveTimer->start(1000*60*KOPrefs::instance()->mAutoSaveInterval);
  }

  setTitle();

  connect(mCalendarView,SIGNAL(modifiedChanged(bool)),SLOT(setTitle()));
  connect(mCalendarView,SIGNAL(configChanged()),SLOT(updateConfig()));

  connect(mCalendarView,SIGNAL(numIncomingChanged(int)),
          SLOT(setNumIncoming(int)));
  connect(mCalendarView,SIGNAL(numOutgoingChanged(int)),
          SLOT(setNumOutgoing(int)));

  connect(mCalendarView,SIGNAL(statusMessage(const QString &)),
          SLOT(showStatusMessage(const QString &)));

  connect( mCalendarView, SIGNAL( incidenceSelected( Incidence * ) ),
           SLOT( processIncidenceSelection( Incidence * ) ) );

  processIncidenceSelection( 0 );

  // Update state of paste action
  mCalendarView->checkClipboard();
  
  mCalendarView->lookForOutgoingMessages();
  mCalendarView->lookForIncomingMessages();

  kdDebug() << "KOrganizer::KOrganizer() done" << endl;
}

bool KOrganizer::startedKAddressBook = false;

KOrganizer::~KOrganizer()
{
  kdDebug() << "~KOrganizer()" << endl;

  delete mNewStuff;
  
  //close down KAddressBook if we started it
  if (KOrganizer::startedKAddressBook == true)
  {
   kdDebug() << "Closing down kaddressbook" << endl;
   DCOPClient *client = KApplication::kApplication()->dcopClient();
   const QByteArray noParamData;
   client->send("kaddressbook", "KAddressBookIface", "exit()",  noParamData);
  }

  if (mTempFile) delete mTempFile;

  // Take this window out of the window list.
  windowList->removeWindow(this);

  delete mCalendarView;

  kdDebug() << "~KOrganizer() done" << endl;
}


void KOrganizer::readSettings()
{
  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig *config = kapp->config();

  config->setGroup("KOrganizer Geometry");

  int windowWidth = config->readNumEntry("Width",600);
  int windowHeight = config->readNumEntry("Height",400);

  resize(windowWidth,windowHeight);

  mRecent->loadEntries(config);

  mCalendarView->readSettings();
    
  config->sync();
}


void KOrganizer::writeSettings()
{
  kdDebug() << "KOrganizer::writeSettings" << endl;

  KConfig *config = kapp->config();

  config->setGroup("KOrganizer Geometry");

  config->writeEntry("Width",width());
  config->writeEntry("Height",height());

  mCalendarView->writeSettings();

  config->setGroup("Settings");
  config->writeEntry("Filter Visible",mFilterViewAction->isChecked());

  mRecent->saveEntries(config);

  saveMainWindowSettings(config);

  config->sync();
}


void KOrganizer::initActions()
{
  KAction *action;

  // File menu.

  KStdAction::openNew(this, SLOT(file_new()), actionCollection());
  KStdAction::open(this, SLOT(file_open()), actionCollection());
  mRecent = KStdAction::openRecent(this, SLOT(file_openRecent(const KURL&)),
                                   actionCollection());
  KStdAction::revert(this,SLOT(file_revert()),actionCollection());
  KStdAction::save(this, SLOT(file_save()), actionCollection());
  KStdAction::saveAs(this, SLOT(file_saveas()), actionCollection());
  KStdAction::close(this, SLOT(file_close()), actionCollection());
  (void)new KAction(i18n("&Import From Ical"), 0, this, SLOT(file_import()),
                    actionCollection(), "import_ical");
  (void)new KAction(i18n("&Merge Calendar..."), 0, this, SLOT(file_merge()),
                    actionCollection(), "merge_calendar");
  (void)new KAction(i18n("Archive old Entries..."), 0, this, SLOT(file_archive()),
                    actionCollection(), "file_archive");

  (void)new KAction(i18n("iCalendar..."), 0,
                    mCalendarView, SLOT(exportICalendar()),
                    actionCollection(), "export_icalendar");
  (void)new KAction(i18n("vCalendar..."), 0,
                    mCalendarView, SLOT(exportVCalendar()),
                    actionCollection(), "export_vcalendar");

// This is now done by KPrinter::setup().
#if 0
  (void)new KAction(i18n("Print Setup..."), 0,
                    mCalendarView, SLOT(printSetup()),
                    actionCollection(), "print_setup");
#endif

  KStdAction::print(mCalendarView, SLOT(print()), actionCollection());
#if 1
  KStdAction::printPreview(mCalendarView, SLOT(printPreview()),
                           actionCollection());
#endif
  (void)new KAction(i18n("Make Active"),0,this,SLOT(makeActive()),
                    actionCollection(),"make_active");
  KStdAction::quit(this, SLOT(close()), actionCollection());


  new KAction( i18n("delete completed To-Dos","Purge Completed"), 0,
               mCalendarView, SLOT( purgeCompleted() ), actionCollection(),
               "purge_completed" );

  // edit menu

  mCutAction = KStdAction::cut(mCalendarView,SLOT(edit_cut()),
                               actionCollection());

  mCopyAction = KStdAction::copy(mCalendarView,SLOT(edit_copy()),
                                 actionCollection());

  action = KStdAction::paste(mCalendarView,SLOT(edit_paste()),
                             actionCollection());
  action->setEnabled( false );
  connect( mCalendarView, SIGNAL( pasteEnabled( bool ) ),
           action, SLOT( setEnabled( bool ) ) );

  mDeleteAction = new KAction(i18n("&Delete"),"editdelete",0,
                              mCalendarView,SLOT(appointment_delete()),
                              actionCollection(), "edit_delete");

  KStdAction::find(mCalendarView->dialogManager(), SLOT(showSearchDialog()),
                   actionCollection());


  // view menu

  (void)new KAction(i18n("What's &Next"), "whatsnext", 0,
                    mCalendarView->viewManager(), SLOT(showWhatsNextView()),
                    actionCollection(), "view_whatsnext");
  (void)new KAction(i18n("&List"), "list", 0,
                    mCalendarView->viewManager(), SLOT(showListView()),
                    actionCollection(), "view_list");
  (void)new KAction(i18n("&Day"), "1day", 0,
                    mCalendarView->viewManager(), SLOT(showDayView()),
                    actionCollection(), "view_day");
  (void)new KAction(i18n("W&ork Week"), "5days", 0,
                    mCalendarView->viewManager(), SLOT(showWorkWeekView()),
                    actionCollection(), "view_workweek");
  (void)new KAction(i18n("&Week"), "7days", 0,
                    mCalendarView->viewManager(), SLOT(showWeekView()),
                    actionCollection(), "view_week");
  mNextXDays = new KAction(
                    i18n("&Next %1 Days").arg(KOPrefs::instance()->mNextXDays),
                    "xdays", 0,mCalendarView->viewManager(),
                    SLOT(showNextXView()),actionCollection(), "view_nextx");
  (void)new KAction(i18n("&Month"), "month", 0,
                    mCalendarView->viewManager(), SLOT(showMonthView()),
                    actionCollection(), "view_month");
  (void)new KAction(i18n("&To-Do List"), "todo", 0,
                    mCalendarView->viewManager(), SLOT(showTodoView()),
                    actionCollection(), "view_todo");
  (void)new KAction(i18n("&Journal"), 0,
                    mCalendarView->viewManager(), SLOT(showJournalView()),
                    actionCollection(), "view_journal");
  (void)new KAction(i18n("&Time Span"), "timespan", 0,
                    mCalendarView->viewManager(), SLOT(showTimeSpanView()),
                    actionCollection(), "view_timespan");
  (void)new KAction(i18n("&Update"), 0,
                    mCalendarView, SLOT(update()),
                    actionCollection(), "update");

  // actions menu

  (void)new KAction(i18n("New E&vent..."), "appointment", 0,
                    mCalendarView,SLOT(appointment_new()),
                    actionCollection(), "new_event");
  (void)new KAction(i18n("New &To-Do..."), "newtodo", 0,
                    mCalendarView,SLOT(newTodo()),
                    actionCollection(), "new_todo");
  action = new KAction(i18n("New Su&b-To-Do..."), 0,
                    mCalendarView,SLOT(newSubTodo()),
                    actionCollection(), "new_subtodo");
  action->setEnabled(false);
  connect(mCalendarView,SIGNAL(todoSelected(bool)),
          action,SLOT(setEnabled(bool)));

  mShowIncidenceAction = new KAction(i18n("&Show..."), 0,
                         mCalendarView,SLOT(showIncidence()),
                         actionCollection(), "show_incidence");
  mEditIncidenceAction = new KAction(i18n("&Edit..."), 0,
                         mCalendarView,SLOT(editIncidence()),
                         actionCollection(), "edit_incidence");
  mDeleteIncidenceAction = new KAction(i18n("&Delete..."), 0,
                         mCalendarView,SLOT(deleteIncidence()),
                         actionCollection(), "delete_incidence");

#if 0
  action = new KAction(i18n("T&ake over Event"), 0,
                       mCalendarView,SLOT(takeOverEvent()),
                       actionCollection(), "takeover_event");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  (void)new KAction(i18n("T&ake over Calendar"), 0,
                    mCalendarView,SLOT(takeOverCalendar()),
                    actionCollection(), "takeover_calendar");

  action = new KAction(i18n("&Mail Appointment"), "mail_generic", 0,
                    mCalendarView,SLOT(action_mail()),
                    actionCollection(), "mail_appointment");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
#endif

  action = new KAction(i18n("&Make Sub-To-Do Independent"), 0,
                    mCalendarView,SLOT(todo_unsub()),
                    actionCollection(), "unsub_todo");
  action->setEnabled(false);
  connect(mCalendarView,SIGNAL(todoSelected(bool)),
          action,SLOT(setEnabled(bool)));

  // Schedule menu.

  (void)new KAction(i18n("Outgoing Messages"),0,
                    mCalendarView->dialogManager(),SLOT(showOutgoingDialog()),
                    actionCollection(),"outgoing");
  (void)new KAction(i18n("Incoming Messages"),0,
                    mCalendarView->dialogManager(),SLOT(showIncomingDialog()),
                    actionCollection(),"incoming");
  mPublishEvent = new KAction(i18n("Publish..."),"mail_send",0,
                       mCalendarView,SLOT(schedule_publish()),
                       actionCollection(),"publish");
  mPublishEvent->setEnabled(false);
  action = new KAction(i18n("Request"),"mail_generic",0,
                       mCalendarView,SLOT(schedule_request()),
                       actionCollection(),"request");
  action->setEnabled(false);
  connect(mCalendarView,SIGNAL(organizerEventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("Refresh"),0,
                       mCalendarView,SLOT(schedule_refresh()),
                       actionCollection(),"refresh");
  action->setEnabled(false);
  connect(mCalendarView,SIGNAL(groupEventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("Cancel"),0,
                       mCalendarView,SLOT(schedule_cancel()),
                       actionCollection(),"cancel");
  action->setEnabled(false);
  connect(mCalendarView,SIGNAL(organizerEventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
/*  action = new KAction(i18n("Add"),0,
                       mCalendarView,SLOT(schedule_add()),
                       actionCollection(),"add");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
*/  action = new KAction(i18n("Reply"),"mail_reply",0,
                       mCalendarView,SLOT(schedule_reply()),
                       actionCollection(),"reply");
  action->setEnabled(false);
  connect(mCalendarView,SIGNAL(groupEventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("counter proposal","Counter"),0,
                       mCalendarView,SLOT(schedule_counter()),
                       actionCollection(),"counter");
  action->setEnabled(false);
  connect(mCalendarView,SIGNAL(groupEventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("Publish Free Busy Information"),0,
                       mCalendarView,SLOT(schedule_publish_freebusy()),
                       actionCollection(),"publish_freebusy");
  action->setEnabled(true);
/*  action = new KAction(i18n("Decline Counter"),0,
                       mCalendarView,SLOT(schedule_declinecounter()),
                       actionCollection(),"declinecounter");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
*/
  action = new KAction(i18n("Addressbook..."),"contents",0,
                       mCalendarView,SLOT(openAddressbook()),
                       actionCollection(),"addressbook");

  // Navigation menu
  bool isRTL = QApplication::reverseLayout();

  (void)new KAction(i18n("Go to &Today"), "today", 0,
                    mCalendarView,SLOT(goToday()),
                    actionCollection(), "go_today");
  action = new KAction(i18n("Go &Backward"), isRTL ? "1rightarrow" : "1leftarrow", 0,
                       mCalendarView,SLOT(goPrevious()),
                       actionCollection(), "go_previous");

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
                       actionCollection(), "go_next");
/*
  connect(mCalendarView,SIGNAL(changeNavStringNext(const QString &)),
          action,SLOT(setText(const QString &)));
*/


  // Settings menu.

  (void)new KAction(i18n("Configure &Date && Time..."), 0,
                    this,SLOT(configureDateTime()),
                    actionCollection(), "conf_datetime");

  mStatusBarAction = KStdAction::showStatusbar(this,SLOT(toggleStatusBar()),
                                               actionCollection());

  mFilterViewAction = new KToggleAction(i18n("Show Filter"),0,this,
                                        SLOT(toggleFilterView()),
                                        actionCollection(),
                                        "show_filter");

  KStdAction::configureToolbars(this, SLOT(configureToolbars()),
                                actionCollection());
  KStdAction::preferences(mCalendarView, SLOT(edit_options()),
                          actionCollection());
  KStdAction::keyBindings(this, SLOT(editKeys()), actionCollection());

  (void)new KAction(i18n("Edit C&ategories..."), 0,
                    mCalendarView->dialogManager(),
                    SLOT(showCategoryEditDialog()),
                    actionCollection(),"edit_categories");
  (void)new KAction(i18n("Edit &Filters..."), 0,
                    mCalendarView,SLOT(editFilters()),
                    actionCollection(),"edit_filters");
  (void)new KAction(i18n("Configure &Plugins..."), 0,
                    mCalendarView->dialogManager(),SLOT(showPluginDialog()),
                    actionCollection(),"configure_plugins");

#if 0
  (void)new KAction(i18n("Show Intro Page"), 0,
                    mCalendarView,SLOT(showIntro()),
                    actionCollection(),"show_intro");
#endif

  (void)new KAction(i18n("&Tip of the Day..."), 0,
                    this, SLOT(showTip()), actionCollection(), "help_tipofday");

  new KAction( i18n("Get Hot New Stuff..."), 0, this,
               SLOT( downloadNewStuff() ), actionCollection(),
               "downloadnewstuff" );
                 
  new KAction( i18n("Upload Hot New Stuff..."), 0, this,
               SLOT( uploadNewStuff() ), actionCollection(),
               "uploadnewstuff" );

  setInstance( KGlobal::instance() );
  
  setXMLFile("korganizerui.rc");
  createGUI(0);

  KConfig *config = kapp->config();

  applyMainWindowSettings(config);

  config->setGroup("Settings");
  mFilterViewAction->setChecked(config->readBoolEntry("Filter Visible",false));
  toggleFilterView();

  mStatusBarAction->setChecked(!statusBar()->isHidden());

  QPtrListIterator<KToolBar> it = toolBarIterator();
  for ( ; it.current() ; ++it ) {
    KToggleAction *act = new KToggleAction(i18n("Show %1 Toolbar")
                                           .arg((*it)->text()),0,
                                           actionCollection(),(*it)->name());
    connect( act,SIGNAL(toggled(bool)),SLOT(toggleToolBars(bool)));
    act->setChecked(!(*it)->isHidden());
    mToolBarToggles.append(act);
  }
  plugActionList("toolbartoggles",mToolBarToggles);
}

void KOrganizer::initParts()
{
  kdDebug() << "KOrganizer::initParts()" << endl;

  KOrg::Part::List parts = KOCore::self()->loadParts(this);
  KOrg::Part *it;
  for( it=parts.first(); it; it=parts.next() ) {    
    guiFactory()->addClient(it);
  }
}

#if 0
void KOrganizer::initViews()
{
  kdDebug() << "KOrganizer::initViews()" << endl;

  // TODO: get calendar pointer from somewhere
  KOrg::View::List views = KOCore::self()->views(this);
  KOrg::View *it;
  for( it=views.first(); it; it=views.next() ) {
    guiFactory()->addClient(it);
  }
}
#endif

void KOrganizer::file_new()
{
  // Make new KOrganizer window containing empty calendar
  (new KOrganizer())->show();
}

void KOrganizer::file_open()
{
  KURL url;
  QString defaultPath = locateLocal("appdata", "");
  url = KFileDialog::getOpenURL(defaultPath,i18n("*.vcs *.ics|Calendar files"),this);

  if (url.isEmpty()) return;

  KOrganizer *korg=KOrganizer::findInstance(url);
  if ((0 != korg)&&(korg != this)) {
    KWin::setActiveWindow(korg->winId());
    return;
  }

  kdDebug() << "KOrganizer::file_open(): " << url.prettyURL() << endl;

  if (!mCalendarView->isModified() && mFile.isEmpty()) {
    openURL(url);
  } else {
    KOrganizer *korg = new KOrganizer;
    if (korg->openURL(url)) {
      korg->show();
    } else {
      delete korg;
    }
  }
}

void KOrganizer::file_openRecent(const KURL& url)
{
  if (!url.isEmpty()) {
    KOrganizer *korg=KOrganizer::findInstance(url);
    if ((0 != korg)&&(korg != this)) {
      KWin::setActiveWindow(korg->winId());
      return;
    }
    openURL(url);
  }
}

void KOrganizer::file_import()
{
  // eventually, we will need a dialog box to select import type, etc.
  // for now, hard-coded to ical file, $HOME/.calendar.
  int retVal = -1;
  QString progPath;
  KTempFile tmpfn;

  QString homeDir = QDir::homeDirPath() + QString::fromLatin1("/.calendar");
		  
  if (!QFile::exists(homeDir)) {
    KMessageBox::error(this,
		       i18n("You have no ical file in your home directory.\n"
		            "Import cannot proceed.\n"));
    return;
  }
  
  KProcess proc;
  proc << "ical2vcal" << tmpfn.name();
  bool success = proc.start( KProcess::Block );

  if ( !success ) {
    kdDebug() << "Error starting ical2vcal." << endl;
    return;
  } else {
    retVal = proc.exitStatus();
  } 

  kdDebug() << "ical2vcal return value: " << retVal << endl;
  
  if (retVal >= 0 && retVal <= 2) {
    // now we need to MERGE what is in the iCal to the current calendar.
    mCalendarView->openCalendar(tmpfn.name(),1);
    if (!retVal)
      KMessageBox::information(this,
			       i18n("KOrganizer succesfully imported and "
				    "merged your .calendar file from ical "
				    "into the currently opened calendar."));
    else
      KMessageBox::information(this,
			   i18n("KOrganizer encountered some unknown fields while "
				"parsing your .calendar ical file, and had to "
				"discard them. Please check to see that all "
				"your relevant data was correctly imported."),
                                 i18n("ICal Import Successful With Warning"));
  } else if (retVal == -1) {
    KMessageBox::error(this,
			 i18n("KOrganizer encountered an error parsing your "
			      ".calendar file from ical. Import has failed."));
  } else if (retVal == -2) {
    KMessageBox::error(this,
			 i18n("KOrganizer doesn't think that your .calendar "
			      "file is a valid ical calendar. Import has failed."));
  }
  tmpfn.unlink();
}

void KOrganizer::file_merge()
{
  KURL url = KFileDialog::getOpenURL(locateLocal("appdata", ""),
                                     i18n("*.vcs *.ics|Calendar files"),this);
  openURL(url,true);
}

void KOrganizer::file_archive()
{
  mCalendarView->archiveCalendar();
}

void KOrganizer::file_revert()
{
  openURL(mURL);
}

void KOrganizer::file_saveas()
{
  KURL url = getSaveURL();

  if (url.isEmpty()) return;

  saveAsURL(url);
}

void KOrganizer::file_save()
{
  if (mURL.isEmpty()) file_saveas();
  else saveURL();
}

void KOrganizer::file_close()
{
  if (!saveModifiedURL()) return;

  mCalendarView->closeCalendar();
  KIO::NetAccess::removeTempFile(mFile);
  mURL="";
  mFile="";

  setActive(false);
  
  setTitle();
}

void KOrganizer::file_quit()
{
  close();
}


bool KOrganizer::openURL(const KURL &url,bool merge)
{
  kdDebug() << "KOrganizer::openURL()" << endl;

  if (url.isEmpty()) {
    kdDebug() << "KOrganizer::openURL(): Error! Empty URL." << endl;
    return false;
  }
  if (url.isMalformed()) {
    kdDebug() << "KOrganizer::openURL(): Error! URL is malformed." << endl;
    return false;
  }

  QString tmpFile;
  if(KIO::NetAccess::download(url,tmpFile)) {
    kdDebug() << "--- Downloaded to " << tmpFile << endl;
    bool success = mCalendarView->openCalendar(tmpFile,merge);
    if (merge) {
      KIO::NetAccess::removeTempFile(tmpFile);
      if (success) {
        showStatusMessage(i18n("Merged calendar '%1'.").arg(url.prettyURL()));
      }
    } else {
      if (success) {
        KIO::NetAccess::removeTempFile(mFile);
        mURL = url;
        mFile = tmpFile;
        KGlobal::config()->setGroup("General");
        QString active = KGlobal::config()->readEntry("Active Calendar");
        if (KURL(active) == mURL) setActive(true);
        else setActive(false);
        setTitle();
        kdDebug() << "-- Add recent URL: " << url.prettyURL() << endl;
        mRecent->addURL(url);
        showStatusMessage(i18n("Opened calendar '%1'.").arg(mURL.prettyURL()));
      }
    }
    return success;
  } else {
    QString msg;
    msg = i18n("Cannot download calendar from '%1'.").arg(url.prettyURL());
    KMessageBox::error(this,msg);
    return false;
  }
}

void KOrganizer::closeURL()
{
  kdDebug() << "KOrganizer::closeURL()" << endl;

  file_close();
}

bool KOrganizer::saveURL()
{
  QString ext;

  if (mURL.isLocalFile()) {
    ext = mFile.right(4);
  } else {
    ext = mURL.filename().right(4);
  }

  if (ext == ".vcs") {
    int result = KMessageBox::warningContinueCancel(this,
        i18n("Your calendar will be saved in iCalendar format. Use "
              "'Export vCalendar' to save in vCalendar format."),
        i18n("Format Conversion"),i18n("Proceed"),"dontaskFormatConversion",
        true);
    if (result != KMessageBox::Continue) return false;

    // Tell the alarm daemon to stop monitoring the vCalendar file
    mAlarmDaemonIface.removeCal( mURL.url() );
    if (!mAlarmDaemonIface.ok() ) {
      kdDebug() << "KOrganizer::saveURL(): dcop send failed" << endl;
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
    kdDebug() << "KOrganizer::saveURL(): calendar view save failed." << endl;
    return false;
  } else {
    mCalendarView->setModified( false );
  }

  if (!mURL.isLocalFile()) {
    if (!KIO::NetAccess::upload(mFile,mURL)) {
      QString msg = i18n("Cannot upload calendar to '%1'").arg(mURL.prettyURL());
      KMessageBox::error(this,msg);
      return false;
    }
  }

  if (isActive()) {
    kdDebug() << "KOrganizer::saveURL(): Notify alarm daemon" << endl;
    mAlarmDaemonIface.reloadCal("korgac",mURL.url());
    if (!mAlarmDaemonIface.ok()) {
      kdDebug() << "KOrganizer::saveUrl(): reloadCal call failed." << endl;
    }
  }

  // keep saves on a regular interval
  if (KOPrefs::instance()->mAutoSave) {
    mAutoSaveTimer->stop();
    mAutoSaveTimer->start(1000*60*KOPrefs::instance()->mAutoSaveInterval);
  }

  showStatusMessage(i18n("Saved calendar '%1'.").arg(mURL.prettyURL()));

  return true;
}

bool KOrganizer::saveAsURL(const KURL &url)
{
  kdDebug() << "KOrganizer::saveAsURL() " << url.prettyURL() << endl;

  if (url.isEmpty()) {
    kdDebug() << "KOrganizer::saveAsURL(): Empty URL." << endl;
    return false;
  }
  if (url.isMalformed()) {
    kdDebug() << "KOrganizer::saveAsURL(): Malformed URL." << endl;
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
    KGlobal::config()->setGroup("General");
    QString active = KGlobal::config()->readEntry("Active Calendar");
    if (KURL(active) == mURL) {
      setActive(true);
//      emit calendarActivated(this);
    } else {
      setActive(false);
    }
    setTitle();
    mRecent->addURL(mURL);
  } else {
    kdDebug() << "KOrganizer::saveAsURL() failed" << endl;
    mURL = URLOrig;
    mFile = fileOrig;
    delete tempFile;
  }

  return success;
}


bool KOrganizer::saveModifiedURL()
{
  kdDebug() << "KOrganizer::saveModifiedURL()" << endl;

  // If calendar isn't modified do nothing.
  if (!mCalendarView->isModified()) return true;

  if (KOPrefs::instance()->mAutoSave && !mURL.isEmpty()) {
    // Save automatically, when auto save is enabled.  
    return saveURL();
  } else {
    int result = KMessageBox::warningYesNoCancel(this,
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
        return false;
    }
  }
}


KURL KOrganizer::getSaveURL()
{
  KURL url = KFileDialog::getSaveURL(locateLocal("appdata", ""),
                                     i18n("*.vcs *.ics|Calendar files"),this);

  if (url.isEmpty()) return url;

  QString filename = url.fileName(false); 

  QString e = filename.right(4);
  if (e != ".vcs" && e != ".ics") {
    // Default save format is iCalendar
    filename += ".ics";
#if 0
    if (KOPrefs::instance()->mDefaultFormat == KOPrefs::FormatVCalendar) {
      filename += ".vcs";
    } else if (KOPrefs::instance()->mDefaultFormat == KOPrefs::FormatICalendar) {
      filename += ".ics";
    }
#endif
  }

  url.setFileName(filename);

  kdDebug() << "KOrganizer::getSaveURL(): url: " << url.url() << endl;

  return url;
}


bool KOrganizer::queryClose()
{
  if (windowList->lastInstance() && !isActive() && !mURL.isEmpty()) {
    int result = KMessageBox::questionYesNo(this,i18n("Do you want to make this"
      " calendar active?\nThis means that it is monitored for alarms and loaded"
      " as default calendar."));
    if (result == KMessageBox::Yes) makeActive();
  }

  bool success = saveModifiedURL();

  // Write configuration. I don't know if it really makes sense doing it this
  // way, when having opened multiple calendars in different CalendarViews.
  writeSettings();
  
  return success;
}

bool KOrganizer::queryExit()
{
  // Don't call writeSettings here, because filename isn't valid anymore. It is
  // now called in queryClose.
//  writeSettings();
  return true;
}


void KOrganizer::saveProperties(KConfig *config)
{
  config->writeEntry("Calendar",mURL.url());
}

void KOrganizer::readProperties(KConfig *config)
{
  QString calendarUrl = config->readEntry("Calendar");
  if (!calendarUrl.isEmpty()) {
    KURL u(calendarUrl);
    openURL(u);

    KGlobal::config()->setGroup("General");
    QString active = KGlobal::config()->readEntry("Active Calendar");
    if (active == calendarUrl) setActive(true);
  }
}


void KOrganizer::setTitle()
{
//  kdDebug() << "KOrganizer::setTitle" << endl;

  QString tmpStr;

  if (!mURL.isEmpty()) {
    if (mURL.isLocalFile()) tmpStr = mURL.fileName();
    else tmpStr = mURL.prettyURL();
  }
  else tmpStr = i18n("New Calendar");

  if (mCalendarView->isReadOnly())
    tmpStr += " [" + i18n("read-only") + "]";

  if (isActive()) tmpStr += " [" + i18n("active") + "]";

  setCaption(tmpStr,!mCalendarView->isReadOnly()&&mCalendarView->isModified());
}

void KOrganizer::checkAutoSave()
{
  kdDebug() << "KOrganizer::checkAutoSave()" << endl;

  // Don't save if auto save interval is zero
  if (KOPrefs::instance()->mAutoSaveInterval == 0) return;

  // has this calendar been saved before? If yes automatically save it.
  if (KOPrefs::instance()->mAutoSave && !mURL.isEmpty()) {
    saveURL();
  }
}


// Configuration changed as a result of the options dialog.
void KOrganizer::updateConfig()
{
  kdDebug() << "KOrganizer::updateConfig()" << endl;

  if (KOPrefs::instance()->mAutoSave && !mAutoSaveTimer->isActive()) {
    checkAutoSave();
    if (KOPrefs::instance()->mAutoSaveInterval > 0) {
      mAutoSaveTimer->start(1000*60*KOPrefs::instance()->mAutoSaveInterval);
    }
  }
  if (!KOPrefs::instance()->mAutoSave) mAutoSaveTimer->stop();
  mNextXDays->setText(i18n("&Next %1 Days").arg(KOPrefs::instance()->mNextXDays));
}

void KOrganizer::configureDateTime()
{
  KProcess *proc = new KProcess;
  *proc << "kcmshell" << "language";

  connect(proc,SIGNAL(processExited(KProcess *)),
          SLOT(configureDateTimeFinished(KProcess *)));

  if (!proc->start()) {
    KMessageBox::sorry(this,
        i18n("Couldn't start control module for date and time format."));
  }
}

void KOrganizer::configureToolbars()
{
  saveMainWindowSettings( KGlobal::config(), "MainWindow" );

  KEditToolbar dlg(factory());
  connect(&dlg,SIGNAL(newToolbarConfig()),this,SLOT(slotNewToolbarConfig()));

  dlg.exec();
}

void KOrganizer::slotNewToolbarConfig() // This is called when OK or Apply is clicked
{
  plugActionList("toolbartoggles",mToolBarToggles);

  applyMainWindowSettings( KGlobal::config(), "MainWindow" );
}

void KOrganizer::editKeys()
{
  KKeyDialog::configureKeys(actionCollection(),xmlFile(),true,this);
}

void KOrganizer::showTip()
{
  KTipDialog::showTip(this,QString::null,true);
}

void KOrganizer::showTipOnStart()
{
  KTipDialog::showTip(this);
}

KOrganizer* KOrganizer::findInstance(const KURL &url)
{
  if (windowList)
    return windowList->findInstance(url);
  else
    return 0;
}

void KOrganizer::setActive(bool active)
{
  if (active == mActive) return;
  
  mActive = active;
  setTitle();
}

void KOrganizer::makeActive()
{
  if (mURL.isEmpty()) {
    KMessageBox::sorry(this,i18n("The calendar does not have a filename. "
                                 "Please save it before activating."));
    return;
  }
  
  if (!mURL.isLocalFile()) {
    int result = KMessageBox::warningContinueCancel(this,
      i18n("Your calendar is a remote file. Activating it can cause "
           "synchronization problems leading to data loss.\n"
           "Make sure that it is accessed by no more than one single "
           "KOrganizer instance at the same time."),
      i18n("Activating Calendar."),i18n("Activate Calendar"),"dontaskActivate",
      true);
    if (result == KMessageBox::Cancel) return;
  }

  writeActiveState();

  mAlarmDaemonIface.reloadCal( "korgac", mURL.url() );
  if ( !mAlarmDaemonIface.ok() ) {
    kdDebug() << "KOrganizer::makeActive(): dcop send failed" << endl;
  }
  setActive();
  emit calendarActivated(this);
}

void KOrganizer::writeActiveState()
{
  KConfig *config(kapp->config());
  config->setGroup("General");
  config->writeEntry("Active Calendar",mURL.url());
  config->sync();
}

void KOrganizer::dumpText(const QString &str)
{
  kdDebug() << "KOrganizer::dumpText(): " << str << endl;
}

void KOrganizer::toggleToolBars(bool toggle)
{
  KToolBar *bar = toolBar(sender()->name());
  if (bar) {
    if (toggle) bar->show();
    else bar->hide();
  } else {
    kdDebug() << "KOrganizer::toggleToolBars(): Toolbar not found" << endl;
  }
}

void KOrganizer::toggleToolBar()
{
  QPtrListIterator<KToolBar> it = toolBarIterator();
  for ( ; it.current() ; ++it ) {
    if (mToolBarToggleAction->isChecked()) (*it)->show();
    else (*it)->hide();
  }
}

void KOrganizer::toggleStatusBar()
{
  bool show_statusbar = mStatusBarAction->isChecked();
  if (show_statusbar)
     statusBar()->show();
  else
     statusBar()->hide();
}

void KOrganizer::toggleFilterView()
{
  bool visible = mFilterViewAction->isChecked();
  mCalendarView->showFilter(visible);
}

void KOrganizer::statusBarPressed(int id)
{
  if (id == ID_MESSAGES_IN)
    mCalendarView->dialogManager()->showIncomingDialog();
  else if (id == ID_MESSAGES_OUT)
    mCalendarView->dialogManager()->showOutgoingDialog();
}

void KOrganizer::setNumIncoming(int num)
{
  statusBar()->changeItem(i18n(" Incoming messages: %1 ").arg(num),
                          ID_MESSAGES_IN);
}

void KOrganizer::setNumOutgoing(int num)
{
  statusBar()->changeItem(i18n(" Outgoing messages: %1 ").arg(num),
                          ID_MESSAGES_OUT);
}

void KOrganizer::showStatusMessage(const QString &message)
{
  statusBar()->message(message,2000);
}

bool KOrganizer::openURL(QString url)
{
  return openURL(KURL(url));
}

bool KOrganizer::mergeURL(QString url)
{
  return openURL(KURL(url),true);
}

bool KOrganizer::saveAsURL(QString url)
{
  return saveAsURL(KURL(url));
}

QString KOrganizer::getCurrentURLasString() const
{
  return mURL.url();
}

bool KOrganizer::deleteEvent(QString uid)
{
  return mCalendarView->deleteEvent(uid);
}

void KOrganizer::configureDateTimeFinished(KProcess *proc)
{
  delete proc;
}

void KOrganizer::processIncidenceSelection( Incidence *incidence )
{
//  kdDebug() << "KOrganizer::processIncidenceSelection()" << endl;
  
  if ( !incidence ) {
    enableIncidenceActions( false );
    return;
  }
  
  enableIncidenceActions( true );
  
  if ( incidence->type() == "Event" ) {
    mShowIncidenceAction->setText( i18n("&Show Event...") );
    mEditIncidenceAction->setText( i18n("&Edit Event...") );
    mDeleteIncidenceAction->setText( i18n("&Delete Event...") );
    mPublishEvent->setEnabled(true);
  } else if ( incidence->type() == "Todo" ) {
    mShowIncidenceAction->setText( i18n("&Show To-Do...") );
    mEditIncidenceAction->setText( i18n("&Edit To-Do...") );
    mDeleteIncidenceAction->setText( i18n("&Delete To-Do...") );
    mPublishEvent->setEnabled(false);
  } else {
    mShowIncidenceAction->setText( i18n("&Show...") );
    mShowIncidenceAction->setText( i18n("&Edit...") );
    mShowIncidenceAction->setText( i18n("&Delete...") );
    mPublishEvent->setEnabled(false);
  }
}

void KOrganizer::enableIncidenceActions( bool enabled )
{
  mShowIncidenceAction->setEnabled( enabled );
  mEditIncidenceAction->setEnabled( enabled );
  mDeleteIncidenceAction->setEnabled( enabled );

  mCutAction->setEnabled( enabled );
  mCopyAction->setEnabled( enabled );
  mDeleteAction->setEnabled( enabled );
}

void KOrganizer::downloadNewStuff()
{
  kdDebug() << "KOrganizer::downloadNewStuff()" << endl;

  if ( !mNewStuff ) mNewStuff = new KONewStuff( mCalendarView );
  mNewStuff->download();
}

void KOrganizer::uploadNewStuff()
{
  if ( !mNewStuff ) mNewStuff = new KONewStuff( mCalendarView );
  mNewStuff->upload();
}

QString KOrganizer::localFileName()
{
  return mFile;
}
