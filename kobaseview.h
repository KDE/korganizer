#ifndef _KOBASEVIEW_H
#define _KOBASEVIEW_H
// $Id$
/*
 * KOBaseView is the abstract base class of all calendar views.
 * This file is part of the KOrganizer project.
 * (c) 1999 Preston Brown <pbrown@kde.org>
 */

#include <qwidget.h>
#include <qlist.h>

#include "event.h"
#include "calendar.h"

using namespace KCal;

class CalPrinter;

/**
  This class provides an interface for all views being displayed within the main
  calendar view. It has functions to update the view, to specify date range and
  other display parameter and to return selected objects. An important class,
  which inherits KOBaseView is KOEventView, which provides the interface for all
  views of event data like the agenda or the month view.

  @short Base class for calendar views
  @author Preston Brown
  @see KOTodoView, KOEventView, KOListView, KOAgendaView, KOWeekView, KOMonthView  
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
    KOBaseView(Calendar *cal, QWidget *parent = 0, const char *name = 0);

    /**
     * Destructor.  Views will do view-specific cleanups here.
     */
    virtual ~KOBaseView();
  
    /**
     * @return a list of selected events.  Most views can probably only
     * select a single event at a time, but some may be able to select
     * more than one.
     */
    virtual QList<Incidence> getSelected() = 0;
  
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
     * Return number of currently shown dates. A return value of 0 means no idea.
     */
    virtual int currentDateCount() = 0;

    /** Return if this view is an view for displaying events. */
    virtual bool isEventView() { return false; }
    
  public slots:
    /**
     * Updates the current display to reflect changes that may have happened
     * in the calendar since the last display refresh.
     */
    virtual void updateView() = 0;
  
    /**
      Write all unsaved data back to calendar store.
    */
    virtual void flushView() {}
  
    /**
     * Updates the current display to reflect the changes to one particular event.
     */
    virtual void changeEventDisplay(Event *, int) = 0;
  
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
    virtual void selectEvents(QList<Event> eventList) = 0;

  protected:
    Calendar *mCalendar;
};
  
#endif
