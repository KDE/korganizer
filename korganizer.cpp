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

#include <qcursor.h>
#include <qtimer.h>
#include <qvbox.h>

#include <kglobal.h>
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
  : KMainWindow(0,name)
{
  qDebug("KOrganizer::KOrganizer()");

  mTempFile = 0;
  mActive = false;

  // add this instance of the window to the static list.
  if (!windowList) windowList = new KOWindowList;
  windowList->addWindow(this);

//  setMinimumSize(600,400);	// make sure we don't get resized too small...

  mCalendarView = new CalendarView(this,"KOrganizer::CalendarView");
  setCentralWidget(mCalendarView);

  initActions();

// We don't use a status bar up to now.
#if 0
  sb = new KStatusBar(this, "sb");
  setStatusBar(sb);
#endif

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

  // Update state of event instance related actions
  mCalendarView->emitEventsSelected();

  // Update state of paste action
  mCalendarView->checkClipboard();
  
  qDebug("KOrganizer::KOrganizer() done");
}

KOrganizer::~KOrganizer()
{
  qDebug("~KOrganizer()");

  if (mTempFile) delete mTempFile;

  // Take this window out of the window list.
  windowList->removeWindow(this);

  qDebug("~KOrganizer() done");
}


void KOrganizer::readSettings()
{
  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig *config(kapp->config());

  int windowWidth = 600;
  int windowHeight = 400;
	
  config->setGroup("General");

  windowWidth = config->readNumEntry("Width");
  windowHeight = config->readNumEntry("Height");
  resize(windowWidth,windowHeight);

  mRecent->loadEntries(config);

  mCalendarView->readSettings();
    
  config->sync();
}


void KOrganizer::writeSettings()
{
  qDebug("KOrganizer::writeSettings");

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
  (void)new KAction(i18n("Export as web page..."), 0,
                    mCalendarView, SLOT(exportWeb()),
                    actionCollection(), "export_web");
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

  // view menu
  (void)new KAction(i18n("&List"), "list", 0,
                    mCalendarView, SLOT(view_list()),
                    actionCollection(), "view_list");
  (void)new KAction(i18n("&Day"), "1day", 0,
                    mCalendarView, SLOT(view_day()),
                    actionCollection(), "view_day");
  (void)new KAction(i18n("W&ork Week"), "5days", 0,
                    mCalendarView, SLOT(view_workweek()),
                    actionCollection(), "view_workweek");
  (void)new KAction(i18n("&Week"), "7days", 0,
                    mCalendarView, SLOT(view_week()),
                    actionCollection(), "view_week");
  (void)new KAction(i18n("&Month"), "month", 0,
                    mCalendarView, SLOT(view_month()),
                    actionCollection(), "view_month");
  (void)new KAction(i18n("&To-do list"), "todo", 0,
                    mCalendarView, SLOT(view_todolist()),
                    actionCollection(), "view_todo");
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
  action = new KAction(i18n("&Delete Appointment"), 0,
                    mCalendarView,SLOT(appointment_delete()),
                    actionCollection(), "delete_appointment");
  connect(mCalendarView,SIGNAL(eventsSelected(bool)),
          action,SLOT(setEnabled(bool)));

  KStdAction::find(mCalendarView, SLOT(action_search()), actionCollection());

  action = new KAction(i18n("&Mail Appointment"), "send", 0,
                    mCalendarView,SLOT(action_mail()),
                    actionCollection(), "mail_appointment");
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
  mToolBarToggleAction = KStdAction::showToolbar(this,SLOT(toggleToolBar()),
                                                 actionCollection());
//  KStdAction::showStatusbar(this, SLOT(toggleStatusBar()), actionCollection());

/*
  (void)new KAction(i18n("Configure &Date & Time..."), 0,
                    this,SLOT(configureDateTime()),
                    actionCollection(), "conf_datetime");
*/

  KStdAction::configureToolbars(this, SLOT(configureToolbars()),
                                actionCollection());
  KStdAction::preferences(mCalendarView, SLOT(edit_options()),
                          actionCollection());
  
  createGUI();

// Disabled because of the message freeze. Enabling it would add new messages.
#if 0
  QListIterator<KToolBar> it = toolBarIterator();
  for ( ; it.current() ; ++it ) {
    KToggleAction *act = new KToggleAction(i18n("Show %1 Toolbar")
                                           .arg((*it)->text()),0,
                                           actionCollection(),(*it)->name());
    connect( act,SIGNAL(toggled(bool)),SLOT(toggleToolBars(bool)));
    act->setChecked(true);
    mToolBarToggles.append(act);
  }
  plugActionList("toolbartoggles",mToolBarToggles);
#endif
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
  url = KFileDialog::getOpenURL(defaultPath,"*.vcs",this);

  if (openURL(url)) {
    setTitle();
    mRecent->addURL(url);
    setActive(false);
  }
}


