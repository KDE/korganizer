/*
 * KOBaseView is the abstract base class of all calendar views.
 * This file is part of the KOrganizer project.
 * (c) 1999 Preston Brown <pbrown@kde.org>
 */


#ifndef _KOBASEVIEW_H
#define _KOBASEVIEW_H

#include <qwidget.h>
#include <qlist.h>

#include "koevent.h"

class CalObject;
class CalPrinter;

/**
 * KOBaseView is the abstract base class from which all other
 * calendar views are derived.  It provides methods for displaying
 * appointments and events on one or more days.  The actual number of
 * days that a view actually supports is not defined by this abstract class;
 * that is up to the classes that inherit from it.  It also provides
 * methods for updating the display, retrieving the currently selected
 * event (or events), and the like.
 * 
 * @short Abstract class from which all real views are derived.
 * @author Preston Brown <pbrown@kde.org>
 * @see KOListView, KOAgendaView, KOWeekView, KOMonthView
 */
class KOBaseView : public QWidget
{
  Q_OBJECT

public:
  /**
   * Constructs a view. 
   * @param cal is a pointer to the calendar object from which events
   *        will be retrieved for display.
   */
  KOBaseView(CalObject *cal, QWidget *parent = 0, const char *name = 0);

  /**
   * Destructor.  Views will do view-specific cleanups here.
   */
  virtual ~KOBaseView();

  /**
   * provides a hint back to the caller on the maximum number of dates
   * that the view supports.  A return value of 0 means no maximum.
   */
  virtual int maxDatesHint() = 0;

  /**
   * Return number of currently shown dates. A return value of 0 means no idea.
   */
  virtual int currentDateCount() = 0;

  /**
   * @return a list of selected events.  Most views can probably only
   * select a single event at a time, but some may be able to select
   * more than one.
   */
  virtual QList<KOEvent> getSelected() = 0;

  /**
   * Generate a print preview of this event view.
   * @param calPrinter Calendar printer object used for printing
   * @param fd from date
   * @param td to date
   */
  virtual void printPreview(CalPrinter *calPrinter, 
                            const QDate &fd, const QDate &td);
  
  /**
   * Print this event view.
   * @param calPrinter Calendar printer object used for printing
   */
  virtual void print(CalPrinter *calPrinter);

  /**
   * Construct a standard context menu for an event.
   */
  QPopupMenu *eventPopup();

public slots:
  /**
   * Updates the current display to reflect changes that may have happened
   * in the calendar since the last display refresh.
   */
  virtual void updateView() = 0;

  /**
   * Updates the current display to reflect the changes to one particular event.
   */
  virtual void changeEventDisplay(KOEvent *, int) = 0;

  /**
   * re-reads the KOrganizer configuration file and picks up relevant
   * changes which are applicable to the view.
   */
  virtual void updateConfig() {}

  /**
   * selects the dates specified in the list.  If the view cannot support
   * displaying all the dates requested, or it needs to change the dates
   * in some manner, it may call @see datesSelected.
   * @param dateList is the list of dates to try and select.
   */
  virtual void selectDates(const QDateList dateList) = 0;

  /**
   * Select events visible in the current display
   * @param eventList a list of events to select.
   */
  virtual void selectEvents(QList<KOEvent> eventList) = 0;

  /**
   * Show context menu for event.
   * @param event event, which is to be manipulated by the menu actions
   * @param popup a popop menu created with eventPopup()
   */
  void showEventPopup(QPopupMenu *popup,KOEvent *event);

  /**
   * Perform the default action for an event. E.g. open the event editor, when
   * double-clicking an event in the agenda view.
   */
  void defaultEventAction(KOEvent *event);
  
signals:
  /**
   * when the view changes the dates that are selected in one way or
   * another, this signal is emitted.  It should be connected back to
   * the @see KDateNavigator object so that it changes appropriately,
   * and any other objects that need to be aware that the list of 
   * selected dates has changed.
   */
  void datesSelected(const QDateList);
  /**   
   * instructs the receiver to begin editing the event specified in
   * some manner.  Doesn't make sense to connect to more than one 
   * receiver.
   */
  void editEventSignal(KOEvent *);
  /**
   * instructs the receiver to delete the event in some manner; some
   * possibilities include automatically, with a confirmation dialog
   * box, etc.  Doesn't make sense to connect to more than one receiver.
   */
  void deleteEventSignal(KOEvent *);
  /**
   * instructs the receiver to create a new event.  Doesn't make
   * sense to connect to more than one receiver.
   */
  void newEventSignal();
  /**
   * instructs the receiver to create a new event, with the specified
   * beginning end ending times.  Doesn't make sense to connect to more
   * than one receiver.
   */
  void newEventSignal(QDateTime, QDateTime);

  /**
   * instructs the receiver to show the event in read-only mode.
   */
  void showEventSignal(KOEvent *);

  /**
   * Emitted, when events are selected or deselected. The argument is true, if
   * there are selected events and false if there are no selected events.
   */
  void eventsSelected(bool selected);
  
protected slots:
  void popupShow();
  void popupEdit();
  void popupDelete();

protected:
  CalObject *mCalendar;
  KOEvent *mCurrentEvent;  // event selected e.g. for a context menu
};

#endif
