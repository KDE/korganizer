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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef _KOEVENTVIEW_H
#define _KOEVENTVIEW_H

#include <libkcal/incidencebase.h>

#include <korganizer/baseview.h>


namespace KCal {
class Incidence;
}
using namespace KCal;

class KOEventPopupMenu;
class CalPrinter;
class QPopupMenu;

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
     * @param parent is the parent QWidget.
     * @param name is the view name.
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

    /**
     * Construct a standard context that allows to create a new event.
     */
    QPopupMenu *newEventPopup();

    /** This view is an view for displaying events. */
    bool isEventView() { return true; }

  public slots:

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
    virtual void showNewEventPopup();

  protected:
    Incidence *mCurrentIncidence;  // Incidence selected e.g. for a context menu
};

#endif
