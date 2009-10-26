/*
  This file is part of KOrganizer.

  Copyright (c) 2002 Mike Pilone <mpilone@slac.com>
  Copyright (c) 2002 Don Sanders <sanders@kde.org>
  Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef KORG_ACTIONMANAGER_H
#define KORG_ACTIONMANAGER_H

#include "korganizer_export.h"
#include "korganizer/part.h"
#include <KMenuBar>

#include <KUrl>

#include <QDateTime>
#include <QObject>

#include <akonadi/item.h>

namespace KCal {
  class Calendar;
  class Incidence;
}

namespace KOrg {
  class AkonadiCalendar;
  class AkonadiCalendar;
  class HTMLExportSettings;
}

namespace Akonadi {
  class CalendarModel;
}

class AkonadiCollectionView;
class CalendarView;
class ImportDialog;
class KOWindowList;

using namespace KCal;

class KRecentFilesAction;
class KSelectAction;
class KTemporaryFile;
class KToggleAction;

/**
  The ActionManager creates all the actions in KOrganizer. This class
  is shared between the main application and the part so all common
  actions are in one location.
  It also provides D-Bus interfaces.
*/
class KORGANIZERPRIVATE_EXPORT ActionManager : public QObject
{
  Q_OBJECT
  public:
    ActionManager( KXMLGUIClient *client, CalendarView *widget,
                   QObject *parent, KOrg::MainWindow *mainWindow,
                   bool isPart, KMenuBar *menuBar = 0 );
    virtual ~ActionManager();

    /** Peform initialization that requires this* to be full constructed */
    void init();

    CalendarView *view() const { return mCalendarView; }

    /**
      Create Calendar object based on the akonadi framework and set it on the view.
    */
    void createCalendarAkonadi();


  public slots:
    /** Add a new resource
        @param mUrl The url for the new resource. Either a local or a remote
                    resource will be added, depending on the type of the url.
    */
    bool addResource( const KUrl &mUrl );

    /**
      Open calendar file from URL. Merge into current calendar, if \a merge is true.
        @param url The URL to open
        @param merge If true, the items from the url will be inserted into the
                     current calendar (default resource). Otherwise a new
                     resource is added for the given url.
    */
    bool openURL( const KUrl &url, bool merge = false );

    /** Save calendar file to URL of current calendar */
    bool saveURL();

    /** Save calendar file to URL */
    bool saveAsURL( const KUrl &kurl );

    void exportHTML();
    void exportHTML( KOrg::HTMLExportSettings * );
    void toggleMenubar( bool dontShowWarning = false );

  public:
    /** Get current URL */
    KUrl url() const { return mURL; }

    /** Is there a instance with this URL? */
    static KOrg::MainWindow *findInstance( const KUrl &url );

    /** Open calendar file from URL */
    bool openURL( const QString &url );

    /** Open calendar file from URL */
    bool mergeURL( const QString &url );

    /** Save calendar file to URL */
    bool saveAsURL( const QString &url );

    /** Close calendar file opened from URL */
    void closeUrl();

    /** Get current URL as QString */
    QString getCurrentURLasString() const;

    /**
      Delete the incidence with the given unique id from current calendar.
      @param uid UID of the incidence to delete.
      @param force If true, all recurrences and sub-todos (if applicable) will
      be deleted without prompting for confirmation.
    */
    virtual bool deleteIncidence( const Akonadi::Item::Id &uid, bool force = false );

    bool editIncidence( const Akonadi::Item::Id &uid );

    /**
      Add an incidence to the active calendar.
      @param ical A calendar in iCalendar format containing the incidence.
    */
    bool addIncidence( const QString &ical );
    //bool addIncidence( const Akonadi::Item::Id &ical );

    bool showIncidence( const Akonadi::Item::Id &uid );

    /**
      Show an incidence in context. This means showing the todo, agenda or
      journal view (as appropriate) and scrolling it to show the incidence.
      @param uid Unique ID of the incidence to show.
    */
    bool showIncidenceContext( const Akonadi::Item::Id &uid );

  public slots:
    void openEventEditor( const QString &);
    void openEventEditor( const QString &summary,
                          const QString &description,
                          const QStringList &attachments );
    void openEventEditor( const QString &summary,
                          const QString &description,
                          const QStringList &attachments,
                          const QStringList &attendees );
    void openEventEditor( const QString &summary,
                          const QString &description,
                          const QString &uri,
                          const QString &file,
                          const QStringList &attendees,
                          const QString &attachmentMimetype );

