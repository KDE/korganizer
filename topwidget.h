/* 	$Id$	 */

#ifndef _TOPWIDGET_H
#define _TOPWIDGET_H

#include <qframe.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qlist.h>
#include <qtabdlg.h>
#include <qframe.h>
#include <qsplitter.h>

#include <ktoolbar.h>
#include <ktmainwindow.h>
#include <kapp.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kmenubar.h>

#include "qdatelist.h"
#include "calobject.h"
#include "kdatenav.h"
#include "eventwin.h"
#include "editeventwin.h"
#include "todoeventwin.h"
#include "koagendaview.h"
//#include "listview.h"
#include "kolistview.h"
#include "kotodoview.h"
#include "komonthview.h"
#include "optionsdlg.h"
#include "searchdialog.h"

class CalPrinter;

/**
 *
 * This is the main class for KOrganizer. It extends the KDE KTMainWindow.
 * it provides the main view that the user sees upon startup, as well as
 * menus, buttons, etc. etc.
 *
 * @short constructs a new main window for korganizer
 * @author Preston Brown
 * @version $Revision$
 */
class TopWidget : public KTMainWindow
{
  Q_OBJECT
public:
  /**
   *
   * Constructs a new main window.
   *
   * @param cal is a newly allocated calendar passed from the main program
   * @param parent this should usually be 0, unless it is a child win (when?)
   * @param name this is the name of the window to display in the titlebar
   * @param fnOverride specifies whether if the filename given is empty,
   * KOrg. will try to obtain a filename from the config file instead of 
   * starting up with an empty calendar.
   *
   */
  TopWidget(CalObject *cal, QString fn="", 
	    const char *name=0, bool fnOverride = TRUE);
  virtual ~TopWidget(void);

  /** supplied so that close events call file_close()/file_close() properly.*/
  bool queryClose();

  // public variables
  static QList<TopWidget> windowList;

  // View Types in enum
  enum { AGENDAVIEW, LISTVIEW, MONTHVIEW, TODOVIEW };
  enum { EVENTADDED, EVENTEDITED, EVENTDELETED };

signals:

  /** when change is made to options dialog, the topwidget will catch this
   *  and emit this signal which notifies all widgets which have registered 
   *  for notification to update their settings. */  
  void configChanged();
  /** emitted when the topwidget is closing down, so that any attached
      child windows can also close. */
  void closingDown();
  /** emitted right before we die */
  void closed(QWidget *);

public slots:
  /** used for creating a new main window after the first main window has
   * already been created. */
  void newTopWidget();

  /** options dialog made a changed to the configuration. we catch this
   *  and notify all widgets which need to update their configuration. */
  void updateConfig();


  /** create an editeventwin with supplied date/time, and if bool is true,
   * make the event take all day. */
  void newEvent(QDateTime, QDateTime);
  void newEvent(QDateTime, QDateTime, bool allDay);
  void newEvent(QDateTime fh) { newEvent(fh,QDateTime(fh.addSecs(3600))); };
  void newEvent(QDate dt) { newEvent(QDateTime(dt, QTime(0,0,0)),
	   		    QDateTime(dt, QTime(0,0,0)), TRUE); };
  /** create new event without having a date hint. Takes current date as
   default hint. */ 
  void newEvent();
  /** create an editeventwin for the supplied event */
  void editEvent(KOEvent *);
  /** delete the supplied event */
  void deleteEvent(KOEvent *);

  /** create new todo */
  void newTodo();  
  /** create new todo with a parent todo */
  void newSubTodo(KOEvent *);
  
  //void eventsSelected(QList<KOEvent>);

  
  /** change Agenda view */
  void changeAgendaView( int view );

  /** next Agenda view */
  void nextAgendaView();


 protected slots:
  /** slot that sets up a single shot timer to call the dateNavigator 
   * and update the current date when midnight comes around (if we are
   * running, of course)
   */
  void setupRollover();
 
  /** using the KConfig associated with the kapp variable, read in the
   * settings from the config file. 
   */
  void readSettings();

  /** write current state to config file. */
  void writeSettings();

  /** passes on the message that an event has changed to the currently
   * activated view so that it can make appropriate display changes. */
  void changeEventDisplay(KOEvent *, int);

  /** changes the view to be the currently selected view */
  void changeView(KOBaseView *);
  
  void updateView(const QDateList);
  void updateView();

  /** open a file, load it into the calendar. */
  void file_open();

  /** open a file from the list of recent files. */
  void file_openRecent(int i);

  /** import a calendar from another program like ical. */
  void file_import();

  /** open a calendar and add the contents to the current calendar. */
  void file_merge();

  /** delete or archive old entries in your calendar for speed/space. */
  void file_archive();

  /** close a file, prompt for save if changes made. */
  void file_close();

  /** save a file with the current fileName. returns nonzero on error. */
  int file_save();

  /** save a file under a (possibly) different filename. Returns nonzero
   * on error. */
  int file_saveas();

  /** exit the program, prompt for save if files are "dirty". */
  void file_quit();

  /** cut the current appointment to the clipboard */
  void edit_cut();

  /** copy the current appointment(s) to the clipboard */
  void edit_copy();

