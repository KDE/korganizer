/*
    This file is part of KOrganizer.
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef _KOEVENTVIEW_H
#define _KOEVENTVIEW_H
// $Id$

#include <libkcal/calendar.h>
#include <libkcal/event.h>

#include <korganizer/baseview.h>

#include "koeventpopupmenu.h"

using namespace KCal;

class CalPrinter;

/**
  KOEventView is the abstract base class from which all other
  calendar views for event data are derived.  It provides methods for
  displaying
  appointments and events on one or more days.  The actual number of
  days that a view actually supports is not defined by this abstract class;
  that is up to the classes that inherit from it.  It also provides
  methods for updating the display, retrieving the currently selected
  event (or events), and the like.
  
  @short Abstract class from which all event views are derived.
  @author Preston Brown <pbrown@kde.org>
  @see KOListView, KOAgendaView, KOWeekView, KOMonthView
*/
class KOEventView : public KOrg::BaseView
{
    Q_OBJECT
  
  public:
    /**
     * Constructs a view. 
     * @param cal is a pointer to the calendar object from which events
     *        will be retrieved for display.
     */
    KOEventView(Calendar *cal,QWidget *parent=0,const char *name=0);
  
    /**
     * Destructor.  Views will do view-specific cleanups here.
     */
    virtual ~KOEventView();
  
    /**
     * provides a hint back to the caller on the maximum number of dates
     * that the view supports.  A return value of 0 means no maximum.
     */
    virtual int maxDatesHint() = 0;
      
    /**
     * Construct a standard context menu for an event.
     */
    KOEventPopupMenu *eventPopup();

    /** This view is an view for displaying events. */
    bool isEventView() { return true; }
  
  public slots:
  
    /**
     * Show context menu for event.
     * @param event event, which is to be manipulated by the menu actions
     * @param popup a popop menu created with eventPopup()
     */
    void showEventPopup(QPopupMenu *popup,Event *event);

    /**
     Perform the default action for an incidence, e.g. open the event editor,
     when double-clicking an event in the agenda view.
    */
    void defaultAction( Incidence * );
    
  signals:
    /**
     * when the view changes the dates that are selected in one way or
     * another, this signal is emitted.  It should be connected back to
     * the @see KDateNavigator object so that it changes appropriately,
     * and any other objects that need to be aware that the list of 
     * selected dates has changed.
     */
    void datesSelected(const DateList);
    /**   
     * instructs the receiver to begin editing the event specified in
     * some manner.  Doesn't make sense to connect to more than one 
     * receiver.
     */
    void editEventSignal(Event *);
    /**
     * instructs the receiver to delete the event in some manner; some
     * possibilities include automatically, with a confirmation dialog
     * box, etc.  Doesn't make sense to connect to more than one receiver.
     */
    void deleteEventSignal(Event *);
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
    void showEventSignal(Event *);

    /**
     * Emitted, when events are selected or deselected. The argument is true, if
     * there are selected events and false if there are no selected events.
     */
    void eventsSelected(bool selected);

    //ET CVS MERGE !
    /**
     * Emitted when an event is moved using the mouse in an agenda
     * view (week / month).
     */
    void shiftedEvent(const QDate& olddate, const QDate& newdate);


  protected slots:
    void popupShow();
    void popupEdit();
    void popupDelete();

  protected:
    Event *mCurrentEvent;  // event selected e.g. for a context menu
};

#endif
