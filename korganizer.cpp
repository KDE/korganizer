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

#include <sys/types.h>
#include <signal.h>

#include <qfiledlg.h>
#include <qcursor.h>
#include <qmlined.h>
#include <qmsgbox.h>
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

#include "misc.h"
#include "version.h"
#include "koarchivedlg.h"
#include "komailclient.h"
#include "calprinter.h"
#include "aboutdlg.h"
#include "exportwebdialog.h"
#include "calendarview.h"

#include "korganizer.h"
#include "korganizer.moc"

#define AGENDABUTTON 0x10
#define NOACCEL 0

QList<KOrganizer> KOrganizer::windowList;

KOrganizer::KOrganizer(QString filename, bool fnOverride, const char *name ) 
  : KTMainWindow( name )
{
  qDebug("KOrganizer::KOrganizer()");

  mAutoSave = false;

  // add this instance of the window to the static list.
  windowList.append(this);

  toolBarEnable = statusBarEnable = true;
//  setMinimumSize(600,400);	// make sure we don't get resized too small...

  if (!fnOverride) {
    KConfig *config(kapp->config());
    config->setGroup("General");
    QString str = config->readEntry("Current Calendar (2.0)");
    if (!str.isEmpty() && QFile::exists(str))
      mFilename = str;
  } else {
    mFilename = filename;
  }

  mCalendarView = new CalendarView(mFilename,this,"KOrganizer::CalendarView");
  setView(mCalendarView);

  // setup toolbar, menubar and status bar, NOTE: this must be done
  // after the widget creation, because setting the menubar, toolbar
  // or statusbar will generate a call to updateRects, which assumes
  // that all of them are around.

//  initMenus();
//  initToolBar();

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
  connect(mAutoSaveTimer, SIGNAL(timeout()),
	  this, SLOT(checkAutoSave()));
  if (autoSave())
    mAutoSaveTimer->start(1000*60);

  setTitle();

  qDebug("KOrganizer::KOrganizer() done");
}

KOrganizer::~KOrganizer()
{
  qDebug("~KOrganizer()");
  hide();

  // Free memory allocated for widgets (not children)
  // Take this window out of the window list.
  windowList.removeRef( this );
  qDebug("~KOrganizer() done");
}


void KOrganizer::readSettings()
{
  QString str;

  // read settings from the KConfig, supplying reasonable
  // defaults where none are to be found

  KConfig *config(kapp->config());

  int windowWidth = 600;
  int windowHeight = 400;
	
  config->setGroup("General");

  str = config->readEntry("Width");
  if (!str.isEmpty())
    windowWidth = str.toInt();
  str = config->readEntry("Height");
  if (!str.isEmpty())
    windowHeight = str.toInt();
  this->resize(windowWidth, windowHeight);

// We currently don't use a status bar
#if 0
  statusBarEnable = config->readBoolEntry("Status Bar", TRUE);
#endif

//  toolBarEnable = config->readBoolEntry("Tool Bar", TRUE);

  mAutoSave = config->readBoolEntry("Auto Save", FALSE);

  mCalendarView->readSettings();
    
  config->sync();
}


void KOrganizer::writeSettings()
{
  KConfig *config(kapp->config());

  QString tmpStr;
  config->setGroup("General");

  tmpStr.sprintf("%d", this->width() );
  config->writeEntry("Width",	tmpStr);
	
  tmpStr.sprintf("%d", this->height() );
  config->writeEntry("Height",	tmpStr);

/*
  tmpStr.sprintf("%s", optionsMenu->isItemChecked(toolBarMenuId) ? 
		 "true" : "false");
  config->writeEntry("Tool Bar", tmpStr);
*/

// We currently don't use a status bar
#if 0
  tmpStr.sprintf("%s", optionsMenu->isItemChecked(statusBarMenuId) ?
		 "true" : "false");
  config->writeEntry("Status Bar", tmpStr);
#endif

  // Write version number to prevent automatic loading and saving of a calendar
  // written by a newer KOrganizer, because this can lead to the loss of
  // information not processed by Korganizer 1.1.
  config->writeEntry("Current Calendar (2.0)", mFilename);

  mCalendarView->writeSettings();

  config->sync();
}


