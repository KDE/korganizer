/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001,2003,2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef CALENDARVIEW_H
#define CALENDARVIEW_H

#include <qwidget.h>
#include <qptrlist.h>
#include <qmap.h>

#include <libkcal/scheduler.h>

#include <korganizer/calendarviewbase.h>

class QWidgetStack;
class QSplitter;

class CalPrinter;
class KOFilterView;
class KOViewManager;
class KODialogManager;
class KOTodoView;
class DateNavigatorContainer;
class DateNavigator;
class KOIncidenceEditor;
class ResourceView;
class NavigatorBar;
class DateChecker;

namespace KOrg { class History; }

using namespace KCal;

class CalendarViewExtension : public QWidget
{
  public:
    CalendarViewExtension( QWidget *parent, const char *name = 0 )
      : QWidget( parent, name ) {}

    class Factory
    {
      public:
        virtual CalendarViewExtension *create( QWidget *parent ) = 0;
    };
};

/**
  This is the main calendar widget. It provides the different vies on t he
  calendar data as well as the date navigator. It also handles synchronization
  of the different views and controls the different dialogs like preferences,
  event editor, search dialog etc.

  @short main calendar view widget
  @author Cornelius Schumacher
*/
class CalendarView : public KOrg::CalendarViewBase, public Calendar::Observer
{
    Q_OBJECT
  public:
    /**
      Constructs a new calendar view widget.

      @param parent   parent window
      @param name     Qt internal widget object name
    */
    CalendarView( QWidget *parent = 0, const char *name = 0 );
    virtual ~CalendarView();

    void setCalendar( Calendar * );
    Calendar *calendar();

    KOrg::History *history() { return mHistory; }

    KOViewManager *viewManager();
    KODialogManager *dialogManager();

    QDate startDate();
    QDate endDate();

    QWidgetStack *viewStack();
    QWidget *leftFrame();
    NavigatorBar *navigatorBar();
    KOIncidenceEditor *editorDialog( Incidence* );

    DateNavigator *dateNavigator();

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

  signals:
    /** when change is made to options dialog, the topwidget will catch this
     *  and emit this signal which notifies all widgets which have registered
     *  for notification to update their settings. */
    void configChanged();
    /** emitted when the topwidget is closing down, so that any attached
        child windows can also close. */
    void closingDown();
    /** emitted right before we die */
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
    void dayPassed( QDate );


    /**
      Emitted, when clipboard content changes. Parameter indicates if paste
      is possible or not.
    */
    void pasteEnabled( bool );

    /** Emitted, when the number of incoming messages has changed. */
    void numIncomingChanged( int );

    /** Emitted, when the number of outgoing messages has changed. */
    void numOutgoingChanged( int );

    /** Send status message, which can e.g. be displayed in the status bar. */
    void statusMessage( const QString & );

    void calendarViewExpanded( bool );

    /** Emitted when auto-archiving options were modified */
    void autoArchivingSettingsModified();

  public slots:
    /** options dialog made a changed to the configuration. we catch this
     *  and notify all widgets which need to update their configuration. */
    void updateConfig();

    /**
      Load calendar from file \a filename. If \a merge is true, load
      calendar into existing one, if it is false, clear calendar, before
      loading. Return true, if calendar could be successfully loaded.
    */
    bool openCalendar( const QString &filename, bool merge = false );

    /**
      Save calendar data to file. Return true if calendar could be
      successfully saved.
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
    bool editIncidence( const QString& uid );
    void deleteIncidence();

    /** create an editeventwin with supplied date/time, and if bool is true,
     * make the event take all day. */
    void newEvent( QDateTime, QDateTime, bool allDay = false );
    void newEvent( QDateTime fh );
    void newEvent( QDate dt );
    /** create new event without having a date hint. Takes current date as
     default hint. */
    void newEvent();
    /**
      Create new Event from given string.
    */
    void newEvent( const QString & );
    void newEvent( const QString &summary, const QString &description,
                   const QString &attachment );
    void newFloatingEvent();

    /** Create a read-only viewer dialog for the supplied incidence. It calls the correct showXXX method*/
    void showIncidence( Incidence * );
    /** Create an editor for the supplied incidence. It calls the correct editXXX method*/
    bool editIncidence( Incidence * );
    /** Delete the supplied incidence. It calls the correct deleteXXX method*/
    void deleteIncidence( Incidence * );

    /** Create an editor for the supplied Journal. */
    void editJournal( Journal * );
    /** Delete the supplied journal. */
    void deleteJournal( Journal * );
    /** Create a read-only viewer dialog for the supplied event. */
    void showJournal( Journal * );

    /** Create an editor for the supplied event. */
    void editEvent( Event * );
    /** Delete the supplied event. */
    void deleteEvent( Event * );
    /**
      Delete the event with the given unique ID. Returns false, if event wasn't
      found.
    */
    bool deleteEvent( const QString &uid );
    /** Create a read-only viewer dialog for the supplied event. */
    void showEvent( Event * );

    /** Create an editor dialog for a todo */
    void editTodo( Todo * );
    /** Create a read-only viewer dialog for the supplied todo */
    void showTodo( Todo * );
    /** create new todo */
    void newTodo();
    /** create new todo, due on date */
    void newTodo( QDate date );
    /** create new todo with a parent todo */
    void newSubTodo();
    /** create new todo with a parent todo */
    void newSubTodo( Todo * );
    /** Delete todo */
    void deleteTodo( Todo * );
    /** Takes the todo's next occurence and marks the original as complete.*/
    void recurTodo( Todo * );

    void newTodo( const QString & );
    void newTodo( const QString &summary, const QString &description,
                  const QString &attachment );

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

