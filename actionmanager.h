/*
    This file is part of KOrganizer.

    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>
    Copyright (c) 2002 Don Sanders <sanders@kde.org>
    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KORG_ACTIONMANAGER_H
#define KORG_ACTIONMANAGER_H

#include <qobject.h>
#include <kurl.h>
#include <korganizer/part.h>

#include "kcalendariface.h"

namespace KCal
{
  class Calendar;
  class CalendarResources;
  class Incidence;
  class ResourceCalendar;
}
namespace KOrg
{
  class MainWindow;
}

class KAction;
class KActionCollection;
class KRecentFilesAction;
class KSelectAction;
class KToggleAction;
class KConfig;
class KProcess;
class KTempFile;
class KXMLGUIClient;
class CalendarView;
class KOrganizer;
class KONewStuff;
class KOWindowList;
class ImportDialog;
class ResourceView;
class HTMLExportSettings;

using namespace KCal;

/**
  The ActionManager creates all the actions in KOrganizer. This class
  is shared between the main application and the part so all common
  actions are in one location.
  It also provides DCOP interface[s].
*/
class ActionManager : public QObject, public KCalendarIface
{
    Q_OBJECT
  public:
    ActionManager( KXMLGUIClient *client, CalendarView *widget,
                   QObject *parent, KOrg::MainWindow *mainWindow,
                   bool isPart );
    virtual ~ActionManager();

    /** Peform initialization that requires this* to be full constructed */
    void init();

    CalendarView *view() const { return mCalendarView; }

    /**
      Create Calendar object based on local file and set it on the view.
    */
    void createCalendarLocal();
    /**
      Create Calendar object based on the resource framework and set it on the
      view.
    */
    void createCalendarResources();

    /**
      Save calendar to disk.
    */
    void saveCalendar();

    /**
      Save the resource based calendar. Return false if an error occured and the
      user decidec to not ignore the error. Otherwise it returns true.
    */
    bool saveResourceCalendar();

  public slots:
    /** Add a new resource */
    bool addResource( const KURL &mUrl );
    /**
      Open calendar file from URL. Merge into current calendar, if \a merge is
      true.
    */
    bool openURL( const KURL &url, bool merge = false );
    /** Save calendar file to URL of current calendar */
    bool saveURL();
    /** Save calendar file to URL */
    bool saveAsURL( const KURL &kurl );
    /** Save calendar if it is modified by the user. Ask user what to do. */
    bool saveModifiedURL();

    void exportHTML();
    void exportHTML( HTMLExportSettings * );
  public:
    /** Get current URL */
    KURL url() const { return mURL; }

    /** Is there a instance with this URL? */
    static KOrg::MainWindow* findInstance( const KURL &url );
    /** Open calendar file from URL */
    bool openURL( QString url );
    /** Open calendar file from URL */
    bool mergeURL( QString url );
    /** Save calendar file to URL */
    bool saveAsURL( QString url );
    /** Close calendar file opened from URL */
    void closeURL();
    /** Get current URL as QString */
    QString getCurrentURLasString() const;
    /** Delete event with the given unique id from current calendar. */
    virtual bool deleteIncidence( const QString& uid );

    bool editIncidence( const QString& uid );

    //// Implementation of the DCOP interface
    virtual ResourceRequestReply resourceRequest( const QValueList<QPair<QDateTime, QDateTime> >& busy,
                                                  const QCString& resource,
                                                  const QString& vCalIn );

    void openEventEditor( QString );
    void openEventEditor( QString summary, QString description,
                          QString attachment );
    void openEventEditor( QString summary, QString description,
                          QString attachment, QStringList attendees );

    void openTodoEditor( QString );
    void openTodoEditor( QString summary, QString description,
                         QString attachment );
    void openTodoEditor( QString summary, QString description,
                         QString attachment, QStringList attendees );

    void openJournalEditor( QDate date );
    void openJournalEditor( QString text, QDate date );
    void openJournalEditor( QString text );
   //TODO:
   // void openJournalEditor( QString summary, QString description,
   //                         QString attachment );

    void showJournalView();
    void showTodoView();
    void showEventView();

    void goDate( QDate );
    void goDate( QString );

    QString localFileName();

