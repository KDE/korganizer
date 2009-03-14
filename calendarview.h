/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2003,2004 Cornelius Schumacher <schumacher@kde.org>
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
#ifndef CALENDARVIEW_H
#define CALENDARVIEW_H

#include "korganizer_export.h"
#include "interfaces/korganizer/calendarviewbase.h"

#include <QList>
#include <QMap>
#include <QWidget>

class CalPrinter;
class DateChecker;
class DateNavigator;
class DateNavigatorContainer;
class KODialogManager;
class KOEventEditor;
class KOEventViewer;
class KOIncidenceEditor;
class KOTodoView;
class KOViewManager;
class NavigatorBar;

class KVBox;

class QByteArray;
class QSplitter;
class QStackedWidget;

namespace KOrg {
  class History;
  class IncidenceChangerBase;
}
namespace KCal {
  class HTMLExportSettings;
}

using namespace KOrg;
using namespace KCal;

class CalendarViewExtension : public QWidget
{
  public:
    explicit CalendarViewExtension( QWidget *parent ) : QWidget( parent ) {}

    class Factory
    {
      public:
        virtual ~Factory() {}
        virtual CalendarViewExtension *create( QWidget *parent ) = 0;
    };
};

/**
  This is the main calendar widget. It provides the different views on the
  calendar data as well as the date navigator. It also handles synchronization
  of the different views and controls the different dialogs like preferences,
  event editor, search dialog etc.

  @short main calendar view widget
  @author Cornelius Schumacher
*/
class KORGANIZERPRIVATE_EXPORT CalendarView : public KOrg::CalendarViewBase,
                                              public Calendar::CalendarObserver
{
  Q_OBJECT
  public:
    /**
      Constructs a new calendar view widget.
      @param parent   parent window
    */
    CalendarView( QWidget *parent=0 );
    virtual ~CalendarView();

    class CalendarViewVisitor : public IncidenceBase::Visitor
    {
      public:
        CalendarViewVisitor() : mView( 0 ) {}
        bool act( IncidenceBase *incidence, CalendarView *view )
        {
          mView = view;
          return incidence->accept( *this );
        }
      protected:
        CalendarView *mView;
    };

    class CanDeleteIncidenceVisitor : public CalendarViewVisitor
    {
      protected:
        bool visit( Event *event ) { return mView->deleteEvent( event ); }
        bool visit( Todo *todo ) { return mView->deleteTodo( todo ); }
        bool visit( Journal *journal ) { return mView->deleteJournal( journal ); }
        bool visit( FreeBusy * ) { return false; }
    };

    void setCalendar( Calendar * );
    Calendar *calendar();

    History *history() const { return mHistory; }

    KOViewManager *viewManager() const { return mViewManager; }
    KODialogManager *dialogManager() const { return mDialogManager; }

    QStackedWidget *viewStack() const { return mRightFrame; }
    QWidget *leftFrame() const { return mLeftFrame; }
    NavigatorBar *navigatorBar() const { return mNavigatorBar; }
    DateNavigator *dateNavigator() const { return mNavigator; }

    KOIncidenceEditor *editorDialog( Incidence *incidence ) const;
    KOrg::IncidenceChangerBase *incidenceChanger() const { return mChanger; }

    QDate startDate();
    QDate endDate();

    void addView( KOrg::BaseView * );
    void showView( KOrg::BaseView * );

    /**
      Add calendar view extension widget. CalendarView takes ownership of the
      objects created by the factory.
    */
    void addExtension( CalendarViewExtension::Factory * );

    /** currentSelection() returns a pointer to the incidence selected in the current view */
    Incidence *currentSelection();

    /** Return a pointer to the incidence selected in the current view. If there
        is no selection, return the selected todo from the todo list on the left */
    Incidence *selectedIncidence();

    /** Returns true if there's a filter applied */
    bool isFiltered() const;

    /** Returns the name of the current filter */
    QString currentFilterName() const;

  signals:
    /** when change is made to options dialog, the topwidget will catch this
     *  and emit this signal which notifies all widgets which have registered
     *  for notification to update their settings. */
    void configChanged();

    /** Emitted when the categories were updated, and thus the categories editor
     *  dialog needs to reload the list of categories */
    void categoriesChanged();

    /** Emitted when the categories were edited by the user, and thus the views
     *  need to reload the list of categories */
    void categoryConfigChanged();

    /** Emitted when the topwidget is closing down, so that any attached
        child windows can also close. */
    void closingDown();

    /** Emitted right before we die */
    void closed( QWidget * );

    /** Emitted when state of modified flag changes */
    void modifiedChanged( bool );

    /** Emitted when state of read-only flag changes */
    void readOnlyChanged( bool );

    /** Emitted when the unit of navigation changes */
    void changeNavStringPrev( const QString & );
    void changeNavStringNext( const QString & );

    /** Emitted when state of events selection has changed and user is organizer*/
    void organizerEventsSelected( bool );
    /** Emitted when state of events selection has changed and user is attendee*/
    void groupEventsSelected( bool );
    /**
      Emitted when an incidence gets selected. If the selection is cleared the
      signal is emitted with 0 as argument.
    */
    void incidenceSelected( Incidence * );
    /** Emitted, when a todoitem is selected or deselected.
        the connected slots enables/disables the corresponding menu items */
    void todoSelected( bool );
    void subtodoSelected( bool );

    /** Emitted, when a day changed (i.e. korganizer was running at midnight).
        The argument is the new date */
    void dayPassed( const QDate & );
    /**
      Attendees were removed from this incidence. Only the removed attendees
      are present in the incidence, so we just need to send a cancel messages
      to all attendees groupware messages are enabled at all.
    */
    void cancelAttendees( Incidence * );

    /**
      Emitted, when clipboard content changes. Parameter indicates if paste
      is possible or not.
    */
    void pasteEnabled( bool );
    /** Send status message, which can e.g. be displayed in the status bar. */
    void statusMessage( const QString & );

    void calendarViewExpanded( bool );

    /** Emitted when auto-archiving options were modified */
    void autoArchivingSettingsModified();

    void newIncidenceChanger( KOrg::IncidenceChangerBase * );
    void exportHTML( HTMLExportSettings * );

    void newFilterListSignal( const QStringList & );
    void selectFilterSignal( int );
    void filterChanged();

  public slots:
    /** options dialog made a changed to the configuration. we catch this
     *  and notify all widgets which need to update their configuration. */
    void updateConfig( const QByteArray & );

    /** Calendar configuration was changed, so refresh categories list
    */
    void updateCategories();

    /**
      Load calendar from file \a filename. If \a merge is true, load
      calendar into existing one, if it is false, clear calendar, before
      loading. Return true, if calendar could be successfully loaded.
        @param filename the file name to load the calendar from
        @param merge If true, the items from the file are inserted into the
                     current calendar (default resource or calendar file). If
                     false, the file is added as a new calendar resource.
    */
    bool openCalendar( const QString &filename, bool merge=false );

    /**
      Save calendar data to file. Return true if calendar could be
      successfully saved.
        @param filename The file name to save the calendar to
    */
    bool saveCalendar( const QString &filename );

    /**
      Close calendar. Clear calendar data and reset views to display an empty
      calendar.
    */
    void closeCalendar();

    /** Archive old events of calendar */
    void archiveCalendar();

    void showIncidence();
    void editIncidence();
    bool editIncidence( const QString &uid );
    bool showIncidence( const QString &uid );

    /**
      Show an incidence in context. This means showing the todo, agenda or
      journal view (as appropriate) and scrolling it to show the incidence.
      @param uid Unique ID of the incidence to show.
    */
    bool showIncidenceContext( const QString &uid );
    void deleteIncidence();

    /**
      Add an incidence to the active calendar.
      @param ical A calendar in iCalendar format containing the incidence. The
                  calendar must consist of a VCALENDAR component which contains
                  the incidence (VEVENT, VTODO, VJOURNAL or VFREEBUSY) and
                  optionally a VTIMEZONE component. If there is more than one
                  incidence, only the first is added to KOrganizer's calendar.
    */
    bool addIncidence( const QString &ical );

    void connectIncidenceEditor( KOIncidenceEditor * );

    /** create new event without having a date hint. Takes current date as
     default hint. */
    void newEvent();

    /** create an editeventwin with supplied date/time, and if bool is true,
     * make the event take all day. */
    void newEvent( const QDate &startDt );
    void newEvent( const QDateTime &startDt );
    void newEvent( const QDateTime &startDt, const QDateTime &EndDt, bool allDay=false );

    /**
      Create new Event from given summary, description, attachment list and
      attendees list
    */
    void newEvent( const QString &summary, const QString &description=QString(),
                   const QStringList &attachment=QStringList(),
                   const QStringList &attendees=QStringList(),
                   const QStringList &attachmentMimetypes=QStringList(),
                   bool inlineAttachment=false );
    void newFloatingEvent();

    /** Create a read-only viewer dialog for the supplied incidence.
        It calls the correct showXXX method */
    void showIncidence( Incidence * );

    /**
      Show an incidence in context. This means showing the todo, agenda or
      journal view (as appropriate) and scrolling it to show the incidence.
      @param incidence The incidence to show.
    */
    void showIncidenceContext( Incidence *incidence );

    /** Create an editor for the supplied incidence. It calls the correct editXXX method*/
    bool editIncidence( Incidence *incidence, bool isCounter = false );

    /**
      Delete the supplied incidence. It calls the correct deleteXXX method
      @param force If true, all recurrences and sub-todos (if applicable) will be
                   deleted without prompting for confirmation.
    */
    void deleteIncidence( Incidence *, bool force=false );

    /**
      Cuts the selected incidence using the edit_cut() method
    */
    void cutIncidence( Incidence * );

    /**
      Copies the selected incidence using the edit_copy() method
    */
    void copyIncidence( Incidence *);

    /**
      Pastes the current incidence using the edit_paste() method
    */
    void pasteIncidence();

    /** Delete the supplied todo and all sub-todos */
    void deleteSubTodosIncidence ( Todo *todo );

    /**
      Delete the todo incidence, and its sub-to-dos.
      @param todo The todo to delete.
      @param force If true, all sub-todos will be deleted without prompting for confirmation.
    */
    void deleteTodoIncidence ( Todo *todo, bool force=false );

    /** Check if deleting the supplied event is allowed. */
    bool deleteEvent( Event * ) { return true; }

    /** Check if deleting the todo is allowed */
    bool deleteTodo( Todo * ) {return true; }

    /** Check if deleting the supplied journal is allowed. */
    bool deleteJournal( Journal * ) { return true; }

    /**
      Delete the incidence with the given unique ID. Returns false, if event wasn't found.
      @param uid The UID of the incidence to delete.
      @param force If true, all recurrences and sub-todos (if applicable) will be
                   deleted without prompting for confirmation.
    */
    bool deleteIncidence( const QString &uid, bool force=false );

    /** create new todo */
    void newTodo();

    /** create new todo, due on date */
    void newTodo( const QDate &date );

    /** create new todo with a parent todo */
    void newSubTodo();

    /** create new todo with a parent todo */
    void newSubTodo( Todo * );

    void newTodo( const QString &summary, const QString &description=QString(),
                  const QStringList &attachments=QStringList(),
                  const QStringList &attendees=QStringList(),
                  const QStringList &attachmentMimetypes=QStringList(),
                  bool inlineAttachment=false );

    void newJournal();
    void newJournal( const QDate &date );
    void newJournal( const QString &text, const QDate &date=QDate() );

    void toggleAlarm( Incidence *incidence );
    void toggleTodoCompleted( Incidence *incidence );
    void dissociateOccurrences( Incidence *incidence, const QDate &date );

    /**
      Check if clipboard contains vCalendar event. The signal pasteEnabled() is
      emitted as result.
    */
    void checkClipboard();

    /**
      Using the KConfig associated with the kapp variable, read in the
      settings from the config file.

      You have to call setCalendar before calling readSettings.
    */
    void readSettings();

    /** write current state to config file. */
    void writeSettings();

    /** read settings for calendar filters */
    void readFilterSettings( KConfig *config );

    /** write settings for calendar filters */
    void writeFilterSettings( KConfig *config );

    /** passes on the message that an event has changed to the currently
     * activated view so that it can make appropriate display changes. */
    void changeIncidenceDisplay( Incidence *, int );

    void incidenceAdded( Incidence * );
    void incidenceChanged( Incidence *oldEvent, Incidence *newEvent );
    void incidenceChanged( Incidence *oldEvent, Incidence *newEvent, int what );
    void incidenceToBeDeleted( Incidence *incidence );
    void incidenceDeleted( Incidence * );
    void startMultiModify( const QString &text );
    void endMultiModify();

    void editCanceled( Incidence * );

    void updateView( const QDate &start, const QDate &end, const bool updateTodos );
    void updateView();

    void updateUnmanagedViews();

    /** cut the current appointment to the clipboard */
    void edit_cut();

    /** copy the current appointment(s) to the clipboard */
    void edit_copy();

    /** paste the current vobject(s) in the clipboard buffer into calendar */
    void edit_paste();

    /** edit viewing and configuration options. */
    void edit_options();

    /**
      Functions for printing, previewing a print, and setting up printing
      parameters.
    */
    void print();
    void printPreview();

    /** Export as HTML file */
    void exportWeb();

    /** Export as iCalendar file */
    void exportICalendar();

    /** Export as vCalendar file */
    void exportVCalendar();

    /** pop up a dialog to show an existing appointment. */
    void appointment_show();
    /**
     * pop up an Appointment Dialog to edit an existing appointment. Get
     * information on the appointment from the list of unique IDs that is
     * currently in the View, called currIds.
     */
    void appointment_edit();

    /**
     * pop up dialog confirming deletion of currently selected event in the
     * View.
     */
    void appointment_delete();

    /** Frees a subtodo from it's relation, update the view */
    void todo_unsub();

    /** Free a subtodo from it's relation, without update the view */
    bool todo_unsub( Todo *todo );

    /** Make all sub-to-dos of todo independents, update the view*/
    bool makeSubTodosIndependents ( );

    /** Make all sub-to-dos of todo independents, not update the view*/
    bool makeSubTodosIndependents ( Todo *todo );

    /** Take ownership of selected event. */
    void takeOverEvent();

    /** Take ownership of all events in calendar. */
    void takeOverCalendar();

    /** query whether or not the calendar is "dirty". */
    bool isModified();

    /** set the state of calendar. Modified means "dirty", i.e. needing a save.
        @param modified Whether the calendar has been modified
    */
    void setModified( bool modified=true );

    /** query if the calendar is read-only. */
    bool isReadOnly();

    /** set state of calendar to read-only
        @param readOnly whether the calendar view should be set read-only or not
    */
    void setReadOnly( bool readOnly=true );

    void eventUpdated( Incidence * );

    /* iTIP scheduling actions */
    void schedule_publish( Incidence *incidence=0 );
    void schedule_request( Incidence *incidence=0 );
    void schedule_refresh( Incidence *incidence=0 );
    void schedule_cancel( Incidence *incidence=0 );
    void schedule_add( Incidence *incidence=0 );
    void schedule_reply( Incidence *incidence=0 );
    void schedule_counter( Incidence *incidence=0 );
    void schedule_declinecounter( Incidence *incidence=0 );
    void schedule_forward( Incidence *incidence=0 );
    void mailFreeBusy( int daysToPublish=30 );
    void uploadFreeBusy();

    void openAddressbook();

    void editFilters();

    void updateFilter();

    void showIntro();

    void showDateNavigator( bool );
    void showTodoView( bool );
    void showEventViewer( bool );

    /** Move the current view date to the specified date */
    void goDate( const QDate &date );

    /** Show the given date without changing date selection length. */
    void showDate( const QDate &date );

    /** Move the current view date to today */
    void goToday();

    /** Move to the next date(s) in the current view */
    void goNext();

    /** Move to the previous date(s) in the current view */
    void goPrevious();

    void showLeftFrame( bool show=true );

    void dialogClosing( Incidence * );

    void processMainViewSelection( Incidence * );
    void processTodoListSelection( Incidence * );

    void processIncidenceSelection( Incidence * );

    void purgeCompleted();

    void slotAutoArchivingSettingsModified() { emit autoArchivingSettingsModified(); }

    void showErrorMessage( const QString & );
    void schedule( iTIPMethod, Incidence *incidence );
    void addIncidenceOn( Incidence *, const QDate & );
    void moveIncidenceTo( Incidence *, const QDate & );
    void filterActivated( int filterNum );

    void resourcesChanged();

  protected slots:
    /** Select a view or adapt the current view to display the specified dates. */
    void showDates( const KCal::DateList & );

  public:
    int msgCalModified();

    /**
      Adapts navigation units according to the current view's navigation step size.
    */
    void adaptNavigationUnits();

    /**
      Returns the best guess at the current active date in the view.

      @param fallbackToToday If guessing doesn't work, some views will prefer
      today to be returned instead of the first select date in the day matrix,
      Journal view for example.
    */
    QDate activeDate( bool fallbackToToday = false );

  protected:
    void setIncidenceChanger( KOrg::IncidenceChangerBase *changer );

    int msgItemDelete( Incidence *incidence );

    Todo *selectedTodo();

    void warningChangeFailed( Incidence * );
    void checkForFilteredChange( Incidence *incidence );

    /**
      Adjust the given date/times by valid defaults (selection or configured
      defaults, if invalid values are given) and allow the view to adjust the type.
    */
    void dateTimesForNewEvent( QDateTime &startDt, QDateTime &endDt, bool &allDay );
    KOEventEditor *newEventEditor( const QDateTime &startDtParam=QDateTime(),
                                   const QDateTime &endDtParam=QDateTime(),
                                   bool allDayParam=false );

    bool eventFilter( QObject *watched, QEvent *event );

  private:
    void init();

    void createPrinter();

    void dissociateOccurrence( Incidence *, const QDate & );
    void dissociateFutureOccurrence( Incidence *, const QDate & );

    void calendarModified( bool, Calendar * );

    // Helper function for purgeCompleted that recursively purges a todo and
    // its subitems. If it cannot delete a completed todo (because it has
    // uncompleted subitems), notAllPurged is set to true.
    bool purgeCompletedSubTodos( Todo *todo, bool &notAllPurged );

    KOrg::History *mHistory;

    CalPrinter *mCalPrinter;

    QSplitter *mPanner;
    QSplitter *mLeftSplitter;
    QWidget *mLeftFrame;
    QStackedWidget *mRightFrame;

    NavigatorBar *mNavigatorBar;

    DateNavigatorContainer *mDateNavigator;

    QList<CalendarViewExtension*> mExtensions;

    Calendar *mCalendar;

    DateNavigator *mNavigator;
    DateChecker *mDateChecker;

    KVBox *mEventViewerBox;
    KOEventViewer *mEventViewer;
    KOViewManager *mViewManager;
    KODialogManager *mDialogManager;

    // Calendar filters
    QList<CalFilter*> mFilters;
    CalFilter *mCurrentFilter;

    // various housekeeping variables.
    bool  mModified; // flag indicating if calendar is modified
    bool  mReadOnly; // flag indicating if calendar is read-only
    QDate mSaveSingleDate;

    Incidence *mSelectedIncidence;
    KOTodoView *mTodoList;
    QMap<Incidence*,KOIncidenceEditor*> mDialogList;

    KOrg::IncidenceChangerBase *mChanger;
    QList<int> mMainSplitterSizes; // temp store for main splitter sizes while left frame is hidden
    bool mSplitterSizesValid;
};

#endif