  /** paste the current vobject(s) in the clipboard buffer into calendar */
  void edit_paste();

  /** edit viewing and configuration options. */
  void edit_options();

  /**
   *functions for printing, previewing a print, and setting up printing
   * parameters 
   */
  void print();
  void printSetup();
  void printPreview();

  /**
   *
   * pop up an Appointment Dialog to make a new appointment. Uses date that
   * is currently selected in the dateNavigator.
   */
  void apptmnt_new();
  /** same as apptmnt_new, but sets "All Day Event" to true by default. */
  void allday_new();
  /**
   *
   * pop up an Appointment Dialog to edit an existing appointment.	Get
   * information on the appointment from the list of unique IDs that is
   * currently in the View, called currIds.
   */
  void apptmnt_edit();
  /**
   *
   * pop up dialog confirming deletion of currently selected event in the
   * View.
   */
  void apptmnt_delete();

  /**
   *
   * deletes the currently selected todo item, if one is selected.
   */
  void action_deleteTodo();
  /**
   * allows the user to enter a search RegExp and a date range, and then
   * displays / returns a list of appointments that fit the criteria.
   */
  void action_search();

  /** mails the currently selected event to a particular user as a vCalendar 
    attachment. */
  void action_mail();

  void help_contents();
  void help_about();
  void help_postcard();
  /** query whether or not the calendar is "dirty". */
  bool isModified();
  /** set the state of calendar to be "dirty", i.e. needing a save. */
  void setModified();
  /** unset the dirty bit on the current calendar. */
  void unsetModified();

  // figures out what the title should be, and sets it accordingly
  void set_title();

  void view_list();
  void view_day();
  void view_workweek();
  void view_week();
  void view_month();
  void view_todolist();

protected slots:
  // clean up after a child window closes
  void cleanWindow(QWidget *);

  virtual void updateRects();

  /** Move the current view date to today */
  void goToday();

  /** Move to the next date(s) in the current view */
  void goNext();

  /** Move to the previous date(s) in the current view */
  void goPrevious();
  
  /** called by the autoSaveTimer to automatically save the calendar */
  void checkAutoSave();
  /** toggle the appearance of the menuBar. */
  void toggleToolBar() 
    { 
      tb->enable(KToolBar::Toggle);
      updateRects();
      optionsMenu->setItemChecked(toolBarMenuId,
				  !optionsMenu->isItemChecked(toolBarMenuId));
    };
  /** toggle the appearance of the statusBar. */
  void toggleStatusBar() 
    { 
      sb->enable(KStatusBar::Toggle);
      updateRects();
      optionsMenu->setItemChecked(statusBarMenuId, 
				  !optionsMenu->isItemChecked(statusBarMenuId));
    };

protected:
  void initMenus();
  void initToolBar();
  void hookupSignals();
  void initCalendar(QString fn, bool fnOverride);

  // show a standard warning
  // returns KMsgBox::yesNoCancel()
  int msgCalModified();
  // returns KMsgBox::OKCandel()
  int msgItemDelete();

  /** show a file browser and get a file name.
    * open_save is 0 for open, 1 for save. */
  QString file_getname(int open_save);

  /**
   * takes the given fileName and adds it to the list of recent
   * files opened.
   */
  void add_recent_file(QString recentFileName);

  /** tell the alarm daemon that we have saved, and he needs to reread */
  void signalAlarmDaemon();

  // variables

  // Other variables
  CalPrinter *calPrinter;

  // menu stuff
  QPopupMenu *fileMenu, *editMenu;
  QPopupMenu *actionMenu, *optionsMenu, *viewMenu;
  QPopupMenu *helpMenu;
  QPopupMenu *recentPop;

  // toolbar stuff
  KToolBar    *tb;
  KMenuBar    *menubar;
  KStatusBar  *sb;
  int agendaButtonID;

  // the main display space frame. note that this is neccessary to
  // avoid things being put under the toolbar
  QSplitter    *panner;
  QFrame       *mainFrame;
  QFrame       *leftFrame;
  QWidgetStack *rightFrame;

  QFrame         *dateNavFrame;
  KDateNavigator *dateNavigator;       // widget showing small month view.

  KOAgendaView   *agendaView;          // "week" view
  KOListView     *listView;            // "list/day" view
  KOMonthView    *monthView; // a frame to hold the month view
  KOTodoView     *todoList, *todoView;

  KOBaseView     *currentView;  // currently active event view

  // view mode of agenda view (day, workweek, week, ...)
  int             agendaViewMode;

  // calendar object for this viewing instance
  CalObject      *calendar;		

  // various housekeeping variables.
  QString         fileName;		// the currently loaded filename
  QStrList        recentFileList;	// a list of recently accessed files
  bool            modifiedFlag;		// flag indicating if calendar i
  QTimer         *autoSaveTimer;        // used if calendar is to be autosaved
  int toolBarMenuId, statusBarMenuId;
  bool toolBarEnable, statusBarEnable; // only used at initialization time
  QDate saveSingleDate;                

  // dialogs
  OptionsDialog *optionsDlg;
  SearchDialog *searchDlg;

};

#endif // _TOPWIDGET_H