    bool queryClose();

  signals:
    /**
      Emitted when the "New" action is activated.
    */
    void actionNew( const KURL &url = KURL() );

    /**
      When change is made to options dialog, the topwidget will catch this
      and emit this signal which notifies all widgets which have registered
      for notification to update their settings.
    */
    void configChanged();

    /**
      Emitted when the topwidget is closing down, so that any attached
      child windows can also close.
    */
    void closingDown();

    /**
      Announce filter selection changes.
    */
    void filterActivated( int );

    /** Indicates that a new resource was added */
    void resourceAdded( ResourceCalendar * );

  public slots:
    /**
      Options dialog made a changed to the configuration. we catch this
      and notify all widgets which need to update their configuration.
    */
    void updateConfig();

    void setDestinationPolicy();

    void processIncidenceSelection( Incidence * );
    void keyBindings();

    /**
      Using the KConfig associated with the kapp variable, read in the
      settings from the config file.
    */
    void readSettings();

    /**
      Write current state to config file.
    */
    void writeSettings();

    /* Session management */
    void saveProperties( KConfig * );
    void readProperties( KConfig * );

    void loadParts();

    void importCalendar( const KURL &url );

  protected slots:

    /** open new window */
    void file_new();

    /** open a file, load it into the calendar. */
    void file_open();

    /** open a file from the list of recent files. */
    void file_openRecent( const KURL &url );

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

    /** Open kcontrol module for configuring date and time formats */
    void configureDateTime();

    /** Show tip of the day */
    void showTip();

    /** Show tip of the day */
    void showTipOnStart();

    void downloadNewStuff();
    void uploadNewStuff();

    void toggleResourceButtons();

    /** called by the autoSaveTimer to automatically save the calendar */
    void checkAutoSave();

    /** connected to CalendarView's signal which comes from the ArchiveDialog */
    void slotAutoArchivingSettingsModified();

    /** called by the auto archive timer to automatically delete/archive events */
    void slotAutoArchive();

    void configureDateTimeFinished(KProcess *);

    void setTitle();

    void updateUndoAction( const QString & );

    void updateRedoAction( const QString & );

    void slotImportDialogFinished( ImportDialog * );

  protected:
    /** Get URL for saving. Opens FileDialog. */
    KURL getSaveURL();

    void showStatusMessageOpen( const KURL &url, bool merge );

    void initCalendar( Calendar *cal );

    /**
      Return widget used as parent for dialogs and message boxes.
    */
    QWidget *dialogParent();

  private slots:
    void dumpText( const QString & );  // only for debugging purposes

  private:
    class ActionStringsVisitor;

    /** Create all the actions. */
    void initActions();
    void enableIncidenceActions( bool enable );

    KOrg::Part::List mParts; // List of parts loaded
    KURL mURL;      // URL of calendar file
    QString mFile;  // Local name of calendar file
    QString mLastUrl;  // URL of last loaded calendar.

    KTempFile *mTempFile;
    QTimer *mAutoSaveTimer;   // used if calendar is to be autosaved
    QTimer *mAutoArchiveTimer; // used for the auto-archiving feature

    // list of all existing KOrganizer instances
    static KOWindowList *mWindowList;

    // Actions
    KRecentFilesAction *mRecent;
    KToggleAction *mResourceButtonsAction;
    KAction *mShowIncidenceAction;
    KAction *mEditIncidenceAction;
    KAction *mDeleteIncidenceAction;

    KAction *mCutAction;
    KAction *mCopyAction;
    KAction *mDeleteAction;
    KAction *mNextXDays;
    KAction *mPublishEvent;

    KAction *mUndoAction;
    KAction *mRedoAction;

    KSelectAction *mFilterAction;

    KXMLGUIClient *mGUIClient;
    KActionCollection *mACollection;
    CalendarView *mCalendarView;
    KOrg::MainWindow *mMainWindow;
    bool mIsPart;

    KONewStuff *mNewStuff;
    bool mHtmlExportSync;

    // Either mCalendar *or* mCalendarResources is set.
    Calendar *mCalendar;
    CalendarResources *mCalendarResources;

    ResourceView *mResourceView;

    bool mIsClosing;
};

#endif
