/*
  $Id$
  
  Requires the Qt and KDE widget libraries, available at no cost at
  http://www.troll.no and http://www.kde.org respectively
  
  Copyright (c) 1997, 1998, 1999, 2000
  Preston Brown (preston.brown@yale.edu)
  Fester Zigterman (F.J.F.ZigtermanRustenburg@student.utwente.nl)
  Ian Dawes (iadawes@globalserve.net)
  Laszlo Boloni (boloni@cs.purdue.edu)
  Cornelius Schumacher (schumacher@kde.org)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdlib.h>

#include <qcursor.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qfile.h>

#include <kglobal.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kstddirs.h>
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

#include "komailclient.h"
#include "calprinter.h"
#include "exportwebdialog.h"
#include "calendarview.h"
#include "kowindowlist.h"
#include "koprefs.h"

#include "korganizer.h"
#include "korganizer.moc"


KOWindowList *KOrganizer::windowList = 0;

KOrganizer::KOrganizer(const char *name)
  : KMainWindow(0,name), DCOPObject("KOrganizerIface")
{
  kdDebug() << "KOrganizer::KOrganizer()" << endl;

  mTempFile = 0;
  mActive = false;

  // add this instance of the window to the static list.
  if (!windowList) windowList = new KOWindowList;
  windowList->addWindow(this);

//  setMinimumSize(600,400);	// make sure we don't get resized too small...

  mCalendarView = new CalendarView(this,"KOrganizer::CalendarView");
  setCentralWidget(mCalendarView);

  initActions();

  statusBar()->insertItem("",ID_GENERAL,10);

//  statusBar()->insertFixedItem(i18n("Active"),ID_ACTIVE);

  if (KOPrefs::instance()->mEnableGroupScheduling) {
    statusBar()->insertItem(i18n("Incoming Messages: %1").arg(0),
                            ID_MESSAGES_IN);
    statusBar()->insertItem(i18n("Outgoing Messages: %2").arg(0),
                            ID_MESSAGES_OUT);
    statusBar()->setItemAlignment(ID_MESSAGES_IN,AlignRight);
    statusBar()->setItemAlignment(ID_MESSAGES_OUT,AlignRight);
  }
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

  // Update state of event instance related actions
  mCalendarView->emitEventsSelected();

  // Update state of paste action
  mCalendarView->checkClipboard();
  
  kdDebug() << "KOrganizer::KOrganizer() done" << endl;
}

KOrganizer::~KOrganizer()
{
  kdDebug() << "~KOrganizer()" << endl;

  if (mTempFile) delete mTempFile;

  // Take this window out of the window list.
  windowList->removeWindow(this);

  kdDebug() << "~KOrganizer() done" << endl;
}


void KOrganizer::readSettings()
{
  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig *config(kapp->config());

  config->setGroup("General");

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

  KConfig *config(kapp->config());

  QString tmpStr;
  config->setGroup("General");

  config->writeEntry("Width",width());
  config->writeEntry("Height",height());

  mRecent->saveEntries(config);

  mCalendarView->writeSettings();

  config->sync();
}


void KOrganizer::initActions()
{
  KAction *action;

  KStdAction::openNew(this, SLOT(file_new()), actionCollection());
  KStdAction::open(this, SLOT(file_open()), actionCollection());
  mRecent = KStdAction::openRecent(this, SLOT(file_openRecent(const KURL&)),
                                   actionCollection());
  KStdAction::save(this, SLOT(file_save()), actionCollection());
  KStdAction::saveAs(this, SLOT(file_saveas()), actionCollection());
  KStdAction::close(this, SLOT(file_close()), actionCollection());
  (void)new KAction(i18n("&Import From Ical"), 0, this, SLOT(file_import()),
                    actionCollection(), "import_ical");
  (void)new KAction(i18n("&Merge Calendar..."), 0, this, SLOT(file_merge()),
                    actionCollection(), "merge_calendar");
  (void)new KAction(i18n("Archive Old Entries..."), 0, this, SLOT(file_archive()),
                    actionCollection(), "file_archive");

  (void)new KAction(i18n("Web Page..."), 0,
                    mCalendarView, SLOT(exportWeb()),
                    actionCollection(), "export_web");
  (void)new KAction(i18n("iCalendar..."), 0,
                    mCalendarView, SLOT(exportICalendar()),
                    actionCollection(), "export_icalendar");
  (void)new KAction(i18n("vCalendar..."), 0,
                    mCalendarView, SLOT(exportVCalendar()),
                    actionCollection(), "export_vcalendar");

  (void)new KAction(i18n("Print Setup..."), 0,
                    mCalendarView, SLOT(printSetup()),
                    actionCollection(), "print_setup");
  KStdAction::print(mCalendarView, SLOT(print()), actionCollection());
  KStdAction::printPreview(mCalendarView, SLOT(printPreview()),
                           actionCollection());
  (void)new KAction(i18n("Make active"),0,this,SLOT(makeActive()),
                    actionCollection(),"make_active");
  KStdAction::quit(this, SLOT(close()), actionCollection());

  // setup edit menu
  action = KStdAction::cut(mCalendarView,SLOT(edit_cut()),actionCollection());
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = KStdAction::copy(mCalendarView,SLOT(edit_copy()),actionCollection());
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = KStdAction::paste(mCalendarView,SLOT(edit_paste()),
                             actionCollection());
  connect(mCalendarView,SIGNAL(pasteEnabled(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("&Delete"),"editdelete",0,
                    mCalendarView,SLOT(appointment_delete()),
                    actionCollection(), "edit_delete");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));

  // view menu
  (void)new KAction(i18n("What's &Next"), 0,
                    mCalendarView, SLOT(showWhatsNextView()),
                    actionCollection(), "view_whatsnext");
  (void)new KAction(i18n("&List"), "list", 0,
                    mCalendarView, SLOT(showListView()),
                    actionCollection(), "view_list");
  (void)new KAction(i18n("&Day"), "1day", 0,
                    mCalendarView, SLOT(showDayView()),
                    actionCollection(), "view_day");
  (void)new KAction(i18n("W&ork Week"), "5days", 0,
                    mCalendarView, SLOT(showWorkWeekView()),
                    actionCollection(), "view_workweek");
  (void)new KAction(i18n("&Week"), "7days", 0,
                    mCalendarView, SLOT(showWeekView()),
                    actionCollection(), "view_week");
  (void)new KAction(i18n("&Month"), "month", 0,
                    mCalendarView, SLOT(showMonthView()),
                    actionCollection(), "view_month");
  (void)new KAction(i18n("&To-do list"), "todo", 0,
                    mCalendarView, SLOT(showTodoView()),
                    actionCollection(), "view_todo");
// Unconditionally enable project view
//  if (KOPrefs::instance()->mEnableProjectView) {
    (void)new KAction(i18n("&Project"), 0,
                      mCalendarView, SLOT(showProjectView()),
                      actionCollection(), "view_project");
//  }
  (void)new KAction(i18n("&Journal"), 0,
                    mCalendarView, SLOT(showJournalView()),
                    actionCollection(), "view_journal");
  (void)new KAction(i18n("&Update"), 0,
                    mCalendarView, SLOT(update()),
                    actionCollection(), "update");

  // event handling menu
  (void)new KAction(i18n("New E&vent..."), "appointment", 0,
                    mCalendarView,SLOT(appointment_new()),
                    actionCollection(), "new_event");
  (void)new KAction(i18n("New &To-Do..."), 0,
                    mCalendarView,SLOT(newTodo()),
                    actionCollection(), "new_todo");
  action = new KAction(i18n("&Show Appointment..."), 0,
                    mCalendarView,SLOT(appointment_show()),
                    actionCollection(), "show_appointment");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("&Edit Appointment..."), 0,
                    mCalendarView,SLOT(appointment_edit()),
                    actionCollection(), "edit_appointment");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("T&ake over Event"), 0,
                       mCalendarView,SLOT(takeOverEvent()),
                       actionCollection(), "takeover_event");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  (void)new KAction(i18n("T&ake over Calendar"), 0,
                    mCalendarView,SLOT(takeOverCalendar()),
                    actionCollection(), "takeover_calendar");
  

  KStdAction::find(mCalendarView, SLOT(action_search()), actionCollection());

  action = new KAction(i18n("&Mail Appointment"), "mail_generic", 0,
                    mCalendarView,SLOT(action_mail()),
                    actionCollection(), "mail_appointment");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));

  // Schedule menu
  (void)new KAction(i18n("Outgoing Messages..."),0,
                    mCalendarView,SLOT(schedule_outgoing()),
                    actionCollection(),"outgoing");
  (void)new KAction(i18n("Incoming Messages..."),0,
                    mCalendarView,SLOT(schedule_incoming()),
                    actionCollection(),"incoming");
  action = new KAction(i18n("Publish"),0,
                       mCalendarView,SLOT(schedule_publish()),
                       actionCollection(),"publish");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("Request"),0,
                       mCalendarView,SLOT(schedule_request()),
                       actionCollection(),"request");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("Refresh"),0,
                       mCalendarView,SLOT(schedule_refresh()),
                       actionCollection(),"refresh");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("Cancel"),0,
                       mCalendarView,SLOT(schedule_cancel()),
                       actionCollection(),"cancel");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("Add"),0,
                       mCalendarView,SLOT(schedule_add()),
                       actionCollection(),"add");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("Reply"),0,
                       mCalendarView,SLOT(schedule_reply()),
                       actionCollection(),"reply");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("Counter"),0,
                       mCalendarView,SLOT(schedule_counter()),
                       actionCollection(),"counter");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));
  action = new KAction(i18n("Decline Counter"),0,
                       mCalendarView,SLOT(schedule_declinecounter()),
                       actionCollection(),"declinecounter");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));

  // Navigation menu  
  (void)new KAction(i18n("Go to &Today"), "today", 0,
                    mCalendarView,SLOT(goToday()),
                    actionCollection(), "go_today");
  action = new KAction(i18n("Go &Backward"), "1leftarrow", 0,
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
  action = new KAction(i18n("Go &Forward"), "1rightarrow", 0,
                       mCalendarView,SLOT(goNext()),
                       actionCollection(), "go_next");
/*
  connect(mCalendarView,SIGNAL(changeNavStringNext(const QString &)),
          action,SLOT(setText(const QString &)));
*/
      
  // setup Settings menu
