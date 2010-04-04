/*
    This file is part of KOrganizer.

    Copyright (c) 2000,2001,2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef CALENDARVIEW_H
#define CALENDARVIEW_H

#include <qwidget.h>
#include <qptrlist.h>
#include <qmap.h>
#include <kfile.h>
#include <korganizer/koeventviewer.h>
#include <libkcal/scheduler.h>
#include <kdepimmacros.h>

#include "koglobals.h"
#include "interfaces/korganizer/calendarviewbase.h"

class QWidgetStack;
class QSplitter;

class KOViewManager;
class KODialogManager;
class KOTodoView;
class KOEventEditor;
class DateNavigatorContainer;
class DateNavigator;
class KOIncidenceEditor;
class ResourceView;
class NavigatorBar;
class DateChecker;

namespace KOrg { class History; class IncidenceChangerBase; }
class HTMLExportSettings;

using namespace KOrg;
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
  This is the main calendar widget. It provides the different views on the
  calendar data as well as the date navigator. It also handles synchronization
  of the different views and controls the different dialogs like preferences,
  event editor, search dialog etc.

  @short main calendar view widget
  @author Cornelius Schumacher
*/
class KDE_EXPORT CalendarView : public KOrg::CalendarViewBase, public Calendar::Observer
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
    };

    void setCalendar( Calendar * );
    Calendar *calendar();

    QPair<ResourceCalendar *, QString> viewSubResourceCalendar();

    KOrg::History *history() const { return mHistory; }

    KOViewManager *viewManager() const { return mViewManager; }
    KODialogManager *dialogManager() const { return mDialogManager; }

    QWidgetStack *viewStack() const { return mRightFrame; }
    QWidget *leftFrame() const { return mLeftFrame; }
    NavigatorBar *navigatorBar() const { return mNavigatorBar; }
    DateNavigator *dateNavigator() const { return mDateNavigator; }

    KOIncidenceEditor *editorDialog( Incidence* ) const;
    IncidenceChangerBase *incidenceChanger() const { return mChanger; }

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
    void incidenceSelected( Incidence *incidence, const QDate &date );
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

    void newIncidenceChanger( IncidenceChangerBase* );
    void exportHTML( HTMLExportSettings* );

    void newFilterListSignal( const QStringList & );
    void selectFilterSignal( int );
    void filterChanged();

  public slots:
    /** options dialog made a changed to the configuration. we catch this
     *  and notify all widgets which need to update their configuration. */
    void updateConfig( const QCString& );
    /** Calendar configuration was changed, so refresh categories list
    */
    void updateCategories();


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
    bool editIncidence( const QString &uid );
    bool editIncidence( const QString &uid, const QDate &date );
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

    void connectIncidenceEditor( KOIncidenceEditor *editor );

    /** create new event without having a date hint. Takes current date as
     default hint. */
    void newEvent();
    void newEvent( ResourceCalendar *res, const QString &subRes );
    /** create an editeventwin with supplied date/time, and if bool is true,
     * make the event take all day. */
    void newEvent( ResourceCalendar *res, const QString &subRes,
                   const QDate &startDt );
    void newEvent( ResourceCalendar *res, const QString &subRes,
                   const QDateTime &startDt );
    void newEvent( ResourceCalendar *res, const QString &subRes,
                   const QDateTime &startDt, const QDateTime &EndDt,
                   bool allDay = false );
    /**
      Create new Event from given summary, description, attachment list and
      attendees list
    */
    void newEvent( ResourceCalendar *res, const QString &subRes,
                   const QString &summary,
                   const QString &description = QString::null,
                   const QStringList &attachment = QStringList(),
                   const QStringList &attendees = QStringList(),
                   const QStringList &attachmentMimetypes = QStringList(),
                   bool inlineAttachment = false );

    /** Create a read-only viewer dialog for the supplied incidence. It calls the correct showXXX method*/
    void showIncidence( Incidence *, const QDate & );
    /** Create an editor for the supplied incidence. It calls the correct editXXX method*/
    bool editIncidence( Incidence *incidence, const QDate &date, bool isCounter = false );
    /**
      Delete the supplied incidence. It calls the correct deleteXXX method
      @param force If true, all recurrences and sub-todos (if applicable) will be
                   deleted without prompting for confirmation.
    */
    void deleteIncidence( Incidence *, bool force = false );
    /**
      Cuts the selected incidence using the edit_cut() method
    */
    void cutIncidence( Incidence * );
    /**
      Copies the selected incidence using the edit_copy() method
    */
    void copyIncidence( Incidence *);
    /**
      Pastes the curren incidence using the edit_paste() method
    */
    void pasteIncidence();

    /** Delete the supplied todo and all sub-todos */
    void deleteSubTodosIncidence ( Todo *todo );
    /**
      Delete the todo incidence, and its sub-to-dos.
      @param todo The todo to delete.
      @param force If true, all sub-todos will be deleted without prompting for confirmation.
    */
    void deleteTodoIncidence ( Todo *todo, bool force = false );
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
    bool deleteIncidence( const QString &uid, bool force = false );

    /** create new todo */
    void newTodo();
    void newTodo( ResourceCalendar *res, const QString &subRes );
    /** create new todo, due on date */
    void newTodo( ResourceCalendar *res, const QString &subRes,
                  const QDate &date );
    /** create new todo with a parent todo */
    void newSubTodo();
    /** create new todo with a parent todo */
    void newSubTodo( Todo * );

    void newTodo( ResourceCalendar *res, const QString &subRes,
                  const QString &summary,
                  const QString &description = QString::null,
                  const QStringList &attachments = QStringList(),
                  const QStringList &attendees = QStringList(),
                  const QStringList &attachmentMimetypes = QStringList(),
                  bool inlineAttachment = false, bool createTask = false );

    void newJournal();
    void newJournal( ResourceCalendar *res, const QString &subRes );
    void newJournal( ResourceCalendar *res, const QString &subRes,
                     const QDate &date );
    void newJournal( ResourceCalendar *res, const QString &subRes,
                     const QString &text, const QDate &date = QDate() );

    void toggleAlarm( Incidence * );
    void dissociateOccurrence( Incidence *, const QDate & );
    void dissociateFutureOccurrence( Incidence *, const QDate & );


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
    void incidenceChanged( Incidence *oldEvent, Incidence *newEvent,
                           KOGlobals::WhatChanged modification );
    void incidenceToBeDeleted( Incidence *incidence );
    void incidenceDeleted( Incidence * );
    void startMultiModify( const QString &text );
    void endMultiModify();

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

    /* frees a subtodo from it's relation, update the view */
    void todo_unsub();
    /* Free a subtodo from it's relation, without update the view */
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
    void schedule_forward( Incidence *incidence = 0 );
    void mailFreeBusy( int daysToPublish = 30 );
    void uploadFreeBusy();

    void openAddressbook();

    void editFilters();

    void updateFilter();

    void showIntro();

    void showDateNavigator( bool );
    void showTodoView( bool );
    void showEventViewer( bool );

    /** Move the current view date to the specified date */
    void goDate( const QDate& date );

    /** Show the given date without changing date selection length. */
    void showDate( const QDate &date );

    /** Move the current view date to today */
    void goToday();

    /** Move to the next date(s) in the current view */
    void goNext();

    /** Move to the previous date(s) in the current view */
    void goPrevious();

    void toggleExpand();
    void showLeftFrame( bool show = true );

    void dialogClosing( Incidence * );

    void processMainViewSelection( Incidence *incidence, const QDate &date );
    void processTodoListSelection( Incidence *incidence, const QDate &date );

    void processIncidenceSelection( Incidence *incidence, const QDate &date );

    void purgeCompleted();

    void slotAutoArchivingSettingsModified() { emit autoArchivingSettingsModified(); }

    void showErrorMessage( const QString & );
    void schedule( Scheduler::Method, Incidence *incidence );
    void addIncidenceOn( Incidence *, const QDate & );
    void moveIncidenceTo( Incidence *, const QDate & );
    void filterActivated( int filterNum );

    void resourcesChanged();

    /**
      The user clicked on a week number in the date navigator

      Lets select a week or a work week depending on the user's
      config option.
    */
    void selectWeek( const QDate & );

  protected slots:
    /** Select a view or adapt the current view to display the specified dates.
        preferredMonth is useful when the datelist crosses months, if valid,
        any month-like component should honour this
    */
    void showDates( const KCal::DateList &, const QDate &preferredMonth = QDate() );

  public:
    // show a standard warning
    // returns KMsgBox::yesNoCancel()
    int msgCalModified();

    /** Adapt navigation units corresponding to step size of navigation of the
     * current view.
     */
    void adaptNavigationUnits();

    /**
      Returns the date of the selected incidence.

      If the selected incidence is recurring, it will return
      the date of the selected occurrence
    */
    QDate activeIncidenceDate();

    /**
      Returns the best guess at the current active date in the view.
      This has nothing to do with selected incidences, use activeIncidenceDate()
      for that, for example, agenda supports time selection and incidence selection
      and they can have diferent dates.

      @param fallbackToToday If guessing doesn't work, some views will prefer
      today to be returned instead of the first select date in the day matrix,
      Journal view for example.
    */
    QDate activeDate( bool fallbackToToday = false );

    /**
       Asks the user if he wants to edit only this occurrence, all
       occurrences or only future occurrences, and then dissociates
       the incidence if needed.

       @param inc The recurring incidence that's about to be edited.
       @param userAction Specifies what the user is doing with the occurrence,
              like cutting, pasting or editing, it only influences the strings
              in the message box.
       @param chosenOption After calling this function, it will hold the user's
              chosen option.
       @param itemDate The date of the selected view item
       @param commitToCalendar If true, mChanger is called and the dissociation
              is saved to the calendar. If false, it's up to the caller to do that.

       @return A pointer to the incidence that should be edited which is
               0 if the user pressed cancel, inc if the user pressed
               "All Occurrences", or points to a newly created incidence
               when dissociation is involved in which case the caller
               is responsible to add it to the calendar and freeing it.
     **/
    Incidence* singleOccurrenceOrAll( Incidence *inc,
                                      KOGlobals::OccurrenceAction userAction,
                                      KOGlobals::WhichOccurrences &chosenOption,
                                      const QDate &itemDate = QDate(),
                                      const bool commitToCalendar = false );

  protected:
    void setIncidenceChanger( IncidenceChangerBase *changer );