void KOrganizer::file_openRecent(const KURL& url)
{
  if (!url.isEmpty()) {
    if (openURL(url)) {
      setTitle();
    }
  }
}


void KOrganizer::file_import()
{
  // eventually, we will need a dialog box to select import type, etc.
  // for now, hard-coded to ical file, $HOME/.calendar.
  int retVal;
  QString progPath;
  char *tmpFn;

  QString homeDir;
  homeDir.sprintf("%s/.calendar",getenv("HOME"));
		  
  if (!QFile::exists(homeDir)) {
    KMessageBox::error(this,
		       i18n("You have no ical file in your home directory.\n"
		            "Import cannot proceed.\n"));
    return;
  }
  tmpFn = tmpnam(0);
  progPath = locate("exe", "ical2vcal") + tmpFn;

  retVal = system(progPath.data());
  
  if (retVal >= 0 && retVal <= 2) {
    // now we need to MERGE what is in the iCal to the current calendar.
    mCalendarView->mergeCalendar(tmpFn);
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
}


void KOrganizer::file_merge()
{
  KURL url = KFileDialog::getOpenURL(locateLocal("appdata", ""),"*.vcs",this);
  mergeURL(url);
}


void KOrganizer::file_archive()
{
  mCalendarView->archiveCalendar();
}


void KOrganizer::file_saveas()
{
  KURL url = getSaveURL();

  if (saveAsURL(url)) {
    setTitle();
  }
}


KURL KOrganizer::getSaveURL()
{
  KURL url = KFileDialog::getSaveURL(locateLocal("appdata", ""),"*.vcs",this);

  QString filename = url.fileName(false); 

  if(filename.length() >= 3) {
    QString e = filename.right(4);
    // Extension ending in '.vcs' or anything else '.xxx' is cool.
    if(e != ".vcs" && e.right(1) != ".")
    // Otherwise, force the default extension.
    filename += ".vcs";
  }

  url.setFileName(filename);

  qDebug("KOrganizer::getSaveURL(): url: %s",url.url().latin1());

  return url;
}


void KOrganizer::file_save()
{
  if (mURL.isEmpty()) file_saveas();
  else saveURL();
}


void KOrganizer::file_close()
{
  closeURL();
  mLastFile = "";
  setTitle();
}


void KOrganizer::file_quit()
{
  close();
}


bool KOrganizer::queryClose()
{
  if (windowList->lastInstance() && !mActive && !mURL.isEmpty()) {
    int result = KMessageBox::questionYesNo(this,i18n("Do you want to make this"
      " calendar active?\nThis means that it is monitored for alarms and loaded"
      " as default calendar."));
    if (result == KMessageBox::Yes) makeActive();
  }

  bool success = closeURL();

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


void KOrganizer::setTitle()
{
//  qDebug("KOrganizer::setTitle");

  QString tmpStr;

  if (!mURL.isEmpty()) tmpStr = mURL.fileName();
  else tmpStr = i18n("New Calendar");

  if (mCalendarView->isReadOnly()) {
    tmpStr += " (" + i18n("read-only") + ")";
  } else {
    // display the modified thing in the title
    if (mCalendarView->isModified()) {
      tmpStr += " (" + i18n("modified") + ")";
    }
  }

  if (mActive) tmpStr += " [" + i18n("active") + "]";

  setCaption(tmpStr);
}

void KOrganizer::checkAutoSave()
{
  qDebug("KOrganizer::checkAutoSave()");

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
  qDebug("KOrganizer::updateConfig()");

  if (KOPrefs::instance()->mAutoSave && !mAutoSaveTimer->isActive()) {
    checkAutoSave();
    if (KOPrefs::instance()->mAutoSaveInterval > 0) {
      mAutoSaveTimer->start(1000*60*KOPrefs::instance()->mAutoSaveInterval);
    }
  }
  if (!KOPrefs::instance()->mAutoSave) mAutoSaveTimer->stop();
}

/*
void KOrganizer::configureDateTime()
{
  KProcess proc;
  proc << "xeyes";
//  proc << "kcmshell" << "Personalization/language";
  if (!proc.start()) {
    KMessageBox::sorry(this,
        i18n("Couldn't start control module for date and time format"));
  }
}
*/

void KOrganizer::configureToolbars()
{
  KEditToolbar dlg(actionCollection());

  if (dlg.exec())
  {
    createGUI();
  }
}


bool KOrganizer::openURL( const KURL &url )
{
  qDebug("KOrganizer::openURL()");
  if (url.isMalformed()) return false;
  if (!closeURL()) return false;
  mURL = url;
  mFile = "";
  if(KIO::NetAccess::download(mURL,mFile)) {
    return mCalendarView->openCalendar(mFile);
  } else {
    QString msg;
    msg = i18n("Cannot download calendar from %1").arg(mURL.prettyURL());
    KMessageBox::error(this,msg);
    return false;
  }
}


bool KOrganizer::mergeURL( const KURL &url )
{
  qDebug("KOrganizer::mergeURL()");
  if (url.isMalformed()) return false;

  QString tmpFile;
  if( KIO::NetAccess::download(url,tmpFile)) {
    bool success = mCalendarView->mergeCalendar(tmpFile);
    KIO::NetAccess::removeTempFile(tmpFile);
    return success;
  } else {
    return false;
  }
}


bool KOrganizer::closeURL()
{
  qDebug("KOrganizer::closeURL()");

  if (KOPrefs::instance()->mAutoSave && !mURL.isEmpty()) {
    // Save automatically, when auto save is enabled.  
    if (!saveURL()) return false;
  } else {
    if (mCalendarView->isModified()) {
      int result = KMessageBox::warningYesNoCancel(0L,
            i18n("The calendar has been modified.\nDo you want to save it?"));

      switch(result) {
        case KMessageBox::Yes :
          if (mURL.isEmpty()) {
            KURL url = getSaveURL();
            if (url.isEmpty()) return false;
            if (!saveAsURL(url)) return false;
          }
          if (!saveURL()) return false;
          break;
        case KMessageBox::No :
          break;
        default : // case KMessageBox::Cancel :
          return false;
      }
    }
  }

  mCalendarView->closeCalendar();
  if (mURL.isLocalFile()) mLastFile = mFile;
  mURL="";
  mFile="";
  
  KIO::NetAccess::removeTempFile( mFile );

  return true;
}

bool KOrganizer::saveURL()
{
  if (!mCalendarView->saveCalendar(mFile)) {
    qDebug("KOrganizer::saveURL(): calendar view save failed.");
    return false;
  }
  
  if (mActive) {
    qDebug("KOrganizer::saveURL(): Notify alarm daemon");
    if (!kapp->dcopClient()->send("alarmd","ad","reloadCal()","")) {
      qDebug("KOrganizer::saveURL(): dcop send failed");
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

  return true;
}

bool KOrganizer::saveAsURL( const KURL & kurl )
{
  if (kurl.isMalformed()) {
    qDebug("KOrganizer::saveAsURL(): Malformed URL.");
    return false;
  }
  mURL = kurl; // Store where to upload

  // Local file
  if ( mURL.isLocalFile() ) {
    // get rid of a possible temp file first
    if ( mTempFile ) {  // (happens if previous url was remote)
      delete mTempFile;
      mTempFile = 0;
    }
    mFile = mURL.path();
  } else { // Remote file
    // We haven't saved yet, or we did but locally - provide a temp file
    if ( mFile.isEmpty() || !mTempFile ) {
      mTempFile = new KTempFile;
      mFile = mTempFile->name();
    }
    // otherwise, we already had a temp file
  }
  bool success = saveURL(); // Save local file and upload local file
  qDebug("KOrganizer::saveAsURL() %s",mURL.prettyURL().latin1());
  if (success) mRecent->addURL(mURL);
  else qDebug("  failed");
  return success;
}

void KOrganizer::setActive(bool active)
{
  if (active == mActive) return;
  
  mActive = active;
  setTitle();
}

void KOrganizer::makeActive()
{
  // Write only local Files to config file. This prevents loading of a remote
  // file automatically on startup, which could block KOrganizer even before
  // it has opened. Perhaps this is to strict...
  if (mURL.isEmpty()) {
    KMessageBox::sorry(this,i18n("The calendar does not have a filename. "
                                 "Please save it before activating."));
  } else if (mURL.isLocalFile()) {
    KConfig *config(kapp->config());
    config->writeEntry("Active Calendar",mFile);
    config->sync();
    if (!kapp->dcopClient()->send("alarmd","ad","reloadCal()","")) {
      qDebug("KOrganizer::saveURL(): dcop send failed");
    }
    setActive();
    emit calendarActivated(this);
  } else {
    KMessageBox::sorry(this,i18n("Only local files can be active calendars."));
  }
}

void KOrganizer::dumpText(const QString &str)
{
  qDebug("KOrganizer::dumpText(): %s",str.latin1());
}

void KOrganizer::toggleToolBars(bool toggle)
{
  KToolBar *bar = toolBar(sender()->name());
  if (bar) {
    if (toggle) bar->show();
    else bar->hide();
  } else {
    qDebug("KOrganizer::toggleToolBars(): Toolbar not found");
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

void KOrganizer::saveProperties(KConfig *config)
{
  qDebug("KOrganizer::saveProperties()");
  config->writeEntry("Calendar",mLastFile);
  if (mURL.isLocalFile()) {
    config->writeEntry("Calendar",mFile);
  }
}

void KOrganizer::readProperties(KConfig *config)
{
  QString calendarUrl = config->readEntry("Calendar");
  if (!calendarUrl.isEmpty()) {
    KURL u;
    u.setPath(calendarUrl);
    openURL(u);
  }
}
