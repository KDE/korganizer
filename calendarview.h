/* 	$Id$	 */

#ifndef _CALENDARVIEW_H
#define _CALENDARVIEW_H

#include <qframe.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qlist.h>
#include <qframe.h>
#include <qsplitter.h>
#include <qvbox.h>

#include <ktoolbar.h>
#include <kapp.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kmenubar.h>

#include "qdatelist.h"
#include "calendar.h"
#include "kdatenav.h"
#include "koagendaview.h"
#include "kolistview.h"
#include "kotodoview.h"
#include "komonthview.h"
#include "searchdialog.h"
#include "scheduler.h"

class QWidgetStack;
class CalPrinter;
class ExportWebDialog;
class KOPrefsDialog;
class ArchiveDialog;
class OutgoingDialog;
class IncomingDialog;
class CategoryEditDialog;
class KOFilterView;
class KOProjectView;
class FilterEditDialog;
class KOWhatsNextView;
class KOJournalView;
class KOEventEditor;
class KOTodoEditor;
class PluginDialog;

using namespace KCal;

/**
  This class provides the initialisation of a KOListViewItem for calendar
  components using the IncidenceVisitor.
*/
class CreateEditorVisitor : public IncidenceVisitor
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
class CalendarView : public QWidget
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
  
    enum { EVENTADDED, EVENTEDITED, EVENTDELETED };
  
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
  
    /** Emitted, when clipboard content changes. Parameter indicates if paste
     * is possible or not */
    void pasteEnabled(bool);
    
    /** Emitted, when the number of incoming messages has changed */
    void numIncomingChanged(int);

    /** Emitted, when the number of outgoing messages has changed */
    void numOutgoingChanged(int);

    /** Send status message, which can e.g. be displayed in the status bar */
    void statusMessage(const QString &);
    
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
  
    /** create an editeventwin with supplied date/time, and if bool is true,
     * make the event take all day. */
    void newEvent(QDateTime, QDateTime);
    void newEvent(QDateTime, QDateTime, bool allDay);
    void newEvent(QDateTime fh);
    void newEvent(QDate dt);
    /** create new event without having a date hint. Takes current date as
     default hint. */ 
    void newEvent();
    
    /** create an editeventwin for the supplied event */
    void editEvent(Event *);
    /** delete the supplied event */
    void deleteEvent(Event *);
    /** delete the event with unique ID VUID. Returns false, if event wasn't
    found */
    bool deleteEvent(const QString &VUID);
    /** Create a read-only viewer dialog for the supplied event */
    void showEvent(Event *);

    /** Create an editor dialog for a todo */
    void editTodo(Todo *);
    /** Create a read-only viewer dialog for the supplied todo */
    void showTodo(Todo *);
    /** create new todo */
    void newTodo();
    /** create new todo with a parent todo */
    void newSubTodo(Todo *);
    /** Delete todo */
    void deleteTodo(Todo *);
    
    //void eventsSelected(QList<Event>);
    
    /** change Agenda view */
    void changeAgendaView( int view );
  
    /** next Agenda view */
    void nextAgendaView();
  
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
    
    /** Read which view was shown last from config file */
    void readCurrentView(KConfig *);
    /** Write which view is currently shown to config file */
    void writeCurrentView(KConfig *);
  
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
  
    void updateView(const QDateList);
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
     *functions for printing, previewing a print, and setting up printing
     * parameters 
     */
    void print();
    void printSetup();
    void printPreview();
  
    /** Export calendar as web page */
    void exportWeb();
  
    /** Export as iCalendar file */
    void exportICalendar();
  
    /** Export as vCalendar file */
    void exportVCalendar();
  
  
    /**
     *
     * pop up an Appointment Dialog to make a new appointment. Uses date that
     * is currently selected in the dateNavigator.
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
  
    /**
     *
     * deletes the currently selected todo item, if one is selected.
     */
    void action_deleteTodo();
    /**
     * allows the user to enter a search RegExp and a date range, and then
     * displays / returns a list of appointments that fit the criteria.
     */
    void action_search();
  
    /** mails the currently selected event to a particular user as a vCalendar 
      attachment. */
    void action_mail();

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
  
    void showWhatsNextView();
    void showListView();
    void showAgendaView();
    void showDayView();
    void showWorkWeekView();
    void showWeekView();
    void showMonthView();
    void showTodoView();
    void showProjectView();
    void showJournalView();

    void schedule_outgoing();
    void schedule_incoming();

    /* iTIP scheduling actions */  
    void schedule_publish();
    void schedule_request();
    void schedule_refresh();
    void schedule_cancel();
    void schedule_add();
    void schedule_reply();
    void schedule_counter();
    void schedule_declinecounter();

    void editCategories();
    void editFilters();

    void showFilter(bool visible);
    void updateFilter();
    void filterEdited();

    void showIntro();

    void configurePlugins();

  protected slots:
    /** Move the current view date to today */
    void goToday();
  
    /** Move to the next date(s) in the current view */
    void goNext();
  
    /** Move to the previous date(s) in the current view */
    void goPrevious();
  
    /** Select a week to be displayed in the calendar view */
    void selectWeek(QDate weekstart);
  
    /** Select a view or adapt the current view to display the specified dates. */
    void selectDates(const QDateList);
  
    void processEventSelection(bool selected);
  
  public:
    // show a standard warning
    // returns KMsgBox::yesNoCancel()
    int msgCalModified();
  
    void emitEventsSelected();

    Incidence *currentSelection();
  
  protected:
    void schedule(Scheduler::Method);
    
    // returns KMsgBox::OKCandel()
    int msgItemDelete();
  
    /** tell the alarm daemon that we have saved, and he needs to reread */
    void signalAlarmDaemon();
  
    /** Adapt navigation units correpsonding to step size of navigation of the
     * current view.
     */
    void adaptNavigationUnits();

    /** changes the view to be the currently selected view */
    void showView(KOBaseView *);

    /** Get an editor dialog for an Event. */
    KOEventEditor *getEventEditor();
    
    /** Get an editor dialog for a Todo. */
    KOTodoEditor *getTodoEditor();
    
  private:
    void raiseCurrentView();

    void createOptionsDialog();
    void createIncomingDialog();
    void createOutgoingDialog();
    void createPrinter();
  
    CalPrinter *mCalPrinter;
  
    QSplitter    *mPanner;
    QSplitter    *mLeftFrame;
    QWidgetStack *mRightFrame;
  
    KDateNavigator *mDateNavigator;       // widget showing small month view.
    
    KOFilterView *mFilterView;
  
    KOAgendaView    *mAgendaView;          // "week" view
    KOListView      *mListView;            // "list/day" view
    KOMonthView     *mMonthView; 
    KOTodoView      *mTodoView;
    KOTodoView      *mTodoList;  // Small todo list under date navigator
    KOProjectView   *mProjectView;
    KOWhatsNextView *mWhatsNextView;
    KOJournalView   *mJournalView;
  
    KOBaseView     *mCurrentView;  // currently active event view
  
    // calendar object for this viewing instance
    Calendar      *mCalendar;
  
    // Calendar filters
    QList<CalFilter> mFilters;
  
    // various housekeeping variables.
    bool            mModified;	   // flag indicating if calendar is modified
    bool            mReadOnly; // flag indicating if calendar is read-only
    QDate mSaveSingleDate;                
    int mEventsSelected;
    int mAgendaViewMode;
  
    // dialogs
    KOPrefsDialog *mOptionsDialog;
    SearchDialog *mSearchDialog;
    ExportWebDialog *mExportWebDialog;
    ArchiveDialog *mArchiveDialog;
    OutgoingDialog *mOutgoingDialog;
    IncomingDialog *mIncomingDialog;
    CategoryEditDialog *mCategoryEditDialog;
    FilterEditDialog *mFilterEditDialog;
    PluginDialog *mPluginDialog;
};

#endif // _CALENDARVIEW_H
