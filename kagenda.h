// 	$Id$

#ifndef KAgenda_included
#define KAgenda_included

#include <qmlined.h>
#include <qpopmenu.h>
#include <qlayout.h>
#include <qtablevw.h>
#include <qpainter.h>
#include <qscrbar.h>
#include <ktoolbar.h>
#include <kconfig.h>
#include <kapp.h>
#include <kiconloader.h>
#include "kpbutton.h"

#include "qdatelist.h"
#include "eventwidget.h"
#include "calobject.h"
#include "kpropdlg.h"

/** This class provides the sheet on which events are displayed. */
class KAgendaSheet : public QWidget
{
	Q_OBJECT
public:
	enum { SELECTING, DRAGGING, RESIZING };

/**	Contructor, takes the following parameters:
 *	@param style - 12/24 hour display
 *	@param size - pixels per hour cell
 *	@param cols - number of columns to make
 *	@param parent - the parent of this widget ( defaults to no parent )
 *	@param name - name of the widget ( defaults to no name )
 */
	KAgendaSheet( int style, int size, int cols, 
		      QWidget *parent = 0, char *name = 0 );

/**	Destructor */
	virtual ~KAgendaSheet();

/**	Set the time style to Time_12 of Time_24 */ 
	void setTimeStyle( int style );
	int timeStyle();

/**	styles for time display */
	enum { Time_24, Time_12 };

/**	Set number of columns to display */
	void setColumns( int cols );
	int columns();

/**	Set the size in pixels of an hour cell */
	void setRowSize( int );
	int rowSize();

/**	Return the size in pixels of a column */
	int colSize();

/** Return the size of the time bar */
	int timeWidth();

/**	Returns the start position in pixels for the grid */
	int gridOffset();

/**	Set child widget geometry to given dimensions */
	void setWidgetGeometry( QWidget *, int x, int y, int w, int h );

/**	Set child widget geometry to given time and column coords. */
	void setWidgetGeometry( QWidget *, QTime start, QTime end, int col = -1 );
	/** returns the time that has currently been clicked. */
	const QTime &getSelectedTime() const { return selTime; };
	const QDate &getSelectedDate() const { return selDate; };

/** set the font for the time bar */
	void setTimeFont( QFont timefont );
	
/** get the time bar font */
	QFont timeFont();

/** update widget after cofngi change */
	void updateConfig();

/** The date where the view starts to count.
 * this used to be in KAgenda, but I moved it here because AgendaSheet
 * needs to know about it to do selDate.  This whole view is a fucking
 * mess that needs a rewrite in 2.x in a major way.
 */
	QDate startDate;

signals:
/**	signals to clear the current event widget selection */
	void clearSelection();
	void popupActivated( int );
/**     signals that a new event is to be added at this column (for date,
        which KAgendaSheet doesn't really know about) and the specified
	start time. (This should be replaced by a generic signal - Fester ) */
        void addNewEventSignal(int, QTime);

protected:
/**	Handles resize events for the time subwidget */
	void resizeEvent( QResizeEvent *);

/**	Handles mouse press events on the agenda sheet */
	void mousePressEvent( QMouseEvent *);

/**     Handles mouse double click events on the agenda sheet */
        void mouseDoubleClickEvent(QMouseEvent *);

/**	Handles paint updates for the grid */
	void paintEvent( QPaintEvent *);

/**	Handles paint updates for the time bar */
	void timePaintEvent( QPaintEvent *);

/** Handles event for the time subwidget */
	bool eventFilter( QObject *, QEvent *);
	
/** restore custom colors after global palette change */
	void paletteChange(QPalette);


/**	Sub widget for time display */
	QFrame *TimeZone;

/**	agenda sheet popup menu */
	QPopupMenu *sheetPopup;

/**	Internal state variables */
	int RowSize,Columns,TimeStyle;
	QTime selTime;
	QDate selDate;
	int   selRow;
	QFont _timeFont;
	int _timeWidth;
};

/** This class provides a time scaled view of daily events. By default
  * it displays a 7 day week starting on monday, but it is able to 
  * display more or less days and starting on a specific date.
  */
class KAgenda : public QFrame
{
    Q_OBJECT

public:
/** Constructor using currentdate as a startdate */
    	KAgenda( CalObject*, QWidget *parent = 0, const char* name = 0 );
    	~KAgenda();

/** Returns the selected event in the agenda view. Returns 0 if no widget is selected */
	KOEvent *getSelected();

/** Returns Id of selected event in the agenda view. Returns -1 if no widget is selected */
	int getSelectedId();

/** Returns the current view of the agenda widget */
	int currentView() { return ViewType; }