void KOrganizer::initActions()
{
  KStdAction::openNew(this, SLOT(file_new()), actionCollection());
  KStdAction::open(this, SLOT(file_open()), actionCollection());
  KStdAction::openRecent(this, SLOT(file_openRecent(int)), actionCollection());
  KStdAction::save(this, SLOT(file_save()), actionCollection());
  KStdAction::saveAs(this, SLOT(file_saveas()), actionCollection());
  KStdAction::close(this, SLOT(file_close()), actionCollection());
  (void)new KAction(i18n("&Import From Ical"), 0, this, SLOT(file_import()),
                    actionCollection(), "import_ical");
  (void)new KAction(i18n("&Merge Calendar"), 0, this, SLOT(file_merge()),
                    actionCollection(), "merge_calendar");
  (void)new KAction(i18n("Archive Old Entries"), 0, this, SLOT(file_archive()),
                    actionCollection(), "file_archive");
  (void)new KAction(i18n("Export as web page"), 0,
                    mCalendarView, SLOT(exportWeb()),
                    actionCollection(), "export_web");
  (void)new KAction(i18n("Print Setup"), 0,
                    mCalendarView, SLOT(printSetup()),
                    actionCollection(), "print_setup");
  KStdAction::print(mCalendarView, SLOT(print()), actionCollection());
  KStdAction::printPreview(mCalendarView, SLOT(printPreview()),
                           actionCollection());
  KStdAction::quit(this, SLOT(close()), actionCollection());

  // setup edit menu
  KStdAction::cut(mCalendarView, SLOT(edit_cut()), actionCollection());
  KStdAction::copy(mCalendarView, SLOT(edit_copy()), actionCollection());
  KStdAction::paste(mCalendarView, SLOT(edit_paste()), actionCollection());

  // view menu
  (void)new KAction(i18n("&List"), BarIcon("listicon"), 0,
                    mCalendarView, SLOT(view_list()),
                    actionCollection(), "view_list");
  (void)new KAction(i18n("&Day"), BarIcon("dayicon"), 0,
                    mCalendarView, SLOT(view_day()),
                    actionCollection(), "view_day");
  (void)new KAction(i18n("W&ork Week"), BarIcon("5dayicon"), 0,
                    mCalendarView, SLOT(view_workweek()),
                    actionCollection(), "view_workweek");
  (void)new KAction(i18n("&Week"), BarIcon("weekicon"), 0,
                    mCalendarView, SLOT(view_week()),
                    actionCollection(), "view_week");
  (void)new KAction(i18n("&Month"), BarIcon("monthicon"), 0,
                    mCalendarView, SLOT(view_month()),
                    actionCollection(), "view_month");
  (void)new KAction(i18n("&To-do list"), BarIcon("todolist"), 0,
                    mCalendarView, SLOT(view_todolist()),
                    actionCollection(), "view_todo");
  (void)new KAction(i18n("&Update"), 0,
                    mCalendarView, SLOT(update()),
                    actionCollection(), "update");

  // event handling menu
  (void)new KAction(i18n("New &Appointment"), BarIcon("newevent"), 0,
                    mCalendarView,SLOT(appointment_new()),
                    actionCollection(), "new_appointment");
  (void)new KAction(i18n("New E&vent"), 0,
                    mCalendarView,SLOT(allday_new()),
                    actionCollection(), "new_event");
  (void)new KAction(i18n("New To-Do"), 0,
                    mCalendarView,SLOT(newTodo()),
                    actionCollection(), "new_todo");
  (void)new KAction(i18n("&Edit Appointment"), 0,
                    mCalendarView,SLOT(appointment_edit()),
                    actionCollection(), "new_appointment");
  (void)new KAction(i18n("&Delete Appointment"), 0,
                    mCalendarView,SLOT(appointment_delete()),
                    actionCollection(), "delete_appointment");

  KStdAction::find(mCalendarView, SLOT(action_search()), actionCollection());

  (void)new KAction(i18n("&Mail Appointment"), BarIcon("send"), 0,
                    mCalendarView,SLOT(action_mail()),
                    actionCollection(), "mail_appointment");
  
  (void)new KAction(i18n("Go to &Today"), BarIcon("todayicon"), 0,
                    mCalendarView,SLOT(goToday()),
                    actionCollection(), "go_today");
  (void)new KAction(i18n("&Previous Day"), BarIcon("1leftarrow"), 0,
                    mCalendarView,SLOT(goPrevious()),
                    actionCollection(), "go_previous");
  (void)new KAction(i18n("&Next Day"), BarIcon("1rightarrow"), 0,
                    mCalendarView,SLOT(goNext()),
                    actionCollection(), "go_next");
      
  // setup Settings menu
  KStdAction::showToolbar(this, SLOT(toggleToolBar()), actionCollection());
//  KStdAction::showStatusbar(this, SLOT(toggleStatusBar()), actionCollection());

  KStdAction::configureToolbars(this, SLOT(configureToolbars()),
                                actionCollection());
  KStdAction::preferences(mCalendarView, SLOT(edit_options()),
                          actionCollection());
  
  createGUI("korganizer.rc");
}


