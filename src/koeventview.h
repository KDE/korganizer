/*
  This file is part of KOrganizer.

  SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2000, 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#pragma once

#include "baseview.h"

namespace Akonadi
{
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
    explicit KOEventView(QWidget *parent = nullptr);

    /**
     * Destructor.  Views will do view-specific cleanups here.
     */
    ~KOEventView() override;

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
    bool isEventView() override
    {
        return true;
    }

    /*
     * Sets the QObject that will receive key events that were made
     * while the new event dialog was still being created.
     *
     * This is virtual so KOAgendaView can call EventViews::AgendaView::setTypeAheadReceiver().
     * because not all views are in kdepim/calendarviews yet
     *
     */
    virtual void setTypeAheadReceiver(QObject *o);

    bool supportsDateNavigation() const override
    {
        return true;
    }

public Q_SLOTS:
    void focusChanged(QWidget *, QWidget *);

Q_SIGNALS:
    /**
     * When the view changes the dates that are selected in one way or
     * another, this signal is emitted.  It should be connected back to
     * the KDateNavigator object so that it changes appropriately,
     * and any other objects that need to be aware that the list of
     * selected dates has changed.
     *   @param datelist the new list of selected dates
     */
    void datesSelected(const KCalendarCore::DateList &datelist);

    /**
     * Emitted when an event is moved using the mouse in an agenda
     * view (week / month).
     */
    void shiftedEvent(const QDate &olddate, const QDate &ewdate);

protected Q_SLOTS:
    void popupShow();
    void popupEdit();
    void popupDelete();
    void popupCut();
    void popupCopy();
    virtual void showNewEventPopup();

protected:
    Akonadi::Item mCurrentIncidence; // Incidence selected e.g. for a context menu

private:
    /*
     * This is called when the new event dialog is shown. It sends
     * all events in mTypeAheadEvents to the receiver.
     */
    void finishTypeAhead();

private:
    bool mTypeAhead = false;
    QObject *mTypeAheadReceiver = nullptr;
    QList<QEvent *> mTypeAheadEvents;
};
