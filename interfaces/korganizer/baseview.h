/*
    This file is part of the KOrganizer interfaces.
    Copyright (c) 1999 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KORG_BASEVIEW_H
#define KORG_BASEVIEW_H
// $Id$
// KOBaseView is the abstract base class of all calendar views.

#include <qwidget.h>
#include <qptrlist.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>

#include <libkcal/event.h>
#include <libkcal/calendar.h>

using namespace KCal;

class CalPrinter;

namespace KOrg {

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
class BaseView : public QWidget
{
    Q_OBJECT
  public:
    /**
     * Constructs a view. 
     * @param cal is a pointer to the calendar object from which events
     *        will be retrieved for display.
     */
    BaseView(Calendar *cal, QWidget *parent = 0, const char *name = 0) :
        QWidget(parent, name), mCalendar(cal) {}

    /**
     * Destructor.  Views will do view-specific cleanups here.
     */
    virtual ~BaseView() {}
  
    /**
     * @return a list of selected events.  Most views can probably only
     * select a single event at a time, but some may be able to select
     * more than one.
     */
    virtual QPtrList<Incidence> getSelected() = 0;
  
    /**
     * Generate a print preview of this event view.
     * @param calPrinter Calendar printer object used for printing
     * @param fd from date
     * @param td to date
     */
/*
  The date parameters should be determined by the view itself and not given as
  parameters. At the moment I just move the code from the topwidget to the
  individual views.
*/
    virtual void printPreview(CalPrinter *, 
                              const QDate &, const QDate &)
    {
      KMessageBox::sorry(this, i18n("Unfortunately, we don't handle printing for\n"
			            "that view yet.\n"));
    }
    
    /**
     * Print this event view.
     * @param calPrinter Calendar printer object used for printing
     */
    virtual void print(CalPrinter *)
    {
      KMessageBox::sorry(this, i18n("Unfortunately, we don't handle printing for\n"
	                            "that view yet.\n"));
    }
  
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
    virtual void selectEvents(QPtrList<Event> eventList) = 0;

  protected:
    Calendar *mCalendar;
};

}  
#endif