void KOrganizer::initMenus()
{
  QPixmap pixmap;
  //  int itemId;
  KStdAccel stdAccel;

  fileMenu = new QPopupMenu;
  pixmap = BarIcon("mini/korganizer");
  fileMenu->insertItem(pixmap,i18n("&New Window"), this,
		       SLOT(file_new()), stdAccel.openNew());
  fileMenu->insertSeparator();
  pixmap = BarIcon("fileopen");
  fileMenu->insertItem(pixmap, i18n("&Open"), this,
		       SLOT(file_open()), stdAccel.open());

  recentPop = new QPopupMenu;
  fileMenu->insertItem(i18n("Open &Recent"), recentPop);
  connect( recentPop, SIGNAL(activated(int)), SLOT(file_openRecent(int)));

  // setup the list of recently used files
  recentPop->clear();
  for (int i = 0; i < (int) recentFileList.count(); i++)
    recentPop->insertItem(recentFileList.at(i));
  add_recent_file(mFilename);

  fileMenu->insertItem(i18n("&Close"), this,
		       SLOT(file_close()), stdAccel.close());
  fileMenu->insertSeparator();

  pixmap = BarIcon("filefloppy");
  fileMenu->insertItem(pixmap, i18n("&Save"), this,
		       SLOT(file_save()), stdAccel.save());
  fileMenu->insertItem(i18n("Save &As"), this,
		       SLOT(file_saveas()));

  fileMenu->insertSeparator();

  fileMenu->insertItem(i18n("&Import From Ical"), this,
  		       SLOT(file_import()));
  fileMenu->insertItem(i18n("&Merge Calendar"), this,
		       SLOT(file_merge()));
  
  fileMenu->setItemEnabled(fileMenu->insertItem(i18n("Archive Old Entries"),
						this,
						SLOT(file_archive())),
			   FALSE);
  

  fileMenu->insertItem(i18n("Export as web page"), mCalendarView,
                       SLOT(exportWeb()));

  fileMenu->insertSeparator();
  fileMenu->insertItem(i18n("Print Setup"), mCalendarView,
		       SLOT(printSetup()));

  pixmap = BarIcon("fileprint");
  fileMenu->insertItem(pixmap, i18n("&Print"), mCalendarView,
		       SLOT(print()), stdAccel.print());
  fileMenu->insertItem(i18n("Print Pre&view"), mCalendarView,
		       SLOT(printPreview()));
  
  fileMenu->insertSeparator();
  fileMenu->insertItem(i18n("&Quit"), this,
		       SLOT(file_quit()), stdAccel.quit());

  editMenu = new QPopupMenu;
  //put stuff for editing here 
  pixmap = BarIcon("editcut");
  editMenu->insertItem(pixmap, i18n("C&ut"), mCalendarView,
		       SLOT(edit_cut()), stdAccel.cut());
  pixmap = BarIcon("editcopy");
  editMenu->insertItem(pixmap, i18n("&Copy"), mCalendarView,
		       SLOT(edit_copy()), stdAccel.copy());
  pixmap = BarIcon("editpaste");
  editMenu->insertItem(pixmap, i18n("&Paste"), mCalendarView,
		       SLOT(edit_paste()), stdAccel.paste());

  viewMenu = new QPopupMenu;

  pixmap = BarIcon("listicon");
  viewMenu->insertItem(pixmap, i18n("&List"), mCalendarView,
			SLOT( view_list() ) );

  pixmap = BarIcon("dayicon");
  viewMenu->insertItem(pixmap, i18n("&Day"), mCalendarView,
			SLOT( view_day() ) );
  pixmap = BarIcon("5dayicon");
  viewMenu->insertItem(pixmap, i18n("W&ork Week"), mCalendarView,
			SLOT( view_workweek() ) );
  pixmap = BarIcon("weekicon");
  viewMenu->insertItem(pixmap, i18n("&Week"), mCalendarView,
			SLOT( view_week() ) );
  pixmap = BarIcon("monthicon");
  viewMenu->insertItem(pixmap, i18n("&Month"), mCalendarView,
			SLOT( view_month() ) );
  pixmap = BarIcon("todolist");
  viewMenu->insertItem(pixmap,i18n("&To-do list"), mCalendarView,
			SLOT( view_todolist()), FALSE );
  viewMenu->insertSeparator();
  viewMenu->insertItem( i18n("&Update"), mCalendarView,
			SLOT( update() ) );
	
  actionMenu = new QPopupMenu;
  pixmap = BarIcon("newevent");
  actionMenu->insertItem(pixmap, i18n("New &Appointment"), 
			 mCalendarView, SLOT(apptmnt_new()));
  actionMenu->insertItem(i18n("New E&vent"),
			 mCalendarView, SLOT(allday_new()));
  actionMenu->insertItem(i18n("New To-do"),
			 mCalendarView, SLOT(newTodo()));
  actionMenu->insertItem(i18n("&Edit Appointment"), mCalendarView,
			  SLOT(apptmnt_edit()));
  pixmap = BarIcon("delete");
  actionMenu->insertItem(pixmap, i18n("&Delete Appointment"),
			 mCalendarView, SLOT(apptmnt_delete()));
  actionMenu->insertItem(i18n("Delete To-do"),
			 mCalendarView, SLOT(action_deleteTodo()));

  actionMenu->insertSeparator();

  pixmap = BarIcon("search");
  actionMenu->insertItem(pixmap,i18n("&Search"), mCalendarView,
			 SLOT(action_search()), stdAccel.find());

  pixmap = BarIcon("send");
  actionMenu->insertItem(pixmap, i18n("&Mail Appointment"), mCalendarView,
				  SLOT(action_mail()));

  actionMenu->insertSeparator();

  pixmap = BarIcon("todayicon");
  actionMenu->insertItem(pixmap,i18n("Go to &Today"), mCalendarView,
			 SLOT(goToday()));

  pixmap = BarIcon("1leftarrow");
  actionMenu->insertItem(pixmap, i18n("&Previous Day"), mCalendarView,
				  SLOT(goPrevious()));

  pixmap = BarIcon("1rightarrow");
  actionMenu->insertItem(pixmap, i18n("&Next Day"), mCalendarView,
				  SLOT(goNext()));

  optionsMenu = new QPopupMenu;
  toolBarMenuId = optionsMenu->insertItem(i18n("Show &Tool Bar"), this,
				   SLOT(toggleToolBar()));
  optionsMenu->setItemChecked(toolBarMenuId, TRUE);

// We currently don't use a status bar
#if 0
  statusBarMenuId = optionsMenu->insertItem(i18n("Show St&atus Bar"), this,
				   SLOT(toggleStatusBar()));
  optionsMenu->setItemChecked(statusBarMenuId, TRUE);
#endif

  optionsMenu->insertSeparator();			  
  optionsMenu->insertItem(i18n("&Edit Options"), 
			  mCalendarView, SLOT(edit_options()));

  helpMenu = new QPopupMenu;
  helpMenu->insertItem(i18n("&Contents"), mCalendarView,
		       SLOT(help_contents()),
		       stdAccel.help());
  helpMenu->insertSeparator();
  helpMenu->insertItem(i18n("&About"), mCalendarView,
		       SLOT(help_about())); 
  helpMenu->insertItem(i18n("Send &Bug Report"), mCalendarView,
		       SLOT(help_postcard()));
    
  // construct a basic menu
  menubar = new KMenuBar(this, "menubar_0");
  menubar->insertItem(i18n("&File"), fileMenu);
  menubar->insertItem(i18n("&Edit"), editMenu);
  menubar->insertItem(i18n("&View"), viewMenu);
  menubar->insertItem(i18n("&Actions"), actionMenu);
  menubar->insertItem(i18n("&Options"), optionsMenu);
  menubar->insertSeparator();
  menubar->insertItem(i18n("&Help"), helpMenu);

  setMenu(menubar);
}