    void openTodoEditor( const QString &);
    void openTodoEditor( const QString &summary,
                         const QString &description,
                         const QStringList &attachments );
    void openTodoEditor( const QString &summary,
                         const QString &description,
                         const QStringList &attachments,
                         const QStringList &attendees );
    void openTodoEditor( const QString &summary,
                         const QString &description,
                         const QString &uri,
                         const QString &file,
                         const QStringList &attendees,
                         const QString &attachmentMimetype,
                         bool isTask );

    void openJournalEditor( const QDate &date );
    void openJournalEditor( const QString &text, const QDate &date );
    void openJournalEditor( const QString &text );

    void showJournalView();
    void showTodoView();
    void showEventView();

    void goDate( const QDate &);
    void goDate( const QString &);
    void showDate( const QDate &date );

  public:
    QString localFileName();

    bool queryClose();

  signals:
    /**
      Emitted when the "New" action is activated.
    */
    void actionNew( const KUrl &url = KUrl() );
    void toggleMenuBar();
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

  public slots:
    /**
      Options dialog made a changed to the configuration. we catch this
      and notify all widgets which need to update their configuration.
    */
    void updateConfig();

    void processIncidenceSelection( const Akonadi::Item &item, const QDate &date );
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
    void saveProperties( KConfigGroup &);
    void readProperties( const KConfigGroup &);

    void loadParts();

    void importCalendar( const KUrl &url );

  protected slots:
     void setItems( const QStringList & );

    /** open new window */
    void file_new();

    /** open a file, load it into the calendar. */
    void file_open();

    /** open a file from the list of recent files. Also called from file_open()
        after the URL is obtained from the user.
        @param url the URL to open
    */
    void file_open( const KUrl &url );

    /** import a calendar from another program like ical. */
    void file_icalimport();

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

    void toggleDateNavigator();
    void toggleTodoView();
    void toggleEventViewer();
    void toggleResourceView();


    /** connected to CalendarView's signal which comes from the ArchiveDialog */
    void slotAutoArchivingSettingsModified();

    /** called by the auto archive timer to automatically delete/archive events */
    void slotAutoArchive();

    void setTitle();

    void updateUndoAction( const QString & );

    void updateRedoAction( const QString & );

    void slotImportDialogFinished( ImportDialog * );

    void agentCreated( KJob* );

  protected:
    /** Get URL for saving. Opens FileDialog. */
    KUrl getSaveURL();

    void showStatusMessageOpen( const KUrl &url, bool merge );

    /**
      Return widget used as parent for dialogs and message boxes.
    */
    QWidget *dialogParent();

  private slots:
    void dumpText( const QString & );  // only for debugging purposes

    void slotResourcesChanged(bool);
    void slotChangeComboActionItem(int);
    void slotNewStuffDownloaded(KJob *job);

  private:
    class ActionStringsVisitor;

    /** Create all the actions. */
    void initActions();
    void enableIncidenceActions( bool enable );

    KOrg::Part::List mParts; // List of parts loaded
    KUrl mURL;               // URL of calendar file
    QString mFile;           // Local name of calendar file
    QString mLastUrl;        // URL of last loaded calendar.

    KTemporaryFile *mTempFile;
    QTimer *mAutoArchiveTimer; // used for the auto-archiving feature

    // list of all existing KOrganizer instances
    static KOWindowList *mWindowList;

    // Actions
    KRecentFilesAction *mRecent;

    KToggleAction *mDateNavigatorShowAction;
    KToggleAction *mTodoViewShowAction;
    KToggleAction *mResourceViewShowAction;
    KToggleAction *mEventViewerShowAction;

    KToggleAction *mHideMenuBarAction;

    KAction *mNewEventAction;
    KAction *mNewTodoAction;
    KAction *mNewSubtodoAction;
    KAction *mNewJournalAction;

    KAction *mShowIncidenceAction;
    KAction *mEditIncidenceAction;
    KAction *mDeleteIncidenceAction;

    KAction *mCutAction;
    KAction *mCopyAction;
    KAction *mDeleteAction;
    KAction *mNextXDays;
    KAction *mPublishEvent;
    KAction *mForwardEvent;

    KAction *mUndoAction;
    KAction *mRedoAction;
    KMenuBar *mMenuBar;

    KSelectAction *mFilterAction;

    KXMLGUIClient *mGUIClient;
    KActionCollection *mACollection;
    CalendarView *mCalendarView;
    KOrg::MainWindow *mMainWindow;
    bool mIsPart;

    bool mHtmlExportSync;

    Akonadi::CalendarModel *mCalendarModel;
    KOrg::AkonadiCalendar *mCalendar;
    AkonadiCollectionView *mResourceView;

    bool mIsClosing;
};

#endif
