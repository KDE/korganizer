/* 	$Id$	 */

#ifndef _CALENDARVIEW_H
#define _CALENDARVIEW_H

#include <qframe.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qlist.h>
#include <qtabdlg.h>
#include <qframe.h>
#include <qsplitter.h>
#include <qvbox.h>

#include <ktoolbar.h>
#include <kapp.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kmenubar.h>

#include "qdatelist.h"
#include "calobject.h"
#include "kdatenav.h"
//#include "eventwin.h"
//#include "editeventwin.h"
//#include "todoeventwin.h"
#include "koagendaview.h"
//#include "listview.h"
#include "kolistview.h"
#include "kotodoview.h"
#include "komonthview.h"
#include "optionsdlg.h"
#include "searchdialog.h"

class CalPrinter;
class ExportWebDialog;
class KOOptionsDialog;

/**
 *
 * This is the main class for KOrganizer. It extends the KDE KTMainWindow.
 * it provides the main view that the user sees upon startup, as well as
 * menus, buttons, etc. etc.
 *
 * @short constructs a new main window for korganizer
 * @author Preston Brown
 * @version $Revision$
 */
class CalendarView : public QWidget
{
  Q_OBJECT
public:
  /**
   *
   * Constructs a new main window.
   *
   * @param cal is a newly allocated calendar passed from the main program
   * @param parent this should usually be 0, unless it is a child win (when?)
   * @param name this is the name of the window to display in the titlebar
   * @param fnOverride specifies whether if the filename given is empty,
   * KOrg. will try to obtain a filename from the config file instead of 
   * starting up with an empty calendar.
   *
   */
  CalendarView(QString filename="", QWidget *parent=0, const char *name=0 );
  virtual ~CalendarView();

  // View Types in enum
  enum { AGENDAVIEW, LISTVIEW, MONTHVIEW, TODOVIEW };
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

public slots:
  /** options dialog made a changed to the configuration. we catch this
   *  and notify all widgets which need to update their configuration. */
  void updateConfig();

  /** set calendar file to be displayed. */
  bool openCalendar(QString filename);

  /** merge data from calendar file into currently displayed calendar. */
  bool mergeCalendar(QString filename);

  /** Save calendar data to file. */
  bool saveCalendar(QString filename);

  /** Close calendar */
  void closeCalendar();

  /** create an editeventwin with supplied date/time, and if bool is true,
   * make the event take all day. */
  void newEvent(QDateTime, QDateTime);
  void newEvent(QDateTime, QDateTime, bool allDay);
  void newEvent(QDateTime fh) { newEvent(fh,QDateTime(fh.addSecs(3600))); };
  void newEvent(QDate dt) { newEvent(QDateTime(dt, QTime(0,0,0)),
	   		    QDateTime(dt, QTime(0,0,0)), TRUE); };
  /** create new event without having a date hint. Takes current date as
   default hint. */ 
  void newEvent();
  /** create an editeventwin for the supplied event */
  void editEvent(KOEvent *);
  /** delete the supplied event */
  void deleteEvent(KOEvent *);

  /** create new todo */
  void newTodo();  
  /** create new todo with a parent todo */
  void newSubTodo(KOEvent *);
  
  //void eventsSelected(QList<KOEvent>);
  
  /** change Agenda view */
  void changeAgendaView( int view );

  /** next Agenda view */
  void nextAgendaView();


// Made public from protected because we call these slots now from KOrganizer
// main view widget.
 public slots:
  /** slot that sets up a single shot timer to call the dateNavigator 
   * and update the current date when midnight comes around (if we are
   * running, of course)
   */
  void setupRollover();
 
  /** using the KConfig associated with the kapp variable, read in the
   * settings from the config file. 
   */
  void readSettings();
  
  /** using the KConfig associated with the kapp variable, read in the
   * settings for the current view from the config file. 
   */
  void readCurrentView();

  /** write current state to config file. */
  void writeSettings();

  /** passes on the message that an event has changed to the currently
   * activated view so that it can make appropriate display changes. */
  void changeEventDisplay(KOEvent *, int);

  void eventAdded(KOEvent *);
  void eventChanged(KOEvent *);
  void eventToBeDeleted(KOEvent *);
  void eventDeleted();

  /** changes the view to be the currently selected view */
  void changeView(KOBaseView *);
  
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

  /**
   *
   * pop up an Appointment Dialog to make a new appointment. Uses date that
   * is currently selected in the dateNavigator.
   */
  void appointment_new();
  /** same as apptmnt_new, but sets "All Day Event" to true by default. */
  void allday_new();
  /**
   *
   * pop up an Appointment Dialog to edit an existing appointment.	Get
   * information on the appointment from the list of unique IDs that is
   * currently in the View, called currIds.
   */
  void appointment_edit();
  /**
   *
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

  /** query whether or not the calendar is "dirty". */
  bool isModified();
  /** set the state of calendar. Modified means "dirty", i.e. needing a save. */
  void setModified(bool modified=true);

  void eventUpdated(KOEvent *);

  void view_list();
  void view_day();
  void view_workweek();
  void view_week();
  void view_month();
  void view_todolist();

protected slots:
  // clean up after a child window closes
  void cleanWindow(QWidget *);

  /** Move the current view date to today */
  void goToday();

  /** Move to the next date(s) in the current view */
  void goNext();

  /** Move to the previous date(s) in the current view */
  void goPrevious();
  
public:
  // show a standard warning
  // returns KMsgBox::yesNoCancel()
  int msgCalModified();

protected:
  void hookupSignals();
  bool initCalendar(QString filename);

  // returns KMsgBox::OKCandel()
  int msgItemDelete();

  /** tell the alarm daemon that we have saved, and he needs to reread */
  void signalAlarmDaemon();


  // variables

  // Other variables
  CalPrinter *calPrinter;

  // the main display space frame. note that this is neccessary to
  // avoid things being put under the toolbar
  QSplitter    *panner;
  QFrame       *mainFrame;
  QFrame       *leftFrame;
  QWidgetStack *rightFrame;

  QFrame         *dateNavFrame;
  KDateNavigator *dateNavigator;       // widget showing small month view.

  KOAgendaView   *agendaView;          // "week" view
  KOListView     *listView;            // "list/day" view
  KOMonthView    *monthView; // a frame to hold the month view
  KOTodoView     *todoList, *todoView;

  KOBaseView     *currentView;  // currently active event view
  QList<KOBaseView> mCalendarViews;  // list of available calendar views

  // view mode of agenda view (day, workweek, week, ...)
  int             agendaViewMode;

  // calendar object for this viewing instance
  CalObject      *mCalendar;

  // various housekeeping variables.
  bool            mModified;	   // flag indicating if calendar is modified
  QDate saveSingleDate;                

  // dialogs
  KOOptionsDialog *mOptionsDialog;
  SearchDialog *searchDlg;
  ExportWebDialog *mExportWebDialog;
};

#endif // _CALENDARVIEW_H