void KOrganizer::initToolBar()
{
  QPixmap pixmap;
  QString dirName;

  tb = new KToolBar(this);

  pixmap = BarIcon("fileopen");
  tb->insertButton(pixmap, 0,
		   SIGNAL(clicked()), this,
		   SLOT(file_open()), TRUE, 
		   i18n("Open A Calendar"));
	
  pixmap = BarIcon("filefloppy");
  tb->insertButton(pixmap, 0,
		   SIGNAL(clicked()), this,
		   SLOT(file_save()), TRUE, 
		   i18n("Save mCalendarView Calendar"));

  pixmap = BarIcon("fileprint");
  tb->insertButton(pixmap, 0,
		   SIGNAL(clicked()), mCalendarView,
		   SLOT(print()), TRUE, 
		   i18n("Print"));

  tb->insertSeparator();
//  QFrame *sepFrame = new QFrame(tb);
//  sepFrame->setFrameStyle(QFrame::VLine|QFrame::Raised);
//  tb->insertWidget(0, 10, sepFrame);

  pixmap = BarIcon("newevent");
  tb->insertButton(pixmap, 0,
		   SIGNAL(clicked()), mCalendarView,
		   SLOT(apptmnt_new()), TRUE, 
		   i18n("New Appointment"));
  pixmap = BarIcon("delete");

  tb->insertButton(pixmap, 0,
		   SIGNAL(clicked()), mCalendarView,
		   SLOT(apptmnt_delete()), TRUE,
		   i18n("Delete Appointment"));

  pixmap = BarIcon("findf");
  tb->insertButton(pixmap, 0,
		   SIGNAL(clicked()), mCalendarView,
		   SLOT(action_search()), TRUE,
		   i18n("Search For an Appointment"));

  pixmap = BarIcon("send");
  tb->insertButton(pixmap, 0,
		   SIGNAL(clicked()), mCalendarView,
		   SLOT(action_mail()), TRUE,
		   i18n("Mail Appointment"));

  tb->insertSeparator();
//  sepFrame = new QFrame(tb);
//  sepFrame->setFrameStyle(QFrame::VLine|QFrame::Raised);
//  tb->insertWidget(0, 10, sepFrame);

//  KPTButton *bt = new KPTButton(tb);
//  bt->setText(i18n("Go to Today"));
//  bt->setPixmap(Icon("todayicon"));
//  connect(bt, SIGNAL(clicked()), SLOT(goToday()));
//  tb->insertWidget(0, bt->sizeHint().width(), bt);
//
// ! replaced the "Go to Today" button with an icon

  pixmap = BarIcon("todayicon");
  tb->insertButton(pixmap, 0,
      SIGNAL(clicked()), mCalendarView,
      SLOT(goToday()), TRUE,
      i18n("Go to Today"));

  pixmap = BarIcon("1leftarrow");
  tb->insertButton(pixmap, 0,
      SIGNAL(clicked()),
      mCalendarView, SLOT(goPrevious()), TRUE,
      i18n("Previous Day"));

  pixmap = BarIcon("1rightarrow");
  tb->insertButton(pixmap, 0,
      SIGNAL(clicked()),
      mCalendarView, SLOT(goNext()), TRUE,
      i18n("Next Day"));

  tb->insertSeparator();
//  sepFrame = new QFrame(tb);
//  sepFrame->setFrameStyle(QFrame::VLine|QFrame::Raised);
//  tb->insertWidget(0, 10, sepFrame);

  QPopupMenu *agendaViewPopup = new QPopupMenu();
  agendaViewPopup->insertItem( BarIcon("dayicon"), "Show one day",
                               KOAgendaView::DAY );
  agendaViewPopup->insertItem( BarIcon("5dayicon"), "Show a work week",
                               KOAgendaView::WORKWEEK );
  agendaViewPopup->insertItem( BarIcon("weekicon"), "Show a week",
                               KOAgendaView::WEEK );
  connect( agendaViewPopup, SIGNAL( activated(int) ), mCalendarView , SLOT( changeAgendaView(int) ) );

  pixmap = BarIcon("listicon");
  tb->insertButton(pixmap, 0, SIGNAL(clicked()), mCalendarView,
		   SLOT(view_list()), TRUE,
		   i18n("List View"));

  pixmap = BarIcon("agenda");
  tb->insertButton(pixmap, AGENDABUTTON, SIGNAL(clicked()),
		   mCalendarView, SLOT( nextAgendaView()), TRUE,
		   i18n("Schedule View"));
  tb->setDelayedPopup( AGENDABUTTON, agendaViewPopup );
  
  pixmap = BarIcon("monthicon");
  tb->insertButton(pixmap, 0,
		   SIGNAL(clicked()), mCalendarView,
		   SLOT(view_month()), TRUE,
		   i18n("Month View"));
  
  pixmap = BarIcon("todolist");
  tb->insertButton(pixmap, 0,
		   SIGNAL(clicked()), mCalendarView,
		   SLOT(view_todolist()), TRUE,
		   i18n("To-do list view"));

  addToolBar(tb);
}

