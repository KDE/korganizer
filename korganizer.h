/*
    This file is part of KOrganizer.
    Copyright (c) 1997, 1998, 1999
    Preston Brown (preston.brown@yale.edu)
    Fester Zigterman (F.J.F.ZigtermanRustenburg@student.utwente.nl)
    Ian Dawes (iadawes@globalserve.net)
    Laszlo Boloni (boloni@cs.purdue.edu)
    Copyright (c) 2000, 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KORGANIZER_H
#define KORGANIZER_H

#include <qframe.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qptrlist.h>
#include <qtabdialog.h>
#include <qsplitter.h>

#include <ktoolbar.h>
#include <kapplication.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kmenubar.h>
#include <kurl.h>

#include <libkcal/calendar.h>

#include <korganizer/mainwindow.h>
#include <korganizer/part.h>

#include "calendarview.h"
#include "korganizeriface.h"

class KTempFile;
class KRecentFilesAction;
class KOWindowList;
class KToggleAction;
class KProcess;

class KONewStuff;

using namespace KCal;

/**
  This is the main class for KOrganizer. It extends the KDE KMainWindow.
  it provides the main view that the user sees upon startup, as well as
  menus, buttons, etc. etc.
  
  @short constructs a new main window for korganizer
  @author Preston Brown
*/
class KOrganizer : public KOrg::MainWindow, virtual public KOrganizerIface
{
    Q_OBJECT
  public:
    /**
     * Constructs a new main window.
     *
     * @param name Qt internal widget name
     */
    KOrganizer( const char *name=0 );
    virtual ~KOrganizer();

    KOrg::CalendarViewBase *view() const { return mCalendarView; }

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
    static void setStartedKAddressBook(bool tmpBool) { startedKAddressBook = tmpBool; }
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
    /** Delete event with the given unique id from current calendar. */
    virtual bool deleteEvent(QString uid);

    bool isActive() { return mActive; }

    QString localFileName();

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

    /** Configure key bindings */
    void editKeys();
    
    /** Show tip of the day */
    void showTip();
    
    /** Show tip of the day */
    void showTipOnStart();
    
    void processIncidenceSelection( Incidence * );
    
    void downloadNewStuff();
    void uploadNewStuff();
    
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
    
    void slotNewToolbarConfig();
        
  protected:
    void initActions();
//    void initViews();

    /** supplied so that close events close calendar properly.*/
    bool queryClose();
    bool queryExit();

    /* Session management */
    void saveProperties(KConfig *);
    void readProperties(KConfig *);

    /** Get URL for saving. Opens FileDialog. */
    KURL getSaveURL();

    void enableIncidenceActions( bool enable );

  private slots:
    void dumpText(const QString &);  // only for debugging purposes

  private:
    void writeActiveState();
  
    // variables
    CalendarView *mCalendarView;  // Main view widget
    KOrg::Part::List mParts; // List of parts loaded
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

    QPtrList<KAction> mToolBarToggles; // List of toolbar hiding toggle actions
    KToggleAction *mToolBarToggleAction;

    KToggleAction *mStatusBarAction;
    KToggleAction *mFilterViewAction;

    KAction *mShowIncidenceAction;
    KAction *mEditIncidenceAction;
    KAction *mDeleteIncidenceAction;

    KAction *mCutAction;
    KAction *mCopyAction;
    KAction *mDeleteAction;
    
    KAction *mNextXDays;

    KAction *mPublishEvent;

    // status bar ids
    enum { ID_HISTORY, ID_GENERAL, ID_ACTIVE, ID_MESSAGES_IN, ID_MESSAGES_OUT };

    static bool startedKAddressBook; //whether we started KAddressBook ourselves

    KONewStuff *mNewStuff;

    // if true then the html-export at savetime is synchonous (blocking)
    // this is needed when saving while quiting
    bool mHtmlExportSync;
};

#endif
