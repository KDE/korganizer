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
class KOWindowList;
class ArchiveDialog;

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
     * Constructs a new main window.
     *
     * @param name Qt internal widget name
     */
    KOrganizer(const char *name=0);
    virtual ~KOrganizer();

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
    
    /** emitted when this calendar has been made active */
    void calendarActivated(KOrganizer *);
    
  public slots:

    /** options dialog made a changed to the configuration. we catch this
     *  and notify all widgets which need to update their configuration. */
    void updateConfig();

    /** Sets the active state of the calendar belonging to this window. If a
      * calendar is active the alarm daemon checks and signals events for
      * alarm notification. The active calendar is loaded by default, when
      * starting KOrganizer.
      */
    void setActive(bool active=true);

    /** Make calendar active */
    void makeActive();

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

    /** Open kcontrol module for configuring date and time formats */
//    void configureDateTime();

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

    QTimer         *mAutoSaveTimer;   // used if calendar is to be autosaved

  private:
    // list of all existing KOrganizer instances
    static KOWindowList *windowList;

    bool mActive;  // Indicates if this calendar is active (for alarm daemon)

    ArchiveDialog *mArchiveDialog;

  private slots:
    void dumpText(const QString &);  // only for debugging purposes
};

#endif // _KORGANIZER_H