void KOrganizer::file_new()
{
  // Make new KOrganizer window containing empty calendar
  (new KOrganizer("",true))->show();
}

void KOrganizer::file_open()
{
  int whattodo = 0; // the same as button numbers from QMessageBox return

  if (mCalendarView->isModified() &&
      (mFilename.isEmpty() || autoSave())) {
    whattodo = mCalendarView->msgCalModified();
  } else if (mCalendarView->isModified()) {
    whattodo = 0; // save if for sure
  } else {
    whattodo = 1; // go ahead and just open a new one
  }

  switch (whattodo) {
  case 0: // Save
    if (file_save()) // bail on error
      return;
  
  case 1: { // Open
    QString newFileName = file_getname(0);
    
    if (newFileName == "")
      return;
    
    QApplication::setOverrideCursor(waitCursor);

    // child windows no longer valid
    emit closingDown();
    
    if (mCalendarView->setFile(newFileName)) {
      mFilename = newFileName;
      add_recent_file(newFileName);
      setTitle();
    }
    
    QApplication::restoreOverrideCursor();
    break;
  } // case 1
  } // switch
}

void KOrganizer::file_openRecent(int i)
{
  int  whattodo = 0; // the same as button numbers from QMessageBox return
  
  if (mCalendarView->isModified() && (mFilename.isEmpty() || !autoSave())) {
    whattodo = mCalendarView->msgCalModified();
  } else if (mCalendarView->isModified()) {
    whattodo = 0; // save if for sure
  } else {
    whattodo = 1; // go ahead and just open a new one
  }

  switch (whattodo) {
    case 0: // save ("Yes")
      if (file_save())
        return;
    
    case 1: { // open ("No")
      QString newFileName = recentFileList.at(i);

      // this should never happen
      ASSERT(newFileName != "");

      QApplication::setOverrideCursor(waitCursor);
      // child windows no longer valid
      emit closingDown();
    
      if(mCalendarView->setFile(newFileName)) {
        mFilename = newFileName;
        add_recent_file(newFileName);
      }

      QApplication::restoreOverrideCursor();
      break;
    } // case 1
  } // switch
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
    QMessageBox::critical(this, i18n("KOrganizer Error"),
			  i18n("You have no ical file in your home directory.\n"
			       "Import cannot proceed.\n"));
    return;
  }
  tmpFn = tmpnam(0);
  progPath = locate("exe", "ical2vcal") + tmpFn;

  retVal = system(progPath.data());
  
  if (retVal >= 0 && retVal <= 2) {
    // now we need to MERGE what is in the iCal to the current calendar.
    mCalendarView->mergeFile(tmpFn);
    if (!retVal)
      QMessageBox::information(this, i18n("KOrganizer Info"),
			       i18n("KOrganizer succesfully imported and "
				    "merged your\n.calendar file from ical "
				    "into the currently\nopened calendar.\n"),
			       QMessageBox::Ok);
    else
      QMessageBox::warning(this, i18n("ICal Import Successful With Warning"),
			   i18n("KOrganizer encountered some unknown fields while\n"
				"parsing your .calendar ical file, and had to\n"
				"discard them.  Please check to see that all\n"
				"your relevant data was correctly imported.\n"));
  } else if (retVal == -1) {
    QMessageBox::warning(this, i18n("KOrganizer Error"),
			 i18n("KOrganizer encountered some error parsing your\n"
			      ".calendar file from ical.  Import has failed.\n"));
  } else if (retVal == -2) {
    QMessageBox::warning(this, i18n("KOrganizer Error"),
			 i18n("KOrganizer doesn't think that your .calendar\n"
			      "file is a valid ical calendar. Import has failed.\n"));
  }
}