  enum { DAY, WORKWEEK, WEEK, LIST };
  enum { EVENTADDED, EVENTEDITED, EVENTDELETED };

public slots:

 /** used to re-read configuration when it has been modified by the
  *  options dialog. */
	 void updateConfig();

/**     updates the display of the specified event */
        void changeEventDisplay(KOEvent *, int);

/* Refresh the Agenda View, using QDate as a start date.  */
//	void updateView( QDate );

/** Change the View. The AgendaStartWeek is used to determine which week is showed,
  * or in the list view which date is showed. 
  * If no value is given for an arg. the current value will be used.
  * The view type can be: 
  *         WORKWEEK - only do monday - friday,
  *         WEEK - 7 day week view 
  *         LIST - use agenda view with arbitrary number of selected days.
  */
	void setView( int ViewType );

/** Slot for selecting 1/5/7 day view */
	void slotViewChange(int newView);
	void slotViewChange() { slotViewChange(ViewType + 1 != LIST ?
					       ViewType + 1 : DAY); };


/* Add a date to the view date list. */
//	void addDate( QDate );

/** Put a list of dates in the view. Changes View type to LIST */
	void selectDates( const QDateList );

/** Slot for selecting next dates */
	void slotNextDates();

/** Slot for selecting prior dates */
	void slotPrevDates();


signals:
	void datesSelected( const QDateList );
	void newEventSignal( QDateTime );
	void editEventSignal( KOEvent *);
	void deleteEventSignal( KOEvent *);
	void toggleAlarmSignal( KOEvent *);
	void optionsSignal( int tab );

protected slots:

/**	handle changes caused in EventWidget, like dragging move/resize */
	void updateEventWidget( EventWidget *, int action );
	
/**     adds a new event on the sheet at the specified date/time,
	and selects the new event for a description edit.  The date
	is converted from the column number. */
	void addNewEvent(int col, QTime);

/**     Create a new event widget on the agenda view from the pointer
        passed (a KOEvent), and returns a pointer to the new widget */
        EventWidget *newEventWidget(KOEvent *, QDate displayDate);

/** Set the selected widget, catches the signal from the selected widget */
	void setSelected( EventWidget *selectedWidget );

/**     convert a given x coordinate into the proper date */
        QDate xposToDate(int x) { 
	  int dayOffset = (x - agendaSheet->gridOffset()) / 
	    agendaSheet->colSize();
	  return agendaSheet->startDate.addDays(dayOffset);
	};

/**     convert a given y coordinate to the proper time */
	QTime yposToTime(int y); 

/**	Slot for receiving sheetPopup signals */
	void sheetPopupSlot( int );

/** Slot for moving the scrollbar */
	void slotScrollBar( int );


protected:

/* Initialize Agenda view using startdate as a Starting Date for the view. */
//	void initAgenda( QDate startdate );

/**	Fill agenda with days of events starting on startdate */
	void fillAgenda();

/**	calculate the Column height = header + 48 * eventheader + eventbody
  *	this makes a resolution of 30 min. per event
  */
//	int columnHeight();

/**	calculate event height = header + body */
	int eventHeight();

/**	Access to events */
	CalObject *AgendaCal;  // pointer to calendar object
	
/**	Layout the event Widgets in the view */
	void doLayout();

/**	Take care of resize events */
	void resizeEvent( QResizeEvent *);

/** Process the keyevent, including the ignored keyevents of eventwidgets.
  * Implements pgup/pgdn and cursor key navigation in the view.
  */
  void keyPressEvent( QKeyEvent * );

/**	Icon loader */
	KIconLoader loader;

private:

/** The frames and layouts... to keep the view shape in form :-) */
	QLabel *dayFrame, *sheetFrame;
	QFrame *allDayFrame;
	KAgendaSheet *agendaSheet;
	QScrollBar *scrollBar;
	KPButton *nextdatbt, *prevdatbt, *viewbt;	
	int floatCount; // number of events which are in allDayFrame
	QList<KOEvent> floatEvents;

/** Various variables */ 
	QFont eventHeaderFont;
	QFont eventBodyFont;
	
	QList<EventWidget> WidgetList;
	QDateList selectedDates;
	QList<QLabel> dateLabels;

/** View properties */
	KConfig *config;
	int ViewType;
	int timeGridSize;
	int startScrollVal;
	bool weekStartsMonday;

/** Selected event in the view */
	EventWidget *selectedWidget;

/** Property dialog with tree list */
	KPropDlg *PropDlg;
};

#endif // kagenda_included