//     // returns KMsgBox::OKCancel()
    int msgItemDelete( Incidence *incidence );

    Todo *selectedTodo();

    void warningChangeFailed( Incidence * );
    void checkForFilteredChange( Incidence *incidence );
    /** Adjust the given date/times by valid defaults (selection or configured
        defaults, if invalid values are given) and allow the view to adjust the
        type. */
    void dateTimesForNewEvent( QDateTime &startDt, QDateTime &endDt, bool &allDay );
    KOEventEditor *newEventEditor( ResourceCalendar *res, const QString &subRes,
                                   const QDateTime &startDtParam = QDateTime(),
                                   const QDateTime &endDtParam = QDateTime() ,
                                   bool allDayParam = false );

  private:
    void init();

    /**
      Returns the incidence that should be sent to clipboard.
      Usually it's just returns the selected incidence, but, if
      the incidence is recurring, it will ask the user what he wants to
      cut/paste and dissociate the incidence if necesssary.
    **/
    Incidence *incToSendToClipboard( bool cut );

    void calendarModified( bool, Calendar * );
    // Helper function for purgeCompleted that recursively purges a todo and
    // its subitems. If it cannot delete a completed todo (because it has
    // uncompleted subitems), notAllPurged is set to true.
    bool purgeCompletedSubTodos( Todo* todo, bool &notAllPurged );

    KOrg::History *mHistory;

    QSplitter    *mPanner;
    QSplitter    *mLeftSplitter;
    QWidget      *mLeftFrame;
    QWidgetStack *mRightFrame;

    // This navigator bar is used when in full window month view
    // It has nothing to do with the date navigator
    NavigatorBar *mNavigatorBar;

    DateNavigatorContainer *mDateNavigatorContainer;


    QPtrList<CalendarViewExtension> mExtensions;

    Calendar *mCalendar;

    DateNavigator *mDateNavigator;
    DateChecker *mDateChecker;

    KOEventViewer *mEventViewer;
    KOViewManager *mViewManager;
    KODialogManager *mDialogManager;

    // Calendar filters
    QPtrList<CalFilter> mFilters;
    CalFilter *mCurrentFilter;

    // various housekeeping variables.
    bool            mModified; // flag indicating if calendar is modified
    bool            mReadOnly; // flag indicating if calendar is read-only

    Incidence *mSelectedIncidence;
    QDate mSaveDate;

    KOTodoView *mTodoList;
    QMap<Incidence*,KOIncidenceEditor*> mDialogList;

    KOrg::IncidenceChangerBase *mChanger;
};




#endif