void KOrganizer::file_merge()
{
  QString mergeFileName;

  mergeFileName = file_getname(0);

  // If file dialog box was cancelled (trap for null) 
  if(mergeFileName.isEmpty())
    return;

  if(mCalendarView->mergeFile(mergeFileName)) {
  }
}

void KOrganizer::file_archive()
{
  ArchiveDialog ad;

  if (ad.exec()) {
    // ok was pressed.
  }
}

int KOrganizer::file_saveas()
{
  QString newFileName = file_getname(1);

  if (newFileName == "")
    return 1;

  if (mCalendarView->saveCalendar(newFileName))
    return 1;

  mFilename = newFileName;
  add_recent_file(newFileName);

  setTitle();

  // keep saves on a regular interval
  if (autoSave()) {
    mAutoSaveTimer->stop();
    mAutoSaveTimer->start(1000*60);
  }

  return 0;
}

int KOrganizer::file_save()
{
  // has this calendar been saved before?
  if (mFilename.isEmpty())
    return file_saveas();

  if (mCalendarView->saveCalendar(mFilename)) {
    setTitle();
    return 1;
  }

  // keep saves on a regular interval
  if (autoSave()) {
    mAutoSaveTimer->stop();
    mAutoSaveTimer->start(1000*60);
  }

  return 0;
}