    void editCanceled( Incidence * );

    void updateView( const QDate &start, const QDate &end );
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
    void printSetup();
    void printPreview();

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

    /** mails the currently selected event to a particular user as a vCalendar
      attachment. */
    void action_mail();

    /* frees a subtodo from it's relation */
    void todo_unsub();

    /** Take ownership of selected event. */
    void takeOverEvent();

    /** Take ownership of all events in calendar. */
    void takeOverCalendar();

    /** query whether or not the calendar is "dirty". */
    bool isModified();
    /** set the state of calendar. Modified means "dirty", i.e. needing a save. */
    void setModified( bool modified = true );

    /** query if the calendar is read-only. */
    bool isReadOnly();
    /** set state of calendar to read-only */
    void setReadOnly( bool readOnly = true );

    void eventUpdated( Incidence * );

    /* iTIP scheduling actions */
    void schedule_publish( Incidence *incidence = 0 );
    void schedule_request( Incidence *incidence = 0 );
    void schedule_refresh( Incidence *incidence = 0 );
    void schedule_cancel( Incidence *incidence = 0 );
    void schedule_add( Incidence *incidence = 0 );
    void schedule_reply( Incidence *incidence = 0 );
    void schedule_counter( Incidence *incidence = 0 );
    void schedule_declinecounter( Incidence *incidence = 0 );
    void mailFreeBusy( int daysToPublish = 30 );
    void uploadFreeBusy();

    void openAddressbook();

    void editFilters();

    void showFilter( bool visible );
    void updateFilter();
    void filterEdited();

    void showIntro();

    /** Move the current view date to the specified date */
    void goDate( const QDate& date );

    /** Move the current view date to today */
    void goToday();

    /** Move to the next date(s) in the current view */
    void goNext();

    /** Move to the previous date(s) in the current view */
    void goPrevious();

    void toggleExpand();
    void showLeftFrame( bool show = true );

    void dialogClosing( Incidence * );

    /** Look for new messages in the inbox */
    void lookForIncomingMessages();
   /** Look for new messages in the outbox */
    void lookForOutgoingMessages();

    void processMainViewSelection( Incidence * );
    void processTodoListSelection( Incidence * );

    void processIncidenceSelection( Incidence * );

    void purgeCompleted();

    void slotCalendarChanged();

    void slotAutoArchivingSettingsModified() { emit autoArchivingSettingsModified(); }

    void importQtopia( const QString &categoriesFile,
                       const QString &datebookFile,
                       const QString &tasklistFile );

    void showErrorMessage( const QString & );

  protected slots:
    /** Select a view or adapt the current view to display the specified dates. */
    void showDates( const KCal::DateList & );

  public:
    // show a standard warning
    // returns KMsgBox::yesNoCancel()
    int msgCalModified();

    /** Adapt navigation units corresponding to step size of navigation of the
     * current view.
     */
    void adaptNavigationUnits();

    //Attendee* getYourAttendee( Event *event );

  protected:
    void schedule( Scheduler::Method, Incidence *incidence = 0 );

    // returns KMsgBox::OKCandel()
    int msgItemDelete();

    Todo *selectedTodo();

    void warningChangeFailed( Incidence * );

  private:
    void init();

    void createPrinter();

    void calendarModified( bool, Calendar * );

    KOrg::History *mHistory;

    CalPrinter *mCalPrinter;

    QSplitter    *mPanner;
    QSplitter    *mLeftSplitter;
    QWidget      *mLeftFrame;
    QWidgetStack *mRightFrame;

    NavigatorBar *mNavigatorBar;

    DateNavigatorContainer *mDateNavigator;

    KOFilterView *mFilterView;

    QPtrList<CalendarViewExtension> mExtensions;

    Calendar *mCalendar;

    DateNavigator *mNavigator;
    DateChecker *mDateChecker;

    KOViewManager *mViewManager;
    KODialogManager *mDialogManager;

    // Calendar filters
    QPtrList<CalFilter> mFilters;

    // various housekeeping variables.
    bool            mModified; // flag indicating if calendar is modified
    bool            mReadOnly; // flag indicating if calendar is read-only
    QDate mSaveSingleDate;

    Incidence *mSelectedIncidence;

    KOTodoView *mTodoList;
    QMap<Incidence*,KOIncidenceEditor*> mDialogList;
};


class CalendarViewVisitor : public Incidence::Visitor
{
  public:
    CalendarViewVisitor() : mView( 0 ) {}

    bool act( Incidence *incidence, CalendarView *view )
    {
      mView = view;
      return incidence->accept( *this );
    }

  protected:
    CalendarView *mView;
};

class ShowIncidenceVisitor : public CalendarViewVisitor
{
  protected:
    bool visit( Event *event ) { mView->showEvent( event ); return true; }
    bool visit( Todo *todo ) { mView->showTodo( todo ); return true; }
    bool visit( Journal *journal ) { mView->showJournal( journal ); return true; }
};

class EditIncidenceVisitor : public CalendarViewVisitor
{
  protected:
    bool visit( Event *event ) { mView->editEvent( event ); return true; }
    bool visit( Todo *todo ) { mView->editTodo( todo ); return true; }
    bool visit( Journal *journal ) { mView->editJournal( journal ); return true; }
};

class DeleteIncidenceVisitor : public CalendarViewVisitor
{
  protected:
    bool visit( Event *event ) { mView->deleteEvent( event ); return true; }
    bool visit( Todo *todo ) { mView->deleteTodo( todo ); return true; }
    bool visit( Journal *journal ) { mView->deleteJournal( journal ); return true; }
};


#endif
