/*
  This file is part of KOrganizer.

  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef KORG_KOEVENTVIEW_H
#define KORG_KOEVENTVIEW_H

#include "korganizer/baseview.h"

namespace Akonadi {
  class Item;
}

class KOEventPopupMenu;

class QMenu;

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
  @see KOListView, KOAgendaView, KOMonthView
*/
class KOEventView : public KOrg::BaseView
{
  Q_OBJECT
  public:
    enum {
      // This value is passed to QColor's lighter(int factor) for selected events
      BRIGHTNESS_FACTOR = 125
    };

    /**
     * Constructs a view.
     * @param cal is a pointer to the calendar object from which events
     *        will be retrieved for display.
     * @param parent is the parent QWidget.
     */
    explicit KOEventView( QWidget *parent=0 );

    /**
     * Destructor.  Views will do view-specific cleanups here.
     */
    virtual ~KOEventView();

    /**
     * provides a hint back to the caller on the maximum number of dates
     * that the view supports.  A return value of 0 means no maximum.
     */
    virtual int maxDatesHint() const = 0;

    /**
     * Construct a standard context menu for an event.
     */
    KOEventPopupMenu *eventPopup();

    /**
     * Construct a standard context that allows to create a new event.
     */
    QMenu *newEventPopup();

    /** This view is a view for displaying events. */
    bool isEventView() { return true; }

    /*
     * Sets the QObject that will receive key events that were made
     * while the new event dialog was still being created.
     *
     * This is virtual so KOAgendaView can call EventViews::AgendaView::setTypeAheadReceiver().
     * because not all views are in kdepim/calendarviews yet
     *
     */
    virtual void setTypeAheadReceiver( QObject *o );

    /*
     * Returns true if the view item, that represents a to-do, should use the "completed"
     * pixmap.
     *
     * @param todo The to-do associated with the view item.
     * @param date The date in which the item appears in the view, for non recuring to-dos
     * this is the same as the start date, but, for recurring to-dos this is the date of
     * a particular occurrence.
     *
     */
    static bool usesCompletedTodoPixmap( const Akonadi::Item &todo, const QDate &date );

    bool supportsDateNavigation() const { return true; }

  public slots:
    void focusChanged( QWidget *, QWidget * );

    /**
     * Performs the default action for an incidence, e.g. open the event editor,
     * when double-clicking an event in the agenda view.
     */
    void defaultAction( const Akonadi::Item &incidence );

  signals:
    /**
     * When the view changes the dates that are selected in one way or
     * another, this signal is emitted.  It should be connected back to
     * the KDateNavigator object so that it changes appropriately,
     * and any other objects that need to be aware that the list of
     * selected dates has changed.
     *   @param datelist the new list of selected dates
     */
    void datesSelected( const KCalCore::DateList datelist );

    /**
     * Emitted when an event is moved using the mouse in an agenda
     * view (week / month).
     */
    void shiftedEvent( const QDate &olddate, const QDate &ewdate );

  protected slots:
    void popupShow();
    void popupEdit();
    void popupDelete();
    void popupCut();
    void popupCopy();
    virtual void showNewEventPopup();

  protected:
    Akonadi::Item mCurrentIncidence;  // Incidence selected e.g. for a context menu

  private:
    /*
     * This is called when the new event dialog is shown. It sends
     * all events in mTypeAheadEvents to the receiver.
     */
    void finishTypeAhead();

  private:

    /* When we receive a QEvent with a key_Return release
     * we will only show a new event dialog if we previously received a
     * key_Return press, otherwise a new event dialog appears when
     * you hit return in some yes/no dialog */
    bool mReturnPressed;

    bool mTypeAhead;
    QObject *mTypeAheadReceiver;
    QList<QEvent*> mTypeAheadEvents;
};

#endif
