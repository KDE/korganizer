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
#include <kurl.h>

#include "qdatelist.h"
#include "calobject.h"

class CalendarView;
class KTempFile;
class KRecentFilesAction;

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
    KOrganizer(const char *name=0);
    virtual ~KOrganizer();

    // list of all existing KOrganizer instances
    static QList<KOrganizer> *windowList;

    /** Open calendar file from URL */
    bool openURL(const KURL &url);
    /** Merge calendar file from URL to current calendar */
    bool mergeURL(const KURL &url);
    /** Close calendar file opened from URL */
    bool closeURL();
    /** Save calendar file to URL of current calendar */
    bool saveURL();
    /** Save calendar file to URL */
    bool saveAsURL(const KURL & kurl);

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
    void file_openRecent(const KURL& url);

    /** import a calendar from another program like ical. */
    void file_import();

    /** open a calendar and add the contents to the current calendar. */
    void file_merge();

    /** delete or archive old entries in your calendar for speed/space. */
    void file_archive();

    /** save a file with the current fileName. */
    void file_save();

    /** save a file under a (possibly) different filename. */
    void file_saveas();

    /** close a file, prompt for save if changes made. */
    void file_close();

    /** exit the program, prompt for save if files are "dirty". */
    void file_quit();

    /** Open toolbar configuration dialog */
    void configureToolbars();

  protected slots:

    /** toggle the appearance of the tool bar. */
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

    /** Sets title of window according to filename and modification state */
    void setTitle();

  protected:
    void initActions();

    /** supplied so that close events close calendar properly.*/
    bool queryClose();
    bool queryExit();

    /** query whether autoSave is set or not */
    bool autoSave() { return mAutoSave; };

    /** Get URL for saving. Opens FileDialog. */
    KURL getSaveURL();


    // variables
    CalendarView *mCalendarView;  // Main view widget
    KURL mURL;      // URL of calendar file
    QString mFile;  // Local name of calendar file

    KTempFile *mTempFile;

//    KStatusBar  *sb;

    // Actions
    KRecentFilesAction *mRecent;

    // configuration settings
    bool mAutoSave;  // Auto-Save calendar on close

    QTimer         *mAutoSaveTimer;   // used if calendar is to be autosaved
};

#endif // _KORGANIZER_H