void KOrganizer::file_close()
{
  mCalendarView->closeCalendar();
  mFilename = "";

  setTitle();
}


void KOrganizer::file_quit()
{
  close();
/*
  // Close all open windows. Make sure that this widget is closed as last.
  KOrganizer *tw;
  for(tw = windowList.first(); tw; tw = windowList.next()) {
    if (tw != this) tw->close();
  }
  close();
*/
}


bool KOrganizer::queryClose()
{
  // Write configuration. I don't know if it really makes sense doing it this
  // way, when having opened multiple calendars in different CalendarViews.
  writeSettings();

  int whattodo = 0; // the same as button numbers from QMessageBox return

  if (mCalendarView->isModified() && (mFilename.isEmpty() || !autoSave())) {
    whattodo = mCalendarView->msgCalModified();
  } else if (mCalendarView->isModified()) {
    whattodo = 0; // save if for sure
  } else {
    whattodo = 1; // go ahead and just open a new one
  }

  switch (whattodo) {
    case 0: // save ("Yes")
      if (file_save()) {
        return true;
      } else {
        qDebug("CalendarView::queryClose(): file_save() failed");
        return false;
      }
      break;

    case 1: // open ("No", or wasn't modified)
      // child windows no longer valid
      emit closingDown();
      mCalendarView->setFile("");
      mFilename = "";
      return true;
      break;

    case 2: // cancel
      return false;

  } // switch

  return false;
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
  QString tmpStr;

  if (!mFilename.isEmpty())
    tmpStr = mFilename.mid(mFilename.findRev('/')+1, mFilename.length());
  else
    tmpStr = i18n("New Calendar");

  // display the modified thing in the title
  // if auto-save is on, we only display it on new calender (no file name)
  if (mCalendarView->isModified() && (mFilename.isEmpty() || !autoSave())) {
    tmpStr += " (";
    tmpStr += i18n("modified");
    tmpStr += ")";
  }

  setCaption(tmpStr);
}

