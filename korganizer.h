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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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
#include <kparts/mainwindow.h>

#include "calendarview.h"
#include "korganizeriface.h"

class KTempFile;
class KRecentFilesAction;
class KOWindowList;
class KToggleAction;
class KProcess;
class KONewStuff;
class ActionManager;
class CalendarView;

using namespace KCal;

/**
  This is the main class for KOrganizer. It extends the KDE KMainWindow.
  it provides the main view that the user sees upon startup, as well as
  menus, buttons, etc. etc.

  @short constructs a new main window for korganizer
  @author Preston Brown
*/

class KOrganizer : public KParts::MainWindow, virtual public KOrganizerIface, public KOrg::MainWindow
{
    Q_OBJECT
  public:
    /**
      Constructs a new main window.
    
      @param document If true this window shows a calendar as document, if false
                      the resource based backend is used. 
      @param name     Qt internal widget name
    */
    KOrganizer( bool document, const char *name = 0 );
    virtual ~KOrganizer();

    KOrg::CalendarViewBase *view() const;
    ActionManager *actionManager() { return mActionManager; }

    /**
      Open calendar file from URL. Merge into current calendar, if \a merge is
      true.
    */
    bool openURL(const KURL &url,bool merge=false);
    /** Save calendar file to URL of current calendar */
    bool saveURL();
    /** Save calendar file to URL */
    bool saveAsURL(const KURL & kurl);
    /** Get current URL */
    KURL getCurrentURL() const;

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

    virtual KXMLGUIFactory *mainGuiFactory() { return factory(); }
    virtual QWidget *topLevelWidget() { return this; }
    virtual void addPluginAction( KAction * ) {}

  public slots:
    /** show status message */
    void showStatusMessage(const QString &);

    /** Sets the active state of the calendar belonging to this window. If a
      * calendar is active the alarm daemon checks and signals events for
      * alarm notification. The active calendar is loaded by default, when
      * starting KOrganizer.
      */
    void setActive(bool active=true);

  protected slots:

    /** using the KConfig associated with the kapp variable, read in the
     * settings from the config file.
     */
    void readSettings();

    /** write current state to config file. */
    void writeSettings();

    /** Open toolbar configuration dialog */
    void configureToolbars();

    /** toggle the appearance of the tool bars. */
    void toggleToolBars(bool);

    void toggleToolBar();

    void toggleStatusBar();

    void statusBarPressed(int);

    /** Sets title of window according to filename and modification state */
    void setTitle();

    void setNumIncoming(int);
    void setNumOutgoing(int);

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

  private:
    bool mDocument;
    Calendar *mCalendar;
    CalendarView *mCalendarView;  // Main view widget
    KOrg::Part::List mParts; // List of parts loaded

    QPtrList<KAction> mToolBarToggles; // List of toolbar hiding toggle actions
    KToggleAction *mToolBarToggleAction;
    KToggleAction *mStatusBarAction;

    // status bar ids
    enum { ID_HISTORY, ID_GENERAL, ID_ACTIVE, ID_MESSAGES_IN, ID_MESSAGES_OUT };
    ActionManager *mActionManager;

    bool mIsClosing;
};

#endif