//  mToolBarToggleAction = KStdAction::showToolbar(this,SLOT(toggleToolBar()),
//                                                 actionCollection());
//  KStdAction::showStatusbar(this, SLOT(toggleStatusBar()), actionCollection());


  (void)new KAction(i18n("Configure &Date & Time..."), 0,
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
  KStdAction::saveOptions(this, SLOT(saveOptions()), actionCollection());
  KStdAction::keyBindings(this, SLOT(editKeys()), actionCollection());

  (void)new KAction(i18n("Edit Categories"), 0,
                    mCalendarView,SLOT(editCategories()),
                    actionCollection(),"edit_categories");
  (void)new KAction(i18n("Edit Filters"), 0,
                    mCalendarView,SLOT(editFilters()),
                    actionCollection(),"edit_filters");

#if 0
  (void)new KAction(i18n("Show Intro Page"), 0,
                    mCalendarView,SLOT(showIntro()),
                    actionCollection(),"show_intro");
#endif
  
  if (KOPrefs::instance()->mEnableGroupScheduling) {
    createGUI("korganizergsui.rc");
  } else {  
    createGUI();
  }

  KConfig *config = kapp->config();

  applyMainWindowSettings(config);

  config->setGroup("Settings");
  mFilterViewAction->setChecked(config->readBoolEntry("Filter Visible",false));
  toggleFilterView();

  mStatusBarAction->setChecked(!statusBar()->isHidden());

  QListIterator<KToolBar> it = toolBarIterator();
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
  int retVal;
  QString progPath;
  KTempFile tmpfn;

  QString homeDir = QDir::homeDirPath() + QString::fromLatin1("/.calendar");
		  
  if (!QFile::exists(homeDir)) {
    KMessageBox::error(this,
		       i18n("You have no ical file in your home directory.\n"
		            "Import cannot proceed.\n"));
    return;
  }
  progPath = locate("exe", "ical2vcal") + " " + tmpfn.name();

  retVal = system(QFile::encodeName(progPath));
  
  if (retVal >= 0 && retVal <= 2) {
    // now we need to MERGE what is in the iCal to the current calendar.
    mCalendarView->openCalendar(tmpfn.name(),1);
    if (!retVal)
      KMessageBox::information(this,
			       i18n("KOrganizer succesfully imported and "
				    "merged your\n.calendar file from ical "
				    "into the currently\nopened calendar.\n"));
    else
      KMessageBox::information(this,
			   i18n("KOrganizer encountered some unknown fields while\n"
				"parsing your .calendar ical file, and had to\n"
				"discard them.  Please check to see that all\n"
				"your relevant data was correctly imported.\n"),
                                 i18n("ICal Import Successful With Warning"));
  } else if (retVal == -1) {
    KMessageBox::error(this,
			 i18n("KOrganizer encountered some error parsing your\n"
			      ".calendar file from ical.  Import has failed.\n"));
  } else if (retVal == -2) {
    KMessageBox::error(this,
			 i18n("KOrganizer doesn't think that your .calendar\n"
			      "file is a valid ical calendar. Import has failed.\n"));
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
        mRecent->addURL(url);
        showStatusMessage(i18n("Opened calendar '%1'.").arg(mURL.prettyURL()));
      }
    }


    return success;
  } else {
    QString msg;
    msg = i18n("Cannot download calendar from %1.").arg(mURL.prettyURL());
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
  if (!mCalendarView->saveCalendar(mFile)) {
    kdDebug() << "KOrganizer::saveURL(): calendar view save failed." << endl;
    return false;
  }

  if (isActive()) {
    kdDebug() << "KOrganizer::saveURL(): Notify alarm daemon" << endl;
    if (!kapp->dcopClient()->send("alarmd","ad","reloadCal()","")) {
      kdDebug() << "KOrganizer::saveURL(): dcop send failed" << endl;
    }
  }

  if (!mURL.isLocalFile()) {
    if (!KIO::NetAccess::upload(mFile,mURL)) {
      QString msg = i18n("Cannot upload calendar to %1").arg(mURL.prettyURL());
      KMessageBox::error(this,msg);
      return false;
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
    mRecent->addURL(mURL);
    mURL = url;
    delete mTempFile;
    mTempFile = tempFile;
    KIO::NetAccess::removeTempFile(fileOrig);    
    KGlobal::config()->setGroup("General");
    QString active = KGlobal::config()->readEntry("Active Calendar");
    if (KURL(active) == mURL) setActive(true);
    else setActive(false);
    setTitle();
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
        i18n("The calendar has been modified.\nDo you want to save it?"));
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
    if (KOPrefs::instance()->mDefaultFormat == KOPrefs::FormatVCalendar) {
      filename += ".vcs";
    } else if (KOPrefs::instance()->mDefaultFormat == KOPrefs::FormatICalendar) {
      filename += ".ics";
    }
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
}

void KOrganizer::configureDateTime()
{
  KProcess *proc = new KProcess;
  *proc << "kcmshell" << "Personalization/language";

  connect(proc,SIGNAL(processExited(KProcess *)),
          SLOT(configureDateTimeFinished(KProcess *)));

  if (!proc->start()) {
    KMessageBox::sorry(this,
        i18n("Couldn't start control module for date and time format"));
  }
}

void KOrganizer::configureToolbars()
{
  KEditToolbar dlg(actionCollection());

  if (dlg.exec()) {
    createGUI();
    plugActionList("toolbartoggles",mToolBarToggles);
  }
}

void KOrganizer::editKeys()
{
  KKeyDialog::configureKeys(actionCollection(),xmlFile(),true,this);
}

void KOrganizer::saveOptions()
{
  KConfig *config = kapp->config();

  saveMainWindowSettings(config);

  config->setGroup("Settings");
  config->writeEntry("Filter Visible",mFilterViewAction->isChecked());
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

  KConfig *config(kapp->config());
  config->setGroup("General");
  config->writeEntry("Active Calendar",mURL.url());
  config->sync();
  if (!kapp->dcopClient()->send("alarmd","ad","reloadCal()","")) {
    kdDebug() << "KOrganizer::saveURL(): dcop send failed" << endl;
  }
  setActive();
  emit calendarActivated(this);
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
  QListIterator<KToolBar> it = toolBarIterator();
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
  if (id == ID_MESSAGES_IN) mCalendarView->schedule_incoming();
  else if (id == ID_MESSAGES_OUT) mCalendarView->schedule_outgoing();
}

void KOrganizer::setNumIncoming(int num)
{
  statusBar()->changeItem(i18n("Incoming Messages: %1").arg(num),
                          ID_MESSAGES_IN);
}

void KOrganizer::setNumOutgoing(int num)
{
  statusBar()->changeItem(i18n("Outgoing Messages: %1").arg(num),
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

bool KOrganizer::deleteEvent(QString VUID)
{
  return mCalendarView->deleteEvent(VUID);
}

void KOrganizer::configureDateTimeFinished(KProcess *proc)
{
  delete proc;
}