void KOrganizer::checkAutoSave()
{
  // has this calendar been saved before? 
  if (autoSave() && !mFilename.isEmpty()) {
    add_recent_file(mFilename);
    mCalendarView->saveCalendar(mFilename);
  }
}


// Configuration changed as a result of the options dialog.
// I wanted to break this up, in order to avoid inefficiency 
// introduced as we were ALWAYS updating configuration
// in multiple widgets regardless of changed in information.
void KOrganizer::updateConfig()
{
  emit configChanged();
  readSettings(); // this is the best way to assure that we have them back
  if (autoSave() && !mAutoSaveTimer->isActive()) {
    checkAutoSave();
    mAutoSaveTimer->start(1000*60);
  }
  if (!autoSave())
    mAutoSaveTimer->stop();

  // static slot calls here
  KOEvent::updateConfig();
}


void KOrganizer::add_recent_file(QString recentFileName)
{
#if 0
  KListAction *recent;
  recent = (KListAction*)actionCollection()->action(KStdAction::stdName(KStdAction::OpenRecent));
  recent->setItems( recent_files );

  const char *rf;
  QFileInfo tf(recentFileName);

  // if it is not a file or is not readable bail.
  if (!tf.isFile() || !tf.isReadable())
    return;

  // sanity check
  if (recentFileName.isEmpty())
    return;

  // check to see if it is already in the list.
  for (rf = recentFileList.first(); rf;
       rf = recentFileList.next()) {
    if (strcmp(rf, recentFileName.data()) == 0)
      return;  
  }

  if( recentFileList.count() < 5)
    recentFileList.insert(0,recentFileName.data());
  else {
    recentFileList.remove(4);
    recentFileList.insert(0,recentFileName.data());
  }
  if (recentPop) {
    recentPop->clear();
    for ( int i =0 ; i < (int)recentFileList.count(); i++)
      {
	recentPop->insertItem(recentFileList.at(i));
      }
    if (recentFileList.count() == 0) {
      // disable the "Open Recent" option on the file menu if necessary
      fileMenu->setItemEnabled(1, FALSE);
    }
  }
#endif
}


QString KOrganizer::file_getname(int open_save)
{
  QString    fileName;
  QString    defaultPath;

  // Be nice and tidy and have all the files in a nice easy to find place.
  defaultPath = locateLocal("appdata", "");
  
  switch (open_save) {
  case 0 :
    // KFileDialog works?
    fileName = QFileDialog::getOpenFileName(defaultPath, "*.vcs", this);
    //fileName = KFileDialog::getOpenFileName(defaultPath, "*.vcs", this);
    break;
  case 1 :
    // KFileDialog works?
    fileName = QFileDialog::getSaveFileName(defaultPath, "*.vcs", this);
    //fileName = KFileDialog::getSaveFileName(defaultPath, "*.vcs", this);
    break;
  default :
    debug("Internal error: CalendarView::file_getname(): invalid paramater");
    return "";
  } // switch

  // If file dialog box was cancelled or blank file name
  if (fileName.isEmpty()) {
    if(!fileName.isNull())
      QMessageBox::warning(this, i18n("KOrganizer Error"), 
			   i18n("You did not specify a valid file name."));
    return "";
  }
  
  QFileInfo tf(fileName);

  if(tf.isDir()) {
    QMessageBox::warning(this, i18n("KOrganizer Error"),
			 i18n("The file you specified is a directory,\n"
			      "and cannot be opened. Please try again,\n"
			      "this time selecting a valid calendar file."));
    return "";
  }

  if (open_save == 1) {
    // Force the extension for consistency.
    if(fileName.length() >= 3) {
      QString e = fileName.right(4);
      // Extension ending in '.vcs' or anything else '.???' is cool.
      if(e != ".vcs" && e.right(1) != ".")
      // Otherwise, force the default extension.
      fileName += ".vcs";
    }
  }

  return fileName;
}

void KOrganizer::configureToolbars()
{
  KEditToolbar dlg(actionCollection());

  if (dlg.exec())
  {
    createGUI("korganizer.rc");
  }
}

