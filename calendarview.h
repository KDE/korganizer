/*
    This file is part of KOrganizer.
    Copyright (c) 2000, 2001
    Cornelius Schumacher <schumacher@kde.org>

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
#ifndef CALENDARVIEW_H
#define CALENDARVIEW_H
// $Id$

#include <qframe.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qptrlist.h>
#include <qframe.h>
#include <qvbox.h>
#include <qmap.h>

#include <libkcal/calendar.h>
#include <libkcal/scheduler.h>

#include <korganizer/calendarviewbase.h>

#include "kotodoview.h"
#include "kdatenav.h"

class QWidgetStack;
class QSplitter;

class CalPrinter;
class KOFilterView;
class KOViewManager;
class KODialogManager;
class KOTodoView;

using namespace KCal;

/**
  This class provides the initialisation of a KOListViewItem for calendar
  components using the IncidenceVisitor.
*/
class CreateEditorVisitor : public Incidence::Visitor
{
  public:
    CreateEditorVisitor() {};
    ~CreateEditorVisitor() {};
    
    bool visit(Event *);
    bool visit(Todo *);
    bool visit(Journal *);
};


/**
  This is the main calendar widget. It provides the different vies on t he
  calendar data as well as the date navigator. It also handles synchronisation
  of the different views and controls the different dialogs like preferences,
  event editor, search dialog etc.
  
  @short main calendar view widget
  @author Cornelius Schumacher
  @version $Revision$
*/
class CalendarView : public KOrg::CalendarViewBase, public Calendar::Observer
{
    Q_OBJECT
  public:
    /**
      Constructs a new calendar view widget.
      @param parent parent window
      @param name Qt internal widget object name
    */
    CalendarView(QWidget *parent=0, const char *name=0 );
    virtual ~CalendarView();
  
    Calendar *calendar() { return mCalendar; }

    KOViewManager *viewManager();
    KODialogManager *dialogManager();

    QDate startDate();
    QDate endDate();

    QWidgetStack *viewStack();
    QWidget *leftFrame();

    KDateNavigator *dateNavigator();

    void addView(KOrg::BaseView *);
    void showView(KOrg::BaseView *);

    Incidence *currentSelection();

  signals:
  
    /** when change is made to options dialog, the topwidget will catch this
     *  and emit this signal which notifies all widgets which have registered 
     *  for notification to update their settings. */
    void configChanged();
    /** emitted when the topwidget is closing down, so that any attached
        child windows can also close. */
    void closingDown();
    /** emitted right before we die */
    void closed(QWidget *);
    
    /** Emitted when state of modified flag changes */
    void modifiedChanged(bool);
  
    /** Emitted when state of read-only flag changes */
    void readOnlyChanged(bool);
  
    /** Emitted when the unit of navigation changes */
    void changeNavStringPrev(const QString &);
    void changeNavStringNext(const QString &);
  
    /** Emitted when state of events selection has changed. */
    void eventsSelected(bool);
   /** Emitted when state of events selection has changed and user is organizer*/
    void organizerEventsSelected(bool);
    /** Emitted when state of events selection has changed and user is attendee*/
    void groupEventsSelected(bool);

    /** Emitted, when clipboard content changes. Parameter indicates if paste
     * is possible or not */
    void pasteEnabled(bool);
    
    /** Emitted, when the number of incoming messages has changed */
    void numIncomingChanged(int);

    /** Emitted, when the number of outgoing messages has changed */
    void numOutgoingChanged(int);

    /** Send status message, which can e.g. be displayed in the status bar */
    void statusMessage(const QString &);
    
    void calendarViewExpanded( bool );
    
    /** Emitted, when a todoitem is selected or deselected */
    void todoSelected( bool );
    
  public slots:
    /** options dialog made a changed to the configuration. we catch this
     *  and notify all widgets which need to update their configuration. */
    void updateConfig();

    /**
      Load calendar from file \a filename. If \a merge is true, load 
      calendar into existing one, if it is false, clear calendar, before
      loading. Return true, if calendar could be successfully loaded.
    */
    bool openCalendar(QString filename, bool merge=false);

    /**
      Save calendar data to file. Return true if calendar could be
      successfully saved.
    */
    bool saveCalendar(QString filename);
  
    /**
      Close calendar. Clear calendar data and reset views to display an empty
      calendar.
    */
    void closeCalendar();
  
    /** Archive old events of calendar */
    void archiveCalendar();

    void deleteIncidence();
  
    /** create an editeventwin with supplied date/time, and if bool is true,
     * make the event take all day. */
    void newEvent(QDateTime, QDateTime);
    void newEvent(QDateTime, QDateTime, bool allDay);
    void newEvent(QDateTime fh);
    void newEvent(QDate dt);
    /** create new event without having a date hint. Takes current date as
     default hint. */ 
    void newEvent();
    
    /** Create an editor for the supplied event. */
    void editEvent(Event *);
    /** Delete the supplied event. */
    void deleteEvent(Event *);
    /** Delete the event with the given unique ID. Returns false, if event wasn't
    found. */
    bool deleteEvent(const QString &uid);
    /** Create a read-only viewer dialog for the supplied event. */
    void showEvent(Event *);

