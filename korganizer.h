/* 	$Id$	 */

#ifndef _KORGANIZER_H
#define _KORGANIZER_H

#include <qframe.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qlist.h>
#include <qtabdialog.h>
#include <qframe.h>
#include <qsplitter.h>

#include <ktoolbar.h>
#include <kparts/mainwindow.h>
#include <kapp.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kmenubar.h>
#include <kurl.h>
#include "korganizeriface.h"

#include "qdatelist.h"
#include "calendar.h"

class CalendarView;
class KTempFile;
class KRecentFilesAction;
class KOWindowList;
class KToggleAction;
class KProcess;

using namespace KCal;

/**
 *
 * This is the main class for KOrganizer. It extends the KDE KMainWindow.
 * it provides the main view that the user sees upon startup, as well as
 * menus, buttons, etc. etc.
 *
 * @short constructs a new main window for korganizer
 * @author Preston Brown
 * @version $Revision$
 */
class KOrganizer : public KParts::MainWindow, virtual public KOrganizerIface
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

    /**
      Open calendar file from URL. Merge into current calendar, if \a merge is
      true.
    */
    bool openURL(const KURL &url,bool merge=false);
    /** Merge calendar file from URL to current calendar */
    bool mergeURL(const KURL &url);
    /** Save calendar file to URL of current calendar */
    bool saveURL();
    /** Save calendar file to URL */
    bool saveAsURL(const KURL & kurl);
    /** Save calendar if it is modified by the user. Ask user what to do. */
    bool saveModifiedURL();
    /** Get current URL */
    KURL getCurrentURL() const { return mURL; }

    /** Is there a instance with this URL? */
    static KOrganizer* findInstance(const KURL &url);

    /** Open calendar file from URL */
    bool openURL(QString url);
    /** Open calendar file from URL */
    bool mergeURL(QString url);
    /** Save calendar file to URL */
    bool saveAsURL(QString url);
    /** Close calendar file opened from URL */
    void closeURL();
    /** Get current URL as QString */
    QString getCurrentURLasString() const;
    /** Delete event with unique id VUID from current calendar */
    virtual bool deleteEvent(QString VUID);

    bool isActive() { return mActive; }

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

    /** show status message */
    void showStatusMessage(const QString &);

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

    /** revert to saved */
    void file_revert();

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
    void configureDateTime();

    /** Open toolbar configuration dialog */
    void configureToolbars();

    /** Save toolbar/statusbar options to disk */
    void saveOptions();

    /** Configure key bindings */
    void editKeys();
    
    /** Show tip of the day */
    void showTip();
    
    /** Show tip of the day */
    void showTipOnStart();
    
  protected slots:

    /** toggle the appearance of the tool bars. */
    void toggleToolBars(bool);

    void toggleToolBar();

    void toggleStatusBar();

    void toggleFilterView();

    void statusBarPressed(int);
    
    /** called by the autoSaveTimer to automatically save the calendar */
    void checkAutoSave();

    /** Sets title of window according to filename and modification state */
    void setTitle();

    void setNumIncoming(int);
    void setNumOutgoing(int);

    void configureDateTimeFinished(KProcess *);
    
  protected:
    void initActions();
    void initParts();

    /** supplied so that close events close calendar properly.*/
    bool queryClose();
    bool queryExit();

    /* Session management */
    void saveProperties(KConfig *);
    void readProperties(KConfig *);

    /** Get URL for saving. Opens FileDialog. */
    KURL getSaveURL();

  private:
    void writeActiveState();
  
    // variables
    CalendarView *mCalendarView;  // Main view widget
    KURL mURL;      // URL of calendar file
    QString mFile;  // Local name of calendar file
    QString mLastUrl;  // URL of last loaded calendar.

    KTempFile *mTempFile;

    QTimer         *mAutoSaveTimer;   // used if calendar is to be autosaved

    bool mActive;  // Indicates if this calendar is active (for alarm daemon)

    // list of all existing KOrganizer instances
    static KOWindowList *windowList;

    // Actions
    KRecentFilesAction *mRecent;

    QList<KAction> mToolBarToggles; // List of toolbar hiding toggle actions
    KToggleAction *mToolBarToggleAction;

    KToggleAction *mStatusBarAction;
    KToggleAction *mFilterViewAction;

    // status bar ids
    enum { ID_HISTORY, ID_GENERAL, ID_ACTIVE, ID_MESSAGES_IN, ID_MESSAGES_OUT };

  private slots:
    void dumpText(const QString &);  // only for debugging purposes
};

#endif // _KORGANIZER_H
