/* 	$Id$	 */

#ifndef _KORGANIZER_H
#define _KORGANIZER_H

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

class CalendarView;

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
class KOrganizer : public KTMainWindow
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
  KOrganizer(QString filename="", bool fnOverride=true, const char *name=0);
  virtual ~KOrganizer();

  // public variables
  static QList<KOrganizer> windowList;

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

public slots:

  /** options dialog made a changed to the configuration. we catch this
   *  and notify all widgets which need to update their configuration. */
  void updateConfig();


 protected slots:
 
  /** using the KConfig associated with the kapp variable, read in the
   * settings from the config file. 
   */
  void readSettings();
  
  /** write current state to config file. */
  void writeSettings();

  /** open new window */
  void file_new();

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

  /** save a file with the current fileName. returns nonzero on error. */
  int file_save();

  /** save a file under a (possibly) different filename. Returns nonzero
   * on error. */
  int file_saveas();

  /** close a file, prompt for save if changes made. */
  void file_close();

  /** exit the program, prompt for save if files are "dirty". */
  void file_quit();


protected slots:

  /** toggle the appearance of the menuBar. */
  void toggleToolBar() 
    { 
      if(toolBar()->isVisible())
        toolBar()->hide();
      else
        toolBar()->show();
    };

// We currently don't use a status bar

  /** toggle the appearance of the statusBar. */
/*
  void toggleStatusBar() 
    {
      sb->enable(KStatusBar::Toggle);
      updateRects();
      optionsMenu->setItemChecked(statusBarMenuId, 
				  !optionsMenu->isItemChecked(statusBarMenuId));
    };
*/

  /** called by the autoSaveTimer to automatically save the calendar */
  void checkAutoSave();

protected:
  void initMenus();
  void initToolBar();

  void initActions();

  /** supplied so that close events call file_close()/file_close() properly.*/
  bool queryClose();
  bool queryExit();

  /** show a file browser and get a file name.
    * open_save is 0 for open, 1 for save. */
  QString file_getname(int open_save);

  /**
   * takes the given fileName and adds it to the list of recent
   * files opened.
   */
  void add_recent_file(QString recentFileName);

  /** query whether autoSave is set or not */
  bool autoSave() { return mAutoSave; };

  /** Sets title of window according to filename and modification state */
  void setTitle();

  // variables
  CalendarView *mCalendarView;  // Main view widget
  QString mFilename;     // Name of calendar file

  // menu stuff
  QPopupMenu *fileMenu, *editMenu;
  QPopupMenu *actionMenu, *optionsMenu, *viewMenu;
  QPopupMenu *helpMenu;
  QPopupMenu *recentPop;

  // toolbar stuff
  KToolBar    *tb;
  KMenuBar    *menubar;
  KStatusBar  *sb;

  // configuration settings
  bool mAutoSave;  // Auto-Save calendar on close
  QStrList        recentFileList;	// a list of recently accessed files

  QTimer         *mAutoSaveTimer;   // used if calendar is to be autosaved

  int toolBarMenuId, statusBarMenuId;
  bool toolBarEnable, statusBarEnable; // only used at initialization time
};

#endif // _KORGANIZER_H