    /** Create an editor dialog for a todo */
    void editTodo(Todo *);
    /** Create a read-only viewer dialog for the supplied todo */
    void showTodo(Todo *);
    /** create new todo */
    void newTodo();
    /** create new todo with a parent todo */
    void newSubTodo();
    /** create new todo with a parent todo */
    void newSubTodo(Todo *);
    /** Delete todo */
    void deleteTodo(Todo *);
    
    //void eventsSelected(QPtrList<Event>);
        
    /** Check if clipboard contains vCalendar event. The signal pasteEnabled() is
     * emitted as result. */
    void checkClipboard();
    
    /** slot that sets up a single shot timer to call the dateNavigator 
     * and update the current date when midnight comes around (if we are
     * running, of course)
     */
    void setupRollover();
   
    /** using the KConfig associated with the kapp variable, read in the
     * settings from the config file. 
     */
    void readSettings();
    
    /** write current state to config file. */
    void writeSettings();

    /** read settings for calendar filters */
    void readFilterSettings(KConfig *config);
    
    /** write settings for calendar filters */
    void writeFilterSettings(KConfig *config);

    /** passes on the message that an event has changed to the currently
     * activated view so that it can make appropriate display changes. */
    void changeEventDisplay(Event *, int);
  
    void eventAdded(Event *);
    void eventChanged(Event *);
    void eventToBeDeleted(Event *);
    void eventDeleted();
  
    void updateView(const QDate &start, const QDate &end);
    void updateView();
  
    /** Full update of visible todo views */
    void updateTodoViews();
  
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
  
    /**
      Pop up an Appointment Dialog to make a new appointment. Uses date that
      is currently selected in the dateNavigator.
    */
    void appointment_new();
    /** same as apptmnt_new, but sets "All Day Event" to true by default. */
    void allday_new();

    /** pop up a dialog to show an existing appointment. */
    void appointment_show();
    /**
     * pop up an Appointment Dialog to edit an existing appointment.	Get
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

    /** pop up a dialog to show an existing todo. */
    void todo_show();
    /** pop up a dialog to edit an existing todo. */
    void todo_edit();
    /* pop up dialog confirming deletion of currently selected todo */
    void todo_delete();

    /** Take ownership of selected event. */
    void takeOverEvent();

    /** Take ownership of all events in calendar. */
    void takeOverCalendar();
  
    /** query whether or not the calendar is "dirty". */
    bool isModified();
    /** set the state of calendar. Modified means "dirty", i.e. needing a save. */
    void setModified(bool modified=true);

    /** query if the calendar is read-only. */
    bool isReadOnly();
    /** set state of calendar to read-only */
    void setReadOnly(bool readOnly=true);

    void eventUpdated(Incidence *);

    /* iTIP scheduling actions */
    void schedule_publish(Incidence *incidence = 0);
    void schedule_request(Incidence *incidence = 0);
    void schedule_refresh(Incidence *incidence = 0);
    void schedule_cancel(Incidence *incidence = 0);
    void schedule_add(Incidence *incidence = 0);
    void schedule_reply(Incidence *incidence = 0);
    void schedule_counter(Incidence *incidence = 0);
    void schedule_declinecounter(Incidence *incidence = 0);
		
		void openAddressbook();

    void editFilters();

    void showFilter(bool visible);
    void updateFilter();
    void filterEdited();

    void showIntro();

    /** Move the current view date to today */
    void goToday();

    /** Move to the next date(s) in the current view */
    void goNext();
  
    /** Move to the previous date(s) in the current view */
    void goPrevious();

    void processEventSelection(bool selected);

    void toggleExpand();
    
    void todoSelect(bool);
    
    void dialogClosing(Incidence *);
  
  protected slots:
    /** Select a week to be displayed in the calendar view */
    void selectWeek(QDate weekstart);
  
    /** Select a view or adapt the current view to display the specified dates. */
    void selectDates(const DateList &);
  
  
  public:
    // show a standard warning
    // returns KMsgBox::yesNoCancel()
    int msgCalModified();
  
    void emitEventsSelected();

    /** Adapt navigation units correpsonding to step size of navigation of the
     * current view.
     */
    void adaptNavigationUnits();
    
    //Attendee* getYourAttendee(Event *event);
  
  protected:
    void schedule(Scheduler::Method, Incidence *incidence = 0);
    
    // returns KMsgBox::OKCandel()
    int msgItemDelete();
  
    /** tell the alarm daemon that we have saved, and he needs to reread */
    void signalAlarmDaemon();
  
    Todo *selectedTodo();
  
  private:
    void createPrinter();

    void calendarModified( bool, Calendar * );

    CalPrinter *mCalPrinter;

    QSplitter    *mPanner;
    QSplitter    *mLeftSplitter;
    QWidget      *mLeftFrame;
    QWidgetStack *mRightFrame;

    KDateNavigator *mDateNavigator;       // widget showing small month view.

    KOFilterView *mFilterView;

    // calendar object for this viewing instance
    Calendar      *mCalendar;

    KOViewManager *mViewManager;
    KODialogManager *mDialogManager;

    // Calendar filters
    QPtrList<CalFilter> mFilters;

    // various housekeeping variables.
    bool            mModified;	   // flag indicating if calendar is modified
    bool            mReadOnly; // flag indicating if calendar is read-only
    QDate mSaveSingleDate;
    int mEventsSelected;

    KOTodoView *mTodoList;
    QMap<Incidence*,QDialog*> mDialogList;
};

#endif
